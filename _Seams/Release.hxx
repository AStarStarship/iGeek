// Copyright AStarship <https://astarship.net>.
namespace iGeekTest {
using namespace ::_;
using namespace iGeek;

inline const CHA* iGeekRelease(const CHA* args) {
  // The release seam is the final sown test pass: it runs the complete
  // tree of every world's checks in one pass.
  return TTestTree<Core, MazeWorldTest::MazeWorld, ArmWorldTest::ArmWorld,
    BikeWorldTest::BikeWorld, CarWorldTest::CarWorld,
    CardsWorldTest::CardsWorld, ClassicalTest::Classical,
    PacWorldTest::PacWorld, PolygonWorldTest::PolygonWorld,
    PrisonerWorldTest::PrisonerWorld, SpaceWorldTest::SpaceWorld,
    SudokuWorldTest::SudokuWorld, TileWorldTest::TileWorld,
    TypingWorldTest::TypingWorld,
    iGeekGymTest::Gym,
    TerminalWorldTest::TerminalWorld>(args);
}

}  //< namespace iGeekTest
