#include "YA_FSM.h"

FSM_State*  YA_FSM::CurrentState(){
	return _currentState;
}


FSM_State*  YA_FSM::GetStateAt(uint8_t index){
	for(FSM_State* state = _firstState; state != nullptr; state = state->nextState)
		if(state->index == index )
		 	return state;
	return nullptr;
}


uint8_t YA_FSM::AddState(const char* name, uint32_t maxTime, uint32_t minTime,
				action_cb onEntering, action_cb onState, action_cb onLeaving){

	FSM_State *state = new FSM_State();
	if (_firstState == nullptr) {
		_firstState = state;
		_currentState = state;
	}
	else{
		_lastState->nextState = state;
		_stateIndex++;
	}

	_lastState = state;
	state->OnEntering = onEntering;
	state->OnLeaving = onLeaving;
	state->OnState = onState;
	state->stateName = name;
	state->maxTime = maxTime;
	state->minTime = minTime;
	state->index = _stateIndex;
	//state->actions[0] = nullptr;
	return _stateIndex;
}


uint8_t YA_FSM::AddState(const char* name, uint32_t maxTime, action_cb onEntering, action_cb onState, action_cb onLeaving){
	return AddState(name, maxTime, 0, onEntering, onState, onLeaving);
}

uint8_t YA_FSM::AddState(const char* name, action_cb onEntering, action_cb onState, action_cb onLeaving){
	return AddState(name, 0, 0, onEntering, onState, onLeaving);
}


uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition){

	FSM_Transition *transition = new FSM_Transition();
	if (_firstTransition == nullptr)
		_firstTransition = transition;
	else
		_lastTransition->nextTransition = transition;
	_lastTransition = transition;

	transition->Condition = condition;
	transition->InputState = inputState;
	transition->OutputState = outputState;

	return _currentTransitionIndex++;
}

uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, bool &condition){

	FSM_Transition *transition = new FSM_Transition();
	if (_firstTransition == nullptr)
		_firstTransition = transition;
	else
		_lastTransition->nextTransition = transition;
	_lastTransition = transition;

	transition->ConditionVar = &condition;
	transition->InputState = inputState;
	transition->OutputState = outputState;

	return _currentTransitionIndex++;
}

uint8_t	YA_FSM::AddTimedTransition(uint8_t inputState, uint8_t outputState){
	FSM_Transition *transition = new FSM_Transition();
	if (_firstTransition == nullptr)
		_firstTransition = transition;
	else
		_lastTransition->nextTransition = transition;
	_lastTransition = transition;

	transition->TimedTransition = true;
	transition->InputState = inputState;
	transition->OutputState = outputState;

	return _currentTransitionIndex++;
}


uint8_t YA_FSM::AddAction(uint8_t inputState, uint8_t type, bool &target, uint32_t _time) {

    // Create new action object and assign values
    FSM_Action *newAction = new FSM_Action();
		if (_firstAction == nullptr)
		_firstAction = newAction;
	else
		_lastAction->nextAction = newAction;
	_lastAction = newAction;

	newAction->StateIndex = inputState;
	newAction->Type = type;
	newAction->Target = &target;
	newAction->Delay = _time;

    // Set lastAction to the target state (in order to know, is one or more action are to be runned on state)
	FSM_State* state = GetStateAt(inputState);
    state->lastAction = newAction;

	return _currentActionIndex++;
}

/*
void YA_FSM::executeAction(FSM_State* state, uint8_t actIndex, bool onExit) {
    bool * target = state->actions[actIndex]->Target;

	switch(state->actions[actIndex]->Type) {
		case YA_FSM::S :
			*target = true;
			break;
		case YA_FSM::R :
			*target = false;
			break;
		case YA_FSM::N :
            if(onExit) { *target = false;  break; }

			*target = (_currentState->index == state->index);
			break;
		case YA_FSM::L :		// Time Limited action
		{
            if(onExit) {
                *target = false;
                state->actions[actIndex]->lTime = false;
				state->actions[actIndex]->lTime = -1;
                break;
            }

			if( !state->actions[actIndex]->xEdge) {
				*target = true;
				state->actions[actIndex]->lTime = millis();
                state->actions[actIndex]->xEdge = true;
			}

			if( (millis() -  state->actions[actIndex]->lTime) > state->actions[actIndex]->Delay
				&& state->actions[actIndex]->xEdge
				&& state->actions[actIndex]->lTime > 0 )
			{
				*target = false;
			}
			break;
		}
		case YA_FSM::D :		// Time Delayed action
		{
            if(onExit) {
                *target = false;
                state->actions[actIndex]->xEdge = false;
				state->actions[actIndex]->lTime = -1;
                break;
            }

			if( !state->actions[actIndex]->xEdge) {
				state->actions[actIndex]->lTime = millis();
                state->actions[actIndex]->xEdge = true;
			}

			if( (millis() -  state->actions[actIndex]->lTime) > state->actions[actIndex]->Delay
				&& state->actions[actIndex]->xEdge
				&& state->actions[actIndex]->lTime > 0 )
			{
				*target = true;
				state->actions[actIndex]->lTime = -1;		// Action executed
			}
			break;
		}

	}
}
*/

void YA_FSM::executeAction(FSM_State* state, FSM_Action* action,  bool onExit) {
	bool * target = action->Target;

	switch(action->Type) {
		case YA_FSM::S :
			*target = true;
			break;
		case YA_FSM::R :
			*target = false;
			break;
		case YA_FSM::N :
            if(onExit) { *target = false;  break; }

			*target = (_currentState->index == state->index);
			break;
		case YA_FSM::L :		// Time Limited action
		{
            if(onExit) {
                *target = false;
                action->xEdge = false;
				action->lTime = -1;
                break;
            }

			if( !action->xEdge) {
				*target = true;
				action->lTime = millis();
                action->xEdge = true;
			}

			if( (millis() - action->lTime) > action->Delay
				&& action->xEdge
				&& action->lTime > 0 )
			{
				*target = false;
			}
			break;
		}
		case YA_FSM::D :		// Time Delayed action
		{
            if(onExit) {
                *target = false;
                action->xEdge = false;
				action->lTime = -1;
                break;
            }

			if( !action->xEdge) {
				action->lTime = millis();
                action->xEdge = true;
			}

			if( (millis() - action->lTime) > action->Delay
				&&action->xEdge
				&& action->lTime > 0 )
			{
				*target = true;
				action->lTime = -1;		// Action executed
			}
			break;
		}

	}
}


bool YA_FSM::Update(){

	for(FSM_Transition* actualtr = _firstTransition; actualtr != nullptr; actualtr = actualtr->nextTransition) {
		if(actualtr->InputState == _currentState->index){

			// Check if state is on timeout
			if(_currentState->maxTime > 0){
				uint32_t milss = millis() - _currentState->enterTime;
				if( milss  > _currentState->maxTime)	{
					_currentState->timeout = true;
				}
			}

			bool _trigger = false;
			if (actualtr->TimedTransition == true)
				_trigger = _currentState->timeout;
			else if(actualtr->Condition == nullptr)
				_trigger = *(actualtr->ConditionVar);
			else
				_trigger = actualtr->Condition();

			if (_trigger){
				// Check if state is on at least from minTime
				if(_currentState->minTime > 0){
					uint32_t mils = millis() - _currentState->enterTime;
					if( mils  < _currentState->minTime)	{
						return false;
					}
				}

				// One of the transitions has triggered, set the new state
				if(_currentState->OnLeaving != nullptr)
					_currentState->OnLeaving();

                // Call the actions on exit previuos state to clear target if necessary
				if( _currentState->lastAction != nullptr) {
					for(FSM_Action* actualAct = _firstAction; actualAct != nullptr; actualAct = actualAct->nextAction) {
						if(actualAct->StateIndex == _currentState->index){
							executeAction(_currentState, actualAct, true);
						}
					}
				}

				FSM_State* targetState = GetStateAt(actualtr->OutputState);
				_currentState = targetState;

				_currentState->enterTime = millis();
				_currentState->timeout = false;

				if(_currentState->OnEntering != nullptr)
					_currentState->OnEntering();
				return true;
			}
		}
	}
	if(_currentState->OnState != nullptr)
			_currentState->OnState();

	// Run actions (if defined) for current state
	if( _currentState->lastAction != nullptr) {
		for(FSM_Action* actualAct = _firstAction; actualAct != nullptr; actualAct = actualAct->nextAction) {
			if(actualAct->StateIndex == _currentState->index){
				executeAction(_currentState, actualAct, false);
			}
		}
	}
	return false;
}


uint8_t YA_FSM::StateIndex() const{
	return _currentState->index;
}

// Return the current active state
uint8_t YA_FSM::GetState() const{
	return _currentState->index;
}

// Change to State
void YA_FSM::SetState(uint8_t index, bool callOnEntering, bool callOnLeaving) {
	FSM_State* newState = GetStateAt(index);
	// If found state at index
	if(newState != nullptr ) {
		// Guarantee that will run OnLeaving()
		if(_currentState->OnLeaving != nullptr && callOnLeaving)
			_currentState->OnLeaving();

		_currentState = newState;

		// Guarantee that will run OnEntering()
		if(_currentState->OnEntering != nullptr && callOnEntering)
			_currentState->OnEntering();

		// Update Enter Time
		_currentState->enterTime = millis();
		_currentState->timeout = false;
	}
}

void YA_FSM::SetTimeout(uint8_t index, uint32_t preset) {
	FSM_State* state = GetStateAt(index);
	if(state != nullptr ){
		state->maxTime = preset;
	}
}

// Return true if timeout
bool YA_FSM::GetTimeout(uint8_t index){
	FSM_State* state = GetStateAt(index);
	if(state != nullptr ){
		return state->timeout;
	}
	return false;
}

bool YA_FSM::Timeout(uint8_t index){
	FSM_State* state = GetStateAt(index);
	if(state != nullptr ){
		return state->timeout;
	}
	return false;
}


// Return current state entering time
uint32_t YA_FSM::GetEnteringTime(uint8_t index) {
	FSM_State* state = GetStateAt(index);
	if(state != nullptr ){
		return state->enterTime;
	}
	return 0;
}

int32_t YA_FSM::SetEnteringTime(uint8_t index) {
	FSM_State* state = GetStateAt(index);
	if(state != nullptr ){
		state->enterTime = millis();
		return state->enterTime;
	}
	else
		return -1;
}

void YA_FSM::initVariables(){
	for(uint8_t i=0; i<_numStates; i++){
		AddState("", 0, nullptr, nullptr, nullptr);
	}
}



//////////////////////////////////////////////////////////////////////////////////

void YA_FSM::SetOnState(uint8_t index, action_cb action, uint32_t setTimeout ){
	FSM_State* state = GetStateAt(index);
	state->maxTime = setTimeout;
	state->OnState = action;
}

void YA_FSM::SetOnEntering(uint8_t index, action_cb action){
	FSM_State* state = GetStateAt(index);
	state->OnEntering = action;
}

void YA_FSM::SetOnLeaving(uint8_t index, action_cb action){
	FSM_State* state = GetStateAt(index);
	state->OnLeaving = action;
}

void YA_FSM::ClearOnState(uint8_t index){
	FSM_State* state = GetStateAt(index);
	state->OnState = nullptr;
}

void YA_FSM::ClearOnEntering(uint8_t index){
	FSM_State* state = GetStateAt(index);
	state->OnEntering = nullptr;
}

void YA_FSM::ClearOnLeaving(uint8_t index){
	FSM_State* state = GetStateAt(index);
	state->OnLeaving = nullptr;
}