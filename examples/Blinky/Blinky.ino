#include <YA_FSM.h>

const uint8_t button = 2;
const uint8_t led = 13;

// Create new Finite State Machine
YA_FSM myFSM;

// State Alias 
enum State {BLINK1, BLINK2, BLINK3};

// Helper for print labels instead integer when state change
const char * const stateName[] PROGMEM = { "Blink1", "Blink2", "Blink3"};

#define BLINK1_TIME  	1000		
#define BLINK2_TIME 	300 
#define BLINK3_TIME 	100

// A simple bool var just to test transition's trigger
bool dummyBoolVar = false;
bool var = false;

void setup() {
	pinMode(button, INPUT_PULLUP);	
	pinMode(led, OUTPUT);
	Serial.begin(115200);
	Serial.println(F("Starting State Machine...\n"));
	setupStateMachine();	

	// Initial state
	Serial.print(F("Active state: "));
	Serial.println(myFSM.ActiveStateName());
}


void loop() {
	// Update State Machine	(true is state changed)
	if(myFSM.Update()){
		Serial.print(F("Active state: "));
		Serial.println(myFSM.ActiveStateName());
	}	
	
	// When BLINK3 state is active, wait some time and the set true dummyBoolVar
	// to test transition from BLINK3 to BLINK1 with a bool variables instead a callback function
	if(myFSM.GetState() == BLINK3) {
		if(millis() - myFSM.GetEnteringTime(BLINK3) > 5000){
			dummyBoolVar = true;	
		}	
	}
}


// Check button status, return true only on rising edge (oldButton == false)
bool checkButton() {
	static bool oldButton;
	bool but = !digitalRead(button);
	delay(40);	// simple debounce button
	if( but != oldButton){
		oldButton = but;
		return true && !oldButton;
	}
	return false;
}

/////////// STATE MACHINE FUNCTIONS //////////////////

// Define "on entering" callback functions (just a message in this example)
void onEntering(){
  Serial.print(F("\non entering state "));
  Serial.println(myFSM.ActiveStateName());
}

// Define "on leaving" callback functions (just a message in this example)
void onLeaving(){	
  Serial.print(F("\non leaving state "));
  Serial.println(myFSM.ActiveStateName());
}

// Define "on state" callback functions (where led blinking is done)
void onStateBlink1(){	
	static uint32_t ledTime;
  dummyBoolVar = false;
	if(millis() - ledTime > BLINK1_TIME){
		ledTime = millis();
		digitalWrite(led, ! digitalRead(led));
	}
}

void onStateBlink2(){	
	static uint32_t ledTime;
	if(millis() - ledTime > BLINK2_TIME){
		ledTime = millis();
		digitalWrite(led, ! digitalRead(led));
	}
}

void onStateBlink3(){	
	static uint32_t ledTime;
	if(millis() - ledTime > BLINK3_TIME){
		ledTime = millis();
		digitalWrite(led, ! digitalRead(led));
	}
}

// Setup the State Machine properties
void setupStateMachine(){
  
  // Follow the order of defined enumeration for the state definition (will be used as index)
	//Add States  => name, 		timeout, onEnter callback, onState cb, 	  onLeave cb	
	myFSM.AddState(stateName[BLINK1], 0, onEntering, onStateBlink1, onLeaving);
	myFSM.AddState(stateName[BLINK2], 0, onEntering, onStateBlink2, onLeaving);
	myFSM.AddState(stateName[BLINK3], 0, onEntering, onStateBlink3, onLeaving);

	// Add transitions with related "trigger" callback functions	
	myFSM.AddTransition(BLINK1, BLINK2, checkButton );
	myFSM.AddTransition(BLINK2, BLINK3, checkButton );
	// For this transition let'use a bool variable (must be global, because we need fix address)
	myFSM.AddTransition(BLINK3, BLINK1, dummyBoolVar );
}