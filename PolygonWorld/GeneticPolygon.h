// Copyright AStarship <https://astarship.net>.

namespace iGeek {

/* A class that represents a this that can morph through a genetic algorithm with other shapes.
 */
class GeneticPolygon : public Polygon {
  public:
  static const FPD maxRandomPoints     = 5.00,     //< And this was for what again?
                   defaultAngle        = 0.00,     //< The default angle of orientation.
                   matingPercent       = 0.10,     //< The percent of the population that mates durring each generation.
                   minorMutationVector = 0.50,     //< Note sure if I need this or what its for because I didn't document it right.
                   majorMutationVector = 1.00,     //< The 
                   minorScalar         = 0.10,     //< The scale of minor mutations.
                   majorScalar         = 0.15,     //< The scale of major mutations.
                   minorMutation       = 0.10,     //< The probability of a minor mutation.
                   majorMutation       = 0.05,     //< The probability of a major mutation.
                   angularMutation     = 0.10;     //< The angular mutation TArray.
                                                  /*< When a GP sprouts a new point, the system tries to even out
                                                      the points at equal angles. The angular mutation is the amount that
                                                      the point will move over time. */
    
    static const ISC defaultNumPoints = 5,              /< The default number of points for a VIrus.
                     defaultLifespan  = 30000,          /< The default life span in milliseconds.
                     defaultSize      = 50,             /< The default size in pixels.
                     minSize          = defaultSize / 2,/< The minimum size in pixels.
                     maxSize          = 2 * defaultSize;
   // The maximum size of pixels.
        ISC lifespan,
        width, / < The width of this GP in pixels.
        height;
    //< The height of this GP in pixels. */

        FPD age,
        / < This variable represents the age a GP in generations.
            /*< Initially the age is set to the GP's life span, and is decremented. */
            angle;
    / < The angle of this GP in radians.

        Color color;
    / < The ARGB color of this GP.

        long numPixels;
    / < The number of pixels in this image.

        BufferedImage bitmap;
    / < The bitmap rendering of this object.

        GeneticPolygon mother,
        / < The thisMother of this GP.father, / < The father of this GP.grandparents;
    / < The grandparents of this GP.

        static Color
        getDefaultColor(ISC numPoints);

    GeneticPolygon();

    GeneticPolygon(ISC point_count, ISC color);

    GeneticPolygon(ISC point_count, ISC width, ISC height, ISC color, ISC lifespan, FPD angle);

    GeneticPolygon(GeneticPolygon thisMother, GeneticPolygon thisFather);

    void rebound();

    void printGeometry();

    void setupPoints(ISC numberOfPoints);

    BOL age(ISC ageTime);

    static Color mixColors(Color motherColor, Color fatherColor);

    Color breedColor(Color motherColor, Color fatherColor, Color matGrandColor,
                     Color patGrandColor);

    ISC mutateInt(ISC thisValue, ISC mutateDirection);

    FPD mutateFloat(FPD thisValue, ISC mutateDirection);

    ISC breedInt(ISC motherInt, ISC fatherInt, ISC matGrandInt, ISC patGrandInt);

    BOL containsGeneticPolygon(GeneticPolygon that);

    FPD breedFloat(FPD motherInt, FPD fatherInt, FPD matGrandInt, FPD patGrandInt);

    /*  @brief  Function that returns the BufferedImage for this this. */
    BufferedImage getBitmap();

    ISC Width();

    ISC Height();

    FPD getAngle();

    Color getColor();

    long getNumPixels();

    ISC getLifespan();

    ISC getNumPoints();

    // Function that prints out the this to a bitstream.
    void PrintGenes(ISC[] stream, ISC currentIndex);

    BOL Intersects(Polygon thatPolygon);

    BOL Intersects(GeneticPolygon thatPolygon);

    BOL Intersects(Polygon thatPolygon, FPD offsetX, FPD offsetY);

    BOL Intersects(Polygon thatPolygon, ISC offsetX, ISC offsetY);

    void EqualizePoints();

    void RenderBitmap();

    static FPD CrossOver(FPD dominant, FPD recessive);

    void Rotate();

    FPD BoundBetween0and1(FPD inputValue);

    FPD RandomizeDouble(FPD inputValue);

    ISC RandomizeInt(ISC inputValue);
};
