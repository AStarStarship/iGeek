// Copyright AStarship <https://astarship.net>.

#include <_Config.h>

#include "geneticpolygon.h"

namespace _ {

Color GeneticPolygon::getDefaultColor(ISC numPoints) {
  switch (numPoints) {
    case 3:
      return Color.blue;
    case 4:
      return Color.red;
    case 5:
      return Color.green;
    case 6:
      return Color.orange;
    case 7:
      return Color.gray;
    case 8:
      return Color.pink;
  }
  return new Color((ISC)Math.random());
}

GeneticPolygon::GeneticPolygon() {
  ISC numPoints = (ISC)(3.0 + Math.random() * maxRandomPoints);

  mother = father = null;
  lifespan = defaultLifespan;

  width = randomizeInt(defaultSize);
  height = randomizeInt(defaultSize);
  age = randomizeDouble(defaultLifespan);
  angle = randomizeDouble(defaultAngle);

  color = getDefaultColor(numPoints);

  setupPoints(numPoints);

  renderBitmap();
}

GeneticPolygon::GeneticPolygon(ISC point_count, ISC color) {
  mother = father = null;
  lifespan = defaultLifespan;

  width = randomizeInt(defaultSize);
  height = randomizeInt(defaultSize);
  age = randomizeDouble(defaultLifespan);
  angle = randomizeDouble(defaultAngle);

  color = new Color(color);

  setupPoints(point_count);

  renderBitmap();
}

GeneticPolygon::GeneticPolygon(ISC point_count, ISC width, ISC height,
                               ISC color, ISC lifespan, FPD angle) {
  mother = father = null;
  lifespan = lifespan;

  age = (ISC)lifespan;

  width = randomizeInt(width);
  height = randomizeInt(height);
  angle = randomizeDouble(angle);

  color = new Color(color);

  setupPoints(point_count);
  renderBitmap();
}

GeneticPolygon::GeneticPolygon(GeneticPolygon thisMother,
                               GeneticPolygon thisFather) {
  mother = thisMother;
  father = thisFather;

  if (mother == null || father == null) {
    if (mother == null)
      Print("Error breeding GeneticPolygon: Mother was null!\n");
    if (father == null)
      Print("Error breeding GeneticPolygon: Father was null!\n");

    grandparents = null;

    width = randomizeInt(GeneticPolygon.defaultSize);
    height = randomizeInt(GeneticPolygon.defaultSize);
    angle = randomizeDouble(GeneticPolygon.defaultAngle);

    color = getDefaultColor(defaultNumPoints);

    setupPoints(GeneticPolygon.defaultNumPoints);
  } else {
    grandparents = new GeneticPolygon(
        (thisMother.getNumPoints() + thisFather.getNumPoints()) / 2,
        (thisMother.Width() + thisFather.Width()) / 2,
        (thisMother.Height() + thisFather.Height()) / 2,
        (thisMother.getColor().getRGB() + thisFather.getColor().getRGB()) / 2,
        (thisMother.getLifespan() + thisFather.getLifespan()) / 2,
        (thisMother.getAngle() + thisFather.getAngle()) / 2.0f);

    GeneticPolygon matGrandparents = mother.grandparents,
                   patGrandparents = father.grandparents;

    if (matGrandparents == null) matGrandparents = grandparents;
    if (patGrandparents == null) patGrandparents = grandparents;

    width = breedInt(mother.width, father.width, matGrandparents.width,
                     patGrandparents.width);

    if (width < minSize)
      width = minSize;
    else if (height > maxSize)
      width = maxSize;

    height = breedInt(mother.height, father.height, matGrandparents.height,
                      patGrandparents.height);

    if (height < minSize)
      height = minSize;
    else if (height > maxSize)
      height = maxSize;

    angle = breedFloat(mother.angle, father.angle, matGrandparents.angle,
                       patGrandparents.angle);
    color = breedColor(mother.color, father.color, matGrandparents.color,
                       patGrandparents.color);

    setupPoints(breedInt(mother.getNumPoints(), father.getNumPoints(),
                         matGrandparents.getNumPoints(),
                         patGrandparents.getNumPoints()));
  }

  renderBitmap();
}

void GeneticPolygon::rebound() {
  // if (npoints == 0); // This shouldn't happen.

  // Once a shape has been made from the idea width and height, the actual
  // polygon width will not be the full width or height. For this reason, we
  // need to find the left, right, top, and bottom bounds, and translate all of
  // the points so that the farthest point to the left and top are both 0.

  ISC lowestX = xpoints[0],  // The largest positive integer.
      highestY = ypoints[0],
      highestX = lowestX,  // The largest negative integer.
      lowestY = highestY;

  for (ISC i = 0; i < npoints; i++) {
    ISC currentX = xpoints[i], currentY = ypoints[i];

    if (currentX < lowestX)
      lowestX = currentX;
    else if (currentX > highestX)
      highestX = currentX;

    if (currentY > highestY)
      highestY = currentY;
    else if (currentY < lowestY)
      lowestY = currentY;

    // Print ("lowestX = " + lowestX + ", highestX = " + highestX + ", highestY
    // = " + highestY + ", lowestY = " + lowestY + "\n");
  }

  // Now translate the pots such that the farthest point left and top point are
  // both 0

  for (ISC i = 0; i < npoints; i++) {
    xpoints[i] = xpoints[i] - lowestX;
    ypoints[i] = ypoints[i] - lowestY;
  }

  // Print ("initWidth = " + width + ", initHeight = " + height + "\n");

  width = highestX - lowestX + 1;
  height = highestY - lowestY + 1;

  // printGeometry ();
}

void GeneticPolygon::PrintGeometry() {
  ISC i;

  Print("\nPrinting Geometry...\nwidth = " + width + ", height = " + height +
        "\nnpoints = " + npoints + "\n");

  for (i = 0; i < npoints; i++) {
    Print("xpoints[" + i + "] = " + xpoints[i] + ", ypoints[" + i +
          "] = " + xpoints[i] + "\n");
  }

  Print("\nChecking bounds...\n");

  for (i = 0; i < npoints; i++) {
    if (xpoints[i] > width)
      Print("Error: xpoint[" + i + "] = " + xpoints[i] +
            " which is greater than width!!!\n");
    else if (xpoints[i] < 0)
      Print("Error: xpoint[" + i + "] = " + xpoints[i] +
            " which is less than 0!!!\n");

    if (ypoints[i] > height)
      Print("Error: ypoint[" + i + "] = " + ypoints[i] +
            " which is greater than height!!!\n");
    else if (ypoints[i] < 0)
      Print("Error: xpoint[" + i + "] = " + ypoints[i] +
            " whichis less than 0!!!\n");
  }
  Print("\n\n");
}

void GeneticPolygon::setupPoints(ISC numberOfPoints) {
  if (numberOfPoints < 3) {
    Print("Error: numberOfPoints < 3!\n");
    numberOfPoints = 3;
  }

  FPD deltaAngle = (2.0 * Math.PI) / (FPD)numberOfPoints, rotationAngle,
      cosAngle, sinAngle, stretchFactor, doubleWidth = (FPD)width,
      doubleHeight = (FPD)height, halfWidth = doubleWidth / 2.0,
      halfHeight = doubleHeight / 2.0, x, y;

  if (width > height) {
    stretchFactor = doubleWidth /
                    doubleHeight;  // We want stretchFactor to be greater than 1

    for (ISC i = 0; i < numberOfPoints; i++) {
      rotationAngle = i * deltaAngle;

      cosAngle = Math.cos(rotationAngle);
      sinAngle = Math.sin(rotationAngle);

      x = halfWidth + (halfHeight * cosAngle) * stretchFactor;
      y = halfHeight + (halfHeight * sinAngle);

      addPoint((ISC)x, (ISC)y);
    }
  } else {
    stretchFactor = doubleHeight / doubleWidth;

    for (ISC i = 0; i < numberOfPoints; i++) {
      rotationAngle = i * deltaAngle;

      cosAngle = Math.cos(rotationAngle);
      sinAngle = Math.sin(rotationAngle);

      x = halfWidth + (halfWidth * cosAngle);
      y = halfHeight + (halfWidth * sinAngle) * stretchFactor;

      addPoint((ISC)x, (ISC)y);
    }
  }

  rebound();
}

BOL GeneticPolygon::age(ISC ageTime) {
  age += ageTime;

  if (age >= lifespan) return false;

  return true;
}

Color GeneticPolygon::mixColors(Color motherColor, Color fatherColor) {
  float[] motherHSB = new float[3], fatherHSB = new float[3];

  Color.RGBtoHSB(motherColor.getRed(), motherColor.getGreen(),
                 motherColor.getBlue(), motherHSB);
  Color.RGBtoHSB(fatherColor.getRed(), fatherColor.getGreen(),
                 fatherColor.getBlue(), fatherHSB);
  return new Color(Color.HSBtoRGB((motherHSB[0] + fatherHSB[0]) / 2,
                                  (motherHSB[1] + fatherHSB[1]) / 2,
                                  (motherHSB[2] + fatherHSB[2]) / 2));
}

Color GeneticPolygon::breedColor(Color motherColor, Color fatherColor,
                                 Color matGrandColor, Color patGrandColor) {
  return mixColors(motherColor, fatherColor);
}

ISC GeneticPolygon::mutateInt(ISC thisValue, ISC mutateDirection) {
  FPD mutation,    //< The
      TArray,      // The TArray tells you if its getting bigger or smaller
      scalar = 0;  // Tells you how much of a mutation there will be.

  // Output = thisValue * (1 + scallar * trend)

  if (mutateDirection > 0) {
    TArray = 1.0;
  } else if (mutateDirection == 0) {
    if (Math.random() < 0.5)
      TArray = 1.0;
    else
      TArray = -1.0;
  } else  // mutateDirection
  {
    TArray = -1.0;
  }

  mutation = Math.random();

  if (mutation < minorMutation) {
    if (mutation < minorMutation) {
      if (mutation < majorMutation)
        scalar = majorScalar;
      else
        scalar = minorScalar;
    }
  }

  ISC newValue = (ISC)((FPD)thisValue * (1.0 + TArray * scalar));

  return newValue;
}

FPD GeneticPolygon::mutateFloat(FPD thisValue, ISC mutateDirection) {
  FPD mutation,
      TArray,      // The TArray tells you if its getting bigger or smaller
      scalar = 0;  // Tells you how much of a mutation there will be.

  // Output = thisValue * (1 + scallar*trend)

  if (mutateDirection > 0) {
    TArray = 1.0f;
  } else if (mutateDirection == 0) {
    if (Math.random() < 0.5)
      TArray = 1.0;
    else
      TArray = -1.0;
  } else  // mutateDirection
  {
    TArray = -1.0;
  }

  mutation = Math.random();

  if (mutation < minorMutation) {
    if (mutation < minorMutation) {
      if (mutation < majorMutation)
        scalar = majorScalar;
      else
        scalar = minorScalar;
    }
  }
  return (FPD)((FPD)thisValue * (1.0 + TArray * scalar));
}

ISC GeneticPolygon::breedInt(ISC motherInt, ISC fatherInt, ISC matGrandInt,
                             ISC patGrandInt) {
  // The way that this algorithm works is that we are trying to mimic nature.
  // First we need to determine if we are going to grow bigger or smaller
  // the the parents. The idea is that we need to see what the reproductive
  // trend is. If the grandparents was greater than its offspring, then that
  // shows that there is trend of having larger offspring. If both grandparents
  // were larger, then the offspring will be larger.

  ISC largerValue, smallerValue;

  // Step one: Determine which is the larger and smaller value.

  if (motherInt > fatherInt) {
    largerValue = motherInt;
    smallerValue = fatherInt;
  } else {
    largerValue = fatherInt;
    smallerValue = motherInt;
  }

  if (motherInt > matGrandInt) {
    if (fatherInt >
        patGrandInt)  // Then both grandparents were bigger so grow bigger.
    {
      return mutateInt(largerValue, 1);
    } else {
    }
  } else  // Then mother getting smaller
  {
    if (fatherInt <
        patGrandInt)  // Then both grandparents were smaller so getting smaller
    {
      return mutateInt(smallerValue, -1);
    }
  }

  return mutateInt(smallerValue, 0);
}

BOL GeneticPolygon::containsGeneticPolygon(GeneticPolygon that) {
  // The theory behind this algorithm is that in order for two polygons to
  // intersect, at least one point of either object has to be inside of the
  // other.

  // Currently I am unaware of how Java does its contains (ISC, ISC) function so
  // I have absolutly no idea how efficient this is. The other method would be
  // to compair their bitmaps. This might be able to be done in hardware via
  // OpenCL.

  Print("BOL contains (GeneticPolygon that)\n");
  ISC i;

  for (i = 0; i < this.npoints; i++)
    if (this.contains(xpoints[i], ypoints[i])) return false;

  for (i = 0; i < that.npoints; i++)
    if (that.contains(xpoints[i], ypoints[i])) return false;

  return true;
}

FPD GeneticPolygon::breedFloat(FPD motherInt, FPD fatherInt, FPD matGrandInt,
                               FPD patGrandInt) {
  return 0.0f;
}

/*  @brief  Function that returns the BufferedImage for this this.

 */

BufferedImage GeneticPolygon::getBitmap() { return bitmap; }

ISC GeneticPolygon::Width() { return width; }

ISC GeneticPolygon::Height() { return height; }

FPD GeneticPolygon::getAngle() { return angle; }

Color GeneticPolygon::getColor() { return color; }

long GeneticPolygon::getNumPixels() { return numPixels; }

ISC GeneticPolygon::getLifespan() { return lifespan; }

ISC GeneticPolygon::getNumPoints() { return this.npoints; }

// Function that prints out the this to a bitstream

void GeneticPolygon::printGenes(ISC[] thisStream, ISC currentIndex) {
  thisStream[currentIndex] = width;
  thisStream[currentIndex + 1] = height;
  thisStream[currentIndex + 2] = (ISC)angle;
  thisStream[currentIndex + 5] = color.getRGB();
  thisStream[currentIndex + 6] = (ISC)this.npoints;

  currentIndex += 7;

  for (ISC i = 0; i < super.npoints; i++) {
    thisStream[currentIndex] = xpoints[i];
    thisStream[currentIndex + 1] = ypoints[i];
    currentIndex += 2;
  }
}

/* @brief   Function that compares
 */

BOL GeneticPolygon::intersects(Polygon thatPolygon) {
  Rectangle bounds = thatPolygon.getBounds();

  ISC offsetX = (width - bounds.width) / 2,
      offsetY = (height - bounds.height) / 2;

  // Print ("offsetX = " + offsetX + ", offsetY = " + offsetY + "\n");
  return intersects(thatPolygon, offsetX, offsetY);
}

BOL GeneticPolygon::intersects(GeneticPolygon thatPolygon) {
  ISC offsetX = (width - thatPolygon.width) / 2,
      offsetY = (height - thatPolygon.height) / 2;

  // Print ("offsetX = " + offsetX + ", offsetY = " + offsetY + "\n");
  return intersects(thatPolygon, offsetX, offsetY);
}

BOL GeneticPolygon::intersects(Polygon thatPolygon, FPD offsetX, FPD offsetY) {
  return intersects(thatPolygon, (ISC)offsetX, (ISC)offsetY);
}

BOL GeneticPolygon::intersects(Polygon thatPolygon, ISC offsetX, ISC offsetY) {
  // In order to determine if one Polygons ccontain eachother, we have to check
  // every point in both Polygon(s).

  if (thatPolygon == null) return false;

  for (ISC i = 0; i < thatPolygon.npoints; i++) {
    if (super.contains(thatPolygon.xpoints[i] + offsetX,
                       thatPolygon.ypoints[i] + offsetX)) {
      return true;
    }
  }
  for (ISC i = 0; i < npoints; i++) {
    if (thatPolygon.contains(xpoints[i] - offsetX, ypoints[i] - offsetY))
      return true;
  }
  return false;
}

void GeneticPolygon::equalizePoints() {
  FPD centerX = (FPD)width / 2.0, centerY = (FPD)height / 2.0,
      deltaY = ypoints[1] - ypoints[0], deltaX = xpoints[1] - xpoints[0],
      deltaAngle = (2 * Math.PI) / (FPD)this.npoints,
      initAngle = Math.atan(deltaY / deltaX), currentAngle, idealAngle,
      rotationAngle, x, y, cosAngle, sinAngle;

  for (ISC i = 0; i < this.npoints - 1; i++) {
    // Step 1: find angle between the current and next point.

    x = (FPD)xpoints[i];
    y = (FPD)ypoints[i];

    deltaX = xpoints[i + 1] - x;
    deltaY = ypoints[i + 1] - y;
    currentAngle = Math.atan((deltaY / deltaX));

    // Step 2: figure out what the angle should be.

    idealAngle = i * deltaAngle + initAngle;

    // Step 3: rotate the point closer to the

    if (currentAngle < idealAngle)
      rotationAngle = (FPD)angularMutation;
    else if (currentAngle > idealAngle)
      rotationAngle = (FPD)angularMutation * -1.0;
    else
      rotationAngle = 0.0;

    cosAngle = Math.cos(rotationAngle);
    sinAngle = Math.sin(rotationAngle);

    xpoints[i] = (ISC)((x + centerX) * cosAngle - (y + centerY) * sinAngle);
    ypoints[i] = (ISC)((y + centerY) * sinAngle + (y + centerY) * cosAngle);
  }

  rebound();
}

void GeneticPolygon::renderBitmap() {
  bitmap = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);

  Graphics2D g2d = bitmap.createGraphics();
  // g2d.setComposite(AlphaComposite.Clear);
  // g2d.fillRect(0, 0, width, height);

  g2d.setColor(color);
  g2d.fillPolygon(this);

  g2d.setColor(Color.BLACK);
  g2d.drawPolygon(this);

  // g2d.fillRect (0,0,width,height); //< This is to test to see if it draws
  // anything.

  // g2d.dispose();

  // Now count the number of pixels.
  ISC thisNumPixels = 0;

  for (ISC x = 0; x < width; x++)
    for (ISC y = 0; y < height; y++)
      if (contains(x, y)) thisNumPixels++;

  numPixels = thisNumPixels;

  // JOptionPane.showMessageDialog(null, new JLabel (new ImageIcon (bitmap)),
  // "About", JOptionPane.PLAIN_MESSAGE, null);
}

FPD GeneticPolygon::crossOver(FPD dominant, FPD recessive) {
  /* Cross over scheme.
      In real anamals, there are dominant and recessive genes. The dominant gene
     is represented by the most significant digit and the recessive is
     represented by the lest significant.
      


      Currently, the bitmasks were selected by intuition and should be
     reevaluated.
   */

  if (dominant > 1.0f)
    JOptionPane.showMessageDialog(null,
                                  "Error! Dominant FPD was greater than 1!!!");
  else if (dominant < 0.0f)
    JOptionPane.showMessageDialog(null,
                                  "Error! Dominant FPD was less than 0!!!");

  if (recessive > 1.0f)
    JOptionPane.showMessageDialog(null,
                                  "Error! Recessive FPD was greater than 1!!!");
  else if (recessive < 0.0f)
    JOptionPane.showMessageDialog(null,
                                  "Error! Recessive FPD was less than 0!!!");

  AString dominantString = Double.ToString(dominant),
          recessiveString = Double.ToString(recessive);

  ISC dominantStringLength = dominantString.length(),
      recessiveStringLength = recessiveString.length(),
      longestStringLength = (dominantStringLength > recessiveStringLength)
                                ? recessiveStringLength
                                : recessiveStringLength,
      i;

  CHA[] newString = new CHA[longestStringLength];

  BOL dominantTurn = true;

  for (i = 2; i < longestStringLength; i++) {
    if (dominantTurn) {
      dominantTurn = false;

      if (i < dominantStringLength)
        newString[i] = dominantString.charAt(i);
      else
        newString[i] = '0';
    } else  // resessiveTurn
    {
      dominantTurn = true;

      if (i < recessiveStringLength)
        newString[i] = recessiveString.charAt(i);
      else
        newString[i] = '0';
    }
  }

  return Float.parseFloat(new AString(newString));
}

void GeneticPolygon::rotate() {
  // function uses standard 2D rotation matrix.

  FPD cosAngle = Math.cos((FPD)angle), sinAngle = Math.sin((FPD)angle), x, y;

  for (ISC i = 0; i < this.npoints; i++) {
    x = (FPD)xpoints[i];
    y = (FPD)ypoints[i];

    xpoints[i] = (ISC)(x * cosAngle - y * sinAngle);
    ypoints[i] = (ISC)(x * sinAngle + y * cosAngle);
  }
  rebound();
}

FPD GeneticPolygon::boundBetween0and1(FPD inputValue) {
  if (inputValue < 0)
    return 0;

  else if (inputValue > 1)
    return 1;

  return inputValue;
}

FPD GeneticPolygon::randomizeDouble(FPD inputValue) {
  FPD scalar = 0.0, randomNumber = Math.random();

  if (randomNumber < majorMutation) {
    scalar = 1.0 + Math.random() * majorScalar;
  } else
    scalar = 1.0 + Math.random() * minorScalar;

  FPD newValue = inputValue * scalar;

  return newValue;
}

ISC GeneticPolygon::randomizeInt(ISC inputValue) {
  if (inputValue < 0)
    inputValue = inputValue * -1;  //< Chop off the negative values.

  FPD scalar = 1.0 + (Math.random() * 2.0 - 1.0) * minorScalar;

  ISC newValue = (ISC)((FPD)inputValue * scalar);

  return newValue;
}

}  //< namespace _
