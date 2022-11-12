//
// Created by WJG on 2022-5-10.
//

#ifndef SITEMANAGERSERVICE_MDNS_INTERFACE_H
#define SITEMANAGERSERVICE_MDNS_INTERFACE_H

#include <string>
#include <unordered_set>

#define MY_HOST_NAME "changhong"
#define ALL_SITE_SERVICE_NAME "_edgeai.all._tcp.local."
#define SITE_QUERY_SERVICE_NAME "_edgeai.site-query._tcp.local."
#define SITE_QUERY_SERVICE_PORT 9000

#define BLE_LIGHT_SERVICE_NAME "_edgeai.ble-light._tcp.local."
#define BLE_LIGHT_SERVICE_PORT 9001

#define ZIGBEE_LIGHT_SERVICE_NAME "_edgeai.zigbee-light._tcp.local."
#define ZIGBEE_LIGHT_SERVICE_PORT 9002

#define RADAR_SERVICE_NAME "_edgeai.radar._tcp.local."
#define RADAR_SERVICE_PORT 9003

#define VOICE_SERVICE_NAME "_edgeai.voice._tcp.local."
#define VOICE_SERVICE_PORT 9004

#define SOUND_SERVICE_NAME "_edgeai.sound._tcp.local."
#define SOUND_SERVICE_PORT 9005

#define CONFIG_SERVICE_NAME "_edgeai.config._tcp.local."
#define CONFIG_SERVICE_PORT 9006

#define COLLABORATE_SERVICE_NAME "_edgeai.collaborate._tcp.local."
#define COLLABORATE_SERVICE_PORT 9007

#define TV_ADAPTER_SERVICE_NAME "_edgeai.tv-adapter._tcp.local."
#define TV_ADAPTER_SERVICE_PORT 9008

#define LIGHT_MOVE_SERVICE_NAME "_edgeai.light-move._tcp.local."
#define LIGHT_MOVE_SERVICE_PORT 9009


int start_service_mdns();
int service_mdns();

int service_mdns_control();

int send_mdns_query(const char* service);
int get_mdns_query_result(std::unordered_set<std::string> &site_set);

#endif //SITEMANAGERSERVICE_MDNS_INTERFACE_H
