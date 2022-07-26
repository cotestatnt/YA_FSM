#include "YA_FSM.h"
#include "Blinker.h"

#define CHANGE_BTN 2
#define RESET_BTN 3
#define LED 13

#define BLINK1_TIME 1000
#define BLINK2_TIME 500
#define BLINK3_TIME 250

// Create a Blinker (included utility class) for led blink handling
Blinker blinker(LED, BLINK1_TIME);

// Create new Finite State Machine
YA_FSM myFSM;

// State Alias enumeration
enum State {BLINK1, BLINK2, BLINK3};

// Helper for print labels instead integer when state change
const char * const stateName[] PROGMEM = { "Blink1", "Blink2", "Blink3"};

// Let's use an array for change blink time
uint32_t blinkTime[] = {BLINK1_TIME, BLINK2_TIME, BLINK3_TIME};

// A variable for triggering BLINK3 -> BLINK1 transition
bool resetBlinky = false;

// Check button status, return true only on rising edge (oldButton == false)
bool checkButton() {
  static bool oldButton;    // store last button state
  static uint32_t pushTime;	// for button debouncing

	bool but = (digitalRead(CHANGE_BTN) == LOW);

	if( but != oldButton && millis() - pushTime > 200){
    pushTime = millis();
		oldButton = but;
		return !oldButton;
	}
	return false;
}

/////////// STATE MACHINE FUNCTIONS //////////////////

// Define "on entering" callback functions (just a message in this example)
void onEntering(){
  Serial.print(F("\n...on entering state "));
  Serial.println(myFSM.ActiveStateName());
}

// Define "on leaving" callback functions
void onLeaving(){	
  static uint8_t  blinkIndex = 0; 
  blinkIndex = (blinkIndex + 1) % 3;        // 0..1..2
  blinker.setTime(blinkTime[blinkIndex]);   // Update blinker frequency

  Serial.print(F("\n...on leaving state "));
  Serial.println(myFSM.ActiveStateName());
  Serial.print("Next blink time: ");
  Serial.println(blinkTime[blinkIndex]);
}


// Setup the State Machine properties
void setupStateMachine(){  
  /*
    Follow the order of defined enumeration for the state definition (will be used as index)
    You can add the states with 3 different overrides:
    Add States => name, timeout, minTime, onEnter cb, onState cb, onLeave cb
    Add States => name, timeout, onEnter cb, onState cb, onLeave cb
    Add States => name, onEnter cb, onState cb, onLeave cb
  */
	myFSM.AddState(stateName[BLINK1], onEntering, nullptr, onLeaving);
	myFSM.AddState(stateName[BLINK2], onEntering, nullptr, onLeaving);
	myFSM.AddState(stateName[BLINK3], onEntering, nullptr, onLeaving);

	// Add transitions with related "trigger" callback functions	
	myFSM.AddTransition(BLINK1, BLINK2, checkButton );
	myFSM.AddTransition(BLINK2, BLINK3, checkButton );

	// For this transition let'use a bool variable
	myFSM.AddTransition(BLINK3, BLINK1, resetBlinky);
}


void setup() {
	pinMode(RESET_BTN, INPUT_PULLUP);	
	pinMode(CHANGE_BTN, INPUT_PULLUP);	

	Serial.begin(115200);
	Serial.println(F("Starting State Machine...\n"));
	setupStateMachine();	

	// Initial state
	Serial.print(F("Active state: "));
	Serial.println(myFSM.ActiveStateName());
}


void loop() {
  // Read reset button input and update resetBlinky variable
  resetBlinky = (digitalRead(RESET_BTN) == LOW);
   
	// Update State Machine	(true is state changed)
	if(myFSM.Update()){
		Serial.print(F("Active state: "));
		Serial.println(myFSM.ActiveStateName());
	}	

	// Set outputs according to FSM
  blinker.blink(true);
}
