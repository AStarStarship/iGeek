// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_FOLLEARN_T
#define IGEEK_CLASSICAL_FOLLEARN_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// Learning in a logical representation (Figures 19.2, 19.3, 19.8, 19.12). Reference only: no C++ implementation yet.

/* Figure 19.2 The current-best-hypothesis learning algorithm. It searches for a
consistent hypothesis that fits all the examples and backtracks when no
consistent specialization/ generalization can be found. To start the algorithm,
any hypothesis can be passed in; it will be specialized or gneralized as needed.

@code
function CURRENT-BEST-LEARNING(examples, h) returns a hypothesis or fail
    if examples is empty then
        return h
    e←FIRST(examples)
    if e is consistent with h then
        return CURRENT-BEST-LEARNING(REST(examples), h)
    else if e is a false positive for h then
        for each h′ in specializations of h consistent with examples seen so far
do h′′←CURRENT-BEST-LEARNING(REST(examples), h′) if h′′ ≠ fail then return h′′
    else if e is a false negative for h then
        for each h′ in generalizations of h consistent with examples seen so far
do h′′←CURRENT-BEST-LEARNING(REST(examples), h′) if h′′ ≠ fail then return h′′
    return fail
@endcode
*/

/* Figure 19.3 The version space learning algorithm. It finds a subset of V that
is consistent with all the examples.

@code
function VERSION-SPACE-LEARNING(examples) returns a version space
    local variables: V, the version space: the set of all hypotheses

    V ←the set of all hypotheses
    for each example e in examples do
        if V is not empty then V ←VERSION-SPACE-UPDATE(V, e)
    return V

    function VERSION-SPACE-UPDATE(V, e) returns an updated version space

    V ←{h ∈V : h is consistent with e}
@endcode
*/

/* Figure 19.8 An algorithm for finding a minimal consistent determination.

@code
function MINIMAL-CONSISTENT-DET(E,A) returns a set of attributes
    inputs: E, a set of examples
            A, a set of attributes, of size n

    for i = 0 to n do
        for each subset Ai of A of size i do
            if CONSISTENT-DET?(Ai,E) then return Ai

function CONSISTENT-DET?(A,E) returns a truth value
    inputs: A, a set of attributes
            E, a set of examples
    local variables: H, a hash table

    for each example e in E do
        if some example in H has the same values as e for the attributes A
            but a different classification then return false
        store the class of e in H, indexed by the values for attributes A of the
example e return true
@endcode
*/

/* Figure 19.12 Sketch of the FOIL algorithm for learning sets of first-order
Horn clauses from examples. NEW-LITERALS and CHOOSE-LITERAL are explained in the
text.

@code
function FOIL(examples, target) returns a set of Horn clauses
    inputs: examples, set of examples
            target, a literal for the goal predicate
    local variables: clauses, set of clauses, initially empty

    while examples contains positive examples do
        clause←NEW-CLAUSE(examples, target)
        remove positive examples covered by clause from examples
        add clause to clauses
    return clauses

function NEW-CLAUSE(examples, target) returns a Horn clause
    local variables: clause, a clause with target as head and an empty body
            l , a literal to be added to the clause
            extended examples, a set of examples with values for new variables

    extended examples ←examples
    while extended examples contains negative examples do
        l ←CHOOSE-LITERAL(NEW-LITERALS(clause), extended examples)
        append l to the body of clause
        extended examples ←set of examples created by applying EXTEND-EXAMPLE
            to each example in extended examples
    return clause

function EXTEND-EXAMPLE(example, literal ) returns a set of examples

    if example satisfies literal
        then return the set of examples created by extending example with
            each possible constant value for each new variable in literal
    else return the empty set
@endcode
*/

} //< namespace iGeek
#endif
