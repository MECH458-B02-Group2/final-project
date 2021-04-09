//------------------------------------------------------------------------------------------------------//
// GLOBAL VARIABLES ------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

// State Machine
volatile int STATE;

// Stepper Motor
volatile int PolePosition;
volatile int CurPosition;
//acceleration curves for stepper
int onehundred [99] = {22001,19521,17368,15510,13915,12553,11399,10427,9614,8941,8387,7937,7573,7283,7054,6874,6733,
6623,6536,6466,6407,6354,6303,6251,6195,6135,6068,5993,5911,5821,5723,5619,5509,5395,5277,5157,5037,5000,5000,5000,
5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5000,5037,
5157,5277,5395,5509,5619,5723,5821,5911,5993,6068,6135,6195,6251,6303,6354,6407,6466,6536,6623,6733,6874,7054,7283,
7573,7937,8387,8941,9614,10427,11399,12553,13915,15510,17368,19521,22001};

int fifty [49] = {20,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,6,6,6,
6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
6,8,10,13,17,20};

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