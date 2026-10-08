// Copyright AStarship <https://astarship.net>.

// The card world's seam levels, layered on top of the Crabs seams.
// The Crabs seams (CRABS_COUT=1 ... CRABS_RELEASE=23) come from
// ../ASCIICrabs/_Seams/_Seams.h which is #included first.

#ifndef IGEEK_CARDS_SEAMS_H
#define IGEEK_CARDS_SEAMS_H

#include "../../../ASCIICrabs/_Seams/_Seams.h"

// The card world's own seam levels, numbered after the Crabs seams so that
// SEAM >= IGEEK_CARD also implies SEAM >= CRABS_COUT (console I/O).
#define IGEEK_CARD          30
#define IGEEK_CARDSTACK     31
#define IGEEK_DECK          32
#define IGEEK_HAND          33
#define IGEEK_PLAYER        34
#define IGEEK_DEALER        35
#define IGEEK_BLACKJACK     36
#define IGEEK_BLACKJACKENV  37
#define IGEEK_BLACKJACKGYM  38

// The card world's test seams, mimicking ASCIICrabs. The FIRST seam is
// named CORE (the default seam); the LAST seam is named RELEASE. The unit
// tests live in 00.Core.hxx (gated on CARDSWORLD_CORE) and the demo/release
// unit lives in 01.Release.hxx (gated on CARDSWORLD_RELEASE).
#define CARDSWORLD_CORE     40
#define CARDSWORLD_RELEASE  41

// The default seam for building the card world executable: the core unit
// tests (the first seam). Change to CARDSWORLD_RELEASE to build the demo.
#ifndef SEAM
#define SEAM CARDSWORLD_CORE
#endif

// The "named" seam: the one the executable's main() dispatches on. When
// SEAM == SEAM_N, main runs the Release seam; otherwise it runs the test tree.
#ifndef SEAM_N
#define SEAM_N CARDSWORLD_CORE
#endif

#endif  // IGEEK_CARDS_SEAMS_H
