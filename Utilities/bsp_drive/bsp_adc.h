#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "gd32f527.h"

#define ADC_TEMP_CALIBRATION_VALUE_25                REG16(0x1FF0F7C0)
#define ADC_TEMP_CALIBRATION_VALUE_MINUS40           REG16(0x1FF0F7C2)

float Get_temperature(void);
void adc_config(void);
#endif