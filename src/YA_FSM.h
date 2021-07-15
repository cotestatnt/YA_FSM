
/*
  Cotesta Tolentino, 2020.  
  Released into the public domain.
*/
#ifndef YA_FSM_H
#define YA_FSM_H
#include "Arduino.h"
//#include <functional>


typedef bool(*condition_cb)();
typedef void(*action_cb)();

struct FSM_State {		
	uint8_t 	index = 0;
	bool 		timeout = false;
	uint32_t 	maxTime = 0;	// 0 -> No timeout
	uint32_t 	minTime = 0;	// 0 -> No min time
	uint32_t 	enterTime;		
	action_cb 	OnEntering;
	action_cb 	OnLeaving;
	action_cb 	OnState;			
	const char 	*stateName;
	FSM_State	*nextState;
} ;

struct FSM_Transition{
	uint8_t 		InputState;
	uint8_t 		OutputState;
	condition_cb 	Condition;
	bool   			*ConditionVar;
	FSM_Transition	*nextTransition;
} ;


class YA_FSM
{
	//using action_cb = std::function<void()>;
	//using condition_cb = std::function<bool()>;

	
	
public:
	// Default constructor
	YA_FSM() {};
	
	// For compatibility with old version 
	explicit YA_FSM (uint8_t st, uint8_t tr) 
	: _numStates(st), _numTransitions(tr)  {
		initVariables();	
	};
	
	uint8_t 	AddState(const char* name, uint32_t maxTime, uint32_t minTime,
				 	action_cb onEntering, action_cb onState, action_cb onLeaving);
	uint8_t 	AddState(const char* name, uint32_t maxTime,
				 	action_cb onEntering, action_cb onState, action_cb onLeaving);

	uint8_t 	AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition);	
	uint8_t 	AddTransition(uint8_t inputState, uint8_t outputState, bool &condition);
	uint8_t 	GetState() const;
	uint8_t 	StateIndex() const;
	uint32_t 	GetEnteringTime(uint8_t index);
	void 		SetTimeout(uint8_t index, uint32_t preset); 
	
	[[deprecated("Replaced by Timeout()")]]
	bool 		GetTimeout(uint8_t index);
	bool 		Timeout(uint8_t index);
	bool 		Update();	
	FSM_State*	CurrentState();
	FSM_State*  GetStateAt(uint8_t index);	
	
	inline const char* ActiveStateName() {
		return _currentState->stateName;
	}
	
	// only for compatibility with old version    
	[[deprecated("Replaced by all in once method AddState()")]]
	void 	SetOnEntering(uint8_t index, action_cb action);
	[[deprecated("Replaced by Timeout()")]]
	void 	SetOnLeaving(uint8_t index, action_cb action);
	[[deprecated("Replaced by Timeout()")]]
	void 	SetOnState(uint8_t index, action_cb action, uint32_t setTimeout = 0);	
	[[deprecated("Replaced by all in once method AddState()")]]
	void 	ClearOnEntering(uint8_t index);
	[[deprecated("Replaced by all in once method AddState()")]]
	void 	ClearOnLeaving(uint8_t index);
	[[deprecated("Replaced by all in once method AddState()")]]
	void 	ClearOnState(uint8_t index);
	///////////////////////////////////////////////////////
	
private:
	uint8_t 		_stateIndex;
	FSM_State 		*_oldState = nullptr;
	FSM_State 		*_currentState = nullptr;
	FSM_State 		*_firstState = nullptr;
	FSM_State 		*_lastState = nullptr;

	uint8_t 		_currentTransitionIndex;	
	FSM_Transition	*_firstTransition = nullptr;
	FSM_Transition	*_lastTransition = nullptr;

	// only for compatibility with old version
	void 			initVariables();
	uint8_t 		_numStates;
	uint8_t 		_numTransitions;		
	///////////////////////////////////////////
};



#endif
