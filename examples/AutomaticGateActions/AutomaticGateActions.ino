////////////////////// Small class for blink led handling  //////////////////////////////////////
class Blinker
{
  private:
    uint32_t blinkTime;
    const uint8_t ledPin;
    const uint32_t halfPeriod;

  public:
    BlinkLed(uint8_t pin, uint32_t time) : ledPin(pin), halfPeriod(time) {
      pinMode(ledPin, OUTPUT);
    }

    bool blink(bool active) {
      if (active) {
        if (millis() - blinkTime > halfPeriod ) {
          blinkTime = millis();
          digitalWrite(ledPin, !digitalRead(ledPin));
        }
      }
      else
        digitalWrite(ledPin, LOW);
      return active;
    }
};
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "YA_FSM.h"

/*
  //STM32
  const byte BTN_START = PB6;
  const byte SEC_FTC1 = PC7;
  const byte LED_OPEN = PA6;
  const byte LED_CLOSE = PA5;
*/

const byte SEC_FTC1 = 3;
const byte BTN_START = 2;
const byte LED_OPEN = 12;
const byte LED_CLOSE = 10;

// Create new SFC 
YA_FSM stateMachine;

// Input Alias. We use Input as trigging condition in defined transition
enum Input {xIDLE, xSTART_OPEN, xSAFETY_FTC};

// Output variables
bool startOpening = false;
bool safetyFtcTriggered = false;
bool outBlinkClose = false;
bool outBlinkOpen = false;
bool outClosed = false;
bool outOpened = false;


// State Alias
enum State {CLOSED, CLOSING, OPENED, OPENING, STOP_WAIT};

// Helper for print labels instead integer when state change
const char * const stateName[] PROGMEM = { "CLOSED", "CLOSING", "OPENED", "OPENING", "STOP_WAIT"};

// Stores last user input and the current active state
Input input = xIDLE;
uint8_t currentState;

#define OPEN_TIME       10000
#define WAIT_OPEN_TIME  8000
#define CLOSE_TIME      10000
#define WAIT_FTC_TIME   5000    // If FTC1 during CLOSING, stop all, wait and then open again

// Blink at 4 Hz when closing state is active
Blinker blinkClose(LED_CLOSE, 250);

// Blink at 2Hz when opening state is active
Blinker blinkOpen(LED_OPEN, 500);


/////////// STATE MACHINE CALLBACK FUNCTIONS ////////////////////////////////////////////////////

// This callback functions will bel called once (on entering states)
void onEnteringClosed() {
  input = Input::xIDLE;
  Serial.println(F("Gate closed."));
}

void onEnteringOpened() {
  Serial.println(F("Gate opened."));
}

void onExitClosed() {
  Serial.println(F("Start button pressed: going to open gate."));  
}

void onEnteringStopWait() {
  Serial.println(F("Safety FTC interrupted: \nclosing gate aborted, re-open and then restart the sequence after a pause."));
}
/////////////////////////////////////////////////////////////////////////////////////////////////

// Setup the State Machine
void setupStateMachine() {
  /*
    Follow the order of defined enumeration for the state definition (will be used as index)
    You can add the states with 3 different overrides:
    Add States => name, timeout, minTime, onEnter cb, onState cb, onLeave cb
    Add States => name, timeout, onEnter cb, onState cb, onLeave cb
    Add States => name, onEnter cb, onState cb, onLeave cb
  */
  stateMachine.AddState(stateName[CLOSED], onEnteringClosed, nullptr, onExitClosed);
  stateMachine.AddState(stateName[CLOSING], CLOSE_TIME, nullptr, nullptr, nullptr);
  stateMachine.AddState(stateName[OPENED],  WAIT_OPEN_TIME, onEnteringOpened, nullptr, nullptr);
  stateMachine.AddState(stateName[OPENING], OPEN_TIME,  nullptr, nullptr, nullptr);
  stateMachine.AddState(stateName[STOP_WAIT], WAIT_FTC_TIME, onEnteringStopWait, nullptr, nullptr);

  // Add transitions with related callback functions ( FROM, TO, lambda callback function)
  stateMachine.AddTransition(CLOSED, OPENING, startOpening);
  stateMachine.AddTransition(CLOSING, STOP_WAIT, safetyFtcTriggered);

  // Add "timed" transitions: it will be triggered on defined max state time
  stateMachine.AddTimedTransition(OPENING, OPENED);
  stateMachine.AddTimedTransition(OPENED, CLOSING);
  stateMachine.AddTimedTransition(CLOSING, CLOSED);
  stateMachine.AddTimedTransition(STOP_WAIT, OPENING);

  // Add the actions related to each state 
  /*
    N -> while state is active target variable == true
    S -> SET target variable
    R -> RESET target variable
    L -> set target true for a limited time (Toff)
    D -> set target to true after a delay (Ton)
  */
  stateMachine.AddAction(CLOSED, YA_FSM::N, outClosed);
  stateMachine.AddAction(OPENED, YA_FSM::N, outOpened);
  stateMachine.AddAction(CLOSING, YA_FSM::N, outBlinkClose);
  stateMachine.AddAction(OPENING, YA_FSM::N, outBlinkOpen);
  stateMachine.AddAction(STOP_WAIT, YA_FSM::N, outClosed);
  stateMachine.AddAction(STOP_WAIT, YA_FSM::N, outOpened);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(SEC_FTC1, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println(F("Setup Finite State Machine...\n"));
  setupStateMachine();

  Serial.print(F("Active state: "));
  Serial.println(stateMachine.ActiveStateName());
}

void loop() {
  // Read start button and update input variable
  startOpening = ((digitalRead(BTN_START) == LOW) && (currentState == CLOSED));

  // Read safety FTC input and update input variable
  safetyFtcTriggered = (digitalRead(SEC_FTC1) == LOW);

  // Update State Machine  (true is state changed)
  if (stateMachine.Update()) {
    currentState = stateMachine.GetState();
    Serial.print(F("Active state: "));
    Serial.println(stateMachine.ActiveStateName());
  }

  // Set outputs according to FSM
  if (!blinkOpen.blink(outBlinkOpen)) {
    // if not blinking set led according to value of outOpened
    digitalWrite(LED_OPEN, outOpened);  
  }  
  if (!blinkClose.blink(outBlinkClose)) {
    // if not blinking set led according to value of outClosed
    digitalWrite(LED_CLOSE, outClosed);
  }

}
