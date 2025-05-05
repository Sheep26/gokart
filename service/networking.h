#include <string>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

extern class Networking {
public:
    static void wait_for_network();
    static bool check_network();
};