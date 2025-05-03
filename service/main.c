#include <stdlib.h>
#include <stdio.h>
#include <wiringPi.h>

int main() {
    printf("Starting gokart deamon.");

    // Setup GPIO
    // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
    wiringPiSetupGpio();

    

    // Start ffmpeg
    printf("Starting ffmpeg live video feed.");
    system("ffmpeg -f v4l2 -i /dev/video0 -vcodec libx264 -f flv rtmp://gokartrtmp.sheepland.xyz/live/01");
    return 0;
}