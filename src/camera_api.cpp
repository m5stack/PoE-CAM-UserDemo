#include "camera_api.h"

static camera_config_t camera_config = {
    .pin_pwdn     = CAM_PIN_PWDN,
    .pin_reset    = CAM_PIN_RESET,
    .pin_xclk     = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7    = CAM_PIN_D7,
    .pin_d6    = CAM_PIN_D6,
    .pin_d5    = CAM_PIN_D5,
    .pin_d4    = CAM_PIN_D4,
    .pin_d3    = CAM_PIN_D3,
    .pin_d2    = CAM_PIN_D2,
    .pin_d1    = CAM_PIN_D1,
    .pin_d0    = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href  = CAM_PIN_HREF,
    .pin_pclk  = CAM_PIN_PCLK,

    .xclk_freq_hz = CAM_XCLK_FREQ,  // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or
                                    // ESP32-S3 to enable EDMA mode
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,  // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size =
        CAM_FRAMESIZE,  // QQVGA-UXGA, For ESP32, do not use sizes above QVGA
                        // when not JPEG. The performance of the ESP32-S series
                        // has improved a lot, but JPEG mode always gives
                        // better frame rates.
    .jpeg_quality = CAM_JPEG_QUALITY,  // 0-63, for OV series camera sensors,
                                       // lower number means higher quality
    .fb_count = 2,  // When jpeg mode is used, if fb_count more than one, the
                    // driver will work in continuous mode.
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY  // CAMERA_GRAB_LATEST. Sets when
                                         // buffers should be filled
};

esp_err_t camrea_init() {
    esp_err_t err = esp_camera_init(&camera_config);
    return err;
}
