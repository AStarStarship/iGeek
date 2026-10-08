// Copyright AStarship <https://astarship.net>.

namespace iGeek {

class Host : public JPanel {
 public:
  Host(ISC width, ISC height, ISC numberOfCells, ISC numberOfViruses);

  ISC getNumCells();

  VirusPopulation virusPopulation();

  void Update();

  void paintComponent(Graphics g);

 private:
  ISC numCells;
  / < The number of Cell(s)

          Cell[] cells;
  / < The array of Cell(s)

          VirusPopulation viruses;
  / < The Virus population array.

      Color hostColor,
      / < The background color.backgroundColor;
  / < The off screen

      static const Color DefaultHostColor = Color.gray,
                         / < The default background color of the
                                 DefaultBackgroundColor = Color.black;
  / < The default background color.
}
