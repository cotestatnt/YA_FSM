
/*
  Cotesta Tolentino, 2020.  
  Released into the public domain.
*/
#ifndef YA_FSM_H
#define YA_FSM_H
#include "Arduino.h"
//#include <functional>

class YA_FSM
{
	//using action_cb = std::function<void()>;
	//using condition_cb = std::function<bool()>;

	typedef bool(*condition_cb)();
	typedef void(*action_cb)();

	struct State {		
		uint8_t 	index = 0;
		bool 		timeout = false;
		uint32_t 	setTimeout = 0;	// 0 -> No timeout
		uint32_t 	enterTime;		
		action_cb 	OnEntering;
		action_cb 	OnLeaving;
		action_cb 	OnState;			
		const char 	*stateName;
		State		*nextState;
	} ;

	struct Transition{
		uint8_t 		InputState;
		uint8_t 		OutputState;
		condition_cb 	Condition;
		bool   			*ConditionVar;
		Transition 		*nextTransition;
	} ;
	
public:
	// Default constructor
	YA_FSM() {};
	
	// For compatibility with old version 
	explicit YA_FSM (uint8_t st, uint8_t tr) 
	: _numStates(st), _numTransitions(tr)  {
		initVariables();	
	};
	
	uint8_t 	AddState(const char* name, uint32_t setTimeout,
				 	action_cb onEntering, action_cb onState, action_cb onLeaving);

	uint8_t 	AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition);	
	uint8_t 	AddTransition(uint8_t inputState, uint8_t outputState, bool &condition);
	uint8_t 	GetState() const;
	uint32_t 	GetEnteringTime(uint8_t index);
	void 		SetTimeout(uint8_t index, uint32_t preset); 
	bool 		GetTimeout(uint8_t index);
	bool 		Update();	
	YA_FSM::State*	CurrentState();
	
	// only for compatibility with old version    
	void 	SetOnEntering(uint8_t index, action_cb action);
	void 	SetOnLeaving(uint8_t index, action_cb action);
	void 	SetOnState(uint8_t index, action_cb action, uint32_t setTimeout = 0);	
	void 	ClearOnEntering(uint8_t index);
	void 	ClearOnLeaving(uint8_t index);
	void 	ClearOnState(uint8_t index);
	///////////////////////////////////////////////////////
	
private:
	uint8_t 		_stateIndex;
	State 			*_oldState = nullptr;
	State 			*_currentState = nullptr;
	State 			*_firstState = nullptr;
	State 			*_lastState = nullptr;

	uint8_t 		_currentTransitionIndex;	
	Transition 		*_firstTransition = nullptr;
	Transition 		*_lastTransition = nullptr;

	YA_FSM::State*  GetStatePt(uint8_t index);	
	void 			SetState(uint8_t index);

	// only for compatibility with old version
	void 			SetState(uint8_t index, bool launchEntering); 
	void 			initVariables();
	uint8_t 		_numStates;
	uint8_t 		_numTransitions;		
	///////////////////////////////////////////
};



#endif

