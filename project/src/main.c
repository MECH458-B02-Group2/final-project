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
	TCCR0B |= _BV(CS01) | _BV(CS00); // Set the timer prescalar to 1/64 --> (8MHz/(N*256) (Recommended from Simon -> previous was 1/8)
	
	// LEDs and LCD module (PORTC)
	InitLCD(LS_BLINK|LS_ULINE);
	LCDClear();
	LCDWriteString("ACTIVE");

	DDRC = 0xFF;		// LED Output
	PORTC = 0x00; // Set display off to start

	// External Interrupts
	cli();		// Disables all interrupts

	// EIMSK |= (_BV(INT0)); // enable INT2
	EIMSK |= (_BV(INT2)); // enable INT2
	EIMSK |= (_BV(INT3)); // enable INT3
	EIMSK |= (_BV(INT4)); // enable INT4

	// EICRA |= _BV(ISC01); // Falling edge interrupt - Active Lo
	EICRA |= _BV(ISC21) | _BV(ISC20); // Rising edge interrupt - Active Hi
	EICRA |= _BV(ISC31); // INT3 Falling edge - Active Lo
	EICRB |= _BV(ISC41) | _BV(ISC40); // INT4 Rising edge interrupt with Active Lo - wait until button is released

	// A-D Conversion (ADC) (Reflective Sensor)
	// Configure ADC -> by default, the ADC input (analog input) is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	ADMUX |= _BV(REFS0); // Analog supply voltage (AVCC) with external capacitor at AREF pin
	ADMUX |= _BV(MUX0);  // Use PF1 (ADC1) as the input channel

	// Variable declarations
	Alum = 0;
	Steel = 0;
	White = 0;
	Black = 0;

//**********************LINKED LIST IMPLEMENTATION***************************************************************************************************************
	// Linked Queue
	link *bucket_h; // Pointer to the last link that has received a ferromagnetic reading - also the linked queue head
	link *reflect; // Pointer to the last link that has received a reflective reading
	link *ferro_t; // Pointer to the last link that has been sorted - also the linked queue tail
	lq_setup(&bucket_h, &reflect, &ferro_t); // Set all pointers to NULL
	link *newLink; // temp link which will be allocated memory with initLink() before enqueueLink()
	newLink = NULL; // set newLink to NULL


	// I/O Ports (Check necessity of these)
	DDRD = 0b11110000;	// Going to set up INT2 & INT3 on PORTD
	DDRA = 0b00111111; // A7 as input for HE sensor, A0-A5 as output for stepper motor
	PORTA = 0b00000000;
	
	// Stepper Motor
	step_home(); // Working correctly as per TR3
	LCDClear(); // TESTING CODE - to be deleted
	LCDWriteString("ACTIVE"); // TESTING CODE - to be deleted
	
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

	// take reading
//**********************MOVE TO ISR(OPTICAL SENSOR)**************************************************************************************************************
//**********************LINKED LIST IMPLEMENTATION***************************************************************************************************************
	// Magnetic Stage Linked Queue
	// Enqueue new link each time a ferromagnetic reading is taken
	//initLink(&newLink);
	//enqueueLink(&bucket_h, &reflect, &ferro_t, &newLink);

	//ferro_t->e.ferro_val = 1; // = ferro_val; // Store ferro_val in link element
	STATE = 0; //Reset the state variable
	goto POLLING_STAGE;

	// #endregion MAGNETIC STAGE -------------------------------------------------------------------------//
	
	/* REFLECTIVE STAGE ----------------------------------------------------------------------------------*/
	// #region

	// Description: 

	REFLECTIVE_STAGE:
	LCDClear(); // TESTING CODE _ TO BE DELETED - writing on the second line
	LCDWriteIntXY(0,1,reflect_val,4); // TESTING CODE _ TO BE DELETED - writing on the second line
	DC_Stop(); // TESTING CODE _ TO BE DELETED
	mTimer(1000); // TESTING CODE _ TO BE DELETED
	DC_Start(); // TESTING CODE _ TO BE DELETED
	STATE = 0; //Reset the state variable
	goto POLLING_STAGE;

	// #endregion REFLECTIVE STAGE -----------------------------------------------------------------------//

	/* BUCKET STAGE --------------------------------------------------------------------------------------*/
	// #region

	// Description: 
  
	BUCKET_STAGE:
	STATE = 0; //Reset the state variable
	goto POLLING_STAGE;

	// #endregion BUCKET STAGE ---------------------------------------------------------------------------//

	/* PAUSE STAGE ---------------------------------------------------------------------------------------*/
	// #region

	// Description: Pauses the DC motor when the pause button (INT4, PE4) is pressed until the pause button 
	//              is pressed again.

	PAUSE_STAGE:

	LCDWriteStringXY(0, 0, "PAUSED"); // Output "PAUSE" to LCD
	DC_Stop(); // Stop the DC Motor
	while(STATE == 4); // Wait until pause button is pressed again
	
	DC_Start(); // Start the DC Motor
	
	STATE = 0;
	LCDWriteStringXY(0, 0, "ACTIVE"); // Output "ACTIVE" to LCD for Test 2 - Pause functionality
	goto POLLING_STAGE;

	// #endregion PAUSE STAGE ----------------------------------------------------------------------------//

	/* END STAGE -----------------------------------------------------------------------------------------*/
	// #region

	// Description:

	END_STAGE:

	// The closing STATE ... how would you get here?
	//PORTC = 0xF0;	// Indicates this state is active
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
	
	LCDClear();
	LCDWriteString("HOMING..");

	while((PINA & 0b10000000) != 0b00000000) {
		stepcw(1);
	}

	PolePosition = 0;
	CurPosition = 0;

} // Homing Function - validated

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
		
		if(step == 1){
		mTimer(20); // Step Delay
		} else if(step == 50){
			mTimer(fifty[j]);
		} else if(step == 100){
			mTimer(onehundred[j]);
		} // Stepper acceleration and deceleration 
		CurPosition++;
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

		if(step == 1){
		mTimer(20); // Step Delay
		} else if(step == 50){
			mTimer(fifty[j]);
		} else if(step == 100){
			mTimer(onehundred[j]);
		} // Stepper acceleration and deceleration 
		CurPosition--;
	} // for
} // stepccw

// #endregion

/*------------------------------------------------------------------------------------------------------*/
/* DC MOTOR SUBROUTINES --------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

void DC_Start(void) {
	PORTB = 0x07; // Motor running forward
	return;
} // Motor start

void DC_Stop(void) {
	PORTB = 0x0F; // Brake
	mTimer(10); // wait 10 ms
	PORTB = 0x00; // Motor off
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

// LINKED QUEUE SETUP
// Description: This subroutine sets all linked queue pointers to NULL.
void lq_setup(link **bucket_h, link **reflect, link **ferro_t) {
	*bucket_h = NULL;
	*reflect = NULL;
	*ferro_t = NULL;
	return;
}

// INITIALIZE LINK
// Description: This subroutine initializes a new link with allocated memory to prepare the link for
//							enqueuing.
void initLink(link **newLink){
	*newLink = malloc(sizeof(link)); // Allocate memory
	(*newLink)->next = NULL; // set next link as NULL
	// Set values to negative (to determine if the link has received a specific reading yet)
	(*newLink)->e.reflect_val = -1;
	(*newLink)->e.ferro_val = -1;
	return;
} //initLink

// ENQUEUE
// Description: This subroutine enqueues a new link, which the tail (ferro_t) will always point to. 
//              This occurs when there is a ferromagnetic reading.
void enqueueLink(link **bucket_h, link **reflect, link **ferro_t, link **newLink){

	if (*ferro_t != NULL){ // If linked queue is not empty
		(*ferro_t)->next = *newLink; // append newLink
		*ferro_t = *newLink; // move ferro_t pointer to new link
	}
	else{ // If linked queue is empty, move all pointers to newLink
		*bucket_h = *newLink;
		*reflect = *newLink;
		*ferro_t = *newLink;
	}
	return;
}/*enqueueLink*/


// NEXT LINK
// Description: This subroutine points the reflect pointer to the next link in the queue.
//              This occurs when there is a reflective reading
void nextLink(link **reflect) {
	*reflect = (*reflect)->next; // move reflect pointer to next link
	return;
}

// DEQUEUE
// Description: This subroutine dequeues the link pointed to by the head (bucket_h). This occurs 
//              during the bucket stage when the piece is sorted.
void dequeueLink(link **bucket_h, link **reflect, link **ferro_t){

	link *temp;

	if (*bucket_h != NULL){ // Ensure it is not an empty queue
		temp = *bucket_h; // Point temp to same link as head pointer (bucket_h)
		if (*reflect == *bucket_h) { // Shift reflect pointer if it points to same link as head pointer
			*reflect = (*reflect)->next;
		}
		*bucket_h = (*bucket_h)->next; // Shift head pointer
		free(temp); // Free memory of dequeued link
	}

	// If it becomes an empty queue, set other pointers to NULL
	if (*bucket_h == NULL) {
		*reflect = NULL;
		*ferro_t = NULL;
	}
	
	return;
}/*dequeue*/

// #endregion

/*------------------------------------------------------------------------------------------------------*/
/* INTERRUPT SERVICE ROUTINES --------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

// Optical Sensor for Magnetic Stage (OI)
// s
//ISR(INT0_vect){
	// STATE = 1; // will goto MAGNETIC_STAGE
//} // PD0 = OI (INT0) (Active Lo)


ISR(INT2_vect){
	STATE = 2; // Enter state 2 after finished readings
	reflect_val = 0x400; // Start high - sensor is active low - 1024 is 2^10
	ADCSRA |= _BV(ADSC); // Take another ADC reading
} // Reflective optical sensor - PD2 = OR Sensor (Active Hi)

// Optical Sensor for Bucket Stage (EX)
ISR(INT3_vect){
	STATE = 3; // will goto BUCKET_STAGE
	bucket_psn = 0;
	bucket_val = 0;
	bucket_move = 0;

//**********************LINKED LIST IMPLEMENTATION***************************************************************************************************************
	// Pull value from linked list tail
	//bucket_val = reflect->e.reflect_val; // Store reflect_val in link element

	bucket_val = reflect_val; // TESTING CODE _ TO BE DELETED
	// Determine which type of material
	if(Al_low <= bucket_val && bucket_val <= Al_high) {
		bucket_psn=0;
		Alum++;
		LCDClear(); // TESTING CODE _ TO BE DELETED
		LCDWriteString("ALUMINUM"); // TESTING CODE _ TO BE DELETED
	} else if(St_low <= bucket_val && bucket_val <= St_high) {
		bucket_psn=50;
		Steel++;
		LCDClear(); // TESTING CODE _ TO BE DELETED
		LCDWriteString("STEEL"); // TESTING CODE _ TO BE DELETED
	} else if(Wh_low <= bucket_val && bucket_val <= Wh_high) {
		bucket_psn=100;
		White++;
		LCDClear(); // TESTING CODE _ TO BE DELETED
		LCDWriteString("WHITE"); // TESTING CODE _ TO BE DELETED
	} else if(Bl_low <= bucket_val && bucket_val <= Bl_high) {
		bucket_psn=150;
		Black++;
		LCDClear(); // TESTING CODE _ TO BE DELETED
		LCDWriteString("BLACK"); // TESTING CODE _ TO BE DELETED
	}
//**********************LINKED LIST IMPLEMENTATION***************************************************************************************************************
	// Sorting algorithm - (Start of Malaki's code)
	// Bucket Stage Linked Queue
	// Dequeue link after the reading have been extracted for the sorting algorithm
	//dequeueLink(&bucket_h, &reflect, &ferro_t); // Dequeue the link pointed to by the head (bucket_h)
	// End of Malaki's code

	if(CurPosition%200 != bucket_psn) { // if bucket is not at correct stage
		DC_Stop();
		// 200 steps per revolution -> 1.8 degrees per rev
		bucket_move = bucket_psn - (CurPosition%200);
		if(bucket_move == -50 || bucket_move == 150) {
			stepcw(50); // TESTING CODE - swapped directions incase I had them wrong
		} else if(bucket_move == 50 || bucket_move == -150){
			stepccw(50);
		} else if(abs(bucket_move) == 100){
			stepcw(100);
		}
	} // CW/CCW might be backwards
	// Can add direction later, Nigel had a good idea for it to keep track of directionality
	// Only really matters when its the same distance either way
	// Table is stopped either way so does it really matter?
	DC_Start();
} // PD3 = EX Sensor (Active Lo)

// Pause button
ISR(INT4_vect) {
	//Home board debugging purposes
	mTimer(20); // debounce
	
	if(STATE == 4) {
		STATE = 0; // will goto POLLING_STAGE
	}
	else {
		STATE = 4; // will goto PAUSE_STAGE
	}
} // Pause

ISR(ADC_vect) {

	if(ADC<reflect_val){
		reflect_val = ADC;
	} // Find the lowest ADC value
	
	if((PIND & 0b00000100) == 0b00000100) { 
		ADCSRA |= _BV(ADSC); // Take another ADC reading
	} else{
//**********************LINKED LIST IMPLEMENTATION***************************************************************************************************************
		// Reflective Stage Linked Queue
		// Move the reflect pointer to next link if there is already a reading in the current link and if it
		// is not pointing to the same link as the tail (ferro_t) (which would result in reflect pointing to NULL)
		// enqueue
		//if (reflect->e.reflect_val >= 0 && reflect != ferro_t) {
		//nextLink(&reflect); // Move reflect pointer to next link
		//}

		//reflect->e.reflect_val = reflect_val; // Store reflect_val in link element
	} // Continue taking readings and then add to the linked list
} // ADC end

/* PE5 = RampDown (Active Lo) */
//ISR(INT5_vect){
//	STATE = 5;
//} 


ISR(BADISR_vect)
{
	// If an unexpected interrupt occurs (interrupt is enabled and no handler is installed,
	// which usually indicates a bug), then the default action is to reset the device by jumping
	// to the reset vector. You can override this by supplying a function named BADISR_vect which
	// should be defined with ISR() as such. (The name BADISR_vect is actually an alias for __vector_default.
	// The latter must be used inside assembly code in case <avr/interrupt.h> is not included.
	// Write BADISR to LCD
	LCDClear();
	LCDWriteString("BADISR");
	mTimer(2000);
	// user code here
}

// #endregion

