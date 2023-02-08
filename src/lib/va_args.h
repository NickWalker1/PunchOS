#pragma once

typedef unsigned char *va_list; 

#define va_start(arg_list, start) (arg_list = ((va_list) &start) + sizeof(start))
#define va_arg(args, type) (*(type*)((args+=sizeof(type)) - sizeof(type)))