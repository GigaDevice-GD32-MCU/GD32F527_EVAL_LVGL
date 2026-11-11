/**
 * @file lv_port_disp.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include <stdint.h>

#include "gd32f527.h"
#include "gd32f527b_lcd_eval.h"

/*********************
 *      DEFINES
 *********************/
/* 1: one partial buffer + IPA copy
 * 2: two partial buffers + IPA copy (default)
 * 3: LVGL renders directly into the TLI frame buffer (no IPA copy) */
#define LV_BUF_TYPE     2
/* 1: IPA transfer complete is reported by interrupt, 0: polling */
#define USE_IPA_INT     1

#define BYTE_PER_PIXEL  (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */

/* Number of lines of a partial draw buffer */
#define LV_BUF_LINES    64

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
/* TLI frame buffer (shown on the LCD) */
static uint8_t buf_disp[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL] __attribute__((section("SDRAM"), aligned(32)));

#if (LV_BUF_TYPE == 1)
/* LVGL draw buffer */
#define LV_BUF_SIZE     (MY_DISP_HOR_RES * LV_BUF_LINES * BYTE_PER_PIXEL)
static uint8_t buf_2_1[LV_BUF_SIZE] __attribute__((section("SDRAM"), aligned(32)));
#elif (LV_BUF_TYPE == 2)
/* LVGL draw buffers */
#define LV_BUF_SIZE     (MY_DISP_HOR_RES * LV_BUF_LINES * BYTE_PER_PIXEL)
static uint8_t buf_2_1[LV_BUF_SIZE] __attribute__((section("SDRAM"), aligned(32)));
static uint8_t buf_2_2[LV_BUF_SIZE] __attribute__((section("SDRAM"), aligned(32)));
#endif

#if (LV_BUF_TYPE != 3) && (USE_IPA_INT == 1)
static volatile uint8_t g_ipa_flag = 0U;
static lv_display_t * g_disp;
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, disp_flush);

#if (LV_BUF_TYPE == 1)
    /* One buffer for partial rendering */
    lv_display_set_buffers(disp, buf_2_1, NULL, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
#elif (LV_BUF_TYPE == 2)
    /* Two buffers for partial rendering, IPA copies the rendered area to the frame buffer in the background */
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
#else
    /* LVGL renders directly into the frame buffer shown by the TLI */
    lv_display_set_buffers(disp, buf_disp, NULL, sizeof(buf_disp), LV_DISPLAY_RENDER_MODE_DIRECT);
#endif

#if (LV_BUF_TYPE != 3) && (USE_IPA_INT == 1)
    g_disp = disp;
    nvic_irq_enable(IPA_IRQn, 2, 0);
#endif
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /* IPA init */
    rcu_periph_clock_enable(RCU_IPA);
    ipa_interval_clock_num_config(5);
    ipa_inter_timer_config(IPA_INTER_TIMER_ENABLE);

    /* initialize the LCD and layers */
    lcd_init();
    lcd_layer_init(LCD_LAYER_BACKGROUND, (uint32_t)buf_disp, MY_DISP_HOR_RES, MY_DISP_VER_RES, LAYER_PPF_RGB565);
    lcd_layer_enable(LCD_LAYER_BACKGROUND);
    lcd_layer_set(LCD_LAYER_BACKGROUND);
    lcd_transparency_set(255);
    lcd_clear(LCD_COLOR_BLACK);

    tli_enable();
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *IPA is used to copy the pixels in the background and
 *'lv_display_flush_ready()' is called when it's finished.*/
static void disp_flush(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
#if (LV_BUF_TYPE == 3)
    /* Rendered directly into the frame buffer, nothing to copy */
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    lv_display_flush_ready(disp);
#else
    if(!disp_flush_enabled) {
        lv_display_flush_ready(disp);
        return;
    }

    uint32_t w = (uint32_t)lv_area_get_width(area);
    uint32_t h = (uint32_t)lv_area_get_height(area);

#if (USE_IPA_INT == 1)
    /* wait for the previous transfer */
    while(1U == g_ipa_flag) {
    }
#endif

    IPA_CTL = 0x0U;
    /* clear stale flags, a pending error flag would make the IPA interrupt fire endlessly */
    IPA_INTC = IPA_INTC_TAEIFC | IPA_INTC_FTFIFC | IPA_INTC_TLMIFC |
               IPA_INTC_LACIFC | IPA_INTC_LLFIFC | IPA_INTC_WCFIFC;
    /* destination format must match the RGB565 frame buffer (reset value is ARGB8888) */
    IPA_DPCTL = (IPA_DPCTL & ~IPA_DPCTL_DPF) | (uint32_t)IPA_DPF_RGB565;
    IPA_FMADDR = (uint32_t)px_map;
    IPA_DMADDR = (uint32_t)buf_disp + BYTE_PER_PIXEL * (MY_DISP_HOR_RES * area->y1 + area->x1);
    IPA_FLOFF = 0U;
    IPA_DLOFF = MY_DISP_HOR_RES - w;
    IPA_FPCTL = FOREGROUND_PPF_RGB565;
    IPA_IMS = (uint32_t)(w << 16) | (uint16_t)h;

#if (USE_IPA_INT == 1)
    /* enable IPA interrupt, lv_display_flush_ready() is called in IPA_IRQHandler() */
    g_ipa_flag = 1U;
    IPA_CTL |= IPA_CTL_FTFIE | IPA_CTL_TAEIE | IPA_CTL_WCFIE;
    IPA_CTL |= IPA_CTL_TEN;
#else
    uint32_t time_out = 0U;
    IPA_CTL |= IPA_CTL_TEN;
    /* Wait until transfer is done */
    while(IPA_CTL & IPA_CTL_TEN) {
        if(time_out++ >= 0xFFFFFFF0U) break;
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp);
#endif
#endif /* LV_BUF_TYPE == 3 */
}

#if (LV_BUF_TYPE != 3) && (USE_IPA_INT == 1)
/*!
    \brief      this function handles IPA exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void IPA_IRQHandler(void)
{
    uint32_t flags = IPA_INTF;

    /* Clear every flag that is set. TAE/WCF are enabled in disp_flush() too: if they were
     * left uncleared, the interrupt would re-enter forever and the system would hang. */
    IPA_INTC = flags;

    if(0U != (flags & (IPA_INTF_FTFIF | IPA_INTF_TAEIF | IPA_INTF_WCFIF))) {
        IPA_CTL &= (uint32_t)(~(IPA_CTL_FTFIE | IPA_CTL_TAEIE | IPA_CTL_WCFIE));

        /* Also finish on error, otherwise LVGL would wait for the flush forever */
        if(1U == g_ipa_flag) {
            g_ipa_flag = 0U;
            /*IMPORTANT!!!
             *Inform the graphics library that you are ready with the flushing*/
            lv_display_flush_ready(g_disp);
        }
    }
}
#endif

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
