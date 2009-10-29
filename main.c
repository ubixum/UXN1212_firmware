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

#include <fx2macros.h>

#include "firmware.h"
#include "vendor_commands.h"
#include "terminals.h"
#include "handlers.h"
#include "fx2term.h"

volatile bit dorenum=FALSE;


extern code WORD str_serial;

xdata rdwr_data_t rdwr_data;

volatile bit is_fpga_trans; // quick test so that main loop doesn't have to switch the terminals

typedef struct {
    BYTE cmd;
    BYTE control;
    WORD term;
    DWORD reg;
    DWORD length;
    WORD reserved;
    WORD ack;
} fpga_control_cmd;

xdata fpga_control_cmd next_fpga_command;


volatile xdata WORD in_packet_max = 64; // max size for full speed usb (the default before hi-speed interrupt)


void reset_endpoints() {
     RESETFIFO(0x02); 
     OUTPKTEND=0x82; 
     SYNCDELAY(); 
     OUTPKTEND=0x82; 
     SYNCDELAY(); 
     OUTPKTEND=0x82;
     SYNCDELAY();
     OUTPKTEND=0x82;
     SYNCDELAY();
     RESETFIFO(0x06);
     printf ( "Reset Endpoints.\n" );
}

volatile xdata BYTE interface=0;
volatile xdata BYTE alt=0; // alt interface

// set *alt_ifc to the current alt interface for ifc
BOOL handle_get_interface(BYTE ifc, BYTE* alt_ifc) {
 *alt_ifc=alt;
 return TRUE;
}
// return TRUE if you set the interface requested
// NOTE this function should reconfigure and reset the endpoints
// according to the interface descriptors you provided.
BOOL handle_set_interface(BYTE ifc,BYTE alt_ifc) {  
    printf ( "Set Interface.\n" );
    RESETTOGGLE(0x02); 
    RESETTOGGLE(0X86); 
    reset_endpoints();
 interface=ifc;
 alt=alt_ifc;
 return TRUE;
}

// handle getting and setting the configuration
// 1 is the default.  If you support more than one config
// keep track of the config number and return the correct number
// config numbers are set int the dscr file.
// volatile BYTE config=1;
BYTE handle_get_configuration() { 
 return 1;
}
// return TRUE if you handle this request
// NOTE changing config requires the device to reset all the endpoints
BOOL handle_set_configuration(BYTE cfg) { 
 printf ( "Set Configuration.\n" );
 //config=cfg;
 return TRUE;
}


//******************* VENDOR COMMAND HANDLERS **************************

// handle funcs take type, length, value, index
// must return TRUE if we handled command ok
// or FALSE if an error.
typedef BOOL (*VCHandleFunc)(); //(BYTE type,WORD length,WORD value,WORD index);

typedef struct {
 BYTE cmd;
 VCHandleFunc handleFunc;
} vc_handler;


extern io_handler code io_handlers[];

io_handler_read_func cur_read_handler;
io_handler_write_func cur_write_handler;
io_handler_init_func cur_init_handler;
io_handler_status_func cur_status_handler;
io_handler_chksum_func cur_chksum_handler;

BOOL handleRDWR () { 
    BYTE cur=0; // counter

    printf ( "Received RDWR VC\n" );

    if (SETUP_TYPE != 0x40) return FALSE; 
    if (SETUP_LENGTH() != sizeof(rdwr_data_header)) return FALSE;

    // before every transaction, we tell the fpga to stop if it just happens to be in 
    // a state of reading or writing.

    IFCONFIG &= ~3; // out of slave fifo mode for everything but fpga 
    IFCONFIG |= 2;  // must use gpif mode so we can set mode pins
                    // without setting using pin flags
//    PINFLAGSCD = 0x0f; // puts flagc=1
    GPIFIDLECTL = 4;

    // clear structure back to 0
    memset(&rdwr_data, 0, sizeof(rdwr_data));

    EP0BCL=0; // arm for transfer
    while ( (EP0CS & bmEPBUSY) && !new_vc_cmd ); // host transfer the data
    if (new_vc_cmd) return FALSE;

    memcpy ( &rdwr_data, EP0BUF, sizeof(rdwr_data_header) );
    rdwr_data.in_progress=TRUE;
   
    // NOTE fix
    //i2c_term = LSB(rdwr_data.term_addr); // if not i2c transaction, ignored

    EP2FIFOCFG =0 ; // don't autoout write data for anything but fpga

    reset_endpoints(); // clear any old data

    is_fpga_trans = 0;
    
    // NOTE fix
    // perhaps a status function?
    //status_ptr = &default_status;

    while (TRUE) {
      if (io_handlers[cur].term_addr == 0) { is_fpga_trans = TRUE; break; }
      if (io_handlers[cur].term_addr == rdwr_data.h.term_addr) {
        printf ( "Found handlers for %d\n" , rdwr_data.h.term_addr );
        cur_read_handler = io_handlers[cur].read_handler; 
        cur_write_handler = io_handlers[cur].write_handler;
        cur_init_handler = io_handlers[cur].init_handler;
        cur_status_handler = io_handlers[cur].status_handler;
        cur_chksum_handler = io_handlers[cur].chksum_handler;
        break;
      } 
      ++cur;
    }

    if (is_fpga_trans) {
        printf ( "Initializing FPGA transaction for terminal %d\n", rdwr_data.h.term_addr );
        if (!PA1) return FALSE;
        IFCONFIG |= 3;
        // other outputs [7:4] are disabled anyway when switch to slave fifo mode
        // go back to slave fifo mode
        EP2FIFOCFG |= bmWORDWIDE; //|bmOEP; 
        PINFLAGSCD=0x0b; // flagc=0 during trans

        memset ( &next_fpga_command , 0, sizeof(next_fpga_command ) ); 
        next_fpga_command.control = 0xc3; // fpga command
        next_fpga_command.cmd = (rdwr_data.h.command & bmSETWRITE) ? 2 : 1;  // 1==READ, 2=WRITE
        next_fpga_command.term = rdwr_data.h.term_addr;
        next_fpga_command.reg = rdwr_data.h.reg_addr;
        next_fpga_command.length = rdwr_data.h.transfer_length;           
        next_fpga_command.ack = 0xaa55;

        FIFORESET = 0x80; SYNCDELAY();
        FIFORESET = 0x02; SYNCDELAY(); // put ep2 buffers in cpu domain
        memcpy ( EP2FIFOBUF, (void*)&next_fpga_command, sizeof(next_fpga_command) );

        SYNCDELAY();
        EP2BCH = 0;
        SYNCDELAY();
        EP2BCL = sizeof(next_fpga_command); SYNCDELAY();

        // skip some packets
        OUTPKTEND=0x82; SYNCDELAY();
        OUTPKTEND=0x82; SYNCDELAY();
        OUTPKTEND=0x82; SYNCDELAY();
        FIFORESET = 0; SYNCDELAY();

        EP2FIFOCFG |= bmAUTOOUT; // next packets come from pc 
    } else if (cur_init_handler) {
        cur_init_handler();
    }
     
    return TRUE;
}




BOOL rdwr_stat( ) { 


    if (SETUP_LENGTH() != sizeof(rdwr_data)) return FALSE;

    memcpy ( EP0BUF, &rdwr_data, sizeof(rdwr_data) );
    EP0BCH=0;
    SYNCDELAY();
    EP0BCL=sizeof(rdwr_data);
    return TRUE;

}

BOOL handle_serial () {
    BYTE c;
    BYTE buf[2];
    if (SETUP_LENGTH() != 8) return FALSE;

    switch ( SETUP_TYPE ) {

//        case 0x40:
//            EP0BCL=0; // allow transfer in.
//            while ((EP0CS & bmEPBUSY) && !new_vc_cmd ); // make sure the data came in ok.
//            if (new_vc_cmd) return FALSE;
//                
//            buf[1]=0;
//            for (c=0;c<8;++c) {
//                i2c_addr_buf[0] = MSB(PROM_SERIAL_OFFSET+c*2);
//                i2c_addr_buf[1] = LSB(PROM_SERIAL_OFFSET+c*2);
//                buf[0] = EP0BUF[c];
//                i2c_write ( TERM_FX2PROM, 2, i2c_addr_buf, 2, buf); 
//            }
//            return TRUE;
//       case 0xc0:
//            while ((EP0CS & bmEPBUSY) && !new_vc_cmd); 
//            if (new_vc_cmd) return FALSE;
//            for (c=0;c<8;++c) {
//                i2c_addr_buf[0] = MSB(PROM_SERIAL_OFFSET+c*2);
//                i2c_addr_buf[1] = LSB(PROM_SERIAL_OFFSET+c*2);
//                i2c_write ( TERM_FX2PROM, 2, i2c_addr_buf, 0, NULL );
//                i2c_read ( TERM_FX2PROM, 1, EP0BUF+c );
//            }
//            EP0BCH=0; SYNCDELAY();
//            EP0BCL=8;
//            return TRUE;
        default:
            return FALSE;
    }

}


BOOL handle_renum() {
    dorenum=TRUE;
    return TRUE;
}

// last element of vc_handlers will have a handler w/ 0 for cmd
vc_handler code vc_handlers[] = {
//    { VC_HI_REGVAL, handleGETSET },
    { VC_HI_RDWR, handleRDWR },
    { VC_RDWR_STAT, rdwr_stat },
    { VC_RENUM, handle_renum },
    { VC_SERIAL, handle_serial },
    { 0 }
 };

BOOL handle_vendorcommand(BYTE cmd) {
 xdata BYTE i=0;
 //printf ( "setupdat[5] %02x setupdat[4] %02x\n", SETUPDAT[5], SETUPDAT[4] );
 while (TRUE) {
   if (vc_handlers[i].cmd == cmd) {
       return vc_handlers[i].handleFunc();
   } else if (vc_handlers[i].cmd == 0) {
     break;
   } else {
     ++i;
   }
 } 
 return FALSE; // not handled by handlers

}


void main_init() {
 BYTE c;
 BYTE addr;

 CPUCS &= ~bmCLKOE; // don't drive clkout;
 REVCTL=3;


 printf ( "Someone Called Init\n" );

 IFCONFIG = 0xE2; // F2=48mhz // NOTE bit 4 is clock polarity. 1=clock inverted 
 GPIFIDLECS = 0;
 GPIFCTLCFG = 0; // for when gpif is used to drive ctl outputs (flags a-c)
 GPIFIDLECTL = 0;
 PORTACFG = 0; // make sure PA7 does not output a FLAGD state (we always want it to be in input during slave fifo)

 // interrupts
 ENABLE_SUTOK(); 

 // initialize the device serial number
 // NOTE fix
// i2c_addr_buf[0] = MSB(PROM_SERIAL_OFFSET);
// i2c_addr_buf[1] = LSB(PROM_SERIAL_OFFSET);
// i2c_write( TERM_FX2PROM, 2, i2c_addr_buf, 0, NULL );
// i2c_read ( TERM_FX2PROM, 16, (xdata BYTE*)&str_serial + 2 ); 

 // other endpoints not valid
 EP1OUTCFG &= ~bmVALID;
 EP4CFG &= ~bmVALID;
 EP8CFG &= ~bmVALID;
  
 EP2CFG = 0xA0;   // 10100000 VALID, OUT, BULK, 512 BYTES, QUAD 
 EP2FIFOCFG = 0; SYNCDELAY();

 EP6CFG = 0xE0; SYNCDELAY();  // 11100000 VALID, IN , BULK, 512 BYTES, QUAD 

 EP6FIFOCFG = 0; SYNCDELAY(); // zero to one transition just in case
 EP6FIFOCFG = bmWORDWIDE|bmAUTOIN; SYNCDELAY();
 EP6AUTOINLENH=MSB(in_packet_max); SYNCDELAY();
 EP6AUTOINLENL=LSB(in_packet_max); SYNCDELAY();

 // DECIS=0 (fire when level is >= setting (active low))
 // PKTSTAT=0 (pkts + current bytes)
 // PKTS = 3
 // current bytes = 508 // 4 byte buffer
 // 0x19, 0xfc = 508
 // 0x18, 0x00 = 3 buffers no bytes
 EP6FIFOPFH = 0x18; SYNCDELAY(); 
 EP6FIFOPFL = 0x0; SYNCDELAY(); // 508=0x1fc 
 //EP6FIFOPFH = 0x19; SYNCDELAY();
 //EP6FIFOPFL = 0xfc; SYNCDELAY();

 FIFOPINPOLAR = 0; // everything active low.
 PINFLAGSAB = 0x68; SYNCDELAY(); // flagb = ep6 PF flaga = ep2 ef
 PINFLAGSCD = 0x0B; SYNCDELAY(); // flagc = ep8 EF (this is to make sure flagc doesn't assert during normal operation since we aren't using ep8)
 
 printf ( "Initialization Done.\n" );

}

void send_ack_packet() {
    WORD status=0;
    WORD checksum=0;
    if (cur_status_handler) status=cur_status_handler();
    if (cur_chksum_handler) checksum=cur_chksum_handler();
    EP6FIFOBUF[0] = 0x0f;
    EP6FIFOBUF[1] = 0xa5;
    EP6FIFOBUF[2] = LSB(checksum);
    EP6FIFOBUF[3] = MSB(checksum);
    EP6FIFOBUF[4] = LSB(status); // status low
    EP6FIFOBUF[5] = MSB(status); // status high
    EP6FIFOBUF[6] = 0; // reserved low
    EP6FIFOBUF[7] = 0; // reserved high
    EP6BCH=0; SYNCDELAY();
    EP6BCL=8; // send the ack
}


void main_loop() {
 BYTE c;
 WORD checksum=0;
 WORD status=0;

 // data to read from a terminal?
 if ( rdwr_data.in_progress && !(rdwr_data.h.command & bmSETWRITE) && !rdwr_data.aborted &&!is_fpga_trans) {
    xdata DWORD readlen = rdwr_data.h.transfer_length - rdwr_data.bytes_read;
    xdata WORD this_read = readlen > in_packet_max ? in_packet_max : readlen;
    printf ("do a read.\n" );
    if (this_read>0) { 
        cur_read_handler(this_read);
        rdwr_data.bytes_read += rdwr_data.bytes_avail;
        if (rdwr_data.bytes_avail) {
         EP6BCH=MSB(rdwr_data.bytes_avail);
         SYNCDELAY();
         EP6BCL=LSB(rdwr_data.bytes_avail);
        }
        // NOTE should I send a status packet if the trans is aborted here? 
    }
    
    if ( rdwr_data.bytes_read >= rdwr_data.h.transfer_length && !(EP2468STAT & bmEP6FULL)) {
        printf ( "Finished Read.\n" );
        rdwr_data.in_progress = FALSE;
        PA2=1; // chip select should be high (set low for programming spi flash)
        OEA=8;
        send_ack_packet();
    }

 } 
 
 // received data to write to a terminal
 
 if (!(EP2468STAT & bmEP2EMPTY) && !is_fpga_trans) {

     if ( !(rdwr_data.h.command & bmSETWRITE) || rdwr_data.aborted || !rdwr_data.in_progress ) {
        OUTPKTEND = 0x82;
     } else {
        rdwr_data.bytes_avail = MAKEWORD ( EP2BCH, EP2BCL );
        //printf ( "Writing %d Bytes..\n", rdwr_data.bytes_avail );
        if (cur_write_handler ()) { 
         rdwr_data.bytes_written += rdwr_data.bytes_avail;
         OUTPKTEND = 0x82; 
         //EP2BCL=0x80;
        }

        if ( rdwr_data.bytes_written  >= rdwr_data.h.transfer_length || rdwr_data.aborted ) {
            rdwr_data.in_progress = FALSE;
            send_ack_packet();
            PA2=1;
            OEA=8; 
        }
     }

 }

 if (dorenum) {
    printf ( "Received USB renum command.\n" );
    dorenum=FALSE;
    RENUMERATE_UNCOND();
 }


}


/********* Optional Interrupts ***************/
void sof_isr() interrupt SOF_ISR {}
void sutok_isr() interrupt SUTOK_ISR {
 new_vc_cmd=1;
 cancel_i2c_trans=TRUE;
 printf ( "sutok..\n" );
 CLEAR_SUTOK();
}
void ep0ack_isr() interrupt EP0ACK_ISR {}
void ep0in_isr() interrupt EP0IN_ISR {}
void ep0out_isr() interrupt EP0OUT_ISR {}
void ep1in_isr() interrupt EP1IN_ISR {}
void ep1out_isr() interrupt EP1OUT_ISR {}
void ep2_isr() interrupt EP2_ISR {}
void ep4_isr() interrupt EP4_ISR {}
void ep6_isr() interrupt EP6_ISR {}
void ep8_isr() interrupt EP8_ISR {}
void ibn_isr() interrupt IBN_ISR {}
void ep0ping_isr() interrupt EP0PING_ISR {}
void ep1ping_isr() interrupt EP1PING_ISR {}
void ep2ping_isr() interrupt EP2PING_ISR {}
void ep4ping_isr() interrupt EP4PING_ISR {}
void ep6ping_isr() interrupt EP6PING_ISR {}
void ep8ping_isr() interrupt EP8PING_ISR {}
void errlimit_isr() interrupt ERRLIMIT_ISR {}
void ep2isoerr_isr() interrupt EP2ISOERR_ISR {}
void ep4isoerr_isr() interrupt EP4ISOERR_ISR {}
void ep6isoerr_isr() interrupt EP6ISOERR_ISR {}
void ep8isoerr_isr() interrupt EP8ISOERR_ISR {}
void spare_isr() interrupt RESERVED_ISR {}
void ep2pf_isr() interrupt EP2PF_ISR{}
void ep4pf_isr() interrupt EP4PF_ISR{}
void ep6pf_isr() interrupt EP6PF_ISR{}
void ep8pf_isr() interrupt EP8PF_ISR{}
void ep2ef_isr() interrupt EP2EF_ISR{}
void ep4ef_isr() interrupt EP4EF_ISR{}
void ep6ef_isr() interrupt EP6EF_ISR{}
void ep8ef_isr() interrupt EP8EF_ISR{}
void ep2ff_isr() interrupt EP2FF_ISR{}
void ep4ff_isr() interrupt EP4FF_ISR{}
void ep6ff_isr() interrupt EP6FF_ISR{}
void ep8ff_isr() interrupt EP8FF_ISR{}
void gpifdone_isr() interrupt GPIFDONE_ISR{}
void gpifwf_isr() interrupt GPIFWF_ISR{}
