#include <YA_FSM.h>

const byte SIG_TRAIN_IN = 2;
const byte SIG_TRAIN_OUT = 3;

const byte OUT_BAR_LOW  = 11;
const byte OUT_BAR_HIGH = 12;
const byte OUT_LIGHT_BELL = 13;

// Create new FSM
YA_FSM stateMachine;

// State Alias
enum State {GATE_OPEN, GATE_LOWERING, GATE_CLOSE, GATE_WAIT, GATE_RISING };

// Helper for print labels instead integer when state change
const char* const stateName[] PROGMEM = { "Gate OPEN", "Lowering GATE", "Gate CLOSE", "Wait time", "Rising GATE"};

// After the train has passed, wait a little time to be sure
#define MOVE_TIME  5000   // Time needed for GATE moving (up and down)
#define WAIT_FREE  15000   
#define BLINK_TIME 250     

// Instead of bool function() callback, in this example we will use 
// bool variables to trig two transitions (the others will trig on timout)
bool theTrainIsComing = false;
bool theTrainIsGone = false;

void setup() {
  // Input/Output configuration
  pinMode(SIG_TRAIN_IN, INPUT_PULLUP);
  pinMode(SIG_TRAIN_OUT, INPUT_PULLUP);
  pinMode(OUT_BAR_LOW, OUTPUT);
  pinMode(OUT_BAR_HIGH, OUTPUT);
  pinMode(OUT_LIGHT_BELL, OUTPUT);

  Serial.begin(115200);
  while (!Serial) { ; }   // Wait for serial to be ready (native USB board only)

  Serial.println(F("Starting State Machine...\n"));
  setupStateMachine();
  
  onEnter();    // Call manually the onEnter() function to set outputs
  Serial.print(F("Active state: "));
  Serial.println(stateMachine.ActiveStateName());
}

void loop() {
  // Update State Machine
  stateMachine.Update(); 

  // Update the bool variables according to the signal inputs
  theTrainIsGone = digitalRead(SIG_TRAIN_OUT) == LOW;
  theTrainIsComing = digitalRead(SIG_TRAIN_IN) == LOW;

  if (theTrainIsComing) {
    // Reset enter time of GATE_WAIT (if active) so timeout became
    // longer enough to wait two ore more trains one after the other 
    stateMachine.SetEnteringTime(GATE_WAIT);
  }
}


// Define "on entering" callback function (the same for all states)
void onEnter(){ 
  switch (stateMachine.GetState() ){
    case GATE_CLOSE: 
      digitalWrite(OUT_BAR_LOW, HIGH);
      Serial.println(F("The GATE is actually close"));
      break;
    case GATE_RISING:
      digitalWrite(OUT_BAR_LOW, LOW);
      digitalWrite(OUT_LIGHT_BELL, LOW);       
      Serial.println(F("The GATE is going to be opened"));
      break;
    case GATE_OPEN:
      digitalWrite(OUT_BAR_HIGH, HIGH);
      Serial.println(F("The GATE is actually open"));
      break;
    case GATE_LOWERING:
      digitalWrite(OUT_BAR_HIGH, LOW);
      Serial.println(F("A new train is coming! Start closing the GATE."));
      Serial.println(F("The GATE is going to be closed"));
      break;
    case GATE_WAIT:
      Serial.println(F("The GATE is stille closed and we have to wait a little"));
      break;
  }
}

 // Blink and play the bell while gate is moving or closed
void onStateMoveOpen(){
  static uint32_t bTime;
  if(millis() - bTime > BLINK_TIME) {
    bTime = millis();
    digitalWrite(OUT_LIGHT_BELL, !digitalRead(OUT_LIGHT_BELL));
  }
}


// Setup the State Machine
void setupStateMachine() {
  
  /* 
   * Follow the order of enumeration for the state definition, if used.
   * Index of states will be increased in library on every AddState()
   * In this way, enumerated index of skecth will match the library index.
  */
  
  // Add States => name,timeout, onEnter cb, onState cb, onLeave cb
  stateMachine.AddState(stateName[GATE_OPEN], 0, onEnter, nullptr, nullptr);  
  stateMachine.AddState(stateName[GATE_LOWERING], MOVE_TIME, onEnter, onStateMoveOpen, nullptr);  
  stateMachine.AddState(stateName[GATE_CLOSE], 0, onEnter, onStateMoveOpen, nullptr); 
  stateMachine.AddState(stateName[GATE_WAIT], WAIT_FREE, onEnter, onStateMoveOpen, nullptr);  
  stateMachine.AddState(stateName[GATE_RISING], MOVE_TIME, onEnter, onStateMoveOpen, nullptr);
  
  // Add transitions with related trigger input callback functions  
  stateMachine.AddTransition(GATE_OPEN,   GATE_LOWERING, theTrainIsComing);
  stateMachine.AddTransition(GATE_LOWERING, GATE_CLOSE,  [](){return stateMachine.CurrentState()->timeout;} );
  stateMachine.AddTransition(GATE_CLOSE,  GATE_WAIT,     theTrainIsGone);
  stateMachine.AddTransition(GATE_WAIT,   GATE_RISING,   [](){return stateMachine.CurrentState()->timeout;} );
  stateMachine.AddTransition(GATE_RISING, GATE_OPEN,     [](){return stateMachine.CurrentState()->timeout;} );
}
