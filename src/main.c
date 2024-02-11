// Program for STC15F104W, to be used in "DC 12V Ultrasonic suspension soldering DIY Kit" from ali express ( https://m.media-amazon.com/images/I/A1v9YJLCl6L.pdf )
// based on https://github.com/platformio/platform-intel_mcs51/tree/master/examples/stc-blink
//#include <reg51.h>     // old header from SDCC
#include "STC15Fxx.h"  // Official header from STC-ISP
#include <stdint.h>
#include <stdio.h>

#define Pin1 P32
#define Pin2 P33
#define PinLED P34

void delay_halfperiod() {
    unsigned int i=10; // for 11.0592 MHz clock
    while (--i);
}

void main()
{
    // // setup P3: 0,1,5 as high impedance, 2,3,4 as output to strong pull-up output 
    // P3M1 = 0xE3 ; // 1110 0011
    // P3M0 = 0x1C ; // 0001 1100
    
    // setup P3: 0,1,5 as high impedance, 4 as open drain, 2,3  as quasi - bidirectional
    P3M1 = 0xF3 ; // 1111 0011
    P3M0 = 0x10 ; // 0001 0000

    PinLED = 0x00 ; //turn LED On
    Pin1 = 0x00 ;
    Pin2 = 0x00 ;
    unsigned int loops = 0 ;
    unsigned char on=0x00 ;
    while (1)
    {
        Pin1= 0xff ;
        delay_halfperiod() ;
        Pin1 = 0x00 ;

        Pin2 = 0xff ;
        delay_halfperiod() ;
        Pin2 = 0x00 ;

        // blink LED... This adds about 7 uS at 5.99MHz
        // if (++loops > 2000) {
        //     on = 0xFF - on ; // invert value
        //     PinLED = on ;
        //     loops = 0 ;
        // }
    }
}