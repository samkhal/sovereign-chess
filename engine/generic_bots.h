// Bots for generic game playing
#pragma once
#include <cstdlib>

#include "sovereign_chess.h"

namespace sovereign_chess {

class RandomBot {
public:
  std::optional<Move> select_move(Board &board) {
    auto legal_moves = Game::get_legal_moves(board);
    if (legal_moves.empty()) {
      return {};
    }
    return legal_moves[rand() % legal_moves.size()];
  }
};

class MinimaxBot {
public:
  std::optional<Move> select_move(Board &board) {
    const int kDepth = 1;
    auto legal_moves = Game::get_legal_moves(board);

    if (legal_moves.empty()) {
      return {};
    }

    double max_score = -std::numeric_limits<double>::infinity();
    Move best_move = legal_moves.front();
    for (const auto &move : legal_moves) {
      Board new_board = board;
      new_board.make_move(move);
      const double score = -negaMax(new_board, kDepth - 1);

      if (score > max_score) {
        max_score = score;
        best_move = move;
      }
    }
    return best_move;
  }

private:
  double negaMax(const Board &board, int depth) {
    if (depth == 0)
      return evaluate(board);

    auto legal_moves = Game::get_legal_moves(board);
    if (legal_moves.empty())
      return evaluate(board);

    double max_score = -std::numeric_limits<double>::infinity();

    for (const auto &move : legal_moves) {
      Board new_board = board;
      new_board.make_move(move);
      const double score = -negaMax(new_board, depth - 1);

      max_score = std::max(max_score, score);
    }
    return max_score;
  }

  // Relative to Player 1
  double evaluate(const Board &board) const {
    auto legal_moves = Game::get_legal_moves(board);

    double score = 0;          // score relative to active player
    if (legal_moves.empty()) { // game over
      if (is_in_check(board))  // checkmate
        return -std::numeric_limits<double>::infinity();
      else // stalemate
        return 0;
    }

    // Otherwise, count up pieces
    int self_piece_count = 0;
    int other_piece_count = 0;
    for (int row = 0; row < 16; row++) {
      for (int col = 0; col < 16; col++) {
        std::optional<Player> player =
            board.controlling_player(board.piece_at(Coord{row, col}).color);
        if (!player)
          continue;
        else if (*player == board.player_to_move())
          self_piece_count++;
        else
          other_piece_count++;
      }
    }

    return (self_piece_count - other_piece_count);
  }
};
} // namespace sovereign_chess