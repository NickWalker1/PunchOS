#pragma once

#include "cpu_state.h"
#include "idt.h"

#include "../lib/debug.h"



void default_exception_handler(exception_state* state);
