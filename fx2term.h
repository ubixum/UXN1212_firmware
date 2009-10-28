
#ifndef FX2TERM_H
#define FX2TERM_H

/**
 * Trigger function to cause FPGA to boot from SPI prom
 **/
void boot_fpga();

/**
 * io_handler for getting fx2 registers
 **/
void get_fx2();

/**
 * io_handler for setting fx2 registers
 **/
BOOL set_fx2();


#endif
