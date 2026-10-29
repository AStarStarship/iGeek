// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_SAT_DECL
#define IGEEK_CLASSICAL_SAT_DECL
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// The implementations in Sat.hxx follow these figures.

/* Figure 7.1 A generic knowledge-based agent. Given a percept, the agent adds
the percept to its knowledge base, asks the knowledge base for the best action,
and tells the knowledge base that it has in fact taken that action.

@code
function KB-AGENT(percept ) returns an action
    persistent: KB, a knowledge base
        t , a counter, initially 0, indicating time

    TELL(KB,MAKE-PERCEPT-SENTENCE(percept , t ))
    action ←ASK(KB,MAKE-ACTION-QUERY(t ))
    TELL(KB,MAKE-ACTION-SENTENCE(action, t ))
    t ←t + 1
    return action
@endcode
*/

/*  Figure 7.7 A BNF (Backus–Naur Form) grammar of sentences in propositional
logic, along with operator precedences, from highest to lowest.
|
|  Logic Type         | Logic |
|--------------------:|:------|
| Sentence            | → AtomicSentence OR ComplexSentence
| AtomicSentence      | → True OR False OR P OR Q OR R OR . . .
| ComplexSentence     | → ( Sentence ) OR [ Sentence ] OR ¬Sentence OR Sentence
∧ Sentence OR Sentence ∨ Sentence OR Sentence ⇒ Sentence OR Sentence ⇔ Sentence
| | OPERATOR PRECEDENCE | : ¬, ∧, ∨,⇒,⇔


*/

/* Figure 7.10 A truth-table enumeration algorithm for deciding propositional
entailment. (TT stands for truth table.) PL-TRUE? returns true if a sentence
holds within a model. The variable model represents a partial model—an
assignment to some of the symbols. The keyword “and” is used here as a logical
operation on its two arguments, returning true or false.

@code
function TT-ENTAILS?(KB,α) returns true or false
    inputs: KB, the knowledge base, a sentence in propositional logic
        α, the query, a sentence in propositional logic
    symbols←a list of the proposition symbols in KB and α
    return TT-CHECK-ALL(KB,α, symbols,{ })

    function TT-CHECK-ALL(KB,α, symbols ,model ) returns true or false
        if EMPTY?(symbols) then
            if PL-TRUE?(KB,model ) then return PL-TRUE?(α,model )
            else return true // when KB is false, always return true
        else do
            P ←FIRST(symbols)
            rest ←REST(symbols)
            return (TT-CHECK-ALL(KB,α, rest ,model ∪ {P = true})
                and
                TT-CHECK-ALL(KB,α, rest ,model ∪ {P = false }))
@endcode
*/

/* Figure 7.11 Standard logical equivalences. The symbols α, β, and γ stand for
arbitrary sentences of propositional logic.

|           Identity                      | Description. |
|:---------------------------------------:|:-------------|
|       (α ∧ β) ≡ (β ∧ α)               | Commutativity of ∧ |
|       (α ∨ β) ≡ (β ∨ α)               | Commutativity of ∨ |
| ((α ∧ β) ∧ γ) ≡ (α ∧ (β ∧ γ))        | Associativity of ∧ |
| ((α ∨ β) ∨ γ) ≡ (α ∨ (β ∨ γ))        | Associativity of ∨ |
|         ¬(¬α) ≡ α                       | Double-negation elimination |
|       (α ⇒ β) ≡ (¬β ⇒ ¬α)             | Contraposition |
|       (α ⇒ β) ≡ (¬α ∨ β)              | Implication elimination |
|       (α ⇔ β) ≡ ((α ⇒ β) ∧ (β ⇒ α))  | Biconditional elimination |
|      ¬(α ∧ β) ≡ (¬α∨ ¬β)              | De Morgan |
|      ¬(α ∨ β) ≡ (¬α∧ ¬β)              | De Morgan |
| (α ∧ (β ∨ γ)) ≡ ((α ∧ β) ∨ (α ∧ γ)) | Distributivity of ∧ over ∨ |
| (α ∨ (β ∧ γ)) ≡ ((α ∨ β) ∧ (α ∨ γ)) | Distributivity of ∨ over ∧ |
*/

/* Figure 7.12 A simple resolution algorithm for propositional logic. The
function PL-RESOLVE returns the set of all possible clauses obtained by
resolving its two inputs.

@code
function PL-RESOLUTION(KB,α) returns true or false
inputs: KB, the knowledge base, a sentence in propositional logic
    α, the query, a sentence in propositional logic
    clauses ←the set of clauses in the CNF representation of KB ∧¬α
    new ←{}
    loop do
        for each pair of clauses Ci, Cj in clauses do
        resolvents ←PL-RESOLVE(Ci,Cj )
        if resolvents contains the empty clause then return true
        new ←new ∪ resolvents
    if new ⊆ clauses then return false
    clauses ←clauses ∪new

@endcode
*/

/* Figure 7.15 The forward-chaining algorithm for propositional logic. The
agenda keeps track of symbols known to be true but not yet “processed.” The
count table keeps track of how many premises of each implication are as yet
unknown. Whenever a new symbol p from the agenda is processed, the count is
reduced by one for each implication in whose premise p appears (easily
identified in constant time with appropriate indexing.) If a count reaches zero,
all the premises of the implication are known, so its conclusion can be added to
the agenda. Finally, we need to keep track of which symbols have been processed;
a symbol that is already in the set of inferred symbols need not be added to
the agenda again. This avoids redundant work and prevents loops caused by
implications such as P ⇒ Q and Q ⇒ P.

@code
function PL-FC-ENTAILS?(KB, q) returns true or false
    inputs: KB, the knowledge base, a set of propositional definite clauses
            q, the query, a proposition symbol
    count ←a table, where count [c] is the number of symbols in c’s premise
    inferred ←a table,where inferred[s] is initially false for all symbols
    agenda ←a queue of symbols, initially symbols known to be true in KB
    while agenda is not empty do
        p←POP(agenda)
        if p = q then return true
        if inferred[p] = false then
            inferred[p]←true
            for each clause c in KB where p is in c.PREMISE do
                decrement count [c]
                if count [c] = 0 then add c.CONCLUSION to agenda
    return false
@endcode
*/

/* Figure 7.17 The DPLL algorithm for checking satisfiability of a sentence in
propositional logic. The ideas behind FIND-PURE-SYMBOL and FIND-UNIT-CLAUSE are
described in the text; each returns a symbol (or null) and the truth value to
assign to that symbol. Like TT-ENTAILS?, DPLL operates over partial models.

@code
function DPLL-SATISFIABLE?(s) returns true or false
    inputs: s, a sentence in propositional logic

    clauses ←the set of clauses in the CNF representation of s
    symbols←a list of the proposition symbols in s
    return DPLL(clauses, symbols,{ })

function DPLL(clauses, symbols,model ) returns true or false
    if every clause in clauses is true in model then return true
    if some clause in clauses is false in model then return false
    P, value←FIND-PURE-SYMBOL(symbols, clauses,model )
    if P is non-null then return DPLL(clauses, symbols – P,model ∪ {P=value})
    P, value←FIND-UNIT-CLAUSE(clauses,model )
    if P is non-null then return DPLL(clauses, symbols – P,model ∪ {P=value})
    P ←FIRST(symbols); rest ←REST(symbols)
    return DPLL(clauses, rest ,model ∪ {P=true}) or
            DPLL(clauses, rest ,model ∪ {P=false}))
@endcode
*/

/* Figure 7.18 The WALKSAT algorithm for checking satisfiability by randomly
flipping the values of variables. Many versions of the algorithm exist.

@code
function WALKSAT(clauses, p,max flips) returns a satisfying model or failure
    inputs: clauses, a set of clauses in propositional logic
            p, the probability of choosing to do a “random walk” move, typically
around 0.5 max flips, number of flips allowed before giving up

    model ←a random assignment of true/false to the symbols in clauses
    for i= 1to max flips do
        if model satisfies clauses then return model
        clause←a randomly selected clause from clauses that is false in model
        with probability p flip the value in model of a randomly selected symbol
from clause else flip whichever symbol in clause maximizes the number of
satisfied clauses return failure
@endcode
*/

/* Figure 7.20 A hybrid agent program for the wumpus world. It uses a
propositional knowledge base to infer the state of the world, and a combination
of problem-solving search and domain-specific code to decide what actions to
take.

@code
function HYBRID-WUMPUS-AGENT(percept ) returns an action
    inputs: percept , a list, [stench,breeze,glitter ,bump,scream]
    persistent: KB, a knowledge base, initially the atemporal “wumpus physics”
            t , a counter, initially 0, indicating time
            plan, an action sequence, initially empty

    TELL(KB,MAKE-PERCEPT-SENTENCE(percept , t ))
    TELL the KB the temporal “physics” sentences for time t
    safe ←{[x , y] : ASK(KB,OKt
    x,y) = true}
    if ASK(KB,Glittert) = true then
        plan ←[Grab] + PLAN-ROUTE(current , {[1,1]}, safe) + [Climb]
    if plan is empty then
        unvisited ←{[x , y] : ASK(KB,Lt
        x,y) = false for all t′ ≤ t}
        plan ←PLAN-ROUTE(current, unvisited ∩safe, safe)
    if plan is empty and ASK(KB,HaveArrowt) = true then
        possible wumpus ←{[x , y] : ASK(KB,¬Wx,y) = false}
        plan ←PLAN-SHOT(current , possible wumpus, safe)
        if plan is empty then // no choice but to take a risk
        not unsafe ←{[x , y] : ASK(KB,¬ OKt
        x,y) = false}
        plan ←PLAN-ROUTE(current, unvisited ∩not unsafe, safe)
    if plan is empty then
        plan ←PLAN-ROUTE(current, {[1, 1]}, safe) + [Climb]
        action ←POP(plan)
        TELL(KB,MAKE-ACTION-SENTENCE(action, t ))
        t ←t + 1
    return action

function PLAN-ROUTE(current,goals,allowed) returns an action sequence
    inputs: current , the agent’s current position
        goals, a set of squares; try to plan a route to one of them
        allowed, a set of squares that can form part of the route

    problem ←ROUTE-PROBLEM(current , goals,allowed)
    return A*-GRAPH-SEARCH(problem)
@endcode
*/

/* Figure 7.22 The SATPLAN algorithm. The planning problem is translated into a
CNF sentence in which the goal is asserted to hold at a fixed time step t and
axioms are included for each time step up to t. If the satisfiability algorithm
finds a model, then a plan is extracted by looking at those proposition symbols
that refer to actions and are assigned true in the model. If no model exists,
then the process is repeated with the goal moved one step later.

@code
function SATPLAN(init , transition, goal , T max) returns solution or failure
    inputs: init , transition, goal , constitute a description of the problem
    T max, an upper limit for plan length
    for t = 0 to T max do
    cnf ←TRANSLATE-TO-SAT(init , transition, goal , t )
    model ←SAT-SOLVER(cnf )
    if model is not null then
    return EXTRACT-SOLUTION(model )
    return failure
@endcode
*/

/* Chapter 7 — propositional-logic reasoning. The proposition-symbol universe
 * is 0..symbol_count-1 with symbol_count <= 64 (clauses are IUD literal
 * masks). A clause is a set of literals; a model is an IUD mask of the
 * symbols assigned true. All solvers are pure over the caller's clause
 * arrays (no mutation) and allocate scratch through the RAMFactory only. */

/* A clause: positive literal mask | negative literal mask (¬symbol). */
struct SatClause {
  IUD positive;
  IUD negative;
};

/* A definite clause (Fig 7.15): all-positive premise set, single-symbol
 * conclusion. */
struct SatDefinite {
  IUD premise;
  IUD conclusion;
};

/* Fig 7.10 TT-ENTAILS?: does the KB clause set entail the query clause set?
 * (For every model: if all KB clauses hold, all query clauses hold.) */
BOL SatTtEntails(const SatClause* kb, ISC kb_count, const SatClause* query,
                 ISC query_count, IUD universe);

/* Fig 7.15 PL-FC-ENTAILS?: forward chaining over definite clauses. `known`
 * is the mask of facts true in KB (non-clause atoms); q is the query symbol. */
BOL SatFcEntails(const SatDefinite* kb, ISC kb_count, IUD known, ISC q);

/* Fig 7.12 PL-RESOLUTION: the caller supplies the CNF clauses of KB ∧ ¬α
 * (the book's `clauses`); iterates until a fixpoint. True when the empty
 * clause is derived (KB entails α); false when the set closes. */
BOL SatResolution(const SatClause* clauses, ISC clause_count, IUD universe);

/* Fig 7.17 DPLL-SATISFIABLE?: pure-symbol and unit-clause propagation plus
 * branch-and-bound. True when some model satisfies all clauses. */
BOL SatDpll(const SatClause* clauses, ISC clause_count, IUD universe);

/* Fig 7.18 WALKSAT: random walk over models; out_model (IUD mask of true
 * symbols) on success. */
BOL SatWalksat(const SatClause* clauses, ISC clause_count, IUD universe, FPC p,
               ISC max_flips, IUD seed, IUD* out_model);

/* Fig 7.1 KB-AGENT: a clause-set KB with TELL (add clauses) and Ask
 * (entailment test: KB entails the positive query when KB ∧ ¬query is
 * unsatisfiable — answered with DPLL; the definite-clause fast path is
 * available separately via SatFcEntails). */
struct TKBAgent {
  Autoject kb_mem_;
  ISC kb_count_;
  ISC kb_cap_;
  TKBAgent();
  ~TKBAgent();
  BOL Init(ISC kb_cap);
  /* Add clauses to the KB (the book's TELL). */
  BOL Tell(const SatClause* clauses, ISC count);
  /* KB entails the query iff KB ∧ ¬query is unsatisfiable. `query_neg`
   * is the CNF of the negated query (the caller's MAKE-ACTION-QUERY). */
  BOL Ask(const SatClause* query_neg, ISC count, IUD universe) const;
  /* Clauses currently in the KB (count = kb_count_). */
  const SatClause* Clauses() const;
};

/* Fig 7.22 SATPLAN. Time-unindexed predicate atoms 0..atom_count-1
 * (atom_count <= 6); action a has preconditions `pre` and effects `add`
 * (precondition masks of 1-2 bits are the contract — more premises need
 * extra CNF clauses the world must build itself). The horizon is t_max:
 * the goal is asserted at each t = t_max down to 0 until satisfiable
 * (the book runs t upward; equivalent: the first t with a model is the
 * shortest plan). out_plan[t] = the action taken at step t
 * (atom_count actions, plan_cap >= t). */
struct SatPlanAction {
  IUD pre;
  IUD add;
  IUD del;
};
struct SatPlanProblem {
  ISC atom_count;
  const SatPlanAction* actions;
  ISC action_count;
  IUD init_atoms;  //< atoms true at time 0 (closed world)
  IUD goal_atoms;  //< atoms that must hold at the final time
};
BOL SatPlan(const SatPlanProblem* problem, ISC t_max, IUD* model_out,
           IUC* out_plan, ISC plan_cap);

/* Fig 7.20 HYBRID-WUMPUS-AGENT on the 2x2 grid (the book's 4x4 world runs
 * out of the 64-symbol universe once times are indexed; 2x2 keeps the full
 * time-indexed KB at t <= 5). Actions: */
static const IUC kWmpForward = 0;
static const IUC kWmpLeft = 1;
static const IUC kWmpRight = 2;
static const IUC kWmpGrab = 3;
static const IUC kWmpClimb = 4;
static const IUC kWmpShootN = 5;
static const IUC kWmpShootS = 6;
static const IUC kWmpShootE = 7;
static const IUC kWmpShootW = 8;

struct WumpusPercept {
  BOL stench;
  BOL breeze;
  BOL glitter;
  BOL bump;
  BOL scream;
};

/* The agent holds the time-indexed definite-clause KB (OK, W, Stench_t,
 * Breeze_t, Glitter_t, HaveArrow_t, At(x,y,t)) and drives the book's action
 * selection: grab-and-climb on glitter, route to an unvisited safe square,
 * shoot possible wumpus squares, or retreat. PLAN-ROUTE is SearchBFS over
 * the grid restricted to `allowed` squares. */
struct THybridWumpusAgent {
  Autoject kb_mem_;
  ISC kb_count_;
  ISC kb_cap_;
  ISC t_;
  IUC x_;
  IUC y_;
  IUC heading_;  //< 0=N, 1=E, 2=S, 3=W
  IUD visited_;  //< mask of (y*2+x) squares ever lit
  IUD known_true_;   //< symbols asserted true by the KB
  IUD known_false_;  //< symbols asserted false by the KB
  BOL arrow_;
  BOL done_;
  IUC plan_[32];  //< pending action sequence
  ISC plan_count_;
  THybridWumpusAgent();
  ~THybridWumpusAgent();
  BOL Init(ISC kb_cap);
  /* TELL the t_-th percept facts and run the fixpoint chain. */
  void TellPercept(const WumpusPercept* p, BOL moved, IUC x, IUC y);
  /* The book's HYBRID-WUMPUS-AGENT step: TELL the t-th percept, re-plan,
   * return the action. The world applies the action and hands back the
   * resulting percept next call (x, y = the new square; moved = the agent
   * entered a new square). */
  IUC Percept(const WumpusPercept* p, BOL moved, IUC x, IUC y);
  /* ASK(KB, OKx,y) — safe to move into (x, y)? */
  BOL Safe(IUC x, IUC y) const;
  /* ASK(KB, ¬Wx,y) — the wumpus is NOT at (x, y)? */
  BOL NotWumpus(IUC x, IUC y) const;
  /* PLAN-ROUTE: append the relative actions moving from `from` to one of
   * `goals` through `allowed | goals` squares (SearchBFS). */
  BOL Route(IUC from, IUD goals, IUD allowed);
  /* PLAN-SHOT: append the actions to shoot a possible wumpus square. */
  BOL PlanShot(IUC current, IUD possible, IUD safe);
};

} //< namespace iGeek
#endif
