/**
 * Copyright (C) 2009 Ubixum, Inc. 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/

#ifndef TERMINALS_H
#define TERMINALS_H


// terminals specific to the 8051 or PCB terminals not written to by the FPGA 
// are defined here.
//
// Any other terminal endpoint is passed to the FPGA and must be handled
// by the FPGA.


/**
 * fx2 registers
 **/
#define TERM_FX2 0x100 
#define REG_FX2_VERSION 0

#define TERM_FX2_IO 0x101 
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
