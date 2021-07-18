#include <YA_FSM.h>

const byte BTN_CALL = 2;
const byte GREEN_LED = 13;
const byte YELLOW_LED = 12;
const byte RED_LED = 11;

// STM32
/*
const byte BTN_CALL = PA10;
const byte GREEN_LED = PA5;
const byte YELLOW_LED = PA6;
const byte RED_LED = PA7;
*/

bool green[] = {1, 0, 0 };
bool yellow[] = {0, 1, 0 };
bool red[] = {0, 0, 1 };

// Create new FSM
YA_FSM stateMachine;

// State Alias
enum State {RED, GREEN, YELLOW, CALL};

// Helper for print labels instead integer when state change
const char * const stateName[] PROGMEM = { "RED", "GREEN", "YELLOW", "CALL"};

// This will trigger transition from GREEN to CALL state, the others transitions on timeout
bool callButton = false;

// Pedestrian traffic light -> green ligth ON until button pressed
// #define GREEN_TIME  20000    // always ON
#define YELLOW_TIME  2000
#define RED_TIME     10000
#define CALL_DELAY   5000


bool redLed = false;
bool greenLed = false;
bool yellowLed = false;

bool dummyVar = false;
bool timeLimited = false;
bool delayed = false;

void setup() {
  pinMode(BTN_CALL, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.begin(115200);
  Serial.println(F("Starting the Finite State Machine...\n"));
  setupStateMachine();

  // Initial state
  Serial.print(F("Active state: "));
  Serial.println(stateMachine.ActiveStateName());
}

void loop() {
  // Read inputs
  callButton = (digitalRead(BTN_CALL) == LOW);

  // Update State Machine (true is state changed)
  if(stateMachine.Update()){
    Serial.print(F("Active state: "));
    Serial.println(stateMachine.ActiveStateName());
  }

  // Set outputs
  digitalWrite(RED_LED, redLed);
  digitalWrite(GREEN_LED, greenLed);
  digitalWrite(YELLOW_LED, yellowLed);

  if(dummyVar) {
    Serial.print (millis());
    Serial.println (" dummyVar true!");
    delay(500);
  }

  if(timeLimited) {
    Serial.print (millis());
    Serial.println (" timeLimited true!");
    delay(500);
  }

  if(delayed) {
    Serial.print (millis());
    Serial.println (" delayed true!");
    delay(500);
  }
}


/////////// STATE MACHINE CALLBACK FUNCTIONS //////////////////
// Define "on entering" callback functions
void onEnterGreen(){ Serial.println(F("Green light ON")); }
void onEnterYellow(){ Serial.println(F("Yellow light ON")); }
void onEnterRed(){ Serial.println(F("Red light ON")); }

// Define "on leaving" callback functions
void onExitCall(){ Serial.println(F("Green light OFF\n")); }
void onExitYellow(){ Serial.println(F("Yellow light OFF\n")); }
void onExitRed(){ Serial.println(F("Red light OFF\n")); }


// Setup the State Machine
void setupStateMachine() {

  // Follow the order of defined enumeration for the state definition (will be used as index)
  // Add States => name,timeout, onEnter cb, onState cb, onLeave cb
  stateMachine.AddState(stateName[RED], RED_TIME, onEnterRed, nullptr, onExitRed);
  stateMachine.AddState(stateName[GREEN], 0, onEnterGreen, nullptr, nullptr);
  stateMachine.AddState(stateName[YELLOW], YELLOW_TIME, onEnterYellow, nullptr, onExitYellow);
  stateMachine.AddState(stateName[CALL], CALL_DELAY, nullptr, nullptr, onExitCall);

  // N -> target redLed will be on while step active
  stateMachine.AddAction(RED, YA_FSM::N, redLed);            // While state is active red led is ON
  stateMachine.AddAction(RED, YA_FSM::L, timeLimited, 1000); // Set unuseful bool var to test Time Limited actions
  stateMachine.AddAction(RED, YA_FSM::D, delayed, 8000);    //  Set unuseful bool var to test Time Delayed  actions
  stateMachine.AddAction(GREEN, YA_FSM::S, greenLed);    // SET green led on
  stateMachine.AddAction(YELLOW, YA_FSM::R, greenLed);   // RESET the green led
  stateMachine.AddAction(YELLOW, YA_FSM::N, yellowLed);  // While state is active yellow led is ON
  stateMachine.AddAction(YELLOW, YA_FSM::N, dummyVar);   // While state is active dummyVar is true

  // Add transitions with related trigger input callback functions
  stateMachine.AddTransition(RED, GREEN, [](){return stateMachine.CurrentState()->timeout;} );
  stateMachine.AddTransition(YELLOW, RED, [](){return stateMachine.CurrentState()->timeout;}  );
  stateMachine.AddTransition(CALL, YELLOW, [](){return stateMachine.CurrentState()->timeout;});
  stateMachine.AddTransition(GREEN, CALL, callButton);
}
