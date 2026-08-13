/**
 * @file lv_demo_app.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include <stdio.h>
#include "lv_demos.h"
#include "gd32f527.h"
#include "lv_file_browse.h"

extern Folder_btnArray *p;
extern File_btnArray *q;
extern char file_name[FILE_NUM][NAME_LENGTH];//�ļ���
extern char folder_name[FOLDER_NUM][NAME_LENGTH];//�ļ�����
extern char mountsel[];
extern uint8_t file_cnt;
extern uint8_t folder_cnt;
extern uint8_t current_path[PATH_LENGTH];
extern lv_obj_t *obj_performance;
void setup_scr_file_browse(void);

extern void lv_demo_app(void);

char *pname;
/*********************
 *      DEFINES
 *********************/
LV_IMG_DECLARE(image_file_28);
LV_IMG_DECLARE(image_folder_28);
lv_obj_t *win;

/* �ļ���������ؼ� */
extern lv_obj_t *back_btn;

/* 待处理的列表刷新动作。
 * 删除/重建窗口必须在事件回调之外执行，否则 LVGL 事件和输入设备
 * 会继续访问已释放的按钮对象(use-after-free)，表现为偶发性卡死。*/
typedef enum {
    FB_ACT_NONE = 0,
    FB_ACT_ENTER,
    FB_ACT_BACK,
    FB_ACT_CLOSE,
} fb_action_t;

static fb_action_t fb_pending_action = FB_ACT_NONE;
static char fb_pending_name[NAME_LENGTH];

static void fb_apply_action(void *data)
{
    LV_UNUSED(data);

    fb_action_t act = fb_pending_action;
    fb_pending_action = FB_ACT_NONE;

    if(win) {
        lv_obj_del(win);
        win = NULL;
        back_btn = NULL;
    }

    switch(act) {
    case FB_ACT_ENTER:
        (void)path_push(fb_pending_name);
        mf_scan_file((char *)(current_path));
        setup_scr_file_browse();
        break;

    case FB_ACT_BACK:
        refersh_parameter();
        setup_scr_file_browse();
        break;

    case FB_ACT_CLOSE:
        memset(current_path, 0, PATH_LENGTH);
        strncpy((char *)(current_path), "0:", PATH_LENGTH - 1);
        strncpy(mountsel, "0:", 3);
        mf_scan_file(mountsel);
        if(obj_performance) {
            lv_obj_clear_flag(obj_performance, LV_OBJ_FLAG_HIDDEN);
        }
        break;

    default:
        break;
    }
}

static void fb_request_action(fb_action_t act, const char *name)
{
    /* 已经有一个动作排队时直接忽略，避免连点造成重复删除窗口 */
    if(fb_pending_action != FB_ACT_NONE) {
        return;
    }

    fb_pending_name[0] = '\0';
    if(name) {
        strncpy(fb_pending_name, name, sizeof(fb_pending_name) - 1);
        fb_pending_name[sizeof(fb_pending_name) - 1] = '\0';
    }

    fb_pending_action = act;
    if(lv_async_call(fb_apply_action, NULL) != LV_RES_OK) {
        fb_pending_action = FB_ACT_NONE;
    }
}

static void filelist_btnback_handler(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);

    switch(code) {
    case LV_EVENT_CLICKED: {
        fb_request_action(FB_ACT_BACK, NULL);
    }
    break;
    default:
        break;
    }
}

void filelist_btnevent_handler(lv_event_t *event)
{
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *obj = lv_event_get_target(event);
    lv_obj_t *list1 = lv_obj_get_parent(obj);

    switch(code) {
    case LV_EVENT_LONG_PRESSED:
    case LV_EVENT_RELEASED: {
        pname = (char *)lv_list_get_btn_text(list1, obj);
        fb_request_action(FB_ACT_ENTER, pname);
    }
    break;
    default:
        break;
    }
}

void file_browse_event_cb(lv_event_t *event)
{
    /* 窗口已打开或有待处理动作时不重复创建 */
    if(win != NULL || fb_pending_action != FB_ACT_NONE) {
        return;
    }

    lv_obj_add_flag(obj_performance, LV_OBJ_FLAG_HIDDEN);
#if 0
    /*Create a list*/
    list1 = lv_list_create(lv_scr_act());
    lv_obj_set_size(list1, 180, 220);
    lv_obj_center(list1);

    /*Add buttons to the list*/
    lv_obj_t *btn;

    lv_list_add_text(list1, "File");
    btn = lv_list_add_btn(list1, LV_SYMBOL_FILE, "New");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_DIRECTORY, "Open");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_SAVE, "Save");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Delete");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_EDIT, "Edit");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Connectivity");
    btn = lv_list_add_btn(list1, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_GPS, "Navigation");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_USB, "USB");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_BATTERY_FULL, "Battery");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_list_add_text(list1, "Exit");
    btn = lv_list_add_btn(list1, LV_SYMBOL_OK, "Apply");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
    btn = lv_list_add_btn(list1, LV_SYMBOL_CLOSE, "Close");
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);
#endif
    mf_scan_file(mountsel);
    setup_scr_file_browse();
#if 0
    /***************/
    p = (Folder_btnArray *)malloc(sizeof(Folder_btnArray) + sizeof(int) * folder_cnt);
    p->folderlen = folder_cnt;


    q = (File_btnArray *)malloc(sizeof(File_btnArray) + sizeof(int) * file_cnt);
    q->filelen = file_cnt;

    for(i = 0; i < p->folderlen; i++) {
        p->Folder_btn[i] = lv_list_add_btn(ui->file_browse_filelist, &_wenjianjia_20x20,
                                           folder_name[i]);
        lv_obj_set_event_cb(p->Folder_btn[i], filelist_btnevent_handler);
    }
    for(j = 0; j < q->filelen; j++) {
        q->File_btn[j] = lv_list_add_btn(ui->file_browse_filelist, &_wenjiankongxin_20x20, file_name[j]);
        lv_obj_set_event_cb(q->File_btn[j], file_btnevent_handler);
    }
    /***************/
#endif
}
//lv_obj_t * win;
lv_obj_t *back_btn;
static void win_close_click_event_cb(lv_event_t *event)
{
    LV_UNUSED(event);
    fb_request_action(FB_ACT_CLOSE, NULL);
}

void events_init_file_browse(void)
{
    if(strcmp(mountsel, (char *)(current_path)) != 0) {
        lv_obj_add_event_cb(back_btn, filelist_btnback_handler, LV_EVENT_PRESSED, NULL);
    }
}

void setup_scr_file_browse(void)
{
    win = lv_win_create(lv_scr_act(), 40);
    lv_obj_set_size(win, 480, 272);
    lv_obj_set_style_radius(win, 8, LV_PART_MAIN);

    lv_obj_clear_flag(win, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(win, LV_ALIGN_CENTER, 0, 0);

    /* Make the window's content area vertically scrollable */
    lv_obj_t * cont = lv_win_get_content(win);
    lv_obj_set_scroll_dir(cont, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_AUTO);

    if(strcmp(mountsel, (char *)(current_path)) != 0) {
        back_btn = lv_win_add_btn(win, LV_SYMBOL_LEFT, 40);
        lv_obj_add_event_cb(back_btn, filelist_btnback_handler, LV_EVENT_CLICKED, NULL);
    }
    lv_win_add_title(win, "");
    lv_obj_t *close_btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);
    lv_obj_set_ext_click_area(close_btn, LV_DPX(10));
    lv_obj_add_event_cb(close_btn, win_close_click_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *label = lv_label_create(close_btn);
    lv_label_set_text(label, LV_SYMBOL_CLOSE);
    const lv_font_t *font = lv_obj_get_style_text_font(close_btn, LV_PART_MAIN);
    lv_coord_t close_btn_size = lv_font_get_line_height(font) + LV_DPX(10);
    lv_obj_set_size(close_btn, close_btn_size, close_btn_size);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(close_btn, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t *file_list = lv_list_create(cont);
    lv_obj_set_size(file_list, 460, LV_SIZE_CONTENT);
    lv_obj_align(file_list, LV_ALIGN_TOP_MID, 0, 0);

    static lv_style_t style_screen_list0_bg;
    static lv_style_t style_screen_list0_scrollable;
    static lv_style_t style_screen_list0_main_child;
    static bool styles_inited = false;

    /* 只初始化一次：重复 lv_style_init() 会丢弃之前已分配的属性数组，
     * 每切换一次目录都泄漏一次 LVGL 堆，最终触发 LV_ASSERT_MALLOC 死循环。*/
    if(!styles_inited) {
        styles_inited = true;

        lv_style_init(&style_screen_list0_bg);
        //Write style state: LV_STATE_DEFAULT for style_screen_list0_bg
        lv_style_set_radius(&style_screen_list0_bg, 3);
        lv_style_set_border_color(&style_screen_list0_bg, lv_color_make(0xe1, 0xe6, 0xee));
        lv_style_set_border_width(&style_screen_list0_bg, 1);

        lv_style_init(&style_screen_list0_scrollable);
        //Write style state: LV_STATE_DEFAULT for style_screen_list0_scrollable
        lv_style_set_radius(&style_screen_list0_scrollable, 3);
        lv_style_set_bg_color(&style_screen_list0_scrollable, lv_color_make(0xff, 0xff, 0xff));
        lv_style_set_bg_grad_color(&style_screen_list0_scrollable, lv_color_make(0xff, 0xff, 0xff));
        lv_style_set_bg_grad_dir(&style_screen_list0_scrollable, LV_GRAD_DIR_VER);
        lv_style_set_bg_opa(&style_screen_list0_scrollable, 255);

        //Write style LV_BTN_PART_MAIN for screen_list0
        lv_style_init(&style_screen_list0_main_child);
        //Write style state: LV_STATE_DEFAULT for style_screen_list0_main_child
        lv_style_set_radius(&style_screen_list0_main_child, 3);
        lv_style_set_bg_color(&style_screen_list0_main_child, lv_color_make(0xff, 0xff, 0xff));
        lv_style_set_bg_grad_color(&style_screen_list0_main_child, lv_color_make(0xff, 0xff, 0xff));
        lv_style_set_bg_grad_dir(&style_screen_list0_main_child, LV_GRAD_DIR_VER);
        lv_style_set_bg_opa(&style_screen_list0_main_child, 255);
        lv_style_set_text_color(&style_screen_list0_main_child, lv_color_make(0x0D, 0x30, 0x55));
    }

    lv_obj_add_style(file_list, &style_screen_list0_bg, LV_PART_MAIN);
    lv_obj_add_style(file_list, &style_screen_list0_scrollable, LV_PART_MAIN);

    /* Let the list grow with its items; scrolling happens on the win content */
    lv_obj_set_width(file_list, 460);
    lv_obj_set_height(file_list, LV_SIZE_CONTENT);
    p = (Folder_btnArray *)malloc(sizeof(Folder_btnArray) + sizeof(lv_obj_t *) * folder_cnt);
    q = (File_btnArray *)malloc(sizeof(File_btnArray) + sizeof(lv_obj_t *) * file_cnt);
    if(p == NULL || q == NULL) {
        free(p);
        free(q);
        p = NULL;
        q = NULL;
        return;
    }
    p->folderlen = folder_cnt;
    q->filelen = file_cnt;

    for(int i = 0; i < p->folderlen; i++) {
        p->Folder_btn[i] = lv_list_add_btn(file_list, &image_folder_28,
                                           folder_name[i]);
        lv_obj_add_event_cb(p->Folder_btn[i], filelist_btnevent_handler, LV_EVENT_RELEASED, NULL);
    }
    for(int j = 0; j < q->filelen; j++) {
        q->File_btn[j] = lv_list_add_btn(file_list, &image_file_28, file_name[j]);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
