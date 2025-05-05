#include "thread.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void display_thread() {
    while (true) {
        sleep_for(milliseconds(33));
    }
}