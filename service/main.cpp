#include <iostream>
#include <wiringPi.h>
#include <thread>
#include <chrono>
#include "data.h"
#include "networking.h"
#include "threads/thread.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

bool networkUtilsRunning = false;

int main() {
    cout << "Starting gokart service." << endl;

    // Setup GPIO
    // Uses BCM numbering of the GPIOs and directly accesses the GPIO registers.
    cout << "Initalizing GPIO" << endl;
    if (wiringPiSetupGpio() == -1) {
        cerr << "Error: Failed to initialize GPIO." << endl;
        return 1;
    }

    // Create display thread.
    thread display_t(Threads::display_t);
    display_t.detach();

    // I don't care that it's bad code right now.
    if (!Networking::check_network()) {
        cout << "Waiting for network." << endl;
        Networking::wait_for_network();
    }

    // Main loop.
    while (true) {
        if (!networkUtilsRunning && Networking::check_network()) {
            networkUtilsRunning = true;
            // Create threads
            thread ffmpeg_t(Threads::ffmpeg_t);
            thread data_t(Threads::data_t);

            // Detach threads to allow independent execution
            ffmpeg_t.detach();
            data_t.detach();
        }

        sleep_for(milliseconds(100));
    }

    // Return 0.
    return 0;
}