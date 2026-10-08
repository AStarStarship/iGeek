// Copyright AStarship <https://astarship.net>.
#include <_Config.h>
#include "VirusPopulation.h"
namespace HomoWorld {

VirusPopulation::VirusPopulation(Host thisHost) {
  host = thisHost;
  viruses = new Virus[DefaultPopulationSize];
  virus_count = DefaultPopulationSize;

  Dimension host_dimensions = host.getPreferredSize();

  FPD x, y, hostWidth = host_dimensions.width,
            hostHeight = host_dimensions.height;

  for (ISC i = 0; i < DefaultPopulationSize; i++) {
    x = Math.random() * (FPD)hostWidth;
    y = Math.random() * (FPD)hostHeight;

    viruses[i] = new Virus(x, y);
  }
}

// Adds the virus to the population.
void VirusPopulation::addVirus(Virus virus) {
  ISC virus_count = viruses.length, i;

  Virus[] virus = new Virus[virus_count + 1];

  for (i = 0; i < virus_count; i++) virus[i] = viruses[i];

  virus[i] = virus;
}

// Returns the virus at the specified index.
Virus VirusPopulation::getVirus(ISC index) {
  if (index < 0 || index > virus_count) return null;

  return viruses[index];
}

// Selects a random virus from the population.
ISC VirusPopulation::selectRandomVirus() {
  if (virus_count == 0) return -1;

  ISC n = 0, maxTries = 2 * virus_count, index;

  do {
    index = (ISC)(Math.random() * (virus_count - 1));
    if (viruses[index] != null) return index;
    ++n;
  } while (n < maxTries);

  // Generate list of non-null indexes.
  ISC populatedIndecies[] = new ISC[virus_count], i = 0;

  for (index = 0; index < virus_count; index++) {
    if (viruses[index] != null) {
      populatedIndecies[i] = index;
      i++;
    }
  }
  index = (ISC)(Math.random() * (virus_count - 1));

  return index;
}

/* Function returns a value greater that -1 if this population contains
polygon.
@return Returns -1 if the population doesn't contain polygon. Else it
will return the index of the Virus.
*/
ISC VirusPopulation::Contains(GeneticPolygon polygon) {
  for (ISC i = 0; i < virus_count; i++) {
    Virus currentVirus = viruses[i];

    if (currentVirus != null &&
        currentVirus.containsGeneticPolygon(polygon)) {
      // Then the virus has occupies the cell
      return i;
    }
  }
  return -1;
}

// Function that selects the virus
ISC VirusPopulation::SelectVirus(ISC mouseX, ISC mouseY) {
  for (ISC i = 0; i < virus_count; i++) {
    // Print (i + ", "); // Debug info
    Virus current = viruses[i];
    if (current != null && current.containsPoint(mouseX, mouseY)) return i;
  }
  return -1;
}

// Returns the number of viruses.
ISC VirusPopulation::GetNumViruses() { return virus_count; }

// Updates the virus population with the new .
void VirusPopulation::Update() {
  Dimension hostSize = host.getPreferredSize();

  ISC width = hostSize.width, height = hostSize.height;

  for (ISC i = 0; i < virus_count; i++) viruses[i].updatePosition(width, height);

  for (ISC i = 0; i < virus_count; i++) {
    ISC j;

    for (j = 0; j < i; j++) viruses[i].collideWith(viruses[j]);

    for (j++; j < virus_count; j++) viruses[i].collideWith(viruses[j]);
  }
}

void VirusPopulation::Draw(Graphics g) {
  Dimension host_dimensions = host.PreferredSize();

  for (ISC i = 0; i < virus_count; i++) {
    Virus virus = viruses[i];

    ISC currentX = (ISC)virus.getX(), currentY = (ISC)virus.getY();

    if (currentX + virus.Width() >= 0 && currentY + virus.Height() >= 0 &&
        currentX <= host_dimensions.width && currentY <= host_dimensions.height) {
      virus.draw(g);
    }
  }

  boxCollidingViruses(g);

  // for (ISC i=0; i < virus_count; i++)
  //{
  //    for (ISC j=0; j < virus_count-1 viruses[i]
  //}
}

void VirusPopulation::BoxCollidingViruses(Graphics& g) {
  for (ISC i = 0; i < virus_count; i++) {
    Virus a = viruses[i];
    ISC j;

    for (j = 0; j < i; j++) {
      Virus b = viruses[j];

      if (a.intersects(b)) BoxTwoViruses(g, a, b);
    }
    for (j++; j < virus_count; j++) {
      Virus b = viruses[j];

      if (a.intersects(b)) BoxTwoViruses(g, a, b);
    }
  }
}

void VirusPopulation::BoxTwoViruses(Graphics& g, Virus& a, Virus& b) {
  ISC left = (ISC)((a.getX() < b.getX()) ? a.getX() : b.getX()),
      top = (ISC)((a.getY() < b.getY()) ? a.getY() : b.getY()),
      right = (ISC)((a.rightEdge() > b.rightEdge()) ? a.rightEdge()
                                                    : b.rightEdge()),
      bottom = (ISC)((a.bottomEdge() > b.bottomEdge()) ? a.bottomEdge()
                                                       : b.bottomEdge());

  g.setColor(new Color(1.0f, 0.0f, 0.0f, 0.5f));

  g.drawRect(left - 2, top - 2, right - left + 2, bottom - top + 2);

  a.drawCircle(g, Color.blue);
  b.drawCircle(g, Color.green);
}

}  //< namespace _
}
