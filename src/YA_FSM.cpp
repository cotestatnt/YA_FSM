#include "YA_FSM.h"

FSM_State *YA_FSM::CurrentState()
{
	return m_currentState;
}

FSM_State *YA_FSM::GetStateAt(uint8_t index)
{
	for (FSM_State *state = m_firstState; state != nullptr; state = state->nextState)
		if (state->index == index)
			return state;
	return nullptr;
}

uint8_t YA_FSM::AddState(const char *name, uint32_t maxTime, uint32_t minTime, action_cb onEntering, action_cb onState, action_cb onLeaving)
{

	FSM_State *state = new FSM_State();
	if (m_firstState == nullptr)
	{
		m_firstState = state;
		m_currentState = state;
	}
	else
	{
		m_lastState->nextState = state;
		m_stateIndex++;
	}

	m_lastState = state;
	state->OnEntering = onEntering;
	state->OnLeaving = onLeaving;
	state->OnState = onState;
	state->stateName = name;
	state->maxTime = maxTime;
	state->minTime = minTime;
	state->index = m_stateIndex;
	// state->actions[0] = nullptr;
	return m_stateIndex;
}

uint8_t YA_FSM::AddState(const char *name, uint32_t maxTime, action_cb onEntering, action_cb onState, action_cb onLeaving)
{
	return AddState(name, maxTime, 0, onEntering, onState, onLeaving);
}

uint8_t YA_FSM::AddState(const char *name, action_cb onEntering, action_cb onState, action_cb onLeaving)
{
	return AddState(name, 0, 0, onEntering, onState, onLeaving);
}

uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, condition_cb condition)
{

	FSM_Transition *transition = new FSM_Transition();
	if (m_firstTransition == nullptr)
		m_firstTransition = transition;
	else
		m_lastTransition->nextTransition = transition;
	m_lastTransition = transition;

	transition->Condition = condition;
	transition->InputState = inputState;
	transition->OutputState = outputState;

	return m_transitionIndex++;
}

uint8_t YA_FSM::AddTransition(uint8_t inputState, uint8_t outputState, bool &condition)
{

	FSM_Transition *transition = new FSM_Transition();
	if (m_firstTransition == nullptr)
		m_firstTransition = transition;
	else
		m_lastTransition->nextTransition = transition;
	m_lastTransition = transition;

	transition->ConditionVar = &condition;
	transition->InputState = inputState;
	transition->OutputState = outputState;

	return m_transitionIndex++;
}

uint8_t YA_FSM::AddAction(uint8_t inputState, uint8_t type, bool &target, uint32_t _time)
{

	// Create new action object and assign values
	FSM_Action *newAction = new FSM_Action();
	if (m_firstAction == nullptr)
		m_firstAction = newAction;
	else
		m_lastAction->nextAction = newAction;
	m_lastAction = newAction;

	newAction->StateIndex = inputState;
	newAction->Type = type;
	newAction->Target = &target;
	newAction->Delay = _time;

	// Set lastAction to the target state (in order to know, is one or more action are to be runned on state)
	FSM_State *state = GetStateAt(inputState);
	state->lastAction = newAction;

	return m_actionIndex++;
}

void YA_FSM::executeAction(FSM_State *state, FSM_Action *action, bool onExit)
{
	bool *target = action->Target;

	switch (action->Type)
	{
		case YA_FSM::S:
			*target = true;
			break;
		case YA_FSM::R:
			*target = false;
			break;
		case YA_FSM::N:
			if (onExit)
			{
				*target = false;
				break;
			}

			*target = (m_currentState->index == state->index);
			break;
		case YA_FSM::L: // Time Limited action
		{
			if (onExit)
			{
				*target = false;
				action->xEdge = false;
				action->lTime = -1;
				break;
			}

			if (!action->xEdge)
			{
				*target = true;
				action->lTime = millis();
				action->xEdge = true;
			}

			if ((millis() - action->lTime) > action->Delay && action->xEdge && action->lTime > 0)
			{
				*target = false;
			}
			break;
		}
		case YA_FSM::D: // Time Delayed action
		{
			if (onExit)
			{
				*target = false;
				action->xEdge = false;
				action->lTime = -1;
				break;
			}

			if (!action->xEdge)
			{
				action->lTime = millis();
				action->xEdge = true;
			}

			if ((millis() - action->lTime) > action->Delay && action->xEdge && action->lTime > 0)
			{
				*target = true;
				action->lTime = -1; // Action executed
			}
			break;
		}
	}
}

bool YA_FSM::Update()
{

	for (FSM_Transition *actualtr = m_firstTransition; actualtr != nullptr; actualtr = actualtr->nextTransition)
	{
		if (actualtr->InputState == m_currentState->index)
		{
			bool _trigger = false;

			// If a max time was defined check if current state is on timeout
			if (m_currentState->maxTime)
			{
				m_currentState->timeout = millis() - m_currentState->enterTime > m_currentState->maxTime;
				// Trigger transition on timeout
				if (m_currentState->timeout)
					_trigger = true;
			}

			// Trigger transition on callback function result if defined
			else if (actualtr->Condition != nullptr)
				_trigger = actualtr->Condition();

			// Trigger transition on bool variable value == true
			else if (actualtr->ConditionVar != nullptr)
				_trigger = *(actualtr->ConditionVar);

			if (_trigger)
			{
				// Check if state is on at least from minTime
				if (m_currentState->minTime > 0)
				{
					if (millis() - m_currentState->enterTime < m_currentState->minTime)
					{
						return false;
					}
				}

				// One of the transitions has triggered, set the new state
				if (m_currentState->OnLeaving != nullptr)
					m_currentState->OnLeaving();

				// Call the actions on exit previuos state to clear target if necessary
				if (m_currentState->lastAction != nullptr)
				{
					for (FSM_Action *actualAct = m_firstAction; actualAct != nullptr; actualAct = actualAct->nextAction)
					{
						if (actualAct->StateIndex == m_currentState->index)
						{
							executeAction(m_currentState, actualAct, true);
						}
					}
				}

				FSM_State *targetState = GetStateAt(actualtr->OutputState);
				m_currentState = targetState;
				m_currentState->enterTime = millis();
				m_currentState->timeout = false;

				if (m_currentState->OnEntering != nullptr)
					m_currentState->OnEntering();
				return true;
			}
		}
	}
	if (m_currentState->OnState != nullptr)
		m_currentState->OnState();

	// Run actions (if defined) for current state
	if (m_currentState->lastAction != nullptr)
	{
		for (FSM_Action *actualAct = m_firstAction; actualAct != nullptr; actualAct = actualAct->nextAction)
		{
			if (actualAct->StateIndex == m_currentState->index)
			{
				executeAction(m_currentState, actualAct, false);
			}
		}
	}
	return false;
}

uint8_t YA_FSM::StateIndex() const
{
	return m_currentState->index;
}

// Return the current active state
uint8_t YA_FSM::GetState() const
{
	return m_currentState->index;
}

// Change to State
void YA_FSM::SetState(uint8_t index, bool callOnEntering, bool callOnLeaving)
{
	FSM_State *newState = GetStateAt(index);
	// If found state at index
	if (newState != nullptr)
	{
		// Guarantee that will run OnLeaving()
		if (m_currentState->OnLeaving != nullptr && callOnLeaving)
			m_currentState->OnLeaving();

		m_currentState = newState;

		// Guarantee that will run OnEntering()
		if (m_currentState->OnEntering != nullptr && callOnEntering)
			m_currentState->OnEntering();

		// Update Enter Time
		m_currentState->enterTime = millis();
		m_currentState->timeout = false;
	}
}

void YA_FSM::SetTimeout(uint8_t index, uint32_t preset)
{
	FSM_State *state = GetStateAt(index);
	if (state != nullptr)
	{
		state->maxTime = preset;
	}
}

// Return true if timeout
bool YA_FSM::GetTimeout(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	if (state != nullptr)
	{
		return state->timeout;
	}
	return false;
}

bool YA_FSM::Timeout(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	if (state != nullptr)
	{
		return state->timeout;
	}
	return false;
}

// Return current state entering time
uint32_t YA_FSM::GetEnteringTime(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	if (state != nullptr)
	{
		return state->enterTime;
	}
	return 0;
}

int32_t YA_FSM::SetEnteringTime(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	if (state != nullptr)
	{
		state->enterTime = millis();
		return state->enterTime;
	}
	else
		return -1;
}

//////////////////////////////////////////////////////////////////////////////////

void YA_FSM::SetOnState(uint8_t index, action_cb action, uint32_t setTimeout)
{
	FSM_State *state = GetStateAt(index);
	state->maxTime = setTimeout;
	state->OnState = action;
}

void YA_FSM::SetOnEntering(uint8_t index, action_cb action)
{
	FSM_State *state = GetStateAt(index);
	state->OnEntering = action;
}

void YA_FSM::SetOnLeaving(uint8_t index, action_cb action)
{
	FSM_State *state = GetStateAt(index);
	state->OnLeaving = action;
}

void YA_FSM::ClearOnState(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	state->OnState = nullptr;
}

void YA_FSM::ClearOnEntering(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	state->OnEntering = nullptr;
}

void YA_FSM::ClearOnLeaving(uint8_t index)
{
	FSM_State *state = GetStateAt(index);
	state->OnLeaving = nullptr;
}


void YA_FSM::setStateMaxTime(uint8_t inputState, uint32_t time)
{
	FSM_State *state = GetStateAt(inputState);
	state->maxTime = time;
}

void YA_FSM::setStateMinTime(uint8_t inputState, uint32_t time)
{
	FSM_State *state = GetStateAt(inputState);
	state->minTime = time;
}