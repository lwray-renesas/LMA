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
* File Name    : r_cg_dsadc_user.c
* Version      : Applilet4 for RL78/I1C V1.01.07.02 [08 Nov 2021]
* Device(s)    : R5F11TLG
* Tool-Chain   : CCRL
* Description  : This file implements device driver for DSADC module.
* Creation Date: 06/03/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/


#include "r_cg_macrodriver.h"
#include "r_cg_dsadc.h"
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/

#pragma interrupt r_dsadc_interrupt(vect=INTDSAD)

/* Start user code for pragma. Do not edit comment generated here */
#include "LMA_Core.h"
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
extern LMA_Phase phase;
extern LMA_Phase neutral;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_dsadc_interrupt
* Description  : None
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_dsadc_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	/* CT Current - neutral*/
	*((uint16_t *)&neutral.ws.samples.current) = *((uint16_t *)&DSADCR0);
	*(((uint8_t *)&neutral.ws.samples.current) + 2) = *((uint8_t *)(&DSADCR0)+2);
	*(((int8_t *)&neutral.ws.samples.current) + 3) = (*((int8_t *)(&DSADCR0)+2)) >> 7;

	/* Voltage - for neutral computation*/
	*((uint16_t *)&neutral.ws.samples.voltage) = *((uint16_t *)&DSADCR3);
	*(((uint8_t *)&neutral.ws.samples.voltage) + 2) = *((uint8_t *)(&DSADCR3)+2);
	*(((int8_t *)&neutral.ws.samples.voltage) + 3) = (*((int8_t *)(&DSADCR3)+2)) >> 7;

	/* Rogowski Current*/
	*((uint16_t *)&phase.ws.samples.current) = *((uint16_t *)&DSADCR1);
	*(((uint8_t *)&phase.ws.samples.current) + 2) = *((uint8_t *)(&DSADCR1)+2);
	*(((int8_t *)&phase.ws.samples.current) + 3) = (*((int8_t *)(&DSADCR1)+2)) >> 7;

	/* Voltage - for phase computation*/
	phase.ws.samples.voltage = neutral.ws.samples.voltage;

    LMA_CB_ADC();
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
