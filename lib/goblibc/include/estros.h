/**
 * @file estros.h
 * @author Sofia "MetalPizzaCat"
 * @brief Very barebones file that contains basic structures required to to interact with kernel for EstrOS.
 * @version 0.1
 * @date 2025-06-27
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <stdint.h>
#include <stdarg.h>


enum Colors
{
    EC_Black,
    EC_Blue,
    EC_Green,
    EC_Cyan,
    EC_Red,
    EC_Magenta,
    EC_Brown,
    EC_LightGray,
    EC_DarkGray,
    EC_LightBlue,
    EC_LightGreen,
    EC_LightCyan,
    EC_LightRed,
    EC_Pink,
    EC_Yellow,
    EC_White
};

/// @brief Colors available to use in the text buffer
typedef enum Colors KernelTerminalColors;


uint16_t* get_text_buffer_address();

/// @brief Wrapper around syscall used for printing text into the standard output
/// @param str Pointer to the string to be displayed
/// @param len Length of the string
void sys_print(const char *str, uint32_t len);

/// @brief Wrapper around syscall used for reading text from standard input
/// @param buffer Pointer to the buffer where symbols will be written to 
/// @param len Length of the buffer
/// @return How many bytes were written
uint32_t sys_read(char *buffer, uint32_t len);

/// @brief Clear the screen
void sys_clear();

void sys_set_cursor(uint16_t x, uint16_t y);