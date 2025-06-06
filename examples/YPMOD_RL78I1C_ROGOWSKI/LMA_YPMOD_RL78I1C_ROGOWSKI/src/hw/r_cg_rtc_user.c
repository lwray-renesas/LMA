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
* File Name    : r_cg_rtc_user.c
* Version      : Applilet4 for RL78/I1C V1.01.07.02 [08 Nov 2021]
* Device(s)    : R5F11TLG
* Tool-Chain   : CCRL
* Description  : This file implements device driver for RTC module.
* Creation Date: 21/05/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_rtc.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_rtc_periodicinterrupt(vect=INTRTCPRD)
/* Start user code for pragma. Do not edit comment generated here */
#include "LMA_Core.h"
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
volatile rtc_capture_value_t g_rtc_calendar_rtcic0;
volatile rtc_capture_value_t g_rtc_calendar_rtcic1;
volatile rtc_capture_value_t g_rtc_calendar_rtcic2;
volatile uint32_t g_rtc_binary_rtcic0;
volatile uint32_t g_rtc_binary_rtcic1;
volatile uint32_t g_rtc_binary_rtcic2;
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */


/***********************************************************************************************************************
* Function Name: r_rtc_periodicinterrupt
* Description  : None
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_rtc_periodicinterrupt(void)
{
    volatile uint8_t tmp;

    if (_04_RTC_PERIODIC_INTERRUPT == (RSR & _04_RTC_PERIODIC_INTERRUPT))
    {
        tmp = RSR;
        tmp &= (uint8_t)~_01_RTC_COUNTER_MATCH; 	/* clear PF */
        RSR = tmp;
        r_rtc_callback_periodic();
    }
}



/***********************************************************************************************************************
* Function Name: r_rtc_callback_periodic
* Description  : This function is real-time clock interrupt service handler.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_rtc_callback_periodic(void)
{
    /* Start user code. Do not edit comment generated here */
	LMA_CB_RTC();
    /* End user code. Do not edit comment generated here */
}




/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
