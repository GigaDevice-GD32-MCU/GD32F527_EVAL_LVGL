/**
 * @file lv_demo_widgets_components.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_widgets_components.h"
#if LV_USE_DEMO_WIDGETS

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *  GLOBAL VARIABLES
 **********************/
disp_size_t disp_size;
lv_style_t style_title;
lv_style_t style_text_muted;
lv_style_t style_icon;
lv_style_t style_bullet;

const lv_font_t * font_large;
const lv_font_t * font_normal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_demo_widgets_components_init(void)
{
#if LV_USE_DEMO_BENCHMARK && LV_DEMO_BENCHMARK_ALIGNED_FONTS
    LV_FONT_DECLARE(lv_font_benchmark_montserrat_12_aligned)
    LV_FONT_DECLARE(lv_font_benchmark_montserrat_14_aligned)
    LV_FONT_DECLARE(lv_font_benchmark_montserrat_16_aligned)
    LV_FONT_DECLARE(lv_font_benchmark_montserrat_18_aligned)
    LV_FONT_DECLARE(lv_font_benchmark_montserrat_20_aligned)
    LV_FONT_DECLARE(lv_font_benchmark_montserrat_24_aligned)
#define USE_ALIGNED_FONTS
#endif

    if(LV_HOR_RES <= 320) disp_size = DISP_SMALL;
    else if(LV_HOR_RES < 720) disp_size = DISP_MEDIUM;
    else disp_size = DISP_LARGE;

    font_large = LV_FONT_DEFAULT;
    font_normal = LV_FONT_DEFAULT;

    if(disp_size == DISP_LARGE) {
#ifdef USE_ALIGNED_FONTS
        font_large     = &lv_font_benchmark_montserrat_24_aligned;
#elif LV_FONT_MONTSERRAT_24
        font_large     = &lv_font_montserrat_24;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_24 or LV_DEMO_BENCHMARK_ALIGNED_FONTS is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#ifdef USE_ALIGNED_FONTS
        font_normal    = &lv_font_benchmark_montserrat_16_aligned;
#elif LV_FONT_MONTSERRAT_16
        font_normal    = &lv_font_montserrat_16;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_16 or LV_DEMO_BENCHMARK_ALIGNED_FONTS is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }
    else if(disp_size == DISP_MEDIUM) {
#ifdef USE_ALIGNED_FONTS
        font_large     = &lv_font_benchmark_montserrat_20_aligned;
#elif LV_FONT_MONTSERRAT_20
        font_large     = &lv_font_montserrat_20;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_20 or LV_DEMO_BENCHMARK_ALIGNED_FONTS is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#ifdef USE_ALIGNED_FONTS
        font_normal    = &lv_font_benchmark_montserrat_14_aligned;
#elif LV_FONT_MONTSERRAT_10
        font_normal    = &lv_font_montserrat_10;    /*GD: smaller font for the 480x272 LCD*/
#elif LV_FONT_MONTSERRAT_14
        font_normal    = &lv_font_montserrat_14;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_14 or LV_DEMO_BENCHMARK_ALIGNED_FONTS is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }
    else {   /* disp_size == DISP_SMALL */
#ifdef USE_ALIGNED_FONTS
        font_large     = &lv_font_benchmark_montserrat_18_aligned;
#elif LV_FONT_MONTSERRAT_18
        font_large     = &lv_font_montserrat_18;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_18 or LV_DEMO_BENCHMARK_ALIGNED_FONTS is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
#ifdef USE_ALIGNED_FONTS
        font_normal    = &lv_font_benchmark_montserrat_12_aligned;
#elif LV_FONT_MONTSERRAT_10
        font_normal    = &lv_font_montserrat_10;    /*GD: smaller font for small LCDs*/
#elif LV_FONT_MONTSERRAT_12
        font_normal    = &lv_font_montserrat_12;
#else
        LV_LOG_WARN("LV_FONT_MONTSERRAT_12 or LV_DEMO_BENCHMARK_ALIGNED_FONTS is not enabled for the widgets demo. Using LV_FONT_DEFAULT instead.");
#endif
    }
#if LV_USE_THEME_DEFAULT
    lv_theme_default_init(lv_display_get_default(), lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                          LV_THEME_DEFAULT_DARK,
                          font_normal);
#endif


    lv_obj_set_style_text_font(lv_screen_active(), font_normal, 0);


    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, font_normal);    /*GD: compact titles*/

    lv_style_init(&style_text_muted);
    lv_style_set_text_opa(&style_text_muted, LV_OPA_50);


    lv_style_init(&style_icon);
    lv_style_set_text_color(&style_icon, lv_theme_get_color_primary(NULL));
    lv_style_set_text_font(&style_icon, font_normal);     /*GD: compact icons*/

    lv_style_init(&style_bullet);
    lv_style_set_border_width(&style_bullet, 0);
    lv_style_set_radius(&style_bullet, LV_RADIUS_CIRCLE);

}

lv_obj_t * lv_demo_widgets_title_create(lv_obj_t * parent, const char * text)
{
    lv_obj_t * label = lv_label_create(parent);
    lv_obj_add_style(label, &style_title, 0);
    lv_label_set_text_static(label, text);

    return label;
}

/*GD: LVGL v8 chart emulation, see `lv_demo_widgets_chart_v8_init()`*/
typedef struct {
    int32_t zoom_x;
    int32_t pad_right_ori;
    int32_t pad_bottom_ori;
    int32_t content_w;      /*Content width calculated as in LVGL v8*/
    int32_t zoomed_w;       /*Width of the series' area (`content_w * zoom_x`)*/
    uint32_t y_label_cnt;
    const char * const * x_texts;
    uint32_t x_label_cnt;
} chart_v8_t;

static void chart_v8_update_size(lv_obj_t * chart, chart_v8_t * d)
{
    int32_t w = lv_obj_get_width(chart);
    int32_t pad_left = lv_obj_get_style_pad_left(chart, LV_PART_MAIN);
    int32_t border_w = lv_obj_get_style_border_width(chart, LV_PART_MAIN);
    lv_border_side_t side = lv_obj_get_style_border_side(chart, LV_PART_MAIN);

    /*LVGL v8 subtracts the border width from both sides regardless of the border side*/
    d->content_w = LV_MAX(w - pad_left - d->pad_right_ori - 2 * border_w, 0);
    d->zoomed_w = (d->content_w * d->zoom_x) >> 8;

    /*Set the paddings so that LVGL v9 draws to the same area as LVGL v8:
     *`lv_obj_get_content_width()` is the zoomed width and the height is calculated as in LVGL v8*/
    int32_t space_left = pad_left + ((side & LV_BORDER_SIDE_LEFT) ? border_w : 0);
    int32_t pad_right = w - space_left - d->zoomed_w - ((side & LV_BORDER_SIDE_RIGHT) ? border_w : 0);
    int32_t pad_bottom = d->pad_bottom_ori + 2 * border_w - ((side & LV_BORDER_SIDE_TOP) ? border_w : 0) -
                         ((side & LV_BORDER_SIDE_BOTTOM) ? border_w : 0);

    if(lv_obj_get_style_pad_right(chart, LV_PART_MAIN) != pad_right) {
        lv_obj_set_style_pad_right(chart, pad_right, LV_PART_MAIN);
    }
    if(lv_obj_get_style_pad_bottom(chart, LV_PART_MAIN) != pad_bottom) {
        lv_obj_set_style_pad_bottom(chart, pad_bottom, LV_PART_MAIN);
    }
    lv_obj_refresh_self_size(chart);
}

static void chart_v8_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * chart = lv_event_get_current_target(e);
    chart_v8_t * d = lv_event_get_user_data(e);

    if(code == LV_EVENT_SIZE_CHANGED) {
        chart_v8_update_size(chart, d);
    }
    else if(code == LV_EVENT_GET_SELF_SIZE) {
        /*Make the chart scrollable horizontally by `zoomed_w - content_w` as in LVGL v8*/
        lv_point_t * p = lv_event_get_param(e);
        p->x = LV_MAX(p->x, 2 * d->zoomed_w - d->content_w);
        p->y = LV_MAX(p->y, lv_obj_get_content_height(chart));
    }
    else if(code == LV_EVENT_SCROLL) {
        /*The axis labels are drawn by the parent*/
        lv_obj_invalidate(lv_obj_get_parent(chart));
    }
    else if(code == LV_EVENT_DELETE) {
        lv_obj_set_user_data(chart, NULL);
        lv_free(d);
    }
}

/*Draw the axis labels exactly where `draw_y_ticks()` and `draw_x_ticks()` of LVGL v8 draw them*/
static void chart_v8_parent_draw_event_cb(lv_event_t * e)
{
    lv_obj_t * chart = lv_event_get_user_data(e);
    chart_v8_t * d = lv_obj_get_user_data(chart);
    if(d == NULL) return;

    lv_layer_t * layer = lv_event_get_layer(e);

    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.font = lv_obj_get_style_text_font(chart, LV_PART_MAIN);
    label_dsc.color = lv_palette_main(LV_PALETTE_GREY);    /*Text color of the ticks in the v8 theme*/

    lv_area_t coords;
    lv_obj_get_coords(chart, &coords);
    int32_t border_w = lv_obj_get_style_border_width(chart, LV_PART_MAIN);
    int32_t tick_line_w = LV_DPX(1);    /*Line width of the ticks in the v8 theme*/
    int32_t label_gap = LV_DPX(2);      /*Padding of the ticks in the v8 theme*/
    uint32_t i;

    if(d->y_label_cnt > 1) {
        int32_t total = (int32_t)d->y_label_cnt - 1;
        int32_t h = lv_obj_get_content_height(chart);
        int32_t y_ofs = coords.y1 + lv_obj_get_style_pad_top(chart, LV_PART_MAIN) + border_w - lv_obj_get_scroll_top(chart);
        int32_t x = coords.x1 - 1;

        for(i = 0; i <= (uint32_t)total; i++) {
            int32_t y = y_ofs + ((h - tick_line_w) * (int32_t)i) / total;

            char buf[16];
            lv_snprintf(buf, sizeof(buf), "%" LV_PRId32, lv_map(total - (int32_t)i, 0, total, 0, 100));

            lv_point_t size;
            lv_text_get_size(&size, buf, label_dsc.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t a;
            a.y1 = y - size.y / 2;
            a.y2 = y + size.y / 2;
            a.x1 = x - size.x - label_gap;
            a.x2 = x - label_gap;

            if(a.y2 >= coords.y1 && a.y1 <= coords.y2) {
                label_dsc.text = buf;
                label_dsc.text_local = 1;
                lv_draw_label(layer, &label_dsc, &a);
            }
        }
    }

    if(d->x_texts && d->x_label_cnt > 1) {
        int32_t total = (int32_t)d->x_label_cnt - 1;
        int32_t w = lv_obj_get_content_width(chart);
        int32_t x_ofs = coords.x1 + lv_obj_get_style_pad_left(chart, LV_PART_MAIN) + border_w - lv_obj_get_scroll_left(chart);

        /*The labels of the columns are aligned to the center of the blocks*/
        if(lv_chart_get_type(chart) == LV_CHART_TYPE_BAR) {
            int32_t block_gap = lv_obj_get_style_pad_column(chart, LV_PART_MAIN);
            int32_t block_w = (w + block_gap) / (int32_t)lv_chart_get_point_count(chart);
            x_ofs += (block_w - block_gap) / 2;
            w -= block_w - block_gap;
        }

        int32_t y = coords.y2 + 1 + label_gap;
        for(i = 0; i <= (uint32_t)total; i++) {
            int32_t x = x_ofs + ((w - tick_line_w) * (int32_t)i) / total;

            lv_point_t size;
            lv_text_get_size(&size, d->x_texts[i], label_dsc.font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

            lv_area_t a;
            a.x1 = x - size.x / 2;
            a.x2 = x + size.x / 2;
            a.y1 = y;
            a.y2 = y + size.y;

            if(a.x2 >= coords.x1 && a.x1 <= coords.x2) {
                label_dsc.text = d->x_texts[i];
                label_dsc.text_local = 0;
                lv_draw_label(layer, &label_dsc, &a);
            }
        }
    }
}

void lv_demo_widgets_chart_v8_init(lv_obj_t * chart, int32_t zoom_x, uint32_t y_label_cnt,
                                   const char * const * x_texts, uint32_t x_label_cnt)
{
    chart_v8_t * d = lv_malloc_zeroed(sizeof(chart_v8_t));
    LV_ASSERT_MALLOC(d);
    if(d == NULL) return;

    d->zoom_x = zoom_x;
    d->pad_right_ori = lv_obj_get_style_pad_right(chart, LV_PART_MAIN);
    d->pad_bottom_ori = lv_obj_get_style_pad_bottom(chart, LV_PART_MAIN);
    d->y_label_cnt = y_label_cnt;
    d->x_texts = x_texts;
    d->x_label_cnt = x_label_cnt;
    lv_obj_set_user_data(chart, d);

    /*LVGL v8 scales the gaps of the bars with the zoom too*/
    if(zoom_x != LV_SCALE_NONE) {
        int32_t block_gap = lv_obj_get_style_pad_column(chart, LV_PART_MAIN);
        int32_t ser_gap = lv_obj_get_style_pad_column(chart, LV_PART_ITEMS);
        lv_obj_set_style_pad_column(chart, (block_gap * zoom_x) >> 8, LV_PART_MAIN);
        lv_obj_set_style_pad_column(chart, (ser_gap * zoom_x) >> 8, LV_PART_ITEMS);
    }

    lv_obj_add_event_cb(chart, chart_v8_event_cb, LV_EVENT_ALL, d);
    chart_v8_update_size(chart, d);

    if(y_label_cnt > 1 || (x_texts && x_label_cnt > 1)) {
        lv_obj_add_event_cb(lv_obj_get_parent(chart), chart_v8_parent_draw_event_cb, LV_EVENT_DRAW_POST, chart);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* LV_USE_DEMO_WIDGETS */
