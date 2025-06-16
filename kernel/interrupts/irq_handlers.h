
#pragma once

#include "error_handlers.h"
#include <stdint.h>

static uint8_t key_pressed = 0;
static uint8_t scancode = 0;
static void (*keyboard_function)(uint8_t scancode) = 0;

__attribute__((interrupt)) void irq1_keyboard(struct interrupt_frame *frame);
__attribute__((interrupt)) void irq7_15_spurious(struct interrupt_frame *frame);

