
#include <Arduino.h>
#include <YA_FSM.h>

const uint8_t button1 = 2;
const uint8_t button2 = 3;

const uint8_t led = 13;

// Create new SM (num States, num Transition)
YA_FSM myFSM(3, 3);

// State Alias 
enum State {BLINK1, BLINK2, BLINK3};

// Helper for print labels instead integer when state change
const char *stateName[] = { "Blink 1", "Blink 2", "Blink 3"};

#define BLINK1_TIME  	1000		
#define BLINK2_TIME 	300 
#define BLINK3_TIME 	100

// A simple bool var just to test transition's trigger
bool dummyBoolVar = false;

void setup() 
{
	pinMode(button1, INPUT_PULLUP);
	pinMode(button2, INPUT_PULLUP);
	pinMode(led, OUTPUT);
	Serial.begin(115200);
	Serial.println(F("Starting State Machine...\n"));
	setupStateMachine();	

	// Initial state
	Serial.print(F("Active state: "));
	Serial.println(stateName[myFSM.GetState()]);
}

void loop() 
{
	// Update State Machine	(true if state changed)
	if(myFSM.Update()){
		Serial.print(F("Active state: "));
		Serial.println(stateName[myFSM.GetState()]);
	}	
	
	// NOT digitalRead() because we enable Pull up resistor in setup() 
	dummyBoolVar = !digitalRead(button1) && !digitalRead(button2);		
	
	/*
	// When BLINK3 state is active, wait some time and the set true dummyBoolVar
	// to test transition from BLINK3 to BLINK1 with a bool variables instead a callback function
	if(myFSM.GetState() == BLINK3) {
		if(millis() - myFSM.GetEnteringTime(BLINK3) > 5000){
			dummyBoolVar = true;	
		}	
	}
	*/
	

}


// Check button status, return true only on rising edge (oldButton == false)
bool checkButton(){
	static bool oldButton;
	bool but = !digitalRead(button2);
	delay(40);	// simple debounce button
	if( but != oldButton){
		oldButton = but;
		return true && !oldButton;
	}
	return false;
}

/////////// STATE MACHINE FUNCTIONS //////////////////


// Define "on entering" callback functions (just a message in this example)
void onEnteringBlink1(){Serial.println(F("\non entering state BLINK1")); dummyBoolVar = false;}

void onEnteringBlink2(){Serial.println(F("\non entering state BLINK2")); }

void onEnteringBlink3(){Serial.println(F("\non entering state BLINK3")); }

// Define "on leaving" callback functions (just a message in this example)
void onLeavingBlink1(){	Serial.println(F("on leaving state BLINK1")); }

void onLeavingBlink2(){	Serial.println(F("on leaving state BLINK2")); }

void onLeavingBlink3(){	Serial.println(F("on leaving state BLINK3")); }

// Define "on state" callback functions (where led blinking is done)
void onStateBlink1(){	
	static uint32_t ledTime;
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
void setupStateMachine()
{
	// Add transitions with related "trigger" callback functions
	///// ---- Be carefull, total number of transitions MUST be as declared -------  ///////
	myFSM.AddTransition(BLINK1, BLINK2, checkButton );
	myFSM.AddTransition(BLINK2, BLINK3, checkButton );
	// For this transition let'use a bool variable (must be global, because we need fix address)
	myFSM.AddTransition(BLINK3, BLINK1, dummyBoolVar );

	// Set On Entering callback functions 
	myFSM.SetOnEntering(BLINK1, onEnteringBlink1);
	myFSM.SetOnEntering(BLINK2, onEnteringBlink2);
	myFSM.SetOnEntering(BLINK3, onEnteringBlink3);

	// Set On Leaving callback functions 
	myFSM.SetOnLeaving(BLINK1, onLeavingBlink1);
	myFSM.SetOnLeaving(BLINK2, onLeavingBlink2);
	myFSM.SetOnLeaving(BLINK3, onLeavingBlink3);

	// Add the onState callback function. This will be excuted while the state is active
	myFSM.SetOnState(BLINK1, onStateBlink1);
	myFSM.SetOnState(BLINK2, onStateBlink2);
	myFSM.SetOnState(BLINK3, onStateBlink3);	
}
