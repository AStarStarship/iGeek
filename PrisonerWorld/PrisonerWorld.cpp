// Copyright AStarship <https://astarship.net>.
#include "PrisonerWorld.h"
#if SEAM >= CRABS_PRISONERWORLD
namespace _ {

/* The one-and-only Prisoner environment instance.
 * Defined here so _Main.cpp can access it via PrisonerEnvPtr.
 */
PrisonerEnv the_prisoner_env;

/* Exported symbol: the one-and-only Prisoner environment instance.
 * Defined as a variable (not a function) so it can be accessed as PrisonerEnvPtr
 * from other translation units.
 */
PrisonerEnv* PrisonerEnvPtr = &the_prisoner_env;

}  // namespace _
#endif  // SEAM >= CRABS_PRISONERWORLD