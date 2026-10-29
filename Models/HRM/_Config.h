// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef HRM_CONFIG_H
#define HRM_CONFIG_H 1

// HRM model seam. CRABS_FILE (19) puts the build past the COut/
// Stringf/Uniprinter seams (1-4) so the printer impls in _Package.hxx
// compile for this TU.
#define HRM_MODEL 19

// Local seam level table (mirrors ASCIICrabs/_Seams/_Seams.h for the levels
// this TU uses) so <_Seams.h> resolves from this directory.
#include "_Seams.h"

// The Crabs defaults (CPU size, console, FP, etc.) must come before the
// header so the header's #if USING_XXX checks see the right values.
#include "../../../ASCIICrabs/_ConfigDefault.h"

// The Crabs config header: defines all the POD types (CHA, ISC, BOL, etc.),
// the AType enum, and the platform macros.
#include "../../../ASCIICrabs/_ConfigHeader.h"

// The Crabs config footer: defines the width aliases (CHR, ISR, IUR, FPR).
#include "../../../ASCIICrabs/_ConfigFooter.h"

// The model seam (HRM_MODEL, defined above) must be visible to the ASCIICrabs
// headers' #if SEAM >= ... checks, and SEAM must not be redefined by any
// upstream config header.
#define SEAM HRM_MODEL

#endif
