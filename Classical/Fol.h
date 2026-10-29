// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_FOL_T
#define IGEEK_CLASSICAL_FOL_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

/* Figure 8.3 The syntax of first-order logic with equality, specified in
Backus–Naur form (see page 1060 if you are not familiar with this notation).
Operator precedences are specified, from highest to lowest. The precedence of
quantifiers is such that a quantifier holds over everything to the right of it.

|                 logic | Equality                            |
|----------------------:|:-----------------------------------:|
|              Sentence | → AtomicSentence OR ComplexSentence |
|        AtomicSentence | → Predicate OR Predicate(Term, . . .) OR Term = Term |
|       ComplexSentence | → ( Sentence ) OR [ Sentence ] OR ¬Sentence OR
Sentence ∧ Sentence OR Sentence ∨ Sentence OR Sentence ⇒ Sentence OR Sentence ⇔
Sentence OR Quantifier Variable, . . . Sentence  | |                  Term | →
Function(Term, . . .) OR Constant OR Variable | |            Quantifier | → ∀ OR
∃ | |              Constant | → A OR X1 OR John OR · · · | | Variable | → a OR x
OR s OR ··· | |             Predicate |→ True OR False OR After OR Loves OR
Raining OR ··· | |              Function | → Mother OR LeftLeg OR · · · | |
OPERATOR PRECEDENCE | : ¬,=, ∧, ∨,⇒,⇔ |
*/

/* Figure 9.1 The unification algorithm. The algorithm works by comparing the
structures of the inputs, element by element. The substitution θ that is the
argument to UNIFY is built up along the way and is used to make sure that later
comparisons are consistent with bindings that were established earlier. In a
compound expression such as F(A,B), the OP field picks out the function symbol F
and the ARGS field picks out the argument list (A,B).

@code
function UNIFY(x , y, θ) returns a substitution to make x and y identical
    inputs: x , a variable, constant, list, or compound expression
            y, a variable, constant, list, or compound expression
            θ, the substitution built up so far (optional, defaults to empty)
    if θ = failure then return failure
    else if x = y then return θ
    else if VARIABLE?(x ) then return UNIFY-VAR(x , y, θ)
    else if VARIABLE?(y) then return UNIFY-VAR(y, x , θ)
    else if COMPOUND?(x ) and COMPOUND?(y) then
        return UNIFY(x.ARGS, y.ARGS, UNIFY(x.OP, y.OP, θ))
    else if LIST?(x ) and LIST?(y) then
        return UNIFY(x .REST, y.REST, UNIFY(x .FIRST, y.FIRST, θ))
    else return failure

function UNIFY-VAR(var, x , θ) returns a substitution
    if {var/val} ∈ θ then return UNIFY(val , x , θ)
    else if {x/val} ∈ θ then return UNIFY(var, val , θ)
    else if OCCUR-CHECK?(var, x ) then return failure
    else return add {var/x } to θ
@endcode
*/

/* Figure 9.3 A conceptually straightforward, but very inefficient,
forward-chaining algorithm. On each iteration, it adds to KB all the atomic
sentences that can be inferred in one step from the implication sentences and
the atomic sentences already in KB. The function STANDARDIZE-VARIABLES replaces
all variables in its arguments with new ones that have not been used before.

@code
function FOL-FC-ASK(KB,α) returns a substitution or false
    inputs: KB, the knowledge base, a set of first-order definite clauses
            α, the query, an atomic sentence

            local variables: new, the new sentences inferred on each iteration

    repeat until new is empty
        new ←{}
        for each rule in KB do
        (p1 ∧ . . . ∧ pn ⇒ q)←STANDARDIZE-VARIABLES(rule)
        for each θ such that SUBST(θ, p1 ∧ . . . ∧ pn) = SUBST(θ, p′
            1 ∧ . . . ∧ p′
            n)
                for some p′
            1 , . . . , p′
            n in KB
            q′←SUBST(θ, q)
            if q′ does not unify with some sentence already in KB or new then
                add q′ to new
                φ←UNIFY(q′,α)
                if φ is not fail then return φ
        add new to KB
    return false
@endcode
*/

/* Figure 9.6 A simple backward-chaining algorithm for first-order knowledge
bases.

@code
function FOL-BC-ASK(KB, query) returns a generator of substitutions
    return FOL-BC-OR(KB, query,{ })

generator FOL-BC-OR(KB, goal , θ) yields a substitution
    for each rule (lhs ⇒ rhs) in FETCH-RULES-FOR-GOAL(KB, goal ) do
        (lhs, rhs)←STANDARDIZE-VARIABLES((lhs, rhs))
        for each θ′ in FOL-BC-AND(KB, lhs, UNIFY(rhs, goal , θ)) do
            yield θ′

generator FOL-BC-AND(KB, goals, θ) yields a substitution
    if θ = failure then return
    else if LENGTH(goals) = 0 then yield θ
    else do
        first,rest ←FIRST(goals), REST(goals)
        for each θ′ in FOL-BC-OR(KB, SUBST(θ, first), θ) do
            for each θ′′ in FOL-BC-AND(KB, rest , θ′) do
                yield θ′′

@endcode
*/

/* Figure 9.8 Pseudocode representing the result of compiling the Append
predicate. The function NEW-VARIABLE returns a new variable, distinct from all
other variables used so far. The procedure CALL(continuation) continues
execution with the specified continuation.

@code
procedure APPEND(ax , y, az , continuation)

    trail ←GLOBAL-TRAIL-POINTER()
    if ax = [ ] and UNIFY(y, az ) then CALL(continuation)
    RESET-TRAIL(trail )
    a, x , z ←NEW-VARIABLE(), NEW-VARIABLE(), NEW-VARIABLE()
    if UNIFY(ax , [a | x ]) and UNIFY(az , [a | z ]) then APPEND(x , y, z ,
continuation)
@endcode
*/


/* Fig 8.3 is the syntax of the logic; this module operates on already-parsed term trees. Term encoding (documented choice):
  kind 0 — atom:  value = a small ground-atom id (caller-assigned, < 2^40)
  kind 1 — var:   value = variable id, 0..var_count-1
  kind 2 — pair:  left = head node id, right = tail node id (lists)
  kind 3 — nil:   empty list
Nodes live in a caller-owned pool of FolTerm; a node id is a pool index.
Compound expressions (the book's OP/ARGS branch) are out of scope: the
caller flattens them to lists. Atoms are ids, not trees. */
struct FolTerm {
  IUD value;
  IUD left;
  IUD right;
  IUC kind;  //< 0 atom, 1 var, 2 pair, 3 nil (see above)
};

/* Substitution entry: var id -> node (any pool node: atom, var, or pair).
The book's add {var/x} to θ. */
struct FolSubst {
  ISC var;
  IUD node;
};

/* Fig 9.1 UNIFY(x, y, θ) with occurs check. x and y are node ids into the
caller pool. theta_in/out are caller arrays of FolSubst; out_n is the
resulting size. out_cap must fit at most (x_n + y_n) new bindings. */
BOL FolUnify(const FolTerm* pool, IUD x, IUD y,
             const FolSubst* theta, ISC theta_n, ISC var_count,
             FolSubst* out_theta, ISC out_cap, ISC* out_n);

/* Fig 9.6 backward chaining over definite clauses.
A clause is a list of premise nodes (vars act as existential wildcards —
documented deviation from full θ-carrying: premise variables bind to
anything) plus one ground-atom conclusion. Ask runs goal-directed
backward chaining with a per-path visited mask (at most 64 atom ids) and
a max_depth bound, both per the loop-avoidance intent of the figure. */
struct FolKb {
  Autoject tab_;
  Autoject pool_;
  ISC var_count_;
  ISC clause_cap_;
  ISC clause_count_;
  ISC pool_count_;
  BOL Init(ISC clause_cap, ISC var_count);
  BOL Tell(const FolTerm* premises, ISC premise_count, IUD conclusion_atom);
  BOL Ask(IUD goal_atom, ISC max_depth) const;
  FolKb();
  ~FolKb();
  /* Fig 9.3 NAIVE-ENTAILS: ground every clause over the domain of
  domain_size constants, then for each grounding of the goal (a var goal
  grounds to every constant) check with SatDpll that
  groundKB ∪ {¬g} is unsatisfiable. Exponential in the grounding count —
  the point of the figure. Atom ids must be < 64; returns false when the
  grounding count exceeds 4096 (documented bound). */
  BOL NaiveEntails(const FolTerm* goal, const IUD* constants,
                   ISC domain_size) const;
};

/* Fig 9.8 compiled Append: APPEND(ax, y, az, continuation). The compiled
scheme's trail/continuation is the bounded-depth list walk; here it is
the direct length+prefix check (same semantics, iteration replaces
recursion). ax/y/az are flat atom-id arrays; [] is the empty array.
az is the output: on success az_n = ax_n + y_n and az = ax ++ y. */
BOL FolAppend(const IUD* ax, ISC ax_n, const IUD* y, ISC y_n,
              IUD* az, ISC az_cap, ISC* az_n);


} //< namespace iGeek
#endif