#include <cstdio>
#define _CrabsCore			1
#define _CrabsIToS			2
#define _CrabsFToS			3
#define _CrabsUniprinter			4
#define _CrabsClock			5
#define _CrabsArray			6
#define _CrabsStack			7
#define _CrabsString			8
#define _CrabsRegEx			9
#define _CrabsLoom			10
#define _CrabsList			11
#define _CrabsBook			12
#define _CrabsMap			13
#define _CrabsTable			14
#define _CrabsDictionary			15
#define _CrabsOp			16
#define _CrabsRoom			17
#define _CrabsMathCore			18
#define _CrabsRandom			19
#define _CrabsFile			20
#define _CrabsPromise			21
#define _CrabsScript			22
#define _CrabsPassword			23
#define _CrabsRelease			24
#define _iGeekCore			25
#define _iGeekMazeWorldCore			27
#define _iGeekMazeWorldRelease			28
#define _iGeekArmWorldCore			29
#define _iGeekArmWorldRelease			30
#define _iGeekBikeWorldCore			31
#define _iGeekBikeWorldRelease			32
#define _iGeekCarWorldCore			33
#define _iGeekCarWorldRelease			34
#define _iGeekClassicalCore			37
#define _iGeekClassicalRelease			38
#define _iGeekClassicalSearch			39
#define _iGeekClassicalCSPH			40
#define _iGeekClassicalMdp			41
#define _iGeekClassicalSat			42
#define _iGeekClassicalGame			43
#define _iGeekPolygonWorldCore			46
#define _iGeekPolygonWorldRelease			47

#define _iGeekPacWorldCore			44
#define _iGeekPacWorldRelease			45
#define _iGeekPrisonerWorldCore			48
#define _iGeekPrisonerWorldRelease			49
#define _iGeekSpaceWorldCore			50
#define _iGeekSpaceWorldRelease			51
#define _iGeekSudokuWorldCore			52
#define _iGeekSudokuWorldRelease			53
#define _iGeekTileWorldCore			54
#define _iGeekTileWorldRelease			55
#define _iGeekTypingWorldCore			56
#define _iGeekTypingWorldRelease		57
#define _iGeekCardsWorldCore			58
#define _iGeekCardsWorldBlackjack			59
#define _iGeekCardsWorldRelease			60
#define _iGeekEnvironmentCore			61
#define _iGeekEnvironmentRolloutCompare			62
#define _iGeekEnvironmentJobQueue      63
/* Gym's official-output unit ships at the module's top seam; no number of its
   own, so it stays in lockstep with the last Environment unit. */
#define _iGeekEnvironmentRelease _iGeekEnvironmentJobQueue
#define _iGeekTerminalWorldCore			64
#define _iGeekTerminalWorldRelease			65
#define _iGeekRelease			66
#define _SeamN _iGeekArmWorldCore
#include "../../ASCIICrabs/_Config.h"
#define _Seam _SeamN
#include "../../ASCIICrabs/Math.hxx"
#include "../../ASCIICrabs/_Package.hxx"
#include "../../ASCIICrabs/Test.hpp"
#include "../_Package.hxx"
#include "../../Worlds/Arm/ArmGym.hxx"
#include "../../PPO.h"
#include "../../LinearPolicy.hxx"
#include "ArmWorld/01.Core.hxx"
int main() {
  const CHA* r = ArmWorldTest::Core("arm-core");
  printf("\nCore() -> %s\n", r ? r : "ok");
  return r ? 1 : 0;
}
