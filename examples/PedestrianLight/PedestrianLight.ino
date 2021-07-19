#include <YA_FSM.h>

const byte BTN_CALL = 2;
const byte GREEN_LED = 12;
const byte YELLOW_LED = 11;
const byte RED_LED = 10;

// Create new FSM
YA_FSM stateMachine;

// State Alias
enum State {RED, GREEN, YELLOW, CALL};

// Helper for print labels instead integer when state change
const char * const stateName[] PROGMEM = { "RED", "GREEN", "YELLOW", "CALL"};

// Pedestrian traffic light -> green ligth ON until button pressed
#define YELLOW_TIME  2000
#define RED_TIME     10000
#define CALL_DELAY   5000

// Input (trig transition from GREEN to CALL state, the others transitions on timeout)
bool callButton = false;

// Output variables
bool redLed = false;
bool greenLed = false;
bool yellowLed = false;

/////////// STATE MACHINE CALLBACK FUNCTIONS //////////////////
// Define "on entering" callback function (the same for all "light" states)
void onEnter() {
  Serial.print(stateMachine.ActiveStateName());
  Serial.println(F(" light ON")); 
}
// Define "on leaving" callback function (the same for all "light"  states)
void onExit() {
  Serial.print(stateMachine.ActiveStateName());
  Serial.println(F(" light OFF\n"));
}

void onEnterCall() {
  Serial.println(F("Call registered, please wait a little time.")); 
}

// Setup the State Machine
void setupStateMachine() {

  // Follow the order of defined enumeration for the state definition (will be used as index)
  // Add States => name,timeout, onEnter cb, onState cb, onLeave cb
  stateMachine.AddState(stateName[RED], RED_TIME, onEnter, nullptr, onExit);
  stateMachine.AddState(stateName[GREEN], 0, onEnter, nullptr, onExit);
  stateMachine.AddState(stateName[YELLOW], YELLOW_TIME, onEnter, nullptr, onExit);
  stateMachine.AddState(stateName[CALL], CALL_DELAY, onEnterCall, nullptr, nullptr);

  stateMachine.AddAction(RED, YA_FSM::N, redLed);        // N -> while state is active red led is ON
  stateMachine.AddAction(GREEN, YA_FSM::S, greenLed);    // S -> SET green led on
  stateMachine.AddAction(YELLOW, YA_FSM::R, greenLed);   // R -> RESET the green led
  stateMachine.AddAction(YELLOW, YA_FSM::N, yellowLed);  // N -> while state is active yellow led is ON

  // Add transitions with related trigger input callback functions
  stateMachine.AddTransition(RED, GREEN, [](){return stateMachine.CurrentState()->timeout;} );
  stateMachine.AddTransition(YELLOW, RED, [](){return stateMachine.CurrentState()->timeout;}  );
  stateMachine.AddTransition(CALL, YELLOW, [](){return stateMachine.CurrentState()->timeout;});
  stateMachine.AddTransition(GREEN, CALL, callButton);
}

void setup() {
  pinMode(BTN_CALL, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  
  Serial.begin(115200);
  while(!Serial) {}
  Serial.println(F("Starting the Finite State Machine...\n"));
  setupStateMachine();
}

void loop() {
  // Read inputs
  callButton = (digitalRead(BTN_CALL) == LOW);

  // Update State Machine
  if(stateMachine.Update()){
    Serial.print(F("Active state: "));
    Serial.println(stateMachine.ActiveStateName());
  }

  // Set outputs
  digitalWrite(RED_LED, redLed);
  digitalWrite(GREEN_LED, greenLed);
  digitalWrite(YELLOW_LED, yellowLed);
}
