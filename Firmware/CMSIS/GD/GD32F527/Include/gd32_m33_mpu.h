/*!
    \file    cortex_m33_mpu.h
    \brief   definitions for the MPU

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

#ifndef GD32_M33_NS_MPU_H
#define GD32_M33_NS_MPU_H

#include "gd32f527.h"

#if (__MPU_PRESENT == 1)

/* MPU region init parameter struct definitions */
typedef struct
{
    uint8_t  region_number;           /*!< region number */
    uint32_t region_base_address;     /*!< region base address */
    uint32_t region_limit_address;    /*!< region limit address */
    uint8_t  access_permission;       /*!< access permissions(AP) field */
    uint8_t  instruction_exec;        /*!< execute never */
    uint8_t  shareability;            /*!< defines the shareability for Normal memory */
    uint8_t  attribute_index;         /*!< attribute index */
}mpu_region_init_struct;

typedef struct
{
  uint8_t attribute_number;           /*!< attribute number */
  uint8_t memory_type;                /*!< memory type */
  uint8_t outer_attributes;           /*!< outer memory attributes */
  uint8_t inner_attributes;           /*!< inner memory attributes */
} mpu_attribute_init_struct;

#define MPU_REGION_NUMBER0              ((uint8_t)0x00U)          /*!< MPU region number 0 */
#define MPU_REGION_NUMBER1              ((uint8_t)0x01U)          /*!< MPU region number 1 */
#define MPU_REGION_NUMBER2              ((uint8_t)0x02U)          /*!< MPU region number 2 */
#define MPU_REGION_NUMBER3              ((uint8_t)0x03U)          /*!< MPU region number 3 */
#define MPU_REGION_NUMBER4              ((uint8_t)0x04U)          /*!< MPU region number 4 */
#define MPU_REGION_NUMBER5              ((uint8_t)0x05U)          /*!< MPU region number 5 */
#define MPU_REGION_NUMBER6              ((uint8_t)0x06U)          /*!< MPU region number 6 */
#define MPU_REGION_NUMBER7              ((uint8_t)0x07U)          /*!< MPU region number 7 */

#define MPU_REGION_PRIVILEGED_RW        ((uint8_t)0x00U)          /*!< MPU region read/write by privileged code only */
#define MPU_REGION_ALL_RW               ((uint8_t)0x01U)          /*!< MPU region read/write by any privilege level  */
#define MPU_REGION_PRIVILEGED_RO        ((uint8_t)0x02U)          /*!< MPU region read-only by privileged code only  */
#define MPU_REGION_ALL_RO               ((uint8_t)0x03U)          /*!< MPU region read-only by any privilege level   */

#define MPU_ACCESS_NOT_SHAREABLE        ((uint8_t)0x00U)          /*!< MPU access shareable */
#define MPU_ACCESS_OUTER_SHAREABLE      ((uint8_t)0x01U)          /*!< MPU region outer shareable */
#define MPU_ACCESS_INNER_SHAREABLE      ((uint8_t)0x03U)          /*!< MPU region inner shareable */

#define MPU_INSTRUCTION_EXEC_PERMIT     ((uint8_t)0x00U)          /*!< execution of an instruction fetched from this region permitted */
#define MPU_INSTRUCTION_EXEC_NOT_PERMIT ((uint8_t)0x01U)          /*!< execution of an instruction fetched from this region not permitted */

#define MPU_MODE_HFNMI_PRIVDEF_NONE     ((uint32_t)0x00000000U)   /*!< HFNMIENA and PRIVDEFENA are 0 */
#define MPU_MODE_HARDFAULT_NMI          MPU_CTRL_HFNMIENA_Msk     /*!< use the MPU for memory accesses by HardFault and NMI handlers only */
#define MPU_MODE_PRIV_DEFAULT           MPU_CTRL_PRIVDEFENA_Msk   /*!< enables the default memory map as a background region for privileged access only */
#define MPU_MODE_HFNMI_PRIVDEF          ((uint32_t)MPU_CTRL_HFNMIENA_Msk | MPU_CTRL_PRIVDEFENA_Msk) /*!< HFNMIENA and PRIVDEFENA are 1 */

#define MPU_ATTRIBUTE_NUMBER0           ((uint8_t)0x00U)          /*!< MPU attribute number 0 */
#define MPU_ATTRIBUTE_NUMBER1           ((uint8_t)0x01U)          /*!< MPU attribute number 1 */
#define MPU_ATTRIBUTE_NUMBER2           ((uint8_t)0x02U)          /*!< MPU attribute number 2 */
#define MPU_ATTRIBUTE_NUMBER3           ((uint8_t)0x03U)          /*!< MPU attribute number 3 */
#define MPU_ATTRIBUTE_NUMBER4           ((uint8_t)0x04U)          /*!< MPU attribute number 4 */
#define MPU_ATTRIBUTE_NUMBER5           ((uint8_t)0x05U)          /*!< MPU attribute number 5 */
#define MPU_ATTRIBUTE_NUMBER6           ((uint8_t)0x06U)          /*!< MPU attribute number 6 */
#define MPU_ATTRIBUTE_NUMBER7           ((uint8_t)0x07U)          /*!< MPU attribute number 7 */

#define MPU_MEMORY_DEVICE               ((uint8_t)0x00U)          /*!< MPU device memory */
#define MPU_MEMORY_NORMAL               ((uint8_t)0x01U)          /*!< MPU normal memory */

#define MPU_DEVICE_nGnRnE               ((uint8_t)0x00U)          /*!< Device, noGather, noReorder, noEarly acknowledge */
#define MPU_DEVICE_nGnRE                ((uint8_t)0x04U)          /*!< Device, noGather, noReorder, Early acknowledge*/
#define MPU_DEVICE_nGRE                 ((uint8_t)0x08U)          /*!< Device, noGather, Reorder, Early acknowledge */
#define MPU_DEVICE_GRE                  ((uint8_t)0x0CU)          /*!< Device, Gather, Reorder, Early acknowledge */

#define MPU_OUTER_DEVICE                       ((uint8_t)0x00U)        /*!< Device memory */
#define MPU_NORMAL_OUTER_WT_TRAN_W_ALLOC       ((uint8_t)0x10U)        /*!< Normal memory, Outer write-through transient, write with allocate */
#define MPU_NORMAL_OUTER_WT_TRAN_R_ALLOC       ((uint8_t)0x20U)        /*!< Normal memory, Outer write-through transient, read with allocate */
#define MPU_NORMAL_OUTER_WT_TRAN_RW_ALLOC      ((uint8_t)0x30U)        /*!< Normal memory, Outer write-through transient, read and write with allocate */
#define MPU_NORMAL_OUTER_NON_CACHEABLE         ((uint8_t)0x40U)        /*!< Normal memory, Outer non-cacheable */
#define MPU_NORMAL_OUTER_WB_TRAN_W_ALLOC       ((uint8_t)0x50U)        /*!< Normal memory, Outer write-back transient, write with allocate */
#define MPU_NORMAL_OUTER_WB_TRAN_R_ALLOC       ((uint8_t)0x60U)        /*!< Normal memory, Outer write-back transient, read with allocate */
#define MPU_NORMAL_OUTER_WB_TRAN_RW_ALLOC      ((uint8_t)0x70U)        /*!< Normal memory, Outer write-back transient, read and write with allocate */
#define MPU_NORMAL_OUTER_WT_NON_TRAN_NO_ALLOC  ((uint8_t)0x80U)        /*!< Normal memory, Outer write-through non-transient, no allocate */
#define MPU_NORMAL_OUTER_WT_NON_TRAN_W_ALLOC   ((uint8_t)0x90U)        /*!< Normal memory, Outer write-back non-transient, write with allocate */
#define MPU_NORMAL_OUTER_WT_NON_TRAN_R_ALLOC   ((uint8_t)0xA0U)        /*!< Normal memory, Outer write-back non-transient, read with allocate */
#define MPU_NORMAL_OUTER_WT_NON_TRAN_RW_ALLOC  ((uint8_t)0xB0U)        /*!< Normal memory, Outer write-back non-transient, read and write with allocate */
#define MPU_NORMAL_OUTER_WB_NON_TRAN_NO_ALLOC  ((uint8_t)0xC0U)        /*!< Normal memory, Outer write-back non-transient, no allocate */
#define MPU_NORMAL_OUTER_WB_NON_TRAN_W_ALLOC   ((uint8_t)0xD0U)        /*!< Normal memory, Outer write-back non-transient, write with allocate */
#define MPU_NORMAL_OUTER_WB_NON_TRAN_R_ALLOC   ((uint8_t)0xE0U)        /*!< Normal memory, Outer write-back non-transient, read with allocate */
#define MPU_NORMAL_OUTER_WB_NON_TRAN_RW_ALLOC  ((uint8_t)0xF0U)        /*!< Normal memory, Outer write-back non-transient, read and write with allocate */

#define MPU_NORMAL_INNER_WT_TRAN_W_ALLOC       ((uint8_t)0x01U)        /*!< Normal memory, Inner write-through transient, write with allocate */
#define MPU_NORMAL_INNER_WT_TRAN_R_ALLOC       ((uint8_t)0x02U)        /*!< Normal memory, Inner write-through transient, read with allocate */
#define MPU_NORMAL_INNER_WT_TRAN_RW_ALLOC      ((uint8_t)0x03U)        /*!< Normal memory, Inner write-through transient, read and write with allocate */
#define MPU_NORMAL_INNER_NON_CACHEABLE         ((uint8_t)0x04U)        /*!< Normal memory, Inner non-cacheable */
#define MPU_NORMAL_INNER_WB_TRAN_W_ALLOC       ((uint8_t)0x05U)        /*!< Normal memory, Inner write-back transient, write with allocate */
#define MPU_NORMAL_INNER_WB_TRAN_R_ALLOC       ((uint8_t)0x06U)        /*!< Normal memory, Inner write-back transient, read with allocate */
#define MPU_NORMAL_INNER_WB_TRAN_RW_ALLOC      ((uint8_t)0x07U)        /*!< Normal memory, Inner write-back transient, read and write with allocate */
#define MPU_NORMAL_INNER_WT_NON_TRAN_NO_ALLOC  ((uint8_t)0x08U)        /*!< Normal memory, Inner write-through non-transient */
#define MPU_NORMAL_INNER_WT_NON_TRAN_W_ALLOC   ((uint8_t)0x09U)        /*!< Normal memory, Inner write-back non-transient, write with allocate */
#define MPU_NORMAL_INNER_WT_NON_TRAN_R_ALLOC   ((uint8_t)0x0AU)        /*!< Normal memory, Inner write-back non-transient, read with allocate */
#define MPU_NORMAL_INNER_WT_NON_TRAN_RW_ALLOC  ((uint8_t)0x0BU)        /*!< Normal memory, Inner write-back non-transient, read and write with allocate */
#define MPU_NORMAL_INNER_WB_NON_TRAN_NO_ALLOC  ((uint8_t)0x0CU)        /*!< Normal memory, Inner write-back non-transient, no allocate */
#define MPU_NORMAL_INNER_WB_NON_TRAN_W_ALLOC   ((uint8_t)0x0DU)        /*!< Normal memory, Inner write-back non-transient, write with allocate */
#define MPU_NORMAL_INNER_WB_NON_TRAN_R_ALLOC   ((uint8_t)0x0EU)        /*!< Normal memory, Inner write-back non-transient, read with allocate */
#define MPU_NORMAL_INNER_WB_NON_TRAN_RW_ALLOC  ((uint8_t)0x0FU)        /*!< Normal memory, Inner write-back non-transient, read and write with allocate */

/* function declarations */
/* initialize mpu_region_init_struct with the default values */
void mpu_region_struct_para_init(mpu_region_init_struct *region_init_struct);
/* initialize mpu_region_init_struct with the default values */
void mpu_attribute_struct_para_init(mpu_attribute_init_struct *attribute_init_struct);
/* configure the MPU region */
void mpu_region_config(mpu_region_init_struct *region_init_struct);
/* configure the MPU attribute */
void mpu_attribute_config(mpu_attribute_init_struct *attribute_init_struct);
/* enable the MPU region */
void mpu_region_enable(void);
#endif /* __MPU_PRESENT */

#endif /* GD32H7XX_MISC_H */
