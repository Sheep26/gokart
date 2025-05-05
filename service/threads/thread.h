#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <chrono>
#include <curl/curl.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <fmt/core.h>
#include ".networking.h"

class Threads {
public:
    static void data_t();
    static void ffmpeg_t();
    static void display_t();
};