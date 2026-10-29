// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_CSP_DECL
#define IGEEK_CLASSICAL_CSP_DECL
#include "../ASCIICrabs/_Config.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// The implementations below follow these figures.

/* Figure 6.3 The arc-consistency algorithm AC-3. After applying AC-3, either
every arc is arc-consistent, or some variable has an empty domain, indicating
that the CSP cannot be solved. The name “AC-3” was used by the algorithm’s
inventor (Mackworth, 1977) because it’s the third version developed in the
paper.

@code
function AC-3(csp) returns false if an inconsistency is found and true otherwise
inputs: csp, a binary CSP with components (X, D, C)
local variables: queue, a queue of arcs, initially all the arcs in csp

while queue is not empty do
    (Xi, Xj)←REMOVE-FIRST(queue)
    if REVISE(csp, Xi, Xj ) then
        if size of Di = 0then return false
        for each Xk in Xi.NEIGHBORS - {Xj} do
            add (Xk, Xi) to queue
return true

function REVISE(csp, Xi, Xj ) returns true iff we revise the domain of Xi

revised ←false
for each x in Di do
if no value y in Dj allows (x ,y) to satisfy the constraint between Xi and Xj
then delete x from Di revised ←true return revised
@endcode
*/

/* Figure 6.5 A simple backtracking algorithm for constraint satisfaction
problems. The algorithm is modeled on the recursive depth-first search of
Chapter 3. By varying the functions SELECT-UNASSIGNED-VARIABLE and
ORDER-DOMAIN-VALUES, we can implement the general-purpose heuristics discussed
in the text. The function INFERENCE can optionally be used to impose arc-,
path-, or k-consistency, as desired. If a value choice leads to failure (noticed
either by INFERENCE or by BACKTRACK), then value assignments (including those
made by INFERENCE) are removed from the current assignment and a new value is
tried.

@code
function BACKTRACKING-SEARCH(csp) returns a solution, or failure
    return BACKTRACK({ }, csp)

function BACKTRACK(assignment, csp) returns a solution, or failure
    if assignment is complete then return assignment
    var ←SELECT-UNASSIGNED-VARIABLE(csp)
    for each value in ORDER-DOMAIN-VALUES(var, assignment, csp) do
        if value is consistent with assignment then
            add {var = value} to assignment
            inferences ←INFERENCE(csp, var , value)
            if inferences ≠ failure then
            add inferences to assignment
            result ←BACKTRACK(assignment, csp)
            if result ≠ failure then
                return result
        remove {var = value} and inferences from assignment
    return failure
@endcode
*/

/* Figure 6.8 The MIN-CONFLICTS algorithm for solving CSPs by local search. The
initial state may be chosen randomly or by a greedy assignment process that
chooses a minimal conflict value for each variable in turn. The CONFLICTS
function counts the number of constraints violated by a particular value, given
the rest of the current assignment.

@codefunction MIN-CONFLICTS(csp,max steps) returns a solution or failure
inputs: csp, a constraint satisfaction problem
        max steps, the number of steps allowed before giving up

    current ←an initial complete assignment for csp
    for i = 1 to max steps do
        if current is a solution for csp then return current
        var ←a randomly chosen conflicted variable from csp.VARIABLES
        value←the value v for var that minimizes CONFLICTS(var, v, current ,
csp) set var =value in current return failure
@endcode
*/

/* Figure 6.11 The TREE-CSP-SOLVER algorithm for solving tree-structured CSPs.
If the CSP has a solution, we will find it in linear time; if not, we will
detect a contradiction.

@code
function TREE-CSP-SOLVER(csp) returns a solution, or failure
    inputs: csp, a CSP with components X, D, C

    n←number of variables in X
    assignment ←an empty assignment
    root ←any variable in X
    X ←TOPOLOGICALSORT(X, root )
    for j = n down to 2 do
        MAKE-ARC-CONSISTENT(PARENT(Xj),Xj )
        if it cannot be made consistent then return failure
    for i = 1 to n do
        assignment[Xi]←any consistent value from Di
        if there is no consistent value then return failure
    return assignment
@endcode
*/


/* Binary constraint: a value x for var_a and a value y for var_b satisfy
the constraint iff Compatible(x, y) is true. A value pair that fails
the callback violates the constraint. */
struct CspConstraint {
  ISC var_a;
  ISC var_b;
  BOL (*Compatible)(IUC x, IUC y);
};

/* Uniform-domain binary CSP: every variable takes a value 0..domain_count-1.
initial_domains is a var_count array of IUC bitmasks (bit v set = value v
available); 0..domain_count-1 bits are meaningful, the rest are ignored.
the problem description is const — the solvers work on their own RAMFactory copies of the domains and never mutate the problem.

Solvers (implemented in Csp.hxx, following the relocated AIMA figures):
  CspAc3         — Fig 6.3  arc consistency
  CspBacktrack   — Fig 6.5  backtracking search with the MRV heuristic
  CspMinConflicts— Fig 6.8  min-conflicts local search
  CspTree        — Fig 6.11 tree-structured CSP solver
All succeed-failure functions return true with out[var] = value on
success, false on failure (out untouched). */
struct CSP {
  ISC var_count;
  ISC domain_count;
  IUC* initial_domains;
  const CspConstraint* constraints;
  ISC constraint_count;
};

/* Fig 6.3: enforce arc consistency on the (caller-owned) domain copy
`domains`. Returns true when every arc is arc-consistent, false when some
domain was wiped (the CSP cannot be solved). The queue holds every arc
of every constraint, both directions; REVISE deletes unsupported values. */
BOL CspAc3(CSP* csp, IUC* domains);

/* Fig 6.5: backtracking search with MRV variable selection
(SELECT-UNASSIGNED-VARIABLE = fewest remaining values, ties by index;
ORDER-DOMAIN-VALUES = ascending; INFERENCE = forward domain filtering,
undoable via the trail). Works from the given initial `domains`. */
BOL CspBacktrack(CSP* csp, const IUC* domains, IUC* out);

/* Fig 6.8: min-conflicts local search. Greedy initial assignment (each
variable gets the value with fewest conflicts against already-assigned
earlier variables), then up to max_steps steps: pick a random conflicted
variable (seeded LCG) and assign it the value with the fewest conflicts.
Returns true with `out` set when the current assignment is conflict-free. */
BOL CspMinConflicts(CSP* csp, const IUC* domains, IUC* out, ISC max_steps,
                    IUD seed);

/* Fig 6.11: tree-structured CSP solver. The constraint graph must be a tree
(a back edge is reported as failure). Topological order from root 0,
MAKE-ARC-CONSISTENT parent-to-leaf, then forward assignment of any
consistent value. Linear time when a solution exists. */
BOL CspTree(CSP* csp, const IUC* domains, IUC* out);

} //< namespace iGeek
#endif
