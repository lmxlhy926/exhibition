//
// Created by 78472 on 2022/10/22.
//

#include "sslUtil.h"


sslUtil* sslUtil::instance = nullptr;

sslUtil::SingleSite::SingleSite(string host, string ca_cert_path) {
    host_ = std::move(host);
    ca_cert_path_ = std::move(ca_cert_path);
}

bool sslUtil::SingleSite::send(string &uri, qlibc::QData& request, qlibc::QData& response) {
    if(clientptr == nullptr){
        clientptr = new httplib::SSLClient(host_);
        clientptr->set_ca_cert_path(ca_cert_path_.c_str());
        clientptr->enable_server_certificate_verification(true);
        clientptr->set_keep_alive(true);
        clientptr->set_connection_timeout(1, 0);
    }

    if(!clientptr->is_socket_open()){
        clientptr->stop();
        delete clientptr;

        clientptr = new httplib::SSLClient(host_);
        clientptr->set_ca_cert_path(ca_cert_path_.c_str());
        clientptr->enable_server_certificate_verification(true);
        clientptr->set_keep_alive(true);
        clientptr->set_connection_timeout(1, 0);
    }

    httplib::Params  params;
    qlibc::QData param = request.getData("param");
    for(const auto& key : param.getMemberNames()){
        params.insert(std::pair<string, string>(key, param.getString(key)));
    }

    httplib::Result result =  clientptr->Post(uri.c_str(), params);
    if(result != nullptr){
        response.setInitData(qlibc::QData(result.value().body));
        return true;
    }
    return false;
}

void sslUtil::SingleSite::deleteClient() {
    if(clientptr != nullptr){
        clientptr->stop();
        delete clientptr;
        clientptr = nullptr;
    }
}

sslUtil *sslUtil::getInstance() {
    if(instance == nullptr){
        instance = new sslUtil();
    }
    return instance;
}

void sslUtil::addCloudSite(string cloudService, string& host, string& ca_cert_path) {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    auto pos = sites.find(cloudService);
    if(pos != sites.end()){
        pos->second.deleteClient();
        sites.erase(cloudService);
    }
    sites.emplace(cloudService, SingleSite(host, ca_cert_path));
}

bool sslUtil::sendRequest2Site(const string& cloudService, string uri, qlibc::QData& request, qlibc::QData& response) {
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    auto pos = sites.find(cloudService);
    if(pos != sites.end()){
        return pos->second.send(uri, request, response);
    }
    return false;
}
