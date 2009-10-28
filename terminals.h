
#ifndef TERMINALS_H
#define TERMINALS_H


// terminals specific to the 8051 or PCB terminals not written to by the FPGA 
// are defined here.
//
// Any other terminal endpoint is passed to the FPGA and must be handled
// by the FPGA.

/**
 * FPGA_bitfile has no registers.  Write a bitfile to this terminal to program the FPGA.
 **/
#define TERM_FPGA_bitfile 2
#define TERM_FPGA_parbit 8


#define TERM_SPIFLASH 4
#define REG_SPIFLASH_STATUS 5
#define REG_SPIFLASH_SECTOR 6
#define REG_SPIFLASH_BE 8
#define REG_SPIFLASH_CHIPID 0x9f

#define TERM_SPIFLASH_DATA 3


/**
 * fx2 registers
 **/
#define TERM_FX2 6
#define REG_FX2_VERSION 4
#define REG_FX2_BOOTFPGA 5

#define TERM_FX2_IO 7
// sfrs
#define REG_FX2_IOA 0x80
#define REG_FX2_IOB 0x90
#define REG_FX2_IOC 0xA0
#define REG_FX2_IOD 0xB0
#define REG_FX2_IOE 0xB1
#define REG_FX2_OEA 0XB2
#define REG_FX2_OEB 0XB3
#define REG_FX2_OEC 0XB4
#define REG_FX2_OED 0XB5
#define REG_FX2_OEE 0XB6


/**
 * The EEPROM is contents are loaded when the 8051 receives power to configure how the chip 
 * responds to usb commands.  c0 contents load just a vendor and product id, while c2 contents
 * will load an entire firmware to the chip.  See TRM 3.4.
 *
 * Write firmware contents to register address 0.  Multipe bytes auto-increment.
 *
 **/
#define TERM_FX2PROM 0x51


#ifdef DEBUG_FIRMWARE
/**
 * Dummy terminal for testing read
 **/
#define TERM_DUMMY 0xcc

#endif

#endif