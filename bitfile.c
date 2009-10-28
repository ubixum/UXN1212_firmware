#include "bitfile.h"

void init_bitfile() {

    printf ("fpga_program_init()\n" );
    GPIFIDLECTL |= 0x07; // set mode pins to 1:1:1
    IOA = 0x4; // prog_b low, cs_b high

    OEA=0xac; //output on 3,5,7
    delay(1);
    PA3 = 1;  // prog_b back high
    delay(1);
    if ( !PA0 ) { // wait until init_b high
        rdwr_data.aborted=TRUE;
    } 
    GPIFIDLECTL=4; // 

}

BOOL write_bitfile() {
    

    //WORD t=rdwr_data.bytes_avail;
    BYTE cur, step;
    WORD stop = (WORD)EP2FIFOBUF+rdwr_data.bytes_avail;
    BYTE stoph = MSB(stop);
    BYTE stopl = LSB(stop);

    printf ("fpga_program()\n" );

    AUTOPTRH1 = MSB((WORD)EP2FIFOBUF);
    AUTOPTRL1 = LSB((WORD)EP2FIFOBUF);


    // NOTE recheck init_b at appropriate point?

    do {
       cur = XAUTODAT1;

        step = 8;
        do { //( --step ) {
           // set data and clock
           PA5=0; // clkfall
           PA7 = cur & 0x80; // msb 0 or 1
           cur = cur << 1;
           PA5=1; // clkrise
        } while (--step );
       
     } while ( stoph != AUTOPTRH1 || stopl != AUTOPTRL1 );

    return TRUE;

}

void init_parbitfile() {
   
   //IFCONFIG &= 0x02; // not in gpif mode
   OEB=0xff; // output on port b
   PA6=1; // unselect SPI
   PA4=1; // CSI_B=1
   PA2=1; // RDWR_B=1
   OEA=0x76;
   PA3=0; // PROGB
   delay(1);
   if (!PA0) {
    rdwr_data.aborted=TRUE;
   }
   PA4=0; // csi_b
   PA2=1; // rdwr_b
   PA5=1; // clkrise
   PA2=0; // rdwr_b
}

BOOL write_parbitfile() {
    //WORD t=rdwr_data.bytes_avail;

    WORD stop = (WORD)EP2FIFOBUF+rdwr_data.bytes_avail;
    BYTE stoph = MSB(stop);
    BYTE stopl = LSB(stop);

    printf ("fpga_program() (parallel)\n" );

    AUTOPTRH1 = MSB((WORD)EP2FIFOBUF);
    AUTOPTRL1 = LSB((WORD)EP2FIFOBUF);


    // NOTE recheck init_b at appropriate point?
    //


    do {
        PA5=0; //clkfall;
        IOB = XAUTODAT1;
        PA5=1; // clkrise;
       
     } while ( stoph != AUTOPTRH1 || stopl != AUTOPTRL1 );

    //EP2BCL=0x80;
    return TRUE;

}
