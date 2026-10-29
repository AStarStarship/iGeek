// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_LEARN_T
#define IGEEK_CLASSICAL_LEARN_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// Inductive learning (Figures 18.5, 18.8, 18.11, 18.24, 18.34). Reference only: no C++ implementation yet.

/* Figure 18.5 The decision-tree learning algorithm. The function IMPORTANCE is
described in Section 18.3.4. The function PLURALITY-VALUE selects the most
common output value among a set of examples, breaking ties randomly.

@code
function DECISION-TREE-LEARNING(examples, attributes, parent examples) returns
    a tree

    if examples is empty then return PLURALITY-VALUE(parent examples)
    else if all examples have the same classification then return the
classification else if attributes is empty then return PLURALITY-VALUE(examples)
    else
        A←argmaxa ∈ attributes IMPORTANCE(a, examples)
        tree←a new decision tree with root test A
        for each value vk of A do
            exs ←{e : e ∈examples and e.A = vk}
            subtree←DECISION-TREE-LEARNING(exs, attributes −A, examples)
            add a branch to tree with label (A = vk) and subtree subtree
        return tree
@endcode
*/

/* Figure 18.8 An algorithm to select the model that has the lowest error rate
on validation data by building models of increasing complexity, and choosing the
one with best empirical error rate on validation data. Here errT means error
rate on the training data, and errV means error rate on the validation data.
Learner (size, examples) returns a hypothesis whose complexity is set by the
parameter size, and which is trained on the examples. PARTITION(examples, fold,
k) splits examples into two subsets: a validation set of size N/k and a training
set with all the other examples. The split is different for each value of fold.

@code
function CROSS-VALIDATION-WRAPPER(Learner , k, examples) returns a hypothesis

    local variables: errT, an array, indexed by size, storing training-set error
rates errV , an array, indexed by size, storing validation-set error rates

    for size = 1 to∞ do
        errT[size], errV [size]←CROSS-VALIDATION(Learner , size, k, examples)
        if errT has converged then do
            best size←the value of size with minimum errV [size]
            return Learner (best size, examples)

function CROSS-VALIDATION(Learner , size, k, examples) returns two values:
            average training set error rate, average validation set error rate

    fold errT ←0; fold errV ←0
    for fold = 1 to k do
        training set , validation set ←PARTITION(examples, fold , k)
        h←Learner (size, training set )
        fold errT ←fold errT + ERROR-RATE(h, training set)
        fold errV ←fold errV +ERROR-RATE(h, validation set )
    return fold errT/k, fold errV /k
@endcode
*/

/* Figure 18.11 An algorithm for learning decision lists.

@code
function DECISION-LIST-LEARNING(examples) returns a decision list, or failure

    if examples is empty then return the trivial decision list No
    t ←a test that matches a nonempty subset examplest of examples
        such that the members of examplest are all positive or all negative
    if there is no such t then return failure
    if the examples in examplest are positive then o←Yes else o←No
    return a decision list with initial test t and outcome o and remaining tests
given by DECISION-LIST-LEARNING(examples − examplest)
@endcode
*/

/* Figure 18.24 The back-propagation algorithm for learning in multilayer
networks.

@code
function BACK-PROP-LEARNING(examples, network ) returns a neural network
    inputs: examples, a set of examples, each with input TArray x and output
TArray y network , a multilayer network with L layers, weights wi,j , activation
function g local variables: Δ, a TArray of errors, indexed by network node

    repeat
        for each weight wi,j in network do
            wi,j ←a small random number
        for each example (x, y) in examples do

            // Propagate the inputs forward to compute the outputs

            for each node i in the input layer do
                ai←xi
            for λ = 2 to L do
            for each node j in layer λ do
                inj ←Σi wi,j ai
                aj ←g(inj)

            // Propagate deltas backward from output layer to input layer

            for each node j in the output layer do
                Δ[j]←g′(inj) × (yj − aj)
            for λ = L − 1 to 1 do
                for each node i in layer λ do
                    Δ[i]←g′(ini)Σj wi,j Δ[j]

            // Update every weight in network using deltas

            for each weight wi,j in network do
            wi,j ←wi,j + α × ai × Δ[j]
    until some stopping criterion is satisfied
    return network
@endcode
*/

/* Figure 18.34 The ADABOOST variant of the boosting method for ensemble
learning. The algorithmgenerates hypotheses by successively reweighting the
training examples. The function WEIGHTED-MAJORITY generates a hypothesis that
returns the output value with the highest vote from the hypotheses in h, with
votes weighted by z.

@code
function ADABOOST(examples,L,K) returns a weighted-majority hypothesis
    inputs: examples, set of N labeled examples (x1, y1), . . . , (xN, yN)
            L, a learning algorithm
            K, the number of hypotheses in the ensemble
    local variables: w, a TArray of N example weights, initially 1/N
            h, a TArray of K hypotheses
            z, a TArray of K hypothesis weights

    for k = 1 to K do
        h[k]←L(examples,w)
        error ←0
        for j = 1 to N do
            if h[k](xj ) ≠ yj then error ←error + w[j]
        for j = 1 to N do
            if h[k](xj) = yj then w[j]←w[j] · error/(1 − error )
        w←NORMALIZE(w)
        z[k]←log (1 − error )/error
    return WEIGHTED-MAJORITY(h, z)
@endcode
*/

} //< namespace iGeek
#endif
