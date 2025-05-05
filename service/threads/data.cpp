#include "thread.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

void Threads::data_t() {
    // Send data to server every 100ms
    while (true) {
        if (!Networking::check_network()) {
            return;
        }

        CURL* curl = curl_easy_init();

        if (curl) {
            string body = fmt::format(R"({
                "speed": {},
                "speed_avg": {},
                "speed_max": {},
                "rpm": {},
                "rpm_avg": {},
                "rpm_max": {},
                "power": {},
                "power_avg": {},
                "power_max": {},
                "throttle": {},
                "throttle_avg": {},
                "throttle_max": {}
            })", data.speed.current, data.speed.avg, data.speed.max, data.rpm.current, data.rpm.avg, data.rpm.max, data.power.current, data.power.avg, data.power.max, data.throttle.current, data.throttle.avg, data.throttle.max);
            const char* body_cstr = body.c_str();

            curl_easy_setopt(curl, CURLOPT_URL, "https://gokart.sheepland.xyz/api/update_data");
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_cstr);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(body_cstr));
    
            CURLcode res = curl_easy_perform(curl);

            // Clean up memory.
            curl_easy_cleanup(curl);
    
            if (res == CURLE_OK) {
                std::cout << "Response:\n" << std::endl;
            } else {
                std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
            }
        }

        sleep_for(milliseconds(100));
    }
}