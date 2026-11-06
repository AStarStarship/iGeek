// Copyright AStarship <https://astarship.net>.
#pragma once
#include <_Config.h>
#if _Seam >= _iGeekCore
#ifndef _iGeekSchoolH
#define _iGeekSchoolH
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
