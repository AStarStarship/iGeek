// Copyright AStarship <https://astarship.net>.

namespace iGeek {

/* Class that represents a DNA Virus.
A DNA Virus is a type of virus that is made of DNA and replicates itself inside
of the cell cytoplasm.
@see    https:/en.wikipedia.org/wiki/DNA_virus
*/
class DNAVirus : public Virus {
 public:
  DNAVirus(FPD init_x, FPD init_y);
};
}  //< namespace iGeek
