#include "../../iGeek/Env.hxx"

ISN main(ISN arg_count, const char** args) {
  return iGeek::Env(args, arg_count).Run();
}
