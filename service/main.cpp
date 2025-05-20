#include <iostream>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <thread>
#include <chrono>
#include <cstring>
#include <alsa/asoundlib.h> // Include ALSA library
#include <cstdlib>

#include "data.h"
#include "networking.h"
#include "threads.h"
#include "ssd1309.h"
#include "OledScreen.h"

using namespace std;
using namespace std::this_thread;
using namespace std::chrono;

#define TELEMENTRY_PIN 10
#define RADIO_BUTTON 11
#define RADIO_SWITCH 12
#define DC 5
#define RST 6

bool telementry_running = false;

// WAV file header structure
struct WAVHeader {
    char riff[4];            // "RIFF"
    uint32_t chunk_size;     // File size - 8 bytes
    char wave[4];            // "WAVE"
    char fmt[4];             // "fmt "
    uint32_t subchunk1_size; // Size of the fmt chunk (16 for PCM)
    uint16_t audio_format;   // Audio format (1 for PCM)
    uint16_t num_channels;   // Number of channels
    uint32_t sample_rate;    // Sample rate
    uint32_t byte_rate;      // Byte rate = sample_rate * num_channels * bytes_per_sample
    uint16_t block_align;    // Block align = num_channels * bytes_per_sample
    uint16_t bits_per_sample;// Bits per sample
    char data[4];            // "data"
    uint32_t subchunk2_size; // Size of the data chunk
};

struct Server { // I don't want to have to deal with memory realloc, lets use strings.
    string ip;
    string username;
    string passwd;
    string session;
    string id;
};

/*
 https://www.hpinfotech.ro/SSD1309.pdf - Datasheet
 Display pinout.
 GND    --- Gnd
 VCC    --- 3.3V
 DO     --- SCLK(Pin#23)
 DI     --- MOSI(Pin#19)
 RES/RST    --- GPIO18(Pin#6) (You can use Any Pin)
 DC     --- GPIO17(Pin#5) (You can use Any Pin)
 CS     --- CS0(Pin#24)
*/

Server server;

void Threads::data_t() {
    // Send data to server every 100ms
    try {
        while (true) {
            if (!Networking::check_network() || !telementry_running) {
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
    
                curl_easy_setopt(curl, CURLOPT_URL, ("https://" + SERVERIP + "/api/update_data").c_str());
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
    catch (...) {
        telementry_running = false;
        return;
    }
}

void Threads::ffmpeg_t() {
    cout << "Starting ffmpeg live video feed.";

    // Start ffmpeg.
    string cmd = "ffmpeg -f v4l2 -i /dev/video0 -f flv rtmp://" + server.ip +
                  "/live/stream?id=" + server.id + "&session=" + server.session;
    int ret = system(cmd.c_str());

    if (ret != 0) {
        cerr << "Error: ffmpeg command failed with exit code " << ret << endl;
    }
    
    telementry_running = false;
}

void Threads::display_t() {
    OledScreen oled;

    unsigned char initcode[] = {
        DISPLAY_OFF,                     // 0xAE: Turn display off
        SET_DISPLAY_CLOCK_DEVICE_RATIO,  0x80, // 0xD5: Set clock divide ratio/oscillator frequency
        SET_MULTIPLEX_RATIO,             0x3F, // 0xA8: Set multiplex ratio (1/64 duty cycle)
        SET_DISPLAY_OFFSET,              0x00, // 0xD3: Set display offset to 0
        SET_DISPLAY_START_LINE | 0x00,         // 0x40: Set display start line to 0
        SET_SEGMENT_REMAP_ON,                  // 0xA1: Set segment re-map (column address 127 is mapped to SEG0)
        SET_COM_OUTPUT_SCAN_DIRECTION_8,       // 0xC8: Set COM output scan direction (remapped mode)
        SET_COM_PINS_HARDWARE_CONFIG,    0x12, // 0xDA: Set COM pins hardware configuration
        SET_CONTRAST,                   0x7F,  // 0x81: Set contrast control to medium
        SET_PRECHANGE_PERIOD,           0xF1,  // 0xD9: Set pre-charge period
        SET_VCOMH_DESELECT_LEVEL,       0x40,  // 0xDB: Set VCOMH deselect level
        SET_MEMORY_ADDRESS_MODE,        0x00,  // 0x20: Set memory addressing mode to horizontal
        SET_LOW_COLUMN,                       // 0x00: Set lower column start address
        SET_HIGH_COLUMN,                      // 0x10: Set higher column start address
        SET_CHARGE_PUMP,               0x14,  // 0x8D: Enable charge pump regulator
        DEACTIVATE_SCROLL,                   // 0x2E: Deactivate scrolling
        SET_NORMAL_DISPLAY,                  // 0xA6: Set normal display mode (not inverted)
        DISPLAY_ON 
    };

    unsigned char poscode[] = {
        SET_LOW_COLUMN,            // low col = 0
        SET_HIGH_COLUMN,           // hi col = 0
        SET_DISPLAY_START_LINE     // line #0
    };

    pinMode (DC, OUTPUT);
    pinMode (RST, OUTPUT);
    wiringPiSPISetup(0, 8*1000*1000);
    
    // reset
    digitalWrite(RST,  LOW);
    sleep_for(milliseconds(50));
    digitalWrite(RST,  HIGH);
    
    // init
    digitalWrite(DC, LOW);
    wiringPiSPIDataRW(0, initcode, 28);

    while (true) {
        // Oled data.
        // Clear screen.
        oled.clear();

        // Print Hello World to display.
        oled.println("Hello world!", 16, 16, 32, 1);
        
        // Send data to display.
        digitalWrite(DC, LOW);
        wiringPiSPIDataRW(0, poscode, 3);
        digitalWrite(DC, HIGH);
        wiringPiSPIDataRW(0, oled.pix_buf, 1024);

        sleep_for(milliseconds(33));
    }
}

void Threads::radio_t() {
    const char* device = "plughw:1,0"; // ALSA device (adjust later if needed)
    snd_pcm_t* handle;
    snd_pcm_hw_params_t* params;
    unsigned int rate = 44100; // Sample rate
    int channels = 1;          // Mono
    snd_pcm_uframes_t frames = 32; // Frames per period
    int buffer_size = frames * channels * 2; // 2 bytes per sample (16-bit audio)
    //char* buffer = new char[buffer_size];
    bool recording_last = false;
    bool recording = false;

    // Open PCM device for recording
    if (snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        cerr << "Error: Unable to open PCM device." << endl;
        return;
    }

    // Allocate hardware parameters object
    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(handle, params);

    // Set hardware parameters
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(handle, params, channels);
    snd_pcm_hw_params_set_rate_near(handle, params, &rate, nullptr);
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, nullptr);

    // Apply hardware parameters
    if (snd_pcm_hw_params(handle, params) < 0) {
        cerr << "Error: Unable to set hardware parameters." << endl;
        snd_pcm_hw_params_free(params);
        snd_pcm_close(handle);
        delete[] buffer;
        return;
    }

    snd_pcm_hw_params_free(params); // Free hardware parameters object

    WAVHeader header;
    memset(&header, 0, sizeof(WAVHeader));
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt");
    header.subchunk1_size = 16; // PCM format.
    header.audio_format = 1; // PCM format.
    header.num_channels = channels;
    header.sample_rate = rate;
    header.bits_per_sample = 16;
    header.byte_rate = rate * channels * (header.bits_per_sample / 8);
    memcpy(header.data, "data", 4);
    header.subchunk2_size = 0; // Will update later.
    header.chunk_size = 36 + header.subchunk2_size; // Will update later.

    // Recording loop
    size_t total_data_size = 0;
    FILE* output_file = null;
    while (true) {
        recording = (digitalRead(RADIO_BUTTON) == HIGH);
        if (recording) {
            if (!recording_last) {
                cout << "Recording started." << endl;
                char* buffer = new char[buffer_size];

                // Open output file for saving recorded audio
                output_file = fopen("/tmp/mic_recording.wav", "wb");
                if (!output_file) {
                    cerr << "Error: Unable to open output file for recording." << endl;
                    snd_pcm_close(handle);
                    delete[] buffer;
                    return;
                }
            }

            int rc = snd_pcm_readi(handle, buffer, frames);
            if (rc == -EPIPE) {
                // Buffer overrun
                cerr << "Warning: Buffer overrun occurred." << endl;
                snd_pcm_prepare(handle);
            } else if (rc < 0) {
                cerr << "Error: Failed to read from PCM device: " << snd_strerror(rc) << endl;
                break;
            } else if (rc != (int)frames) {
                cerr << "Warning: Short read, only " << rc << " frames captured." << endl;
            }

            // Write captured audio to file
            size_t written = fwrite(buffer, 1, buffer_size, output_file);
            total_data_size += written;
        } else if (recording_last) {
            cout << "Recording stopped." << endl;
            header.subchunk2_size = total_data_size;
            header.chunk_size = 36 + header.subchunk2_size;
            fseek(output_file, 0, SEEK_SET);
            fwrite(&header, sizeof(WAVHeader), 1, output_file);
            fclose(output_file);

            try {
                delete[] buffer;
            } catch (...) {
                cout << "Buffer doesn't exist?? This shouldn't happen." << endl;
            }

            // Send radio message.
            system("/usr/bin/pi_fm_rds -freq 103.7 -audio /tmp/mic_recording.wav")

            // Cleanup.
            system("rm /tmp/mic_recording.wav");
        }

        recording_last = recording;
        if (!recording) sleep_for(milliseconds(50));
    }

    cout << "Recording stopped." << endl;

    // Cleanup
    //fclose(output_file);
    snd_pcm_close(handle);
    //delete[] buffer;
}

void start_telementry() {
    telementry_running = true;

    // Create threads
    thread ffmpeg_thread(Threads::ffmpeg_t);
    thread data_thread(Threads::data_t);

    // Detach threads to allow independent execution
    ffmpeg_thread.detach();
    data_thread.detach();
}

int main(int argc, char **argv) {
    cout << "Starting gokart service." << endl;

    // Configure server.
    cout << "Reading environment varibles" << endl;
    server.ip = (string) getenv("SERVERIP");
    server.username = (string) getenv("SERVERUSERNAME");
    server.passwd = (string) getenv("SERVERPASSWD");
    cout << "Server configured at " << server.ip << endl;
    cout << "attempting login" << endl;

    // Login.

    // Setup GPIO
    cout << "Initalizing GPIO" << endl;
    if (wiringPiSetupPinType(WPI_PIN_BCM) == -1) {
        cerr << "Error: Failed to initialize GPIO." << endl;
        return -1;
    }

    // Set pin modes.
    pinMode(TELEMENTRY_PIN, INPUT);
    pinMode(RADIO_BUTTON, INPUT);
    pinMode(RADIO_PIN, OUTPUT);

    // Create display thread.
    thread display_thread(Threads::display_t);

    display_thread.detach();

    // Check if radio enabled.
    if (digitalRead(TELEMENTRY_PIN) == HIGH){
        thread radio_thread(Threads::radio_t);
        radio_thread.detach();
    }

    // Check if telementry enabled.
    if (digitalRead(TELEMENTRY_PIN) == HIGH){
        cout << "Waiting for network." << endl;
        cout << "Network connected, took " << Networking::wait_for_network() << "s" << endl;
    
        while (true) {
            if (Networking::check_network() && !telementry_running) {
                start_telementry();
            }

            sleep_for(milliseconds(10000));
        }
    }

    // Return 0.
    return 0;
}