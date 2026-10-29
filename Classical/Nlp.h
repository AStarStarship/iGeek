// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef IGEEK_CLASSICAL_NLP_T
#define IGEEK_CLASSICAL_NLP_T 1
#include "../ASCIICrabs/_Config.h"
#include "../ASCIICrabs/Autoject.h"
namespace iGeek {

// --- AIMA reference pseudocode (relocated from Agent.h, 2026-10-28) --------
// Natural language processing (Figures 22.1, 23.5). Reference only: no C++ implementation yet.

/* Figure 22.1 The HITS algorithm for computing hubs and authorities with
respect to a query. RELEVANT-PAGES fetches the pages that match the query, and
EXPAND-PAGES adds in every page that links to or is linked from one of the
relevant pages. NORMALIZE divides each page’s score by the sum of the squares of
all pages’ scores (separately for both the authority and hubs scores).

@code
function HITS(query) returns pages with hub and authority numbers
    pages ←EXPAND-PAGES(RELEVANT-PAGES(query))
    for each p in pages do
        p.AUTHORITY←1
        p.HUB←1
    repeat until convergence do
        for each p in pages do
            p.AUTHORITY←Σi INLINKi(p).HUB
            p.HUB←Σi OUTLINKi(p).AUTHORITY
        NORMALIZE(pages)
    return pages
@endcode
*/

/* Figure 23.5 The CYK algorithm for parsing. Given a sequence of words, it
finds the most probable derivation for the whole sequence and for each
subsequence. It returns the whole table, P, in which an entry P[X, start , len]
is the probability of the most probable X of length len starting at position
start . If there is no X of that size at that location, the probability is 0.

@code
function CYK-PARSE(words, grammar ) returns P, a table of probabilities

    N ←LENGTH(words)
    M ←the number of nonterminal symbols in grammar
    P ←an array of size [M, N, N], initially all 0

    // Insert lexical rules for each word

    for i = 1 to N do
        for each rule of form (X → wordsi [p]) do
            P[X, i, 1]←p

    // Combine first and second parts of right-hand sides of rules, from short
to long

    for length = 2 to N do
        for start = 1 to N − length + 1 do
            for len1 = 1 to N − 1 do
            len2 ←length − len1
            for each rule of the form (X → Y Z [p]) do
                P[X, start , length]←MAX(P[X, start , length],
                    P[Y, start , len1 ] × P[Z, start + len1 , len2 ] × p)
    return P
@endcode
*/

} //< namespace iGeek
#endif
