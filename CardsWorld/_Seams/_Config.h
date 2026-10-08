// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_CARDS_CONFIG
#define IGEEK_CARDS_CONFIG 1

// Pull in the card-world seam defines (IGEEK_*) and, transitively, the
// Crabs seam defines (CRABS_*). The _ConfigHeader.h #includes <_Seams.h>,
// which resolves to our _Seams/_Seams.h via the CMake include path.
#include "_Seams.h"

// The Crabs defaults (CPU size, console, FP, etc.) must come before the
// header so the header's #if USING_XXX checks see the right values.
#include "../../../ASCIICrabs/_ConfigDefault.h"

// The Crabs config header: defines all the POD types (CHA, ISC, BOL, etc.),
// the AType enum, and the platform macros. It #includes <_Seams.h> (already
// guarded) and <cstdint>.
#include "../../../ASCIICrabs/_ConfigHeader.h"

// The Crabs config footer: defines the width aliases (CHR, ISR, IUR, FPR),
// the LOM type, and the room name length.
#include "../../../ASCIICrabs/_ConfigFooter.h"

#endif
