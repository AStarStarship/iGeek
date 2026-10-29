// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_ARMWORLD_HXX_3D
#define CRABS_ARMWORLD_HXX_3D
#include "ArmConfig.h"
#if SEAM >= CRABS_ARMWORLD
namespace iGeek {
/* Placeholder for the 3D phase: add the third joint (j3, z-rotation or pitch), the 3x3 inertia matrix via Math::GemmRR, and the spherical target workspace.
The 2D core (ArmPhysics.hxx) stays untouched; 3D extends TArmParams/TArmState 
with the z terms. */
struct TArmParams3D {
  FPD length_3 = 0.15;
  FPD mass_3 = 0.5;
  FPD inertia_3 = 0.004;
};
} //< namespace iGeek
#endif
#endif
