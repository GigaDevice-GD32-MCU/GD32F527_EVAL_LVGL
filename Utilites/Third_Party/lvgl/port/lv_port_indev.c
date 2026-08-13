#include "lv_port_indev.h"

#ifdef RESISTIVE_TOUCH_SCREEN
#include "touch_panel.h"
#else
#include "bsp_i2c_touch.h"
#include "bsp_ts_gt911.h"
#endif

static void touchpad_init(void);
static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
static lv_obj_t * touch_cursor_create(void);

lv_indev_t * indev_touchpad;

#ifdef RESISTIVE_TOUCH_SCREEN
extern int16_t touch_ad_x;
extern int16_t touch_ad_y;
#else
extern uint8_t touch_buf[48];
extern uint8_t clear_flag;
#endif

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;

    touchpad_init();

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    indev_drv.scroll_limit = 30;
    indev_touchpad = lv_indev_drv_register(&indev_drv);
    lv_indev_set_cursor(indev_touchpad, touch_cursor_create());
}

static lv_obj_t * touch_cursor_create(void)
{
    lv_obj_t * cursor = lv_obj_create(lv_scr_act());

    lv_obj_set_size(cursor, LV_HOR_RES * 20 / 1024, LV_VER_RES * 20 / 600);
    lv_obj_set_style_bg_color(cursor, lv_color_hex(0xff0000), 0);
    lv_obj_set_style_radius(cursor, LV_RADIUS_CIRCLE, 0);

    return cursor;
}

static void touchpad_init(void)
{
#ifdef RESISTIVE_TOUCH_SCREEN
    touch_panel_gpio_config();
#else
    gt911_init();
#endif
}

static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
#ifdef RESISTIVE_TOUCH_SCREEN
    ErrStatus err = ERROR;

    LV_UNUSED(indev_drv);

    err = touch_scan();
    if(err == SUCCESS) {
        /* The panel's raw channels are rotated relative to the LCD axes. */
        data->point.x = touch_coordinate_x_get((uint16_t)touch_ad_y);
        data->point.y = touch_coordinate_y_get((uint16_t)touch_ad_x);
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
#else
    ErrStatus err = SUCCESS;
    uint8_t i = 0U;

    LV_UNUSED(indev_drv);

    if(gt911.enable == 0U) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    err &= gt911_readreg(GT911_PRESSED_INFO_REG, touch_buf, 1);
    gt911.pressed_info = touch_buf[0];

    if(gt911.pressed_info & 0x0FU) {
        err &= gt911_readreg(GT911_COORDINATE_REG, touch_buf, 8U * (gt911.pressed_info & 0x0FU));
        for(i = 0; i < (gt911.pressed_info & 0x0FU); i++) {
            gt911.x[i] = ((uint16_t)touch_buf[2 + i * 8] << 8) + touch_buf[1 + i * 8];
            gt911.y[i] = ((uint16_t)touch_buf[4 + i * 8] << 8) + touch_buf[3 + i * 8];
            gt911.s[i] = ((uint16_t)touch_buf[6 + i * 8] << 8) + touch_buf[5 + i * 8];
        }

        gt911.pressed = 2;
        data->point.x = gt911.x[0];
        data->point.y = gt911.y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    if(gt911.pressed_info) {
        err &= gt911_writereg(GT911_PRESSED_INFO_REG, &clear_flag, 1);
    }

    if(err != SUCCESS) {
        data->state = LV_INDEV_STATE_RELEASED;
    }
#endif
}
