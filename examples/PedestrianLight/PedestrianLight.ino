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

// Create new FSM
YA_FSM stateMachine;

// State Alias
enum State {RED, GREEN, YELLOW, CALL};

// Helper for print labels instead integer when state change
const char *const stateName[] PROGMEM = {"RED", "GREEN", "YELLOW", "CALL"};

// Pedestrian traffic light -> green ligth ON until button pressed
#define YELLOW_TIME 2000
#define RED_TIME 5000
#define CALL_DELAY 3000

// Input (trig transition from GREEN to CALL state, the others transitions on timeout)
bool callButton = false;

// Output variables
bool redLed = false;
bool greenLed = false;
bool yellowLed = false;

/////////// STATE MACHINE CALLBACK FUNCTIONS //////////////////
// Define "on entering" callback function (the same for all "light" states)
void onEnter()
{
  Serial.print(stateMachine.ActiveStateName());
  Serial.println(F(" light ON"));
}

// Define "on leaving" callback function (the same for all "light"  states)
void onExit()
{
  Serial.print(stateMachine.ActiveStateName());
  Serial.println(F(" light OFF\n"));
}

// Define "on enter" for CALL button state
void onEnterCall()
{
  Serial.println(F("Call registered, please wait a little time."));
}

// Setup the State Machine
void setupStateMachine()
{

  /*
   Follow the order of defined enumeration for the state definition (will be used as index)
   You can add the states with 3 different overrides:
   Add States => name, timeout, minTime, onEnter cb, onState cb, onLeave cb
   Add States => name, timeout, onEnter cb, onState cb, onLeave cb
   Add States => name, onEnter cb, onState cb, onLeave cb
  */
  stateMachine.AddState(stateName[RED], RED_TIME, onEnter, nullptr, onExit);
  stateMachine.AddState(stateName[GREEN], onEnter, nullptr, onExit);
  stateMachine.AddState(stateName[YELLOW], YELLOW_TIME, onEnter, nullptr, onExit);
  stateMachine.AddState(stateName[CALL], CALL_DELAY, onEnterCall, nullptr, nullptr);

  stateMachine.AddAction(RED, YA_FSM::N, redLed);       // N -> while state is active red led is ON
  stateMachine.AddAction(GREEN, YA_FSM::S, greenLed);   // S -> SET green led on
  stateMachine.AddAction(YELLOW, YA_FSM::R, greenLed);  // R -> RESET the green led
  stateMachine.AddAction(YELLOW, YA_FSM::N, yellowLed); // N -> while state is active yellow led is ON

  // Add "timed" transitions: it will be triggered on previous defined state timeout
  stateMachine.AddTransition(RED, GREEN);
  stateMachine.AddTransition(YELLOW, RED);

  // This is the same as before, just with a more specific method name
  stateMachine.AddTimedTransition(CALL, YELLOW);

  // Add transitions with related trigger bool variable
  stateMachine.AddTransition(GREEN, CALL, callButton);
}

// Print on serial some infos about each state
void fsmInfo(YA_FSM &fsm)
{
  for (int i = 0; i <= fsm.GetNumStates(); i++)
  {
    FSM_State *state = fsm.GetStateAt(i);
    Serial.print(state->stateName);
    Serial.print(": Timeout ");
    Serial.print(state->maxTime);
    Serial.print("ms; Min duration ");
    Serial.print(state->minTime);
    Serial.println("ms.");
  }
}

void setup()
{
  // Setup Input/Output
  pinMode(BTN_CALL, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.begin(115200);
  Serial.println(F("Setup the Finite State Machine...\n"));
  setupStateMachine();
  fsmInfo(stateMachine);
  Serial.print(F("\nActive state: "));
  Serial.println(stateMachine.ActiveStateName());
}

void loop()
{
  // Read button input and update callButton variable
  callButton = (digitalRead(BTN_CALL) == LOW);

  // Update State Machine
  if (stateMachine.Update())
  {
    Serial.print(F("Active state: "));
    Serial.println(stateMachine.ActiveStateName());
  }

  // Set outputs according to FSM
  digitalWrite(RED_LED, redLed);
  digitalWrite(GREEN_LED, greenLed);
  digitalWrite(YELLOW_LED, yellowLed);
}
