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
#define T1MS (FOSC/1000) // inital value of timer for 1 millisecond
#define T5MS (5*(FOSC/1000)) // initial value for 5 milllisecond
#define T1US (FOSC/1000000) // initial value for 1 microsecond

enum Timers {T0, T2} ;

// Timer 0 divider
volatile unsigned int time_div = 150 ;// 12.5 micro seconds at 12MHz : half period for 40kHz

/* Updating timer counter initial value does not work from within the ISR routine,
  * so it sets the variable value, and sets shpuld_update to 1
  * main routine will update counter when should_update == 1 */
volatile __bit should_update = 0 ;

// Pins that goes to TC4427 : P32 and P33
#define Pin1 P32
#define Pin2 P33
#define PinByte P3
#define PinMask 0b00001100   // P32 and P33 in P3

// Pins used for programming and button control, linked to J2
#define PinButPlus P31          // 
#define PinButMinus P30       // INT4/

// Pin linking LED to ground. LED is lit when this pin is at 0.
#define PinLED P34

/* utility function to set timerX divider*/
void set_timer (enum Timers timer, unsigned int value) __critical {
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

/* Timer0 interrupt routine: signal generation */
INTERRUPT(tm0_isr, 1)  {
    // invert both pins
    PinByte = ( PinByte ^ PinMask ) & 0xFF ;
}

/* Timer2 interrupt routine: butons handling */
/* will be called every 5 ms */
INTERRUPT(tm2_isr,12) {
    // state of buttons
    enum ButtonStates { ON, OFF, ON_OFF, OFF_ON};
    static enum ButtonStates but_plus = OFF ;
    static enum ButtonStates but_minus = OFF ;

    // Button Plus pressed
    if (PinButPlus == 0) {
        switch(but_plus) {
            case OFF:   // register state transition from OFF to ON
                but_plus = OFF_ON ;
                break ;
            case ON_OFF:    // bounce: cancel state transition
                but_plus = ON ;
                break ;
            case OFF_ON:    // confirmed press
                if (time_div >25) {
                    time_div-=5 ; // if button Plus is pressed, decrease divider.
                    should_update = 1 ;
                }
                but_plus = ON ;
                PinLED = 1 ; // turn LED off while button is pressed
                break ;                
        }
    }
    // Button Plus released
    if (PinButPlus == 1) {
        switch(but_plus) {
            case ON:    // register state transition
                but_plus = ON_OFF ;
                break ;
            case OFF_ON:    // bounce: cancel state transition
                but_plus = OFF ;
                break ;
            case ON_OFF: // confirmed release
                but_plus = OFF ;
                PinLED = 0 ; // turn LED back on
                break ;
        }
    }
    // Button Minus pressed
    if (PinButMinus == 0) {
        switch(but_minus) {
            case OFF:
                but_minus = OFF_ON ;
                break ;
            case ON_OFF:
                but_minus = ON ;
                break ;
            case OFF_ON:
                if (time_div < 65531) {
                    time_div+=5 ;
                    should_update = 1 ;
                }
                but_minus = ON ;
                PinLED = 1 ;
                break ;
        }
    }
    // Button Minus released
    if (PinButMinus == 1){
        switch (but_minus) {
            case ON:
                but_minus = ON_OFF ;
                break ;
            case OFF_ON:
                but_minus  = OFF ;
                break ;
            case ON_OFF:
                but_minus = OFF ;
                PinLED = 0 ;
                break ;
        }
    }
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
    // timer0 used for ultrasonic signal generation
    set_timer(T0, time_div) ;
    AUXR |= 1 << 7 ; // // B7=1: Timer0 in 1T mode
    TMOD = 0x00 ;    //set timer0 as mode0 (16-bit auto-reload)
    TR0 = 1;    //timer0 start running
    ET0 = 1;    //enable timer0 interrupt

    // timer2 used for button debounce    
    set_timer(T2, T5MS) ; 
    AUXR |= 1 << 4 ;    // B4=1: run Timer2
    AUXR &= ~(1 << 3) ;    // B3=0: Timer2 as timer
    AUXR |= 1 << 2 ;    // B2=1: Timer2 in 1T mode    

    IE2 |= 1 << 2 ;   // enable timer2 interrupt

    EA = 1; //open global interrupt switch

    while (1) {
        if (should_update) {
            set_timer(T0, time_div) ;
            should_update=0 ;
        }
    } // loop

}
