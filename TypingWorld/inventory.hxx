// Copyright AStarship <https://astarship.net>.

#include "Inventory.h"

namespace Typecraft {

Inventory::Inventory(ISC max_size) {}

ISC Inventory::GetCount() { return 0; }

ISC Inventory::AddItem(Item* item) { return 0; }

Item* Inventory::RemoveItem(ISC index) { return 0; }

void Inventory::DeleteAll() {}

void Inventory::Print() {}

}  // namespace Typecraft
