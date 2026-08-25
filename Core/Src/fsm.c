#include "fsm.h"

static FsmState current_state = FSM_IDLE;

void FSM_Init(void)
{
    current_state = FSM_IDLE;
}

void FSM_Process(void)
{
    switch (current_state)
    {
        case FSM_IDLE:
            break;

        case FSM_READ_SENSOR:
            break;

        case FSM_VALIDATE:
            break;

        case FSM_EVALUATE:
            break;

        case FSM_UPDATE_OUTPUT:
            break;

        case FSM_ERROR:
            break;

        case FSM_SENSOR_FAULT:
            break;

        default:
            current_state = FSM_IDLE;
            break;
    }
}