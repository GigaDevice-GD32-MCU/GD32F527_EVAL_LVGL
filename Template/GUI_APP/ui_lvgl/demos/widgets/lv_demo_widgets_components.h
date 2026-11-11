/**
 * @file lv_demo_widgets_components.h
 *
 */

#ifndef LV_DEMO_WIDGETS_COMPONENTS_H
#define LV_DEMO_WIDGETS_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

#if LV_USE_DEMO_WIDGETS

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef enum {
    DISP_SMALL,
    DISP_MEDIUM,
    DISP_LARGE,
} disp_size_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_demo_widgets_components_init(void);

lv_obj_t * lv_demo_widgets_title_create(lv_obj_t * parent, const char * text);

/**
 * GD: make a chart look and behave like the charts of the LVGL v8 widgets demo:
 * - `zoom_x` emulates `lv_chart_set_zoom_x()` (256: 100%): the series are drawn on
 *   a wider area which can be scrolled horizontally inside the chart,
 * - the gaps of the bars are scaled with the zoom,
 * - the size of the drawing area is calculated like in LVGL v8,
 * - Y and X axis labels are drawn around the chart like `lv_chart_set_axis_tick()`
 *   with `major_len = 0, minor_len = 0, minor_cnt = 1, label_en = true`.
 * Call it after the chart's padding and border side are set.
 * The labels are drawn by the parent of the chart in its post draw phase.
 * @param chart         pointer to a chart
 * @param zoom_x        horizontal zoom (256: no zoom)
 * @param y_label_cnt   number of the Y axis labels (100 ... 0), 0: no labels
 * @param x_texts       texts of the X axis labels, NULL: no labels
 * @param x_label_cnt   number of the X axis labels
 */
void lv_demo_widgets_chart_v8_init(lv_obj_t * chart, int32_t zoom_x, uint32_t y_label_cnt,
                                   const char * const * x_texts, uint32_t x_label_cnt);

/**********************
 * GLOBAL VARIABLES
 **********************/

extern disp_size_t disp_size;

extern lv_style_t style_title;
extern lv_style_t style_text_muted;
extern lv_style_t style_icon;
extern lv_style_t style_bullet;

extern const lv_font_t * font_large;
extern const lv_font_t * font_normal;

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_DEMO_BENCHMARK */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_DEMO_WIDGETS_COMPONENTS_H*/
