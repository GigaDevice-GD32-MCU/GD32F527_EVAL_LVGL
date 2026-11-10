/*!
    \file    touch_panel.c 
    \brief   LCD touch panel functions

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

#include "touch_panel.h"
#include "gd32f527b_eval.h"
#include "systick.h"
#include "math.h"
#include <stdlib.h>
#include <stdio.h>

int fputc(int ch, FILE *f)
{
    (void)f;
    usart_data_transmit(EVAL_COM0, (uint8_t)ch);
    while(RESET == usart_flag_get(EVAL_COM0, USART_FLAG_TBE)) {
    }
    return ch;
}

/* number of filter reads */
#define FILTER_READ_TIMES           5
/* lost value of filter */
#define FILTER_LOST_VAL             1
/* error range of AD sample value */  
#define AD_ERR_RANGE                6
/* prevent a broken SPI state from blocking the LVGL input loop forever */
#define TOUCH_SPI_TIMEOUT           1000000U

int16_t touch_ad_x=0,touch_ad_y=0;

/* set or reset touch screen chip select pin */
static void spi_cs(uint8_t a);
/* release the touch controller chip select */
static void spi_stop(void);
/* exchange one byte through hardware SPI4 */
static uint8_t spi_transfer(uint8_t data);
/* SPI delay function */
static void spi_delay(uint16_t i);

/*!
    \brief      set or reset touch screen chip select pin
    \param[in]  a: specified the low or high level of chip select pin output
    \param[out] none
    \retval     none
*/
static void spi_cs(uint8_t a)
{
    if(a){
        SPI_TOUCH_CS_HIGH();
    }else{
        SPI_TOUCH_CS_LOW();
    }
}

/* release the touch controller chip select */
static void spi_stop(void)
{
    spi_cs(1);
}

static uint8_t spi_transfer(uint8_t data)
{
    uint32_t timeout = TOUCH_SPI_TIMEOUT;

    /* Restore the master state before every transaction. */
    spi_nss_internal_high(TOUCH_SPI_PERIPH);
    spi_enable(TOUCH_SPI_PERIPH);

    while((RESET == spi_i2s_flag_get(TOUCH_SPI_PERIPH, SPI_FLAG_TBE)) &&
          (timeout > 0U)) {
        timeout--;
    }
    if(timeout == 0U) {
        spi_stop();
        return 0U;
    }

    spi_i2s_data_transmit(TOUCH_SPI_PERIPH, data);

    timeout = TOUCH_SPI_TIMEOUT;
    while((RESET == spi_i2s_flag_get(TOUCH_SPI_PERIPH, SPI_FLAG_RBNE)) &&
          (timeout > 0U)) {
        timeout--;
    }
    if(timeout == 0U) {
        spi_stop();
        return 0U;
    }

    return (uint8_t)spi_i2s_data_receive(TOUCH_SPI_PERIPH);
}

/*!
    \brief      SPI delay function
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void spi_delay(uint16_t i)
{
    __IO uint16_t k;
    for(k=0;k<i;k++);
}

/*!
    \brief      configure touch panel GPIO 
    \param[in]  none 
    \param[out] none
    \retval     none
*/
void touch_panel_gpio_config(void)
{
    spi_parameter_struct spi_init_struct;

    /* GPIO clock enable */
    rcu_periph_clock_enable(SPI_SCK_CLK);
    rcu_periph_clock_enable(SPI_MOSI_CLK);
    rcu_periph_clock_enable(SPI_MISO_CLK);
    rcu_periph_clock_enable(SPI_TOUCH_CS_CLK);
    rcu_periph_clock_enable(TOUCH_PEN_INT_CLK);
    rcu_periph_clock_enable(RCU_SPI4);
    
    /* configure SPI4 SCK(PH6) pin */
    gpio_af_set(SPI_SCK_PORT, TOUCH_SPI_AF, SPI_SCK_PIN);
    gpio_mode_set(SPI_SCK_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SPI_SCK_PIN);
    gpio_output_options_set(SPI_SCK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_SCK_PIN);
    /* configure SPI4 MOSI(PF9) pin */
    gpio_af_set(SPI_MOSI_PORT, TOUCH_SPI_AF, SPI_MOSI_PIN);
    gpio_mode_set(SPI_MOSI_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SPI_MOSI_PIN);
    gpio_output_options_set(SPI_MOSI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_MOSI_PIN);
    /* configure SPI4 MISO(PH7) pin */
    gpio_af_set(SPI_MISO_PORT, TOUCH_SPI_AF, SPI_MISO_PIN);
    gpio_mode_set(SPI_MISO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, SPI_MISO_PIN);

    /* configure chip select(SPI-Touch) pin */
    gpio_mode_set(SPI_TOUCH_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, SPI_TOUCH_CS_PIN);
    gpio_output_options_set(SPI_TOUCH_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SPI_TOUCH_CS_PIN);
    
    /* configure touch pen IRQ pin */
    gpio_mode_set(TOUCH_PEN_INT_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, TOUCH_PEN_INT_PIN);
    
    /* set chip select pin high */
    SPI_TOUCH_CS_HIGH();

    /* SPI4 mode 0, 8-bit, MSB first, software NSS, low clock rate */
    spi_i2s_deinit(TOUCH_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);
    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.prescale = SPI_PSC_128;
    spi_init(TOUCH_SPI_PERIPH, &spi_init_struct);

    SPI_CTL0(TOUCH_SPI_PERIPH) |= SPI_CTL0_MSTMOD | SPI_CTL0_SWNSSEN |
                                  SPI_CTL0_SWNSS;
    SPI_CTL0(TOUCH_SPI_PERIPH) &= ~(SPI_CTL0_FF16 | SPI_CTL0_CKPH |
                                    SPI_CTL0_CKPL | SPI_CTL0_LF |
                                    SPI_CTL0_RO | SPI_CTL0_BDEN |
                                    SPI_CTL0_BDOEN);

    /* Keep the SPI peripheral's internal NSS high when using a GPIO CS. */
    spi_nss_internal_high(TOUCH_SPI_PERIPH);
    spi_enable(TOUCH_SPI_PERIPH);
}

/*!
    \brief      touch start
    \param[in]  none
    \param[out] none
    \retval     none
*/
void touch_start(void)
{
    spi_cs(0);
}

/*!
    \brief      write data to touch screen
    \param[in]  d: the data to be written
    \param[out] none
    \retval     none
*/
void touch_write(uint8_t d)
{
    (void)spi_transfer(d);
}

/*!
    \brief      read the touch AD value
    \param[in]  None
    \param[out] none
    \retval     the value of touch AD
*/
uint16_t touch_read(void)
{
    uint16_t buf;

    /* After the command, the controller returns one null bit, 12 data bits,
       and three trailing bits. */
    buf = (uint16_t)spi_transfer(0x00U) << 8;
    buf |= spi_transfer(0x00U);
    spi_stop();
    return (buf >> 3) & 0x0FFFU;
}

/*!
    \brief      read the touch pen interrupt request signal
    \param[in]  none
    \param[out] none
    \retval     the status of touch pen: SET or RESET
      \arg        SET: touch pen is inactive
      \arg        RESET: touch pen is active
*/
FlagStatus touch_pen_irq(void)
{
    return TOUCH_PEN_INT_READ();
}

/*!
    \brief      get the AD sample value of touch location at X coordinate
    \param[in]  none
    \param[out] none
    \retval     channel X+ AD sample value
*/
uint16_t touch_ad_x_get(void)
{
    if(RESET != touch_pen_irq()){
        /* touch pen is inactive */
        return 0;
    }
    touch_start();
    touch_write(CH_X);
    return (touch_read());
}

/*!
    \brief      get the AD sample value of touch location at Y coordinate
    \param[in]  none
    \param[out] none
    \retval     channel Y+ AD sample value
*/
uint16_t touch_ad_y_get(void)
{
    if(RESET != touch_pen_irq()){
        /* touch pen is inactive */
        return 0;
    }
    touch_start();
    touch_write(CH_Y);
    return (touch_read());
}

/*!
    \brief      get channel X+ AD average sample value
    \param[in]  none
    \param[out] none
    \retval     channel X+ AD average sample value
*/
uint16_t touch_average_ad_x_get(void)
{
    uint8_t i;
    uint16_t temp=0;
    for (i=0;i<8;i++){
        temp += touch_ad_x_get();
        spi_delay(1000);
    }
    temp>>=3;
    
    return temp;
}

/*!
    \brief      get channel Y+ AD average sample value
    \param[in]  none
    \param[out] none
    \retval     channel Y+ AD average sample value
*/
uint16_t touch_average_ad_y_get(void)
{
    uint8_t i;
    uint16_t temp=0;
    for (i=0;i<8;i++){
        temp += touch_ad_y_get();
        spi_delay(1000);
    }
    temp>>=3;

    return temp;
}

/*!
    \brief      get X coordinate value of touch point on LCD screen
    \param[in]  adx: channel X+ AD average sample value
    \param[out] none
    \retval     X coordinate value of touch point
*/
uint16_t touch_coordinate_x_get(uint16_t adx)
{
    if(adx <= AD_X_MIN) {
        return 0;
    }
    if(adx >= AD_X_MAX) {
        return LCD_X - 1;
    }
    return (uint16_t)(((uint32_t)(AD_X_MAX - adx) * (LCD_X - 1U)) /
                      (AD_X_MAX - AD_X_MIN));
}

/*!
    \brief      get Y coordinate value of touch point on LCD screen
    \param[in]  ady: channel Y+ AD average sample value
    \param[out] none
    \retval     Y coordinate value of touch point
*/
uint16_t touch_coordinate_y_get(uint16_t ady)
{
    if(ady <= AD_Y_MIN) {
        return LCD_Y - 1;
    }
    if(ady >= AD_Y_MAX) {
        return 0;
    }
    return (uint16_t)(((uint32_t)(AD_Y_MAX - ady) * (LCD_Y - 1U)) /
                      (AD_Y_MAX - AD_Y_MIN));
}

/*!
    \brief      get a value (X or Y) for several times. Order these values, 
                remove the lowest and highest and obtain the average value
    \param[in]  channel_select: select channel X or Y
      \arg        CH_X: channel X
      \arg        CH_Y: channel Y
    \param[out] none
    \retval     a value(X or Y) of touch point
*/
uint16_t touch_data_filter(uint8_t channel_select)
{
    uint16_t i=0, j=0; 
    uint16_t buf[FILTER_READ_TIMES]; 
    uint16_t sum=0; 
    uint16_t temp=0;
    /* read data in FILTER_READ_TIMES times */
    for(i=0; i < FILTER_READ_TIMES; i++){
        if(CH_X == channel_select){
            buf[i] = touch_ad_x_get();
        }else{
            /* CH_Y == channel_select */
            buf[i] = touch_ad_y_get();
        }
    }
    /* sort in ascending sequence */
    for(i = 0; i < FILTER_READ_TIMES - 1; i++){
        for(j = i + 1; j < FILTER_READ_TIMES; j++){
            if(buf[i] > buf[j]){
                temp = buf[i]; 
                buf[i] = buf[j]; 
                buf[j] = temp;
            }
        }
    }
    sum = 0;
    for(i = FILTER_LOST_VAL; i < FILTER_READ_TIMES - FILTER_LOST_VAL; i++){
        sum += buf[i];
    }
    temp = sum / (FILTER_READ_TIMES - 2 * FILTER_LOST_VAL);
    
    return temp;
}

/*!
    \brief      get the AD sample value of touch location. 
                get the sample value for several times,order these values,remove the lowest and highest and obtain the average value
    \param[in]  channel_select: select channel X or Y
    \param[out] none
      \arg        ad_x: channel X AD sample value
      \arg        ad_y: channel Y AD sample value
    \retval     ErrStatus: SUCCESS or ERROR
*/
ErrStatus touch_ad_xy_get(int16_t *ad_x, int16_t *ad_y)
{
    uint16_t ad_x1=0, ad_y1=0, ad_x2=0, ad_y2=0; 

    ad_x1 = touch_data_filter(CH_X); 
    ad_y1 = touch_data_filter(CH_Y); 
    ad_x2 = touch_data_filter(CH_X); 
    ad_y2 = touch_data_filter(CH_Y);
    
    if((abs(ad_x1 - ad_x2) > AD_ERR_RANGE) || (abs(ad_y1 - ad_y2) > AD_ERR_RANGE)){
        return ERROR;
    }
    *ad_x = (ad_x1 + ad_x2) / 2; 
    *ad_y = (ad_y1 + ad_y2) / 2;
   
    return SUCCESS;
}

/*!
    \brief      detect the touch event
    \param[in]  none
    \param[out] none
    \retval     ErrStatus: SUCCESS or ERROR
*/
ErrStatus touch_scan(void)
{
    uint8_t invalid_count = 0;
    if (RESET == touch_pen_irq()){
        /* touch pen is active */
        while((SUCCESS != touch_ad_xy_get(&touch_ad_x, &touch_ad_y))&& (invalid_count < 20)){
            invalid_count++; 
        }
        
        if(invalid_count >= 20){ 
            touch_ad_x = -1; 
            touch_ad_y = -1;
            return ERROR;
        }
    }else{ 
        touch_ad_x = -1; 
        touch_ad_y = -1;
        return ERROR;
    }
    return SUCCESS;
}

void touch_debug_print_ad(void)
{
    uint16_t raw_x;
    uint16_t raw_y;

    printf("[TOUCH] IRQ=%u\r\n", (unsigned int)touch_pen_irq());
    if(RESET != touch_pen_irq()) {
        printf("[TOUCH] released\r\n");
        return;
    }

    raw_x = touch_ad_x_get();
    raw_y = touch_ad_y_get();
        printf("[TOUCH] raw_x=%u raw_y=%u coord=(%u,%u) raw_x_range=[%u,%u] raw_y_range=[%u,%u]\r\n",
           (unsigned int)raw_x, (unsigned int)raw_y,
            (unsigned int)touch_coordinate_x_get(raw_y),
            (unsigned int)touch_coordinate_y_get(raw_x),
            AD_Y_MIN, AD_Y_MAX, AD_X_MIN, AD_X_MAX);
}

