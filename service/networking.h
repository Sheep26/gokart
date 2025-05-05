#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

class Networking {
public:
    static void wait_for_network();
    static bool check_network();
    static bool wifi_enabled();
    static bool set_wifi(bool enabled);
};

#endif