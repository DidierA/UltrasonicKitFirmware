// Program for STC15F104W, to be used in "DC 12V Ultrasonic suspension soldering DIY Kit" from ali express ( https://m.media-amazon.com/images/I/A1v9YJLCl6L.pdf )
// based on https://github.com/platformio/platform-intel_mcs51/tree/master/examples/stc-blink
// and page 121 of "STC15F101E series MCU datasheet"
//#include <reg51.h>     // old header from SDCC
#include "STC15Fxx.h"  // Official header from STC-ISP
#include <stdint.h>
#include <stdio.h>

// #define FOSC 11059200L
#define FOSC 6000000L
#define T1MS (65536-FOSC/1000) // inital value of timer for 1 millisecond
#define T1uS (65536 - FOSC / 1000000) // initial value for 1 microsecond

#define T40kHz (65536 - 75 ) // initial value of timer for half period of 40KHz (12.5 micro seconds)

// Global used for Led flashing
unsigned int count ;

#define Pin1 P32
#define Pin2 P33
#define PinLED P34

/* Timer0 interrupt routine */
void tm0_isr() __interrupt(1)  {
    /* invert both pins*/
    Pin1 = !Pin1 ;
    Pin2 = !Pin2 ;
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

    Pin1 = 1 ;
    Pin2 = 0 ;

    // set timers:
    AUXR = 0x80 ;    //timer0 work in 1T mode
    TMOD = 0x00 ;    //set timer0 as mode0 (16-bit auto-reload)

    TL0 = T40kHz & 0xFF ;   //initial timer0 low byte
    TH0 = T40kHz >> 8 ; //initial timer0 high byte

    TR0 = 1;    //timer0 start running
    ET0 = 1;    //enable timer0 interrupt
    EA = 1; //open global interrupt switch

    while (1) ; // loop

}
