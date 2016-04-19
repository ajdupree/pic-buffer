/**
 *******************************************************************
 *  PICKit 3 Starter Kit Circular Buffer implementation
 * 
 * Loosely based on sample code #10 - interrupts 
 * Requires one momentary pushbutton external to the starter kit.
 * This button should connect RA3 and GND, and connect RA3 to ground when
 * the button is pushed.
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
#define SWITCH2             PORTAbits.RA3
#define LED_RIGHT           1
#define LED_LEFT            0
#define PULL_UPS  //if this is uncommented, the trace under JP5 can be cut with no ill effects
#define SIZE 4

//config bits that are part-specific for the PIC18F14K22
__CONFIG(1, FOSC_IRC & PLLEN_OFF & FCMEN_OFF);
__CONFIG(2, PWRTEN_OFF & BOREN_OFF & WDTEN_OFF);
__CONFIG(3, HFOFST_OFF & MCLRE_OFF);
__CONFIG(4, STVREN_ON & LVP_OFF & DEBUG_ON);
__CONFIG(5, CP0_OFF & CP1_OFF & CPB_OFF & CPD_OFF);
__CONFIG(6, WRT0_OFF & WRT1_OFF & WRTC_OFF & WRTB_OFF & WRTD_OFF);
__CONFIG(7, EBTR0_OFF & EBTR1_OFF & EBTRB_OFF);

/*globals*/
unsigned char prev_switch_state; //need to add this for the PIC18 since there is no specific interrupt-on-change NEGATIVE....
unsigned char prev_switch_state_2;
int pushIndex, popIndex;
unsigned char buffer[SIZE] = {0,0,0,0};

    /* -------------------LATC-----------------
     * Bit#:  -7---6---5---4---3---2---1---0---
     * LED:   ---------------|DS4|DS3|DS2|DS1|-
     *-----------------------------------------
     */

/*function prototypes*/
void push();
void pop();
int next();

/*functions*/

//track where the next push should go
int next()
{
	if (pushIndex < SIZE-1) return pushIndex+1;
	else return 0;
}

//push function - add data to next open spot in the buffer
void push()
{
	buffer[pushIndex] = UP;

	//decide to increment the popIndex or not
	if(pushIndex == popIndex && buffer[next()] != 0)
	{
		popIndex++;
	}

	//always increment push index
	pushIndex++;

	//reset indices if necessary
	if(pushIndex == SIZE) pushIndex = 0;
	if(popIndex == SIZE) popIndex = 0;
}

//pop function - remove oldest item from buffer
void pop()
{
	if (buffer[popIndex] == 0)
	{
        //do nothing
		return;
	}
	else
	{
		buffer[popIndex]=DOWN;
		popIndex++;
		if(popIndex == SIZE) popIndex=0;
	}
}

//map the buffer state to the LED array
void output()
{
    LATCbits.LATC0 = buffer[0];
    LATCbits.LATC1 = buffer[1];
    LATCbits.LATC2 = buffer[2];
    LATCbits.LATC3 = buffer[3];    
}

void main(void) {
    OSCCON = 0b00100010;  //500KHz clock speed
    TRISC = 0; //all LED pins are outputs   
    TRISAbits.TRISA2 = 1; //switch 1 input
    ANSELbits.ANS2 = 0; //digital for switch   
        
    //initialize LED array to all off
    LATC = 0b00000000; 
    
#ifdef PULL_UPS
    //by using the internal resistors, you can save cost by eliminating an external pull-up/down resistor
    WPUA2 = 1; //enable the weak pull-up for the switch     
    //this bit is active HIGH, meaning it must be cleared for it to be enabled
    nRABPU = 0; //enable the global weak pull-up bit    
#endif

    //setup interrupt on change for the switch
    INTCONbits.RABIE = 1; //enable interrupt on change global
    IOCAbits.IOCA2 = 1; //when SW1 is pressed/released, enter the ISR
    IOCAbits.IOCA3 = 1; //when SW2 is pressed/release, enter the ISR too 

    RCONbits.IPEN = 0; //disable interrupt priorities
    INTCONbits.GIE = 1; //enable global interrupts
    
    while (1)
    {
        continue; //can spend rest of time doing something critical here
    }
}

void interrupt ISR(void) 
{
    //if we're here, one of the switches was pressed    
    //must clear the flag in software
    INTCONbits.RABIF = 0; 
    //debounce by waiting and seeing if still held down  
    __delay_ms(5);       

    /*
     * figure out which switch was pressed by comparing current/prev states 
     */
    
    //switch1       
    //on falling edge, take action
    if (SWITCH == DOWN && prev_switch_state == UP) pop();                    
    //track previous state
    if (SWITCH == DOWN) prev_switch_state = DOWN;
    else prev_switch_state = UP;

    //switch2
    //on falling edge, take action
    if(SWITCH2 == DOWN && prev_switch_state_2 == UP) push();           
    //track previous state
    if(SWITCH2) prev_switch_state_2 = UP;
    else prev_switch_state_2 = DOWN;

    //update the LED array with buffer state
    output();            
}