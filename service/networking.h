#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

using namespace std;

class Networking {
public:
    static int wait_for_network();
    static bool check_network();
    static bool wifi_enabled();
    static void set_wifi(bool enabled);
    static void scan_wifi();
    static void connect_wifi(string ssid, string passwd);
    static bool send_http_request(const string& url, const string& body, const CURLoption method, const struct curl_slist* header);
};

#endif