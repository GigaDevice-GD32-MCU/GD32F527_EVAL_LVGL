/**
 * @file lv_demo_app.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include <stdio.h>
#include <lv_demos.h>
#include "gd32f527.h"
#include "sys_config.h"
#if SYS_configUSE_FAT32FS
#include "lv_file_browse.h"
#endif
#include "lv_about.h"
#include "lv_port_disp.h"

/*********************
 *      DEFINES
 *********************/
#ifndef SCALE_POS_X
#define SCALE_POS_X(x) ((int32_t)((x) * LV_HOR_RES_MAX / 1024))
#endif
#ifndef SCALE_POS_Y
#define SCALE_POS_Y(y) ((int32_t)((y) * LV_VER_RES_MAX / 600))
#endif

LV_IMAGE_DECLARE(img_background);
LV_IMAGE_DECLARE(img_widget);
LV_IMAGE_DECLARE(img_music);
LV_IMAGE_DECLARE(img_speed);
LV_IMAGE_DECLARE(img_about);
LV_IMAGE_DECLARE(img_file);
LV_IMAGE_DECLARE(img_qrcode);
LV_IMAGE_DECLARE(img_home);
LV_IMAGE_DECLARE(img_home_pressed);
LV_IMAGE_DECLARE(img_gd_logo);
LV_IMAGE_DECLARE(img_picture);
LV_IMAGE_DECLARE(img_setting);
LV_IMAGE_DECLARE(img_temp);
LV_IMAGE_DECLARE(img_mem);

LV_FONT_DECLARE(lv_font_digital_20);
LV_FONT_DECLARE(lv_font_digital_30);

uint8_t g_music_flag = 0;

typedef struct {
    uint16_t year;                                                               /*!< RTC year value: 0x0 - 0x99(BCD format) */
    uint8_t month;                                                              /*!< RTC month value */
    uint8_t day;                                                               /*!< RTC date value: 0x1 - 0x31(BCD format) */
    uint8_t hour;                                                               /*!< RTC hour value */
    uint8_t minute;                                                             /*!< RTC minute value: 0x0 - 0x59(BCD format) */
    uint8_t second;                                                   /*!< RTC time notation */
} rtc_para_struct;


const uint8_t month_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
rtc_para_struct rtc_para;
uint32_t seccount;
lv_obj_t *font_label;
lv_obj_t *font_label_year;
lv_obj_t *calendar;

lv_obj_t *tv_main;       /* main page */
lv_obj_t *img;
lv_obj_t *obj_main;
lv_obj_t *obj_performance;
lv_obj_t *imgbtn_widget;
lv_obj_t *imgbtn_music;
lv_obj_t *imgbtn_speed;
lv_obj_t *imgbtn_about;
lv_obj_t *imgbtn_qrcode;
lv_obj_t *imgbtn_file;
lv_obj_t *imgbtn_picture;
lv_obj_t *imgbtn_setting;

lv_obj_t *cpu_chart;
lv_obj_t *fps_chart;
lv_chart_series_t *cpu_chart_series;
lv_chart_series_t *fps_chart_series;

lv_obj_t *led_main;
lv_obj_t *led_performance;

/* Button labels for language switching */
lv_obj_t *label_widget;
lv_obj_t *label_music;
lv_obj_t *label_speed;
lv_obj_t *label_picture;
lv_obj_t *label_about;
lv_obj_t *label_qrcode;
lv_obj_t *label_file;
lv_obj_t *label_setting;

uint32_t g_fps;
uint32_t g_cpu;
lv_mem_monitor_t g_mon;

static lv_timer_t *g_chart_timer;
static lv_timer_t *g_meter_timer;
static lv_timer_t *g_rtc_timer;
static bool rtc_initialized;

typedef enum {
    APP_DEMO_NONE,
    APP_DEMO_WIDGETS,
    APP_DEMO_MUSIC,
    APP_DEMO_BENCHMARK,
} app_demo_t;

static app_demo_t active_demo = APP_DEMO_NONE;
static bool return_home_pending;

/* LVGL v9 demos have no close function. The widgets/music demos are created in
 * demo_cont, the benchmark runs on its own screen (bench_scr). A home button is
 * placed on lv_layer_top() to return to the main page. */
static lv_obj_t *app_scr;
static lv_obj_t *demo_cont;
static lv_obj_t *bench_scr;
static lv_obj_t *demo_home_btn;

extern uint16_t back_to_home;

extern float g_temperature;
extern void update_main_page_language(void);
extern void update_performance_labels(void);
extern void lv_demo_qrcode(lv_obj_t *parent);
extern void lv_demo_setting(lv_obj_t * parent);
extern void lv_demo_picture(lv_obj_t * parent);
void lv_demo_app(void);
void lv_demo_app_return_home(void);

/* 停掉所有持有主页对象的定时器，并把即将被销毁的对象指针置空。
 * 返回主页时会对 lv_screen_active() 执行 lv_obj_clean()，
 * 如果定时器继续访问这些已释放的对象就会出现偶发性卡死/硬件异常。*/
static void app_stop_page_timers(void);

/* Close the running demo and delete everything it created */
static void app_close_active_demo(void)
{
    if(bench_scr != NULL) {
        /* The benchmark scene timer uses the benchmark screen as user data */
        lv_timer_t *timer = lv_timer_get_next(NULL);
        while(timer != NULL) {
            lv_timer_t *next = lv_timer_get_next(timer);
            if(lv_timer_get_user_data(timer) == bench_scr) {
                lv_timer_delete(timer);
            }
            timer = next;
        }
    }

    /* Delete all running animations of the demo */
    lv_anim_delete(NULL, NULL);

    /* Home button and the benchmark title are on the top layer */
    lv_obj_clean(lv_layer_top());
    lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);
    lv_obj_set_clickable(lv_layer_top(), false);
    demo_home_btn = NULL;

    if(bench_scr != NULL) {
        lv_screen_load(app_scr);
        lv_obj_delete(bench_scr);
        bench_scr = NULL;
    }

    /* main page and the demo container are rebuilt by lv_demo_app() */
    lv_obj_clean(lv_screen_active());
    demo_cont = NULL;
}

static void return_home_async(void *data)
{
    LV_UNUSED(data);

    return_home_pending = false;

    app_stop_page_timers();
    app_close_active_demo();

    active_demo = APP_DEMO_NONE;
    back_to_home = 1;
    lv_demo_app();
}

static void demo_home_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    lv_demo_app_return_home();
}

/* Create the home button of the demos on the top layer */
static void demo_home_btn_create(void)
{
    demo_home_btn = lv_image_create(lv_layer_top());
    lv_image_set_src(demo_home_btn, &img_home);
    lv_obj_set_clickable(demo_home_btn, true);
    lv_obj_set_ext_click_area(demo_home_btn, 5);
    lv_obj_align(demo_home_btn, LV_ALIGN_TOP_RIGHT, -3, 3);
    lv_obj_add_event_cb(demo_home_btn, demo_home_event_cb, LV_EVENT_CLICKED, NULL);
}

/* Create a full screen container on the active screen for the widgets/music demo */
static lv_obj_t *demo_cont_create(void)
{
    demo_cont = lv_obj_create(lv_screen_active());
    lv_obj_remove_style_all(demo_cont);
    lv_obj_set_size(demo_cont, lv_pct(100), lv_pct(100));
    lv_obj_set_scrollable(demo_cont, false);

    return demo_cont;
}

void lv_demo_app_return_home(void)
{
    if(active_demo != APP_DEMO_NONE && !return_home_pending) {
        return_home_pending = true;
        lv_async_call(return_home_async, NULL);
    }
}

/* Language switching function */
extern void update_main_page_language(void);
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
bool lv_img_bin_load(const char *path, uint8_t *buf, uint32_t expected_size)
{
if(!path || !buf || expected_size == 0) return false;

    lv_fs_file_t f;
    lv_fs_res_t res = lv_fs_open(&f, path, LV_FS_MODE_RD);
    if(res != LV_FS_RES_OK) return false;

    /* Get total file size first */
    uint32_t size_total = 0;
    res = lv_fs_seek(&f, 0, LV_FS_SEEK_END);
    if(res != LV_FS_RES_OK) { lv_fs_close(&f); return false; }

    res = lv_fs_tell(&f, &size_total);
    if(res != LV_FS_RES_OK) { lv_fs_close(&f); return false; }

    /* Move to payload start (skip 4 bytes header if present) */
    const uint32_t skip = 4;
    if(size_total < skip) { lv_fs_close(&f); return false; }
    uint32_t size = size_total - skip;

    res = lv_fs_seek(&f, (int32_t)skip, LV_FS_SEEK_SET);
    if(res != LV_FS_RES_OK) { lv_fs_close(&f); return false; }

    if(size > expected_size) {
        /* Buffer too small for file */
        lv_fs_close(&f);
        return false;
    }

    uint32_t read_total = 0;
    while(read_total < size) {
        uint32_t to_read = size - read_total;
        if(to_read > 1024) to_read = 1024; /* chunk read */
        uint32_t br = 0;
        res = lv_fs_read(&f, buf + read_total, to_read, &br);
        if(res != LV_FS_RES_OK) { lv_fs_close(&f); return false; }
        if(br == 0) break; /* EOF */
        read_total += br;
    }

    lv_fs_close(&f);
    return read_total == size;
}

static void widget_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(active_demo != APP_DEMO_NONE) return;

    back_to_home = 0;
    active_demo = APP_DEMO_WIDGETS;

    lv_obj_set_hidden(led_main, true);
    lv_obj_set_hidden(led_performance, true);
    lv_obj_set_hidden(tv_main, true);

    app_stop_page_timers();

    lv_demo_args_t args;
    lv_demo_args_init(&args);
    args.parent = demo_cont_create();
    lv_demo_widgets_with_args(&args);   /* The demo creates its own home button (as in LVGL v8) */
}

static void music_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(active_demo != APP_DEMO_NONE) return;

    back_to_home = 0;
    active_demo = APP_DEMO_MUSIC;

    lv_obj_set_hidden(led_main, true);
    lv_obj_set_hidden(led_performance, true);
    lv_obj_set_hidden(tv_main, true);

    app_stop_page_timers();

    lv_demo_args_t args;
    lv_demo_args_init(&args);
    args.parent = demo_cont_create();
    lv_demo_music_with_args(&args);     /* The demo creates its own home button (as in LVGL v8) */
}

/* Print the benchmark result to the UART (printf is retargeted to USART0) */
static void benchmark_end_cb(const lv_demo_benchmark_summary_t *summary)
{
    const lv_demo_benchmark_scene_dsc_t *scenes = summary->scenes;
    uint32_t i;

    printf("\r\n[BENCHMARK] Summary (LVGL %d.%d.%d)\r\n",
           LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, LVGL_VERSION_PATCH);
    printf("Name, Avg. CPU(%%), Avg. FPS, Avg. time(ms), Render time(ms), Flush time(ms)\r\n");

    if(summary->valid_scene_cnt < 1) {
        printf("All scenes avg., N/A, N/A, N/A, N/A, N/A\r\n");
    }
    else {
        uint32_t render_time = (uint32_t)(summary->total_avg_render_time / summary->valid_scene_cnt);
        uint32_t flush_time = (uint32_t)(summary->total_avg_flush_time / summary->valid_scene_cnt);
        printf("All scenes avg., %lu, %lu, %lu, %lu, %lu\r\n",
               (unsigned long)(summary->total_avg_cpu / summary->valid_scene_cnt),
               (unsigned long)(summary->total_avg_fps / summary->valid_scene_cnt),
               (unsigned long)(render_time + flush_time),
               (unsigned long)render_time, (unsigned long)flush_time);
    }

    for(i = 0; scenes[i].create_cb != NULL; i++) {
        uint32_t cnt = scenes[i].measurement_cnt;
        if(cnt == 0) {
            printf("%s, N/A, N/A, N/A, N/A, N/A\r\n", scenes[i].name);
        }
        else {
            uint32_t render_time = scenes[i].render_avg_time / cnt;
            uint32_t flush_time = scenes[i].flush_avg_time / cnt;
            printf("%s, %lu, %lu, %lu, %lu, %lu\r\n", scenes[i].name,
                   (unsigned long)(scenes[i].cpu_avg_usage / cnt),
                   (unsigned long)(scenes[i].fps_avg / cnt),
                   (unsigned long)(render_time + flush_time),
                   (unsigned long)render_time, (unsigned long)flush_time);
        }
    }
    printf("[BENCHMARK] End\r\n");

    /* Keep the on-screen result table */
    lv_demo_benchmark_summary_display(summary);
}

static void start_benchmark_cb(lv_timer_t * t)
{
    LV_UNUSED(t);

    lv_obj_set_hidden(tv_main, true);

    /* 主页对象在返回时会被删除，所以必须先把引用它们的定时器全部停掉。*/
    app_stop_page_timers();

    /* The benchmark modifies and cleans the active screen, run it on its own screen */
    bench_scr = lv_obj_create(NULL);
    lv_screen_load(bench_scr);

    lv_demo_benchmark_set_end_cb(benchmark_end_cb);
    lv_demo_benchmark();
    demo_home_btn_create();
}

static void speed_event_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if(active_demo != APP_DEMO_NONE) return;

    back_to_home = 0;
    active_demo = APP_DEMO_BENCHMARK;
    lv_timer_t * timer = lv_timer_create(start_benchmark_cb, 1, NULL);
    if(timer) {
        lv_timer_set_repeat_count(timer, 1);
    }
    else {
        active_demo = APP_DEMO_NONE;
    }
}

static void qrcode_event_cb(lv_event_t *e)
{
    lv_obj_set_hidden(led_main, true);
    lv_obj_set_hidden(led_performance, true);
    lv_demo_qrcode(lv_event_get_user_data(e));
    lv_obj_set_hidden(obj_performance, true);
}

static void setting_event_cb(lv_event_t *e)
{
    lv_demo_setting(lv_event_get_user_data(e));
    lv_obj_set_hidden(obj_performance, true);
}

static void picture_event_cb(lv_event_t *e)
{
    lv_obj_set_hidden(led_main, true);
    lv_obj_set_hidden(led_performance, true);
    lv_demo_picture(lv_event_get_user_data(e));
    lv_obj_set_hidden(obj_performance, true);
}

static void update_demo_performance_stats(void)
{
    static uint32_t sample_index;
    static uint32_t random_state = 0x527F527U;
    static int32_t cpu_filtered = 18;
    uint32_t target_cpu;
    int32_t noise;

    random_state = random_state * 1664525U + 1013904223U;
    noise = (int32_t)((random_state >> 28) & 0x0FU) - 7;

    /* Emulate the idle, active, and occasional busy phases of a UI workload. */
    switch((sample_index / 12U) % 5U) {
    case 0:
        target_cpu = 13U;
        break;
    case 1:
        target_cpu = 24U;
        break;
    case 2:
        target_cpu = 39U;
        break;
    case 3:
        target_cpu = 29U;
        break;
    default:
        target_cpu = 17U;
        break;
    }

    if(sample_index % 29U == 0U) target_cpu += 22U;

    cpu_filtered += ((int32_t)target_cpu + noise - cpu_filtered) / 2;
    if(cpu_filtered < 5) cpu_filtered = 5;
    if(cpu_filtered > 85) cpu_filtered = 85;

    g_cpu = (uint32_t)cpu_filtered;
    g_fps = 60U - g_cpu / 4U + ((random_state >> 24) & 0x03U);
    if(g_fps < 30U) g_fps = 30U;

    sample_index++;
}

static void update_chart(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if(cpu_chart == NULL || fps_chart == NULL) return;

    update_demo_performance_stats();
    lv_chart_set_next_value(cpu_chart, cpu_chart_series, g_cpu);
    lv_chart_set_next_value(fps_chart, fps_chart_series, g_fps);
}

static void calendar_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_current_target_obj(e);

    if(code == LV_EVENT_VALUE_CHANGED) {
        lv_calendar_date_t date;
        if(lv_calendar_get_pressed_date(obj, &date) == LV_RESULT_OK) {

            rtc_para.year = date.year;
            rtc_para.month = date.month;
            rtc_para.day = date.day;
            lv_label_set_text_fmt(font_label_year, "#FFFFFF %d/%02d/%02d", rtc_para.year, rtc_para.month, rtc_para.day);

            lv_obj_delete(calendar);
            calendar = NULL;
            lv_obj_set_clickable(lv_layer_top(), false);
            lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);
        }

    }
}

static void year_label_evevt_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        lv_obj_set_clickable(lv_layer_top(), true);
        calendar = lv_calendar_create(lv_layer_top());
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_50, 0);
        lv_obj_set_style_bg_color(lv_layer_top(), lv_palette_main(LV_PALETTE_GREY), 0);

        lv_obj_set_size(calendar, SCALE_POS_X(400), SCALE_POS_Y(400));
        lv_obj_align_to(calendar, font_label_year, LV_ALIGN_LEFT_MID, SCALE_POS_X(0), SCALE_POS_Y(0));
        static lv_calendar_date_t highlighted_days[1];       /*Only its pointer will be saved so should be static*/
        highlighted_days[0].year = rtc_para.year;
        highlighted_days[0].month = rtc_para.month;
        highlighted_days[0].day = rtc_para.day;
        lv_calendar_set_highlighted_dates(calendar, highlighted_days, 1);

        lv_calendar_set_month_shown(calendar, rtc_para.year, rtc_para.month);
        lv_calendar_add_header_dropdown(calendar);
        lv_calendar_add_header_arrow(calendar);
        lv_obj_add_event_cb(calendar, calendar_event_cb, LV_EVENT_ALL, NULL);
    }
}



uint8_t is_leap_year(uint16_t year)
{
    if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1;
    } else {
        return 0;
    }
}

uint32_t rtc_seccount_get(rtc_para_struct *rtc_initpara_struct)
{
    uint16_t t;
    uint32_t sec = 0;

    if(rtc_initpara_struct->year < 1970 || rtc_initpara_struct->year > 2099) {
        return 1;
    }

    for(t = 1970; t < rtc_initpara_struct->year; t++) {
        if(is_leap_year(t)) {
            sec += 31622400;
        } else {
            sec += 31536000;
        }
    }
    //rtc_initpara_struct->month -= 1;

    for(t = 0; t < rtc_initpara_struct->month - 1; t++) {
        sec += (uint32_t)month_table[t] * 86400;
        if(is_leap_year(rtc_initpara_struct->year) && t == 1) {
            sec += 86400;
        }
    }

    sec += (uint32_t)(rtc_initpara_struct->day - 1) * 86400;
    sec += (uint32_t)rtc_initpara_struct->hour * 3600;
    sec += (uint32_t)rtc_initpara_struct->minute * 60;
    sec += rtc_initpara_struct->second;
    //rtc_counter_set(sec);
    return sec;
}

void rtc_time_get(uint32_t timevar, rtc_para_struct *rtc_initpara_struct)
{
    static uint16_t daycnt = 0;
    uint32_t temp = 0;
    uint16_t temp1 = 0;
    temp = timevar / 86400;

    if(daycnt != temp) {
        daycnt = temp;
        temp1 = 1970;

        while(temp >= 365) {
            if(is_leap_year(temp1)) {
                if(temp >= 366) {
                    temp -= 366;
                } else {
                    break;
                }
            } else {
                temp -= 365;
            }
            temp1++;
        }

        rtc_initpara_struct->year = temp1;
        temp1 = 0;

        while(temp >= 28) {
            if(is_leap_year(rtc_initpara_struct->year) && temp1 == 1) {
                if(temp >= 29) {
                    temp -= 29;
                } else {
                    break;
                }
            } else {
                if(temp >= month_table[temp1]) {
                    temp -= month_table[temp1];
                } else {
                    break;
                }
            }
            temp1++;
        }
        rtc_initpara_struct->month = temp1 + 1;
        rtc_initpara_struct->day = temp + 1;
    }

    temp = timevar % 86400;
    rtc_initpara_struct->hour = temp / 3600;
    rtc_initpara_struct->minute = (temp % 3600) / 60;
    rtc_initpara_struct->second = (temp % 3600) % 60;
}

static void rtc_timer(lv_timer_t *timer)
{
    seccount = rtc_seccount_get(&rtc_para);
    rtc_seccount_get(&rtc_para);
    seccount++;
    rtc_time_get(seccount, &rtc_para);

    if(font_label) {
        lv_label_set_text_fmt(font_label, "#FFFFFF %02d:%02d:%02d", rtc_para.hour, rtc_para.minute, rtc_para.second);
    }
    if(font_label_year) {
        lv_label_set_text_fmt(font_label_year, "#FFFFFF %d/%02d/%02d", rtc_para.year, rtc_para.month, rtc_para.day);
    }

}
static lv_obj_t *mem_meter;

static lv_obj_t *temp_mem_meter;
static lv_obj_t *indic_temp;    /* needle line of the temperature scale */
lv_obj_t * label_temp;

static void tv_main_event_cb(lv_event_t *e)
{
    if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        if(0 == lv_tabview_get_tab_active(tv_main)) {
            lv_led_on(led_main);
            lv_led_off(led_performance);
            back_to_home = 1;
        } else if(1 == lv_tabview_get_tab_active(tv_main)) {
            lv_led_off(led_main);
            lv_led_on(led_performance);
        }
    }
}

//lv_event_code_t testtt;

/* arcs of the memory scale */
static lv_obj_t * indic_total_size;
lv_obj_t * label_total_size;
static lv_obj_t * indic_mem_used;
lv_obj_t * label_mem_used;
static lv_obj_t * indic_frag;
lv_obj_t * label_frag;
//static void scroll_begin_event(lv_event_t * e)
//{
//    /*Disable the scroll animations. Triggered when a tab button is clicked */
////    if(lv_event_get_code(e) == LV_EVENT_SCROLL_BEGIN) {
//    testtt = lv_event_get_code(e);
//    if( LV_EVENT_DRAW_POST_END == lv_event_get_code(e))
//        back_to_home = 0;
////    }
//}

//lv_event_code_t testtt2;
//static void scroll_begin_event2(lv_event_t * e)
//{
//    /*Disable the scroll animations. Triggered when a tab button is clicked */
////    if(lv_event_get_code(e) == LV_EVENT_SCROLL_BEGIN) {
//    testtt2 = lv_event_get_code(e);
//    if( LV_EVENT_DRAW_POST_END == lv_event_get_code(e))
//        back_to_home = 1;
////    }
//}


/* lv_scale_set_line_needle_value() aligns the line to the top-left of the content area,
 * but calculates the points from the full scale size, so the needle is shifted by the
 * padding. Move the line back to the outer top-left corner to start from the center. */
static void temp_needle_set_value(int32_t value)
{
    lv_scale_set_line_needle_value(temp_mem_meter, indic_temp, -12, value);
    lv_obj_align(indic_temp, LV_ALIGN_TOP_LEFT,
                 -lv_obj_get_style_pad_left(temp_mem_meter, LV_PART_MAIN),
                 -lv_obj_get_style_pad_top(temp_mem_meter, LV_PART_MAIN));
}

static void update_meter_box(lv_timer_t *timer)
{
    LV_UNUSED(timer);

    if(mem_meter == NULL || temp_mem_meter == NULL) return;

    lv_mem_monitor(&g_mon);
    lv_arc_set_value(indic_total_size, 100);
    lv_arc_set_value(indic_mem_used, g_mon.used_pct);
    lv_arc_set_value(indic_frag, g_mon.frag_pct);

    /* Only refresh the values here. Fonts/texts of the other labels are updated by
     * update_main_page_language() when the language changes. */
    update_performance_labels();
    
    temp_needle_set_value((int32_t)g_temperature);
}

/* Create an arc on a round scale, it replaces the arc indicator of the v8 lv_meter.
 * r_mod: radius modifier relative to the scale's content area */
static lv_obj_t * scale_arc_create(lv_obj_t * scale, int32_t scale_size, int32_t r_mod, int32_t width,
                                   lv_color_t color, int32_t min, int32_t max, int32_t value)
{
    int32_t arc_size = scale_size - 2 * lv_obj_get_style_pad_left(scale, LV_PART_MAIN) + 2 * r_mod;

    lv_obj_t * arc = lv_arc_create(scale);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_clickable(arc, false);
    lv_obj_set_style_pad_all(arc, 0, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(arc, width, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, color, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, false, LV_PART_INDICATOR);
    lv_obj_set_size(arc, arc_size, arc_size);
    lv_obj_center(arc);

    lv_arc_set_rotation(arc, lv_scale_get_rotation(scale));
    lv_arc_set_bg_angles(arc, 0, lv_scale_get_angle_range(scale));
    lv_arc_set_range(arc, min, max);
    lv_arc_set_value(arc, value);

    return arc;
}

static void create_meter_box(lv_obj_t * parent, const char * title, const char * text1, const char * text2,
                                   const char * text3)
{
    static lv_style_t style_bullet;
    static bool style_inited = false;

    /* 只初始化一次：lv_style_init() 会丢弃之前已分配的属性数组，
     * 每次重建主页都会泄漏一次 LVGL 堆 */
    if(!style_inited) {
        style_inited = true;
        lv_style_init(&style_bullet);
        lv_style_set_border_width(&style_bullet, 0);
        lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);
    }


    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_width(cont, 140);
    lv_obj_set_flex_grow(cont, 1);
    lv_obj_set_pos(cont, SCALE_POS_X(165), 500);

    lv_obj_t * bullet1 = lv_obj_create(cont);
    lv_obj_set_size(bullet1, SCALE_POS_X(13), SCALE_POS_Y(13));
    lv_obj_remove_style(bullet1, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet1, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet1, lv_palette_main(LV_PALETTE_RED), 0);
    label_total_size = lv_label_create(cont);
    lv_label_set_text(label_total_size, text1);

    lv_obj_t * bullet2 = lv_obj_create(cont);
    lv_obj_set_size(bullet2, SCALE_POS_X(13), SCALE_POS_Y(13));
    lv_obj_remove_style(bullet2, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet2, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet2, lv_palette_main(LV_PALETTE_GREEN), 0);
    label_mem_used = lv_label_create(cont);
    lv_label_set_text(label_mem_used, text2);

    lv_obj_t * bullet3 = lv_obj_create(cont);
    lv_obj_set_size(bullet3, SCALE_POS_X(13), SCALE_POS_Y(13));
    lv_obj_remove_style(bullet3,  NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet3, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet3, lv_palette_main(LV_PALETTE_BLUE), 0);
    label_frag = lv_label_create(cont);
    lv_label_set_text(label_frag, text3);

    {
        static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
//        static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
//        lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);
//        lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(bullet2, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(bullet3, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 2, 1);
        lv_obj_set_grid_cell(label_total_size, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(label_mem_used, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(label_frag, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 2, 1);
    }

}

static void create_temp_box(lv_obj_t * parent, const char * text1)
{
    static lv_style_t style_bullet;
    static bool style_inited = false;

    if(!style_inited) {
        style_inited = true;
        lv_style_init(&style_bullet);
        lv_style_set_border_width(&style_bullet, 0);
        lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);
    }


    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_height(cont, LV_SIZE_CONTENT);
    lv_obj_set_width(cont, 140);
    lv_obj_set_flex_grow(cont, 1);
    lv_obj_set_pos(cont, SCALE_POS_X(165+465), 500);

    lv_obj_t * bullet1 = lv_obj_create(cont);
    lv_obj_set_size(bullet1, SCALE_POS_X(13), SCALE_POS_Y(13));
    lv_obj_remove_style(bullet1, NULL, LV_PART_SCROLLBAR);
    lv_obj_add_style(bullet1, &style_bullet, 0);
    lv_obj_set_style_bg_color(bullet1, lv_palette_main(LV_PALETTE_GREY), 0);
    label_temp = lv_label_create(cont);
    lv_label_set_text(label_temp, text1);
    update_main_page_language();

    {
        static int32_t grid_col_dsc[] = {LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
//        static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        static int32_t grid_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
        lv_obj_set_grid_dsc_array(cont, grid_col_dsc, grid_row_dsc);
//        lv_obj_set_grid_cell(title_label, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 0, 1);
//        lv_obj_set_grid_cell(meter, LV_GRID_ALIGN_START, 0, 2, LV_GRID_ALIGN_START, 1, 1);
        lv_obj_set_grid_cell(bullet1, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_set_grid_cell(label_temp, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    }

}
/**********************
 *      MACROS
 **********************/

static void app_stop_page_timers(void)
{
    if(g_chart_timer) {
        lv_timer_delete(g_chart_timer);
        g_chart_timer = NULL;
    }
    if(g_meter_timer) {
        lv_timer_delete(g_meter_timer);
        g_meter_timer = NULL;
    }
    if(g_rtc_timer) {
        lv_timer_delete(g_rtc_timer);
        g_rtc_timer = NULL;
    }

    font_label = NULL;
    font_label_year = NULL;

    label_widget = NULL;
    label_music = NULL;
    label_speed = NULL;
    label_picture = NULL;
    label_about = NULL;
    label_qrcode = NULL;
    label_file = NULL;
    label_setting = NULL;

    label_total_size = NULL;
    label_mem_used = NULL;
    label_frag = NULL;
    label_temp = NULL;

    cpu_chart = NULL;
    fps_chart = NULL;
    mem_meter = NULL;
    temp_mem_meter = NULL;
    indic_total_size = NULL;
    indic_mem_used = NULL;
    indic_frag = NULL;
    indic_temp = NULL;

    tv_main = NULL;
    obj_main = NULL;
    obj_performance = NULL;
    led_main = NULL;
    led_performance = NULL;

    if(calendar) {
        lv_obj_delete(calendar);
        calendar = NULL;
        lv_obj_set_clickable(lv_layer_top(), false);
        lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);
    }
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_demo_app(void)
{
    if(app_scr == NULL) {
        app_scr = lv_screen_active();
    }

#if LV_USE_PERF_MONITOR
    /* The perf monitor is only needed for the benchmark data, the benchmark
     * shows the values in its own title, so hide the default overlay label */
    lv_sysmon_hide_performance(lv_display_get_default());
#endif

//    static lv_obj_t *tv_main;       /* main page */
    tv_main = lv_tabview_create(lv_screen_active());
    /* No tab buttons, the pages are switched by swiping */
    lv_tabview_set_tab_bar_size(tv_main, 0);
    lv_obj_set_hidden(lv_tabview_get_tab_bar(tv_main), true);
    lv_obj_add_event_cb(tv_main, tv_main_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /*Create a LED and switch it OFF*/
    led_main  = lv_led_create(lv_screen_active());
    lv_obj_set_size(led_main, 10, 10);
    lv_obj_align(led_main, LV_ALIGN_CENTER, SCALE_POS_X(-30), SCALE_POS_Y(250));
    lv_led_on(led_main);
//    lv_obj_add_event_cb(led_main, scroll_begin_event2, LV_EVENT_ALL, NULL);

    /*Copy the previous LED and set a brightness*/
    led_performance  = lv_led_create(lv_screen_active());
    lv_obj_set_size(led_performance, 10, 10);
    lv_obj_align(led_performance, LV_ALIGN_CENTER, SCALE_POS_X(30), SCALE_POS_Y(250));
//    lv_led_set_color(led2, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
    lv_led_off(led_performance);
//    lv_obj_add_event_cb(led_performance, scroll_begin_event, LV_EVENT_ALL, NULL);


    obj_main = lv_tabview_add_tab(tv_main, "Main page");
    obj_performance = lv_tabview_add_tab(tv_main, "Performance");

    lv_obj_set_width(obj_main, LV_HOR_RES_MAX);
    lv_obj_set_height(obj_main, LV_VER_RES_MAX);

    lv_obj_set_scrollbar_mode(obj_main, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_top(obj_main, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(obj_main, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(obj_main, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(obj_main, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj_main, 0, LV_PART_MAIN);

    lv_obj_set_width(obj_performance, LV_HOR_RES_MAX);
    lv_obj_set_height(obj_performance, LV_VER_RES_MAX);

    lv_obj_set_scrollable(obj_main, false);
    lv_obj_set_scrollbar_mode(obj_performance, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_top(obj_performance, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(obj_performance, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_left(obj_performance, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(obj_performance, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj_performance, 0, LV_PART_MAIN);

    img = lv_image_create(obj_main);
    lv_image_set_src(img, &img_background);

    imgbtn_widget = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_widget, &img_widget);
    lv_obj_set_clickable(imgbtn_widget, true);
//    lv_obj_set_size(imgbtn_widget, 100, 100);
    lv_obj_set_pos(imgbtn_widget, SCALE_POS_X(390), SCALE_POS_Y(207));

    imgbtn_music = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_music, &img_music);
    lv_obj_set_clickable(imgbtn_music, true);
//    lv_obj_set_size(imgbtn_music, 100, 100);
    lv_obj_set_pos(imgbtn_music, SCALE_POS_X(524), SCALE_POS_Y(207));

    imgbtn_speed = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_speed, &img_speed);
    lv_obj_set_clickable(imgbtn_speed, true);
//    lv_obj_set_size(imgbtn_speed, 100, 100);
    lv_obj_set_pos(imgbtn_speed, SCALE_POS_X(657), SCALE_POS_Y(207));

    imgbtn_picture = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_picture, &img_picture);
    lv_obj_set_clickable(imgbtn_picture, true);
//    lv_obj_set_size(imgbtn_file, 100, 100);
    lv_obj_set_pos(imgbtn_picture, SCALE_POS_X(783), SCALE_POS_Y(207));

    imgbtn_about = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_about, &img_about);
    lv_obj_set_clickable(imgbtn_about, true);
//    lv_obj_set_size(imgbtn_about, 100, 100);
    lv_obj_set_pos(imgbtn_about, SCALE_POS_X(390), SCALE_POS_Y(350));

    imgbtn_qrcode = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_qrcode, &img_qrcode);
    lv_obj_set_clickable(imgbtn_qrcode, true);
//    lv_obj_set_size(imgbtn_qrcode, 100, 100);
    lv_obj_set_pos(imgbtn_qrcode, SCALE_POS_X(524), SCALE_POS_Y(350));

    imgbtn_file = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_file, &img_file);
    lv_obj_set_clickable(imgbtn_file, true);
//    lv_obj_set_size(imgbtn_file, 100, 100);
    lv_obj_set_pos(imgbtn_file, SCALE_POS_X(657), SCALE_POS_Y(350));

    imgbtn_setting = lv_image_create(obj_main);
    lv_image_set_src(imgbtn_setting, &img_setting);
    lv_obj_set_clickable(imgbtn_setting, true);
//    lv_obj_set_size(imgbtn_file, 100, 100);
    lv_obj_set_pos(imgbtn_setting, SCALE_POS_X(783), SCALE_POS_Y(350));

    // /* Add labels under each button */
    label_widget = lv_label_create(obj_main);
    lv_label_set_text(label_widget, "Widget");
    lv_obj_set_style_text_font(label_widget, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_widget, imgbtn_widget, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);


    label_music = lv_label_create(obj_main);
    lv_label_set_text(label_music, "Music");
    lv_obj_set_style_text_font(label_music, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_music, imgbtn_music, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label_speed = lv_label_create(obj_main);
    lv_label_set_text(label_speed, "Benchmark");
    lv_obj_set_style_text_font(label_speed, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_speed, imgbtn_speed, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label_picture = lv_label_create(obj_main);
    lv_label_set_text(label_picture, "Picture");
    lv_obj_set_style_text_font(label_picture, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_picture, imgbtn_picture, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label_about = lv_label_create(obj_main);
    lv_label_set_text(label_about, "About");
    lv_obj_set_style_text_font(label_about, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_about, imgbtn_about, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label_qrcode = lv_label_create(obj_main);
    lv_label_set_text(label_qrcode, "QRCode");
    lv_obj_set_style_text_font(label_qrcode, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_qrcode, imgbtn_qrcode, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label_file = lv_label_create(obj_main);
    lv_label_set_text(label_file, "File");
    lv_obj_set_style_text_font(label_file, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_file, imgbtn_file, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    label_setting = lv_label_create(obj_main);
    lv_label_set_text(label_setting, "Setting");
    lv_obj_set_style_text_font(label_setting, &lv_font_montserrat_10, 0);
    lv_obj_align_to(label_setting, imgbtn_setting, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

    //Write codes CPU usage chart
    cpu_chart = lv_chart_create(obj_performance);

    lv_obj_set_pos(cpu_chart, SCALE_POS_X(7 + 75), 19);
    lv_obj_set_size(cpu_chart, SCALE_POS_X(1004 - 75), 155);
    lv_chart_set_type(cpu_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_axis_range(cpu_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_div_line_count(cpu_chart, 6, 12);
    cpu_chart_series = lv_chart_add_series(cpu_chart, lv_color_make(0x50, 0x0A, 0x88), LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_point_count(cpu_chart, 20);

    //Write codes FPS chart
    fps_chart = lv_chart_create(obj_performance);

    lv_obj_set_pos(fps_chart, SCALE_POS_X(7 + 75), 198);
    lv_obj_set_size(fps_chart, SCALE_POS_X(1004 - 75), 155);
    lv_chart_set_type(fps_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_axis_range(fps_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 100);
    lv_chart_set_div_line_count(fps_chart, 6, 12);
    fps_chart_series = lv_chart_add_series(fps_chart, lv_palette_main(LV_PALETTE_ORANGE), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_set_point_count(fps_chart, 20);

    for(uint32_t index = 0; index < 20U; index++) {
        update_demo_performance_stats();
        lv_chart_set_next_value(cpu_chart, cpu_chart_series, g_cpu);
        lv_chart_set_next_value(fps_chart, fps_chart_series, g_fps);
    }

    //Write codes CPU usage label
    lv_obj_t *cpu_label = lv_label_create(obj_performance);
    lv_obj_set_style_text_font(cpu_label, LV_FONT_DEFAULT, 0);
    lv_label_set_recolor(cpu_label, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(cpu_label, "#F12F12 CPU usage(%)");

    lv_label_set_long_mode(cpu_label, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_pos(cpu_label, SCALE_POS_X(21), 2);
    //Write codes FPS label
    lv_obj_t *fps_label = lv_label_create(obj_performance);
    lv_obj_set_style_text_font(fps_label, LV_FONT_DEFAULT, 0);
    lv_label_set_recolor(fps_label, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(fps_label, "#F12F12 FPS");

    lv_label_set_long_mode(fps_label, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_pos(fps_label, SCALE_POS_X(21), 182);

    g_chart_timer = lv_timer_create(update_chart, 500, NULL);

    /* lv_meter was removed in LVGL v9, the gauges are built with lv_scale + lv_arc */
    int32_t meter_size = SCALE_POS_X(300);

    /* Memory gauge */
    mem_meter = lv_scale_create(obj_performance);
    lv_obj_set_pos(mem_meter, SCALE_POS_X(130), 357);
    lv_obj_set_size(mem_meter, meter_size, meter_size);
    lv_obj_set_style_pad_all(mem_meter, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(mem_meter, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(mem_meter, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mem_meter, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(mem_meter, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_arc_width(mem_meter, 0, LV_PART_MAIN);

    lv_scale_set_mode(mem_meter, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_total_tick_count(mem_meter, 6);
    lv_scale_set_major_tick_every(mem_meter, 1);
    lv_scale_set_label_show(mem_meter, true);
    lv_scale_set_range(mem_meter, 0, 100);
    lv_scale_set_angle_range(mem_meter, 270);
    lv_scale_set_rotation(mem_meter, 90);
    lv_obj_set_style_length(mem_meter, 10, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(mem_meter, 2, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(mem_meter, lv_color_hex3(0xeee), LV_PART_INDICATOR);

    indic_total_size = scale_arc_create(mem_meter, meter_size, 0, 5, lv_palette_main(LV_PALETTE_RED), 0, 100, 100);
    indic_mem_used = scale_arc_create(mem_meter, meter_size, -5, 5, lv_palette_main(LV_PALETTE_GREEN), 0, 100, 20);
    indic_frag = scale_arc_create(mem_meter, meter_size, -10, 5, lv_palette_main(LV_PALETTE_BLUE), 0, 100, 80);

    /* Temperature gauge */
    static lv_style_t style_sec_cold;
    static lv_style_t style_sec_hot;
    static bool sec_styles_inited = false;

    if(!sec_styles_inited) {
        sec_styles_inited = true;

        lv_style_init(&style_sec_cold);
        lv_style_set_arc_color(&style_sec_cold, lv_palette_main(LV_PALETTE_BLUE));
        lv_style_set_arc_width(&style_sec_cold, 1);
        lv_style_set_line_color(&style_sec_cold, lv_palette_main(LV_PALETTE_BLUE));

        lv_style_init(&style_sec_hot);
        lv_style_set_arc_color(&style_sec_hot, lv_palette_main(LV_PALETTE_RED));
        lv_style_set_arc_width(&style_sec_hot, 1);
        lv_style_set_line_color(&style_sec_hot, lv_palette_main(LV_PALETTE_RED));
    }

    temp_mem_meter = lv_scale_create(obj_performance);
    lv_obj_set_pos(temp_mem_meter, SCALE_POS_X(588), 357);
    lv_obj_set_size(temp_mem_meter, meter_size, meter_size);
    lv_obj_set_style_pad_all(temp_mem_meter, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(temp_mem_meter, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(temp_mem_meter, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(temp_mem_meter, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(temp_mem_meter, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_arc_width(temp_mem_meter, 0, LV_PART_MAIN);

    lv_scale_set_mode(temp_mem_meter, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_total_tick_count(temp_mem_meter, 31);
    lv_scale_set_major_tick_every(temp_mem_meter, 5);
    lv_scale_set_label_show(temp_mem_meter, true);
    lv_scale_set_range(temp_mem_meter, -40, 105);
    lv_scale_set_angle_range(temp_mem_meter, 270);
    lv_scale_set_rotation(temp_mem_meter, 135);
    /* minor ticks */
    lv_obj_set_style_length(temp_mem_meter, 6, LV_PART_ITEMS);
    lv_obj_set_style_line_width(temp_mem_meter, 1, LV_PART_ITEMS);
    lv_obj_set_style_line_color(temp_mem_meter, lv_palette_main(LV_PALETTE_GREY), LV_PART_ITEMS);
    /* major ticks */
    lv_obj_set_style_length(temp_mem_meter, 15, LV_PART_INDICATOR);
    lv_obj_set_style_line_width(temp_mem_meter, 2, LV_PART_INDICATOR);
    lv_obj_set_style_line_color(temp_mem_meter, lv_color_black(), LV_PART_INDICATOR);

    /* blue section: -40 ~ 10, red section: 50 ~ 105 */
    lv_scale_section_t * section = lv_scale_add_section(temp_mem_meter);
    lv_scale_set_section_range(temp_mem_meter, section, -40, 10);
    lv_scale_set_section_style_main(temp_mem_meter, section, &style_sec_cold);
    lv_scale_set_section_style_indicator(temp_mem_meter, section, &style_sec_cold);
    lv_scale_set_section_style_items(temp_mem_meter, section, &style_sec_cold);

    section = lv_scale_add_section(temp_mem_meter);
    lv_scale_set_section_range(temp_mem_meter, section, 50, 105);
    lv_scale_set_section_style_main(temp_mem_meter, section, &style_sec_hot);
    lv_scale_set_section_style_indicator(temp_mem_meter, section, &style_sec_hot);
    lv_scale_set_section_style_items(temp_mem_meter, section, &style_sec_hot);

    /* needle */
    indic_temp = lv_line_create(temp_mem_meter);
    lv_obj_set_style_line_width(indic_temp, 2, LV_PART_MAIN);
    lv_obj_set_style_line_color(indic_temp, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(indic_temp, true, LV_PART_MAIN);
    temp_needle_set_value(10);

    lv_obj_t * needle_center = lv_obj_create(temp_mem_meter);
    lv_obj_remove_style_all(needle_center);
    lv_obj_set_size(needle_center, 5, 5);
    lv_obj_set_style_radius(needle_center, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(needle_center, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(needle_center, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_center(needle_center);

    ///////////////////////////////////////////
    create_meter_box(obj_performance, "Memory used", "Total size: 63%", "Memory used: 44%", "Amount of fragmentation: 58%");
    create_temp_box(obj_performance, "Temperature");
    g_meter_timer = lv_timer_create(update_meter_box, 100, NULL);

    /* button event add */
    lv_obj_add_event_cb(imgbtn_widget, widget_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(imgbtn_music, music_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(imgbtn_speed, speed_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(imgbtn_about, about_event_cb, LV_EVENT_CLICKED, obj_main);
    lv_obj_add_event_cb(imgbtn_qrcode, qrcode_event_cb, LV_EVENT_CLICKED, obj_main);
    lv_obj_add_event_cb(imgbtn_setting, setting_event_cb, LV_EVENT_CLICKED, obj_main);
    lv_obj_add_event_cb(imgbtn_picture, picture_event_cb, LV_EVENT_CLICKED, obj_main);
#if SYS_configUSE_FAT32FS
    lv_obj_add_event_cb(imgbtn_file, file_browse_event_cb, LV_EVENT_RELEASED, NULL);
#endif
    
    if(!rtc_initialized) {
        rtc_para.year = 2026;
        rtc_para.month = 8;
        rtc_para.day = 1;
        rtc_para.hour = 10;
        rtc_para.minute = 0;
        rtc_para.second = 0;
        rtc_initialized = true;
    }

    /* add main page timing label */
    font_label = lv_label_create(obj_main);
//    lv_obj_set_style_text_font(font_label, &lv_font_digital_35, 0);
    lv_obj_set_style_text_font(font_label, &lv_font_digital_30, 0);
    lv_label_set_recolor(font_label, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text_fmt(font_label, "#FFFFFF %02d:%02d:%02d", rtc_para.hour, rtc_para.minute, rtc_para.second);
    lv_obj_align(font_label, LV_ALIGN_LEFT_MID, SCALE_POS_X(50), SCALE_POS_Y(-40));

    font_label_year = lv_label_create(obj_main);
//    lv_obj_set_style_text_font(font_label_year, &lv_font_digital_35, 0);
    lv_obj_set_style_text_font(font_label_year, &lv_font_digital_20, 0);
    lv_label_set_recolor(font_label_year, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text_fmt(font_label_year, "#FFFFFF %d/%02d/%02d", rtc_para.year, rtc_para.month, rtc_para.day);
    lv_obj_align(font_label_year, LV_ALIGN_LEFT_MID, SCALE_POS_X(50), SCALE_POS_Y(40));
    lv_obj_set_clickable(font_label_year, true);
    lv_obj_add_event_cb(font_label_year, year_label_evevt_cb, LV_EVENT_ALL, 0);

    if(g_rtc_timer == NULL) {
        g_rtc_timer = lv_timer_create(rtc_timer, 1000, NULL);
        if(g_rtc_timer) {
            lv_timer_ready(g_rtc_timer);
        }
    }

    img = lv_image_create( obj_performance );
    lv_image_set_src(img, &img_temp);
    lv_obj_align_to(img, temp_mem_meter, LV_ALIGN_CENTER, SCALE_POS_X(0), SCALE_POS_Y(90));
    
    img = lv_image_create( obj_performance );
    lv_image_set_src(img, &img_mem);
    lv_obj_align_to(img, mem_meter, LV_ALIGN_CENTER, SCALE_POS_X(0), SCALE_POS_Y(0));
}


#if SYS_configUSE_FAT32FS
    void lv_demo_app_fs_test(void)
    {

        lv_fs_res_t res;
        lv_fs_file_t f;
        uint8_t buf[100];
        uint8_t WriteBuffer[] = "1232This is FatFs working fine!\r\n";
        uint32_t read_num;
        uint32_t pos = 0;
        res = lv_fs_open(&f, "0:/测试.TXT",LV_FS_MODE_RD | LV_FS_MODE_WR);

        if(LV_FS_RES_OK == res){
            res = lv_fs_write(&f, WriteBuffer, sizeof(WriteBuffer), NULL);
            if(LV_FS_RES_OK == res){
                lv_fs_tell(&f, &pos);
                res = lv_fs_seek(&f, 0, LV_FS_SEEK_SET);
                lv_fs_tell(&f, &pos);
                res = lv_fs_read(&f, buf, 8, NULL);
            }
            lv_fs_tell(&f, &pos);
            lv_fs_close(&f);
            read_num = buf[0];
            read_num = buf[1];
            read_num = buf[2];
        }
    }
#endif
/**********************
 *   STATIC FUNCTIONS
 **********************/

