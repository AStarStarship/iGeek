// Copyright AStarship <https://astarship.net>.
#include "ItemGroup.h"
namespace Typecraft {

ItemGroup::ItemGroup(ISC max_size) {}

ItemGroup::~ItemGroup() {}

ISC ItemGroup::GetCount() { return 0; }

ISC ItemGroup::GetSize() { return 0; }

Item* ItemGroup::GetItem(ISC index) { return items[index]; }

ISC ItemGroup::AddItem(Item* item) { return 0; }

Item* ItemGroup::RemoveItem(ISC index) { return nullptr; }

void Print() {}

}  // namespace Typecraft
