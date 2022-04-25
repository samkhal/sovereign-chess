
#include "sovereign_chess.h"
#include <sstream>

#include <cassert>
namespace sovereign_chess {

void print_board_colors() {
  std::ostringstream out;
  for (int rank = 15; rank >= 0; rank--) {
    for (int file = 0; file < 15; file++) {
      auto color = colored_squares.find({rank, file});
      if (color == colored_squares.end())
        out << " ";
      else
        out << color_names.at(color->second);
    }
    out << "\n";
  }
  std::cout << out.str();
}

void test_coords() {
  assert(to_algebraic({0, 0}) == "a1");
  assert(to_algebraic({8, 7}) == "h9");
  assert(to_algebraic({9, 8}) == "iA");
  assert(to_algebraic({15, 15}) == "pG");
}

} // namespace sovereign_chess

int main(int argc, char **argv) {
  using namespace sovereign_chess;
  test_coords();
  print_board_colors();

  std::cout << "Done." << std::endl;
  return 0;
}
