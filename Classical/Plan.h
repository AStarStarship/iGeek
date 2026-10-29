// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_PLAN_T
#define IGEEK_CLASSICAL_PLAN_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// Classical planning (Figures 10.9, 11.5, 11.8). Reference only: no C++ implementation yet.

/* Figure 10.9 The GRAPHPLAN algorithm. GRAPHPLAN calls EXPAND-GRAPH to add a
level until either a solution is found by EXTRACT-SOLUTION, or no solution is
possible.

@code
function GRAPHPLAN(problem) returns solution or failure

    graph ←INITIAL-PLANNING-GRAPH(problem)
    goals ←CONJUNCTS(problem.GOAL)
    nogoods ←an empty hash table
    for tl = 0 to∞do
        if goals all non-mutex in St of graph then
            solution ←EXTRACT-SOLUTION(graph, goals, NUMLEVELS(graph), nogoods)
            if solution ≠ failure then return solution
        if graph and nogoods have both leveled off then return failure
        graph←EXPAND-GRAPH(graph, problem)
@endcode
*/

/* Figure 11.5 A breadth-first implementation of hierarchical forward planning
search. The initial plan supplied to the algorithm is [Act]. The REFINEMENTS
function returns a set of action sequences, one for each refinement of the HLA
whose preconditions are satisfied by the specified state, outcome.

@code
function HIERARCHICAL-SEARCH(problem, hierarchy) returns a solution, or failure

    frontier ←a FIFO queue with [Act] as the only element
    loop do
        if EMPTY?( frontier) then return failure
        plan ←POP( frontier) // chooses the shallowest plan in frontier
        hla←the first HLA in plan, or null if none
        prefix,suffix ←the action subsequences before and after hla in plan
        outcome←RESULT(problem.INITIAL-STATE, prefix)
        if hla is null then // so plan is primitive and outcome is its result
            if outcome satisfies problem.GOAL then return plan
        else for each sequence in REFINEMENTS(hla, outcome, hierarchy) do
            frontier ←INSERT(APPEND(prefix , sequence, suffix), frontier )
@endcode
*/

/* Figure 11.8 A hierarchical planning algorithm that uses angelic semantics to
identify and commit to high-level plans that work while avoiding high-level
plans that don’t. The predicate MAKING-PROGRESS checks to make sure that we
aren’t stuck in an infinite regression of refinements. At top level, call
ANGELIC-SEARCH with [Act ] as the initialPlan .

@code
function ANGELIC-SEARCH(problem, hierarchy, initialPlan ) returns solution or
fail

    frontier ←a FIFO queue with initialPlan as the only element
    loop do
    if EMPTY?( frontier) then return fail
        plan ←POP( frontier) // chooses the shallowest node in frontier
        if REACH+(problem.INITIAL-STATE, plan) intersects problem.GOAL then
            if plan is primitive then return plan // REACH+ is exact for
primitive plans guaranteed ←REACH−(problem.INITIAL-STATE, plan) ∩ problem.GOAL
            if guaranteed≠{ } and MAKING-PROGRESS(plan, initialPlan) then
                finalState←any element of guaranteed
            return DECOMPOSE(hierarchy, problem.INITIAL-STATE, plan, finalState)
            hla←some HLA in plan
            prefix ,suffix ←the action subsequences before and after hla in plan
            for each sequence in REFINEMENTS(hla, outcome, hierarchy) do
                frontier ←INSERT(APPEND(prefix , sequence, suffix), frontier)

function DECOMPOSE(hierarchy, s0 , plan, sf ) returns a solution

    solution ←an empty plan
    while plan is not empty do
        action ←REMOVE-LAST(plan)
        si ←a state in REACH−(s0 , plan) such that sf ∈REACH−(si , action)
        problem ←a problem with INITIAL-STATE = si and GOAL = sf
        solution ←APPEND(ANGELIC-SEARCH(problem, hierarchy, action), solution)
        sf ←si
    return solution
@endcode
*/

} //< namespace iGeek
#endif
