#pragma once
unsigned char port_byte_in(unsigned short port);

void port_byte_out(unsigned short port, unsigned char data);

unsigned short port_word_in(unsigned short port);

void port_word_out(unsigned short port, unsigned short data);

unsigned char inportb (unsigned short _port);

void outportb (unsigned short _port, unsigned char _data);