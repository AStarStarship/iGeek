// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _IgeekSeamsConfig
#define _IgeekSeamsConfig
#include "../../ASCIICrabs/_Config.h"
// #undef _Seam: ASCIICrabs/_ConfigFooter.h sets _Seam=CRABS_RELEASE with
// no guard. Redefine to _SeamN so every _iGeek* gate evaluates true.
#undef _Seam
#define _Seam _SeamN

/* Seams (iGeek block, on top of the ASCIICrabs base 1..24):
#define _iGeekCore			25
#define _iGeekRelease			26
#define _iGeekMazeWorldCore			27
#define _iGeekMazeWorldRelease			28
#define _iGeekArmWorldCore			29
#define _iGeekArmWorldRelease			30
#define _iGeekBikeWorldCore			31
#define _iGeekBikeWorldRelease			32
#define _iGeekCarWorldCore			33
#define _iGeekCarWorldRelease			34
#define _iGeekCardsWorldCore			35
#define _iGeekCardsWorldRelease			36
#define _iGeekClassicalCore			37
#define _iGeekClassicalRelease			38
#define _iGeekClassicalSearch			39
#define _iGeekClassicalCSPH			40
#define _iGeekClassicalMdp			41
#define _iGeekClassicalSat			42
#define _iGeekClassicalGame			43
#define _iGeekPackWorldCore			44
#define _iGeekPackWorldRelease			45
#define _iGeekPolygonWorldCore			46
#define _iGeekPolygonWorldRelease			47
#define _iGeekPrisonerWorldCore			48
#define _iGeekPrisonerWorldRelease			49
#define _iGeekSpaceWorldCore			50
#define _iGeekSpaceWorldRelease			51
#define _iGeekSudokuWorldCore			52
#define _iGeekSudokuWorldRelease			53
#define _iGeekTileWorldCore			54
#define _iGeekTileWorldRelease			55
#define _iGeekTypingWorldCore			56
#define _iGeekTypingWorldRelease			57  (release)
*/
#endif  //< _IgeekSeamsConfig
