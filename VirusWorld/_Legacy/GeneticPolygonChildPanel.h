// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>

namespace _ {

class GeneticPolygonChildPanel : public Panel {
 public:
  GeneticPolygonChildPanel(GeneticPolygon fitShape, GeneticPolygon initChild,
                           AString thisShapeLabel, ISC width, ISC height);

  void paintComponent(Graphics g);

  GeneticPolygon getChild();

  void setChild(GeneticPolygon newChild);

 private:
  GeneticPolygon child, cell;

  Label infoLabel;

  boolean timerOn;
};
}  //< namespace _
}
