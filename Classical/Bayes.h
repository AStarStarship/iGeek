// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_BAYES_T
#define IGEEK_CLASSICAL_BAYES_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// The implementations below follow these figures.

/* Figure 14.9 The enumeration algorithm for answering queries on Bayesian
networks.

@code
function ENUMERATION-ASK(X, e, bn) returns a distribution over X
    inputs: X, the query variable
            e, observed values for variables E
            bn, a Bayes net with variables {X} ∪ E ∪ Y // Y = hidden variables

    Q(X)←a distribution over X, initially empty
    for each value xi of X do
        Q(xi)←ENUMERATE-ALL(bn.VARS, exi )
            where exi is e extended with X = xi
    return NORMALIZE(Q(X))

function ENUMERATE-ALL(vars, e) returns a real number
    if EMPTY?(vars) then return 1.0
    Y ←FIRST(vars)
    if Y has value y in e
        then return P(y | parents(Y )) × ENUMERATE-ALL(REST(vars), e)
        else return Σy P(y | parents(Y )) × ENUMERATE-ALL(REST(vars), ey)
            where ey is e extended with Y = y
@endcode
*/

/* Figure 14.11 The variable elimination algorithm for inference in Bayesian
networks.

@code
function ELIMINATION-ASK(X, e, bn) returns a distribution over X
    inputs: X, the query variable
            e, observed values for variables E
            bn, a Bayesian network specifying joint distribution P(X1,... , Xn)

    factors ←[ ]
    for each var in ORDER(bn.VARS) do
        factors ←[MAKE-FACTOR(var , e)|factors]
        if var is a hidden variable then factors ←SUM-OUT(var, factors )
    return NORMALIZE(POINTWISE-PRODUCT(factors))
@endcode
*/

/* Figure 14.13 A sampling algorithm that generates events from a Bayesian
network. Each variable is sampled according to the conditional distribution
given the values already sampled for the variable’s parents.

@code
function PRIOR-SAMPLE(bn) returns an event sampled from the prior specified by
bn inputs: bn, a Bayesian network specifying joint distribution P(X1, . . . ,
Xn)

    x←an event with n elements
    foreach variable Xi in X1, . . . , Xn do
        x[i]←a random sample from P(Xi | parents(Xi))
    return x
@endcode
*/

/* Figure 14.14 The rejection-sampling algorithm for answering queries given
evidence in a Bayesian network.

@code
function REJECTION-SAMPLING(X, e, bn,N) returns an estimate of P(X|e)
    inputs: X, the query variable
            e, observed values for variables E
            bn, a Bayesian network
            N, the total number of samples to be generated
    local variables: N, a TArray of counts for each value of X, initially zero

    for j = 1 to N do
        x←PRIOR-SAMPLE(bn)
        if x is consistent with e then
            N[x ]←N[x ]+1 where x is the value of X in x
    return NORMALIZE(N)
@endcode
*/

/* Figure 14.15 The likelihood-weighting algorithm for inference in Bayesian
networks. In WEIGHTED-SAMPLE, each nonevidence variable is sampled according to
the conditional distribution given the values already sampled for the variable’s
parents, while a weight is accumulated based on the likelihood for each evidence
variable.

@code
function LIKELIHOOD-WEIGHTING(X, e, bn,N) returns an estimate of P(X|e)
    inputs: X, the query variable
            e, observed values for variables E
            bn, a Bayesian network specifying joint distribution P(X1, . . . ,
Xn) N, the total number of samples to be generated local variables: W, a TArray
of weighted counts for each value of X, initially zero

    for j = 1 to N do
        x,w ←WEIGHTED-SAMPLE(bn, e)
        W[x ]←W[x] + w where x is the value of X in x
    return NORMALIZE(W)

function WEIGHTED-SAMPLE(bn, e) returns an event and a weight

    w ←1; x←an event with n elements initialized from e
    foreach variable Xi in X1, . . . , Xn do
        if Xi is an evidence variable with value xi in e
            then w ←w × P(Xi = xi | parents(Xi))
            else x[i]←a random sample from P(Xi | parents(Xi))
    return x, w
@endcode
*/

/* Figure 14.16 The Gibbs sampling algorithm for approximate inference in
Bayesian networks; this version cycles through the variables, but choosing
variables at random also works.

@code
function GIBBS-ASK(X, e, bn,N) returns an estimate of P(X|e)
    local variables: N, a TArray of counts for each value of X, initially zero
            Z, the nonevidence variables in bn
            x, the current state of the network, initially copied from e
    Initialize x with random values for the variables in Z
    for j = 1 to N do
        for each Zi in Z do
            set the value of Zi in x by sampling from P(Zi|mb(Zi))
            N[x ]←N[x] + 1 where x is the value of X in x
    return NORMALIZE(N)
@endcode
*/


/* Binary domains only (documented scope): every variable takes value 0 or 1.
Variables must be supplied in topological order (0..var_count-1): each
variable's parents are lower-indexed variables.

BaysNet layout (all tables RAMFactory-owned by the net's destructor):
  per var: parent list (parent ids, ascending) and a PPT of 2^pc rows;
  row r (bit j of r = parent j's value) holds P(var=1 | that assignment);
  P(var=0 | ...) = 1 - p. */
struct BaysNet {
  Autoject mem_;
  ISC var_count_;
  ISC parent_cap_;
  ISC ppt_cap_;
  ISC parent_count_;
  ISC ppt_count_;
  BaysNet();
  ~BaysNet();
  BOL Init(ISC var_count, ISC parent_cap, ISC ppt_cap);
  BOL TellVar(ISC var, const ISC* parents, ISC parent_count, const FPC* ppt);
  /* P(var=value | parent_values) — parent_values indexed by parent order. */
  FPC P(ISC var, IUC value, const IUC* parent_values) const;
};

/* Evidence item (Fig 14.9 'e'). */
struct BaysEv {
  ISC var;
  IUC value;
};

/* All inference answers are P(query_var = 1 | evidence). */

/* Fig 14.9 ENUMERATION-ASK: exact, recursive ENUMERATE-ALL over the variable order. */
FPC BaysEnumerate(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                    ISC ev_n);

/* Fig 14.11 ELIMINATION-ASK: variable elimination with join and sum-out
over RAMFactory-owned factors. Factor layout: a scope bit mask plus a
values array indexed by rank — bit i of the rank is scope bit bits[i] of
the assignment, so a scope of k variables spans 2^k cells regardless of
var_count. Variables are processed in reverse topological order (children
before parents) so each var may be summed out only after every factor that
still references it has been joined. Each factor (make/join/sum-out) takes
a fresh non-aliasing cell block from the pool; old factors stay live until
the destructor. */
struct BayesInfer {
  Autoject scope_mem_;
  Autoject val_mem_;
  IUD cells_used_;
  IUD cells_cap_;
  ISC var_count_;
  BayesInfer();
  ~BayesInfer();
  BOL Init(ISC var_count, IUD cells_cap);
  FPC Ask(const BaysNet& bn, ISC query_var, const BaysEv* ev, ISC ev_n);
};

/* Fig 14.13 PRIOR-SAMPLE: one sample into the caller array. */
FPC BaysSample(const BaysNet& bn, IUD seed, IUC* out_event);

/* Fig 14.14 REJECTION-SAMPLING: N samples, fixed seed. */
FPC BaysReject(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                 ISC ev_n, ISC n_samples, IUD seed);

/* Fig 14.15 LIKELIHOOD-WEIGHTING: N weighted samples, fixed seed. */
FPC BaysLw(const BaysNet& bn, ISC query_var, const BaysEv* ev,
             ISC ev_n, ISC n_samples, IUD seed);

/* Fig 14.16 GIBBS-ASK: N full cycles over the nonevidence variables Z
(query included, per the figure), fixed seed. A draw is counted after
every single-variable resample (the figure's placement of the N[x]
increment), so the estimate averages N * |Z| draws. */
FPC BaysGibbs(const BaysNet& bn, ISC query_var, const BaysEv* ev,
                ISC ev_n, ISC n_cycles, IUD seed);

} //< namespace iGeek
#endif