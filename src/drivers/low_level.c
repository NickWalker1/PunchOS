unsigned char port_byte_in(unsigned short port){
    // A handy C wrapper  function  that  reads a byte  from  the  specified  port
    //   "=a" (result) means: put AL  register  in  variable  RESULT  when  finished
    //   "d" (port) means: load  EDX  with  port
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port) );
    return result;
}

void port_byte_out(unsigned short port, unsigned char data){
    // "a" (data) means: load  EAX  with  data
    // "d" (port) means: load  EDX  with  port
    __asm__("out %%al, %%dx" : :"a" (data), "d" (port));
}

unsigned short port_word_in(unsigned short port){
    unsigned short result;
    __asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out(unsigned short port, unsigned short data){
    __asm__("out %%ax, %%dx" : :"a" (data), "d" (port));
}

/* We will use this later on for reading from the I/O ports to get data
*  from devices such as the keyboard. We are using what is called
*  'inline assembly' in these routines to actually do the work */
unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We will use this to write to I/O ports to send bytes to devices. This
*  will be used in the next tutorial for changing the textmode cursor
*  position. Again, we use some inline assembly for the stuff that simply
*  cannot be done in C */
void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}
