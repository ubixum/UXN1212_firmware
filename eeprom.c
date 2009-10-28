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

