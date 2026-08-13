/*!
    \file    main.c
    \brief   TLI_IPA demo

    \version 2023-01-20, V0.1.0, demo for GD32H7xx
*/

/*
    Copyright (c) 2023, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

/* Lib*/
#include "gd32f527.h"
#include "gd32f527b_eval.h"
#include "gd32f527b_lcd_eval.h"
#include "gd32f527_rcu.h"
#include <stdio.h>
/* system Config*/
#include "sys_config.h"
/* bsp*/
#include "systick.h"
#include "exmc_sdram.h"
#include "bsp_adc.h"
#include "touch_panel.h"
/* lvgl*/
#if SYS_configUSE_LVGL
    #include "lvgl.h"
    #include "lv_port_disp.h"
    #include "lv_port_indev.h"
//    #include "lv_examples.h"
    #include "lv_demos.h"
#endif
/* FatFs*/
#if SYS_configUSE_FAT32FS
    #include "ff.h"
    #include "GUI_APP/fatfs_file_browse.h"
#endif

/* FreeRTOS*/
#if SYS_configUSE_FREERTOS
    #include "taskmain.h"
#endif

uint16_t back_to_home = 1;
float g_temperature = 0;

extern void lv_demo_app(void);
void RCU_PLLX_Config(void);

/* configure the ADC peripheral */
void adc_config(void);

/*!
    \brief      main program
    \param[in]  none
    \param[out] none
    \retval     none
*/


int main(void)
{
    uint32_t touch_debug_elapsed = 0U;

    /* enable the CPU Cache */
    // mpu_config();
    /* configure Systick and LCD */
    systick_config();
    gd_eval_com_init(EVAL_COM0);
    printf("[BOOT] USART0 ready, touch debug enabled\r\n");
    /* ADC configuration */
    adc_config();

#if SYS_configUSE_LVGL
    exmc_synchronous_dynamic_ram_init(EXMC_SDRAM_DEVICE0);
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
#endif


 #if SYS_configUSE_FAT32FS
     lv_fs_fatfs_init();
     scanfiles_test();
 #endif

 #if SYS_configUSE_LVGL
     lv_demo_app();
 #endif

#if SYS_configUSE_WATCHDOG
    dbg_periph_enable(DBG_FWDGT_HOLD);
    /* enable write access to FWDGT_PSC and FWDGT_RLD registers.
       FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625 KHz.
       FWDGT timeout is set to 1s (the timeout may varies due to 
                                   IRC40K frequency dispersion). */
    fwdgt_config(0xFFF, FWDGT_PSC_DIV256);
    fwdgt_enable();
#endif

#if SYS_configUSE_FREERTOS
    taskmainInit();
#endif

    while(1){
 #if !SYS_configUSE_FREERTOS
         delay_1ms(5);
     #if TOUCH_AD_DEBUG
         touch_debug_elapsed += 5U;
         if(touch_debug_elapsed >= 1000U) {
             touch_debug_elapsed = 0U;
             touch_debug_print_ad();
         }
     #endif
 #if SYS_configUSE_LVGL
         lv_task_handler();
 #endif

 #if SYS_configUSE_WATCHDOG
         fwdgt_counter_reload();
 #endif
       /* value convert */
       g_temperature = Get_temperature();
#endif
    }
}

/*!
    \brief      Configure PLL as the clock source for SDIO and EXMC peripherals at all times.
    \param[in]  none
    \param[out] none
    \retval     none
*/

void __aeabi_assert(const char *ptr1, const char *ptr2, int val)
{
}
