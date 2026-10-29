// Copyright AStarship <https://astarship.net>.

#include <_Config.h>

#include "c_mazeagent.h"

#include <conio.h>
#include <iostream>
#include <windows.h>
using namespace std;

namespace iGeek {

KeyControl::KeyControl(VKCode code, const CH1* label)
    : code(code), label(label) {}

void KeyControl::Reprogram() { PrintLn(kLF); }

const CH1* XYDirectionAcronyms() {
  // Direction acronyms aligned to a 4-bit boundary.
  static const CH1 kDirectionAcronyms[] = {
      'R', 0,   0, 0,  //
      'U', 'R', 0, 0,  //
      'U', 0,   0, 0,  //
      'U', 'L', 0, 0,  //
      'L', 0,   0, 0,  //
      'L', 'L', 0, 0,  //
      'D', 0,   0, 0,  //
      'L', 'R', 0, 0,  //
      'C', 0,   0, 0,  //
  };
  return kDirectionAcronyms;
}

const CH1* XYDirectionString(SI4 direction) {
  if (direction < 0 || direction >= kXYCenter) direction = kXYCenter;
  return &XYDirectionAcronyms()[direction << 2];  //< << 2 to * 4.
}

void XYDirectionHistoryPrint(SI4 bits) {
  const CH1* dir_Strings = XYDirectionAcronyms();
  for (SI4 i = 0; i < 8; ++i) {
    if ((bits >> i) & 1) {
      Print(&XYDirectionAcronyms()[i]);
      if (i != 7) Print(kLF, ' ');
    }
  }
}

const CH1** Maze1(SI4& height) {
  static const CH1* kMaze[] = {
      "         +--+",  //
      "+--------+  |",  //
      "| @         |",  //
      "| *+-- --+  |",  //
      "|  |     |  |",  //
      "|  |     |  |",  //
      "|  | +-+ |  |",  //
      "|  |X| | |  |",  //
      "|  +-+ | |  |",  //
      "|      | |  |",  //
      "|  +---+ |  |",  //
      "|  |        |",  //
      "|  |        |",  //
      "|  +-----   |",  //
      "|           |",  //
      "+-----------+",  //
  };
  height = 16;
  return kMaze;
}

CH1* PreprocessMaze(MazeLoader get_maze, SI4& width, SI4& height) {
  cout << "\n1.) Load the maze from a GetMaze function.";
  SI4 y;
  const CH1** Strings = get_maze(y);
  const CH1* read = Strings[0];
  SI4 x = 0;
  while (*read++) ++x;
  cout << "\n  width:" << x << " height:" << y << kLF;
  if (!x || !y) {
    cout << "\nERROR: width and/or height can't be zero!";
    return nullptr;
  }
  width = x;
  height = y;

  CH1 *tiles = new CH1[x * y], *cursor = tiles;
  cout << "\n2.) Packing Strings into maze of size " << x * y << ".\n";
  for (SI4 j = 0; j < y; ++j) {
    cout << kLF;
    read = Strings[j];
    for (SI4 k = 0; k < x; ++k) {
      CH1 c = (CH1)ToUpper(*read++);
      cout << c;
      *cursor++ = c;
    }
  }
  cout << kLF;
  return tiles;
}

Maze::Maze(MazeLoader get_maze, SI4 number) : number(number) {
  tiles = PreprocessMaze(get_maze, width, height);
}

Maze::Maze(CH1* tiles, SI4 width, SI4 height, SI4 number)
    : tiles(tiles), width(width), height(height), number(number) {}

Maze::~Maze() {
  if (tiles) delete[] tiles;
}

BOL Maze::IsValid() {
  return tiles == nullptr && width > 0 && height > 0 && number > 0;
}

CH1* Maze::Get(SI4 x, SI4 y) {
  if (x < 0 || y < 0 || x >= height || y >= height) return 0;
  return &tiles[y * width + x];
}

BOL Maze::Set(SI4 x, SI4 y, CH1 c) {
  if (x < 0 || y < 0 || x >= height || y >= height) return false;
  tiles[y * width + x] = c;
  return true;
}

BOL Maze::Move(SI4 x, SI4 y, SI4 new_x, SI4 new_y) {
  SI4 l_width = width;
  if (x < 0 || x >= l_width || y < 0 || y >= height || new_x < 0 ||
      new_x >= l_width || new_y < 0 || new_y >= height)
    return false;

  CH1* read = &tiles[y * width + x];
  CH1 c = *read;
  *read = ' ';
  CH1* write = &tiles[new_y * width + new_x];
  *write = c;
  return true;
}

BOL Maze::CanMove(SI4 x, SI4 y) { return (*Get(x, y)) == ' '; }

BOL Maze::IsEnd(SI4 x, SI4 y) {
  CH1 tile = *Get(x, y);
  return tile == 'F';
}

BOL Maze::Find(SI4& x, SI4& y, CH1 c) {
  if (c >= 'a' && c <= 'z') c -= 'z' - 'Z';
  CH1* cursor = tiles;
  for (SI4 j = 0; j < height; ++j) {
    for (SI4 k = 0; k < width; ++k) {
      CH1 d = *cursor++;
      if (c == d) {
        x = k;
        y = j;
        *(--cursor) = ' ';  //< Remove the CH1.
        return true;
      }
    }
  }
  x = y = -1;
  return false;
}

BOL Maze::FindStart(SI4& x, SI4& y, SI4& dx, SI4& dy) {
  if (!Find(x, y, '@')) return false;
  if (!Find(dx, dy, '*')) return false;
  dx = x - dx;
  dy = y - dy;
  if (dx < -1 || dx > 1 || dy < -1 || dy > 1) return false;
  return true;
}

BOL Maze::FindFinish(SI4& x, SI4& y) { return Find(x, y, 'F'); }

void Maze::Print(SI4 iteration) {
  cout << "\nMaze " << number << "\nIteration:" << iteration
       << " width:" << width << " height:" << height;
  CH1* cursor = tiles;
  for (SI4 y = 0; y < height; ++y) {
    cout << kLF;
    for (SI4 x = 0; x < width; ++x) {
      CH1 c = *cursor++;
      if (!c) break;
      cout << c;
    }
  }
}

SI4 XYVector(SI4 direction, SI4& dy) {
  switch (direction) {
    case kXYRight: {
      dy = 0;
      return 1;
    }
    case kXYUpperRight: {
      dy = -1;
      return 0;
    }
    case kXYUp: {
      dy = 0;
      return 0;
    }
    case kXYUpperLeft: {
      dy = -1;
      return -1;
    }
    case kXYLeft: {
      dy = 0;
      return -1;
    }
    case kXYLowerLeft: {
      dy = 1;
      return -1;
    }
    case kXYDown: {
      dy = 1;
      return 0;
    }
    case kXYLowerRight: {
      dy = 1;
      return 1;
    }
  }
  dy = 0;
  return 0;
}

SI4 XYDirection(SI4 dx, SI4 dy) {
  if (dx == 0) return dy < 0 ? kXYUp : kXYDown;
  if (dy == 0) return dx < 0 ? kXYLeft : kXYRight;
  if (dx < 0) return dy < 0 ? kXYLowerLeft : kXYUpperLeft;
  return dy < 0 ? kXYLowerRight : kXYUpperRight;
}

MazeAgent::State::State() {}

MazeAgent::State::State(const State& other) {
  state = other.state;
  x = other.x;
  y = other.y;
  a = other.a;
  speed_y = other.speed_y;
  speed_a = other.speed_a;
  history = other.history;
}

void MazeAgent::State::Set(SI4 new_x, SI4 new_y, SI4 new_a, SI4 new_speed_a,
                           SI4 new_speed_y, SI4 new_state, SI4 new_history) {
  state = new_state;
  x = new_x;
  y = new_y;
  a = new_a;
  speed_y = new_speed_y;
  speed_a = new_speed_a;
  history = new_history;
}

BOL MazeAgent::State::Contains(SI4 other_x, SI4 other_y) {
  return x == other_x && y == other_y;
}

const CH1* MazeAgent::State::Label(SI4 state) {
  switch (state) {
    case 0:  // kShutdown:
      return "Shutting down";
    case 1:  // kBooting:
      return "Booting";
    case kWaiting:
      return "Waiting";
    case kTurning:
      return "Turning";
    case kMoving:
      return "Moving";
    case kStuck:
      return "Stuck";
  }
  return "Error";
}

const CH1* MazeAgent::State::Label() { return Label(state); }

void MazeAgent::State::Print() {
  cout << "\nState: " << Label() << " x:" << x << " y:" << y << " a:" << a
       << " speed_y:" << speed_y << " speed_a:" << speed_a
       << "\nNodes traversed: ";
  XYDirectionHistoryPrint(history);
}

MazeAgent::Point2D::Point2D() {}

MazeAgent::Point2D::Point2D(State& state) : x(state.x), y(state.y) {}

MazeAgent::Point2D::Point2D(const Point2D& other) : x(other.x), y(other.y) {}

MazeAgent::MazeAgent(PolicyPilot pilot, MazeLoader get_maze, SI4 maze_number)
    : state(),
      pilot(pilot),
      autopilot(pilot),
      stack_height(0),
      iteration(0),
      shortest_height(kStackSize),
      maze(get_maze, maze_number) {
  SI4 x, y, dx, dy;
  maze.FindStart(x, y, dx, dy);
  state.Set(x, y, dx, dy, XYDirection(dx, dy));
}

BOL MazeAgent::IsValid() { return maze.IsValid(); }

BOL MazeAgent::ChangeState(SI4 new_state) {
  SI4 l_stack_height = stack_height;
  if (l_stack_height >= kStackSize) return false;

  switch (new_state) {
    case State::kWaiting: {
      state.speed_a = -1;
      state.speed_y = 0;
      return true;
    }
  }
  return false;
}

BOL MazeAgent::Move(SI4 x, SI4 y, SI4 new_x, SI4 new_y) {
  if (!maze.Set(x, y, ' ') || !maze.Set(new_x, new_y, 'S')) return false;
  maze.Print(++iteration);
  return true;
}

BOL MazeAgent::IsLoop(SI4 new_x, SI4 new_y) {
  for (SI4 i = stack_height - 1; i > 0; --i) {
    if (states[i].Contains(new_x, new_y)) {
      return true;
    }
  }
  return false;
}

SI4 MazeAgent::Update() {
  switch (state.state) {
    case State::kShutdown: {
      return State::kShutdown;
    }
    case State::kBooting: {
      return State::kWaiting;
    }
    case State::kWaiting: {
      return State::kWaiting;
    }
    case State::kTurning: {
      return State::kTurning;
    }
    case State::kMoving: {
      return State::kMoving;
    }
    case State::kStuck: {
      return State::kStuck;
    }
    case State::kError: {
      return State::kError;
    }
  }
  SI4 l_state = pilot(this);
  ChangeState(l_state);
  return l_state;
}

void MazeAgent::Turn() {
  SI4 l_a = state.a + state.speed_a;
  if (l_a < 0)
    l_a = kXYLowerRight;
  else if (l_a > kXYLowerRight)
    l_a = kXYRight;

  SI4 dy, dx = XYVector(l_a, dy), x = state.x, y = state.y, new_x = x + dx,
          new_y = y + dy;
  if (!maze.CanMove(new_x, new_y)) {
    state.speed_a = 0;
    return;
  }
  maze.Move(x, y, new_x, new_y);
}

void MazeAgent::TurnLeft() { state.speed_a = -1; }

void MazeAgent::TurnRight() { state.speed_a = 1; }

void MazeAgent::AutopilotToggle() {
  PolicyPilot manual_pilot = PolicyPilotManual;
  pilot = (pilot == manual_pilot) ? autopilot : manual_pilot;
}

void MazeAgent::Decelerate() {
  SI4 l_speed_y = state.speed_y;
  if (l_speed_y < 0) return;
  state.speed_y = l_speed_y - 1;
}

void MazeAgent::Accellerate() {
  SI4 l_speed = state.speed_y;
  if (l_speed > 0) return;
  state.speed_y = l_speed + 1;
}

void MazeAgent::ShutDown() { state.state = 0; }  // State::kShutdown; }

void MazeAgent::SetAutopilot(PolicyPilot new_pilot) {
  ASSERT(new_pilot);
  autopilot = new_pilot;
}

BOL MazeAgent::IsOnAutopilot() { return pilot != PolicyPilotManual; }

void MazeAgent::Print() {
  ::_::Print("\nAgent:");
  if (pilot == PolicyPilotManual) {
    ::_::Print(" Autopilot.");
    if (pilot == PolicyPilotDepthFirstRoundRobin) {
      ::_::Print("Depth-first Round-robin.");
    } else {
      ::_::Print("unknown.");
    }
  } else {
    ::_::Print(" Puppet.");
  }
  ::_::Print("\n state:");
  ::_::Print(state.Label());
  ::_::Print();
}

SI4 PolicyPilotManual(MazeAgent* agent) {
  return 0;  // MazeAgent::State::kShutdown;
} //< namespace iGeek

SI4 PolicyPilotDepthFirstRoundRobin(MazeAgent* agent) {
  // PEAS: Performance Measurement, Environment,
  // Actuators,
  //       Sensors
  SI4 l_state = agent->state.state;
  if (l_state <= MazeAgent::State::kWaiting) return l_state;

  if (l_state == MazeAgent::State::kWaiting) return l_state;

  SI4 x = agent->state.x, y = agent->state.y;

  if (agent->maze.IsEnd(x, y)) {
    if (agent->stack_height < agent->shortest_height) {
      agent->shortest_height = agent->stack_height;
      return MazeAgent::State::kWaiting;
      // You need to how copy the state stack
      // somewhere and so you compare this path
      // to the others to find the shortest_path.
    }
  }
  // Measurement: N/A because all movements are
  // in units of 1. Environment:
  SI4 dx = agent->state.speed_a, dy = agent->state.speed_y;
  if (agent->maze.CanMove(x + dx, y + dy)) {
    // Keep going the same direction until we hit
    // a wall.
    agent->Move(x, y, x + dx, y + dy);
    return l_state;
  }
  return ++l_state;
}

} //< namespace iGeek
