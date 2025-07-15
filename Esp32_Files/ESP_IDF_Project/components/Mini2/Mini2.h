#pragma once

#include "esp_log.h"
#include "esp_mac.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include <stdio.h>
#include "string.h"

#define Mini2_TAG "Mini2_UART"

enum DetectorRefreshRate {
    Hz30 = 0x1e,
    Hz60 = 0x3c,
    Hz25 = 0x19,
    Hz50 = 0x32,
};

typedef struct Mini2_variant_t {
    int sensor_width;
    int sensor_height;
    enum DetectorRefreshRate low_fps;
    enum DetectorRefreshRate high_fps;
} Mini2_variant_t;

#define Mini2_256 {256, 192, Hz25, Hz50}
#define Mini2_384 {384, 288, Hz30, Hz60}
#define Mini2_640 {640, 512, Hz30, Hz60}

#define EMTPY_PARAMETER 0x00, 0x00, 0x00, 0x00

typedef struct Mini2_t {
    uart_port_t uart_port;
    gpio_num_t uart_tx;
    gpio_num_t uart_rx;
    Mini2_variant_t variant;
} Mini2_t;

enum PseudoColor {
    WHOT = 0x0,
    BHOT = 0x9,
};

enum SceneMode {
    LowHighlight = 0x00,
    LinearStretch = 0x01,
    LowContrast = 0x02,
    GeneralMode = 0x03,
    HighContrast = 0x04,
    Highlight = 0x05,
    Outline = 0x09,
};

enum FlipMode {
    No_Flip = 0x0,
    X_Flip = 0x1,
    Y_Flip = 0x2,
    XY_Flip = 0x3,
};

enum AnalogVideoFormat {
    NTSC = 0x0,
    PAL = 0x1,
};

typedef struct value_preset_t{
    enum PseudoColor pseudo_color;
    enum SceneMode scene_mode;
    enum FlipMode flip_mode;
    enum AnalogVideoFormat av_format;
    uint8_t brightness;
    uint8_t contrast;
    uint8_t edge_enhancment_gear;
    uint8_t detail_enhancement_gear;
    bool burn_protection_en;
    bool auto_shutter_en;
    uint8_t zoom;
    uint16_t zoom_x;
    uint16_t zoom_y;
    enum DetectorRefreshRate fps;
    bool breathing;
} value_preset_t;

uint16_t crc16_xmodem(uint8_t* data, size_t len);
void Mini2_init(Mini2_t* cam);
esp_err_t Mini2_set_color_pallet(Mini2_t* cam, enum PseudoColor pseudo_color);
esp_err_t Mini2_set_scene_mode(Mini2_t* cam, enum SceneMode scene_mode);
esp_err_t Mini2_set_flip_mode(Mini2_t* cam, enum SceneMode FlipMode);
esp_err_t Mini2_set_analog_video_format(Mini2_t* cam, enum AnalogVideoFormat av_format);
esp_err_t Mini2_set_brightness(Mini2_t* cam, uint8_t brightness);
esp_err_t Mini2_set_contrast(Mini2_t* cam, uint8_t contrast);
esp_err_t Mini2_set_edge_enhancment(Mini2_t* cam, uint8_t gear);
esp_err_t Mini2_set_detail_enhancement(Mini2_t* cam, uint8_t gear);
esp_err_t Mini2_set_burn_protection(Mini2_t* cam, bool enabled);
esp_err_t Mini2_set_shutter_position(Mini2_t* cam, bool open);
esp_err_t Mini2_set_auto_shutter(Mini2_t* cam, bool enabled);
esp_err_t Mini2_set_centre_zoom(Mini2_t* cam, uint8_t zoom);
esp_err_t Mini2_set_point_zoom(Mini2_t* cam, uint16_t x, uint16_t y, uint8_t zoom);
esp_err_t Mini2_set_detector_fps(Mini2_t* cam, enum DetectorRefreshRate fps);
esp_err_t Mini2_set_crosshair(Mini2_t* cam, bool enable);
esp_err_t Mini2_set_sleep(Mini2_t* cam, bool sleep);
esp_err_t Mini2_parameters_save(Mini2_t* cam);
esp_err_t Mini2_restore_factory_parameters(Mini2_t* cam);
esp_err_t Mini2_NUC(Mini2_t* cam);
esp_err_t Mini2_Background_Correction(Mini2_t* cam);
void Mini2_apply_preset(Mini2_t* cam, value_preset_t* preset);
