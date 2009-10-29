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
#include <fx2regs.h>
#include "firmware.h"
#include <delay.h>
#include "terminals.h"
#include "handlers.h"

extern code WORD dev_dscr; // the device descriptor.

void get_fx2(WORD len) {
    printf ( "get_fx2, len: %d\n" , len );
    if ( len == 2 ) {
        EP6FIFOBUF[1] = 0; // since most of these are just one byte...
        switch ( rdwr_data.h.term_addr )
        {
            case TERM_FX2:
    
                switch ( rdwr_data.h.reg_addr ) {
                    case REG_FX2_VERSION:
                        EP6FIFOBUF[0] = *((BYTE*)(&dev_dscr) + 12);
                        EP6FIFOBUF[1] = *((BYTE*)(&dev_dscr) + 13);
                        break;
                  default:
                        if (rdwr_data.h.reg_addr >= 0xe600 && rdwr_data.h.reg_addr <= 0xfdff ) {
                            xdata BYTE* a = *((xdata BYTE**)&rdwr_data.h.reg_addr);
                            printf ( "Get %04x a: %04x *a: %02x\n " , (WORD)rdwr_data.h.reg_addr, (WORD)a, *a );
                            EP6FIFOBUF[0] = *a;
                            break;
                        }
                   
                }
                break;
            case TERM_FX2_IO:
                switch ( rdwr_data.h.reg_addr ) {
                    case REG_FX2_IOA:
                        EP6FIFOBUF[0] = IOA;
                        break;
                    case REG_FX2_OEA:
                        EP6FIFOBUF[0] = OEA;
                        break;
                    case REG_FX2_IOB:
                        EP6FIFOBUF[0] = IOB;
                        break;
                    case REG_FX2_OEB:
                        EP6FIFOBUF[0] = OEB;
                        break;                   
                    case REG_FX2_IOC:
                        EP6FIFOBUF[0] = IOC;
                        break;
                    case REG_FX2_OEC:
                        EP6FIFOBUF[0] = OEC;
                        break;
                    case REG_FX2_IOD:
                        EP6FIFOBUF[0] = IOD;
                        break;
                    case REG_FX2_OED:
                        EP6FIFOBUF[0] = OED;
                        break;
                    case REG_FX2_IOE:
                        EP6FIFOBUF[0] = IOE;
                        break;
                    case REG_FX2_OEE:
                        EP6FIFOBUF[0] = OEE;
                        break;       
                } 
                break;
         }
        rdwr_data.bytes_avail=2;
    } else {
        rdwr_data.aborted=TRUE;
    }
}


BOOL set_fx2() {
    BYTE val=EP2FIFOBUF[0];
    printf ( "set_fx2, bytes_avail: %d val: %02x\n", rdwr_data.bytes_avail, val );
    if (rdwr_data.bytes_avail == 2) {
        switch (rdwr_data.h.term_addr) {
            case TERM_FX2:
                if (rdwr_data.h.reg_addr >= 0xe600 && rdwr_data.h.reg_addr <= 0xfdff ) {
                    WORD reg_addr = rdwr_data.h.reg_addr; // cast 31 bit to 16
                    xdata BYTE *a = *((xdata BYTE**)&rdwr_data.h.reg_addr);
                    printf ( "Set %04x a: %04x *a: %02x\n " , (WORD)rdwr_data.h.reg_addr, (WORD)a, *a );
                    *a = val;
                }
                break;
            case TERM_FX2_IO:
                switch ( rdwr_data.h.reg_addr ) {
                     case REG_FX2_IOA:
                         IOA = val;
                         break;
                     case REG_FX2_OEA:
                         OEA = val;
                         break;
                     case REG_FX2_IOB:
                         IOB = val;
                         break;
                     case REG_FX2_OEB:
                         OEB = val;
                         break;
                     case REG_FX2_IOC:
                         IOC = val;
                         break;
                     case REG_FX2_OEC:
                         OEC = val;
                         break;
                     case REG_FX2_IOD:
                         IOD = val;
                         break;
                     case REG_FX2_OED:
                         OED = val;
                         break;
                     case REG_FX2_IOE:
                         IOE = val;
                         break;
                     case REG_FX2_OEE:
                         OEE = val;
                         break;
                }
        }
        ++rdwr_data.h.reg_addr;
    } else {
        rdwr_data.aborted=TRUE;
    }
    return TRUE;
}

