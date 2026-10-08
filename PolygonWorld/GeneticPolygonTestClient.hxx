// Copyright AStarship <https://astarship.net>.

#include "c_geneticpolygontestapp.h"

namespace _ {

GeneticPolygonTestApp::GeneticPolygonTestApp () {
  add (new GeneticPolygonTestPanel ());
}

static void GeneticPolygonTestApp::main (Loom<> args) {
  JFrame window = new JFrame ("GeneticPolygon Test App");
  window.setDefaultCloseOperation (JFrame.EXIT_ON_CLOSE);

  window.setContentPane (new GeneticPolygonTestPanel ());

  window.pack ();
  window.setVisible (true);
}

}
