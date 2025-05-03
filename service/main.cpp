#include <iostream>
#include <wiringPi.h>
#include <data.hpp>
#include <cstdlib>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void data_thread() {

}

void ffmpeg_thread() {
    // Start ffmpeg.
    cout << "Starting ffmpeg live video feed.";

    int ret = system("ffmpeg -f v4l2 -i /dev/video0 -f flv rtmp://gokart.sheepland.xyz/live/stream");
    if (ret != 0) {
        cerr << "Error: ffmpeg command failed with exit code " << ret << endl;
    }
}

int main() {
    cout << "Starting gokart deamon.";

    // Setup GPIO
    // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
    if (wiringPiSetupGpio() == -1) {
        cerr << "Error: Failed to initialize GPIO." << endl;
        return 1;
    }

    // Create a thread for ffmpeg video feed.
    thread ffmpeg_t(ffmpeg_thread);
    ffmpeg_t.join();

    // Create a thread to upload the data to the server.
    thread data_t(data_thread);
    data_t.join();

    // Main loop.
    while (true) {
        sleep_for(milliseconds(33));
        break;
    }

    // Return 0.
    return 0;
}