// Copyright AStarship <https://astarship.net>.

#include <pch.h>

namespace _ {

#include "c_geneticpolygontest.h"

GeneticPolygonTest::GeneticPolygonTest () {
  add (new GeneticPolygonTestPanel ());
}

static void main (Loom<> args)
{
  JFrame window = new JFrame ("GeneticPolygon Test App");
  window.setDefaultCloseOperation (JFrame.EXIT_ON_CLOSE);

  window.setContentPane (new GeneticPolygonTestPanel ());

  window.pack ();
  window.setVisible (true);
}

}
