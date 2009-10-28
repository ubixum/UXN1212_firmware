

#include <string.h>

#include <fx2regs.h>
#include <fx2macros.h>
#include <usbjt.h>
#include <setupdat.h>
#include <i2c.h>
#include <eputils.h>

#define SYNCDELAY() SYNCDELAY4;

#ifdef DEBUG_FIRMWARE
#include <stdio.h>
#define REENTRANT __reentrant
#else
#define printf(...)
#define REENTRANT
#endif

void reset_endpoints();


extern volatile xdata WORD in_packet_max;
extern volatile bit new_vc_cmd;
#define PROM_SERIAL_OFFSET 0xff9b // 0xffff-100 = last 100 bytes of the 16k prom


