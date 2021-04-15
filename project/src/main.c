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

	// EIMSK |= (_BV(INT0)); // enable INT0
	EIMSK |= (_BV(INT2)); // enable INT2
	EIMSK |= (_BV(INT3)); // enable INT3
	EIMSK |= (_BV(INT4)); // enable INT4

	// EICRA |= _BV(ISC01); // Falling edge interrupt - Active Lo
	EICRA |= _BV(ISC21) | _BV(ISC20); // Rising edge interrupt - Active Hi
	EICRA |= _BV(ISC31); // INT3 Falling edge - Active Lo
	EICRB |= _BV(ISC41); // INT4 Falling edge interrupt with Active Lo - wait until button is released

	// A-D Conversion (ADC) (Reflective Sensor)
	// Configure ADC -> by default, the ADC input (analog input) is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN); // enable ADC
	ADCSRA |= _BV(ADIE); // enable interrupt of ADC
	ADCSRA |= _BV(ADPS0) | _BV(ADPS2); // ADC Prescaler
	ADMUX |= _BV(REFS0); // Analog supply voltage (AVCC) with external capacitor at AREF pin
	ADMUX |= _BV(MUX0);  // Use PF1 (ADC1) as the input channel

	// Variable declarations
	Alum = 0;
	Steel = 0;
	White = 0;
	Black = 0;

	// Linked Queue
	lq_setup(&bucket_h, &reflect_t); // Set all pointers to NULL
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

	// TEST BLOCK TO VALIDATE STEPPER MOTOR - LCD can't display negatives, hence abs(Cur)
	// ATHOME & ATLAB
	/*
	LCDClear();
	LCDWriteStringXY(0,0,"Pole");
	LCDWriteIntXY(5,0,PolePosition,4);
	LCDWriteStringXY(0,1,"Curr");
	LCDWriteIntXY(5,1,abs(CurPosition),4);
	stepccw(45); // TESTING CODE - to be deleted
	LCDWriteIntXY(5,0,PolePosition,4);
	LCDWriteIntXY(5,1,abs(CurPosition),4);
	mTimer(2000);
	stepccw(69); // TESTING CODE - to be deleted
	LCDWriteIntXY(5,0,PolePosition,4);
	LCDWriteIntXY(5,1,abs(CurPosition),4);
	mTimer(2000);
	stepcw(34); // TESTING CODE - to be deleted
	LCDWriteIntXY(5,0,PolePosition,4);
	LCDWriteIntXY(5,1,abs(CurPosition),4);
	mTimer(2000);
	stepccw(27); // TESTING CODE - to be deleted
	LCDWriteIntXY(5,0,PolePosition,4);
	LCDWriteIntXY(5,1,abs(CurPosition),4);
	mTimer(2000);
	LCDClear(); // TESTING CODE - to be deleted
	LCDWriteString("Stepping"); // TESTING CODE - to be deleted
	*/
	
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

	STATE = 0; //Reset the state variable
	goto POLLING_STAGE;

	// #endregion MAGNETIC STAGE -------------------------------------------------------------------------//
	
	/* REFLECTIVE STAGE ----------------------------------------------------------------------------------*/
	// #region

	// Description: 

	REFLECTIVE_STAGE:
	//LCDClear(); // TESTING CODE _ ATHOME & ATLAB
	//LCDWriteString("Reflective"); // TESTING CODE _ ATHOME & ATLAB
	STATE = 0; //Reset the state variable
	goto POLLING_STAGE;

	// #endregion REFLECTIVE STAGE -----------------------------------------------------------------------//

	/* BUCKET STAGE --------------------------------------------------------------------------------------*/
	// #region

	// Description: 
  
	BUCKET_STAGE:
	bucket_psn = 0;
	bucket_val = 0;
	bucket_move = 0;
	//bucket_move = lq_size(&bucket_h, &reflect_t); // TESTING CODE _ ATHOME & ATLAB

	// if statements trying to handle int3 triggering with no link in queue
	// if (lq_size(&bucket_h, &reflect_t) != 0) { // using size instead
	// if (bucket_h) {

		// START OF AT LAB

		// // Pull value from linked list head
		// bucket_val = bucket_h->reflect_val; // Store reflect_val in link element

		// // LCDClear(); // TESTING CODE _ ATHOME
		// // LCDWriteStringXY(12,0, "BV:") // TESTING CODE _ ATHOME
		// // LCDWriteStringXY(0, 1, "BP:     CP:") // TESTING CODE _ ATHOME
		// // LCDWriteIntXY(12,0,bucket_val,4); // TESTING CODE _ ATHOME
		// // mTimer(1000); // TESTING CODE _ ATHOME
		// // LCDWriteIntXY(6,1,bucket_move,4); // TESTING CODE _ ATHOME
		// //mTimer(4000); // TESTING CODE _ ATHOME

		// // Dequeue link after the reading have been extracted for the sorting algorithm
		// dequeueLink(&bucket_h, &reflect_t); // Dequeue the link pointed to by the head (bucket_h)

		// // Determine which type of material
		// if(bucket_val==1) {
		// 	bucket_psn=50;
		// 	Alum++;
		// 	// LCDWriteStringXY(0,0,"ALUMINUM"); // TESTING CODE _ ATHOME
		// } else if(bucket_val==2) {
		// 	bucket_psn=150;
		// 	Steel++;
		// 	// LCDWriteStringXY(0,0,"STEEL"); // TESTING CODE _ ATHOME
		// } else if(bucket_val==3) {
		// 	bucket_psn=100;
		// 	White++;
		// 	// LCDWriteStringXY(0,0,"WHITE"); // TESTING CODE _ ATHOME
		// } else if(bucket_val==4) {
		// 	bucket_psn=0;
		// 	Black++;
		// 	// LCDWriteStringXY(0,0,"BLACK"); // TESTING CODE _ ATHOME
		// }
		// // mTimer(2000); // TESTING CODE _ ATHOME

		// if(CurPosition%200 != bucket_psn) { // if bucket is not at correct stage
		// 	// DC_Stop(); - moved to beginning of ISR3 for now
		// 	// 200 steps per revolution -> 1.8 degrees per rev
		// 	bucket_move = bucket_psn - (CurPosition%200);
		// 	if(bucket_move == -50 || bucket_move == 150) {
		// 		stepccw(50);
		// 	} else if(bucket_move == 50 || bucket_move == -150){
		// 		stepcw(50);
		// 	} else if(abs(bucket_move) == 100){
		// 		stepcw(100);
		// 	}
		// }

		// Bucket Stage - TESTING CODE _ ATHOME
		// #region

		// Determine which type of material
		if(bucket_val==1) {
			LCDWriteStringXY(0,0,"ALUMINUM"); // TESTING CODE _ ATHOME
			bucket_psn=1536;
			Alum++;
		} else if(bucket_val==2) {
			LCDWriteStringXY(0,0,"STEEL"); // TESTING CODE _ ATHOME
			bucket_psn=512;
			Steel++;
		} else if(bucket_val==3) {
			LCDWriteStringXY(0,0,"WHITE"); // TESTING CODE _ ATHOME
			bucket_psn=1024;
			White++;
		} else if(bucket_val==4) {
			LCDWriteStringXY(0,0,"BLACK"); // TESTING CODE _ ATHOME
			bucket_psn=0;
			Black++;
		}
		// mTimer(2000); // TESTING CODE _ ATHOME

		LCDWriteIntXY(3, 1, bucket_psn, 4);  // TESTING CODE _ ATHOME
		
		// TESTING CODE _ ATHOME
		if (CurPosition < 0) {
			LCDWriteStringXY(11, 1, "-");
		} else {
			LCDWriteStringXY(11, 1, "+");
		}
		LCDWriteIntXY(12, 1, abs(CurPosition), 4);
		// end TESTING CODE _ ATHOME

		// TESTING CODE _ ATHOME
		if(CurPosition%2048 != bucket_psn) { // if bucket is not at correct stage
			// DC_Stop(); - moved to beginning of ISR3 for now
			// 200 steps per revolution -> 1.8 degrees per rev
			bucket_move = bucket_psn - (CurPosition%2048);
			if(bucket_move == -512 || bucket_move == 1536) {
				stepccw(512);
			} else if(bucket_move == 512 || bucket_move == -1536){
				stepcw(512);
			} else if(abs(bucket_move) == 1024){
				stepcw(1024);
			}
		} // CW/CCW might be backwards

		// TESTING CODE _ ATHOME
		if (CurPosition < 0) {
			LCDWriteStringXY(11, 1, "-");
		} else {
			LCDWriteStringXY(11, 1, "+");
		}
		LCDWriteIntXY(12, 1, abs(CurPosition), 4);
		// end TESTING CODE _ ATHOME

		// end Bucket Stage - TESTING CODE _ ATHOME
		// #endregion

		// Can add direction later, Nigel had a good idea for it to keep track of directionality
		// Only really matters when its the same distance either way
		// Table is stopped either way so does it really matter?

	// }
	
	STATE = 0; //Reset the state variable
	DC_Start(); // Start the DC motor
	goto POLLING_STAGE;

	// #endregion BUCKET STAGE ---------------------------------------------------------------------------//

	/* PAUSE STAGE ---------------------------------------------------------------------------------------*/
	// #region

	// Description: Pauses the DC motor when the pause button (INT4, PE4) is pressed until the pause button 
	//              is pressed again.

	PAUSE_STAGE:
 
	LCDWriteStringXY(0, 0, "PAUSED"); // Output "PAUSE" to LCD
	DC_Stop(); // Stop the DC Motor
	LCDClear();
	LCDWriteStringXY(0,0,"Al");
	LCDWriteIntXY(3,0,Alum,2);
	LCDWriteStringXY(6,0,"St");
	LCDWriteIntXY(9,0,Steel,2);
	LCDWriteStringXY(0,1,"Wh");
	LCDWriteIntXY(3,1,White,2);
	LCDWriteStringXY(6,1,"Bl");
	LCDWriteIntXY(9,1,Black,2);
	LCDWriteStringXY(12,0,"Belt");
	LCDWriteIntXY(12,1,bucket_move,4);
	bucket_move = lq_size(&bucket_h, &reflect_t); // TESTING CODE _ ATHOME & ATLAB
	while(STATE == 4); // Wait until pause button is pressed again
	
	DC_Start(); // Start the DC Motor
	
	STATE = 0;
	LCDClear();
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

	PolePosition = 1; // set to 1 for either cw or ccw home
	
	LCDClear();
	LCDWriteString("HOMING..");

	while((PINA & 0b10000000) != 0b00000000) {
		stepcw(1);
	}

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
			PORTA = 0b00001000 ; // TESTING CODE _ ATHOME
			// PORTA = 0b00110110; // Dual Phase - 1 & 2
			break;
			case 2:
			PORTA = 0b00000100; // TESTING CODE _ ATHOME
			// PORTA = 0b00101110; // Dual Phase - 2 & 3
			break;
			case 3:
			PORTA = 0b00000010; // TESTING CODE _ ATHOME
			// PORTA = 0b00101101; // Dual Phase - 3 & 4
			break;
			case 4:
			PORTA = 0b00000001; // TESTING CODE _ ATHOME
			// PORTA = 0b00110101; // Dual Phase - 4 & 1
			break;
			default:
			PORTA = 0;
			break;
		}
		
		if (j<step-1) {
			PolePosition++;
		}
		
		//if(step == 1){
		// mTimer(5); // TESTING CODE _ ATHOME
		mTimer(20); // TESTING CODE _ ATLAB
		//} else if(step == 50){
		//	mTimer(fifty[j]);
		//} else if(step == 100){
		//	mTimer(onehundred[j]);
		//} // Stepper acceleration and deceleration 
		CurPosition++;
	} // for
} // stepcw

void stepccw (int step) {
	PolePosition--; // move the current position back one
	
	for (int j=0; j<step; j++) {

		if (PolePosition==0) {
			PolePosition=4;
		}

		switch (PolePosition) {
			case 1:
			PORTA = 0b00001000 ; // TESTING CODE _ ATHOME
			// PORTA = 0b00110110; // Dual Phase - 1 & 2
			break;
			case 2:
			PORTA = 0b00000100; // TESTING CODE _ ATHOME
			// PORTA = 0b00101110; // Dual Phase - 2 & 3
			break;
			case 3:
			PORTA = 0b00000010; // TESTING CODE _ ATHOME
			// PORTA = 0b00101101; // Dual Phase - 3 & 4
			break;
			case 4:
			PORTA = 0b00000001; // TESTING CODE _ ATHOME
			// PORTA = 0b00110101; // Dual Phase - 4 & 1
			break;
			default:
			PORTA = 0;
			break;
		}
		
		if (j<step-1) {
			PolePosition--;
		}

		//if(step == 1){
		// mTimer(5); // TESTING CODE _ ATHOME
		mTimer(20); // TESTING CODE _ ATLAB
		//} else if(step == 50){
		//	mTimer(fifty[j]);
		//} else if(step == 100){
		//	mTimer(onehundred[j]);
		//} // Stepper acceleration and deceleration 
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
	PORTB = 0x0F; // Brake high - try not braking at all
	//mTimer(10); // wait 10 ms - was mainly for direction change
	//PORTB = 0x00; // Motor off
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
void lq_setup(link **bucket_h, link **reflect_t) {
	*bucket_h = NULL;
	*reflect_t = NULL;
	return;
}

// INITIALIZE LINK
// Description: This subroutine initializes a new link with allocated memory to prepare the link for
//							enqueuing.
void initLink(link **newLink){
	*newLink = malloc(sizeof(link)); // Allocate memory
	(*newLink)->next = NULL; // set next link as NULL
	// Set values to negative (to determine if the link has received a specific reading yet)

	// (*newLink)->e.reflect_val = -1; // might still need

	return;
} //initLink

// ENQUEUE
// Description: This subroutine enqueues a new link, which the tail (ferro_t) will always point to. 
//              This occurs when there is a ferromagnetic reading.
void enqueueLink(link **bucket_h, link **reflect_t, link **newLink){

	if (*reflect_t != NULL){ // If linked queue is not empty
		(*reflect_t)->next = *newLink; // append newLink
		*reflect_t = *newLink; // move reflect_t pointer to new link
	}
	else{ // If linked queue is empty, move all pointers to newLink
		*bucket_h = *newLink;
		*reflect_t = *newLink;
	}
	return;
}/*enqueueLink*/

// DEQUEUE
// Description: This subroutine dequeues the link pointed to by the head (bucket_h). This occurs 
//              during the bucket stage when the piece is sorted.
void dequeueLink(link **bucket_h, link **reflect_t){

	link *temp;

	if (*bucket_h != NULL){ // Ensure it is not an empty queue
		temp = *bucket_h; // Point temp to same link as head pointer (bucket_h)
		*bucket_h = (*bucket_h)->next; // Shift head pointer
		free(temp); // Free memory of dequeued link
	}

	// If it becomes an empty queue, set other pointers to NULL
	if (*bucket_h == NULL) {
		*reflect_t = NULL;
	}
	
	return;
}/*dequeue*/

// LINKED QUEUE SIZE
// Description: This subroutine measures the number of links from one pointer to another. This will 
//              mostly be used for debugging purposes.

int lq_size(link **head, link **tail) {

	link 	*temp;			/* will store the link while traversing the queue */
	int 	numElements;

	numElements = 0;

	temp = *head;			/* point to the first item in the list */

	while(temp != NULL){
		numElements++;
		temp = temp->next;
	}/*while*/
	
	return(numElements);
}/*lq_size*/

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
	mTimer(100); // TESTING CODE - ATHOME
	if((PIND & 0b00000100) == 0b00000100){
		STATE = 2; // Enter state 2 after finished readings
		reflect_val = 0x400; // Start high - sensor is active low - 1024 is 2^10
		ADCSRA |= _BV(ADSC); // Take another ADC reading
	}
} // Reflective optical sensor - PD2 = OR Sensor (Active Hi) - verified

// Optical Sensor for Bucket Stage (EX)
ISR(INT3_vect){
	mTimer(100); // TESTING CODE - ATHOME
	// MASK the bit to see if it's lo
	DC_Stop(); // Stop DC motor as soon as interrupt is triggered
	STATE = 3; // will goto BUCKET_STAGE
} // PD3 = EX Sensor (Active Lo)

// Pause button
ISR(INT4_vect) {
	mTimer(20); // TESTING CODE _ ATHOME
	
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

		mTimer(100); // TESTING CODE - ATHOME

		// Reflective Stage Linked Queue
		// Enqueue new link each time a reflective reading is taken
		initLink(&newLink);

		LCDClear(); // TESTING CODE - ATHOME & ATLAB
		LCDWriteIntXY(0,0,reflect_val,4); // TESTING CODE - ATHOME & ATLAB
		mTimer(3000); // TESTING CODE - ATHOME

		if(Al_low <= reflect_val && reflect_val <= Al_high) {
			newLink->reflect_val = 1;
			//LCDWriteStringXY(0,0,"ALUMINUM"); // TESTING CODE _ ATHOME & ATLAB
		} else if(St_low <= reflect_val && reflect_val <= St_high) {
			newLink->reflect_val = 2;
			//LCDWriteStringXY(0,0,"STEEL"); // TESTING CODE _ ATHOME & ATLAB
		} else if(Wh_low <= reflect_val && reflect_val <= Wh_high) {
			newLink->reflect_val = 3;
			//LCDWriteStringXY(0,0,"WHITE"); // TESTING CODE _ ATHOME & ATLAB
		} else if(Bl_low <= reflect_val && reflect_val <= Bl_high) {
			newLink->reflect_val = 4;
			//LCDWriteStringXY(0,0,"BLACK"); // TESTING CODE _ ATHOME & ATLAB
		}

		enqueueLink(&bucket_h, &reflect_t, &newLink);

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

