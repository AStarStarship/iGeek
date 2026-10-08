// Copyright AStarship.

#include <pch.h>

namespace _ {

CellEvolutionaryInterface::CellEvolutionaryInterface() {
  setPreferredSize(new Dimension(defaultWidth, defaultHeight));
  setLayout(new BorderLayout());
  addMouseListener(this);
  addKeyListener(this);
  setFocusable(true);

  timer = new Timer(timerUpdateInterval, new TimerAction());

  toolbar = new Toolbar();

  add(toolbar, BorderLayout.SOUTH);

  Initialize();

  currentVirus = viruses.getVirus(viruses.selectRandomVirus());
  currentVirus.setDrawTrail(true);
  currentVirus.setFillCircle(true);

  curvePanelX = 1000;
  curvePanelY = 30;

  drawCurrentBezierCurve = false;
  drawCurrentTrail = false;

  curvePanel = new BezierCurvePanel(curvePanelX, curvePanelY,
                                    currentVirus.getBezierCurve(), Color.white,
                                    Color.blue, Color.red);
}

void CellEvolutionaryInterface::Initialize() {
  host =
      new Host(defaultWidth, defaultHeight, defaultNumCells, defaultNumViruses);
  viruses = host.virusPopulation();

  timerOn = true;
  timer.start();
}

void CellEvolutionaryInterface::paintComponent(Graphics g) {
  super.paintComponent(g);

  host.paintComponent(g);

  if (curvePanel != null && drawCurrentBezierCurve) curvePanel.draw(g);
}

class Toolbar : public JPanel {
  Toolbar() {
    JButton startButton = new JButton("Start");
    JButton stopButton = new JButton("Stop");

    startButton.addActionListener(new StartAction());
    stopButton.addActionListener(new StopAction());

    // JButton nextButton = new JButton ("Next");

    // nextButton.setMnemonic(KeyEvent.VK_N);
    // nextButton.addActionListener(new NextAction ());
    // nextButton.setPreferredSize (new Dimension(150, 40));
    // nextButton.setAlignmentX (Component.CENTER_ALIGNMENT);
    // nextButton.setToolTipText("Click to iterate to the next generation.");

    add(startButton);
    add(stopButton);
  }
}

class TimerAction : public ActionListener {
  void actionPerformed(ActionEvent e) {
    if (timerOn) {
      host.Update();
    }
    repaint();
  }
}

class StartAction : public ActionListener {
  void actionPerformed(ActionEvent e) { timerOn = true; }
}

class StopAction : public ActionListener {
  void actionPerformed(ActionEvent e) { timerOn = false; }
}

void CellEvolutionaryInterface::mousePressed (MouseEvent e)
{
  currentVirusIndex = viruses.selectVirus(e.getX(), e.getY());

  if (currentVirusIndex <
      0)  // Then there was not virus under the mouse cursor.
    return;

  Virus newSelectedVirus = viruses.getVirus(currentVirusIndex);

  if (newSelectedVirus == null) return;

  if (currentVirus != null) currentVirus.deselect();

  currentVirus = newSelectedVirus;
  currentVirus.select();

  curvePanel = new BezierCurvePanel(curvePanelX, curvePanelY,
                                    currentVirus.getBezierCurve(), Color.white,
                                    Color.blue, Color.red);
}

void CellEvolutionaryInterface::mouseReleased(MouseEvent e) { ; }

void CellEvolutionaryInterface::mouseEntered(MouseEvent e) { ; }

void CellEvolutionaryInterface::mouseExited(MouseEvent e) { ; }

void CellEvolutionaryInterface::mouseClicked(MouseEvent e) { ; }

void CellEvolutionaryInterface::keyTyped(KeyEvent e) {
  // Print ("keyTyped = " + e.getKeyChar() + "\n");
}

void CellEvolutionaryInterface::keyPressed(KeyEvent e) {
  CHA key = e.getKeyChar();

  if (key == 'c' || key == 'C') {
    if (drawCurrentBezierCurve)
      drawCurrentBezierCurve = false;
    else
      drawCurrentBezierCurve = true;
  }

  if (key == 't' || key == 'T' || currentVirus == null) {
    if (drawCurrentTrail) {
      drawCurrentTrail = false;
      currentVirus.setDrawTrail(false);
    } else {
      drawCurrentTrail = true;
      currentVirus.setDrawTrail(true);
    }
  }

  if (key == 'k' || key == 'K') {
    kill = true;
  }
}

void CellEvolutionaryInterface::keyReleased(KeyEvent e) {
  CHA key = e.getKeyChar();

  if (key == 'k' || key == 'K') {
    kill = false;
  }
}

}  //< namespace _
}
