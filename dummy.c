#include <fx2regs.h>
#include "dummy.h"

void dummy_reader(WORD len) {
 int c;
 for (c=0;c<len;++c) {
  EP6FIFOBUF[c] = c % 0xff;
 }
 rdwr_data.bytes_avail = len;
}

BOOL dummy_writer() {
 return TRUE;
}


