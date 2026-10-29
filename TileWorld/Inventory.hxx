// Copyright AStarship <https://astarship.net>.
#include "inventory.h"

Inventory::Inventory() {}

Inventory::~Inventory() {}

void Inventory::Initialize() {
  item_count_max_ = 10;
  item_count_ = 0;
  items_ = new Item*[item_count_max_];

  Nullify();
}

void Inventory::Expand() {}

void Inventory::Nullify(const unsigned from) {
  for (size_t i = from; i < item_count_max_; i++) {
    items_ = nullptr;
  }
}

void Inventory::FreeMemory() {
  for (size_t i = 0; i < item_count_; i++) {
    delete items_[i];
  }

  delete[] items_;
}
