#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>
#include <ui.h>
#include <lv_conf.h>
#include <functional>

/* LVGL porting configurations */
#define LVGL_TICK_PERIOD_MS     (2)
#define LVGL_TASK_MAX_DELAY_MS  (500)
#define LVGL_TASK_MIN_DELAY_MS  (1)
#define LVGL_BUF_SIZE           (ESP_PANEL_LCD_H_RES * 20) // 800*20 

#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// I2C Pin define
#define I2C_MASTER_NUM 0
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_SCL_IO 9

#define LVGL_TASKNAME             "lvgl"
#define LVGL_TASK_STACK_SIZE      4 * 1024
#define LVGL_TASK_PRIORITY        10
#define LVGL_COREID               0

#define USE_LCD_TOUCH       0

#define BATTERY_DJI_CELL_NUM    14
#define BMS_CELL_NUM    16

#define DISPLAY_UNIT_TEST       !TRUE

#define NUM_OBJ_VALUE   20
#define NUM_OBJ_TITLE   20
#define NUM_OBJ         20
#define NUM_STYLE       20
#define NUM_FONT        34

LV_IMG_DECLARE(ecodrone_resize);
LV_IMG_DECLARE(symbol_clock);
LV_IMG_DECLARE(product_name);
LV_IMG_DECLARE(charging);
LV_FONT_DECLARE(vni_font);
LV_FONT_DECLARE(vni_number);
LV_FONT_DECLARE(dji_gotham);
LV_FONT_DECLARE(vni_gotham_bold);
LV_FONT_DECLARE(vni_gotham_bold_70);
LV_FONT_DECLARE(vni_gotham_regular);
class Display
{
private:
    static HardwareSerial *debugPort; // Biến tĩnh để giữ tham chiếu
    static ESP_Panel *panel;
    static ESP_IOExpander *expander;
    static SemaphoreHandle_t lvgl_mux;                  // LVGL mutex
    TaskHandle_t TaskDisplay_Handler;
    static void lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
    static bool notify_lvgl_flush_ready(void *user_ctx);
    static void lvgl_port_tp_read(lv_indev_drv_t * indev, lv_indev_data_t * data);
    static void lvgl_port_lock(int timeout_ms);
    static void lvgl_port_unlock(void);
    static void lvgl_port_task(void *arg);
    static void unit_test(uint32_t *_time);

    static void on_create_lv_label(lv_obj_t **obj,
                                    lv_style_t *style,
                                    const lv_font_t *font, 
                                    lv_align_t align,
                                    lv_coord_t x_ofs, 
                                    lv_coord_t y_ofs);
    static void on_change_lv_label_text(lv_obj_t *obj, String str);
    static void on_create_lv_img(lv_obj_t **obj, 
                                const void *src, 
                                int16_t zoom,
                                lv_align_t align, 
                                lv_coord_t x_ofs, 
                                lv_coord_t y_ofs);
    static void on_create_lv_line(lv_obj_t **obj,
                                lv_style_t *style,
                                lv_point_t line_points[],
                                lv_align_t align,
                                lv_coord_t x_ofs, 
                                lv_coord_t y_ofs);

    static lv_obj_t *g_title[NUM_OBJ_TITLE];
    static lv_obj_t *g_value[NUM_OBJ_VALUE];
    static lv_obj_t *g_obj[NUM_OBJ];
    static lv_style_t g_style[NUM_STYLE];
    static lv_style_t g_font[NUM_FONT];

    struct FIX_FRAME
    {
        static lv_obj_t *lb_status_obj;
        static String lb_status_str;
        static lv_obj_t *img_clock_obj;
        static lv_obj_t *lb_clock_obj;
        static lv_style_t font_vni_n;
        static lv_style_t main_panel_style;
        static lv_obj_t *main_panel;
        static bool checkCreate;
        static lv_obj_t *logo_charge;
        static lv_style_t font_vni_g_b;
        static lv_obj_t *logo;
        static void onCreate()
        {
            if(checkCreate)
            {
                // Top panel
                // logo
                //  on_create_lv_img(&logo, &ecodrone_resize, -1, LV_ALIGN_CENTER, 0, 0); 
                on_create_lv_img(&logo, &ecodrone_resize, 100, LV_ALIGN_TOP_LEFT, -45, -40);
                // status label
                // on_create_lv_img(&lb_status_obj, &product_name, 500, LV_ALIGN_TOP_MID, 0, ESP_PANEL_LCD_V_RES*0.068*0.8);
                // product name
                on_create_lv_label(&lb_status_obj, &font_vni_g_b, &vni_gotham_bold, LV_ALIGN_TOP_MID, 0, 5);
                lv_label_set_text(lb_status_obj, "BATTERY");
                // clock
                on_create_lv_label(&lb_clock_obj, &font_vni_n, &vni_number, LV_ALIGN_TOP_RIGHT, -10, ESP_PANEL_LCD_V_RES*0.068*0.5);
                lv_label_set_text(lb_clock_obj, "0:00");

                lv_style_init(&main_panel_style);
                lv_style_set_radius(&main_panel_style, 8);
                lv_style_set_border_width(&main_panel_style, 4);
                lv_style_set_border_color(&main_panel_style, lv_color_make(0xB9, 0xB9, 0xB9));
                main_panel = lv_obj_create(lv_scr_act());
                lv_obj_add_style(main_panel, &main_panel_style, 0);
                lv_obj_set_size(main_panel, ESP_PANEL_LCD_H_RES*0.99, ESP_PANEL_LCD_V_RES*0.86);
                lv_obj_align(main_panel, LV_ALIGN_CENTER, 0, ESP_PANEL_LCD_V_RES*0.068);
                lv_obj_clear_flag(main_panel,LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_clear_flag(lv_scr_act(),LV_OBJ_FLAG_SCROLLABLE);
                checkCreate = false;
            }
        }

        static void onUpdate(String clock_str)
        {
            on_change_lv_label_text(lb_clock_obj, clock_str);
        }

        static void onDelete()
        {
            // logo left
            // lv_obj_del(BOOT_PAGE::logo);
            // status label
            lv_obj_remove_style_all(lb_status_obj);
            lv_obj_del(lb_status_obj);
            lv_style_reset(&font_vni_g_b);
            lv_style_reset(&font_vni_n);
            // clock system
            lv_obj_del(img_clock_obj);
            lv_obj_del(logo_charge);
            // Main Panel
            lv_style_reset(&main_panel_style);
            lv_obj_remove_style_all(main_panel); lv_obj_del(main_panel);
            checkCreate = true;
        }
    };

    struct BATTERY_DJI_PAGE
    {
        static uint8_t step;
        enum STEP
        {
            CREATE,
            UPDATE,
            DELETE,
        };
        static lv_color_t battery_color_percent;
        static lv_style_t cell_style_border[BMS_CELL_NUM];
        static lv_style_t cell_style_bg[BMS_CELL_NUM];
        static lv_obj_t  *cell_bar[BMS_CELL_NUM];
        static lv_obj_t  *cell_lb[BMS_CELL_NUM];

        static void onCreate()
        {
            // battery name
            on_create_lv_label(&g_value[0], &g_font[0], &vni_gotham_bold, LV_ALIGN_TOP_LEFT, 30, 70);
            lv_label_set_text(g_value[0], "N/A");
            // arc percent
            g_obj[0] = lv_arc_create(lv_scr_act());
            lv_obj_set_style_arc_color(g_obj[0], lv_color_make(0xA7, 0xA7, 0xA7), 0); // Arc color
            lv_obj_set_style_arc_color(g_obj[0], lv_color_make(0x16, 0x91, 0x50), LV_PART_INDICATOR);
            lv_obj_set_style_arc_width(g_obj[0], 30, 0); // Arc width
            lv_obj_set_style_arc_width(g_obj[0], 30, LV_PART_INDICATOR); // Arc width'
            lv_obj_set_size(g_obj[0], 220, 220);
            lv_arc_set_rotation(g_obj[0], 270);
            lv_arc_set_bg_angles(g_obj[0], 0, 360);
            lv_obj_remove_style(g_obj[0], NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
            lv_obj_clear_flag(g_obj[0], LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
            lv_obj_align(g_obj[0], LV_ALIGN_LEFT_MID, 60, -5);
            lv_arc_set_value(g_obj[0], 00);
            lv_obj_clear_flag(g_obj[0],LV_OBJ_FLAG_SCROLLABLE);
            // label percent
            lv_style_init(&g_font[1]);
            lv_style_set_text_font(&g_font[1], &dji_gotham); 
            g_value[1] = lv_label_create(g_obj[0]);
            lv_obj_add_style(g_value[1], &g_font[1], 0);  // <--- obj is the label
            lv_obj_center(g_value[1]);
            lv_obj_set_style_text_color(g_value[1], lv_color_make(0x16, 0x91, 0x50), 0);
            lv_label_set_text(g_value[1], "00");
            lv_obj_clear_flag(g_value[1],LV_OBJ_FLAG_SCROLLABLE);

            lv_style_init(&g_font[2]);
            lv_style_set_text_font(&g_font[2], &vni_number); 
            g_value[2] = lv_label_create(g_obj[0]);
            lv_obj_add_style(g_value[2], &g_font[2], 0);  // <--- obj is the label
            lv_obj_set_style_text_color(g_value[2], lv_color_make(0x16, 0x91, 0x50), 0);
            lv_obj_align(g_value[2], LV_ALIGN_BOTTOM_MID, 0, -35);
            lv_label_set_text(g_value[2], "%");
            lv_obj_clear_flag(g_value[2],LV_OBJ_FLAG_SCROLLABLE);

            // voltage
            on_create_lv_label(&g_title[0], &g_font[3], &vni_font, LV_ALIGN_TOP_RIGHT, -ESP_PANEL_LCD_H_RES*0.345, 70);
            lv_label_set_text(g_title[0], "Điện áp        (V)");

            on_create_lv_label(&g_value[3], &g_font[4], &vni_gotham_bold_70, LV_ALIGN_TOP_RIGHT, -ESP_PANEL_LCD_H_RES*0.36, 110);
            lv_obj_set_style_text_color(g_value[3], lv_color_make(0x51, 0x45, 0xAF), 0);
            lv_label_set_text(g_value[3], "00.0");
            // current
            on_create_lv_label(&g_title[1], &g_font[5], &vni_font, LV_ALIGN_TOP_RIGHT, -30, 70);
            lv_label_set_text(g_title[1], "Dòng điện  (A)");

            on_create_lv_label(&g_value[4], &g_font[6], &vni_gotham_bold_70, LV_ALIGN_TOP_RIGHT, -30, 110);
            lv_obj_set_style_text_color(g_value[4], lv_color_make(0x51, 0x45, 0xAF), 0);
            lv_label_set_text(g_value[4], "00.0");
            // temperature
            on_create_lv_label(&g_title[2], &g_font[7], &vni_font, LV_ALIGN_TOP_RIGHT, -ESP_PANEL_LCD_H_RES*0.345, 180);
            lv_label_set_text(g_title[2], "Nhiệt độ    (°C)");

            on_create_lv_label(&g_value[5], &g_font[8], &vni_gotham_bold_70, LV_ALIGN_TOP_RIGHT, -ESP_PANEL_LCD_H_RES*0.36, 230);
            lv_obj_set_style_text_color(g_value[5], lv_color_make(0x51, 0x45, 0xAF), 0);
            lv_label_set_text(g_value[5], "00.0");
            // numberCharge
            on_create_lv_label(&g_title[3], &g_font[9], &vni_font, LV_ALIGN_TOP_RIGHT, -30, 180);
            lv_label_set_text(g_title[3], "Số lần sạc   ");

            on_create_lv_label(&g_value[6], &g_font[10], &vni_gotham_bold_70, LV_ALIGN_TOP_RIGHT, -30, 230);
            lv_obj_set_style_text_color(g_value[6], lv_color_make(0x51, 0x45, 0xAF), 0);
            lv_label_set_text(g_value[6], "0000");
            // version
            on_create_lv_label(&g_title[4], &g_font[11], &vni_font, LV_ALIGN_BOTTOM_LEFT, 330, -150);
            lv_label_set_text(g_title[4], "Ver:");

            on_create_lv_label(&g_value[7], &g_font[12], &vni_font, LV_ALIGN_BOTTOM_LEFT, 400, -150);
            lv_label_set_text(g_value[7], "N/A");   
            // seri number
            on_create_lv_label(&g_title[5], &g_font[13], &vni_font, LV_ALIGN_BOTTOM_LEFT, 330, -110);
            lv_label_set_text(g_title[5], "S/N:");
            
            on_create_lv_label(&g_value[8], &g_font[14], &vni_font, LV_ALIGN_BOTTOM_LEFT, 400, -110);
            lv_label_set_text(g_value[8], "N/A");
            // Err
            on_create_lv_label(&g_title[6], &g_font[15], &vni_font, LV_ALIGN_TOP_RIGHT, -120, 290);
            lv_label_set_text(g_title[6], "Er:");
            
            on_create_lv_label(&g_value[9], &g_font[16], &vni_font, LV_ALIGN_TOP_RIGHT, -30, 290);
            lv_label_set_text(g_value[9], "N/A");

            // cell
            for (uint8_t i = 0; i < BATTERY_DJI_CELL_NUM; i++)
            {
                lv_style_init(&cell_style_bg[i]);
                lv_style_set_border_color(&cell_style_bg[i], lv_color_make(0xC9, 0xC9, 0xC9));
                lv_style_set_border_width(&cell_style_bg[i], 2);
                lv_style_set_pad_all(&cell_style_bg[i], 1); /*To make the indicator smaller*/
                lv_style_set_radius(&cell_style_bg[i], 3);
                lv_style_set_anim_time(&cell_style_bg[i], 1000);

                lv_style_init(&cell_style_border[i]);
                lv_style_set_bg_opa(&cell_style_border[i], LV_OPA_COVER);
                lv_style_set_bg_color(&cell_style_border[i], lv_color_make(0x0A, 0xA6, 0x53));
                lv_style_set_radius(&cell_style_border[i], 3);

                cell_bar[i] = lv_bar_create(lv_scr_act());
                lv_obj_remove_style_all(cell_bar[i]);  /*To have a clean start*/
                lv_obj_add_style(cell_bar[i], &cell_style_bg[i], 0);
                lv_obj_add_style(cell_bar[i], &cell_style_border[i], LV_PART_INDICATOR);

                lv_obj_set_size(cell_bar[i], 30, 70);
                lv_obj_align(cell_bar[i], LV_ALIGN_BOTTOM_LEFT, 30 + i*54.5, -30);
                lv_bar_set_value(cell_bar[i], 0, LV_ANIM_ON);

                cell_lb[i] = lv_label_create(lv_scr_act());
                lv_label_set_text(cell_lb[i] , "0.00"); // Đặt số ban đầu
                lv_obj_align_to(cell_lb[i] , cell_bar[i], LV_ALIGN_OUT_BOTTOM_MID, 12, 0); // Căn chỉnh nhãn phía dưới thanh bar
                lv_obj_set_style_text_color(cell_lb[i] , lv_color_hex(0x000000), LV_PART_MAIN); // Màu đen
                lv_obj_set_style_text_font(cell_lb[i] , &lv_font_montserrat_18, LV_PART_MAIN);
            
                lv_obj_clear_flag(cell_bar[i],LV_OBJ_FLAG_SCROLLABLE);
                lv_obj_clear_flag(cell_lb[i],LV_OBJ_FLAG_SCROLLABLE);
            }

            // clear flag
            lv_obj_clear_flag(lv_scr_act(),LV_OBJ_FLAG_SCROLLABLE);
        }

        static void onUpdate(uint8_t ins)
        {
            //debugPort->printf("ins: %d \n", ins);
            // battery Name
            switch (batfr::capacity)
            {
            case BATTERY_TYPE::BATTERY_UNKNOWN:
                lv_label_set_text(g_value[0], (ins == 0) ? "> N/A" : "< N/A");
                break;
            case BATTERY_TYPE::BATTERY_T20P:
                lv_label_set_text(g_value[0], (ins == 0) ? "> T20P" : "< T20P");
                break;
            case BATTERY_TYPE::BATTERY_T25:
                lv_label_set_text(g_value[0], (ins == 0) ? "> T25" : "< T25");
                break;
            case BATTERY_TYPE::BATTERY_T30:
                lv_label_set_text(g_value[0], (ins == 0) ? "> T30" : "< T30");
                break;
            case BATTERY_TYPE::BATTERY_T40:
                lv_label_set_text(g_value[0], (ins == 0) ? "> T40" : "< T40");
                break;
            case BATTERY_TYPE::BATTERY_T50:
                lv_label_set_text(g_value[0], (ins == 0) ? "> T50" : "< T50");
                break;
            }
            // voltage
            on_change_lv_label_text(g_value[3], (batfr::capacity > 0) ? String((float)(batfr::voltage)/1000.0, 1): "00.0");
            // current
            on_change_lv_label_text(g_value[4], (batfr::capacity > 0) ? String((float)(batfr::current)/1000.0, 1) : "00.0");
            // percent
            on_change_lv_label_text(g_value[1], (batfr::capacity > 0) ? ((batfr::percent < 10) ? "0" : "" ) + (String)batfr::percent: "00");
            lv_arc_set_value(g_obj[0], (batfr::capacity > 0) ? batfr::percent : 0);
            // temperature
            on_change_lv_label_text(g_value[5], (batfr::capacity > 0) ? String((float)(batfr::temperature)/10.0, 1): "00.0");
            // numbercharge
            on_change_lv_label_text(g_value[6], (batfr::capacity > 0) ? String(batfr::numberCharge): "0000");
            // version
            on_change_lv_label_text(g_value[7], (batfr::capacity > 0) ? (String)batfr::version[0] + "." + (String)batfr::version[1] + "." + (String)batfr::version[2] + "." + (String)batfr::version[3]: "N/A"); // update sau
            // seri number
            char _tempSN[14]= {};
            for (uint8_t i = 0; i < 14; i++) _tempSN[i] = (char)batfr::seriNumber[i];
            lv_label_set_text(g_value[8], (batfr::capacity > 0) ? _tempSN : "N/A");
            // error
            on_change_lv_label_text(g_value[9], (batfr::capacity > 0) ? (String)batfr::countError : "N/A");
            // cell
            uint16_t _tempCell = 0;
            for (uint8_t i = 0; i < BATTERY_DJI_CELL_NUM; i++)
            { 
                if(batfr::capacity > 0) _tempCell = map(batfr::cell[i], 3000, 4250, 0, 100);
                else _tempCell= 0;
                // 0 - 10% do
                if(_tempCell <= 10)  lv_style_set_bg_color(&cell_style_border[i], lv_color_hex(0xDF0000));
                // 10-20% vang
                if(_tempCell > 10 && _tempCell <= 20)  lv_style_set_bg_color(&cell_style_border[i], lv_color_hex(0xE2E755));
                // > 20 xanh
                if(_tempCell > 20 && _tempCell <= 100) lv_style_set_bg_color(&cell_style_border[i], lv_color_make(0x0A, 0xA6, 0x53));

                lv_bar_set_value(cell_bar[i], _tempCell, LV_ANIM_ON);
                on_change_lv_label_text(cell_lb[i] , (batfr::capacity > 0) ? String((float)(batfr::cell[i])/1000.0, 2): "00.0"); // Đặt số ban đầu
            }

            // color
            if(batfr::percent <= 10)
            {
                battery_color_percent = lv_color_hex(0xDF0000);
            }
            if (batfr::percent <= 20 && batfr::percent > 10)
            {
                battery_color_percent = lv_color_hex(0xE2E755);
            }
            if(batfr::percent > 20)
            {
                battery_color_percent = lv_color_make(0x0A, 0xA6, 0x53);
            }
            lv_obj_set_style_arc_color(g_obj[0], battery_color_percent, LV_PART_INDICATOR);
            lv_obj_set_style_text_color(g_value[1], battery_color_percent, 0);
            lv_obj_set_style_text_color(g_value[2], battery_color_percent, 0);
        }

        static void onDelete()
        {
            for(uint8_t i = 0; i < 17; i++) lv_style_reset(&g_font[i]);
            for(uint8_t i = 0; i < 7; i++) { lv_obj_remove_style_all(g_title[i]); lv_obj_del(g_title[i]);}
            for(uint8_t i = 0; i < 10; i++) { lv_obj_remove_style_all(g_value[i]); lv_obj_del(g_value[i]);}
            for(uint8_t i = 0; i < 1; i++) { lv_obj_remove_style_all(g_obj[i]); lv_obj_del(g_obj[i]);}
            //cell
            for (uint8_t i = 0; i < BATTERY_DJI_CELL_NUM; i++)
            {
                lv_style_reset(&cell_style_bg[i]);
                lv_style_reset(&cell_style_border[i]);
                lv_obj_remove_style_all(cell_bar[i]); lv_obj_del(cell_bar[i]);
                lv_obj_remove_style_all(cell_lb[i]); lv_obj_del(cell_lb[i]);
            }
        }
    };

    static uint8_t choose_page;
    static uint8_t bms_instance;
    static char digit_buffer[10];
public:
    Display(HardwareSerial &debugPort);
    ~Display();

    static uint8_t page;
    static bool page_created;
    static uint8_t state_value;
    
    struct batfr
    {
        static bool active;
        static bool charging;
        static uint8_t seriNumber[14];
        static uint8_t version[4];
        static uint16_t voltage;
        static int32_t current;
        static uint8_t percent;
        static uint16_t cell[14];
        static int16_t numberCharge;
        static uint16_t temperature;
        static uint16_t capacity;
        static uint8_t countError;
        static uint8_t led;
        static int seconds; 
        static int minutes; 
        // Hàm để reset toàn bộ biến về 0
        static void reset() {
            active = false;
            charging = false;
            memset(seriNumber, 0, sizeof(seriNumber));
            memset(version, 0, sizeof(version));
            voltage = 0;
            current = 0;
            percent = 0;
            memset(cell, 0, sizeof(cell));
            numberCharge = 0;
            temperature = 0;
            capacity = 0;
            countError = 0;
            led = 0;
            seconds = 0;
            minutes = 0;
        }
    };


    enum PAGE
    {
        BOOT,
        MAIN,
        BATTERY_DJI,
        BMS1,
        BMS2,
        SYSTEM,
    };

    enum BATTERY_TYPE
    {
        BATTERY_UNKNOWN = 0,
        BATTERY_T30 = 29000,
        BATTERY_T20P = 13000,
        BATTERY_T40 = 30000,
        BATTERY_T25 = 16818,
        BATTERY_T50 = 33022,
    };

    bool init();
    void handleUpdateFirmware(bool option);
};

#endif // 