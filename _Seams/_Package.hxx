// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef CRABS_IGEOK_SEAMS_PACKAGE_HXX
#define CRABS_IGEOK_SEAMS_PACKAGE_HXX
// The _Seams tree's package rollup. Each world's _Tests.hxx pulls this for the
// ASCIICrabs runtime (TTestTree, TTestCase, the test harness). The iGeek RL
// core (Tensor/Gym/...) is pulled once by the iGeek-root _Package.hxx, which
// this file does NOT re-include (it lives one level up).
#include "../ASCIICrabs/_Package.hxx"
#endif
