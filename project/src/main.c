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
	EIMSK |= (_BV(INT5)); // enable INT5

	// EICRA |= _BV(ISC01); // Falling edge interrupt - Active Lo
	EICRA |= _BV(ISC21) | _BV(ISC20); // Rising edge interrupt - Active Hi
	EICRA |= _BV(ISC31); // INT3 Falling edge - Active Lo
	EICRA |= _BV(ISC41); // INT4 Falling edge - Active Lo
	EICRA |= _BV(ISC51); // INT5 Falling edge - Active Lo

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
	SORT = 0;
	RAMPDOWN_FLAG = 0;
	int RAMPDOWN_TIMER;

	// Linked Queue
	lq_setup(&bucket_h, &reflect_t); // Set all pointers to NULL
	newLink = NULL; // set newLink to NULL


	// I/O Ports (Check necessity of these)
	DDRD = 0b11110000;	// Going to set up INT2 & INT3 on PORTD
	DDRA = 0b00111111; // A7 as input for HE sensor, A0-A5 as output for stepper motor
	PORTA = 0b00000000; // Initialize stepper motor coils
	DDRE = 0b00000000; // PE4 & PE5 as input for interrupt buttons (pause and ramp-down)
	
	// Stepper Motor
	//accel_curve();
	step_home(); // Working correctly as per TR3
	
	LCDClear(); // TESTING CODE _ ATHOME & ATLAB
	LCDWriteString("ACTIVE"); // TESTING CODE _ ATHOME & ATLAB
	
	// DC Motor
	// Set PORTB (DC motor port) to output (B7 = PWM, B3 = IA, B2 = IB, B1 = EA, B0 = EB)
	DDRB = 0xFF; // Initialize port B for output to motor driver
	PORTB = 0x00; //Initialize all pins to be low
	PWM(); // Initialize PWM
	// Start running the motor
	PORTB = 0b00000111; // Motor running forward
	
		LCDClear(); // TESTING CODE _ ATHOME
		LCDWriteStringXY(0,0,"R FLAG"); // TESTING CODE _ ATHOME
		LCDWriteIntXY(9,0,RAMPDOWN_FLAG,1); // TESTING CODE _ ATHOME
		mTimer(2000);

// #endregion
	
	// Enable all interrupts
	sei();	// Note this sets the Global Enable for all interrupts

	while(1) {

		// SORTING (INT3 Active Lo)
		if(SORT) {
			bucket_psn = 0;
			bucket_val = 0;
			bucket_move = 0;

			// if statements trying to handle int3 triggering with no link in queue
			// if (lq_size(&bucket_h, &reflect_t) != 0) { // using size instead
			// if (bucket_h) {

			// Pull value from linked list head
			bucket_val = bucket_h->reflect_val; // Store reflect_val in link element

			// Dequeue link after the reading have been extracted for the sorting algorithm
			dequeueLink(&bucket_h, &reflect_t); // Dequeue the link pointed to by the head (bucket_h)

			/*
			// Determine which type of material
			if(bucket_val==1) {
				bucket_psn=50;
				Alum++;
				// LCDWriteStringXY(0,0,"ALUMINUM"); // TESTING CODE _ ATHOME
			} else if(bucket_val==2) {
				bucket_psn=150;
				Steel++;
				// LCDWriteStringXY(0,0,"STEEL"); // TESTING CODE _ ATHOME
			} else if(bucket_val==3) {
				bucket_psn=100;
				White++;
				// LCDWriteStringXY(0,0,"WHITE"); // TESTING CODE _ ATHOME
			} else if(bucket_val==4) {
				bucket_psn=0;
				Black++;
				// LCDWriteStringXY(0,0,"BLACK"); // TESTING CODE _ ATHOME
			}

			if(CurPosition%200 != bucket_psn) { // if bucket is not at correct stage
				// 200 steps per revolution -> 1.8 degrees per rev
				bucket_move = bucket_psn - (CurPosition%200);
				if(bucket_move == -50 || bucket_move == 150) {
					stepccw(50);
				} else if(bucket_move == 50 || bucket_move == -150){
					stepcw(50);
				} else if(abs(bucket_move) == 100){
					stepcw(100);
				}
			}

			*/

			// Sorting Stage - TESTING CODE _ ATHOME
			// #region
			
			
			LCDClear(); // TESTING CODE _ ATHOME
			LCDWriteStringXY(12,0, "BV:"); // TESTING CODE _ ATHOME
			LCDWriteIntXY(15, 0, bucket_val, 1); // TESTING CODE _ ATHOME
			LCDWriteStringXY(0, 1, "BP:     CP:"); // TESTING CODE _ ATHOME


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
				// 200 steps per revolution -> 1.8 degrees per rev
				bucket_move = bucket_psn - (CurPosition%2048);
				if(bucket_move == -512 || bucket_move == 1536) {
					stepccw(512);
				} else if(bucket_move == 512 || bucket_move == -1536){
					stepcw(512);
				} else if(abs(bucket_move) == 1024){
					stepcw(1024);
				}
			} // end TESTING CODE _ ATHOME

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

			SORT = 0;
			DC_Start();

		}

		// RAMP DOWN (INT5 Button Active Lo)
		RAMPDOWN_TIMER = 0;
		while((RAMPDOWN_FLAG) && (lq_size(&bucket_h, &reflect_t) == 0)) {
			if(RAMPDOWN_TIMER == 0) {
				LCDClear();
				LCDWriteString("Checking belt...");
			}
			RAMPDOWN_TIMER++;
			mTimer(10); // Each count of RAMPDOWN_TIMER is 10ms
			if(RAMPDOWN_TIMER == 500) {
				LCDClear();
				LCDWriteStringXY(6, 0, "RAMP");
				LCDWriteStringXY(6, 1, "DOWN");
				DC_Stop();
				PORTA = 0b00000000;
				PORTB = 0b00000000;

				return(0);
			}
		}
	
	}


} // end main()

/*------------------------------------------------------------------------------------------------------*/
/* STEPPER MOTOR SUBROUTINES ---------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------*/
// #region 

// Acceleration Curve Generator

/*
void accel_curve(void) {
	int sq = sizeof(quarter);
	int sh = sizeof(half);
	int sa = sizeof(accel);
	int sd = sizeof(decel);

	for (int i=0; i<sq; i++) {
		if(i<sa) {
			quarter[i] = accel[i];
		} else if(i>(sq-sd-1)) {
			quarter[i] = decel[sq-i];
		} else {
			quarter[i] = min_accel;
		}
	} // for quarter

	for (int i=0; i<sh; i++) {
		if(i<sizeof(accel)) {
			half[i] = accel[i];
		} else if(i>(sh-sd-1)) {
			half[i] = decel[sh-i];
		} else {
			half[i] = min_accel;
		}
	} // for half
	
} // Acceleration Curve
*/

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
	/*
		// ATLAB
		if(step == 1){
			mTimer(20);
		} else if(step == 50){
			mTimer(quarter[j]);
		} else if(step == 100){
			mTimer(half[j]);
		} // Stepper acceleration and deceleration 
	*/
		// ATHOME
		if(step == 1){
		mTimer(20); // TESTING CODE _ ATHOME
		} else if(step == 512){
			mTimer(5);
		} else if(step == 1024){
			mTimer(10);
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
	/*
		// ATLAB
		if(step == 1){
			mTimer(20);
		} else if(step == 50){
			mTimer(quarter[j]);
		} else if(step == 100){
			mTimer(half[j]);
		} // Stepper acceleration and deceleration 
	*/
		// ATHOME
		if(step == 1){
		mTimer(20); // TESTING CODE _ ATHOME
		} else if(step == 512){
			mTimer(5);
		} else if(step == 1024){
			mTimer(10);
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

// Optical Sensor for Reflective Stage (OR)
ISR(INT2_vect){
	mTimer(100); // TESTING CODE - ATHOME
	if((PIND & 0b00000100) == 0b00000100){
		reflect_val = 0x400; // Start high - sensor is active low - 1024 is 2^10
		ADCSRA |= _BV(ADSC); // Take another ADC reading
	}
} // Reflective optical sensor - PD2 = OR Sensor (Active Hi) - verified

// Optical Sensor for Bucket Stage (EX)
ISR(INT3_vect){
	mTimer(100); // TESTING CODE - ATHOME
	// MASK the bit to see if it's lo
	DC_Stop(); // Stop DC motor as soon as interrupt is triggered
	SORT = 1; // will goto BUCKET_STAGE
} // PD3 = EX Sensor (Active Lo)

// Pause button
ISR(INT4_vect) {

	mTimer(100); // TESTING CODE _ ATHOME

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
	LCDWriteIntXY(13,1,(lq_size(&bucket_h, &reflect_t)),2);
 
	while((PINE & 0b00010000) == 0b00000000); // Wait until button is released - pause
	mTimer(100); // TESTING CODE _ ATHOME

	while((PINE & 0b00010000) == 0b00010000); // Wait until button is pressed - unpause
	mTimer(100); // TESTING CODE _ ATHOME

	while((PINE & 0b00010000) == 0b00000000); // Wait until button is released - unpause
	mTimer(100); // TESTING CODE _ ATHOME
	
	DC_Start(); // Start the DC Motor

	LCDClear();
	LCDWriteStringXY(0, 0, "ACTIVE"); // Output "ACTIVE" to LCD

} // Pause


/* PE5 = RampDown (Active Lo) */
ISR(INT5_vect){
	mTimer(100); // TESTING CODE _ ATHOME
	RAMPDOWN_FLAG = 1;
} 

ISR(ADC_vect) {

	if(ADC<reflect_val){
		reflect_val = ADC;
	} // Find the lowest ADC value
	
	if((PIND & 0b00000100) == 0b00000100) { 
		ADCSRA |= _BV(ADSC); // Take another ADC reading
	} else{

		mTimer(100); // TESTING CODE - ATHOME (WARN: DEF REMOVE FOR ATLAB)

		// Reflective Stage Linked Queue
		// Enqueue new link each time a reflective reading is taken
		initLink(&newLink);

		LCDClear(); // TESTING CODE - ATHOME & ATLAB
		LCDWriteIntXY(12,0,reflect_val,4); // TESTING CODE - ATHOME & ATLAB

		if(Al_low <= reflect_val && reflect_val <= Al_high) {
			newLink->reflect_val = 1;
			LCDWriteStringXY(0,0,"ALUMINUM"); // TESTING CODE _ ATHOME & ATLAB
		} else if(St_low <= reflect_val && reflect_val <= St_high) {
			newLink->reflect_val = 2;
			LCDWriteStringXY(0,0,"STEEL"); // TESTING CODE _ ATHOME & ATLAB
		} else if(Wh_low <= reflect_val && reflect_val <= Wh_high) {
			newLink->reflect_val = 3;
			LCDWriteStringXY(0,0,"WHITE"); // TESTING CODE _ ATHOME & ATLAB
		} else if(Bl_low <= reflect_val && reflect_val <= Bl_high) {
			newLink->reflect_val = 4;
			LCDWriteStringXY(0,0,"BLACK"); // TESTING CODE _ ATHOME & ATLAB
		}

		enqueueLink(&bucket_h, &reflect_t, &newLink);

	} // Continue taking readings and then add to the linked list
} // ADC end

ISR(BADISR_vect)
{
	LCDClear();
	LCDWriteString("BADISR");
	mTimer(2000);
}

// #endregion

