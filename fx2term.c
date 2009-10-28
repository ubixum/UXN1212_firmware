#include <fx2regs.h>
#include "firmware.h"
#include <delay.h>
#include "terminals.h"
#include "handlers.h"

extern code WORD dev_dscr; // the device descriptor.

void boot_fpga() {

 // program from spi 
 OEA = 0x08;
 IOA = 0x04; 
 GPIFIDLECTL = 1;
// PINFLAGSAB=0xbf; SYNCDELAY();
// PINFLAGSCD=0xbb; SYNCDELAY();
 delay(1);
 PA3=1; // prog_b high

 delay(250);
 if (!PA1) {
    // fpga didn't program 
    // hold in reset
    PA3=0;
 }
 
 GPIFIDLECTL = 4;

}


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
                switch ( rdwr_data.h.reg_addr ) {
                     case REG_FX2_BOOTFPGA:
                         if (1==val) boot_fpga();
                         break;
                     default:
                         // WARNING setting 0xe601 (IFCONFIG) has caused me issues here.
                         // some registers don't set! 
                         // (either that or they are getting reset before I can read them again.
                         if (rdwr_data.h.reg_addr >= 0xe600 && rdwr_data.h.reg_addr <= 0xfdff ) {
                             WORD reg_addr = rdwr_data.h.reg_addr; // cast 31 bit to 16
                             xdata BYTE *a = *((xdata BYTE**)&rdwr_data.h.reg_addr);
                             printf ( "Set %04x a: %04x *a: %02x\n " , (WORD)rdwr_data.h.reg_addr, (WORD)a, *a );
                             *a = val;
                         }
                         break;
                 }
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

