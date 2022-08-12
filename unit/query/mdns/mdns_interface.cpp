//
// Created by WJG on 2022-5-7.
//

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <iphlpapi.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <netdb.h>
#include <ifaddrs.h>
#endif

#include "mdns.h"
#include "mdns_interface.h"
#include "sitemanager/StringUtils.h"
#include "sitemanager/SiteTree.h"

#include <thread>
#include <arpa/inet.h>

static char addrbuffer[64];
static char entrybuffer[256];
static char namebuffer[256];
static char sendbuffer[256];
static mdns_record_txt_t txtbuffer[128];

static uint32_t service_address_ipv4;
static uint8_t service_address_ipv6[16];

static int has_ipv4;
static int has_ipv6 = 0;

static std::unordered_set<std::string> mdns_answer_querysite_set;

typedef struct {
    const char* service;
    const char* hostname;
    uint32_t address_ipv4;
    uint8_t* address_ipv6;
    int port;
} service_record_t;

const char* global_hostname = MY_HOST_NAME;
const char* global_service_name = SITE_QUERY_SERVICE_NAME;
int global_service_port = SITE_QUERY_SERVICE_PORT;

mdns_string_t ipv4_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in* addr,
                                     size_t addrlen) {
    char host[NI_MAXHOST] = {0};
    char service[NI_MAXSERV] = {0};
    int ret = getnameinfo((const struct sockaddr*)addr, (socklen_t)addrlen, host, NI_MAXHOST,
                          service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
    int len = 0;
    if (ret == 0)
    {
//        if (addr->sin_port != 0)
//            len = snprintf(buffer, capacity, "%s:%s", host, service);
//        else
            len = snprintf(buffer, capacity, "%s", host);
    }
    if (len >= (int)capacity)
        len = (int)capacity - 1;
    mdns_string_t str;
    str.str = buffer;
    str.length = len;
    return str;
}

mdns_string_t ipv6_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in6* addr,
                                     size_t addrlen) {
    char host[NI_MAXHOST] = {0};
    char service[NI_MAXSERV] = {0};
    int ret = getnameinfo((const struct sockaddr*)addr, (socklen_t)addrlen, host, NI_MAXHOST,
                          service, NI_MAXSERV, NI_NUMERICSERV | NI_NUMERICHOST);
    int len = 0;
    if (ret == 0)
    {
        if (addr->sin6_port != 0)
            len = snprintf(buffer, capacity, "[%s]:%s", host, service);
        else
            len = snprintf(buffer, capacity, "%s", host);
    }
    if (len >= (int)capacity)
        len = (int)capacity - 1;
    mdns_string_t str;
    str.str = buffer;
    str.length = len;
    return str;
}

mdns_string_t ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen) {
    if (addr->sa_family == AF_INET6)
        return ipv6_address_to_string(buffer, capacity, (const struct sockaddr_in6*)addr, addrlen);
    return ipv4_address_to_string(buffer, capacity, (const struct sockaddr_in*)addr, addrlen);
}

int get_myself_ipv4addr(const struct sockaddr* sockaddr) {
#ifdef _WIN32

    IP_ADAPTER_ADDRESSES* adapter_address = 0;
	ULONG address_size = 8000;
	unsigned int ret;
	unsigned int num_retries = 4;
	do {
		adapter_address = malloc(address_size);
		ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0,
		                           adapter_address, &address_size);
		if (ret == ERROR_BUFFER_OVERFLOW) {
			free(adapter_address);
			adapter_address = 0;
		} else {
			break;
		}
	} while (num_retries-- > 0);

	if (!adapter_address || (ret != NO_ERROR)) {
		free(adapter_address);
		printf("Failed to get network adapter addresses\n");
		return num_sockets;
	}

	int first_ipv4 = 1;
	int first_ipv6 = 1;
	for (PIP_ADAPTER_ADDRESSES adapter = adapter_address; adapter; adapter = adapter->Next) {
		if (adapter->TunnelType == TUNNEL_TYPE_TEREDO)
			continue;
		if (adapter->OperStatus != IfOperStatusUp)
			continue;

		for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
		     unicast = unicast->Next) {
			if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
				struct sockaddr_in* saddr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
				if ((saddr->sin_addr.S_un.S_un_b.s_b1 != 127) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b2 != 0) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b3 != 0) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b4 != 1)) {
					int log_addr = 0;
					if (first_ipv4) {
						service_address_ipv4 = saddr->sin_addr.S_un.S_addr;
						first_ipv4 = 0;
						log_addr = 1;
					}
					has_ipv4 = 1;
					if (num_sockets < max_sockets) {
						saddr->sin_port = htons((unsigned short)port);
						int sock = mdns_socket_open_ipv4(saddr);
						if (sock >= 0) {
							sockets[num_sockets++] = sock;
							log_addr = 1;
						} else {
							log_addr = 0;
						}
					}
					if (log_addr) {
						char buffer[128];
						mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
						                                            sizeof(struct sockaddr_in));
						printf("Local IPv4 address: %.*s\n", MDNS_STRING_FORMAT(addr));
					}
				}
			} else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
				struct sockaddr_in6* saddr = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
				static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0,
				                                          0, 0, 0, 0, 0, 0, 0, 1};
				static const unsigned char localhost_mapped[] = {0, 0, 0,    0,    0,    0, 0, 0,
				                                                 0, 0, 0xff, 0xff, 0x7f, 0, 0, 1};
				if ((unicast->DadState == NldsPreferred) &&
				    memcmp(saddr->sin6_addr.s6_addr, localhost, 16) &&
				    memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16)) {
					int log_addr = 0;
					if (first_ipv6) {
						memcpy(service_address_ipv6, &saddr->sin6_addr, 16);
						first_ipv6 = 0;
						log_addr = 1;
					}
					has_ipv6 = 1;
					if (num_sockets < max_sockets) {
						saddr->sin6_port = htons((unsigned short)port);
						int sock = mdns_socket_open_ipv6(saddr);
						if (sock >= 0) {
							sockets[num_sockets++] = sock;
							log_addr = 1;
						} else {
							log_addr = 0;
						}
					}
					if (log_addr) {
						char buffer[128];
						mdns_string_t addr = ipv6_address_to_string(buffer, sizeof(buffer), saddr,
						                                            sizeof(struct sockaddr_in6));
						printf("Local IPv6 address: %.*s\n", MDNS_STRING_FORMAT(addr));
					}
				}
			}
		}
	}

	free(adapter_address);

#else

    struct ifaddrs* ifaddr = 0;
    struct ifaddrs* ifa = 0;

    if (getifaddrs(&ifaddr) < 0)
        printf("Unable to get interface addresses\n");

    struct sockaddr_in *from_addr = (struct sockaddr_in *)(sockaddr);

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
            if((saddr->sin_addr.s_addr & 0xffffff)== (from_addr->sin_addr.s_addr & 0xffffff))
            {
                service_address_ipv4 = saddr->sin_addr.s_addr;
                has_ipv4 = 1;

                char buffer[64] = {0x00};
                mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
                                                            sizeof(struct sockaddr_in));
                printf("Local IPv4 address: %.*s\n", MDNS_STRING_FORMAT(addr));
                break;
            }
        }
    }

    freeifaddrs(ifaddr);

    return 0;

#endif

}

int query_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
                   uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                   size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                   size_t record_length, void* user_data) {
    (void)sizeof(sock);
    (void)sizeof(query_id);
    (void)sizeof(name_length);
    (void)sizeof(user_data);
    mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);
    const char* entrytype = (entry == MDNS_ENTRYTYPE_ANSWER) ?
                            "answer" :
                            ((entry == MDNS_ENTRYTYPE_AUTHORITY) ? "authority" : "additional");
    mdns_string_t entrystr =
            mdns_string_extract(data, size, &name_offset, entrybuffer, sizeof(entrybuffer));
    if (rtype == MDNS_RECORDTYPE_PTR)
    {
        mdns_string_t namestr = mdns_record_parse_ptr(data, size, record_offset, record_length,
                                                      namebuffer, sizeof(namebuffer));
        printf("*******%.*s : %s %.*s PTR %.*s rclass 0x%x ttl %u length %d\n",
               MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
               MDNS_STRING_FORMAT(namestr), rclass, ttl, (int)record_length);
        if(strcmp(entrystr.str, SITE_QUERY_SERVICE_NAME) == 0)
        {
            struct sockaddr_in *tmp = (struct sockaddr_in*)from;
            if(tmp->sin_addr.s_addr != service_address_ipv4)
            {
                std::string str(fromaddrstr.str);
                mdns_answer_querysite_set.insert(str);
            }
        }
    }
//    else if (rtype == MDNS_RECORDTYPE_SRV)
//    {
//        mdns_record_srv_t srv = mdns_record_parse_srv(data, size, record_offset, record_length,
//                                                      namebuffer, sizeof(namebuffer));
//        printf("%.*s : %s %.*s SRV %.*s priority %d weight %d port %d\n",
//               MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
//               MDNS_STRING_FORMAT(srv.name), srv.priority, srv.weight, srv.port);
//    }
//    else if (rtype == MDNS_RECORDTYPE_A)
//    {
//        struct sockaddr_in addr;
//        mdns_record_parse_a(data, size, record_offset, record_length, &addr);
//        mdns_string_t addrstr =
//                ipv4_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
//        printf("%.*s : %s %.*s A %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
//               MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
//    }
//    else if (rtype == MDNS_RECORDTYPE_AAAA)
//    {
//        struct sockaddr_in6 addr;
//        mdns_record_parse_aaaa(data, size, record_offset, record_length, &addr);
//        mdns_string_t addrstr =
//                ipv6_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
//        printf("%.*s : %s %.*s AAAA %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
//               MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
//    }
//    else if (rtype == MDNS_RECORDTYPE_TXT)
//    {
//        size_t parsed = mdns_record_parse_txt(data, size, record_offset, record_length, txtbuffer,
//                                              sizeof(txtbuffer) / sizeof(mdns_record_txt_t));
//        for (size_t itxt = 0; itxt < parsed; ++itxt)
//        {
//            if (txtbuffer[itxt].value.length)
//                printf("%.*s : %s %.*s TXT %.*s = %.*s\n", MDNS_STRING_FORMAT(fromaddrstr),
//                       entrytype, MDNS_STRING_FORMAT(entrystr),
//                       MDNS_STRING_FORMAT(txtbuffer[itxt].key),
//                       MDNS_STRING_FORMAT(txtbuffer[itxt].value));
//            else
//                printf("%.*s : %s %.*s TXT %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
//                       MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(txtbuffer[itxt].key));
//        }
//    }
//    else
//        printf("%.*s : %s %.*s type %u rclass 0x%x ttl %u length %d\n",
//               MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr), rtype,
//               rclass, ttl, (int)record_length);
    return 0;
}

int service_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
                     uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                     size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                     size_t record_length, void* user_data) {
    (void)sizeof(name_offset);
    (void)sizeof(name_length);
    (void)sizeof(ttl);
    if (entry != MDNS_ENTRYTYPE_QUESTION)
        return 0;
    mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);
    if (rtype == MDNS_RECORDTYPE_PTR)
    {
        memset(namebuffer, 0x00, sizeof(namebuffer));
        mdns_string_t service = mdns_record_parse_ptr(data, size, record_offset, record_length,
                                                      namebuffer, sizeof(namebuffer));
        printf("%.*s : question PTR %.*s\n", MDNS_STRING_FORMAT(fromaddrstr),
               MDNS_STRING_FORMAT(service));

//        const char dns_sd[] = "_services._dns-sd._udp.local.";
//        const service_record_t* service_record = (const service_record_t*)user_data;
//        size_t service_length = strlen(service_record->service);
//        if ((service.length == (sizeof(dns_sd) - 1)) &&
//            (strncmp(service.str, dns_sd, sizeof(dns_sd) - 1) == 0))
//        {
//            printf("  --> answer %s\n", service_record->service);
//            mdns_discovery_answer(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
//                                  service_record->service, service_length);
//        }
//        else if ((service.length == service_length) &&
//                   (strncmp(service.str, service_record->service, service_length) == 0))

//add by wjg 2022-05-10
        get_myself_ipv4addr(from);
        service_record_t return_service_record;
        return_service_record.service = service.str;
        return_service_record.hostname = global_hostname;
        return_service_record.address_ipv6 = 0;
        std::string query_service_name = service.str;

#if 1   //for release
        if(query_service_name == SITE_QUERY_SERVICE_NAME)
        {
            return_service_record.address_ipv4 = has_ipv4 ? service_address_ipv4 : 0;
            return_service_record.port = SITE_QUERY_SERVICE_PORT;
        }
        else
        {
             std::vector<std::string> v_str;
            std::string site_name;
            StringUtils::splitString(query_service_name, ".", v_str);
            if(v_str.size() > 3)
            {
                site_name = v_str[1];
                StringUtils::replace(site_name, "-", "_");
            }
            if(site_name.empty())
                return 0;

            SiteTree &stree = SiteTree::GetInstance();
            if(!stree.hasSite(site_name))
                return 0;

            std::string site_ip = stree.getSiteInfo(site_name, SITE_IP);
            std::string site_port = stree.getSiteInfo(site_name, SITE_PORT);
            if(site_ip.empty() || site_port.empty())
                return 0;

            uint32_t siteip_int = inet_addr(site_ip.c_str());
            if(siteip_int == ntohl(INADDR_LOOPBACK))
                return_service_record.address_ipv4 = has_ipv4 ? service_address_ipv4 : 0;
            else
                return_service_record.address_ipv4 = has_ipv4 ? siteip_int : 0;

            return_service_record.port = atoi(site_port.c_str());
        }

#else   //for test
        return_service_record.address_ipv4 = has_ipv4 ? service_address_ipv4 : 0;
        if(query_service_name == SITE_QUERY_SERVICE_NAME)
            return_service_record.port = SITE_QUERY_SERVICE_PORT;
        else if(query_service_name == BLE_LIGHT_SERVICE_NAME)
            return_service_record.port = BLE_LIGHT_SERVICE_PORT;
        else if(query_service_name == ZIGBEE_LIGHT_SERVICE_NAME)
            return_service_record.port = ZIGBEE_LIGHT_SERVICE_PORT;
        else if(query_service_name == RADAR_SERVICE_NAME)
            return_service_record.port = RADAR_SERVICE_PORT;
        else if(query_service_name == VOICE_SERVICE_NAME)
            return_service_record.port = VOICE_SERVICE_PORT;
        else if(query_service_name == SOUND_SERVICE_NAME)
            return_service_record.port = SOUND_SERVICE_PORT;
        else if(query_service_name == CONFIG_SERVICE_NAME)
            return_service_record.port = CONFIG_SERVICE_PORT;
        else if(query_service_name == COLLABORATE_SERVICE_NAME)
            return_service_record.port = COLLABORATE_SERVICE_PORT;
        else if(query_service_name == TV_ADAPTER_SERVICE_NAME)
            return_service_record.port = TV_ADAPTER_SERVICE_PORT;
        else if(query_service_name == LIGHT_MOVE_SERVICE_NAME)
            return_service_record.port = LIGHT_MOVE_SERVICE_PORT;
        else
            return 0;
#endif
        const service_record_t* service_record = &return_service_record;
        size_t service_length = strlen(service_record->service);
//end add by wjg 2022-05-10

        {
            uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
            printf("  --> answer %s.%s port %d (%s)\n", service_record->hostname,
                   service_record->service, service_record->port,
                   (unicast ? "unicast" : "multicast"));

            printf("unicast: %d---%d\n", unicast, addrlen);
            mdns_query_answer(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id,
                              service_record->service, service_length, service_record->hostname,
                              strlen(service_record->hostname), service_record->address_ipv4,
                              service_record->address_ipv6, (uint16_t)service_record->port, 0,0);

            if (!unicast)
                addrlen = 0;
//            char txt_record[] = "test=1";
            printf("multicast: %d---%d\n", unicast, addrlen);
            mdns_query_answer(sock, from, addrlen, sendbuffer, sizeof(sendbuffer), query_id,
                              service_record->service, service_length, service_record->hostname,
                              strlen(service_record->hostname), service_record->address_ipv4,
                              service_record->address_ipv6, (uint16_t)service_record->port, 0,0);
                              //txt_record, sizeof(txt_record));
        }
    }
//    else if (rtype == MDNS_RECORDTYPE_SRV)
//    {
//        mdns_record_srv_t service = mdns_record_parse_srv(data, size, record_offset, record_length,
//                                                          namebuffer, sizeof(namebuffer));
//        printf("%.*s : question SRV %.*s\n", MDNS_STRING_FORMAT(fromaddrstr),
//               MDNS_STRING_FORMAT(service.name));
//    }
    return 0;
}

int open_client_sockets(int* sockets, int max_sockets, int port) {
    // When sending, each socket can only send to one network interface
    // Thus we need to open one socket for each interface and address family
    int num_sockets = 0;

#ifdef _WIN32

    IP_ADAPTER_ADDRESSES* adapter_address = 0;
	ULONG address_size = 8000;
	unsigned int ret;
	unsigned int num_retries = 4;
	do {
		adapter_address = malloc(address_size);
		ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0,
		                           adapter_address, &address_size);
		if (ret == ERROR_BUFFER_OVERFLOW) {
			free(adapter_address);
			adapter_address = 0;
		} else {
			break;
		}
	} while (num_retries-- > 0);

	if (!adapter_address || (ret != NO_ERROR)) {
		free(adapter_address);
		printf("Failed to get network adapter addresses\n");
		return num_sockets;
	}

	int first_ipv4 = 1;
	int first_ipv6 = 1;
	for (PIP_ADAPTER_ADDRESSES adapter = adapter_address; adapter; adapter = adapter->Next) {
		if (adapter->TunnelType == TUNNEL_TYPE_TEREDO)
			continue;
		if (adapter->OperStatus != IfOperStatusUp)
			continue;

		for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
		     unicast = unicast->Next) {
			if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
				struct sockaddr_in* saddr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
				if ((saddr->sin_addr.S_un.S_un_b.s_b1 != 127) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b2 != 0) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b3 != 0) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b4 != 1)) {
					int log_addr = 0;
					if (first_ipv4) {
						service_address_ipv4 = saddr->sin_addr.S_un.S_addr;
						first_ipv4 = 0;
						log_addr = 1;
					}
					has_ipv4 = 1;
					if (num_sockets < max_sockets) {
						saddr->sin_port = htons((unsigned short)port);
						int sock = mdns_socket_open_ipv4(saddr);
						if (sock >= 0) {
							sockets[num_sockets++] = sock;
							log_addr = 1;
						} else {
							log_addr = 0;
						}
					}
					if (log_addr) {
						char buffer[128];
						mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
						                                            sizeof(struct sockaddr_in));
						printf("Local IPv4 address: %.*s\n", MDNS_STRING_FORMAT(addr));
					}
				}
			} else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
				struct sockaddr_in6* saddr = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
				static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0,
				                                          0, 0, 0, 0, 0, 0, 0, 1};
				static const unsigned char localhost_mapped[] = {0, 0, 0,    0,    0,    0, 0, 0,
				                                                 0, 0, 0xff, 0xff, 0x7f, 0, 0, 1};
				if ((unicast->DadState == NldsPreferred) &&
				    memcmp(saddr->sin6_addr.s6_addr, localhost, 16) &&
				    memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16)) {
					int log_addr = 0;
					if (first_ipv6) {
						memcpy(service_address_ipv6, &saddr->sin6_addr, 16);
						first_ipv6 = 0;
						log_addr = 1;
					}
					has_ipv6 = 1;
					if (num_sockets < max_sockets) {
						saddr->sin6_port = htons((unsigned short)port);
						int sock = mdns_socket_open_ipv6(saddr);
						if (sock >= 0) {
							sockets[num_sockets++] = sock;
							log_addr = 1;
						} else {
							log_addr = 0;
						}
					}
					if (log_addr) {
						char buffer[128];
						mdns_string_t addr = ipv6_address_to_string(buffer, sizeof(buffer), saddr,
						                                            sizeof(struct sockaddr_in6));
						printf("Local IPv6 address: %.*s\n", MDNS_STRING_FORMAT(addr));
					}
				}
			}
		}
	}

	free(adapter_address);

#else

    struct ifaddrs* ifaddr = 0;
    struct ifaddrs* ifa = 0;

    if (getifaddrs(&ifaddr) < 0)
        printf("Unable to get interface addresses\n");

    int first_ipv4 = 1;
    int first_ipv6 = 1;
    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
            if (saddr->sin_addr.s_addr != htonl(INADDR_LOOPBACK))
            {
                int log_addr = 0;
                if (first_ipv4)
                {
                    service_address_ipv4 = saddr->sin_addr.s_addr;
                    first_ipv4 = 0;
                    log_addr = 1;
                }
                has_ipv4 = 1;
                if (num_sockets < max_sockets)
                {
                    saddr->sin_port = htons(port);
                    int sock = mdns_socket_open_ipv4(saddr);
                    if (sock >= 0)
                    {
                        sockets[num_sockets++] = sock;
                        log_addr = 1;
                    }
                    else
                        log_addr = 0;
                }
                if (log_addr)
                {
                    char buffer[128];
                    mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
                                                                sizeof(struct sockaddr_in));
                    printf("Local IPv4 address: %.*s\n", MDNS_STRING_FORMAT(addr));
                }
            }
        }

#ifdef _NEDD_USE_IPV6_
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6* saddr = (struct sockaddr_in6*)ifa->ifa_addr;
            static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0,
                                                      0, 0, 0, 0, 0, 0, 0, 1};
            static const unsigned char localhost_mapped[] = {0, 0, 0,    0,    0,    0, 0, 0,
                                                             0, 0, 0xff, 0xff, 0x7f, 0, 0, 1};
            if (memcmp(saddr->sin6_addr.s6_addr, localhost, 16) &&
                memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16))
            {
                int log_addr = 0;
                if (first_ipv6)
                {
                    memcpy(service_address_ipv6, &saddr->sin6_addr, 16);
                    first_ipv6 = 0;
                    log_addr = 1;
                }
                has_ipv6 = 1;
                if (num_sockets < max_sockets)
                {
                    saddr->sin6_port = htons(port);
                    int sock = mdns_socket_open_ipv6(saddr);
                    if (sock >= 0)
                    {
                        sockets[num_sockets++] = sock;
                        log_addr = 1;
                    }
                    else
                        log_addr = 0;
                }
                if (log_addr)
                {
                    char buffer[128];
                    mdns_string_t addr = ipv6_address_to_string(buffer, sizeof(buffer), saddr,
                                                                sizeof(struct sockaddr_in6));
                    printf("Local IPv6 address: %.*s\n", MDNS_STRING_FORMAT(addr));
                }
            }
        }
#endif

    }

    freeifaddrs(ifaddr);

#endif

    return num_sockets;
}

int open_service_sockets(int* sockets, int max_sockets) {
    // When recieving, each socket can recieve data from all network interfaces
    // Thus we only need to open one socket for each address family
    int num_sockets = 0;

    // Call the client socket function to enumerate and get local addresses,
    // but not open the actual sockets
//    open_client_sockets(0, 0, 0);

    if (num_sockets < max_sockets) {
        struct sockaddr_in sock_addr;
        memset(&sock_addr, 0, sizeof(struct sockaddr_in));
        sock_addr.sin_family = AF_INET;
#ifdef _WIN32
        sock_addr.sin_addr = in4addr_any;
#else
        sock_addr.sin_addr.s_addr = INADDR_ANY;
#endif
        sock_addr.sin_port = htons(MDNS_PORT);
#ifdef __APPLE__
        sock_addr.sin_len = sizeof(struct sockaddr_in);
#endif
        int sock = mdns_socket_open_ipv4(&sock_addr);
        if (sock >= 0)
            sockets[num_sockets++] = sock;
    }

#ifdef _NEDD_USE_IPV6_
    if (num_sockets < max_sockets) {
        struct sockaddr_in6 sock_addr;
        memset(&sock_addr, 0, sizeof(struct sockaddr_in6));
        sock_addr.sin6_family = AF_INET6;
        sock_addr.sin6_addr = in6addr_any;
        sock_addr.sin6_port = htons(MDNS_PORT);
#ifdef __APPLE__
        sock_addr.sin6_len = sizeof(struct sockaddr_in6);
#endif
        int sock = mdns_socket_open_ipv6(&sock_addr);
        if (sock >= 0)
            sockets[num_sockets++] = sock;
    }
#endif

    return num_sockets;
}

int send_mdns_query(const char* service) {
    int sockets[8];
    int query_id[8];
    mdns_answer_querysite_set.clear();
    int num_sockets = open_client_sockets(sockets, sizeof(sockets) / sizeof(sockets[0]), 0);
    if (num_sockets <= 0) {
        printf("*******Failed to open any client sockets\n");
        return -1;
    }
    printf("*******Opened %d socket%s for mDNS query\n", num_sockets, num_sockets ? "s" : "");

    size_t capacity = 2048;
    void* buffer = malloc(capacity);
    void* user_data = 0;
    size_t records;

    printf("*******Sending mDNS query: %s\n", service);
    for (int isock = 0; isock < num_sockets; ++isock) {
        query_id[isock] = mdns_query_send(sockets[isock], MDNS_RECORDTYPE_PTR, service,
                                          strlen(service), buffer, capacity, 0);
        if (query_id[isock] < 0)
            printf("*******Failed to send mDNS query: %s\n", strerror(errno));
    }

    // This is a simple implementation that loops for 5 seconds or as long as we get replies
    int res;
    printf("*******Reading mDNS query replies\n");
    do {
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        int nfds = 0;
        fd_set readfs;
        FD_ZERO(&readfs);
        for (int isock = 0; isock < num_sockets; ++isock) {
            if (sockets[isock] >= nfds)
                nfds = sockets[isock] + 1;
            FD_SET(sockets[isock], &readfs);
        }

        records = 0;
        res = select(nfds, &readfs, 0, 0, &timeout);
        if (res > 0) {
            for (int isock = 0; isock < num_sockets; ++isock) {
                if (FD_ISSET(sockets[isock], &readfs)) {
                    records += mdns_query_recv(sockets[isock], buffer, capacity, query_callback,
                                               user_data, query_id[isock]);
                }
                FD_SET(sockets[isock], &readfs);
            }
        }
    } while (res > 0);

    free(buffer);

    for (int isock = 0; isock < num_sockets; ++isock)
        mdns_socket_close(sockets[isock]);
    printf("Closed socket%s\n", num_sockets ? "s" : "");

    return 0;
}

int start_service_mdns(){
    std::thread mdns_service(service_mdns);
    mdns_service.detach();

    return 0;
}

int service_mdns() {
    int sockets[8];
    int num_sockets = open_service_sockets(sockets, sizeof(sockets) / sizeof(sockets[0]));
    if (num_sockets <= 0)
    {
        printf("Failed to open any client sockets\n");
        return -1;
    }
    printf("Opened %d socket%s for mDNS service\n", num_sockets, num_sockets ? "s" : "");

    printf("Service mDNS: %s:%d\n", global_service_name, global_service_port);
    printf("Hostname: %s\n", global_hostname);

    size_t capacity = 2048;
    void* buffer = malloc(capacity);

    service_record_t service_record;
    service_record.service = global_service_name;
    service_record.hostname = global_hostname;
    service_record.address_ipv4 = has_ipv4 ? service_address_ipv4 : 0;
    service_record.address_ipv6 = has_ipv6 ? service_address_ipv6 : 0;
    service_record.port = global_service_port;

    // This is a crude implementation that checks for incoming queries
    while(1)
    {
        int nfds = 0;
        fd_set readfs;
        FD_ZERO(&readfs);
        for(int isock = 0; isock < num_sockets; ++isock)
        {
            if (sockets[isock] >= nfds)
                nfds = sockets[isock] + 1;
            FD_SET(sockets[isock], &readfs);
        }

        if(select(nfds, &readfs, 0, 0, 0) >= 0)
        {
            for(int isock = 0; isock < num_sockets; ++isock)
            {
                if (FD_ISSET(sockets[isock], &readfs))
                    mdns_socket_listen(sockets[isock], buffer, capacity, service_callback, &service_record);
                FD_SET(sockets[isock], &readfs);
            }
        }
        else
            break;
    }

    free(buffer);

    for(int isock = 0; isock < num_sockets; ++isock)
        mdns_socket_close(sockets[isock]);
    printf("Closed socket%s\n", num_sockets ? "s" : "");

    return 0;
}

int get_mdns_query_result(std::unordered_set<std::string> &site_set){
    int ret = mdns_answer_querysite_set.size();

    if(ret > 0)
        site_set = mdns_answer_querysite_set;

    return ret;
}