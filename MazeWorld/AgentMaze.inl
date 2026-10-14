// Copyright AStarship <https://astarship.net>.

// This file replaces the Script2-era AgentMaze.inl, which contained
// the original IGEEK Autopilot maze-agent implementation (2014-2019,
// kabuki-toolkit, UTF-16LE, Windows/MSVC, CH1/CH4 types, conio.h,
// windows.h): KeyControl, XYDirectionAcronyms, the MazeAgent state
// machine, and the pathfinder loop.
//
// All of that logic is now implemented in the current world:
//
//   MazeWorld.h / MazeWorld.hxx   maze grid, LCG, A* solver
//   _Main.cpp                     interactive TUI, COut/CIn console
//   AgentMaze.h                   legacy agent interface (see header)
