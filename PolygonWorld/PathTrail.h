// Copyright AStarship <https://astarship.net>.

namespace iGeek {

class PathTrail
/* A class that represents a trail of lines.
    This class is represented by a circular cue of 2 dimensionsal points.
 */
{
 public:
  static const Color DefaultTrailColor = Color.red;
  / < The default PathTrail color.

      static const ISC DefaultMaxNumPoints = 300;
  / < In order to make a line, we need two points.

                               PathTrail();

  PathTrail(Color thisTrailColor);

  PathTrail(Color thisTrailColor, ISC thisMaxNumPoints);

  void Initialize();

  void setTrailColor(Color newColor);

  Color getTrailColor();

  void draw(Graphics g);

  void addPoint(ISC x, ISC y);

  AString ToString();

 private:
  Color trailColor;
  / < The Color of this PathTrail.

      ISC numPoints,
      / < The number of points.startPoint,
      / < The current startPoint in the circular queue.stopPoint,
      / < The current stopPoint in the circular queue.maxNumPoints;           /< The maximum number of points.
                
    ISC xpoints[],              /< The x values for the points.
        ypoints[];              /< The y values for the points.
};

}  //< namespace iGeek
