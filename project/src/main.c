#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include "mainHeader.h"
#include "lcd.h"

/*------------------------------------------------------------------------------------------------------*/
/* MAIN ROUTINE ----------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[]){

  /* INITIALIZATIONS -----------------------------------------------------------------------------------*/
	// #region

	// Description:

	// Clock & Timer Prescalers
	CLKPR = 0x80;
	CLKPR = 0x01;		//  sets system clock to 8MHz
	TCCR1B |= _BV(CS11); // Set the timer 1 prescaler to 8 --> f=1MHz
	TCCR0B |= _BV(CS01); // Set the timer prescalar to 8 --> 3.9 kHz (8MHz/(N*256)
	
	// LEDs and LCD module (PORTC)
	InitLCD(LS_BLINK|LS_ULINE);
	LCDClear();
	LCDWriteString("ACTIVE");
	DDRC = 0xFF;		// LED Output
	PORTC = 0x00; // Set display off to start

	// External Interrupts
	cli();		// Disables all interrupts
	// Set up the Interrupt 0,3 options
	// See page 112 - EIFR External Interrupt Flags...notice how they reset on their own in 'C'...not in assembly
	// EIMSK |= (_BV(INT1)); // enable INT1
	EIMSK |= (_BV(INT2)); // enable INT2
	EIMSK |= (_BV(INT3)); // enable INT2
	EIMSK |= (_BV(INT4)); // enable INT4
	//External Interrupt Control Register A - EICRA (pg 110 and under the EXT_INT tab to the right
	// Set Interrupt sense control to catch a rising edge
	EICRA |= _BV(ISC21) | _BV(ISC20); // Rising edge interrupt - active high
	EICRA |= _BV(ISC31); // | _BV(ISC30); Falling edge interrupt - active low
	EICRA |= _BV(ISC41); // | _BV(ISC40); Falling edge interrupt - active low
	//	EICRA &= ~_BV(ISC21) & ~_BV(ISC20); /* These lines would undo the above two lines */
	//	EICRA &= ~_BV(ISC31) & ~_BV(ISC30); /* Nice little trick */

	// A-D Conversion (ADC) (Reflective Sensor)
	// Configure ADC -> by default, the ADC input (analog input) is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	ADMUX |= _BV(ADLAR) | _BV(REFS0); // Read Technical Manual & Complete Comment

	// Variables
	int reflect_val;
	int bucket_psn;
	int bucket_val;
	int bucket_move;
	Alum = 0;
	Steel = 0;
	White = 0;
	Black = 0;

	// I/O Ports (Check necessity of these)
	DDRD = 0b11110000;	// Going to set up INT2 & INT3 on PORTD
	DDRA = 0b00111111; // A7 as input for HE sensor, A0-A5 as output for stepper motor
	PORTA = 0b00000000;
	
	// Stepper Motor
	step_home();
	
	// DC Motor
	// Set PORTB (DC motor port) to output (B7 = PWM, B3 = IA, B2 = IB, B1 = EA, B0 = EB)
	DDRB = 0xFF; // Initialize port B for output to motor driver
	PORTB = 0x00; //Initialize all pins to be low
	PWM(); // Initialize PWM
	// Start running the motor
	PORTB = 0b00000111; // Motor running forward
	
// #endregion
	
	// Enable all interrupts
	sei();	// Note this sets the Global Enable for all interrupts

	// Enter polling loop
	STATE = 0;
	goto POLLING_STAGE;

	/* POLLING STAGE -------------------------------------------------------------------------------------*/
	// #region

	// Description:
            
	POLLING_STAGE:
	
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
		goto END_STAGE;
		default :
		goto POLLING_STAGE;
	}//switch STATE

	// #endregion POLLING STAGE --------------------------------------------------------------------------//

	/* MAGNETIC STAGE ------------------------------------------------------------------------------------*/
	// #region

	// Description:
	
	MAGNETIC_STAGE:
	// When OI (First optical sensor) Interrupt is triggered come here
	// Do whatever is necessary HERE
	PORTC = 0x01; // Just output pretty lights know you made it here
	//Reset the state variable
	STATE = 0;
	goto POLLING_STAGE;

	// #endregion MAGNETIC STAGE -------------------------------------------------------------------------//
	
	/* REFLECTIVE STAGE ----------------------------------------------------------------------------------*/
	// #region

	// Description: 

	REFLECTIVE_STAGE:
	// When OR (Second optical sensor) interrupt is triggered come here
	// Read ADC values, while the value is lower than the previous value overwrite the previous value
	reflect_val = 0; // Temporary overwrite variable
	// See if sensor is still active low
	while((PIND & 0b00000100) == 0b00000100) { 
		ADCSRA |= _BV(ADSC); // Take another ADC reading
		if (ADC_result>reflect_val) {
			reflect_val = ADC_result;
		} // Overwrite previous value if bigger
	}
	
	// FOR TEST 1 - Reflective sensor
	mTimer(500);
	DC_Stop();
	
	// Display on LCD
	LCDClear();
	LCDWriteIntXY(0,1,reflect_val,3);
	mTimer(2000);

	PORTC = 0x04; // Just output pretty lights know you made it here
	//Reset the state variable
	STATE = 0;
	goto POLLING_STAGE;

	// #endregion REFLECTIVE STAGE -----------------------------------------------------------------------//

	/* BUCKET STAGE --------------------------------------------------------------------------------------*/
	// #region

	// Description: 
  
	BUCKET_STAGE:
	bucket_psn = 0;
	bucket_val = 0;
	bucket_move = 0;
	// When EX (End optical sensor) Sensor is triggered come here

	// Pull value from linked list tail
	//bucket_val = linkedlist.->material;

	// Determine which type of material
	if(Al_low <= bucket_val && bucket_val <= Al_high) {
		bucket_psn=0;
		Alum++;
	} else if(St_low <= bucket_val && bucket_val <= St_high) {
		bucket_psn=50;
		Steel++;
	} else if(Wh_low <= bucket_val && bucket_val <= Wh_high) {
		bucket_psn=100;
		White++;
	} else if(Bl_low <= bucket_val && bucket_val <= Bl_high) {
		bucket_psn=150;
		Black++;
	}

	// Dequeue here???
	// Move bucket to right position 
	// Can add direction later, Nigel had a good idea for it to keep track of directionality
	// Only really matters when its the same distance either way
	// Table is stopped either way so does it really matter?
	// CW/CCW might be backwards
	if(CurPosition%200 != bucket_psn) { // if bucket is not at correct stage
		DC_Stop();
		// 200 steps per revolution -> 1.8 degrees per rev
		
		bucket_move = bucket_psn - (CurPosition%200));
		if(bucket_move == -50 || bucket_move == 150) {
			stepccw(50);
		} else if(bucket_move == 50 || bucket_move == -150){
			stepcw(50);
		} else if(abs(bucket_move) == 100){
			stepccw(100);
		}
	}

	DC_Start();
	PORTC = 0x08;
	//Reset the state variable
	STATE = 0;
	goto POLLING_STAGE;

	// #endregion BUCKET STAGE ---------------------------------------------------------------------------//

	/* PAUSE STAGE ---------------------------------------------------------------------------------------*/
	// #region

	// Description: Pauses the DC motor when the pause button (INT4, PE4) is pressed until the pause button 
	//              is pressed again.

	PAUSE_STAGE:
	
	LCDClear();
	LCDWriteString("PAUSE"); // Output "PAUSE" to LCD
	DC_Stop(); // Stop the DC Motor
	while(STATE == 4); // Wait until pause button is pressed again
	
	DC_Start(); // Start the DC Motor
	
	STATE = 0;
	LCDClear();
	LCDWriteString("ACTIVE"); // Output "ACTIVE" to LCD for Test 2 - Pause functionality
	goto POLLING_STAGE;

	// #endregion PAUSE STAGE ----------------------------------------------------------------------------//

	/* END STAGE -----------------------------------------------------------------------------------------*/
	// #region

	// Description:

	END_STAGE:

	// The closing STATE ... how would you get here?
	PORTC = 0xF0;	// Indicates this state is active
	// Stop everything here...'MAKE SAFE'
	// cli();
	return(0);

	// #endregion END STAGE ------------------------------------------------------------------------------//

} // end main()

/*------------------------------------------------------------------------------------------------------*/
/* STEPPER MOTOR SUBROUTINES ---------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

//Homing function
void step_home(void) {

	PolePosition = 0; // set the zero
	
	while((PINA & 0b10000000) != 0b00000000) {
		stepcw(1);
	}

	PolePosition = 0;
	CurPosition = 0;

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

// #endregion

/*------------------------------------------------------------------------------------------------------*/
/* DC MOTOR SUBROUTINES --------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

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

// #endregion

/*------------------------------------------------------------------------------------------------------*/
/* TIMERS ----------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region

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

// #endregion

/*------------------------------------------------------------------------------------------------------*/
/* LINKED QUEUE SUBROUTINES ----------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

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

// #endregion

/*------------------------------------------------------------------------------------------------------*/
/* INTERRUPT SERVICE ROUTINES --------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

/* PD0 = OI Sensor (Active Lo) */
//ISR(INT0_vect){
//	STATE = 1;
//} // Ferro optical sensor

/*  PD1 = HE Sensor (Active Lo) or PORTA.7
	Most likely just using port A.7 for this
	Hall effect ISR */

/* PD2 = OR Sensor (Active Hi) */
ISR(INT2_vect){
	STATE = 2;
	// Want to go to do ISR 
} // Reflective optical sensor

/* PD3 = EX Sensor (Active Lo) */
ISR(INT3_vect){
	STATE = 3;
} // End optical sensor

// When the button is pressed, set Escape GV to 1
ISR(INT4_vect) {
	
	if(STATE == 4) {
		STATE = 0;
	}
	else {
		STATE = 4;
	}
} // Break and end interrupt

ISR(ADC_vect) {

	ADC_result = ADCH; // Store the ADC reading in the global variable
	ADC_result_flag = 1; // Indicate that there is a new ADC result to change PWM frequency and to be displayed on LEDs
} // ADC end

/* PE5 = RampDown (Active Lo) */
//ISR(INT5_vect){
//	STATE = 5;
//} 

// If an unexpected interrupt occurs (interrupt is enabled and no handler is installed,
// which usually indicates a bug), then the default action is to reset the device by jumping
// to the reset vector. You can override this by supplying a function named BADISR_vect which
// should be defined with ISR() as such. (The name BADISR_vect is actually an alias for __vector_default.
// The latter must be used inside assembly code in case <avr/interrupt.h> is not included.
ISR(BADISR_vect)
{
	// Write BADISR to LCD
	LCDClear();
	LCDWriteString("BADISR");
	mTimer(2000);
	// user code here
}

// #endregion

