#include <YA_FSM.h>

/*
//STM32
const byte BTN_START = PB6;
const byte SEC_FTC1 = PC7;
const byte LED_OPEN = PA6;
const byte LED_CLOSE = PA5;
*/

const byte SEC_FTC1 = 3;
const byte BTN_START = 2;
const byte LED_OPEN = 13;
const byte LED_CLOSE = 12;

// Create new SC (num States, num Transition)
YA_FSM stateMachine;

// Input Alias. We use Input as trigging condition in defined transition
enum Input {xSTART_OPEN, xOPENED, xWAIT_DONE, xCLOSED, xFTC, xREOPEN};

// State Alias
enum State {CLOSED, CLOSING, OPENED, OPENING, STOP_WAIT};

// Helper for print labels instead integer when state change
const char * const stateName[] PROGMEM = { "CLOSED", "CLOSING", "OPENED", "OPENING", "STOP_WAIT"};

// Stores last user input and the current active state
Input input;
uint8_t currentState;

bool securityOpen = false;

#define OPEN_TIME       10000  
#define WAIT_OPEN_TIME  8000  
#define CLOSE_TIME      10000 
#define WAIT_FTC_TIME   5000    // If FTC1 during CLOSING, stop all, wait and then open again

void setup() {
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(SEC_FTC1, INPUT_PULLUP);
  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_CLOSE, OUTPUT);

  Serial.begin(115200);
  Serial.println(F("Starting State Machine...\n"));
  setupStateMachine();

  // Set initial state
  input = Input::xCLOSED;
  stateMachine.Update();
  currentState = stateMachine.GetState();
  Serial.print(F("Active state: "));
  Serial.println(stateMachine.ActiveStateName());
}

void loop() {
  if (digitalRead(BTN_START) == LOW && currentState == CLOSED) {
    input = Input::xSTART_OPEN;
    Serial.println(F("Start button pressed"));
    delay(500); // debounce button
  }

  if (digitalRead(SEC_FTC1) == LOW) {
    input = Input::xFTC;
    Serial.println(F("FTC interrupted"));
    delay(100); // debounce button
  }

  // Update State Machine  (true is state changed)
  if (stateMachine.Update()) {
    currentState = stateMachine.GetState();
    Serial.print(F("Active state: "));
    Serial.println(stateMachine.ActiveStateName());
  }
}


/////////// STATE MACHINE FUNCTIONS //////////////////

// This callback functions will bel called once on entering states
void onEnteringClosed() {
  Serial.println(F("Gate closed."));
}

void onEnteringOpened() {
  Serial.println(F("Gate opened."));
}

// This callback functions will bel called continuosly while state is active
void onStateClosed() {
  digitalWrite(LED_CLOSE, HIGH);
}

void onStateOpened() {
  digitalWrite(LED_OPEN, HIGH);

  // Wait WAIT_TIME millisends, then trigger for the next state
  if ( stateMachine.CurrentState()->timeout ) {
    input = Input::xWAIT_DONE;
  }
}

void onStateClosing() {
  digitalWrite(LED_OPEN, LOW);

  // Blink at 4 Hz when closing state is active
  static uint32_t blinkTimeClose = 0;
  if (millis() - blinkTimeClose > 250 ) {
    blinkTimeClose = millis();
    digitalWrite(LED_CLOSE, !digitalRead(LED_CLOSE));
  }

  // Check if current state has timeouted,  then trigger for the next state
  if ( stateMachine.CurrentState()->timeout ) {
    input = Input::xCLOSED;
  }
}


void onStateOpening() {
  digitalWrite(LED_CLOSE, LOW);

  // Blink at 2Hz when opening state is active
  static uint32_t blinkTimeOpen = 0;
  if (millis() - blinkTimeOpen > 500 ) {
    blinkTimeOpen = millis();
    digitalWrite(LED_OPEN, !digitalRead(LED_OPEN));
  }

  // After while, gate is opened.
  // Check if current state has timeouted, then trigger for the next state
  if ( stateMachine.CurrentState()->timeout ) {
    input = Input::xOPENED;
  }
}


void onStateStopWait() {
  // Stop immediatly, and after a while re-open the gate
  digitalWrite(LED_CLOSE, LOW);
  digitalWrite(LED_OPEN, LOW);  
  // Check if current state has timeouted, then trigger for the next state
  if ( stateMachine.CurrentState()->timeout ) {
    input = Input::xREOPEN;
  }  
}

// Setup the State Machine
void setupStateMachine() {
  // Follow the order of defined enumeration for the state definition (will be used as index)
  //Add States          => name,     timeout,     onEnter callback, onState cb,    onLeave cb
  stateMachine.AddState(stateName[CLOSED],  0,          onEnteringClosed, onStateClosed, nullptr);
  stateMachine.AddState(stateName[CLOSING], CLOSE_TIME, nullptr, onStateClosing, nullptr);
  stateMachine.AddState(stateName[OPENED],  WAIT_OPEN_TIME,  onEnteringOpened, onStateOpened, nullptr);
  stateMachine.AddState(stateName[OPENING], OPEN_TIME,  nullptr, onStateOpening, nullptr);
  stateMachine.AddState(stateName[STOP_WAIT], WAIT_FTC_TIME,  nullptr, onStateStopWait, nullptr);

   // Add transitions with related callback functions ( FROM, TO, Trigger)
  stateMachine.AddTransition(CLOSED, OPENING, [](){return input == xSTART_OPEN; });
  stateMachine.AddTransition(OPENING, OPENED, [](){return input == xOPENED;     });
  stateMachine.AddTransition(OPENED, CLOSING, [](){return input == xWAIT_DONE;  });
  stateMachine.AddTransition(CLOSING, CLOSED, [](){return input == xCLOSED;     });
  stateMachine.AddTransition(CLOSING, STOP_WAIT,[](){return input == xFTC;      });
  stateMachine.AddTransition(STOP_WAIT, OPENING, [](){return input == xREOPEN;  });
}