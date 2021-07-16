
# YA_FSM Library - YET ANOTHER FINITE STATE MACHINE

The YA_FSM library implements a **Finite State Machine** with pre-defined states and transitions associated to callback functions.

This library try to reproduce the type of automation you can define with a SFC/Grapcet model. In the example folders you can find an image where the inspiring FSM model is represented with an SFC diagram 

#### TO-DO: 
Add method to handle **actions** defined for each state, at the moment demanded to callback functions

___
### Introduction

States represent the different situations in which the machine can be at any time. The transitions connect two states, one at the input and others at the output, and are associated with a trigger condition which carries out the change of state. Triggering can be performed with a bool function() or a bool variable. Also the timeout of state itself (it is a bool) can be used for triggering to next state.
If you take a look at the examples included, you will find some different way for triggering a transition.

To update the states, you must call the Update() function in your loop(), which checks for transitions that have the current state as input and associated conditions.

If any of the transitions associated with the current state satisfy the trigger condition, the machine goes into the next state  defined in transition property.

![SFC_esempio](https://user-images.githubusercontent.com/27758688/125982036-0eab0bb2-ed13-4101-af5c-6e49e82908fd.png)


Each of the states of the machine can be associated with a callback function that will be executed when the state is activated (on entering), when it is left (on leaving) and while it is running (on state). For each status it is also possible to define a maximum duration time, at the end of which a timeout bit will be setted and can be tested with Timeout() or directly from the actual state struct `FSM_State`. Also a minimum duration time can be setted for each state.

To configure the machine according to your needs, define the states (better if you create enumerations for states and for triggers in order to make the usage and layout of FSM clearer) and the configure correctly the transitions between each state. 
In the main loop call update() metod and that's it.

Take a look at the examples provided in the [examples folder](https://github.com/cotestatnt/YA_FSM/tree/master/examples).
Start from the simplest Blinky https://github.com/cotestatnt/YA_FSM/blob/master/examples/Blinky/Blinky.ino

or a more advanced like classic algorithm for opening an automatic gate (simplified)
https://github.com/cotestatnt/YA_FSM/blob/master/examples/AutomaticGate/AutomaticGate.ino


### Constructor

```c++
YA_FSM();

------- OLD VERSION  -------
YA_FSM (uint8_t states, uint8_t transitions);
---------------------------
```
### General methods
```c++

// Get current state index
uint8_t GetState() const;

// Get pointer to current state object
FSM_State* CurrentState();

// Get pointer to a specific state object passing the index
FSM_State*  GetStateAt(uint8_t index);

// Get active state name
const char* ActiveStateName();

// Set or modify a state timeout (preset = milliseconds)
void SetTimeout(uint8_t index, uint32_t preset);

// Check if a state is timeouted
bool GetTimeout(uint8_t index);		// deprecated
bool Timeout(uint8_t index);		// More clear method name
stateMachine.CurrentState()->timeout	// Access directly to the value stored in FSM_State struct

// Get the time (milliseconds) when state was activated
uint32_t GetEnteringTime(uint8_t index) 
	
// Update state machine. Run in loop()
bool Update();

// Set up a transition and trigger input callback function (or variable)
void AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition);
void AddTransition(uint8_t inputState, uint8_t outputState, bool condition);

// Set up a state with properties and callback function
uint8_t AddState(const char* name, uint32_t maxTime, uint32_t minTime,	action_cb onEntering, action_cb onState, action_cb onLeaving);
uint8_t AddState(const char* name, uint32_t maxTime, action_cb onEntering, action_cb onState, action_cb onLeaving);
	
------- OLD VERSION  -------
// Configure input and output and run actions of a state
void SetOnEntering(uint8_t index, action_cb action);
void SetOnLeaving(uint8_t index, action_cb action);
void SetOnState(uint8_t index, action_cb action, uint32_t setTimeout = 0)   // 0 disabled

void ClearOnEntering(uint8_t index);
void ClearOnLeaving(uint8_t index);
void ClearOnState(uint8_t index);
```

### Supported boards
The library works virtually with every boards supported by Arduino framework (no hardware dependency)

+ 1.0.4 Examples simplified, bug fixes
+ 1.0.3 Added ActiveStateName() method and updated all examples with new style (addTransition() and addStep() )
+ 1.0.2 Bug fix
+ 1.0.1 Dinamic memory allocation for states and transitions
+ 1.0.0 Initial version
