// Bots for generic game playing
#pragma once
#include <cstdlib>

template <class Game> class RandomBot {
public:
  auto select_move(const typename Game::Board &board) {
    auto legal_moves = Game::get_legal_moves(board);
    return legal_moves[rand() % legal_moves.size()];
  }
};