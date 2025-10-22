/*
 * Storage.c
 *
 *  Created on: 22 Oct 2025
 *      Author: a5126135
 */

#include "Storage.h"
#include "hal_data.h"

/* Used for eeprom access*/
static bool veeprom_callback_called = false;

void Storage_startup(void)
{
	/* Open the VEEPROM*/
	fsp_err_t err = RM_VEE_FLASH_Open(&g_vee0_ctrl, &g_vee0_cfg);
	if (FSP_SUCCESS != err)
	{
		__BKPT(0);
	}
}

void Storage_shutdown(void)
{
	fsp_err_t err = RM_VEE_FLASH_Close(&g_vee0_ctrl);
	if (FSP_SUCCESS != err)
	{
		__BKPT(0);
	}
}

bool Storage_read(uint8_t id, uint8_t * p_out)
{
	uint32_t num_bytes = 0;
	fsp_err_t err = RM_VEE_FLASH_RecordPtrGet(&g_vee0_ctrl, id, &p_out, &num_bytes);

	return (FSP_SUCCESS == err);
}

void Storage_write(uint8_t id, uint8_t * p_in, uint32_t size)
{
	fsp_err_t err = FSP_SUCCESS;
	veeprom_callback_called = false;
	err = RM_VEE_FLASH_RecordWrite(&g_vee0_ctrl, id, p_in, size);
	if (FSP_SUCCESS != err)
	{
		__BKPT(0);
	}
	/* Wait for the Virtual EEPROM callback to indicate it finished writing data. */
	while (false == veeprom_callback_called)
	{
		__NOP();
	}
}


void VEE_Callback(rm_vee_callback_args_t *p_args)
{
	(void)p_args;
	veeprom_callback_called = true;
}

