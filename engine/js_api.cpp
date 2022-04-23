#include <iostream>
#include <string.h>
#include <string>
#include <string_view>

#include "sovereign_chess.h"

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

extern "C" const char *get_legal_moves(const char *fen) {

  const std::string str = get_legal_moves_impl<sovereign_chess::Game>(fen);
  char *c_str = strcpy(new char[str.length() + 1], str.c_str());
  return c_str;
}