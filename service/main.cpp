#include <iostream>
#include <wiringPi.h>
#include <data.hpp>
#include <cstdlib>
#include <thread>

using namespace std;

int main() {
    cout << "Starting gokart deamon.";

    // Setup GPIO
    // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
    wiringPiSetupGpio();

    // Create a thread for ffmpeg video feed.
    thread ffmpeg_t(ffmpeg_setup);
    ffmpeg_t.join();

    // Return 0
    return 0;
}

void ffmpeg_setup() {
    // Start ffmpeg
    cout << "Starting ffmpeg live video feed.";

    int ret = system("ffmpeg -f v4l2 -i /dev/video0 -vcodec libx264 -f flv rtmp://gokart.sheepland.xyz/apt/videofeed");
    if (ret != 0) {
        cerr << "Error: ffmpeg command failed with exit code " << ret << endl;
    }
}