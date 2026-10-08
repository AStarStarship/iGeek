// Copyright AStarship <https://astarship.net>.

#include <_Config.h>

#ifndef INCLUDED_KABUKI_AI_MAZEAGENT
#define INCLUDED_KABUKI_AI_MAZEAGENT 1

namespace _ {

// Function pointer for getting static mazes and their height.
typedef const CHA** (*MazeLoader)(ISC& height);

// Example maze with function pointer.
const CHA** Maze1(ISC& height);

CHA* PreprocessMaze(MazeLoader get_maze, ISC& width, ISC& height);

/*
# Allowed Moves
Our ASCII robot car takes up 2 characters that cannot be diagonal. */
struct Maze {
  CHA* tiles;
  ISC width, height, number;

  Maze(MazeLoader get_maze = Maze1, ISC number = 1);
  Maze(CHA* tiles, ISC width, ISC height, ISC number);
  BOL IsValid();
  ~Maze();
  CHA* Get(ISC x, ISC y);
  BOL Set(ISC x, ISC y, CHA c);
  BOL Move(ISC x, ISC y, ISC new_x, ISC new_y);
  BOL CanMove(ISC x, ISC y);
  BOL IsEnd(ISC x, ISC y);
  BOL Find(ISC& x, ISC& y, CHA c);
  BOL FindStart(ISC& x, ISC& y, ISC& dx, ISC& dy);
  BOL FindFinish(ISC& x, ISC& y);
  void Print(ISC iteration = 0);
};

enum {
  kXYRight = 0,   //< Direction 0: Right @ 0 degrees.
  kXYUpperRight,  //< Direction 1: Upper Right @ 45 degrees.
  kXYUp,          //< Direction 2: Up @ 90 degrees.
  kXYUpperLeft,   //< Direction 3: Upper Left @ 135 degrees.
  kXYLeft,        //< Direction 4: Left @ 180 degrees.
  kXYLowerLeft,   //< Direction 5: Lower Left @ 225 degrees.
  kXYDown,        //< Direction 6: Down @ 270 degrees.
  kXYLowerRight,  //< Direction 7: Lower Right @ 315 degrees.
  kXYCenter,      //< Direction 7: Center @ i degrees.
};

const CHA* XYDirectionAcronyms();

/* Decodes an XY quadrant packed in +/0/- format.
@param quad The quadrant.
@param value If value < 0 then only the lower angle node has been traversed. If
value == 0 then only the higher value node has been traversed. If value > 0 then
both nodes have been traversed. */
const CHA* XYDirectionString(ISC direction);

const CHA* XYDirectionAcronyms();

void XYDirectionHistoryPrint(ISC bits);

/* Gets the dx and dy values for the given direction.
@return DeltaX.
@param  a  The a-axis.
@param  dy DeltaY. */
ISC XYVector(ISC a, ISC& dy);

/* Gets the direction based on in the dx and dy.
@return A direction.
@param  dx Delta x.
@param  dy Delta y. */
ISC XYDirection(ISC dx, ISC dy);

struct MazeAgent;

typedef ISC (*PolicyPilot)(MazeAgent* agent);

ISC PolicyPilotManual(MazeAgent* agent);

ISC PolicyPilotDepthFirstRoundRobin(MazeAgent* agent);

/* Simple stack AI agent for solving simple mazes with 8 directions.
In order to better pack the data, the visited directions in the stack search
are that each quadrant contains two paths where quad_n is zero if neither path
has been traversed, it's -1 if the lower value angle path has been visited and
+1 if both nodes have been visited.
Speed will always be either -1, 0, or +1, and direction will always be
*/
struct MazeAgent {
  enum { kStackSize = 1024 };

  struct State {
    enum {
      kShutdown = 0,  //< State 0: Shutting Down.
      kBooting,       //< State 1: booting.
      kWaiting,       //< Not doing anything.
      kTurning,       //< Turning.
      kMoving,        //< Moving without stepping.
      kStuck,         //< Stuck on this path.
      kError,         //< Can't find a solution.
    };

    static const CHA* Label(ISC state);

    ISC state,    //< State variable.
        x,        //< X-axis position.
        y,        //< Y-axis position.
        a,        //< A-axis position.
        speed_y,  //< Speed in the Y-axis.
        speed_a,  //< Speed in the X-axis.
        history;  //< History of which of the 8
    State();
    State(const State& other);
    void Set(ISC new_x, ISC new_y, ISC new_a, ISC new_speed_a = 0,
             ISC new_speed_y = 0, ISC new_state = kBooting,
             ISC new_history = 0);
    const CHA* Label();
    void Print();
    BOL Contains(ISC other_x, ISC other_y);
  };

  struct Point2D {
    ISC x, y;
    Point2D();
    Point2D(State& state);
    Point2D(const Point2D& other);
  };

  PolicyPilot pilot, autopilot;
  State state, next_state, states[kStackSize];
  ISC stack_height, iteration, shortest_height, init_dx, init_dy;
  Maze maze;
  Point2D shortest_path[kStackSize];

  MazeAgent(PolicyPilot pilot = PolicyPilotManual, MazeLoader get_maze = Maze1,
            ISC maze_number = 1);
  BOL IsValid();
  BOL ChangeState(ISC new_state);
  BOL Move(ISC x, ISC y, ISC new_x, ISC new_y);
  BOL IsLoop(ISC new_x, ISC new_y);
  ISC Update();
  void Turn();
  void TurnLeft();
  void TurnRight();
  void AutopilotToggle();
  void Decelerate();
  void Accellerate();
  void ShutDown();
  void SetAutopilot(PolicyPilot new_pilot);
  BOL IsOnAutopilot();
  void Print();
};

ISC PolicyPilotManual(MazeAgent* agent);
ISC PolicyPilotDepthFirstRoundRobin(MazeAgent* agent);

}  // namespace _

#endif  //< #ifndef INCLUDED_KABUKI_AI_MAZEAGENT
