

#include <fx2macros.h>

#include "firmware.h"
#include "vendor_commands.h"
#include "handlers.h"
#include "terminals.h"

#include "dummy.h"
#include "fx2term.h"
#include "spi.h"
#include "eeprom.h"
#include "bitfile.h"


io_handler code io_handlers[] = {
#ifdef DEBUG_FIRMWARE
 {TERM_DUMMY, dummy_reader, dummy_writer, 0, 0, 0}, 
#endif
 {TERM_FX2, get_fx2, set_fx2, 0, 0, 0},
 {TERM_FX2_IO, get_fx2, set_fx2, 0, 0, 0},
 {TERM_SPIFLASH, get_spi, set_spi, spi_init, 0, 0},
 {TERM_SPIFLASH_DATA, spi_read, spi_write, spi_init, 0, 0}, 
 {TERM_FX2PROM, eeprom_read_handler, eeprom_write_handler, 0, 0, 0},
 {TERM_FPGA_bitfile, dummy_reader, write_bitfile, init_bitfile, 0, 0},
 {TERM_FPGA_parbit, dummy_reader, write_parbitfile, init_parbitfile, 0, 0},
 {0, 0, 0, 0, 0, 0}
};
