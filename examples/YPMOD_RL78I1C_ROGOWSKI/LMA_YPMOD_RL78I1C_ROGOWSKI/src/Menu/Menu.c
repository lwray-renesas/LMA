/*
 * Menu.c
 *
 *  Created on: 28 Jan 2025
 *      Author: a5126135
 */

#include "Menu.h"
#include "r_cg_macrodriver.h"
#include "r_cg_sau.h"
#include "string.h"
#include "stdarg.h"
#include "stdio.h"
#include <stdbool.h>

/* LLVM Toolchain*/
#if defined(__clang__) && defined(__RL78__)

  #define MENU_CRITICAL_SECTION_PREPARE() char _cs_ie_bit = __builtin_rl78_pswie()
  #define MENU_CRITICAL_SECTION_ENTER() asm("di")
  #define MENU_CRITICAL_SECTION_EXIT()                                                                                          \
    if (_cs_ie_bit != 0)                                                                                                       \
    {                                                                                                                          \
      asm("ei");                                                                                                               \
    }

/* CCRL Toolchain*/
#elif defined(__CCRL__)

  #define MENU_CRITICAL_SECTION_PREPARE() unsigned char _psw_ie_masked = __get_psw() & 0x80
  #define MENU_CRITICAL_SECTION_ENTER() __DI()
  #define MENU_CRITICAL_SECTION_EXIT()                                                                                          \
    if (_psw_ie_masked != 0)                                                                                                   \
    {                                                                                                                          \
      __EI();                                                                                                                  \
    }

/* IAR Toolcahin*/
#elif define(__ICCRL78__)

  #include <intrinsics.h>
  #define MENU_CRITICAL_SECTION_PREPARE() __istate_t _cs_is = __get_interrupt_state()
  #define MENU_CRITICAL_SECTION_ENTER() __disable_interrupt()
  #define MENU_CRITICAL_SECTION_EXIT() __set_interrupt_state(_cs_is)

#else
  #error "Unsupported compiler!"
#endif

/** @brief size of uart ring buffer*/
#define RING_BUFFER_SIZE (128)

/**
 * @brief type for ring buffer
 */
typedef struct Ring_buffer_str
{
	char raw_buf[RING_BUFFER_SIZE]; /**< Raw buffer*/
	uint16_t head; /**< head of buffer*/
	uint16_t tail; /**< tail of buffer*/
}Ring_buffer;

/* Uart variables*/
static Ring_buffer uart_rx_buf = {{0,},0,0};

/* Menu variables*/
static Menu * p_top_menu = NULL;
static Menu * p_current_menu = NULL;

/**
 * @brief Enters character into ring buffer.
 * @param p_buf - pointer to the buffer to work on.
 * @param c - character going into ring buffer.
 */
static void Buffer_putc(Ring_buffer * p_buf, char c)
{
	MENU_CRITICAL_SECTION_PREPARE();

	MENU_CRITICAL_SECTION_ENTER();

	uint16_t next_head = (uint16_t) (p_buf->head + 1) % RING_BUFFER_SIZE;
	if (next_head != p_buf->tail)
	{
		p_buf->raw_buf[p_buf->head] = c;
		p_buf->head = next_head;
	}

	MENU_CRITICAL_SECTION_EXIT();
}

/**
 * @brief checks how many bytes are available.
 * @param p_buf - pointer to the buffer to work on.
 * @return number of bytes available
 */
static uint16_t Buffer_bytes_available(Ring_buffer * p_buf)
{
	MENU_CRITICAL_SECTION_PREPARE();
	uint16_t available = 0;

	if (NULL != p_buf)
	{
		MENU_CRITICAL_SECTION_ENTER();

		if (p_buf->head >= p_buf->tail)
		{
			available = p_buf->head - p_buf->tail;
		}
		else
		{
			available = (uint16_t) (RING_BUFFER_SIZE - p_buf->tail + p_buf->head);
		}

		MENU_CRITICAL_SECTION_EXIT();
	}

	return available;
}

/**
 * @brief Read the buffer contents
 * @param p_buf - pointer to the buffer to work on.
 * @param p_output - pointer to the output buffer.
 * @return number of bytes read
 */
static uint16_t Buffer_read(Ring_buffer * p_buf, char * p_output, const uint16_t count)
{
	MENU_CRITICAL_SECTION_PREPARE();

	if (NULL != p_buf && NULL != p_output)
	{
		uint16_t available = Buffer_bytes_available(p_buf);
		uint16_t num_bytes_read = 0;
		uint16_t num_bytes_to_read = 0;

		if(count > available)
		{
			num_bytes_to_read = available;
		}
		else
		{
			num_bytes_to_read = count;
		}

		MENU_CRITICAL_SECTION_ENTER();
		uint16_t temp_tail = p_buf->tail;
		MENU_CRITICAL_SECTION_EXIT();

		while (temp_tail != p_buf->head && num_bytes_read < num_bytes_to_read)
		{
			MENU_CRITICAL_SECTION_ENTER();
			p_output[num_bytes_read] = p_buf->raw_buf[temp_tail];
			MENU_CRITICAL_SECTION_EXIT();
			temp_tail = (uint16_t) (temp_tail + 1) % RING_BUFFER_SIZE;
			++num_bytes_read;
		}

		MENU_CRITICAL_SECTION_ENTER();
		p_buf->tail = temp_tail;
		MENU_CRITICAL_SECTION_EXIT();
	}

	return count;
}

/**
 * @brief Flush the buffer (set tail and head to 0)
 * @param p_buf - pointer to the buffer to work on.
 */
static void Buffer_flush(Ring_buffer * p_buf)
{
	MENU_CRITICAL_SECTION_PREPARE();
	MENU_CRITICAL_SECTION_ENTER();
	p_buf->tail = 0;
	p_buf->head = 0;
	MENU_CRITICAL_SECTION_EXIT();
}

/**
 * @brief prints character to uart terminal
 * @param c - character to print
 */
static void Uart_putc(char const c)
{
	R_UART1_SendByte(c);
}

void Menu_init(Menu * p_menu)
{
	if(NULL == p_menu)
	{
		while(1)
		{
			NOP();
		}
	}

	/* Start Uart*/
	R_UART1_Start();

	p_menu->count = 0;
	p_top_menu = p_menu;
	p_current_menu = p_top_menu;
}

void Menu_register_option(Menu * p_menu, Menu_option * p_menu_option)
{
	if(NULL == p_menu || NULL == p_menu_option)
	{
		while(1)
		{
			NOP();
		}
	}

	p_menu->p_options[p_menu->count++] = p_menu_option;
}

void Menu_print_help(void)
{
	Menu_printf("\r\n+----------------------------------------------------------------------------------+\r\n");
	Menu_printf("\t%s Options:\r\n", p_current_menu->p_name);
	Menu_printf("+----------------------------------------------------------------------------------+\r\n");

	for (uint16_t i = 0; i < p_current_menu->count; ++i)
	{
		Menu_printf("\t - %s: \t %s\r\n", p_current_menu->p_options[i]->p_cmd, p_current_menu->p_options[i]->p_help);
		Menu_printf("\r\n");
	}

	Menu_printf("+----------------------------------------------------------------------------------+\r\n");

	Menu_print_prompt();
}

void Menu_print_prompt(void)
{
	Menu_printf("\r\nCMD> ");
}

void Menu_printf(char const * p_format, ...)
{
	/* Buffer used for string formatting*/
	static char tx_buf[512] = {0,};
	int len = 0;
	int i = 0;
	va_list     arg = {NULL};
	va_start(arg, p_format);
	len = vsprintf((char *)tx_buf, (const char *)p_format, arg);

	if(len <= 0)
	{
		while(1)
		{
			NOP();
		}
	}

	while(i < len)
	{
		Uart_putc(tx_buf[i++]);
	}
}

void Menu_handler(void)
{
	static char line_buffer[RING_BUFFER_SIZE];
	static uint16_t index = 0;
	bool potential_command = false;

	uint16_t new_bytes = Buffer_bytes_available(&uart_rx_buf);

	if(new_bytes > 0)
	{
		uint16_t end_index = index + new_bytes;
		if(end_index < (RING_BUFFER_SIZE-1))
		{
			uint16_t last_character = end_index-1;

			/* Read the buffer*/
			(void)Buffer_read(&uart_rx_buf, &line_buffer[index], new_bytes);
			line_buffer[end_index] = 0; /* NULL Terminate*/

			/* Scan the buffer*/
			for(uint16_t i = index; i < end_index; ++i)
			{
				if('\b' == line_buffer[i])
				{
					if(i > 0)
					{
						/* If its the last character received - just delete the previous character*/
						if(i == last_character)
						{
							line_buffer[i-1] = 0;
							line_buffer[i] = 0;
							end_index -= 2;
						}
						else
						{
							/* Otherwise, shift the buffer back*/
							for(uint16_t j = i; j < last_character; ++j)
							{
								line_buffer[j-1] = line_buffer[j+1];
								line_buffer[j] = 0;
							}
						}

						/* Erase on terminal*/
						Menu_printf("\b \b");
					}
					else
					{
						/* First character we've received is backspace, so just remove it*/
						end_index -= 1;
					}
				}
				else if('\r' == line_buffer[i] || '\n' == line_buffer[i])
				{
					line_buffer[i] = 0;

					if('\r' == line_buffer[i+1] || '\n' == line_buffer[i+1])
					{
						line_buffer[i+1] = 0;
					}

					end_index = i;
					potential_command = true;
				}
				else
				{
					Uart_putc(line_buffer[i]);
				}
			}

			index = end_index;

			if(potential_command)
			{
				bool successful_command = false;

				for(uint16_t i = 0; i < p_current_menu->count; ++i)
				{
					if((index > 2) && 0 == strcmp(p_current_menu->p_options[i]->p_cmd, line_buffer))
					{
						if(ACTION == p_current_menu->p_options[i]->option_type)
						{
							p_current_menu->p_options[i]->option.action((line_buffer + strlen(p_current_menu->p_options[i]->p_cmd) + 1));
							Menu_print_prompt();
							successful_command = true;
							break;
						}
						else if(MENU_CHANGE == p_current_menu->p_options[i]->option_type)
						{
							p_current_menu = p_current_menu->p_options[i]->option.p_menu;
							Menu_print_help();
							successful_command = true;
							break;
						}
						else
						{
							/* Error, flush all buffers!*/
						}
					}
				}

				/* Handle unrecognised commands*/
				if(!successful_command)
				{
					Menu_printf("\r\n\r\nNot a recognised command!");
					Menu_print_help();
				}

				/* Flush buffers*/
				for(uint16_t i = 0; i < end_index; ++i)
				{
					line_buffer[i] = 0;
				}
				index = 0;
				end_index = 0;
				Buffer_flush(&uart_rx_buf);
			}

		}
		else
		{
			/* Error, flush all buffers!*/
			for(uint16_t i = 0; i < end_index; ++i)
			{
				line_buffer[i] = 0;
			}
			index = 0;
			end_index = 0;
			Buffer_flush(&uart_rx_buf);

			Menu_print_prompt();
		}
	}
}

void Menu_get_byte(uint8_t b)
{
	Buffer_putc(&uart_rx_buf, b);
}

