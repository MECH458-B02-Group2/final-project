//------------------------------------------------------------------------------------------------------//
// GLOBAL VARIABLES ------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

// State Machine
volatile int SORT;
volatile int RAMPDOWN_FLAG;

// Stepper Motor
volatile int PolePosition;
volatile int CurPosition;

//acceleration curves for stepper
//int accel[15] = {20,20,19,18,17,16,15,14,13,12,11,10,9,8,7};
//int decel[5]  = {8,10,13,17,20}; 
int half[100] = {20,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,8,10,13,17,20};
int quarter [50] = {20,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,8,10,13,17,20};

// DC Motor
int const Speed = 0x60;

// Reflective Sensor Values
volatile unsigned int Alum;
volatile unsigned int Steel;
volatile unsigned int White;
volatile unsigned int Black;
int const Al_low = 0;
int const Al_high = 327;	// 255 was OG
int const St_low = 328;		// 400 was OG
int const St_high = 785;	// 700 was OG
int const Wh_low = 786;		// 870 was OG
int const Wh_high = 935;	// No changes
int const Bl_low = 936;		// No changes
int const Bl_high = 1023;	// 980 was OG

// Sorting values
volatile unsigned int reflect_val;
volatile unsigned int bucket_psn;
volatile unsigned int bucket_val;
volatile unsigned int bucket_move;

//------------------------------------------------------------------------------------------------------//
// STEPPER MOTOR SUBROUTINES ---------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

void step_home(void);
void stepcw(int step);
void stepccw(int step);

//------------------------------------------------------------------------------------------------------//
// DC MOTOR SUBROUTINES --------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

void DC_Start(void);
void DC_Stop(void);

//------------------------------------------------------------------------------------------------------//
// TIMERS ----------------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

void PWM(void);
void mTimer(int count);

//------------------------------------------------------------------------------------------------------//
// LINKED QUEUE ----------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

typedef struct link{
	int reflect_val;
	struct link *next;
} link;

link *bucket_h; // Pointer to the last link that has received a ferromagnetic reading - also the linked queue head
link *reflect_t; // Pointer to the last link that has been sorted - also the linked queue tail
link *newLink; // temp link which will be allocated memory with initLink() before enqueueLink()

//------------------------------------------------------------------------------------------------------//
// LINKED QUEUE SUBROUTINES ----------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
void lq_setup(link **bucket_h, link **reflect_t);
void initLink(link **newLink);
void enqueueLink(link **bucket_h, link **reflect_t, link **newLink);
void dequeueLink(link **bucket_h, link **reflect_t);
int lq_size(link **first, link **last);