// Ultrasonic Kit Firmware © 2024 by DidierA is licensed under Attribution-ShareAlike 4.0 International. To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/4.0/
// Program for STC15F104W, to be used in "DC 12V Ultrasonic suspension soldering DIY Kit" from ali express ( https://m.media-amazon.com/images/I/A1v9YJLCl6L.pdf )
// based on https://github.com/platformio/platform-intel_mcs51/tree/master/examples/stc-blink
// and page 121 of "STC15F101E series MCU datasheet"

// STC15F104 : 2 timers, T0 and T2
//#include <reg51.h>     // old header from SDCC
#include "STC15Fxx.h"  // Official header from STC-ISP
#include <stdint.h>
#include <stdio.h>

// #define FOSC 11059200L
#define FOSC 12000000L
#define T1MS (65536-FOSC/1000) // inital value of timer for 1 millisecond
#define T1uS (65536 - FOSC / 1000000) // initial value for 1 microsecond

// Global used for Led flashing
unsigned int count ;

// state of buttons
enum ButtonStates { ON, OFF};
enum ButtonStates but_plus = OFF ;
enum ButtonStates but_minus = OFF ;

enum Timers { T0, T2} ;

// Timer 0 divider
unsigned int time_div = 150 ;// 12.5 micro seconds at 12MHz : half period for 40kHz

// Pins that goes to TC4427 : P32 and P33
#define Pin1 P32
#define Pin2 P33
#define PinByte P3
#define PinMask 0b00001100   // P32 and P33 in P3

// Pins used for programming and button control, linked to J2
#define PinButPlus P31          // 
#define PinButMinus P30       // INT4/

// Pin linkinf LED to ground. LED is lit when this pin is at 0.
#define PinLED P34

/* utility function to set timerX divider*/
void set_timer (enum Timers timer, unsigned int value) {
    unsigned int new_value = 65536 - value ;
    switch(timer) {
        case T0:
            TL0 = new_value & 0xFF ;   //initial timer0 low byte
            TH0 = new_value >> 8 ; //initial timer0 high byte
            break ;
        case T2:
            T2L = new_value & 0xFF ;
            T2H = new_value >> 8 ;
            break ;
    }
}

/* Timer0 interrupt routine */
void tm0_isr() __interrupt(1)  {
    // invert both pins
    PinByte = ( PinByte ^ PinMask ) & 0xFF ;
}

void main()
{
    P3 = 0x00 ;
    // setup all of P3 as high impedance 
    // P3M1 = 0xFF ;
    // P3M0 = 0x00 ;
    // // setup P3: 0,1,5 as high impedance, 2,3,4 as output to strong pull-up output 
    // P3M1 = 0xE3 ; // 1110 0011
    // P3M0 = 0x1C ; // 0001 1100

    // setup P3: 0,1,5 as high impedance, 4 as open drain, 2,3  as quasi - bidirectional
    // P3M1 = 0xF3 ; // 1111 0011
    // P3M0 = 0x10 ; // 0001 0000

    // setup P3: 5 as high impedance, 4 as open drain, 0,1,2,3 as quasi-bidirectional
    P3M1 = 0b1110000 ;
    P3M0 = 0b0010000 ; 

    PinLED = 0x00 ; //turn LED On

    // turn pull-up on on button pins 
    PinButPlus = 1 ;
    PinButMinus = 1;

    // initial output values
    Pin1 = 1 ;
    Pin2 = 0 ;

    // set timers:
    AUXR = 0x80 ;    //timer0 work in 1T mode
    TMOD = 0x00 ;    //set timer0 as mode0 (16-bit auto-reload)

    set_timer(T0, time_div) ;

    TR0 = 1;    //timer0 start running
    ET0 = 1;    //enable timer0 interrupt
    EA = 1; //open global interrupt switch

    while (1) {
        // handle button push TODO: debounce 
        if ((PinButPlus == 0) && (but_plus == OFF)) { // Button Plus pressed
            time_div-- ; // if button up is pressed, decrease divider.
            set_timer(T0, time_div) ;
            but_plus = ON ;
            PinLED = 1 ; // turn LED off while button is pressed
        }

        if ((PinButPlus == 1) && (but_plus == ON)) {
            but_plus = OFF ;
            PinLED = 0 ; // turn LED back on 
        }

        if ((PinButMinus == 0) && (but_minus == OFF)) { 
            time_div++ ;
            set_timer(T0, time_div) ;
            but_minus = ON ;
            PinLED = 1 ;
        }

        if ((PinButMinus == 1)  && (but_minus == ON)) {
            but_minus = OFF ;
            PinLED = 0 ;
        }
    }; // loop

}
