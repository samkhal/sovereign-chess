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

template <class Game> std::string get_legal_moves_impl(std::string_view fen) {
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

template <class Game> std::string select_and_play_move(std::string_view fen) {
  typename Game::Board board = Game::Board::from_fen(fen);

  RandomBot<Game> bot;
  auto move = bot.select_move(board);

  board.make_move(move);

  return move.to_string() + "," + board.to_fen();
}
} // namespace

// For a given fen, produce a space-separated list of legal moves
extern "C" const char *get_legal_moves(const char *fen) {
  return to_new_cstr(get_legal_moves_impl<sovereign_chess::Game>(fen));
}

// For a given fen, return a move and new fen, comma-separated
extern "C" const char *select_and_play_move(const char *fen) {
  return to_new_cstr(select_and_play_move<sovereign_chess::Game>(fen));
}