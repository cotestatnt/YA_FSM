#include "YA_FSM.h"


YA_FSM::State*  YA_FSM::CurrentState(){	
	return _currentState;
}



YA_FSM::State*  YA_FSM::GetStatePt(uint8_t index){
	for(State* state = _firstState; state != nullptr; state = state->nextState) 
		if(state->index == index )
		 	return state;
	return nullptr;
}


uint8_t YA_FSM::AddState(const char* name, uint32_t timeout,
				action_cb onEntering, action_cb onState, action_cb onLeaving){

	State *state = new State();
	if (_firstState == nullptr) {
		_firstState = state;
		_currentState = state;
	}
	else
		_lastState->nextState = state;
	_lastState = state;
	
	state->OnEntering = onEntering;
	state->OnLeaving = onLeaving;
	state->OnState = onState;
	state->stateName = name;
	state->setTimeout = timeout;
	state->index = _stateIndex;

	return _stateIndex++;
}	



void YA_FSM::SetState(uint8_t index, bool launchEntering){
	State* state = GetStatePt(index);
	if(state != nullptr){
		if(launchEntering) 
			state->OnEntering();			
		else	
			_oldState->OnLeaving();

		_currentState = state;
	}	
}



uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition){
	
	Transition *transition = new Transition();
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
	
	Transition *transition = new Transition();
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


void YA_FSM::SetOnState(uint8_t index, action_cb action, uint32_t setTimeout ){
	State* state = GetStatePt(index);
	state->setTimeout = setTimeout;
	state->OnState = action;
}

void YA_FSM::SetOnEntering(uint8_t index, action_cb action){
	State* state = GetStatePt(index);
	state->OnEntering = action;
}

void YA_FSM::SetOnLeaving(uint8_t index, action_cb action){
	State* state = GetStatePt(index);
	state->OnLeaving = action;
}

void YA_FSM::ClearOnState(uint8_t index){
	State* state = GetStatePt(index);
	state->OnState = nullptr;
}

void YA_FSM::ClearOnEntering(uint8_t index){
	State* state = GetStatePt(index);
	state->OnEntering = nullptr;
}

void YA_FSM::ClearOnLeaving(uint8_t index){
	State* state = GetStatePt(index);
	state->OnLeaving = nullptr;
}



// Return the current active state
uint8_t YA_FSM::GetState() const{
	return _currentState->index;
}

// Return true if timeout
void YA_FSM::SetTimeout(uint8_t index, uint32_t preset) {
	State* state = GetStatePt(index);
	if(state != nullptr ){
		state->setTimeout = preset;
	}
}

// Return true if timeout
bool YA_FSM::GetTimeout(uint8_t index){
	State* state = GetStatePt(index);
	if(state != nullptr ){
		return state->setTimeout;		
	}
	return false;
}

 
// Return current state entering time 
uint32_t YA_FSM::GetEnteringTime(uint8_t index) {
	State* state = GetStatePt(index);
	if(state != nullptr ){
		return state->enterTime;	
	}
	return 0;	
}

bool YA_FSM::Update(){
	bool stateChanged = false;

	for(Transition* actualtr = _firstTransition; actualtr != nullptr; actualtr = actualtr->nextTransition) {
		if(actualtr->InputState == _currentState->index){
			bool _trigger = false;
			if(actualtr->Condition == nullptr)			
				_trigger = *(actualtr->ConditionVar);
			else
				_trigger = actualtr->Condition();
			
			if (_trigger){
				// One of the transitions has triggered, set the new state
				//State* targetState = GetStatePt(actualtr->OutputState);
				
				if(_currentState != _oldState){
					// Enter on state
					SetState(actualtr->OutputState, true);	
					_oldState = _currentState;
				}
				else{
					// Exit from state
					SetState(actualtr->OutputState, false);	
				}

				_currentState->enterTime = millis();
				_currentState->timeout = false;
				stateChanged = true;
			}
		}

		if(_currentState->OnState != nullptr)
			_currentState->OnState();

		if(_currentState->setTimeout > 0){
			if( millis() - _currentState->enterTime  > _currentState->setTimeout)	{
				_currentState->timeout = true;
			}
		}		
	}
	return stateChanged;
}



void YA_FSM::initVariables(){	
	for(uint8_t i=0; i<_numStates; i++){
		AddState("", 0, nullptr, nullptr, nullptr);
	}
}