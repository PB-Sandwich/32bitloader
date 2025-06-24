
#pragma once

#include "error_handlers.h"
#include <stdint.h>

void clear_key_pressed(); // clears the key pressed flag
uint8_t key_pressed(); // returns 1 if a key is pressed
uint8_t scancode(); // gets the last scancode
void set_keyboard_function(void (*keyboard_function_)(uint8_t scancode));

__attribute__((interrupt)) void irq1_keyboard(struct interrupt_frame *frame);
__attribute__((interrupt)) void irq7_15_spurious(struct interrupt_frame *frame);

