// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_HMMP_T
#define IGEEK_CLASSICAL_HMMP_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// Probabilistic reasoning over time (Figures 15.4, 15.6, 15.17). Reference only: no C++ implementation yet.

/* Figure 15.4 The forward–backward algorithm for smoothing: computing posterior
probabilities of a sequence of states given a sequence of observations. The
FORWARD and BACKWARD operators are defined by Equations (15.5) and (15.9),
respectively.

@code
function FORWARD-BACKWARD(ev, prior ) returns a TArray of probability
distributions inputs: ev, a TArray of evidence values for steps 1, . . . , t
            prior , the prior distribution on the initial state, P(X0)
    local variables: fv, a TArray of forward messages for steps 0, . . . , t
            b, a representation of the backward message, initially all 1s
            sv, a TArray of smoothed estimates for steps 1, . . . , t

    fv[0]←prior
    for i= 1to t do
        fv[i]←FORWARD(fv[i − 1], ev[i])
    for i= t downto 1 do
        sv[i]←NORMALIZE(fv[i]×b)
        b←BACKWARD(b, ev[i])
    return sv
@endcode
*/

/* Figure 15.6 An algorithm for smoothing with a fixed time lag of d steps,
implemented as an online algorithm that outputs the new smoothed estimate given
the observation for a new time step. Notice that the final output
NORMALIZE(f×B1) is just α f×b, by Equation (15.14).

@code
function FIXED-LAG-SMOOTHING(et,hmm, d) returns a distribution over Xt−d
    inputs: et, the current evidence for time step t
            hmm, a hidden Markov model with S × S transition matrix T
            d, the length of the lag for smoothing
    persistent: t , the current time, initially 1
            f, the forward message P(Xt|e1:t), initially hmm.PRIOR
            B, the d-step backward transformation matrix, initially the identity
matrix et−d:t, double-ended list of evidence from t − d to t, initially empty
    local variables: Ot−d,Ot, diagonal matrices containing the sensor model
information

    add et to the end of et−d:t
    Ot←diagonal matrix containing P(et|Xt)
    if t > d then
        f←FORWARD(f, et)
        remove et−d−1 from the beginning of et−d:t
        Ot−d←diagonal matrix containing P(et−d|Xt−d)
        B←O−1
        t−dT−1BTOt
    else B←BTOt
    t ←t + 1
    if t > d then return NORMALIZE(f × B1) else return null
@endcode
*/

/* Figure 15.17 The particle filtering algorithm implemented as a recursive
Update operation with state (the set of samples). Each of the sampling
operations involves sampling the relevant slice variables in topological order,
much as in PRIOR-SAMPLE. The WEIGHTED-SAMPLE-WITH-REPLACEMENT operation can be
implemented to run in O(N) expected time. The step numbers refer to the
description in the text.

@code
function PARTICLE-FILTERING(e,N, dbn) returns a set of samples for the next time
step inputs: e, the new incoming evidence N, the number of samples to be
maintained dbn, a DBN with prior P(X0), transition model P(X1|X0), sensor model
P(E1|X1) persistent: S, a TArray of samples of size N, initially generated from
P(X0) local variables: W, a TArray of weights of size N

    for i = 1 to N do
        S[i ]←sample from P(X1 | X0 = S[i ]) // step 1
        W[i ]←P(e | X1 = S[i]) // step 2
    S ←WEIGHTED-SAMPLE-WITH-REPLACEMENT(N, S,W) // step 3
    return S
@endcode
*/

} //< namespace iGeek
#endif
