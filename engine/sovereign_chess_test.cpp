
#include "sovereign_chess.h"

#include <cassert>
namespace sovereign_chess {

void test_coords() {
  assert(to_algebraic({0, 0}) == "a1");
  assert(to_algebraic({9, 8}) == "i8");
  assert(to_algebraic({15, 15}) == "pG");
}

} // namespace sovereign_chess

int main(int argc, char **argv) {
  using namespace sovereign_chess;
  test_coords();

  std::cout << "Done." << std::endl;
  return 0;
}
