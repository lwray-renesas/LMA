/*
 * Menu.h
 *
 *  Created on: 28 Jan 2025
 *      Author: a5126135
 */

#ifndef MENU_MENU_H_
#define MENU_MENU_H_

/** @brief maximum number of menu options*/
#define MAX_MENU_OPTIONS (10)

#include <stdint.h>

/** @brief enumerated type to implement submenus*/
typedef enum
{
	ACTION, /**< Action option type*/
	MENU_CHANGE /**< Submenu option type*/
}option_type_t;

/** Forward declaration of option*/
typedef struct Menu_option_str Menu_option;

/**
 * @brief Object comprising a menu's data
 */
typedef struct Menu_str
{
	char * p_name; /**< Name of the menu*/
	Menu_option * p_options[MAX_MENU_OPTIONS+1]; /**< array of supported options*/
	uint16_t count; /**< number of currently registered options*/
}Menu;


/** @brief union for options (action or submenu)*/
typedef union options_un
{
	void (*action)(char * p_args); /**< Action to perform upon detection of cmd*/
	Menu * p_menu; /**< Submenu to enter upon detection of cmd*/
}options_u;

/**
 * @brief Object for adding menu options to menu.
 */
struct Menu_option_str
{
	char * p_cmd; /**< command for menu system to look for*/
	char * p_help; /**< Help message*/
	option_type_t option_type; /**< What is this option, an action or a submenu?*/
	options_u option; /**< option for execution*/
};

/**
 * @brief initialises the menu system (opens drivers)
 * @param p_menu - pointer to top level menu to work from.
 */
void Menu_init(Menu * p_menu);
/**
 * @brief Adds a menu option to a menu.
 * @param p_menu - pointer to menu to work from.
 * @param p_menu_option - pointer to menu option to add to menu.
 */
void Menu_register_option(Menu * p_menu, Menu_option * p_menu_option);

/**
 * @brief Prints help
 */
void Menu_print_help(void);

/**
 * @brief Prints prompt (CMD>)
 */
void Menu_print_prompt(void);

/**
 * @brief Prints string to terminal
 * @param[in] p_data - pointer format string.
 * @param[in] ... - variadic args.
 */
void Menu_printf(char const * p_format, ...);

/**
 * @brief Handles menu subsytem.
 */
void Menu_handler(void);

#endif /* MENU_MENU_H_ */
