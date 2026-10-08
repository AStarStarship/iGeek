// Copyright AStarship <https://astarship.net>.

#pragma once
#include <_Config.h>

namespace _ {

class GeneticPolygonTestPanel : public JPanel {
 private:
  enum {
    panelWidth = 1024,
    panelHeight = 700,
    panelMargin = 10,
    parentPanelSize = 300,
    childPanelSize = 400,
    initPopulationSize = 100,
  };

  GeneticPolygonTestPanel();

  void initializePopulation();

  void paintComponent(Graphics g);

  void iterateGeneration();

  void mateRandom();

  void addChild(GeneticPolygon newChild);

 private:
  GeneticPolygonPanel motherPanel, fatherPanel, matGrandPanel, patGrandPanel;
  GeneticPolygonChildPanel childPanel;

  GeneticPolygon cell;

  GeneticPolygon population[];  //< The population of GeneticPolygon (s)

  ISC timerUpdateInterval =
          1000,        //< The durration of time tha the timer is updated
      populationSize;  //< The size of the pupulation.
  Timer timer;         //< The timer for the
  BOL timerOn;         //< Represents if the timer is on or off.

  class StartAction : public ActionListener {
    void actionPerformed(ActionEvent e);
  }

  class StopAction : public ActionListener {
    void actionPerformed(ActionEvent e);
  }

  class NextAction : public ActionListener {
    void actionPerformed(ActionEvent e);
  }

  class MateAction : public ActionListener {
    void actionPerformed(ActionEvent e);
  }

  class TimerAction : public ActionListener {
    void actionPerformed(ActionEvent e);
  }
};

}  //< namespace _
}
