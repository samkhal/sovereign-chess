
#include "sovereign_chess.h"
#include <algorithm>
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

bool is_legal(const Board &board, const Move &move) {
  auto legal_moves = Game::get_legal_moves(board);
  return std::find(legal_moves.begin(), legal_moves.end(), move) !=
         legal_moves.end();
}

void test_control() {
  {
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/"
        "yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/"
        "srsnppppwpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq");
    assert(b.controlling_player(Color::White) == Player::Player1);
    assert(b.controlling_player(Color::Black) == Player::Player2);
    assert(b.controlling_player(Color::Yellow) == std::nullopt);
  }

  {
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/"
        "yqyp12vpvq/ybyp12vpvb/onop8wp3npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/"
        "srsnppppwpwpwpwpwpwp1wpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq b");
    assert(b.controlling_player(Color::Yellow) == Player::Player1);
  }

  {
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12op1/nqnp12opoq/crcp12rprr/cncp6cq5rprn/gbgp12pppb/gqgp8ob3pppq/"
        "yqyp8wp3vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cp1/rbrp12cpcb/"
        "srsnppppwpwpwpwpwpwp1wpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq b");
    assert(b.controlling_player(Color::Violet) == Player::Player1);
  }

  {
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/"
        "yqyp12vpvq/ybyp12vpvb/onop8pp3npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/"
        "srsnpp1wpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq b ");
    assert(b.controlling_player(Color::Yellow) == std::nullopt);
    assert(b.controlling_player(Color::Pink) == std::nullopt);
  }

  // Cycle between pink and yellow
  // Cycles are not possible in the rules
  // {
  //   auto b = Board::from_fen(
  //       "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbp1ypsnsr/"
  //       "nbnp12opob/nqnp12opoq/crcp12rprr/cncp5yp6rprn/gbgp12pppb/gqgp12pppq/"
  //       "yqyp12vpvq/ybyp12vpvb/onop8pp3npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/"
  //       "srsnpp1wpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq
  //       w");
  //   assert(b.controlling_player(Color::Yellow) == std::nullopt);
  //   assert(b.controlling_player(Color::Pink) == std::nullopt);
  // }
}

void test_rule_2() { // todo
}
void test_rule_3() { // todo
}
void test_rule_4() { // todo
}
void test_rule_5() { // Piece cannot move onto square of its own color

  { // Black can't move to black square
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbb1brynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp7bn4pppq/"
        "yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/"
        "srsnppppwpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq "
        "b");
    auto legal_moves = Game::get_legal_moves(b);
    Move legal("j9", "hA");
    Move illegal("j9", "h8");
    assert(square_color(legal.dest) != Color::Black);
    assert(is_legal(b, legal));
    assert(square_color(illegal.dest) == Color::Black);
    assert(!is_legal(b, illegal));
  }
  { // Black can't capture on black square
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbb1brynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp7bn4pppq/"
        "yqyp5wp6vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/rbrp12cpcb/"
        "srsnppppwpwpwpwpwp1wpwpgpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq b");
    auto legal_moves = Game::get_legal_moves(b);
    Move legal("j9", "hA");
    Move illegal("j9", "h8");
    assert(std::find(legal_moves.begin(), legal_moves.end(), legal) !=
           legal_moves.end());
    assert(std::find(legal_moves.begin(), legal_moves.end(), illegal) ==
           legal_moves.end());
  }
}
void test_rule_6() { // todo
}
void test_rule_7() { // todo
}
void test_rule_9() { // todo
}
void test_rule_10() { // todo
}
void test_rule_11() { // players control pieces via colored squares, unless
                      // opponent's king has that color

  {
    auto b = Board::from_fen(
        "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
        "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp6wp5pppq/"
        "yqyp12vpvq/ybyp12vpvb/onop12npnn/orop9wp2npnr/rqrp12cpcq/rbrp12cpcb/"
        "srsnppppwpwpwpwpwpwp2gpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq w");
    // White controls red but not black
    Move red_knight("pB", "nC");
    Move black_knight("kG", "lE");
    assert(is_legal(b, red_knight));
    assert(!is_legal(b, black_knight));
  }
  /* TODO fix when we implement color-changing kings
    { // same position but black king is red
      auto b = Board::from_fen(
          "aqabvrvnbrbnbbbqrkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbpbpypypsnsr/"
          "nbnp12opob/nqnp12opoq/crcp12rprr/cncp12rprn/gbgp12pppb/gqgp6wp5pppq/"
          "yqyp12vpvq/ybyp12vpvb/onop12npnn/orop9wp2npnr/rqrp12cpcq/rbrp12cpcb/"
          "srsnppppwpwpwpwpwpwp2gpgpanar/sqsbprpnwrwnwbwqwkwbwnwrgngrabaq w");
      // White controls red but not black
      Move red_knight("pB", "nC");
      Move black_knight("kG", "lE");
      assert(!is_legal(b, red_knight));
      assert(is_legal(b, black_knight));
    }
    */
}

void test_rule_12() { // players may only capture pieces controlled by the other
                      // player
  auto b = Board::from_fen(
      "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbp3ypsnsr/"
      "nbnp5np1yp4opob/nq5wp3bp3opoq/crcp6wn2bp2rprr/cncp12rprn/gbgp12pppb/"
      "gqgp12pppq/yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp12cpcq/"
      "rbrp12cpcb/srsnppppwpwpwpwpwp1wpwpgpgpanar/"
      "sqsbprpnwrwnwbwqwkwb1wrgngrabaq w");
  // Black controls blue but not yellow
  Move take_blue("iC", "hE");
  Move take_yellow("iC", "jE");
  Move take_white("iC", "gD");
  Move take_black("iC", "kD");
  assert(is_legal(b, take_blue));
  assert(!is_legal(b, take_yellow));
  assert(is_legal(b, take_black));
  assert(!is_legal(b, take_white));
}

void test_rule_13() { // Only one square of each color may be occupied at a time
  auto b = Board::from_fen(
      "aqabvrvnbrbnbbbqbkbbbnbrynyrsbsq/aranvpvpbpbpbpbpbpbpbp1ypypsnsr/"
      "nbnp12opob/nqnp12opoq/crcp9bp2rprr/cncp12rprn/gbgp12pppb/gqgp12pppq/"
      "yqyp12vpvq/ybyp12vpvb/onop12npnn/orop12npnr/rqrp4wn7cpcq/rbrp12cpcb/"
      "srsnppppwpwpwpwpwpwpwpwpgpgpanar/sqsbprpnwrwnwbwqwkwb1wrgngrabaq w");
  // Black controls blue but not green
  Move land_blue("g4", "e5");
  Move land_green("g4", "f6");
  assert(is_legal(b, land_green));
  assert(!is_legal(b, land_blue));
}

void test_rule_14() {
  // todo
}

void test_rule_15() {
  // todo
}

} // namespace sovereign_chess

int main(int argc, char **argv) {
  using namespace sovereign_chess;
  print_board_colors();
  test_coords();
  test_control();

  test_rule_5();
  test_rule_11();
  test_rule_12();
  test_rule_13();

  std::cout << "Done." << std::endl;
  return 0;
}
