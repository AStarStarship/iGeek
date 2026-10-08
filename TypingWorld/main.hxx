// Copyright AStarship <https://astarship.net>.

#include "World.h"
#include "_Config.h"

CHA ScanKeyboard() {
  CHA key = 127;

  key = CIn.Get();

  if (key == 0 || key == -32) { // get a special key
    key = _getch();

    if (key == 72) { // up arrow
      key = 'u';
    } else if (key == 75) { // left arrow
      key = 'l';
    } else if (key == 77) { // right
      key = 'r';
    } else if (key == 80) { // down
      key = 'd';
    }
  }
  return key;
}

void FrameUpdateHandler() { ScanKeyboard(); }

ISN main() {
  ISN columns, rows;

  cout << "Welcome to Typecraft!";

  while (1) {
    
  }
  return 0;
}
