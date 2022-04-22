#include "chess.h"

#include <cassert>

namespace chess {
using PT = common::PieceType;

// Count how many nodes (states) exist at given depth from a board
int compute_nodes(const Board &board, int depth, bool divide = false) {
  if (depth == 0)
    return 1;

  int node_count = 0;
  std::vector<Move> legal_moves = Game::get_legal_moves(board);
  Board new_board;
  for (const Move &move : legal_moves) {
    new_board = board;
    new_board.make_move(move, /*count=*/true);
    int subnodes = compute_nodes(new_board, depth - 1);
    if (divide) {
      std::cout << subnodes << " ";
      prettyprint_move(board, move);
    }
    node_count += subnodes;
  }
  return node_count;
}

void draw_fen(const std::string &fen) {
  std::cout << "Fen: " << fen << std::endl;
  std::cout << Board::from_fen(fen) << std::endl;
}

/** PERFT **/

struct PerftTestCase {
  std::string fen;
  std::vector<int> expected_nodes_at_depth;
};

// https://www.chessprogramming.org/Perft_Results
// FEN must have "KQkq -": white to move, all castles available, no en passant
// square
const std::vector<PerftTestCase> perft_test_cases = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
     {1, 20, 400, 8902, 197281, 4865609}},
    {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
     {1, 48, 2039, 97862, 4085603}},
    {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
     {1, 14, 191, 2812}}, // castle rights ok
    // Position 4: two mirror images
    {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
     {1, 6, 264, 9467, 422333}}, // castle rights ok
    {"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 ",
     {1, 6, 264, 9467, 422333}}, // castle rights ok
    // Position 5
    {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ",
     {1, 44, 1486, 62379, 2103487}} //
};

bool run_perft_test(const PerftTestCase &test_case) {
  std::cout << "Testing position: " << test_case.fen << std::endl;
  Board board = Board::from_fen(test_case.fen);
  bool success = true;
  for (int depth = 0; depth < test_case.expected_nodes_at_depth.size();
       depth++) {
    global_counters = Counters{};
    int computed_nodes = compute_nodes(board, depth);
    if (computed_nodes == test_case.expected_nodes_at_depth[depth]) {
      std::cout << "Depth " << depth << " success! ";
      std::cout << global_counters << std::endl;
    } else {
      std::cout << "Depth " << depth << " failed: got " << computed_nodes
                << ", expected " << test_case.expected_nodes_at_depth[depth]
                << ". ";
      std::cout << global_counters << std::endl;
      success = false;
    }
  }

  return success;
}

bool run_perft_tests() {
  bool success = true;
  for (auto &test_case : perft_test_cases) {
    success = success && run_perft_test(test_case);
  }
  return success;
}

void test_possible_moves() {
  { // empty
    Board b;
    assert(get_possible_moves(b).size() == 0);
  }
  // PAWNS
  { // push white
    Board b;
    b.place_piece({PT::Pawn, Color::White}, Coord{4, 4});
    auto moves = get_possible_moves(b);
    assert(moves.size() == 1);
    assert(moves[0] == (Move{Coord{4, 4}, Coord{5, 4}, {}}));
  }
  { // push black
    Board b;
    b.place_piece({PT::Pawn, Color::Black}, Coord{4, 4});
    b.side_to_move() = Color::Black;
    auto moves = get_possible_moves(b);
    assert(moves.size() == 1);
    assert(moves[0] == (Move{Coord{4, 4}, Coord{3, 4}, {}}));
  }
  { // push white double
    Board b;
    b.place_piece({PT::Pawn, Color::White}, Coord{1, 4});
    auto moves = get_possible_moves(b);
    assert(moves.size() == 2);
    assert(moves[0] == (Move{Coord{1, 4}, Coord{2, 4}, {}}));
    assert(moves[1] == (Move{Coord{1, 4}, Coord{3, 4}, {}}));
  }

  // Knight
  { // basic
    Board b;
    b.place_piece({PT::Knight, Color::White}, Coord{3, 1});
    auto moves = get_possible_moves(b);
    assert(moves.size() == 6);
  }

  // King can't enter check
  {
    Board b = Board::from_fen("8/8/2r5/8/3K4/8/8/8 w - - 0 1");
    // rook blocks 3 king moves, leaving 5
    auto moves = Game::get_legal_moves(b);
    assert(moves.size() == 5);
  }

  // Stalemate
  {
    Board b = Board::from_fen("8/8/3p4/8/3P4/8/8/8 w - - 0 1");
    int nodes = compute_nodes(
        b, 10, true); // Only one move available and it leads to stalemate
    std::cerr << nodes << std::endl;
    assert(nodes == 0);
  }

  // Castle
  {
    Board b = Board::from_fen("8/8/8/8/8/8/8/R3K2R w - - 0 1");
    global_counters = Counters{};
    print_all_moves = true;
    int nodes = compute_nodes(b, 1);
    print_all_moves = false;
    std::cout << nodes << " " << global_counters;
    assert(nodes == 26);
  }
  // No castle through check
  {
    Board b = Board::from_fen("3r4/8/8/8/8/8/8/R3K2R w - - 0 1");
    global_counters = Counters{};
    print_all_moves = true;
    int nodes = compute_nodes(b, 1);
    print_all_moves = false;
    std::cout << nodes << " " << global_counters << "\n";
    assert(nodes == 23);
  }
  std::cout << "Tests pass." << std::endl;
}
} // namespace chess

int main(int argc, char **argv) {
  using namespace chess;
  test_possible_moves();
  assert(run_perft_tests());

  std::cout << "Done." << std::endl;
  return 0;
}
