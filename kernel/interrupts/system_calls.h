
#pragma once

#include "error_handlers.h"
#include "irq_handlers.h"

__attribute__((naked)) void syscall(struct interrupt_frame *frame);

