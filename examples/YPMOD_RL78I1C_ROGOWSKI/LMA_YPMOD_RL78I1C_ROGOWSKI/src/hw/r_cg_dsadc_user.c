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
#include "Trap_integrator.h"
#include <stdbool.h>
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
extern LMA_Phase phase;
extern LMA_Neutral neutral;

#define PHASE_DELAY (7)
spl_t spls[PHASE_DELAY] = {0,};
uint8_t phase_delay_index = 0;

/** @brief phase shifts voltage signal
 * @details
 * - 50Hz signal is 20ms.
 * - 50Hz signal being 360degree of period, to get 90degree we divide by 4.
 * - 20ms divided by 4 = 5ms.
 * - to delay 5ms with a 3906Hz clock we can do 0.005/(1/3906) = 19.53 samples - so we do 20
 * samples.
 *
 * @param[in] new_voltage - new voltage to store in the buffer
 * @return voltage sample 90degree (20 samples) ago.
 */
static spl_t PhaseShift90(spl_t new_voltage)
{
	static spl_t voltage_buffer[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static uint8_t buffer_index = 0;

	uint8_t buffer_index_19 = buffer_index + 2;
	uint8_t buffer_index_20 = buffer_index + 1;

	if (buffer_index_19 > 21)
	{
		buffer_index_19 -= 21;
	}

	if (buffer_index_20 > 21)
	{
		buffer_index_20 -= 21;
	}

	/* Append new voltage*/
	voltage_buffer[buffer_index] = new_voltage;

	/* Interpolate 19.53 samples - just take the mid point*/
	int32_t interpolated_value = ((voltage_buffer[buffer_index_19] * 60) >> 7) + ((voltage_buffer[buffer_index_20] * 68) >> 7);

	buffer_index = buffer_index_20;

	/* Convert back to its 32b value*/
	return interpolated_value;
}

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
	*((uint16_t *)&neutral.inputs.i_sample) = *((uint16_t *)&DSADCR0);
	*(((uint8_t *)&neutral.inputs.i_sample) + 2) = *((uint8_t *)(&DSADCR0)+2);
	*(((int8_t *)&neutral.inputs.i_sample) + 3) = (*((int8_t *)(&DSADCR0)+2)) >> 7;

	/* Voltage*/
	*((uint16_t *)&phase.inputs.v_sample) = *((uint16_t *)&DSADCR3);
	*(((uint8_t *)&phase.inputs.v_sample) + 2) = *((uint8_t *)(&DSADCR3)+2);
	*(((int8_t *)&phase.inputs.v_sample) + 3) = (*((int8_t *)(&DSADCR3)+2)) >> 7;

	/* Voltage PHase Shifted 90 degrees*/
	phase.inputs.v90_sample = PhaseShift90(phase.inputs.v_sample);

	/* Rogowski Current*/
	*((uint16_t *)&phase.inputs.i_sample) = *((uint16_t *)&DSADCR1);
	*(((uint8_t *)&phase.inputs.i_sample) + 2) = *((uint8_t *)(&DSADCR1)+2);
	*(((int8_t *)&phase.inputs.i_sample) + 3) = (*((int8_t *)(&DSADCR1)+2)) >> 7;

	/* Integrate*/
	phase.inputs.i_sample = Trap_integrate(&rogowski_integrator, phase.inputs.i_sample);

	/* Phase Delay*/
	uint8_t next_phase_delay_index = phase_delay_index + 1;
	spls[phase_delay_index] = phase.inputs.i_sample;

	if(next_phase_delay_index > (PHASE_DELAY-1))
	{
		phase_delay_index = 0;
	}
	else
	{
		phase_delay_index = next_phase_delay_index;
	}

	phase.inputs.i_sample = spls[phase_delay_index];

    LMA_CB_ADC();

    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
