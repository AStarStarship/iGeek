// Copyright AStarship <https://astarship.net>.

#pragma once
#ifndef IGEEK_SCHOOLCOURSE
#define IGEEK_SCHOOLCOURSE
#include "SchoolCourse.h"
#if SEAM >= IGEEK_CORE
namespace _ {

class SchoolDegree: public _::Operand {

  SchoolDegree();

  ISC SectionCount();

  void CreateRepo();

  /* Crabs operations. */
  virtual const Op* Star(CHC index, Crabs* crabs);
  
  private:
  
  ISC section_count_;   //< The number of different sections of this class.
};
}
#endif
#endif
