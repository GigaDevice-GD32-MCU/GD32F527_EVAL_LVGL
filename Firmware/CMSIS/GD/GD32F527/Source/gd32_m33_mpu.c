/*!
    \file    cortex_m33_mpu.c
    \brief   MPU driver

    \version 2024-04-28, V1.0.0, firmware for GD32F5xx
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

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
#include "gd32_m33_mpu.h"

#if (__MPU_PRESENT == 1)

/*!
    \brief      initialize mpu_region_init_struct with the default values
    \param[in]  region_init_struct: pointer to a mpu_region_init_struct structure
    \param[out] none
    \retval     none
*/
void mpu_region_struct_para_init(mpu_region_init_struct *region_init_struct)
{
    region_init_struct->region_number        = MPU_REGION_NUMBER0;
    region_init_struct->region_base_address  = 0x00000000U;
    region_init_struct->region_limit_address = 0x00000000U;
    region_init_struct->instruction_exec     = MPU_INSTRUCTION_EXEC_PERMIT;
    region_init_struct->access_permission    = MPU_REGION_PRIVILEGED_RW;
    region_init_struct->shareability         = MPU_ACCESS_NOT_SHAREABLE;
    region_init_struct->attribute_index      = MPU_ATTRIBUTE_NUMBER0;
}

/*!
    \brief      initialize mpu_attribute_init_struct with the default values
    \param[in]  attribute_init_struct: pointer to a mpu_attribute_init_struct structure
    \param[out] none
    \retval     none
*/
void mpu_attribute_struct_para_init(mpu_attribute_init_struct *attribute_init_struct)
{
    attribute_init_struct->attribute_number = MPU_REGION_NUMBER0;
    attribute_init_struct->memory_type      = MPU_OUTER_DEVICE;
    attribute_init_struct->inner_attributes = MPU_DEVICE_nGnRnE;
    attribute_init_struct->outer_attributes = 0U;
}

/*!
    \brief      configure the MPU region
    \param[in]  region_init_struct: MPU initialization structure
                  region_number: region number
                                 MPU_REGION_NUMBERn (n=0,..,7)
                  region_base_address: region base address
                  region_limit_address: region limit address
                  access_permission: MPU_REGION_PRIVILEGED_RW, MPU_REGION_ALL_RW, MPU_AP_PRIV_RW_UNPRIV_RO, MPU_REGION_ALL_RO
                  instruction_exec: MPU_INSTRUCTION_EXEC_PERMIT, MPU_INSTRUCTION_EXEC_NOT_PERMIT
                  shareability: MPU_ACCESS_NOT_SHAREABLE, MPU_ACCESS_OUTER_SHAREABLE, MPU_ACCESS_INNER_SHAREABLE
                  attribute_index: attribute index
                                   MPU_ATTRIBUTE_NUMBERn (n=0,..,7)
    \param[out] none
    \retval     none
*/
void mpu_region_config(mpu_region_init_struct *region_init_struct)
{
    __DMB();
    /* select the Region number  */
    MPU->RNR = region_init_struct->region_number;
    /* disable the Region  */
    MPU->RLAR &= ~(MPU_RLAR_EN_Msk);
    MPU->RBAR = (((uint32_t)region_init_struct->region_base_address & 0xFFFFFFE0UL)   | 
                 ((uint32_t)region_init_struct->shareability << MPU_RBAR_SH_Pos)      |
                 ((uint32_t)region_init_struct->access_permission << MPU_RBAR_AP_Pos) |
                 ((uint32_t)region_init_struct->instruction_exec << MPU_RBAR_XN_Pos)) ;
    
    MPU->RLAR = (((uint32_t)region_init_struct->region_limit_address & 0xFFFFFFE0UL)      |
                 ((uint32_t)region_init_struct->attribute_index << MPU_RLAR_AttrIndx_Pos));
}

/*!
    \brief      configure the MPU attribute
    \param[in]  attribute_init_struct: MPU attribute initialization structure
                  attribute_number: attribute number
                                 MPU_ATTRIBUTE_NUMBERn (n=0,..,7)
                  memory_type: memory type
                                 MPU_MEMORY_DEVICE, MPU_MEMORY_NORMAL
                  outer_attributes: outer memory attributes
                                 MPU_OUTER_DEVICE
                                 MPU_NORMAL_OUTER_WT_TRAN_W_ALLOC
                                 MPU_NORMAL_OUTER_WT_TRAN_R_ALLOC
                                 MPU_NORMAL_OUTER_WT_TRAN_RW_ALLOC
                                 MPU_NORMAL_OUTER_NON_CACHEABLE
                                 MPU_NORMAL_OUTER_WB_TRAN_W_ALLOC
                                 MPU_NORMAL_OUTER_WB_TRAN_R_ALLOC
                                 MPU_NORMAL_OUTER_WB_TRAN_RW_ALLOC
                                 MPU_NORMAL_OUTER_WT_NON_TRAN_NO_ALLOC
                                 MPU_NORMAL_OUTER_WT_NON_TRAN_W_ALLOC
                                 MPU_NORMAL_OUTER_WT_NON_TRAN_R_ALLOC
                                 MPU_NORMAL_OUTER_WT_NON_TRAN_RW_ALLOC
                                 MPU_NORMAL_OUTER_WB_NON_TRAN_NO_ALLOC
                                 MPU_NORMAL_OUTER_WB_NON_TRAN_W_ALLOC
                                 MPU_NORMAL_OUTER_WB_NON_TRAN_R_ALLOC
                                 MPU_NORMAL_OUTER_WB_NON_TRAN_RW_ALLOC
                  inner_attributes: inner memory attributes
                                 MPU_NORMAL_INNER_WT_TRAN_W_ALLOC
                                 MPU_NORMAL_INNER_WT_TRAN_R_ALLOC
                                 MPU_NORMAL_INNER_WT_TRAN_RW_ALLOC
                                 MPU_NORMAL_INNER_NON_CACHEABLE
                                 MPU_NORMAL_INNER_WB_TRAN_W_ALLOC
                                 MPU_NORMAL_INNER_WB_TRAN_R_ALLOC
                                 MPU_NORMAL_INNER_WB_TRAN_RW_ALLOC
                                 MPU_NORMAL_INNER_WT_NON_TRAN_NO_ALLOC
                                 MPU_NORMAL_INNER_WT_NON_TRAN_W_ALLOC
                                 MPU_NORMAL_INNER_WT_NON_TRAN_R_ALLOC
                                 MPU_NORMAL_INNER_WT_NON_TRAN_RW_ALLOC
                                 MPU_NORMAL_INNER_WB_NON_TRAN_NO_ALLOC
                                 MPU_NORMAL_INNER_WB_NON_TRAN_W_ALLOC
                                 MPU_NORMAL_INNER_WB_NON_TRAN_R_ALLOC
                                 MPU_NORMAL_INNER_WB_NON_TRAN_RW_ALLOC
    \param[out] none
    \retval     none
*/
void mpu_attribute_config(mpu_attribute_init_struct *attribute_init_struct)
{
    uint32_t attr_values;
    uint8_t  attr_x_value;

    __DMB();
    if(attribute_init_struct->attribute_number < MPU_ATTRIBUTE_NUMBER4) {
        /* configure MPU_MAIR0 */
        attr_values = MPU->MAIR0;
        attr_values &=  ~(0xFFUL << (attribute_init_struct->attribute_number * 8U));
        if(MPU_MEMORY_DEVICE == attribute_init_struct->memory_type) {
            attr_x_value = (attribute_init_struct->inner_attributes & 0x0CU);
        }else{
            attr_x_value = attribute_init_struct->inner_attributes | attribute_init_struct->outer_attributes;;
        }
        attr_values |= (attr_x_value  << (attribute_init_struct->attribute_number * 8U));
        MPU->MAIR0 = attr_values;
    } else {
        /* configure MPU_MAIR0 */
        attr_values = MPU->MAIR1;
        attr_values &=  ~(0xFFUL << ((attribute_init_struct->attribute_number -4U) * 8U));
        if(MPU_MEMORY_DEVICE == attribute_init_struct->memory_type) {
            attr_x_value = (attribute_init_struct->inner_attributes & 0x0CU);
        }else{
            attr_x_value = attribute_init_struct->inner_attributes | attribute_init_struct->outer_attributes;
        }
        attr_values |= (attr_x_value  << ((attribute_init_struct->attribute_number - 4U) * 8U));
        MPU->MAIR1 = attr_values;
    }
}

/*!
    \brief      enable the MPU region
    \param[in]  none
    \param[out] none
    \retval     none
*/
void mpu_region_enable(void)
{
    MPU->RLAR |= MPU_RLAR_EN_Msk;
}

#endif /* __MPU_PRESENT */

