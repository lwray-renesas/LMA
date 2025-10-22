/*
 * Storage.h
 *
 *  Created on: 22 Oct 2025
 *      Author: a5126135
 */

#ifndef STORAGE_STORAGE_H_
#define STORAGE_STORAGE_H_

#include "fdl.h"
#include "fdl_descriptor.h"
#include "eel.h"
#include "stdbool.h"
#include "stdint.h"

/** @brief Power on init of EEL*/
void Storage_init(void);

/** @brief Logical startup of EEL*/
void Storage_startup(void);

/** @brief Logical shutdown of EEL*/
void Storage_shutdown(void);

/**
 * @brief Reads data into output buffer.
 * @param id - id of EEPROM data
 * @param p_out- pointer to buffer to store output data.
 * @return true if successful, false otherwise.
 */
bool Storage_read(uint8_t id, __near uint8_t * p_out);

/**
 * @brief Writes data to eeprom.
 * @param id - id of EEPROM data
 * @param p_in - pointer to buffer containing data to write.
 */
void Storage_write(uint8_t id, __near uint8_t * p_in);

#endif /* STORAGE_STORAGE_H_ */
