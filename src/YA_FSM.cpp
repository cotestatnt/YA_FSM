#include "YA_FSM.h"


YA_FSM::YA_FSM(uint8_t numStates, uint8_t numTransitions)
{
	_numStates = numStates;
	_numTransitions = numTransitions;

	_states = new State[numStates];
	_transitions = new Transition[numTransitions];

	initVariables();
}



void YA_FSM::SetState(uint8_t state, bool launchLeaving, bool launchEntering)
{
	if (launchLeaving && _states[_currentStateIndex].OnLeaving != nullptr) _states[_currentStateIndex].OnLeaving();
	if (launchEntering && _states[state].OnEntering != nullptr) _states[state].OnEntering();
	
	_currentStateIndex = state;
}



uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition)
{
	if (_currentTransitionIndex >= _numTransitions) 
		return _currentTransitionIndex;
		
	SetTransition(_currentTransitionIndex, inputState, outputState, condition);

	return _currentTransitionIndex++;
}


uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, bool &condition)
{
	if (_currentTransitionIndex >= _numTransitions) 
		return _currentTransitionIndex;
		
	SetTransition(_currentTransitionIndex, inputState, outputState, condition);

	return _currentTransitionIndex++;
}

void YA_FSM::SetTransition(uint8_t transition, uint8_t inputState, uint8_t outputState, condition_cb condition)
{
	_transitions[transition].InputState = inputState;
	_transitions[transition].OutputState = outputState;
	_transitions[transition].Condition = condition;
}


void YA_FSM::SetTransition(uint8_t transition, uint8_t inputState, uint8_t outputState, bool &condition)
{
	_transitions[transition].InputState = inputState;
	_transitions[transition].OutputState = outputState;
	_transitions[transition].ConditionVar = &condition;
}


void YA_FSM::SetOnState(uint8_t state, action_cb action, uint32_t setTimeout )
{
	_states[state].setTimeout = setTimeout;
	_states[state].OnState = action;
}

void YA_FSM::SetOnEntering(uint8_t state, action_cb action)
{
	_states[state].OnEntering = action;
}

void YA_FSM::SetOnLeaving(uint8_t state, action_cb action)
{
	_states[state].OnLeaving = action;
}

void YA_FSM::ClearOnState(uint8_t state)
{
	_states[state].OnState = nullptr;
}

void YA_FSM::ClearOnEntering(uint8_t state)
{
	_states[state].OnEntering = nullptr;
}

void YA_FSM::ClearOnLeaving(uint8_t state)
{
	_states[state].OnLeaving = nullptr;
}



// Return the current active state
uint8_t YA_FSM::GetState() const
{
	return _currentStateIndex;
}

// Return true if timeout
void YA_FSM::SetTimeout(uint8_t state, uint32_t preset) 
{
	_states[state].setTimeout = preset;
}

// Return true if timeout
bool YA_FSM::GetTimeout(uint8_t state) 
{
	return _states[state].timeout;
}

 
// Return current state entering time 
uint32_t YA_FSM::GetEnteringTime(uint8_t state) 
{
	return _states[state].enterTime;
}

bool YA_FSM::Update()
{
	bool stateChanged = false;

	// check if transition condition is true
	for (int transitionIndex = 0; transitionIndex < _numTransitions; transitionIndex++)
	{
		if (_transitions[transitionIndex].InputState == _currentStateIndex)
		{
			bool _trigger = false;
			if (_transitions[transitionIndex].Condition == nullptr)
				_trigger = *(_transitions[transitionIndex].ConditionVar);
			else
				_trigger = _transitions[transitionIndex].Condition();
			
			if (_trigger)
			{
				// One of the transitions has triggered, set the new state
				SetState(_transitions[transitionIndex].OutputState, true, true);				
				_states[_currentStateIndex].enterTime = millis();
				_states[_currentStateIndex].timeout = false;
				stateChanged = true;
			}
		}

		// Run the OnState callback function (if defined)
		_states[_currentStateIndex].OnState();

		// Check if current state timeout is to be setted
		if(_states[_currentStateIndex].setTimeout > 0)
		{
			if( millis() - _states[_currentStateIndex].enterTime  > _states[_currentStateIndex].setTimeout)
			{
				_states[_currentStateIndex].timeout = true;
			}
		}
		
	}
	
	

	return stateChanged;
}

void YA_FSM::initVariables()
{
	_currentStateIndex = 0;
	_currentTransitionIndex = 0;
	for (int stateIndex = 0; stateIndex < _numStates; stateIndex++)
	{
		_states[stateIndex].OnEntering = nullptr;
		_states[stateIndex].OnLeaving = nullptr;
		_states[stateIndex].OnState = nullptr;
		_states[stateIndex].enterTime = 0;
		_states[stateIndex].setTimeout = 0;
		_states[stateIndex].timeout = false;
	}

	for (int transitionIndex = 0; transitionIndex  < _numTransitions; transitionIndex++)
	{
		_transitions[transitionIndex].Condition = nullptr;
		_transitions[transitionIndex].ConditionVar = nullptr;
	}
}