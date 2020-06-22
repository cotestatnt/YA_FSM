
# YA_FSM Library - YET ANOTHER FINITE STATE MACHINE

The YA_FSM library implements a Petri net inspired state machine with defined states and transitions activated with callback functions.


This library is mainly inpired from https://github.com/luisllamasbinaburo/Arduino-StateMachine
___
### Introduction

The state machine is initialized setting the total number of states and transitions. States represent the different situations in which the machine can be at any time. The transitions connect two states, one at the input and others at the output, and are associated with a trigger condition which carries out the change of state. Triggering can be performed with a bool function() or a global bool variable(check the examples included).

To update the states, you must frequently call the Update() function in your loop(), which checks for transitions that have the current state as input and associated conditions.

If any of the transitions associated with the current state satisfy the trigger condition, the machine goes into the next state  defined in transition property.

![SFC example](/SFC_esempio.png)

Each of the states of the machine can be associated with a callback function that will be executed when the state is activated (on entering), when it is left (on leaving) and while it is running (on state). For each status it is also possible to define a maximum duration time, at the end of which a timeout bit will be setted and can be tested with GetTimeout().

To configure the machine according to your needs, define the states (better if you create enumerations for states and for triggers in order to make the usage and layout of FSM clearer) and the configure correctly the transitions between each state. 
In the main loop call update() metod and that's it.

Take a look at the examples provided in the [examples folder](https://github.com/cotestatnt/YA_FSM/tree/master/examples).
Start from the simplest Blinky https://github.com/cotestatnt/YA_FSM/blob/master/examples/Blinky/Blinky.ino

or a more advanced like classic algorithm for opening an automatic gate (simplified)
https://github.com/cotestatnt/YA_FSM/blob/master/examples/AutomaticGate/AutomaticGate.ino




### Constructor

```c++
YA_FSM(uint8_t numStates, uint8_t numTransitions);
```
### General methods
```c++
// Set up a transition
void SetTransition(uint8_t transition, uint8_t inputState, uint8_t outputState, condition_cb condition);
void SetTransition(uint8_t transition, uint8_t inputState, uint8_t outputState, bool condition);

// Disable a transition
void RemoveTransition(uint8_t transition);
	
// Configure input and output and run actions of a state
void SetOnEntering(uint8_t state, action_cb action);
void SetOnLeaving(uint8_t state, action_cb action);
void SetOnState(uint8_t state, action_cb action, uint32_t setTimeout = 0)   // 0 disabled

void ClearOnEntering(uint8_t state);
void ClearOnLeaving(uint8_t state);
void ClearOnState(uint8_t state)
	
// Get current state
uint8_t GetState() const;

// Set or modify a state timeout (preset = milliseconds)
void SetTimeout(uint8_t state, uint32_t preset);

// Check if a state is timeouted
bool GetTimeout(uint8_t state);

// Get the time (milliseconds) when state was activated
uint32_t GetEnteringTime(uint8_t state) 
	
// Update state machine
bool Update();
```

### Supported boards
The library works virtually with every boards supported by Arduino framework (no hardware dependency)




+ 1.0.0 Initial version
