// Copyright AStarship <https://astarship.net>.

#include <_Config.h>

#include "c_iGeek.h"

namespace _ {

Cell_IGA::Cell_IGA() { add(new LeukocyteWar()); }

static void Cell_IGA::main(AString[] args) {
  JFrame window = new JFrame("Kabuki.Cell_IGA");
  window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

  JScrollPane scrollableArea = new JScrollPane(new LeukocyteWar());

  window.setContentPane(scrollableArea);

  window.pack();
  window.setVisible(true);
}

}  //< namespace _
}
