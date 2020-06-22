
#include <Arduino.h>
#include <YA_FSM.h>

/*
const byte BTN_START = PC13;
const byte BTN_FTC1 = PC12;
const byte LED_OPEN = PA6;
const byte LED_CLOSE = PA5;
*/

const byte BTN_START = 2;
const byte BTN_FTC1 = 3;
const byte LED_OPEN = 4;
const byte LED_CLOSE = 5;


// Create new SC (num States, num Transition)
YA_FSM stateMachine(4, 5);

// Input Alias. We use Input as trigging condition in defined transition
enum Input {xSTART_OPEN, xOPENED, xWAIT_DONE, xCLOSED, xFTC, xUNKNOWN};

// State Alias
enum State {CLOSED, CLOSING, OPENED, OPENING};

// Helper for print labels instead integer when state change
const char *stateName[] = { "CLOSED", "CLOSING", "OPENED", "OPENING"};

// Stores last user input and the current active state
Input input;
uint8_t currentState;


#define OPEN_TIME  10000  // 15s
#define WAIT_TIME  8000  // 15s
#define CLOSE_TIME 10000  // 10s


void setup() 
{
	pinMode(BTN_START, INPUT_PULLUP);
	pinMode(BTN_FTC1, INPUT_PULLUP);
	pinMode(LED_OPEN, OUTPUT);
	pinMode(LED_CLOSE, OUTPUT);

	Serial.begin(115200);

	Serial.println(F("Starting State Machine...\n"));
	setupStateMachine();	

	// Initial state
	input = Input::xCLOSED;
	currentState = stateMachine.GetState();
	Serial.print(F("Active state: "));
	Serial.println(stateName[currentState]);
}

void loop() 
{

	if(digitalRead(BTN_START) == LOW){
		input = Input::xSTART_OPEN;
		Serial.println(F("Start button pressed"));
		delay(100); // debounce button
	}

	if(digitalRead(BTN_FTC1) == LOW){
		input = Input::xFTC;
		Serial.println(F("FTC interrupted"));
		delay(100); // debounce button
	}

	// Update State Machine	(true is state changed)
	if(stateMachine.Update()){
		currentState = stateMachine.GetState();
		Serial.print(F("Active state: "));
		Serial.println(stateName[currentState]);
	}
	
}



/////////// STATE MACHINE FUNCTIONS //////////////////



// Define "on entering" callback functions
// If callback functions are simple, can also be putted inline []()

void onEnteringClosed(){
	Serial.println(F("Gate closed."));
}

void onEnteringOpened(){
	Serial.println(F("Gate opened."));
}


// Setup the State Machine
void setupStateMachine()
{
	// Add transitions with related callback functions
	///// ---- Be carefull, total number of transitions MUST be as declared -------  ///////
	stateMachine.AddTransition(CLOSED, OPENING, [](){return input == Input::xSTART_OPEN;} );	
	stateMachine.AddTransition(OPENING, OPENED, [](){return input == Input::xOPENED;} 	  );
	stateMachine.AddTransition(OPENED, CLOSING, [](){return input == Input::xWAIT_DONE;}  );
	stateMachine.AddTransition(CLOSING, CLOSED, [](){return input == Input::xCLOSED;}	  );
	stateMachine.AddTransition(CLOSING, OPENING, [](){return input == Input::xFTC; }	  );

	// Add on enetering and on leaving actions (previuos defined)
	stateMachine.SetOnEntering(CLOSED, onEnteringClosed);
	stateMachine.SetOnEntering(OPENED, onEnteringOpened);

	// On leavine callback functions ( inline)
	stateMachine.SetOnLeaving(CLOSED, []() {Serial.println(F("Gate is closed. Now start opening.")); });
	stateMachine.SetOnLeaving(OPENED, []() {Serial.println(F("Gate is opened. Now start closing.")); });

	// Add the onState callback function. This will be excuted while the state is active

	// State CLOSED active (without timeout)
	stateMachine.SetOnState(CLOSED,	[](){ digitalWrite(LED_CLOSE, HIGH);});

	// State OPENED active (with timeout)
	stateMachine.SetOnState(OPENED, 
		[](){ 
			digitalWrite(LED_OPEN, HIGH);

			// Wait with gate opened is done, start closing gate
			// Check if current state has timeouted, if yes go to next triggering the right input
			bool timeout = stateMachine.GetTimeout(stateMachine.GetState()); 
			if(timeout) {
				input = Input::xWAIT_DONE;	
			}
		},
		WAIT_TIME );

	// State CLOSING active (with timeout)
	stateMachine.SetOnState(CLOSING, 
		[](){ 
			digitalWrite(LED_OPEN, LOW);

			// Blink at 4 Hz when opening state is active
			static uint32_t blinkTimeClose = 0;				
			if(millis() - blinkTimeClose > 250 ){
				blinkTimeClose = millis();
				digitalWrite(LED_CLOSE, !digitalRead(LED_CLOSE));
			}
			
			// Check if current state has timeouted, if yes go to next triggering the right input
			bool timeout = stateMachine.GetTimeout(stateMachine.GetState()); 
			if(timeout) {
				input = Input::xCLOSED;
			}
		},
		// This state has timeout
		CLOSE_TIME );

	// State OPENING active (with timeout)
	stateMachine.SetOnState(OPENING, 
		[](){ 
			digitalWrite(LED_CLOSE, LOW);

			// Blink at 2Hz when opening state is active
			static uint32_t blinkTimeOpen = 0;				
			if(millis() - blinkTimeOpen > 500 ){
				blinkTimeOpen = millis();
				digitalWrite(LED_OPEN, !digitalRead(LED_OPEN));
			}

			// After while, gate is opened. We set OPENING state with timeout
			// When timeout fired, start waiting WAIT_TIME milliseconds

			// Check if current state has timeouted, if yes go to next triggering the right input
			bool timeout = stateMachine.GetTimeout(stateMachine.GetState()); 
			if(timeout) {
				input = Input::xOPENED;
			}
		},
		// This state has timeout
		OPEN_TIME );

	
}