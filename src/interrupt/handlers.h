#pragma once

#include "cpu_state.h"
#include "idt.h"

#include "../lib/panic.h"



void default_exception_handler(exception_state* state);
