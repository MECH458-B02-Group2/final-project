//------------------------------------------------------------------------------------------------------//
// GLOBAL VARIABLES ------------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

// State Machine
volatile int STATE;

// Stepper Motor
volatile int PolePosition;
volatile int CurPosition;
int const delay = 20;

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
// TYPE DECLARATIONS -----------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

// Linked Queue
typedef struct {
	int ferro_val; // Need to distinguish between aluminum and steel
	int reflect_val; // Need to distinguish between white and black delrin
} element;

typedef struct link{
	element e;
	struct link *next;
} link;

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
// LINKED QUEUE SUBROUTINES ----------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//
void lq_setup(link **bucket_h, link **reflect, link **ferro_t);
void initLink(link **newLink);
void enqueueLink(link **bucket_h, link **reflect, link **ferro_t, link **newLink);
void nextLink(link **reflect);
void dequeueLink(link **bucket_h, link **reflect, link **ferro_t);