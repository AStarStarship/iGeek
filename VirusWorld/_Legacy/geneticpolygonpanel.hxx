// Copyright AStarship <https://astarship.net>.

#include <pch.h>

#include "geneticpolygonpanel.h"

namespace _ {

GeneticPolygonPanel::GeneticPolygonPanel (GeneticPolygon thisShape, AString thisShapeLabel, SI4 width, SI4 height)
{
  setPreferredSize (new Dimension (width, height));
  setBorder (BorderFactory.createLineBorder (Color.BLACK));
  setLayout (new BorderLayout ());

  JLabel thisLabel = new JLabel (thisShapeLabel);
  add (thisLabel, BorderLayout.NORTH);

  gPoly = thisShape;
}

void GeneticPolygonPanel::paintComponent (Graphics g)
{
  super.paintComponent (g);

  SI4 offsetX, offsetY;

  // We want to center the GeneticPolygon

  Dimension panelDimensions = getPreferredSize ();

  if (gPoly != null)
  {
    offsetX = (panelDimensions.width - gPoly.Width ()) / 2;
    offsetY = (panelDimensions.height - gPoly.Height ()) / 2;

    g.drawImage (gPoly.getBitmap (), offsetX, offsetY, null);
  }
}

GeneticPolygonPanel::GeneticPolygon getGeneticPolygon ()
{
  return gPoly;
}

void GeneticPolygonPanel::respawn (SI4 numPoints, SI4 width, SI4 height, SI4 color, SI4 lifespan, FP8 angle)
{
  gPoly = new GeneticPolygon (numPoints, width, height, color, lifespan, angle);
}

void GeneticPolygonPanel::setGeneticPolygon (GeneticPolygon newPoly)
{
  gPoly = newPoly;
}

}
