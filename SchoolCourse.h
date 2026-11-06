// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekSchoolCourseH
#define _iGeekSchoolCourseH
#include "../ASCIICrabs/Operand.h"
#if _Seam >= _iGeekCore
namespace iGeek {

class SchoolCourse: public _::Operand {

  SchoolCourse();

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
