// Copyright AStarship <https://astarship.net>.

#include <_Config.h>

#include "GeneticPolygonChildPanel.h"

namespace _ {

GeneticPolygonChildPanel::GeneticPolygonChildPanel(GeneticPolygon fitShape,
                                                   GeneticPolygon initChild,
                                                   AString thisShapeLabel,
                                                   ISC width, ISC height) {
  setPreferredSize(new Dimension(width, height));
  setBorder(BorderFactory.createLineBorder(Color.BLACK));
  setLayout(new BorderLayout());

  JLabel thisLabel = new JLabel(thisShapeLabel);
  infoLabel = new JLabel("");

  add(thisLabel, BorderLayout.NORTH);
  add(infoLabel, BorderLayout.SOUTH);

  cell = fitShape;
  setChild(initChild);
}

void GeneticPolygonChildPanel::paintComponent(Graphics g) {
  super.paintComponent(g);

  ISC offsetX, offsetY;

  // We want to center the GeneticPolygon

  Dimension panelDimensions = getPreferredSize();

  // draw the cell

  if (cell != null) {
    offsetX = (panelDimensions.width - cell.Width()) / 2;
    offsetY = (panelDimensions.height - cell.Height()) / 2;

    g.drawImage(cell.getBitmap(), offsetX, offsetY, null);
  }
  // draw the child

  if (child != null) {
    offsetX = (panelDimensions.width - child.Width()) / 2;
    offsetY = (panelDimensions.height - child.Height()) / 2;

    g.drawImage(child.getBitmap(), offsetX, offsetY, null);
  }
}

GeneticPolygon GeneticPolygonChildPanel::getChild() { return child; }

void GeneticPolygonChildPanel::setChild(GeneticPolygon newChild) {
  if (newChild == null) return;
  child = newChild;

  // Update childLabel

  AString infoString;

  if (cell.containsGeneticPolygon(newChild)) {
    infoString = "Cell contains child\n" +
                 "Cell mass = " + cell.getNumPixels() + "\n" +
                 "Child mass = " + newChild.getNumPixels() + "\n";
    //+ "Child is " + cell.getNumPixels ()/child.getNumPixels () + "% of the
    // mass of the cell.\n";
  } else {
    infoString = "Cell does not contain child.";
  }

  infoLabel.setText(infoString);
}

}  //< namespace _
