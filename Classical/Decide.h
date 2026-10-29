// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_DECIDE_T
#define IGEEK_CLASSICAL_DECIDE_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

/* Figure 13.1 A decision-theoretic agent that selects rational actions.

@code
function DT-AGENT(percept ) returns an action
    persistent: belief state, probabilistic beliefs about the current state of
the world action, the agent’s action

    Update belief state based on action and percept
    calculate outcome probabilities for actions,
        given action descriptions and current belief state
    select action with highest expected utility
        given probabilities of outcomes and utility information
    return action
@endcode
*/

/* Figure 16.9 Design of a simple information-gathering agent. The agent works
by repeatedly selecting the observation with the highest information value,
until the cost of the next observation is greater than its expected benefit.

@code
function INFORMATION-GATHERING-AGENT(percept ) returns an action
    persistent: D, a decision network

    integrate percept into D
    j ←the value that maximizes VPI (Ej) / Cost (Ej )
    if VPI (Ej) > Cost (Ej )
        return REQUEST(Ej )
    else return the best action from D
@endcode
*/

} //< namespace iGeek
#endif
