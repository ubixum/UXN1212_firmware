#include "i2c.h"
#include "firmware.h"
#include "eeprom.h"
#include "terminals.h"
#include "handlers.h"


void eeprom_read_handler ( WORD len ) {
 // NOTE can redo to not block?
 if(eeprom_read ( TERM_FX2PROM , rdwr_data.h.reg_addr, len, EP6FIFOBUF )) {
    rdwr_data.bytes_avail = len;
    rdwr_data.h.reg_addr += len;
 } else {
    rdwr_data.aborted=TRUE;
 }
}

BOOL eeprom_write_handler () {
    printf ( "eeprom write: %d bytes, %02x%02x...\n" , rdwr_data.bytes_avail, EP2FIFOBUF[0], EP2FIFOBUF[1] );
    if (eeprom_write( TERM_FX2PROM, rdwr_data.h.reg_addr, rdwr_data.bytes_avail, EP2FIFOBUF )) {
        rdwr_data.h.reg_addr += rdwr_data.bytes_avail;
    } else {
        rdwr_data.aborted=TRUE;
    }
    return TRUE;
}

