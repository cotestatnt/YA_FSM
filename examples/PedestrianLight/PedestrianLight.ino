#include <YA_FSM.h>

const byte BTN_CALL = 2;
const byte GREEN_LED = 13;
const byte YELLOW_LED = 12;
const byte RED_LED = 11;

// STM32
// const byte BTN_CALL = PB6;
// const byte GREEN_LED = PA5;
// const byte YELLOW_LED = PA6;
// const byte RED_LED = PA7;

bool green[] = {1, 0, 0 };
bool yellow[] = {0, 1, 0 };
bool red[] = {0, 0, 1 };

// Create new FSM
YA_FSM stateMachine;

// Input Alias. We use Input as trigging condition in defined transition
enum Input {StartCall, StartGreen, StartYellow, StartRed};

// State Alias
enum State {RED, GREEN, YELLOW, CALL};

// Helper for print labels instead integer when state change
const char* stateName[] = { "RED", "GREEN", "YELLOW", "CALL"};

// Stores last user input and the current active state
Input input;
bool callButton = false;

// Pedestrian traffic light -> green ligth ON until button pressed
// #define GREEN_TIME  20000    // always ON
#define YELLOW_TIME  5000     // 5s
#define RED_TIME     10000    // 10s
#define CALL_DELAY   4000     // 4s


void setup() {
  pinMode(BTN_CALL, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(BTN_CALL), isrButtonCall, FALLING);
  
  Serial.begin(115200);
  Serial.println(F("Starting the Finite State Machine...\n"));
  setupStateMachine();

  // Initial state
  Serial.print(F("Active state: "));
  Serial.println(stateName[stateMachine.GetState()]);
}

void loop() {
  // Update State Machine (true is state changed)
  if(stateMachine.Update()){
    Serial.print(F("Active state: "));    
    Serial.println(stateMachine.GetName());
  }
}

// Check the Call button with an Interrupt Service Routine
void isrButtonCall(){
  if(stateMachine.GetState() == GREEN) {
   Serial.println(F("CALL button pressed"));
   input = StartCall;
  }
}

void setLight(bool light[]){
  digitalWrite(GREEN_LED, light[0]);
  digitalWrite(YELLOW_LED, light[1]);
  digitalWrite(RED_LED, light[2]);
}


/////////// STATE MACHINE CALLBACK FUNCTIONS //////////////////
void onStateGreen() {
  setLight(green);
}

void onStateYellow() {
  setLight(yellow);
  // Check if current state has timeouted, if yes go to next  
  if( stateMachine.GetTimeout(stateMachine.GetState()) ) {
    input = StartRed;
  }    
}

void onStateRed() {
  setLight(red);
  // Check if current state has timeouted, if yes go to next
  if( stateMachine.GetTimeout(stateMachine.GetState()) ) {
    input = StartGreen;
  } 
}

void onStateCall() {
  // Check if current state has timeouted, if yes go to next
  if( stateMachine.GetTimeout( stateMachine.GetState() ) ) {
    input = StartYellow;
  }
}

// Define "on entering" callback functions
void onEnterGreen(){ Serial.println(F("Green light ON")); }
void onEnterYellow(){ Serial.println(F("Yellow light ON")); }
void onEnterRed(){ Serial.println(F("Red light ON")); }

// Define "on leaving" callback functions
void onExitGreen(){ Serial.println(F("Green light OFF\n")); }
void onExitYellow(){ Serial.println(F("Yellow light OFF\n")); }
void onExitRed(){ Serial.println(F("Red light OFF\n")); }


// Setup the State Machine
void setupStateMachine() {

  // Follow the order of defined enumeration for the state definition (will be used as index)
  // Add States => name,timeout, onEnter cb, onState cb, onLeave cb
  stateMachine.AddState(stateName[RED], RED_TIME, onEnterRed, onStateRed, onExitRed);  
  stateMachine.AddState(stateName[GREEN], 0, onEnterGreen, onStateGreen, onExitGreen);    
  stateMachine.AddState(stateName[YELLOW], YELLOW_TIME, onEnterYellow, onStateYellow, onExitYellow);  
  stateMachine.AddState(stateName[CALL], CALL_DELAY, nullptr, onStateCall, nullptr);  
  
  // Add transitions with related trigger input callback functions  
  stateMachine.AddTransition(RED, GREEN, [](){return input == Input::StartGreen;} );
  stateMachine.AddTransition(YELLOW, RED, [](){return input == Input::StartRed;}  );
  stateMachine.AddTransition(CALL, YELLOW, [](){return input == Input::StartYellow;});
  stateMachine.AddTransition(GREEN, CALL, [](){return input == Input::StartCall;});
}
