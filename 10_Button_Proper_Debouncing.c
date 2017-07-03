/*
 * This code illustrates a method to debounce a button effectively with Timers and Interrupts
 *
 * Upon detection of a falling edge on the pin connected to the switch, the Port1 Interrupt is called
 * This in turn starts the Timer. Every 0.001 seconds, the Timer gives an interrupt, and the switch is read
 * If the switch remains in the same state for 10 checks in a row, we can confirm the state of the switch and
 * perform appropriate action.
 *
 * Make sure the jumper connecting P1.0 to the LED is connected.
 * The cystall oscillator need not be connected. We are using the internal oscillator.
 */
#include <msp430.h> 

int count=0,state; //Declare required variables

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    P1DIR&=~BIT3; //Set P1.3 as input
    P1REN|=BIT3; //Enable pullup/pulldown resistors for P1.3
    P1OUT|=BIT3; //Set P1.3 to have pull up resistors

    P1IE|=BIT3; //Enable interrupt on P1.3
    P1IES|=BIT3; //Set interrupt flag on the falling edge of logic level on P1.3

    TACCR0=1000; //Make the timer count from 0 to 10000, which will take  ~0.001 seconds
    __enable_interrupt(); //Enable maskable interrupts

    P1DIR|=BIT0; //Set P1.0 as output
    P1OUT&=~BIT0; //Initially turn off the LED
	
    __low_power_mode_0(); //Go to low power mode 0

	return 0;
}

#pragma vector=PORT1_VECTOR
__interrupt void P1_Function()
{
    count=0; //Reset count
    TACTL|=TASSEL_2+MC_1+TAIE; //Start Timer0 with SMCLK clock source, UP mode and enable overflow interrupt
    state=(P1IN&BIT3)>>3; //Save the state of the switch
    P1IE&=~BIT3; //Disable interrupt on P1.3, now the Timer will take care of Debouncing
    P1IFG&=~BIT3; // Reset Port1 interrupt flag
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TMR0()
{
    if(TAIV==TA0IV_TAIFG)   //Check if Timer overflow caused the interrupt
                            //This would be required in projects where multiple interrupts have
                            //the same interrupt vector. Here it is only optional.
        {
            if(state==((P1IN&BIT3)>>3)) //If the state of the LED is the same
                count++; //Increment the counter variable
            else
            {
                count=0; //If not same, reset the counter variable
                state=((P1IN&BIT3)>>3); //And save the present state of the switch
            }
        if(count==10) //If the state has been consistently the same
            {
            if(state==0) //If the switch was pressed
                P1OUT^=BIT0; //Toggle the LED
            P1IE|=BIT3; //We have handled the debouncing, now we again enable interrupt on P1.3, for it to again detect switch bounce
            TACTL=0; //Stop the Timer
            TACTL|=TACLR; //Clear the Timer counter
            }

            TACTL&=~(TAIFG); //Reset the interrupt flag
        }
}
