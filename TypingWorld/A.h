// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#ifndef TYPECRAFT_A_DECL
#define TYPECRAFT_A_DECL
#include <Crabs/Room.hpp>
namespace Typecraft {

/* The A in A*B. */
struct A : public _::Room {
 public:
  A() : A(1024) {}
};

}  //< namespace Typecraft
#endif
