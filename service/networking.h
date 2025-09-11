#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>
#include <cstring>
#include <curl/curl.h>

struct HTTP_Request {
    std::string text;
    long status_code;
};

class Networking {
public:
    static int wait_for_network();
    static bool check_network();
    static bool wifi_enabled();
    static void set_wifi(bool enabled);
    static void scan_wifi();
    static bool connect_wifi(std::string ssid, std::string passwd);
    static std::string list_networks();
    static HTTP_Request send_http_request(const std::string& url, const std::string& body, const bool is_post, const struct curl_slist* headers);
};

#endif