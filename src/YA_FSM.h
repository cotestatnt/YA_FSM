
/*
  Cotesta Tolentino, 2020.  
  Released into the public domain.
*/

#ifndef YA_FSM_H
#define YA_FSM_H
#include "Arduino.h"

class YA_FSM
{
	typedef bool(*condition_cb)();
	typedef void(*action_cb)();

	typedef struct {
		action_cb 	OnEntering;
		action_cb 	OnLeaving;
		action_cb 	OnState;
		uint32_t 	enterTime;		
		uint32_t 	setTimeout;			// 0 -> No timeout
		bool 		timeout = false;
	} State;

	typedef struct {
		uint8_t 		InputState;
		uint8_t 		OutputState;
		condition_cb 	Condition;
		bool   			*ConditionVar;
	} Transition;
	
public:
	YA_FSM(uint8_t numStates, uint8_t numTransitions);
	uint8_t AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition);	
	uint8_t AddTransition(uint8_t inputState, uint8_t outputState, bool &condition);	
	
	void SetOnEntering(uint8_t state, action_cb action);
	void SetOnLeaving(uint8_t state, action_cb action);
	void SetOnState(uint8_t state, action_cb action, uint32_t setTimeout = 0);
	
	void ClearOnEntering(uint8_t state);
	void ClearOnLeaving(uint8_t state);
	void ClearOnState(uint8_t state);
	
	uint8_t GetState() const;
	uint32_t GetEnteringTime(uint8_t _state);
	void SetTimeout(uint8_t state, uint32_t preset);
	bool GetTimeout(uint8_t _state);
	bool Update();
	
private:
	State *_states;
	uint8_t _numStates;
	uint8_t _currentStateIndex;


	Transition *_transitions;
	uint8_t _numTransitions;
	uint8_t _currentTransitionIndex;

	void SetState(uint8_t state, bool launchLeaving, bool launchEntering);
	void SetTransition(uint8_t transition, uint8_t inputState, uint8_t outputState, condition_cb condition);
	void SetTransition(uint8_t transition, uint8_t inputState, uint8_t outputState, bool &condition);

	void initVariables();
};
#endif

