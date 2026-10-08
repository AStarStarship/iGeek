// Copyright AStarship <https://astarship.net>.

#include <_Config.h>

#include "geneticpolygonpanel.h"

namespace _ {

GeneticPolygonPanel::GeneticPolygonPanel(GeneticPolygon thisShape,
                                         AString thisShapeLabel, ISC width,
                                         ISC height) {
  setPreferredSize(new Dimension(width, height));
  setBorder(BorderFactory.createLineBorder(Color.BLACK));
  setLayout(new BorderLayout());

  JLabel thisLabel = new JLabel(thisShapeLabel);
  add(thisLabel, BorderLayout.NORTH);

  gPoly = thisShape;
}

void GeneticPolygonPanel::paintComponent(Graphics g) {
  super.paintComponent(g);

  ISC offsetX, offsetY;

  // We want to center the GeneticPolygon

  Dimension panelDimensions = getPreferredSize();

  if (gPoly != null) {
    offsetX = (panelDimensions.width - gPoly.Width()) / 2;
    offsetY = (panelDimensions.height - gPoly.Height()) / 2;

    g.drawImage(gPoly.getBitmap(), offsetX, offsetY, null);
  }
}

GeneticPolygonPanel::GeneticPolygon getGeneticPolygon() { return gPoly; }

void GeneticPolygonPanel::respawn(ISC numPoints, ISC width, ISC height,
                                  ISC color, ISC lifespan, FPD angle) {
  gPoly = new GeneticPolygon(numPoints, width, height, color, lifespan, angle);
}

void GeneticPolygonPanel::setGeneticPolygon(GeneticPolygon newPoly) {
  gPoly = newPoly;
}

}  //< namespace _
