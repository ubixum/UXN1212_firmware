

#ifdef DEBUG_FIRMWARE 
#include <serial.h>
#include <stdio.h>
#else
#define printf(...)
#endif

#include "firmware.h"


volatile bit dosud=FALSE;
volatile bit dosuspend=FALSE;
volatile bit new_vc_cmd;

// custom functions
extern void main_loop();
extern void main_init();


void main() {

 SETCPUFREQ(CLK_48M); // required for sio0_init 

#ifdef DEBUG_FIRMWARE
 // main_init can still set this to whatever you want.
 sio0_init(57600); // needed for printf if debug defined 
#endif

 main_init();

 // set up interrupts.
 USE_USB_INTS();
 
 ENABLE_SUDAV();
 ENABLE_USBRESET();
 ENABLE_HISPEED(); 
 // NOTE causes some Hydro boards to not enumerate?
// ENABLE_SUSPEND();
// ENABLE_RESUME();

 // this board has no WAKEUP
 WAKEUPCS &= ~0x03;

 EA=1;

// use RENUMERATE_UNCOND() if need to change device descriptor.
 RENUMERATE();
 
 while(TRUE) {

     main_loop();

     if (dosud) {
       dosud=FALSE;
       new_vc_cmd=0;
       handle_setupdata();
     }

     if (dosuspend) {
        dosuspend=FALSE;
        do {
           printf ( "I'm going to Suspend.\n" );
           WAKEUPCS |= bmWU|bmWU2; // make sure ext wakeups are cleared
           SUSPEND=1;
           PCON |= 1;
           SYNCDELAY(); SYNCDELAY();
        } while ( !remote_wakeup_allowed && REMOTE_WAKEUP()); 
        printf ( "I'm going to wake up.\n");

        // resume
        // trm 6.4
        if ( REMOTE_WAKEUP() ) {
            delay(5);
            USBCS |= bmSIGRESUME;
            delay(15);
            USBCS &= ~bmSIGRESUME;
        }

     }

 } // end while

} // end main

void resume_isr() interrupt RESUME_ISR {
 CLEAR_RESUME();
}
  
void sudav_isr() interrupt SUDAV_ISR {
 dosud=TRUE;
 CLEAR_SUDAV();
}
void usbreset_isr() interrupt USBRESET_ISR {
 in_packet_max=64;
 handle_hispeed(FALSE);
 EP6AUTOINLENH=MSB(in_packet_max); SYNCDELAY();
 EP6AUTOINLENL=LSB(in_packet_max); SYNCDELAY();
 CLEAR_USBRESET();
}
void hispeed_isr() interrupt HISPEED_ISR {
 in_packet_max=512;
 handle_hispeed(TRUE);
 EP6AUTOINLENH=MSB(in_packet_max); SYNCDELAY();
 EP6AUTOINLENL=LSB(in_packet_max); SYNCDELAY();
 CLEAR_HISPEED();
}

void suspend_isr() interrupt SUSPEND_ISR {
 dosuspend=TRUE;
 CLEAR_SUSPEND();
}
