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
	state->actions[0] = nullptr;
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


uint8_t YA_FSM::AddAction(uint8_t inputState, uint8_t type, bool &target, uint32_t _time) {

    // Create new action object and assign values
    FSM_Action *newAction = new FSM_Action();
	newAction->Type = type;
	newAction->Target = &target;
	newAction->Delay = _time;

    // Get the target state
	FSM_State* state = GetStateAt(inputState);

    state->actions[state->lastActionIndex++] = newAction;

    // // If first action in list
    // if (state->action == nullptr) {
	// 	state->action = newAction;
    //     state->action->nextAction = nullptr;
    // }
	// else {
    //     //state->action->nextAction = action;        // Skip already defined action for this state
    //     volatile FSM_Action* _action = state->action->nextAction;
    //     while ( _action != nullptr) {
    //         _action = _action->nextAction;
    //     };
    //     _action = newAction;
    // }
	return 0;
}


void YA_FSM::executeAction(FSM_State* state, uint8_t actIndex, bool onExit) {
	static uint32_t actionTime;

    bool * target = state->actions[actIndex]->Target;
    uint8_t actType = state->actions[actIndex]->Type;


	switch(actType) {
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
		case YA_FSM::L : {
            static uint32_t actTime = state->actions[actIndex]->Delay;
            static bool edge = false;

            if(onExit) {
                *target = false;
                edge = false;
                break;
            }

			if( !edge) {
				*target = true;
				actionTime = millis();
                edge = true;
			}

			if(millis() - actionTime > actTime && edge) {
				*target = false;
			}
			break;
		}
		case YA_FSM::D : {
            static uint32_t actTime = state->actions[actIndex]->Delay;
            static bool edge = false;
            if(onExit) {
                *target = false;
                edge = false;
                break;
            }

			if( !edge) {
				actionTime = millis();
                edge = true;
			}

			if(millis() - actionTime > actTime && edge) {
				*target = true;
                edge = false;
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
			if(actualtr->Condition == nullptr)
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
                for(uint8_t i =0; i < _currentState->lastActionIndex; i++) {
                    executeAction(_currentState, i, true);
                }
                // for(FSM_Action* _action = _currentState->action; _action != nullptr; _action = _action->nextAction) {
                //     executeAction(_currentState, _action, true);
                // }

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

	// if(_currentState->action != nullptr) {
    //     // Run all defined actions for current state
    //     for(FSM_Action* _action = _currentState->action; _action != nullptr; _action = _action->nextAction) {
    //         executeAction(_currentState, _action);
    //     }
	// }

    for(uint8_t i =0; i < _currentState->lastActionIndex; i++) {
        executeAction(_currentState, i);
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