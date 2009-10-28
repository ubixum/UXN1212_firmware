#include <fx2regs.h>
#include "firmware.h"
#include "spi.h"
#include "terminals.h"

// spi stuff
//
#define SPI_RDSR 0x05
#define SPI_WREN 0x06
#define SPI_BE 0xc7
#define SPI_RDID 0x9f
#define SPI_READ 0x03
#define SPI_FASTREAD 0x0b
#define SPI_PP 0x02
xdata DWORD spi_addr=0;

void spi_shift ( BYTE b ) {
    BYTE step=8;
    //printf ( "Shift OUT %02x\n\t", b );
    do {
        PA5=0; // CLKFALL
        PA6 = b & 0x80;
     //   printf ( "%d " , PA6 );
        b <<= 1; 
        PA5=1; // CLKRISE
    } while ( --step );
    //printf("\n");
}

BYTE spi_inshift ()  {
    BYTE step=8;
    BYTE res=0;
    do {
        PA5=0; // CLKFALL
        res <<= 1;
        res |= (BYTE)PA7;
        PA5=1; // CLKRISE
    } while(--step);
    //printf ( "Shifted IN %02x\n", res );
    return res;
}

void spi_cmd (BYTE c , BOOL close){
    PA2=0;
    spi_shift(c);
    if(close) PA2=1;
}

BYTE spi_status() {
    BYTE ret;
    
    printf ("spi_status()\n" );

    spi_cmd(SPI_RDSR,FALSE);
    ret=spi_inshift();
    PA2=1;
    return ret; 
}

BYTE spi_chipid() {
    BYTE ret;

    printf ( "spi_chipid()\n" );
    spi_cmd(SPI_RDID,FALSE);
    ret=spi_inshift();
    PA2=1;
    return ret;
}


void get_spi(WORD len) {
    WORD res=0;
    printf ( "get_spi()\n" );
    if (len == sizeof(res)) {
        switch ( rdwr_data.h.reg_addr ) {
           case REG_SPIFLASH_STATUS:
                res=spi_status();
                break;
           case REG_SPIFLASH_CHIPID:
                res=spi_chipid();
                break;
           case REG_SPIFLASH_SECTOR:
                res=LSB(MSW(spi_addr));
                break;
        }
        EP6FIFOBUF[0]=LSB(res);
        EP6FIFOBUF[1]=MSB(res);
        rdwr_data.bytes_avail = 2;
    } else {
        rdwr_data.aborted=TRUE;
    }
}


BOOL set_spi() {
    WORD val;
    if (rdwr_data.bytes_avail==sizeof(val)) {
        memcpy ( &val, EP2FIFOBUF, sizeof(val) );
        switch ( rdwr_data.h.reg_addr ) {
            case REG_SPIFLASH_SECTOR:
                spi_addr = MAKEDWORD ( val, LSW(spi_addr ) );
                break;
            case REG_SPIFLASH_BE:
                if (1==val) { 
                    spi_cmd(SPI_WREN, TRUE);
                    spi_cmd(SPI_BE,TRUE);
                }
                break;
            default:
                break;
        }
    } else {
        rdwr_data.aborted=TRUE; // weird val
    }
    return TRUE;
}

// autoptr length helpers
xdata WORD aptr_stop;
// figure out start
#define SET_APTR_STOP(len, buf, stoph, stopl) \
    aptr_stop=(WORD)buf + len;\
    stoph=MSB(aptr_stop);\
    stopl=LSB(aptr_stop);

void spi_init() {
    // only do this once
    PA3=0; // reset fpga
    PA2=1; // start spi cs high
    OEA=0x6c;
    // NOTE fix while.
    if (rdwr_data.h.command == COMMAND_READ) {
        while ( (spi_status() & 1) && !new_vc_cmd );

        PA2=0;
        spi_shift ( SPI_FASTREAD ); 
        spi_shift ( LSB(MSW(spi_addr)) ); 
        spi_shift ( MSB(LSW(spi_addr)) );
        spi_shift ( LSB(LSW(spi_addr)) );
        spi_shift ( 0 ); // dummy byte
    }
}

void spi_read(WORD bytes) {

    BYTE stoph,stopl;
    SET_APTR_STOP(bytes, EP6FIFOBUF, stoph, stopl);

    printf ( "spi_read, bytes: %d\n", bytes );

    AUTOPTRH1 = MSB((WORD)EP6FIFOBUF);
    AUTOPTRL1 = LSB((WORD)EP6FIFOBUF);

    do {
       XAUTODAT1 = spi_inshift();
    } while ( stoph != AUTOPTRH1 || stopl != AUTOPTRL1 );

    rdwr_data.bytes_avail = bytes;
}

void spi_write_buf( WORD len, BYTE* buf) REENTRANT {

    BYTE stoph,stopl;
    SET_APTR_STOP(len,buf,stoph, stopl);

    while ((spi_status() & 1) && !new_vc_cmd);
    //do {
    //    if (!(spi_status() & 1)) break; // write not in progress
    //    delay(100);
    //} while (--tmp);

    //if (spi_status()&1) {
    //   rdwr_data.aborted=TRUE; 
    //   return;
    //}

    printf ( "Writing next page data: %d\n", len );

    spi_cmd ( SPI_WREN, TRUE ); // WREN
    spi_cmd ( SPI_PP, FALSE ); // PP
    // 3 addr bytes
    spi_shift ( LSB(MSW ( spi_addr )) );
    spi_shift ( MSB(LSW ( spi_addr )) );
    spi_shift ( LSB(LSW ( spi_addr )) ); 

    spi_addr += len;

    AUTOPTRH1 = MSB( (WORD)buf );
    AUTOPTRL1 = LSB( (WORD)buf );

    do {
        spi_shift ( XAUTODAT1 ); 
    } while ( stoph != AUTOPTRH1 || stopl != AUTOPTRL1 );


    PA2=1;

}

BOOL spi_write() {

    WORD written=0;
//    WORD next_page_boundary= 256 - LSB(LSW(spi_addr));
    WORD next_write;
//    next_page_boundary &= 0x00ff;
//    printf ( "Bytes until page boundary: %d\n", next_page_boundary );

    do {
        // send only the number of bytes to the next page boundary 1st
//        if ( next_page_boundary ) {
//           spi_write_buf ( next_page_boundary, EP2FIFOBUF ); 
//           written = next_page_boundary;
//           next_page_boundary = 0;
//        }

        next_write = rdwr_data.bytes_avail - written > 256 ? 256 : rdwr_data.bytes_avail-written ;
        printf ( "Writing %d bytes\n", next_write );

        if (next_write) spi_write_buf ( next_write, EP2FIFOBUF + written ); 
        written += next_write;

    } while ( written < rdwr_data.bytes_avail );
    return TRUE;

}

