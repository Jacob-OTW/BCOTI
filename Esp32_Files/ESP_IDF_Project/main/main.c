#include <stdio.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "json_parser.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include <math.h>
#include "Mini2.h"

#define TAG "MAIN"

#define SSID "BCOTI"
#define PASSWORD "wifipass1234"
#define PRESET_COUNT 5

#define UART_TX GPIO_NUM_1
#define UART_RX GPIO_NUM_2
#define POTI_PIN GPIO_NUM_4
#define MULTI_BTN GPIO_NUM_8

volatile int64_t button_debouce;

extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
extern const uint8_t debug_html_start[] asm("_binary_debug_html_start");
extern const uint8_t debug_html_end[]   asm("_binary_debug_html_end");

static esp_netif_t *sta_netif = NULL;

nvs_handle_t flash_handle;

int8_t brightness_button_count = 0;

Mini2_t cam = {
    .uart_port = UART_NUM_1, // C3 only has num0 and num1, and num0 is used for debug / USB_CDC
    .uart_tx = UART_TX,
    .uart_rx = UART_RX,
    .variant = Mini2_256,
};

typedef struct stored_values_t{
    uint8_t active_preset;
    alignment_preset_t alignment;
    value_preset_t presets[PRESET_COUNT];
    bool first_boot;
} stored_values_t;

value_preset_t default_presets[] = {
    {
        .preset_en = true,
        .pseudo_color = WHOT,
        .scene_mode = Highlight,
        .contrast = 100,
        .edge_enhancment_gear = 1,
        .detail_enhancement_gear = 55,
        .burn_protection_en = true,
        .auto_shutter_en = true,
        .breathing = false
    },
    {
        .preset_en = true,
        .pseudo_color = WHOT,
        .scene_mode = HighContrast,
        .contrast = 100,
        .edge_enhancment_gear = 1,
        .detail_enhancement_gear = 55,
        .burn_protection_en = true,
        .auto_shutter_en = true,
        .breathing = false
    },
    {
        .preset_en = true,
        .pseudo_color = WHOT,
        .scene_mode = Outline,
        .contrast = 90,
        .edge_enhancment_gear = 1,
        .detail_enhancement_gear = 50,
        .burn_protection_en = true,
        .auto_shutter_en = true,
        .breathing = false
    }
};

value_preset_t base_preset = {
    .preset_en = false,
    .pseudo_color = WHOT,
    .scene_mode = Outline,
    .contrast = 50,
    .edge_enhancment_gear = 1,
    .detail_enhancement_gear = 50,
    .burn_protection_en = true,
    .auto_shutter_en = true,
    .breathing = false
};

stored_values_t stored = {
    .active_preset = 0,
    .alignment = {
        .zoom = 11,
        .zoom_x = 128,
        .zoom_y = 96,
        .av_format = PAL,
        .flip_mode = No_Flip,
        .fps = Hz50,
        .refresh_flip_mode = false,
    },
    .first_boot = true
};

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}

void next_preset() { // Preset0 is always seen as active, otherwise the esp32 could crash because it got stuck in a loop during ISR
    stored.active_preset = (stored.active_preset + 1) % PRESET_COUNT;
    if (stored.active_preset != 0 && !stored.presets[stored.active_preset].preset_en) {
        next_preset();
    }
}

void IRAM_ATTR button_isr_handler(void* arg) {
    if (esp_timer_get_time() - button_debouce > 200000) { // 200ms debounce
        button_debouce = esp_timer_get_time();
        next_preset();
    };
}

static void loop_task(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(2000));

    int max_gain = 0;
    int64_t breathing_time_out = 0;
    #define breathing_pause_time_us (2 * 1000 * 1000)

    uint8_t last_seen_preset = 255;

    adc_unit_t unit;
    adc_channel_t channel;
    esp_err_t err = adc_oneshot_io_to_channel(POTI_PIN, &unit, &channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get ADC info about GPIO%d", (int)POTI_PIN);
    }

    int adc_raw;
    int last_adc_val = 0;
    adc_oneshot_unit_handle_t adc_handle;
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, channel, &config));

    int64_t last_flip_check = esp_timer_get_time();
    #define FLIP_CHECK_INTERVAL (5 * 1000 * 1000)

    while (true) {
        adc_oneshot_read(adc_handle, channel, &adc_raw);
        if (abs(adc_raw - last_adc_val) >= 100) {
            last_adc_val = adc_raw;
            float new_brightness = ((float)adc_raw / 4095.0) * 100.0;
            max_gain = (int)new_brightness;
            Mini2_set_brightness(&cam, max_gain);
            breathing_time_out = esp_timer_get_time();
        }
        if (stored.presets[stored.active_preset].breathing && (esp_timer_get_time() - breathing_time_out) >= breathing_pause_time_us) {
            int x_ms = (esp_timer_get_time() - breathing_time_out - breathing_pause_time_us) / 1000;
            float wave_value = (cos(((float)x_ms / 1000.0) * M_PI) + 1.0) / 2.0;
            Mini2_set_brightness(&cam, (int)(wave_value * (float)max_gain));
        }
        if (stored.active_preset != last_seen_preset) {
            Mini2_apply_preset(&cam, &stored.presets[stored.active_preset], &stored.alignment, true);
            last_seen_preset = stored.active_preset;
            esp_err_t err = nvs_set_blob(flash_handle, "stored_values", &stored, sizeof(stored_values_t));
            if (err == ESP_OK) {
                ESP_LOGI(TAG, "Stored values in NVS");
            } else {
                ESP_LOGE(TAG, "Unable to store values in NVS");
            }
        }
        if (stored.alignment.refresh_flip_mode && esp_timer_get_time() - last_flip_check > FLIP_CHECK_INTERVAL) {
            last_flip_check = esp_timer_get_time();
            
            Mini2_set_flip_mode(&cam, stored.alignment.flip_mode);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static esp_err_t post_handler(httpd_req_t *req) {
    int ret;

    char* buf = (char*)malloc(req->content_len);

    if (buf == NULL) {
        ESP_LOGE(TAG, "Unable to malloc buffer for HTTP req");
        return ESP_FAIL;
    }

    jparse_ctx_t jctx;

    int bytes_read = httpd_req_recv(req, buf, req->content_len);

    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, bytes_read, ESP_LOG_WARN);

    ret = json_parse_start(&jctx, buf, bytes_read);
    if (ret != OS_SUCCESS) {
        ESP_LOGE(TAG, "Parser failed");
        return ESP_FAIL;
    }
    
    int value;
    ret = json_obj_get_int(&jctx, "pseudo_color", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_color_pallet(&cam, (enum PseudoColor)value);
        stored.presets[stored.active_preset].pseudo_color = (enum PseudoColor)value;
        Mini2_set_flip_mode(&cam, stored.alignment.flip_mode);
    }
    ret = json_obj_get_int(&jctx, "scene_mode", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_scene_mode(&cam, (enum SceneMode)value);
        stored.presets[stored.active_preset].scene_mode = (enum PseudoColor)value;
        Mini2_set_flip_mode(&cam, stored.alignment.flip_mode);
    }
    ret = json_obj_get_int(&jctx, "flip_mode", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_centre_zoom(&cam, 10);
        Mini2_set_flip_mode(&cam, (enum FlipMode)value);
        stored.alignment.flip_mode = (enum FlipMode)value;
    }
    ret = json_obj_get_int(&jctx, "av_format", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_analog_video_format(&cam, (enum AnalogVideoFormat)value);
        stored.alignment.av_format = (enum PseudoColor)value;
    }
    /* Brightness is done via Poti, so no need
            ret = json_obj_get_int(&jctx, "brightness", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_brightness(&cam, value);
        stored.presets[stored.active_preset].brightness = value;
    }
    */
    ret = json_obj_get_int(&jctx, "contrast", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_contrast(&cam, value);
        stored.presets[stored.active_preset].contrast = value;
    }
    ret = json_obj_get_int(&jctx, "edge_enhancment_gear", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_edge_enhancment(&cam, value);
        stored.presets[stored.active_preset].edge_enhancment_gear = value;
    }
    ret = json_obj_get_int(&jctx, "detail_enhancement_gear", &value);
    if (ret == OS_SUCCESS) {
        Mini2_set_detail_enhancement(&cam, value);
        stored.presets[stored.active_preset].detail_enhancement_gear = value;
    }

    bool bool_val;
    ret = json_obj_get_bool(&jctx, "burn_protection_en", &bool_val);
    if (ret == OS_SUCCESS) {
        Mini2_set_burn_protection(&cam, bool_val);
        stored.presets[stored.active_preset].burn_protection_en = bool_val;
    }
    ret = json_obj_get_bool(&jctx, "auto_shutter_en", &bool_val);
    if (ret == OS_SUCCESS) {
        Mini2_set_auto_shutter(&cam, bool_val);
        stored.presets[stored.active_preset].auto_shutter_en = bool_val;
    }

    ret = json_obj_get_bool(&jctx, "resend", &bool_val);
    if (ret == OS_SUCCESS && bool_val) {
        Mini2_restore_factory_parameters(&cam);
        vTaskDelay(pdMS_TO_TICKS(3000));
        Mini2_set_digital_video_format(&cam, true, UsbProgressive, cam.variant.high_fps);
        vTaskDelay(pdMS_TO_TICKS(3000));
        Mini2_set_analog_video_format(&cam, PAL);
        vTaskDelay(pdMS_TO_TICKS(3000));
        Mini2_set_detector_fps(&cam, cam.variant.high_fps);
        vTaskDelay(pdMS_TO_TICKS(3000));
        Mini2_save_video(&cam);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    ret = json_obj_get_bool(&jctx, "breathing", &bool_val);
    if (ret == OS_SUCCESS) {
        stored.presets[stored.active_preset].breathing = bool_val;
    }

    ret = json_obj_get_bool(&jctx, "refresh_flip_mode", &bool_val);
    if (ret == OS_SUCCESS) {
        stored.alignment.refresh_flip_mode = bool_val;
    }

    ret = json_obj_get_bool(&jctx, "preset_en", &bool_val);
    if (ret == OS_SUCCESS) {
        if (stored.active_preset == 0) {
            stored.presets[stored.active_preset].preset_en = true; // Preset0 is always enabled.
        } else {
            stored.presets[stored.active_preset].preset_en = bool_val;
        }
    }

    ret = json_obj_get_object(&jctx, "zoom");
    if (ret == OS_SUCCESS) {
        int x, y, zoom;
        if (json_obj_get_int(&jctx, "x", &x) == OS_SUCCESS && json_obj_get_int(&jctx, "y", &y) == OS_SUCCESS && json_obj_get_int(&jctx, "zoom", &zoom) == OS_SUCCESS) {
            Mini2_set_flip_mode(&cam, No_Flip);
            stored.alignment.flip_mode = No_Flip;
            Mini2_set_point_zoom(&cam, (uint16_t)x, (uint16_t)y, (uint8_t)zoom);
            stored.alignment.zoom = zoom;
            stored.alignment.zoom_x = x;
            stored.alignment.zoom_y = y;
        }
        json_obj_leave_object(&jctx);
    }

    ret = json_obj_get_int(&jctx, "active_profile", &value);
    if (ret == OS_SUCCESS) {
        if ((0 <= value) && (value < PRESET_COUNT)) {
            stored.active_preset = value;
            Mini2_apply_preset(&cam, &stored.presets[stored.active_preset], &stored.alignment, true);
        } else {
            ESP_LOGE(TAG, "Active profile number would be out-of-bounds!");
        }   
    }
    
    ret = json_obj_get_bool(&jctx, "NUC", &bool_val);
    if (ret == OS_SUCCESS && bool_val) {
        Mini2_NUC(&cam);
    }

    ret = json_obj_get_bool(&jctx, "BG", &bool_val);
    if (ret == OS_SUCCESS && bool_val) {
        Mini2_Background_Correction(&cam);
    }

    ret = json_obj_get_bool(&jctx, "parameters_save", &bool_val);
    if (ret == OS_SUCCESS && bool_val) {
        Mini2_parameters_save(&cam);
    }

    httpd_resp_send_chunk(req, NULL, 0);
    
    json_parse_end(&jctx);
    free(buf);

    if (stored.first_boot) {
        stored.first_boot = false;
    }

    esp_err_t err = nvs_set_blob(flash_handle, "stored_values", &stored, sizeof(stored_values_t));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Stored values in NVS");
    } else {
        ESP_LOGE(TAG, "Unable to store values in NVS");
    }
    return ESP_OK;
}

static esp_err_t retireve_values(httpd_req_t *req) {
    static char out_format[] = "{ \
    \"preset_count\": %u, \
    \"active_profile\": %u, \
    \"preset_en\": %u, \
    \"pseudo_color\": %u, \
    \"scene_mode\": %u, \
    \"contrast\": %u, \
    \"edge_enhancment_gear\": %u, \
    \"detail_enhancement_gear\": %u, \
    \"burn_protection_en\": %u, \
    \"auto_shutter_en\": %u, \
    \"breathing\": %u, \
    \"flip_mode\": %u, \
    \"zoom\": %u, \
    \"zoom_x\": %u, \
    \"zoom_y\": %u, \
    \"av_format\": %u, \
    \"sensor_width\": %u, \
    \"sensor_height\": %u, \
    \"refresh_flip_mode\": %u \
    }";

    char out_json[512];

    int res = snprintf(out_json, sizeof(out_json), out_format,
        PRESET_COUNT,
        stored.active_preset,
        (uint8_t)stored.presets[stored.active_preset].preset_en,
        (uint8_t)stored.presets[stored.active_preset].pseudo_color,
        (uint8_t)stored.presets[stored.active_preset].scene_mode,
        stored.presets[stored.active_preset].contrast,
        stored.presets[stored.active_preset].edge_enhancment_gear,
        stored.presets[stored.active_preset].detail_enhancement_gear,
        (uint8_t)stored.presets[stored.active_preset].burn_protection_en,
        (uint8_t)stored.presets[stored.active_preset].auto_shutter_en,
        (uint8_t)stored.presets[stored.active_preset].breathing,
        (uint8_t)stored.alignment.flip_mode,
        stored.alignment.zoom,
        stored.alignment.zoom_x,
        stored.alignment.zoom_y,
        stored.alignment.av_format,
        cam.variant.sensor_width,
        cam.variant.sensor_height,
        stored.alignment.refresh_flip_mode
    );

    if (res <= 0) {
        return ESP_FAIL;
    }

    httpd_resp_send(req, out_json, res);
    return ESP_OK;
}

static const httpd_uri_t retireve_values_route = {
    .uri       = "/get",
    .method    = HTTP_GET,
    .handler   = retireve_values,
    .user_ctx  = NULL
};

static const httpd_uri_t echo = {
    .uri       = "/set",
    .method    = HTTP_POST,
    .handler   = post_handler,
    .user_ctx  = NULL
};

esp_err_t debug_get_handler(httpd_req_t *req) {
    const uint32_t html_size = debug_html_end - debug_html_start;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)debug_html_start, html_size);
    return ESP_OK;
}

httpd_uri_t debug_uri = {
    .uri      = "/debug",
    .method   = HTTP_GET,
    .handler  = debug_get_handler,
    .user_ctx = NULL
};

esp_err_t index_get_handler(httpd_req_t *req) {
    const uint32_t html_size = index_html_end - index_html_start;
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, (const char *)index_html_start, html_size);
    return ESP_OK;
}

httpd_uri_t index_uri = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = index_get_handler,
    .user_ctx = NULL
};

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << MULTI_BTN),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    for (int i=0; i<PRESET_COUNT; i++) {
        if (i < (sizeof(default_presets) / sizeof(value_preset_t))) {
            stored.presets[i] = default_presets[i];
        } else {
            stored.presets[i] = base_preset;
        }
    }

    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &flash_handle));
    size_t len = sizeof(stored_values_t);
    esp_err_t err = nvs_get_blob(flash_handle, "stored_values", &stored, &len);
    if (err != ESP_OK ) {
        ESP_LOGE(TAG, "Failed to read stores from NVS, going with defaults.");
        stored.alignment.fps = cam.variant.high_fps;
    }

    bool wifi_en = (gpio_get_level(MULTI_BTN) == 0) || (stored.first_boot);
    ESP_LOGI(TAG, "Wifi: %d, stored: %d", (int)wifi_en, stored.first_boot);

    if (wifi_en) {
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        sta_netif = esp_netif_create_default_wifi_ap();
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
        wifi_config_t wifi_config = {
            .ap = {
                .ssid = SSID,
                .ssid_len = strlen(SSID),
                .password = PASSWORD,
                .channel = 1,
                .max_connection = 4,
                .authmode = WIFI_AUTH_WPA2_PSK,
                .pmf_cfg = {
                    .required = true,
                },
            }
        };
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    vTaskDelay(pdMS_TO_TICKS(5000));
    
    Mini2_init(&cam);
    Mini2_apply_preset(&cam, &stored.presets[stored.active_preset], &stored.alignment, false);

    xTaskCreate(loop_task, "loop task", 16384, NULL, 5, NULL);

    if (wifi_en) {
        httpd_handle_t server = NULL;
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        config.lru_purge_enable = true;

        ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
        if (httpd_start(&server, &config) == ESP_OK) {
            ESP_LOGI(TAG, "Registering URI handlers");
            httpd_register_uri_handler(server, &echo);
            httpd_register_uri_handler(server, &index_uri);
            httpd_register_uri_handler(server, &debug_uri);
            httpd_register_uri_handler(server, &retireve_values_route);
        }
    }

    button_debouce = esp_timer_get_time();
    gpio_install_isr_service(0);

    gpio_isr_handler_add(MULTI_BTN, button_isr_handler, NULL);
}
