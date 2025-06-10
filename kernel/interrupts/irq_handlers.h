
#pragma once

#include "error_handlers.h"

__attribute__((interrupt)) void irq1_keyboard(struct interrupt_frame *frame);
__attribute__((interrupt)) void irq7_15_spurious(struct interrupt_frame *frame);

