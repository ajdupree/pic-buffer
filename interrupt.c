/**
 *******************************************************************
 *  Lesson 10 - "Interrupts and Pull-ups"
 *
 *  This lesson will introduce interrupts and how they are useful. It will
 *  also introduce internal weak pull-ups that are available on most PICs.
 *
 *  It should be noted that this lesson is more efficent than the last
 *  one, "Timer0". Notice how the processor is no longer waiting for
 *  Timer0 to roll over. Instead, we let the hardware modules do the work,
 *  freeing the CPU to do other things in the main loop
 *
 *  The switch is no longer continuously polled for a button press. Instead,
 *  an interrupt will occur which will automically place the program counter
 *  inside of the ISR where we can change directions outisde of normal code execution
 *
 *  LEDs rotate at a constant speed and the switch reverses their direction
 *
 *  PIC: 18F14K22
 *  Compiler: XC8 v1.00
 *  IDE: MPLABX v1.10
 *
 *  Board: PICkit 3 Low Pin Count Demo Board
 *  Date: 6.1.2012
 *
 * *******************************************************************
 * See Low Pin Count Demo Board User's Guide for Lesson Information*
 * ******************************************************************
 */

#include <htc.h>                         //PIC hardware mapping
#define _XTAL_FREQ 500000                //Used by the compiler for the delay_ms(x) macro

#define DOWN                0
#define UP                  1

#define SWITCH              PORTAbits.RA2

#define LED_RIGHT           1
#define LED_LEFT            0

#define PULL_UPS  //if this is uncommented, the trace under JP5 can be cut with no ill effects

//config bits that are part-specific for the PIC18F14K22
__CONFIG(1, FOSC_IRC & PLLEN_OFF & FCMEN_OFF);
__CONFIG(2, PWRTEN_OFF & BOREN_OFF & WDTEN_OFF);
__CONFIG(3, HFOFST_OFF & MCLRE_OFF);
__CONFIG(4, STVREN_ON & LVP_OFF & DEBUG_ON);
__CONFIG(5, CP0_OFF & CP1_OFF & CPB_OFF & CPD_OFF);
__CONFIG(6, WRT0_OFF & WRT1_OFF & WRTC_OFF & WRTB_OFF & WRTD_OFF);
__CONFIG(7, EBTR0_OFF & EBTR1_OFF & EBTRB_OFF);

//globals
unsigned char _prev_switch; //need to add this for the PIC18 since there is no specific interrupt-on-change NEGATIVE....

    /* -------------------LATC-----------------
     * Bit#:  -7---6---5---4---3---2---1---0---
     * LED:   ---------------|DS4|DS3|DS2|DS1|-
     *-----------------------------------------
     */

void main(void) {
    OSCCON = 0b00100010;  //500KHz clock speed
    TRISC = 0; //all LED pins are outputs

    TRISAbits.TRISA2 = 1; //switch 1 input
    ANSELbits.ANS2 = 0; //digital for switch
    
    TRISAbits.TRISA0 = 1; //switch 2 input
    ANSELbits.ANS0 = 0; //switch 2 digital
    
    LATC = 0b00000011; //start with DS4 lit
    
#ifdef PULL_UPS
    //by using the internal resistors, you can save cost by eliminating an external pull-up/down resistor
    WPUA2 = 1; //enable the weak pull-up for the switch
    WPUA0 = 1; //add pull-up for switch 2
    //this bit is active HIGH, meaning it must be cleared for it to be enabled
    nRABPU = 0; //enable the global weak pull-up bit
    
#endif

    //setup interrupt on change for the switch
    INTCONbits.RABIE = 1; //enable interrupt on change global
    IOCAbits.IOCA2 = 1; //when SW1 is pressed/released, enter the ISR

    RCONbits.IPEN = 0; //disable interrupt priorities
    INTCONbits.GIE = 1; //enable global interrupts


    while (1) {
        continue; //can spend rest of time doing something critical here
    }
}

void interrupt ISR(void) {

    if (INTCONbits.RABIF)
    {
        //SW1 was just pressed
        INTCONbits.RABIF = 0; //must clear the flag in software
        __delay_ms(5); //debounce by waiting and seeing if still held down
    
        if (SWITCH == DOWN && _prev_switch == UP)
        {
            //do nothing atm
        }
        
        if (SWITCH == DOWN)
        {
            _prev_switch = DOWN;
        } else
            _prev_switch = UP;
    }
    
}