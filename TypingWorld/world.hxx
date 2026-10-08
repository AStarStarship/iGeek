// Copyright AStarship <https://astarship.net>.

#include "World.h"

namespace Typecraft {

World::World() {}

ISC World::GetCount() { return count_; }

ISC World::AddItem(Item* item);

Item* World::RemoveItem(ISC index);

void World::DeleteAll() {}

}  // namespace Typecraft
