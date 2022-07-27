#include <Servo.h>
#include "YA_FSM.h"
#include "Blinker.h"    

#define SIG_TRAIN_IN 3
#define SIG_TRAIN_OUT 2
#define SERVO_PIN  6

#define PASS_NOT_ALLOWED  11
#define PASS_ALLOWED 12
#define OUT_LIGHT_BELL 13

// After the train has passed, wait a little time to be sure
#define MOVE_TIME  1000   // Time needed for GATE moving (up and down)
#define WAIT_FREE  10000  // Wait time after train has gone (in case another train is arriving)
#define BLINK_TIME 250     

#define GATE_CLOSED  0
#define GATE_OPENED  90

// Create new FSM
YA_FSM stateMachine;

// Create a Blinker (included utility class) for led blink handling
Blinker blinker(OUT_LIGHT_BELL, BLINK_TIME);

// Handle the gate position
Servo theGate; 
uint16_t servoPos = GATE_OPENED;

// State Alias
enum State {GATE_OPEN, GATE_LOWERING, GATE_CLOSE, GATE_WAIT, GATE_RISING };

// Helper for print labels instead integer when state change
const char* const stateName[] PROGMEM = { "Gate OPEN", "Lowering GATE", "Gate CLOSE", "Wait time", "Rising GATE"};

// Instead of bool function() callback, in this example we will use 
// bool variables to trig two transitions (the others will trig on timout)
bool theTrainIsComing = false;
bool theTrainIsGone = false;

// Define "on entering" callback function (the same for all states)
void onEnter(){   
  switch (stateMachine.GetState() ){
    case GATE_CLOSE: 
      digitalWrite(PASS_NOT_ALLOWED, HIGH);
      Serial.println(F("The GATE is actually close"));
      break;
    case GATE_RISING:      
      servoPos = GATE_OPENED;
      digitalWrite(OUT_LIGHT_BELL, LOW);       
      Serial.println(F("The GATE is going to be opened"));
      break;
    case GATE_OPEN:
      digitalWrite(PASS_NOT_ALLOWED, LOW);
      digitalWrite(PASS_ALLOWED, HIGH);
      Serial.println(F("The GATE is actually open"));
      break;
    case GATE_LOWERING:
      servoPos = GATE_CLOSED;
      digitalWrite(PASS_ALLOWED, LOW);
      digitalWrite(PASS_NOT_ALLOWED, HIGH);
      Serial.println(F("A new train is coming! Start closing the GATE."));
      Serial.println(F("The GATE is going to be closed"));
      break;
    case GATE_WAIT:
      Serial.println(F("Train passed, but we have to wait a little time more"));
      break;
  }
}

// Blink and play the bell while gate is moving or closed
void blinkAndHorn(){
  blinker.blink(true);
}


// Setup the State Machine
void setupStateMachine() {  
  /*
    Follow the order of defined enumeration for the state definition (will be used as index)
    You can add the states with 3 different overrides:
    Add States => name, timeout, minTime, onEnter cb, onState cb, onLeave cb
    Add States => name, timeout, onEnter cb, onState cb, onLeave cb
    Add States => name, onEnter cb, onState cb, onLeave cb
  */
  stateMachine.AddState(stateName[GATE_OPEN], onEnter, nullptr, nullptr);  
  stateMachine.AddState(stateName[GATE_LOWERING], MOVE_TIME, onEnter, blinkAndHorn, nullptr);  
  stateMachine.AddState(stateName[GATE_CLOSE], onEnter, blinkAndHorn, nullptr); 
  stateMachine.AddState(stateName[GATE_WAIT], WAIT_FREE, onEnter, blinkAndHorn, nullptr);  
  stateMachine.AddState(stateName[GATE_RISING], MOVE_TIME, onEnter, blinkAndHorn, nullptr);
  
  // Add transitions with related trigger input callback functions  
  stateMachine.AddTransition(GATE_OPEN, GATE_LOWERING, theTrainIsComing);
  stateMachine.AddTransition(GATE_CLOSE, GATE_WAIT, theTrainIsGone);

  // Add "timed" transitions: it will be triggered on state timeout 
  stateMachine.AddTransition(GATE_LOWERING, GATE_CLOSE);
  stateMachine.AddTransition(GATE_WAIT, GATE_RISING);
  stateMachine.AddTransition(GATE_RISING, GATE_OPEN);
}


void setup() {
  // Input/Output configuration
  pinMode(SIG_TRAIN_IN, INPUT_PULLUP);
  pinMode(SIG_TRAIN_OUT, INPUT_PULLUP);
  pinMode(PASS_NOT_ALLOWED, OUTPUT);
  pinMode(PASS_ALLOWED, OUTPUT);

  Serial.begin(115200);
  Serial.println(F("Starting State Machine...\n"));
  setupStateMachine();
  
  onEnter();    // Call first time the onEnter() function to set outputs
  Serial.print(F("Active state: "));
  Serial.println(stateMachine.ActiveStateName());

  theGate.attach(SERVO_PIN);
}

void loop() {
  theGate.write(servoPos);

  // Update State Machine
  stateMachine.Update(); 

  // Update the bool variables according to the signal inputs
  theTrainIsGone = digitalRead(SIG_TRAIN_OUT) == LOW;
  theTrainIsComing = digitalRead(SIG_TRAIN_IN) == LOW;

  if (theTrainIsComing) {
    // Reset enter time of GATE_WAIT so timeout became longer enough
    // to wait two ore more trains one after the other 
    stateMachine.SetEnteringTime(GATE_WAIT);
    if (stateMachine.GetState() == GATE_WAIT ) {
      delay(500);
      Serial.println(F("Another train is arriving, wait more time!"));
    }
  }
}
