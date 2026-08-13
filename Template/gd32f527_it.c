/*!
    \file    gd32f527_it.c
    \brief   interrupt service routines

    \version 2023-10-16, V0.0.0, firmware for GD32F5xx
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

#include "gd32f527_it.h"
#include "main.h"
#include "systick.h"
#include "lvgl.h"
#include "sys_config.h"

#if SYS_configUSE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#ifdef  RESISTIVE_TOUCH_SCREEN
#include "touch_panel.h"
#else
#include "bsp_i2c_touch.h"
#include "bsp_ts_gt911.h"
#endif
#include "gd32f527b_eval.h"


#define MULTI_ECC_ERROR_HANDLE(s)    do{}while(1)
#define SINGLE_ECC_ERROR_HANDLE(s)   do{}while(1)

extern __IO int32_t OS_TimeMS;
extern uint32_t g_temperature;

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
    uint32_t msp;
    uint32_t psp;
    uint32_t exc_return;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t afsr;
    uint32_t ccr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t shcsr;
    uint32_t icsr;

    /* Candidates for debugging when EXC_RETURN/stack is corrupted. */
    uint32_t msp_pc;
    uint32_t msp_psr;
    uint32_t frame_offset; /* 0 = basic frame, 18 = extended FPU frame */
    uint32_t frame_address;
    uint32_t frame_word0;
    uint32_t frame_word1;
    uint32_t frame_word2;
    uint32_t frame_word3;
    uint32_t frame_word4;
    uint32_t frame_word5;
    uint32_t frame_word6;
    uint32_t frame_word7;
    uint32_t stack_used; /* 0 = unknown, 1 = MSP, 2 = PSP */
    uint32_t psp_pc;
    uint32_t psp_psr;
} fault_context_t;

volatile fault_context_t g_hardfault_ctx;

#if defined(__ICCARM__)

void HardFault_Handler(void)
{
    g_hardfault_ctx.cfsr = SCB->CFSR;
    g_hardfault_ctx.hfsr = SCB->HFSR;
    g_hardfault_ctx.dfsr = SCB->DFSR;
    g_hardfault_ctx.afsr = SCB->AFSR;
    g_hardfault_ctx.ccr = SCB->CCR;
    g_hardfault_ctx.mmfar = SCB->MMFAR;
    g_hardfault_ctx.bfar = SCB->BFAR;
    g_hardfault_ctx.shcsr = SCB->SHCSR;
    g_hardfault_ctx.icsr = SCB->ICSR;

    while(1)
    {
    }
}

#else

__attribute__((noinline, used)) static void HardFault_HandlerC(uint32_t *mspStack, uint32_t *pspStack, uint32_t exc_return);

/*!
    \brief    this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
//    if((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCME0)) ||
//         (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME1)) ||
//           (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME2)) ||
//             (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME3)) ||
//               (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME4)) ||
//                 (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME5)) ||
//                   (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCME6))) {
//                     MULTI_ECC_ERROR_HANDLE("SRAM or FLASH multi-bits non-correction ECC error\r\n");
//    }else if((SET == syscfg_interrupt_flag_get(SYSCFG_INT_FLAG_ECCSE0)) ||
//               (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE1)) ||
//                 (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE2)) ||
//                   (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE3)) ||
//                     (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE4)) ||
//                       (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE5)) ||
//                         (SET == syscfg_interrupt_flag_get( SYSCFG_INT_FLAG_ECCSE6))) {
//                           SINGLE_ECC_ERROR_HANDLE("SRAM or FLASH single bit correction ECC error\r\n");
//    }else{
//        /* if NMI exception occurs, go to infinite loop */
//        /* HXTAL clock monitor NMI error */
//        while(1) {
//        }
//    }
}

/*!
    \brief    this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile(
        "mrs   r0, msp                      \n" /* r0 = MSP */
        "mrs   r1, psp                      \n" /* r1 = PSP */
        "mov   r2, lr                       \n" /* r2 = EXC_RETURN */
        "b     HardFault_HandlerC           \n"
    );
}

static const uint32_t *prvGetCoreFrame(const uint32_t *sp, uint32_t exc_return)
{
    /* If bit[4] of EXC_RETURN is 0, an extended frame is present and the
     * stacked core registers start after the FP context (18 words).
     */
    return ((exc_return & (1UL << 4)) == 0U) ? (sp + 18U) : sp;
}

static uint32_t prvIsValidCoreFrame(const uint32_t *frame)
{
    const uint32_t pc = frame[6];
    const uint32_t xpsr = frame[7];

    if ((xpsr & (1UL << 24)) == 0U)
    {
        return 0U;
    }

    /* Accept the normal internal Flash aliases used by GD32F527 projects. */
    if (((pc >= 0x08000000UL) && (pc < 0x09000000UL)) ||
        (pc < 0x01000000UL))
    {
        return 1U;
    }

    return 0U;
}

static const uint32_t *prvSelectCoreFrame(const uint32_t *sp, uint32_t exc_return,
                                          volatile uint32_t *frame_offset)
{
    const uint32_t *preferred = prvGetCoreFrame(sp, exc_return);
    const uint32_t *candidate;
    const uint32_t offset = ((exc_return & (1UL << 4)) == 0U) ? 18U : 0U;

    /* The optional alignment word is appended after xPSR; it does not change
     * the position of r0 in the exception frame. */
    candidate = sp + offset;
    if (prvIsValidCoreFrame(candidate) != 0U)
    {
        *frame_offset = offset;
        return candidate;
    }

    *frame_offset = offset;
    return preferred;
}

static void HardFault_HandlerC(uint32_t *mspStack, uint32_t *pspStack, uint32_t exc_return)
{
    /* Store raw stack pointers and EXC_RETURN. */
    g_hardfault_ctx.msp = (uint32_t)mspStack;
    g_hardfault_ctx.psp = (uint32_t)pspStack;
    g_hardfault_ctx.exc_return = exc_return;

    /* EXC_RETURN bit[2] identifies the stack used by the interrupted code:
     * 0 = MSP, 1 = PSP. Do not select a frame by guessing which stack looks
     * plausible; the unused stack can contain arbitrary data. */
    uint32_t *pulFaultStackAddress;
    if ((exc_return & (1UL << 2)) != 0U)
    {
        pulFaultStackAddress = pspStack;
        g_hardfault_ctx.stack_used = 2;
    }
    else
    {
        pulFaultStackAddress = mspStack;
        g_hardfault_ctx.stack_used = 1;
    }

    /* Only inspect the selected stack. The other stack may be uninitialized,
     * especially before the scheduler starts, and must not be dereferenced. */
    {
        const uint32_t *selectedCore = prvSelectCoreFrame(pulFaultStackAddress, exc_return,
                                  &g_hardfault_ctx.frame_offset);
        if (g_hardfault_ctx.stack_used == 1U)
        {
            g_hardfault_ctx.msp_pc = selectedCore[6];
            g_hardfault_ctx.msp_psr = selectedCore[7];
        }
        else
        {
            g_hardfault_ctx.psp_pc = selectedCore[6];
            g_hardfault_ctx.psp_psr = selectedCore[7];
        }
    }

    {
        uint32_t *core = (uint32_t *)prvSelectCoreFrame(pulFaultStackAddress, exc_return,
                                &g_hardfault_ctx.frame_offset);
        g_hardfault_ctx.frame_address = (uint32_t)core;
        g_hardfault_ctx.frame_word0 = core[0];
        g_hardfault_ctx.frame_word1 = core[1];
        g_hardfault_ctx.frame_word2 = core[2];
        g_hardfault_ctx.frame_word3 = core[3];
        g_hardfault_ctx.frame_word4 = core[4];
        g_hardfault_ctx.frame_word5 = core[5];
        g_hardfault_ctx.frame_word6 = core[6];
        g_hardfault_ctx.frame_word7 = core[7];
        g_hardfault_ctx.r0 = core[0];
        g_hardfault_ctx.r1 = core[1];
        g_hardfault_ctx.r2 = core[2];
        g_hardfault_ctx.r3 = core[3];
        g_hardfault_ctx.r12 = core[4];
        g_hardfault_ctx.lr = core[5];
        g_hardfault_ctx.pc = core[6];
        g_hardfault_ctx.psr = core[7];
    }

    g_hardfault_ctx.cfsr = SCB->CFSR;
    g_hardfault_ctx.hfsr = SCB->HFSR;
    g_hardfault_ctx.dfsr = SCB->DFSR;
    g_hardfault_ctx.afsr = SCB->AFSR;
    g_hardfault_ctx.ccr = SCB->CCR;
    g_hardfault_ctx.mmfar = SCB->MMFAR;
    g_hardfault_ctx.bfar = SCB->BFAR;
    g_hardfault_ctx.shcsr = SCB->SHCSR;
    g_hardfault_ctx.icsr = SCB->ICSR;

    /* Stop here so the debugger can inspect g_hardfault_ctx. */
    while(1)
    {
    }
}

#endif

/*!
    \brief    this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1) {
    }
}

/*!
    \brief    this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1) {
    }
}

/*
    \brief    this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    g_hardfault_ctx.msp = __get_MSP();
    g_hardfault_ctx.psp = __get_PSP();
    g_hardfault_ctx.cfsr = SCB->CFSR;
    g_hardfault_ctx.hfsr = SCB->HFSR;
    g_hardfault_ctx.dfsr = SCB->DFSR;
    g_hardfault_ctx.afsr = SCB->AFSR;
    g_hardfault_ctx.ccr = SCB->CCR;
    g_hardfault_ctx.mmfar = SCB->MMFAR;
    g_hardfault_ctx.bfar = SCB->BFAR;
    g_hardfault_ctx.shcsr = SCB->SHCSR;
    g_hardfault_ctx.icsr = SCB->ICSR;

    while(1) {
    }
}

/*
    \brief    this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
    /* if DebugMon exception occurs, go to infinite loop */
    while(1) {
    }
}

/*
    \brief    this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
#if SYS_configUSE_FREERTOS
    if(xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
#endif
    delay_decrement();
#if SYS_configUSE_LVGL
    lv_tick_inc(1);
#endif
}

void GTP_IRQHandler(void)
{
//    if(exti_interrupt_flag_get(GTP_INT_EXTI_LINE) != RESET) //?????"?EXTI Line??
//    {
//        GTP_TouchProcess();
//        exti_interrupt_flag_clear(GTP_INT_EXTI_LINE);     //???????
//    }
}

