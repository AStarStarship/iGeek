// Copyright AStarship <https://astarship.net>.

namespace iGeek {

class Cell : public GeneticPolygon
/* Class represents a cell in the simulation.
 */
{
  static const ISC DefaultWidth = 50, DefaultHeight = 50, circle_buffer = 20;

  ISC numPointsWeakness, shapeAngleWeakness, colorWeakness;
  / < The color this Cell is weak to.
      /*< Viruses that are of weak to a color will allow a cell to infect it
         that smaller. */

      FPD x,
      / < The current x position.y;
  / < The current y position.

      Host host;
  / < A pointer to the host.Virus virus;
  / < The infecting virus.long timeOfInfection;
  / < The time that this cell got infected.

      Cell(Host cellHost);

  Cell(ISC x_pos, ISC y_pos, ISC width, ISC height, ISC point_count, ISC color,
       ISC lifespan, FPD angle);

  void highlight(Graphics g, Color circle_color);

  void Update();

  FPD getX();

  FPD getY();

  ISC Left();

  ISC Top();

  ISC rightEdge();

  void infectCell(Virus thisVirus);
};

}  //< namespace iGeek
