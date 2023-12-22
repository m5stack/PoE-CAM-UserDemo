#include "Arduino.h"
#include "camera_api.h"
#include "ethernet_api.h"
#include <esp_heap_caps.h>

#include <M5UnitOLED.h>
#include <WiFi.h>

#define SERVER_PORT 80

// for factorytest
M5UnitOLED display;
M5Canvas canvas(&display);

void factory_oled_init();
void factory_wifi_test();

bool factory_test_mode = false;

// for poe cam server
static void jpegStream(EthernetClient* client);

static EthernetServer server(SERVER_PORT);

void setup() {
    Serial.begin(115200);

    pinMode(37, INPUT);

    // Blue LED
    pinMode(CAMERA_LED_GPIO, OUTPUT);
    digitalWrite(0, 0);

    Serial.printf("Camera init.....\r\n");

    delay(100);

    if (!digitalRead(37)) {
        delay(500);
        if (!digitalRead(37)) {
            factory_test_mode = true;
        }
    }

    if (factory_test_mode) {
        factory_oled_init();
        factory_wifi_test();
    }

    // Default Use FRAMESIZE_XGA 1024x768
    while (camrea_init() != ESP_OK) {
        if (factory_test_mode) {
            canvas.clear();
            canvas.setCursor(0, 0);
            canvas.printf("Camera init failed\r\n");
            canvas.pushSprite(0, 0);
        }

        Serial.printf("Camera init failed\r\n");
        digitalWrite(0, 1);
        delay(1000);
        digitalWrite(0, 0);
        delay(1000);
    }
    Serial.printf("Cam Init Success\r\n");

    if (factory_test_mode) {
        canvas.clear();
        canvas.setCursor(0, 0);
        canvas.printf("Camera init Success\r\n");
        canvas.pushSprite(0, 0);
    }

    while (!eth_init()) {
        if (factory_test_mode) {
            canvas.clear();
            canvas.setCursor(0, 0);
            canvas.printf("Eth init failed\r\n");
            canvas.pushSprite(0, 0);
        }

        Serial.printf("Eth init failed\r\n");
        digitalWrite(0, 1);
        delay(1000);
        digitalWrite(0, 0);
        delay(1000);
    }

    Serial.print("Eth Got Local IP at: ");
    Serial.println(eth_get_ip());
    server.begin();
    Serial.printf("Server Init Success\r\n");
    Serial.print("Listen on: ");
    Serial.println(eth_get_ip() + ":" + String(SERVER_PORT));

    if (factory_test_mode) {
        canvas.clear();
        canvas.setCursor(0, 0);
        canvas.println("Open " + eth_get_ip());
        canvas.pushSprite(0, 0);
    }
}

void loop() {
    // listen for incoming clients
    EthernetClient client = server.available();
    if (client) {
        Serial.println("new client from: " + client.remoteIP().toString());
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;

        while (client.connected()) {
            if (client.available()) {
                jpegStream(&client);
            }
        }
        // give the web browser time to receive the data
        delay(1);
        // close the connection:
        client.stop();
        Serial.println("client disconnected");
    }
}

// used to image stream
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE =
    "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART =
    "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static void jpegStream(EthernetClient* client) {
    camera_fb_t* fb = NULL;
    Serial.println("Image stream satrt");
    client->println("HTTP/1.1 200 OK");
    client->printf("Content-Type: %s\r\n", _STREAM_CONTENT_TYPE);
    client->println("Content-Disposition: inline; filename=capture.jpg");
    client->println("Access-Control-Allow-Origin: *");
    client->println();

    static int64_t last_frame = 0;
    if (!last_frame) {
        last_frame = esp_timer_get_time();
    }

    for (;;) {
        int64_t fr_start = esp_timer_get_time();
        fb               = esp_camera_fb_get();
        if (fb == NULL) {
            delay(10);
            continue;
        }
        client->print(_STREAM_BOUNDARY);
        client->printf(_STREAM_PART, fb);
        int32_t to_sends    = fb->len;
        int32_t now_sends   = 0;
        uint8_t* out_buf    = fb->buf;
        uint32_t packet_len = 8 * 1024;
        while (to_sends > 0) {
            now_sends = to_sends > packet_len ? packet_len : to_sends;
            if (client->write(out_buf, now_sends) == 0) {
                goto client_exit;
            }
            out_buf += now_sends;
            to_sends -= packet_len;
        }

        int64_t fr_end     = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame         = fr_end;
        frame_time /= 1000;
        Serial.printf("MJPG: %luKB %lums (%.1ffps)\r\n",
                      (uint32_t)(fb->len / 1024), (uint32_t)frame_time,
                      1000.0 / (uint32_t)frame_time);

        esp_camera_fb_return(fb);
        fb = NULL;
        if (digitalRead(37)) {
            digitalWrite(0, 1);
        } else {
            digitalWrite(0, 0);
        }
    }

client_exit:
    if (fb != NULL) {
        esp_camera_fb_return(fb);
    }
    client->stop();
    Serial.printf("Image stream end\r\n");
}

void factory_oled_init() {
    display.init(Ext_PIN_2, Ext_PIN_1);
    display.setRotation(3);
    canvas.setColorDepth(1);  // mono color
    canvas.setFont(&fonts::efontCN_12);
    canvas.setTextWrap(true);
    canvas.setTextSize(1);
    canvas.createSprite(display.width(), display.height());
}

void factory_wifi_test() {
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0) {
        canvas.fillSprite(0x0);
        canvas.setCursor(0, 0);
        canvas.printf("no wifi found\r\n");
        canvas.pushSprite(0, 0);
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < (n > 5 ? 5 : n); ++i) {
            // Print SSID and RSSI for each network found
            if (canvas.getCursorY() > 40) {
                canvas.fillSprite(0x0);
                canvas.setCursor(0, 0);
                canvas.pushSprite(0, 0);

            } else {
                canvas.print(i + 1);
                canvas.print(": ");
                canvas.print(WiFi.SSID(i));
                canvas.print(" (");
                canvas.print(WiFi.RSSI(i));
                canvas.print(")");
                canvas.println(
                    (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
                canvas.pushSprite(0, 0);
            }
            delay(500);
        }
    }

    canvas.fillSprite(0x0);
    canvas.setCursor(0, 0);
    canvas.printf("WiFi Scan: %d\r\n", n);
    canvas.pushSprite(0, 0);
    delay(2000);
}
