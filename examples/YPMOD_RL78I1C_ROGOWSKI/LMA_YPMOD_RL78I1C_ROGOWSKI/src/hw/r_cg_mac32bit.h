/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2015, 2021 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_mac32bit.h
* Version      : Applilet4 for RL78/I1C V1.01.07.02 [08 Nov 2021]
* Device(s)    : R5F11TLG
* Tool-Chain   : CCRL
* Description  : This file implements device driver for MAC32bit module.
* Creation Date: 06/03/2025
***********************************************************************************************************************/
#ifndef MAC32BIT_H
#define MAC32BIT_H

#include <stdint.h>

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/
/*
    Multiplication control register (MULC) 
*/
/* Selection of operation mode (MACMODE,MULSM) */
#define _00_MAC32BIT_MUL_UNSIGNED_MODE              (0x00U) /* multiplication mode (unsigned) */
#define _40_MAC32BIT_MUL_SIGNED_MODE                (0x40U) /* multiplication mode (signed) */
#define _80_MAC32BIT_MUL_ACC_UNSIGNED_MODE          (0x80U) /* multiply-accumulation mode (unsigned) */
#define _C0_MAC32BIT_MUL_ACC_SIGNED_MODE            (0xC0U) /* multiply-accumulation mode (signed) */
/* Selection of fixed point mode (MULFRAC) */
#define _00_MAC32BIT_FIXED_POINT_DISABLED           (0x00U) /* fixed point mode disabled */
#define _10_MAC32BIT_FIXED_POINT_ENABLED            (0x10U) /* fixed point mode enabled */
/* Overflow/underflow flag of multiply-accumulation result (accumulation value) (MACOF) */
#define _00_MAC32BIT_NO_FLOW_OCCURRED               (0x00U) /* no overflow/underflow occurred */
#define _04_MAC32BIT_FLOW_OCCURRED                  (0x04U) /* overflow/underflow occurred */
/* Sign flag of multiply-accumulation result (accumulation value) (MACSF) */
#define _00_MAC32BIT_POSITIVE_ACC_VALUE             (0x00U) /* positive accumulation value */
#define _02_MAC32BIT_NEGATIVE_ACC_VALUE             (0x02U) /* negative accumulation value */
/* Operation processing status bit (MULST) */
#define _00_MAC32BIT_PROCESS_COMPLETED              (0x00U) /* completion of operation processing */
#define _01_MAC32BIT_PROCESS_UNDERWAY               (0x01U) /* during operation processing */


/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
typedef struct
{
    uint16_t low_low;
    uint16_t low_high;
    uint16_t high_low;
    uint16_t high_high;
} mac32bit_uint64_t;
typedef struct
{
    int16_t low_low;
    int16_t low_high;
    int16_t high_low;
    int16_t high_high;
} mac32bit_int64_t;

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_MAC32Bit_Create(void);
/* Start user code for function. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#endif
