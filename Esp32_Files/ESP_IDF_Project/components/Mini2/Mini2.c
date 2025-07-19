#include "Mini2.h"

uint16_t crc16_xmodem(uint8_t* data, size_t len) {
    uint16_t crc = 0;
    for (int i=0; i<len; i++) {
        crc ^= (data[i] << 8);
        for (int j=0; j<8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void Mini2_init(Mini2_t* cam) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(cam->uart_port, 2048, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(cam->uart_port, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(cam->uart_port, cam->uart_tx, cam->uart_rx, -1, -1));
}

esp_err_t Mini2_write_command(Mini2_t* cam, uint8_t* cmd, size_t len) {
    uint8_t full_cmd[] = {0x55, 0x43, 0x49, 0x12, 0x00, 0x00 /*command class*/, 0x00 /*command index*/, 0x00 /*sub command*/, 0x00 /*reserved*/, 0x00, 0x00, 0x00, 0x00 /*4byte Parameter1*/, 0x00, 0x00, 0x00, 0x00 /*4byte Parameter2*/, 0x00, 0x00 /*Read length, 0 when writing*/, 0x00, 0x00 /*2btyes Reserved*/, 0x00, 0x00 /*2byte crc16 xmodem*/};

    if (len > 16) {
        ESP_LOGE(Mini2_TAG, "command length may not be longer than 16bytes");
        return ESP_FAIL;
    }

    memcpy(full_cmd + 5, cmd, len);
    uint16_t checksum = crc16_xmodem(full_cmd + 5, 16);
    memcpy(full_cmd + 21, &checksum, 2);

    ESP_LOG_BUFFER_HEXDUMP(Mini2_TAG, full_cmd, sizeof(full_cmd), ESP_LOG_INFO);
    
    for (int i=0; i < sizeof(full_cmd); i++) {
        int bytes_written = uart_write_bytes(cam->uart_port, full_cmd + i, 1);
        if (bytes_written == -1) {
            ESP_LOGE(Mini2_TAG, "Failed to write byte%d command to UART", i);
            return ESP_FAIL;
        }   
    }

    return ESP_OK;
}

esp_err_t Mini2_set_color_pallet(Mini2_t* cam, enum PseudoColor pseudo_color) {
    uint8_t cmd[] = {0x10, 0x03, 0x45, 0x00, 0x00, pseudo_color};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_scene_mode(Mini2_t* cam, enum SceneMode scene_mode) {
    uint8_t cmd[] = {0x10, 0x04, 0x42, 0x00, scene_mode};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_flip_mode(Mini2_t* cam, enum SceneMode FlipMode) {
    uint8_t cmd[] = {0x10, 0x10, 0x43, 0x00, FlipMode};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_analog_video_format(Mini2_t* cam, enum AnalogVideoFormat av_format) {
    uint8_t cmd[] = {0x10, 0x10, 0x4A, 0x00, 0x01, av_format};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_brightness(Mini2_t* cam, uint8_t brightness) {
    if (brightness > 100) {
        ESP_LOGE(Mini2_TAG, "Brightness must be <= 100");
        return ESP_FAIL;
    }
    uint8_t cmd[] = {0x10, 0x04, 0x47, 0x00, brightness};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_contrast(Mini2_t* cam, uint8_t contrast) {
    if (contrast > 100) {
        ESP_LOGE(Mini2_TAG, "Contrast must be <= 100");
        return ESP_FAIL;
    }
    uint8_t cmd[] = {0x10, 0x04, 0x4A, 0x00, contrast};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_edge_enhancment(Mini2_t* cam, uint8_t gear) {
    if (gear > 2) {
        ESP_LOGE(Mini2_TAG, "Edge enhancment gear must be <= 2");
        return ESP_FAIL;
    }
    uint8_t cmd[] = {0x10, 0x04, 0x4E, 0x00, gear};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_detail_enhancement(Mini2_t* cam, uint8_t gear) {
    if (gear > 100) {
        ESP_LOGE(Mini2_TAG, "Detail enhancement gear must be <= 100");
        return ESP_FAIL;
    }
    uint8_t cmd[] = {0x10, 0x03, 0x85, 0x00, gear};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_burn_protection(Mini2_t* cam, bool enabled) {
    uint8_t cmd[] = {0x10, 0x03, 0x4B, 0x00, (uint8_t)enabled};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_shutter_position(Mini2_t* cam, bool open) {
    uint8_t cmd[] = {0x01, 0x0F, 0x45, 0x00, (uint8_t)open};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_auto_shutter(Mini2_t* cam, bool enabled) {
    uint8_t cmd[] = {0x10, 0x02, 0x41, 0x00, (uint8_t)enabled};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_centre_zoom(Mini2_t* cam, uint8_t zoom) {
    if (zoom < 10 || zoom > 80 ) {
        ESP_LOGE(Mini2_TAG, "Zoom must be between 10 and 80 for 1x to 8x");
        return ESP_FAIL;
    }

    uint8_t cmd[] = {0x01, 0x31, 0x42, 0x00, 0x00, zoom};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_point_zoom(Mini2_t* cam, uint16_t x, uint16_t y, uint8_t zoom) {
    if (zoom < 10 || zoom > 80 ) {
        ESP_LOGE(Mini2_TAG, "Zoom must be between 10 and 80 for 1x to 8x");
        return ESP_FAIL;
    }
    if (x > cam->variant.sensor_width) {
        ESP_LOGE(Mini2_TAG, "X must be between 0 and sensor width");
        return ESP_FAIL;
    }
    if (y > cam->variant.sensor_height) {
        ESP_LOGE(Mini2_TAG, "Y must be between 0 and sensor height");
        return ESP_FAIL;
    }

    ESP_LOGI(Mini2_TAG, "Setting %.1fzoom at %dx, %dy", ((float)zoom / 10.0), (int)x, (int)y);

    uint8_t cmd[] = {0x01, 0x31, 0x51, 0x00, 0x00, zoom, 0x00, 0x00, *((uint8_t*)&x), *(((uint8_t*)&x) + 1)/*X*/, *((uint8_t*)&y), *(((uint8_t*)&y) + 1) /*Y*/};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_detector_fps(Mini2_t* cam, enum DetectorRefreshRate fps) {
    if (!(fps == cam->variant.low_fps || fps == cam->variant.high_fps)) {
        ESP_LOGE(Mini2_TAG, "Selected variant doesn't support this detector fps.");
        return ESP_FAIL;
    }
    uint8_t cmd[] = {0x10, 0x10, 0x44, 0x00, fps};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_sleep(Mini2_t* cam, bool sleep) {
    uint8_t cmd[] = {0x10, 0x10, 0x48, 0x00, (uint8_t)sleep};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_set_crosshair(Mini2_t* cam, bool enable) {
    uint8_t cmd[] = {0x10, 0x11, 0x57, 0x00, (uint8_t)enable};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_parameters_save(Mini2_t* cam) {
    uint8_t cmd[] = {0x10, 0x10, 0x51};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_restore_factory_parameters(Mini2_t* cam) {
    uint8_t cmd[] = {0x10, 0x10, 0x52};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_NUC(Mini2_t* cam) {
    uint8_t cmd[] = {0x10, 0x2, 0x43};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

esp_err_t Mini2_Background_Correction(Mini2_t* cam) {
    uint8_t cmd[] = {0x1, 0x10, 0x52};
    return Mini2_write_command(cam, cmd, sizeof(cmd));
}

void Mini2_apply_preset(Mini2_t* cam, value_preset_t* preset) {
    Mini2_set_color_pallet(cam, preset->pseudo_color);
    Mini2_set_scene_mode(cam, preset->scene_mode);
    Mini2_set_flip_mode(cam, preset->flip_mode);
    Mini2_set_analog_video_format(cam, preset->av_format);
    // Mini2_set_brightness(cam, preset->brightness);
    Mini2_set_contrast(cam, preset->contrast);
    Mini2_set_edge_enhancment(cam, preset->edge_enhancment_gear);
    Mini2_set_detail_enhancement(cam, preset->detail_enhancement_gear);
    Mini2_set_burn_protection(cam, preset->burn_protection_en);
    Mini2_set_auto_shutter(cam, preset->auto_shutter_en);
    // Mini2_set_point_zoom(cam, preset->zoom_x, preset->zoom_y, preset->zoom);
    Mini2_set_detector_fps(cam, preset->fps);
}
