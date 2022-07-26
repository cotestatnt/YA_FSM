
/*
	Cotesta Tolentino, 2020.
	Released into the public domain.
*/
#ifndef YA_FSM_H
#define YA_FSM_H
#include "Arduino.h"

// typedef bool(*condition_cb)();
// typedef void(*action_cb)();

using action_cb = void (*)();
using condition_cb = bool (*)();

struct FSM_Action
{
	bool xEdge = false;
	uint8_t Type;		    // The type of action  { 'N', 'S', 'R', 'L', 'D'};
	uint8_t StateIndex;     // Action valid for state defined
	int32_t lTime = -1;     // Last call time of Action (-1 not called)
	uint32_t Delay;			// For L - limited time and D - delayed actions
	bool *Target;			// The variable wich is affected by action
	FSM_Action *nextAction = nullptr;
};

struct FSM_Transition
{
	uint8_t InputState;
	uint8_t OutputState;
	condition_cb Condition;
	bool *ConditionVar = nullptr;
	FSM_Transition *nextTransition = nullptr;
};

struct FSM_State
{
	uint8_t index = 0;
	bool timeout = false;
	uint32_t maxTime = 0;   // 0 -> No timeout
	uint32_t minTime = 0;   // 0 -> No min time
	uint32_t enterTime;
	action_cb OnEntering;
	action_cb OnLeaving;
	action_cb OnState;
	const char *stateName;
	FSM_State *nextState = nullptr;
	FSM_Action *lastAction = nullptr;
};

class YA_FSM
{
public:
	// Default constructor
	YA_FSM(){};

	enum ActionsType {N, S,	R, L, D};

	uint8_t AddState(const char *name, action_cb onEntering, action_cb onState, action_cb onLeaving);
	uint8_t AddState(const char *name, uint32_t maxTime, uint32_t minTime, action_cb onEntering, action_cb onState, action_cb onLeaving);
	uint8_t AddState(const char *name, uint32_t maxTime, action_cb onEntering, action_cb onState, action_cb onLeaving);

	uint8_t AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition = nullptr);
	uint8_t AddTransition(uint8_t inputState, uint8_t outputState, bool &condition);

	inline uint8_t AddTimedTransition(uint8_t inputState, uint8_t outputState)
	{
		return AddTransition(inputState, outputState);
	}

	uint8_t AddAction(uint8_t inputState, uint8_t type, bool &target, uint32_t _time = 0);

	void SetState(uint8_t index, bool callOnEntering = true, bool callOnLeaving = true);
	uint8_t GetState() const;
	uint8_t StateIndex() const;
	uint32_t GetEnteringTime(uint8_t index);

	// Useful for reset the beginning of state (-1 on error)
	int32_t SetEnteringTime(uint8_t index);
	void SetTimeout(uint8_t index, uint32_t preset);

	bool Timeout(uint8_t index);
	bool Update();
	FSM_State *CurrentState();
	FSM_State *GetStateAt(uint8_t index);

	inline const char *ActiveStateName() const
	{
		return m_currentState->stateName;
	}

	inline const int GetNumStates()
	{
		return m_stateIndex;
	}

	// only for compatibility with old version
	[[deprecated("Replaced by Timeout()")]] bool GetTimeout(uint8_t index);
	[[deprecated("Replaced by all in once method AddState()")]] void SetOnEntering(uint8_t index, action_cb action);
	[[deprecated("Replaced by all in once method AddState()")]] void SetOnLeaving(uint8_t index, action_cb action);
	[[deprecated("Replaced by all in once method AddState()")]] void SetOnState(uint8_t index, action_cb action, uint32_t setTimeout = 0);
	[[deprecated("Replaced by all in once method AddState()")]] void ClearOnEntering(uint8_t index);
	[[deprecated("Replaced by all in once method AddState()")]] void ClearOnLeaving(uint8_t index);
	[[deprecated("Replaced by all in once method AddState()")]] void ClearOnState(uint8_t index);
	///////////////////////////////////////////////////////

private:

	// States handling
	uint8_t m_stateIndex;
	FSM_State *m_firstState = nullptr;
	FSM_State *m_lastState = nullptr;
	FSM_State *m_currentState = nullptr;

	// Transition handling
	uint8_t m_transitionIndex;
	FSM_Transition *m_firstTransition = nullptr;
	FSM_Transition *m_lastTransition = nullptr;

	// Action handling
	uint8_t m_actionIndex;
	FSM_Action *m_firstAction = nullptr;
	FSM_Action *m_lastAction = nullptr;
	void executeAction(FSM_State *state, FSM_Action *action, bool onExit = false);
};

#endif
