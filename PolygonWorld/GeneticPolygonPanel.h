// Copyright AStarship <https://astarship.net>.
#pragma once
#ifndef _iGeekPolygonWorldGeneticPolygonPanelH
#define _iGeekPolygonWorldGeneticPolygonPanelH

#include <_Config.h>

namespace iGeek {

class GeneticPolygonPanel : public JPanel {
  static const ISC defaultMargin = 30;

  GeneticPolygonPanel(GeneticPolygon thisShape, AString thisShapeLabel,
                      ISC width, ISC height);

  void paintComponent(Graphics g);

  GeneticPolygon getGeneticPolygon();

  void respawn(ISC numPoints, ISC width, ISC height, ISC color, ISC lifespan,
               double angle);

  void setGeneticPolygon(GeneticPolygon newPoly);

 private:
  GeneticPolygon gPoly;
};

} //< namespace iGeek
#endif  //< _iGeekPolygonWorldGeneticPolygonPanelH
