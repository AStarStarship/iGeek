// Copyright AStarship <https://astarship.net>.
#include "PrisonerWorld.h"
#if _Seam >= _iGeekPrisonerWorldCore
namespace iGeek {

/* The one-and-only Prisoner environment instance.
 * Defined here so _Seams.cpp can access it via PrisonerEnvironmentironmentPtr.
 */
PrisonerEnvironment the_prisoner_env;

/* Exported symbol: the one-and-only Prisoner environment instance.
 * Defined as a variable (not a function) so it can be accessed as PrisonerEnvironmentironmentPtr
 * from other translation units.
 */
PrisonerEnvironment* PrisonerEnvironmentironmentPtr = &the_prisoner_env;

} //< namespace iGeek
#endif  //< _Seam >= _iGeekPrisonerWorldCore