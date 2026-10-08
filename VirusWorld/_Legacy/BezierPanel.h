// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>

namespace _ {

/* This class draws a BezierCurve drawn on a JPanel. */
class BezierCurvePanel {
  static const ISC defaultNumPoints = 4,  //< The default number of points.
      defaultResolution = 1000,           //< The default resolution.
      defaultPadding = 20,                //< The default padding.
      defaultSize = 200;                  //< The default size of the

  static const double defaultDeltaT =
      1 / (double)defaultResolution;  //< The default increment in the path to
                                      // iterate when drawing.

  static const Color defaultCurveColor =
                         Color.red,      //< The default of of the curve.
      defaultBorderColor = Color.white;  //< The border color.

  BezierCurvePanel(ISC thisX, ISC thisY, BezierCurve thisCurve,
                   Color thisBackgroundColor, Color thisCurveColor,
                   Color thisBorderColor);

  ISC getNumPoints();

  void draw(Graphics g);

  void drawCubic(Graphics g);

 private:
  ISC numPoints,         //< The number of bezier points.
      resolution,        //< The path resolution.
      padding,           //< The panel padding.
      doublePadding,     //< Double to padding for calculation acceleration.
      x,                 //< The x position of this panel on the screen.
      y,                 //< The y position of this panel on the screen.
      width,             //< The width of this panel.
      height,            //< The height of this panel.
      pointRadius,       //< The radius of the point to draw.
      twicePointRadius;  //< Twice the pointRaidus.

  double deltaT,   //< The increment in the path to iterate when drawing.
      curveScale,  //< The scale of the Bezier path.
      /*< The Bezier Curve path is a scalar path bounced from 0<x<1 and -1<y<1.
         This determines how far 1 is. */
      x1,  //< The second x point.
      y1,  //< The second y point.
      x2,  //< The third x point.
      y2;  //< The third y point.

  BezierCurve curve;  //< The bezier curve.

  Color backgroundColor,  //< The background color.
      curveColor,         //< The color of the curve.
      borderColor,        //< The color of the border.
      axesColor,          //< The color to draw the axes.
      pointColor;         //< The color of the points to draw.

  double[] points;  //< The bazier points.
};

}  //< namespace Homo
}  //< namespace World
}  //< namespace iGeek
