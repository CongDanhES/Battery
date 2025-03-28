#include "Display.hpp"

// Khởi tạo biến tĩnh
HardwareSerial *Display::debugPort = nullptr;

// Định nghĩa các biến
ESP_Panel *Display::panel = nullptr; // Hoặc khởi tạo theo cách bạn cần
SemaphoreHandle_t Display::lvgl_mux = nullptr;
ESP_IOExpander *Display::expander = nullptr;

uint8_t Display::page = 1;
bool Display::page_created = false;
uint8_t Display::state_value = 2;
char Display::digit_buffer[10] = {0};
uint8_t Display::choose_page = 2;
uint8_t Display::bms_instance = 0;

// batfr
bool Display::batfr::active = false;
bool Display::batfr::charging = false;
uint8_t Display::batfr::seriNumber[14] = {0};
uint8_t Display::batfr::version[4] = {0};
uint16_t Display::batfr::voltage = 0;
int32_t Display::batfr::current = 0;
uint8_t Display::batfr::percent = 0;
uint16_t Display::batfr::cell[14] = {0};
int16_t Display::batfr::numberCharge = 0;
uint16_t Display::batfr::temperature = 0;
uint16_t Display::batfr::capacity = 0;
uint8_t Display::batfr::countError = 0;
uint8_t Display::batfr::led = 0;
int Display::batfr::seconds = 0;
int Display::batfr::minutes = 0;

// Định nghĩa các thành viên tĩnh
lv_obj_t* Display::g_value[NUM_OBJ_TITLE] = {nullptr};
lv_obj_t* Display::g_title[NUM_OBJ_VALUE] = {nullptr};
lv_obj_t* Display::g_obj[NUM_OBJ] = {nullptr};
lv_style_t Display::g_style[NUM_STYLE];
lv_style_t Display::g_font[NUM_FONT];

// FIX_FRAMES
lv_style_t Display::FIX_FRAME::main_panel_style ;
lv_obj_t *Display::FIX_FRAME::main_panel = nullptr;
bool Display::FIX_FRAME::checkCreate = true;
lv_obj_t *Display::FIX_FRAME::lb_status_obj = nullptr;
String Display::FIX_FRAME::lb_status_str = "";
lv_obj_t *Display::FIX_FRAME::img_clock_obj = nullptr;
lv_obj_t *Display::FIX_FRAME::lb_clock_obj = nullptr;
lv_style_t Display::FIX_FRAME::font_vni_n;
lv_style_t Display::FIX_FRAME::font_vni_g_b;
lv_obj_t* Display::FIX_FRAME::logo_charge = nullptr;
lv_obj_t* Display::FIX_FRAME::logo = nullptr; // update logo from Mainpage

uint8_t Display::BATTERY_DJI_PAGE::step = 0;
lv_color_t Display::BATTERY_DJI_PAGE::battery_color_percent;
lv_style_t Display::BATTERY_DJI_PAGE::cell_style_border[BMS_CELL_NUM] = {};
lv_style_t Display::BATTERY_DJI_PAGE::cell_style_bg[BMS_CELL_NUM] = {};
lv_obj_t  *Display::BATTERY_DJI_PAGE::cell_bar[BMS_CELL_NUM] = {nullptr};
lv_obj_t  *Display::BATTERY_DJI_PAGE::cell_lb[BMS_CELL_NUM] = {nullptr};

Display::Display(HardwareSerial &debugPort) 
{
    this->debugPort = &debugPort;
}

Display::~Display(){}

bool Display::init()
{
    panel = new ESP_Panel();

    /* Initialize LVGL core */
    lv_init();

    /* Initialize LVGL buffers */
    static lv_disp_draw_buf_t draw_buf;
    /* Using double buffers is more faster than single buffer */
    /* Using internal SRAM is more fast than PSRAM (Note: Memory allocated using `malloc` may be located in PSRAM.) */
    uint8_t *buf = (uint8_t *)heap_caps_calloc(1, LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_INTERNAL);
    assert(buf);
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, LVGL_BUF_SIZE);

    /* Initialize the display device */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /* Change the following line to your display resolution */
    disp_drv.hor_res = ESP_PANEL_LCD_H_RES;
    disp_drv.ver_res = ESP_PANEL_LCD_V_RES;
    disp_drv.flush_cb = lvgl_port_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

#if ESP_PANEL_USE_LCD_TOUCH && USE_LCD_TOUCH
    /* Initialize the input device */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_port_tp_read;
    lv_indev_drv_register(&indev_drv);
#endif
    /* Initialize bus and device of panel */
    panel->init();
#if ESP_PANEL_LCD_BUS_TYPE != ESP_PANEL_BUS_TYPE_RGB
    /* Register a function to notify LVGL when the panel is ready to flush */
    /* This is useful for refreshing the screen using DMA transfers */
    panel->getLcd()->setCallback(notify_lvgl_flush_ready, &disp_drv);
#endif

    /**
     * These development boards require the use of an IO expander to configure the screen,
     * so it needs to be initialized in advance and registered with the panel for use.
     *
     */
    debugPort->println("Initialize IO expander");
    /* Initialize IO expander */
    // ESP_IOExpander *expander = new ESP_IOExpander_CH422G(I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
    expander = new ESP_IOExpander_CH422G(I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000);
    expander->init();
    expander->begin();
    expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
    expander->multiDigitalWrite(TP_RST | LCD_BL | LCD_RST | SD_CS, HIGH);

    // Turn off backlight
    // expander->digitalWrite(USB_SEL, LOW);
    expander->digitalWrite(USB_SEL, LOW);
    /* Add into panel */
    panel->addIOExpander(expander);

    /* Start panel */
    panel->begin();

    /* Create a task to run the LVGL task periodically */
    lvgl_mux = xSemaphoreCreateRecursiveMutex();
    xTaskCreate(lvgl_port_task, LVGL_TASKNAME, LVGL_TASK_STACK_SIZE, NULL, LVGL_TASK_PRIORITY, &TaskDisplay_Handler);

    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);

    /* Release the mutex */
    lvgl_port_unlock();

    debugPort->println("Setup display done");
    return true;
}

#if ESP_PANEL_LCD_BUS_TYPE == ESP_PANEL_BUS_TYPE_RGB
/* Display flushing */
void Display::lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    panel->getLcd()->drawBitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
    lv_disp_flush_ready(disp);
}
#else
/* Display flushing */
void Display::lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    panel->getLcd()->drawBitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_p);
}

bool Display::notify_lvgl_flush_ready(void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}
#endif /* ESP_PANEL_LCD_BUS_TYPE */

#if ESP_PANEL_USE_LCD_TOUCH && USE_LCD_TOUCH
/* Read the touchpad */
void Display::lvgl_port_tp_read(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    panel->getLcdTouch()->readData();

    bool touched = panel->getLcdTouch()->getTouchState();
    if(!touched) {
        data->state = LV_INDEV_STATE_REL;
    } else {
        TouchPoint point = panel->getLcdTouch()->getPoint();

        data->state = LV_INDEV_STATE_PR;
        /*Set the coordinates*/
        data->point.x = point.x;
        data->point.y = point.y;

        debugPort->printf("Touch point: x %d, y %d\n", point.x, point.y);
    }
}
#endif

void Display::lvgl_port_lock(int timeout_ms)
{
    const TickType_t timeout_ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    xSemaphoreTakeRecursive(lvgl_mux, timeout_ticks);
}

void Display::lvgl_port_unlock(void)
{
    xSemaphoreGiveRecursive(lvgl_mux);
}

void Display::lvgl_port_task(void *arg)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0xffffff), LV_PART_MAIN);

    // BOOT_PAGE::onCreate();

    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS/4;
    uint32_t lv_time = millis();
    uint32_t lv_time_update = millis();

    unsigned long previousMillis; // Lưu thời gian trước đó
    int lv_sys_seconds; // Biến đếm giây
    int lv_sys_minutes; // Biến đếm phút

    // ceate fix frame to display
    FIX_FRAME::onCreate();

    while (1) {
        // Lock the mutex due to the LVGL APIs are not thread-safe
        lvgl_port_lock(-1);
        task_delay_ms = lv_timer_handler();
        // Release the mutex
        lvgl_port_unlock();

        if (task_delay_ms > LVGL_TASK_MAX_DELAY_MS/4) {
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS/4;
        } else if (task_delay_ms < LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;
        }

        switch (choose_page)
        {
        case PAGE::BATTERY_DJI:
            switch (BATTERY_DJI_PAGE::step)
            {
            case BATTERY_DJI_PAGE::STEP::CREATE:
                BATTERY_DJI_PAGE::onCreate();
                BATTERY_DJI_PAGE::step = BATTERY_DJI_PAGE::STEP::UPDATE;
                break;
            case BATTERY_DJI_PAGE::STEP::UPDATE:
                vTaskDelay((50L * configTICK_RATE_HZ) / 1000L); 
                BATTERY_DJI_PAGE::onUpdate(0);
                page_created = true;
                break;
            case BATTERY_DJI_PAGE::STEP::DELETE:
                BATTERY_DJI_PAGE::onDelete();
                BATTERY_DJI_PAGE::step = BATTERY_DJI_PAGE::STEP::CREATE;
                choose_page = PAGE::SYSTEM;
                page_created = false;
                break;
            }
            break;
        }

        if (choose_page > PAGE::BOOT)
        {
            if (millis() - previousMillis >= 1000) 
            {
                previousMillis = millis(); // Cập nhật thời gian trước đó
                lv_sys_seconds++; // Tăng biến đếm giây
                if (lv_sys_seconds >= 60) 
                {
                    lv_sys_seconds = 0; // Đặt lại giây
                    lv_sys_minutes++; // Tăng biến đếm phút
                }
                
                FIX_FRAME::onUpdate((String)lv_sys_minutes + ":" + ((lv_sys_seconds < 10) ? "0" : "") +(String)lv_sys_seconds);
                
                if(batfr::capacity > 0 && batfr::current > 1000 && batfr::voltage > 40000)
                {
                    batfr::seconds++; 
                    if (batfr::seconds >= 60) 
                    {
                        batfr::seconds = 0; 
                        batfr::minutes++; 
                    }
                    batfr::charging = true;
                }
                else
                {
                    if(batfr::capacity > 0) {}
                    else
                    {
                        batfr::seconds = 0;
                        batfr::minutes = 0; 
                    }
                    
                    batfr::charging = false;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

void Display::handleUpdateFirmware(bool option)
{
    if (option)
    {
        vTaskSuspend(TaskDisplay_Handler);
        panel->del();
        expander->del();
    }
    else
    {
        init();
        vTaskResume(TaskDisplay_Handler);
    }
}

void Display::on_create_lv_label(lv_obj_t **obj,
                                lv_style_t *style,
                                const lv_font_t *font, 
                                lv_align_t align,
                                lv_coord_t x_ofs, 
                                lv_coord_t y_ofs)
{
    lv_style_init(style);
    lv_style_set_text_font(style, font); 
    *obj = lv_label_create(lv_scr_act());
    lv_obj_add_style(*obj, style, 0);  // <--- obj is the label
    lv_obj_align(*obj, align, x_ofs, y_ofs);
    // clear flag
    lv_obj_clear_flag(*obj,LV_OBJ_FLAG_SCROLLABLE);
}

void Display::on_change_lv_label_text(lv_obj_t *obj, String str)
{
    char buf[100];
    lv_snprintf(buf, sizeof(buf), "%s", str);
    lv_label_set_text(obj, buf);
}

void Display::on_create_lv_img(lv_obj_t **obj, 
                                const void *src, 
                                int16_t zoom,
                                lv_align_t align, 
                                lv_coord_t x_ofs, 
                                lv_coord_t y_ofs)
{
    *obj = lv_img_create(lv_scr_act());
    lv_img_set_src(*obj, src);
    if(zoom > -1) lv_img_set_zoom(*obj, zoom);
    lv_obj_align(*obj, align, x_ofs, y_ofs);
    // clear flag
    lv_obj_clear_flag(*obj,LV_OBJ_FLAG_SCROLLABLE);
}

void Display::on_create_lv_line(lv_obj_t **obj,
                                lv_style_t *style,
                                lv_point_t line_points[],
                                lv_align_t align,
                                lv_coord_t x_ofs, 
                                lv_coord_t y_ofs)
{
    lv_style_init(style);
    lv_style_set_line_width(style, 3);
    lv_style_set_line_color(style, lv_color_make(0xB9, 0xB9, 0xB9));
    lv_style_set_line_rounded(style, true);

    /*Create a line and apply the new style*/
    *obj = lv_line_create(lv_scr_act());
    lv_line_set_points(*obj, line_points, 2);     /*Set the points*/
    lv_obj_add_style(*obj, style, 0);
    lv_obj_align(*obj, align, x_ofs, y_ofs);
    // clear flag
    lv_obj_clear_flag(*obj,LV_OBJ_FLAG_SCROLLABLE);
}

