// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekSchoolDegreeH
#define _iGeekSchoolDegreeH
#include "SchoolCourse.h"
#if _Seam >= _iGeekCore
namespace iGeek {

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
