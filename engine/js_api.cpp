#include <iostream>
#include <string.h>
#include <string>
#include <string_view>

#include "generic_bots.h"
#include "sovereign_chess.h"

namespace {
char *to_new_cstr(const std::string str) {
  return strcpy(new char[str.length() + 1], str.c_str());
}
} // namespace

namespace sovereign_chess {

std::string get_legal_moves_impl(std::string_view fen) {
  std::cout << fen << std::endl;

  const typename Game::Board board = Game::Board::from_fen(fen);

  auto legal_moves = Game::get_legal_moves(board);
  std::ostringstream ss;
  for (int i = 0; i < legal_moves.size(); i++) {
    ss << legal_moves[i].to_string();
    if (i < legal_moves.size() - 1)
      ss << " ";
  }
  return ss.str();
}

std::string select_move_impl(std::string_view fen) {
  typename Game::Board board = Game::Board::from_fen(fen);

  RandomBot<Game> bot;
  auto move = bot.select_move(board);
  return move.to_string();
}

std::string make_move_impl(std::string_view fen, std::string_view move_str) {
  Board board = Game::Board::from_fen(fen);

  Move move{move_str};
  board.make_move(move);

  return board.to_fen();
}
} // namespace sovereign_chess

// For a given fen, produce a space-separated list of legal moves
extern "C" const char *get_legal_moves(const char *fen) {
  return to_new_cstr(sovereign_chess::get_legal_moves_impl(fen));
}

// For a given fen, return a move and new fen, comma-separated
extern "C" const char *select_move(const char *fen) {
  return to_new_cstr(sovereign_chess::select_move_impl(fen));
}

// For a given fen, return a move and new fen, comma-separated
extern "C" const char *make_move(const char *fen, const char *move) {
  return to_new_cstr(sovereign_chess::make_move_impl(fen, move));
}