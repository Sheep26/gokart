#ifndef NETWORKING_H
#define NETWORKING_H

#include <string>
#include <iostream>
#include <cstdio>
#include <chrono>
#include <thread>

extern class Networking {
public:
    static void wait_for_network();  // Declaration of static method
    static bool check_network();     // Declaration of static method
};

#endif // NETWORKING_H