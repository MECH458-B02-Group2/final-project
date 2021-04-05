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
int const Speed = 0x50;

// Reflective Sensor Values
/*  Aluminum - < 255
	Steel    - 400 - 700
	White    - 870 - 935
	Black    - 936 - 980 */
int const ADC_min = 5;
int const Al_low = 0;
int const Al_high = 255;
int const St_low = 400;
int const St_high = 700;
int const Wh_low = 870;
int const Wh_high = 935;
int const Bl_low = 936;
int const Bl_high = 980;

// Other
volatile unsigned char ADC_result;
volatile unsigned int ADC_result_flag;
volatile unsigned int Escape; // Probably WONT NEED, was for the end routine interrupt for lab 4b

//------------------------------------------------------------------------------------------------------//
// TYPE DECLARATIONS -----------------------------------------------------------------------------------//
//------------------------------------------------------------------------------------------------------//

// Linked Queue
typedef struct {
  int ferro; // Need to distinguish between aluminum and steel
	int reflect; // Need to distinguish between white and black delrin
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
void enqueue_link(link **bucket_h, link **reflect, link **ferro_t, link **newLink);
void next_link(link **reflect);
void dequeue_link(link **bucket_h, link **reflect, link **ferro_t);