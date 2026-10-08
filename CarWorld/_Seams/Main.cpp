// Copyright AStarship.

#include "../../KabukiToolkit/server.hxx"
int main(int arg_count, const char** args) {
  return kabuki::toolkit::Server(args, arg_count).Run();
}
