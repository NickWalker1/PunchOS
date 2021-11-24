# PunchOS
An Operating System for teaching.

## Lab 1

## Using PunchOS in DCS
In order to run the OS, there is an included qemu_boot script and an associated config file.

Before beginging, update the config file to where PunchOS has been cloned.

Then to run the script in the DCS ensure the Makefile vars are correct for dcs.

Make the OS.

Then run by running the qemu_boot script.

Then connect to the VNC server in another terminal using the command specified in the boot script output.

## Dependencies 
* Make
* GCC
* NASM
* Grub
* Xorriso
* Qemu-system-i386