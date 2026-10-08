// Copyright AStarship <https://astarship.net>.

#include <pch.h>

#include "host.h"

namespace _ {


Host::Host (SI4 width, SI4 height, SI4 numberOfCells, SI4 numberOfViruses)
{
  setPreferredSize (new Dimension (width, height));
  setBorder (BorderFactory.createLineBorder (Color.BLACK));

  cells = new Cell[numberOfCells];

  numCells = numberOfCells;

  for (SI4 i = 0; i < numberOfCells; i++)
    cells[i] = new Cell (this);

  viruses = new VirusPopulation (this);

  hostColor = DefaultHostColor;
  backgroundColor = DefaultBackgroundColor;
}

SI4 Host::getNumCells ()
{
  return numCells;
}
VirusPopulation Host::virusPopulation ()
{
  return viruses;
}

void Host::update ()
{
  viruses.update ();

  // Check to see if a Virus occupies a cell.

  for (SI4 i = 0; i < numCells; i++)
  {
    Cell currentCell = cells[i];

    if (viruses.contains (currentCell) >= 0)
    {
      //if (currentCell.getMass ())
      //    ;
    }
  }
}

void Host::paintComponent (Graphics g)
{
  super.paintComponent (g);

  setBackground (backgroundColor);

  Dimension bounds = getPreferredSize ();

  g.setColor (hostColor);
  g.fillRect (0, 0, bounds.width, bounds.height);

  viruses.draw (g);
}

}
}
    
