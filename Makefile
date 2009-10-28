FX2LIB = lib/fx2lib
INCLUDES = -I$(FX2LIB)/include
LIBS = fx2.lib -L$(FX2LIB)/lib
VID=0x1fe1
PID=0x4830

# possible env flags
# -D DEBUG_FIRMWARE - enable stdio & printf on sio-0 (57600 buad)
SDCCFLAGS := $(SDCCFLAGS)

###
# dscr is under 200 bytes currently
# int2jt is under 200 bytes
# 400 bytes for xram

# usbjt 4000 - 100(256) = 3f00 (must lie on page boundary) 
# dscr 3f00 - 100(256) = 3e00 
# xram 3e00 - 200(512) = 3c00 
###

CC = sdcc -mmcs51 \
	$(SDCCFLAGS) \
   --code-size 0x3c00 \
   --xram-size 0x0200 \
    --xram-loc 0x3c00 \
	-Wl"-b DSCR_AREA = 0x3e00" \
	-Wl"-b INT2JT = 0x3f00" \

BASENAME = firmware

SOURCES = firmware.c \
          dummy.c \
		  main.c \
		  handlers.c \
		  fx2term.c \
		  spi.c \
		  eeprom.c \
		  bitfile.c

OBJS = $(patsubst %.c,%.rel, $(SOURCES)) dscr.rel 

.PHONY: all
all: $(BASENAME).ihx


%.rel: %.c
	$(CC) -c $(INCLUDES) $?

$(FX2LIB)/lib/fx2.lib:
	make -C $(FX2LIB)/lib/

$(BASENAME).ihx: $(OBJS) $(FX2LIB)/lib/fx2.lib
	$(CC) $(OBJS) $(LIBS) -o $(BASENAME).ihx

dscr.rel: dscr.a51
	asx8051 -logs $?

$(BASENAME).bix: $(BASENAME).ihx
	objcopy -I ihex -O binary $(BASENAME).ihx $(BASENAME).bix

$(BASENAME).iic: $(BASENAME).ihx
	$(FX2LIB)/utils/ihx2iic.py -v $(VID) -p $(PID) $(BASENAME).ihx $(BASENAME).iic

clean:
	rm -f *.{asm,ihx,lnk,lst,map,mem,rel,rst,sym,adb,cdb,bix,iic}

.PHONY: bix iic

bix: $(BASENAME).bix

iic: $(BASENAME).iic

fx2_di.xml: fx2_di.py
	python -c "from nitro import XmlWriter; import fx2_di; x=XmlWriter('fx2_di.xml'); x.write(fx2_di.di)"
