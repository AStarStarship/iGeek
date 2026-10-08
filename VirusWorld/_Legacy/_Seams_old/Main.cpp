#include "../KabukiToolkit/client.hxx"

int main(const char** args, int arg_count) {
  return kabuki::toolkit::Client(args, arg_count).Run();
}
