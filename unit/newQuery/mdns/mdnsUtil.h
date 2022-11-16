//
// Created by 78472 on 2022/11/16.
//

#ifndef EXHIBITION_MDNSUTIL_H
#define EXHIBITION_MDNSUTIL_H

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stdio.h>
#include <errno.h>
#include <signal.h>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif

// Alias some things to simulate recieving data to fuzz library
#if defined(MDNS_FUZZING)
#define recvfrom(sock, buffer, capacity, flags, src_addr, addrlen) ((mdns_ssize_t)capacity)
#define printf
#endif

#include "mdns.h"

#if defined(MDNS_FUZZING)
#undef recvfrom
#endif



// Data for our service including the mDNS records
typedef struct {
    mdns_string_t service;
    mdns_string_t hostname;
    mdns_string_t service_instance;
    mdns_string_t hostname_qualified;
    struct sockaddr_in address_ipv4;
    struct sockaddr_in6 address_ipv6;
    int port;
    mdns_record_t record_ptr;
    mdns_record_t record_srv;
    mdns_record_t record_a;
    mdns_record_t record_aaaa;
    mdns_record_t txt_record[2];
} service_t;

mdns_string_t
ipv4_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in* addr,
                       size_t addrlen);

mdns_string_t
ipv6_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in6* addr,
                       size_t addrlen);

mdns_string_t
ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen);

// Callback handling parsing answers to queries sent
int
query_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
               uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
               size_t size, size_t name_offset, size_t name_length, size_t record_offset,
               size_t record_length, void* user_data);

// Callback handling questions incoming on service sockets
int
service_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
                 uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                 size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                 size_t record_length, void* user_data);

// Callback handling questions and answers dump
int
dump_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
              uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
              size_t size, size_t name_offset, size_t name_length, size_t record_offset,
              size_t record_length, void* user_data);

// Open sockets for sending one-shot multicast queries from an ephemeral port
int
open_client_sockets(int* sockets, int max_sockets, int port);

// Open sockets to listen to incoming mDNS queries on port 5353
int
open_service_sockets(int* sockets, int max_sockets);

// Send a DNS-SD query
int send_dns_sd(void);

// Send a mDNS query
int send_mdns_query(mdns_query_t* query, size_t count);

// Provide a mDNS service, answering incoming DNS-SD and mDNS queries
int service_mdns(const char* hostname, const char* service_name, int service_port);


// Dump all incoming mDNS queries and answers
int dump_mdns(void);

#ifdef MDNS_FUZZING
#undef printf
// Fuzzing by piping random data into the recieve functions
void fuzz_mdns(void);
#endif

#ifdef _WIN32
BOOL console_handler(DWORD signal);
#else
void signal_handler(int signal);
#endif



#endif //EXHIBITION_MDNSUTIL_H
