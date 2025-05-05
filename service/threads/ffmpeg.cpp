#include "thread.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void Threads::ffmpeg_thread() {
    cout << "Starting ffmpeg live video feed.";

    // Check if ffmpeg installed.
    if (system("which ffmpeg > /dev/null 2>&1") != 0) {
        cerr << "Error: ffmpeg is not installed on the system, exiting." << endl;
        return;
    }

    // Start ffmpeg.
    int ret = system("ffmpeg -f v4l2 -i /dev/video0 -f flv rtmp://gokart.sheepland.xyz/live/stream");
    if (ret != 0) {
        cerr << "Error: ffmpeg command failed with exit code " << ret << endl;
    }
}