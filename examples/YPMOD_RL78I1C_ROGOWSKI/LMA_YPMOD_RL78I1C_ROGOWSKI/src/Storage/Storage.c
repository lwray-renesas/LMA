/*
 * Storage.c
 *
 *  Created on: 22 Oct 2025
 *      Author: a5126135
 */

#include "Storage.h"
#include "r_cg_macrodriver.h"
#include "stddef.h"

/** @brief Executes request (blocking)
 *
 * @param p_req - pointer to request to execute.
 */
static void EEL_DoCommand(eel_request_t *p_req)
{
  do
  {
    EEL_Handler();
    EEL_Execute(p_req);
  } while (EEL_ERR_REJECTED == p_req->status_enu);

  while (EEL_BUSY == p_req->status_enu)
  {
    EEL_Handler();
  }
}

/** @brief Unrecoverable Error Handler*/
static void Storage_ErrorHandler(void)
{
  while (1)
  {
    NOP();
  }
}

void Storage_init(void)
{
  fdl_status_t fdl_stat;
  eel_status_t eel_stat;

  /* Initialise the EEL*/
  fdl_stat = FDL_Init(&fdl_descriptor_str);
  if (FDL_OK != fdl_stat)
  {
    Storage_ErrorHandler();
  }

  FDL_Open();

  eel_stat = EEL_Init();
  if (EEL_OK != eel_stat)
  {
    Storage_ErrorHandler();
  }
}

void Storage_startup(void)
{
  static eel_request_t eel_req;

  EEL_Open();

  eel_req.command_enu = EEL_CMD_STARTUP;
  eel_req.address_pu08 = NULL;
  eel_req.identifier_u08 = 0U;
  eel_req.status_enu = EEL_ERR_PARAMETER;
  EEL_Execute(&eel_req);

  while (EEL_BUSY == eel_req.status_enu)
  {
    EEL_Handler();
  }

  if (EEL_ERR_POOL_INCONSISTENT == eel_req.status_enu)
  {
    eel_req.command_enu = EEL_CMD_FORMAT;
    EEL_Execute(&eel_req);

    while (EEL_BUSY == eel_req.status_enu)
    {
      EEL_Handler();
    }

    if (EEL_OK != eel_req.status_enu)
    {
      /* Unrecoverable Error*/
      Storage_ErrorHandler();
    }

    /* Try startup again!*/
    eel_req.command_enu = EEL_CMD_STARTUP;
    EEL_Execute(&eel_req);
    while (EEL_BUSY == eel_req.status_enu)
    {
      EEL_Handler();
    }

    if (EEL_OK != eel_req.status_enu)
    {
      /* Unrecoverable Error*/
      Storage_ErrorHandler();
    }
  }
  else if (EEL_ERR_VERIFY == eel_req.status_enu)
  {
    eel_req.command_enu = EEL_CMD_REFRESH;
    EEL_Execute(&eel_req);

    while (EEL_BUSY == eel_req.status_enu)
    {
      EEL_Handler();
    }

    if (EEL_OK != eel_req.status_enu)
    {
      /* Unrecoverable Error*/
      Storage_ErrorHandler();
    }
  }
  else if (EEL_OK == eel_req.status_enu)
  {
    /* Do Nothing - move on*/
  }
  else
  {
    /* Unrecoverable Error*/
    Storage_ErrorHandler();
  }
}

void Storage_shutdown(void)
{
  static eel_request_t eel_req;

  eel_req.command_enu = EEL_CMD_VERIFY;
  eel_req.address_pu08 = NULL;
  eel_req.identifier_u08 = 0U;
  eel_req.status_enu = EEL_ERR_PARAMETER;

  EEL_DoCommand(&eel_req);

  if (EEL_ERR_VERIFY == eel_req.status_enu)
  {
    eel_req.command_enu = EEL_CMD_REFRESH;

    EEL_DoCommand(&eel_req);

    if (EEL_OK != eel_req.status_enu)
    {
      Storage_ErrorHandler();
    }
  }
  else if (EEL_OK != eel_req.status_enu)
  {
    Storage_ErrorHandler();
  }
  else
  {
    /* Do Nothing - all ok*/
  }

  eel_req.command_enu = EEL_CMD_SHUTDOWN;

  EEL_DoCommand(&eel_req);

  if (EEL_OK != eel_req.status_enu)
  {
    Storage_ErrorHandler();
  }

  EEL_Close();
}

bool Storage_read(uint8_t id, __near uint8_t *p_out)
{
  static eel_request_t eel_req;

  eel_req.command_enu = EEL_CMD_READ;
  eel_req.address_pu08 = p_out;
  eel_req.identifier_u08 = id;
  eel_req.status_enu = EEL_ERR_PARAMETER;

  EEL_DoCommand(&eel_req);

  return (EEL_OK == eel_req.status_enu);
}

void Storage_write(uint8_t id, __near uint8_t *p_in)
{
  static eel_request_t eel_req;
  static eel_request_t refr_req;

  refr_req.command_enu = EEL_CMD_REFRESH;
  refr_req.address_pu08 = NULL;
  refr_req.identifier_u08 = 0U;
  refr_req.status_enu = EEL_ERR_PARAMETER;

  eel_req.command_enu = EEL_CMD_WRITE;
  eel_req.address_pu08 = p_in;
  eel_req.identifier_u08 = id;
  eel_req.status_enu = EEL_ERR_PARAMETER;

  do
  {
    EEL_DoCommand(&eel_req);

    if (EEL_ERR_POOL_FULL == eel_req.status_enu)
    {
      EEL_DoCommand(&refr_req);

      if (EEL_OK != refr_req.status_enu)
      {
        Storage_ErrorHandler();
      }
    }
    else if (EEL_OK != eel_req.status_enu)
    {
      Storage_ErrorHandler();
    }
    else
    {
      /* Do Nothing - all OK*/
    }

  } while (EEL_ERR_POOL_FULL == eel_req.status_enu);
}
