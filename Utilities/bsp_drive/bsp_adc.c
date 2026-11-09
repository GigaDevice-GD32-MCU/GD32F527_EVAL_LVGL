#include "bsp_ADC.h"
#include "systick.h"

int32_t temperature_value_25;
int32_t temperature_value_minus40;
/*!
    \brief      configure the ADC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void adc_config(void)
{
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC0);
    /* config ADC clock */
    adc_clock_config(ADC_ADCCK_PCLK2_DIV4);
    
    /* ADC channel length config */
    adc_channel_length_config(ADC0,ADC_INSERTED_CHANNEL,3);

    /* ADC temperature sensor channel config */
    adc_inserted_channel_config(ADC0,0,ADC_CHANNEL_16,ADC_SAMPLETIME_480);
    /* ADC internal reference voltage channel config */
    adc_inserted_channel_config(ADC0,1,ADC_CHANNEL_17,ADC_SAMPLETIME_480);
    /* ADC 1/4 voltate of external battery config */
    adc_inserted_channel_config(ADC0,2,ADC_CHANNEL_18,ADC_SAMPLETIME_480);

    /* ADC external trigger enable */
    adc_external_trigger_config(ADC0,ADC_INSERTED_CHANNEL,DISABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0,ADC_DATAALIGN_RIGHT);
    /* ADC SCAN function enable */
    adc_special_function_config(ADC0,ADC_SCAN_MODE,ENABLE);
    /* ADC Vbat channel enable */
    adc_channel_16_to_18(ADC_VBAT_CHANNEL_SWITCH,ENABLE);
    /* ADC temperature and Vrefint enable */
    adc_channel_16_to_18(ADC_TEMP_VREF_CHANNEL_SWITCH,ENABLE);
    
    /* enable ADC interface */
    adc_enable(ADC0);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
}

float Get_temperature(void)
{
    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC0,ADC_INSERTED_CHANNEL);
    /* delay a time in milliseconds */
//    delay_1ms(100);
    /* value convert  */
    return ((1.225-ADC_IDATA0(ADC0)*3.3/4096)*1000/3.9+25);
}