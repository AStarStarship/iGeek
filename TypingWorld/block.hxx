// Copyright AStarship <https://astarship.net>.

#include "Block.h"

namespace Typecraft {

Block::Block(ISC type, ISC variant, ISC count) {}

ISC Block::GetNumItems() { return items->GetCount(); }

ItemType Block::Getype() { return type_; }

ISC Block::Mine(Item* tool) { return 0; }

}  // namespace Typecraft
