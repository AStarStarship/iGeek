// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>
#if SEAM >= IGEEK_CORE
#ifndef IGEEK_COURSE_CODE
#define IGEEK_COURSE_CODE
#include "../ASCIICrabs/Operand.h"
namespace iGeek {

class School : public Operand {

  School();

  ISC ClassCount();

  void CreateRepo();

  /* Crabs operations. */
  virtual const Op* Star(CHC index, Crabs* crabs);

  private:

  ISC class_count_;   //< The number of 
      
};
} //< namespace iGeek
#endif
#endif
