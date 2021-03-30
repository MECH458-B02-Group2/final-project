#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include "mainHeader.h"

// Main Start
int main(int argc, char *argv[]){
	
	CLKPR = 0x80;
	CLKPR = 0x01;		//  sets system clock to 8MHz
	TCCR1B |= _BV(CS11); // Set the timer 1 prescaler to 8 --> f=1MHz
	TCCR0B |= _BV(CS01); // Set the timer prescalar to 8 --> 3.9 kHz (8MHz/(N*256)
	
	STATE = 0;

	cli();		// Disables all interrupts
	
	// Set up the Interrupt 0,3 options
	// See page 112 - EIFR External Interrupt Flags...notice how they reset on their own in 'C'...not in assembly
	// EIMSK |= 0x0C; // == 0b00000100 | 0b00000010 // THIS LINE WAS IN THE EXAMPLE CODE BUT WE REPLACED IT WITH THE BELOW CODE
	// EIMSK |= (_BV(INT1)); // enable INT1
	// EIMSK |= (_BV(INT2)); // enable INT2
	EIMSK |= (_BV(INT4)); // enable INT2
	//External Interrupt Control Register A - EICRA (pg 110 and under the EXT_INT tab to the right
	// Set Interrupt sense control to catch a rising edge
	//EICRA |= _BV(ISC21) | _BV(ISC20);
	//EICRA |= _BV(ISC31) | _BV(ISC30);
	EICRA |= _BV(ISC41) | _BV(ISC40);

	//	EICRA &= ~_BV(ISC21) & ~_BV(ISC20); /* These lines would undo the above two lines */
	//	EICRA &= ~_BV(ISC31) & ~_BV(ISC30); /* Nice little trick */

	// Configure ADC -> by default, the ADC input (analog input) is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	ADMUX |= _BV(ADLAR) | _BV(REFS0); // Read Technical Manual & Complete Comment

	// Initialize ports
	DDRD = 0b11110000;	// Going to set up INT2 & INT3 on PORTD
	DDRC = 0xFF;		// just use as a display
	PORTC = 0x00; // Set display off to start
	// Set PORTB (DC motor port) to output (B7 = PWM, B3 = IA, B2 = IB, B1 = EA, B0 = EB)
	DDRB = 0xFF; // Initialize port B for output to motor driver
	PORTB = 0x00; //Initialize all pins to be low
	DDRA = 0b00111111; // A7 as input for HE sensor, A0-A5 as output for stepper motor
	PORTA = 0b00000000;

	// Home Stepper Motor
	initialize();
	
	// Enable PWM for motor pin
	PWM(); // Initialize PWM
	// Enable all interrupts
	
	sei();	// Note this sets the Global Enable for all interrupts
	
	// Start running the motor
	PORTB = 0x07; // Motor running forward
	
	goto POLLING_STAGE;
	
	// POLLING STATE
	POLLING_STAGE:
	PORTC |= 0xF0;	// Indicates this state is active
	
	switch(STATE){
		case (0) :
		goto POLLING_STAGE;
		break;	//not needed but syntax is correct
		case (1) :
		goto MAGNETIC_STAGE;
		break;
		case (2) :
		goto REFLECTIVE_STAGE;
		break;
		case (3) :
		goto BUCKET_STAGE;
		break;
		case (4) :
		goto PAUSE_STAGE;
		break;
		case (5) :
		goto END;
		default :
		goto POLLING_STAGE;
	}//switch STATE
	
	MAGNETIC_STAGE:
	// Do whatever is necessary HERE
	PORTC = 0x01; // Just output pretty lights know you made it here
	//Reset the state variable
	STATE = 0;
	goto POLLING_STAGE;

	REFLECTIVE_STAGE:
	// Do whatever is necessary HERE
	PORTC = 0x04; // Just output pretty lights know you made it here
	//Reset the state variable
	STATE = 0;
	goto POLLING_STAGE;
	
	BUCKET_STAGE:
	// Do whatever is necessary HERE
	PORTC = 0x08;
	//Reset the state variable
	STATE = 0;
	goto POLLING_STAGE;
	
	PAUSE_STAGE:
	
	PORTC = 0b00001000;
	DC_Stop();
	while(STATE == 4);
	DC_Start();
	
	STATE = 0;
	goto POLLING_STAGE;
	
	END:
	// The closing STATE ... how would you get here?
	PORTC = 0xF0;	// Indicates this state is active
	// Stop everything here...'MAKE SAFE'
	// cli();
	return(0);

}

//------------------------------------------------------------------------------------------------------//
// STEPPER MOTOR SUBROUTINES ---------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

//Homing function
void initialize(void) {

	// DEBUG - Check to see if we enter this function
	PORTC = 0b01010101;
	mTimer(1000);

	PolePosition = 0; // set the zero
	
	while((PINA & 0b10000000) != 0b10000000) {
		stepcw(1);
	}
	
	// DEBUG - Check to see that we've passed the while loop above
	PORTC = 0b11111111;
	mTimer(1000);

	PolePosition = 0;
	CurPosition = 0;

	
	// NOTE: We do not have our actual position

} // Homing Function

void stepcw (int step) {
	PolePosition++; // move the current position forward one
	
	for (int j=0; j<step; j++) {

		if (PolePosition==5) {
			PolePosition=1;
		}
		
		switch (PolePosition) {
			case 1:
			PORTA = 0b00110000;
			break;
			case 2:
			PORTA = 0b00000110;
			break;
			case 3:
			PORTA = 0b00101000;
			break;
			case 4:
			PORTA = 0b00000101;
			break;
			default:
			PORTA = 0;
			break;
		}
		
		if (j<step-1) {
			PolePosition++;
		}
		
		CurPosition++;
		
		mTimer(delay); // Step Delay
		
	} // for
} // stepcw

void stepccw (int step) {
	PolePosition--; // move the current position back one
	
	for (int j=0; j<step; j++) {

		if (PolePosition==0) {
			PolePosition=4;
		}

		switch (CurPosition) {
			case 1:
			PORTA = 0b00110000;
			break;
			case 2:
			PORTA = 0b00000110;
			break;
			case 3:
			PORTA = 0b00101000;
			break;
			case 4:
			PORTA = 0b00000101;
			break;
			default:
			PORTA = 0;
			break;
		}
		
		if (j<step-1) {
			PolePosition--;
		}
		
		CurPosition--;
		
		mTimer(delay); // StepDelay
		
	} // for
} // stepccw

//------------------------------------------------------------------------------------------------------//
// DC MOTOR SUBROUTINES --------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

void DC_Start(void) {
	// Start running the motor
	PORTB = 0x07; // Motor running forward
	return;
} // Motor start

void DC_Stop(void) {

	PORTB = 0x0F; // Brake
	mTimer(10); // wait 10 ms
	return;
} // Motor stop

//------------------------------------------------------------------------------------------------------//
// TIMERS ----------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

void PWM(void) {
	
	TCCR0A|= _BV(WGM00); // Enable fast PWM
	TCCR0A|= _BV(WGM01); // Enable fast PWM
	
	//TIMSK0|= _BV(OCIE0A); // Set output compare match A enable bit to 1 for timer 0
	TCCR0A|= _BV(COM0A1); // Clear OC0A on compare match and set at bottom
	OCR0A = Speed; // set ORC0A to 128 for a 50% duty cycle
	return;
} // PWM

void mTimer(int count) {
	int i = 0;
	TCCR1B |= _BV(WGM12); // Set CTC mode
	OCR1A = 0x03e8; // Set output compare register to 1000 cycles = 1ms
	TCNT1 = 0x0000; // Set initial value of timer counter
	//TIMSK1 = TIMSK1 | 0b00000010; // (DO NOT) Enable the output compare interrupt enable
	TIFR1 |= _BV(OCF1A); // Clear the timer interrupt flag and begin new timing
	
	// Poll the timer to determine when the timer has reached 0x03e8 (1000)
	while(i < count) {
		if((TIFR1 & 0x02) == 0x02) {
			TIFR1 |= _BV(OCF1A);
			i++; // increment loop number if timer counter has reached output compare register
		} // end if
	} // end while

	return;

} // mTimer

//------------------------------------------------------------------------------------------------------//
// LINKED QUEUE SUBROUTINES ----------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

/****************************************************************************************
*  DESC: Accepts as input a new link by reference, and assigns the head and tail
*  of the queue accordingly
*  INPUT: the head and tail pointers, and a pointer to the new link that was created
*/
/* will put an item at the tail of the queue */
void enqueue(link **h, link **t, link **nL){

	if (*t != NULL){
		/* Not an empty queue */
		(*t)->next = *nL;
		*t = *nL; //(*t)->next;
	}/*if*/
	else{
		/* It's an empty Queue */
		//(*h)->next = *nL;
		//should be this
		*h = *nL;
		*t = *nL;
	}/* else */
	return;
}/*enqueue*/

/**************************************************************************************
* DESC : Removes the link from the head of the list and assigns it to deQueuedLink
* INPUT: The head and tail pointers, and a ptr 'deQueuedLink'
* 		 which the removed link will be assigned to
*/
/* This will remove the link and element within the link from the head of the queue */
void dequeue(link **h, link **t, link **deQueuedLink){
	/* ENTER YOUR CODE HERE */
	*deQueuedLink = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;
	}/*if*/
  
  /* If the queue is empty and h is NULL, set t to NULL */
  if (*h == NULL){
    *t = NULL;
  }

	return;
}/*dequeue*/

//------------------------------------------------------------------------------------------------------//
// INTERRUPT SERVICE ROUTINES --------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

/* Set up the External Interrupt 2 Vector */
ISR(INT2_vect){
	/* Toggle PORTC bit 2 */
	STATE = 2;
}

ISR(INT3_vect){
	/* Toggle PORTC bit 3 */
	STATE = 3;
}

// When the button is pressed, set Escape GV to 1
ISR(INT4_vect) {
	while ((PINE & 0b00010000) == 0);		// ACTIVE-LO sits in loop until button is released (masking button bit)
	
	if(STATE == 0) {
		STATE = 4;
	}
	
	if(STATE == 4) {
		STATE = 0;
	}
} // Break and end interrupt

ISR(ADC_vect) {
	ADC_result = ADCH; // Store the ADC reading in the global variable
	ADC_result_flag = 1; // Indicate that there is a new ADC result to change PWM frequency and to be displayed on LEDs
} // ADC end

// If an unexpected interrupt occurs (interrupt is enabled and no handler is installed,
// which usually indicates a bug), then the default action is to reset the device by jumping
// to the reset vector. You can override this by supplying a function named BADISR_vect which
// should be defined with ISR() as such. (The name BADISR_vect is actually an alias for __vector_default.
// The latter must be used inside assembly code in case <avr/interrupt.h> is not included.
ISR(BADISR_vect)
{
	// user code here
}














