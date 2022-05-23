#include <iostream>
#include <string.h>
#include <string>
#include <string_view>

#include <emscripten/emscripten.h>

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

extern "C" {
// For a given fen, produce a space-separated list of legal moves
const char *EMSCRIPTEN_KEEPALIVE get_legal_moves(const char *fen) {
  return to_new_cstr(sovereign_chess::get_legal_moves_impl(fen));
}

// For a given fen, return a move and new fen, comma-separated
const char *EMSCRIPTEN_KEEPALIVE select_move(const char *fen) {
  return to_new_cstr(sovereign_chess::select_move_impl(fen));
}

// For a given fen, return a move and new fen, comma-separated
const char *EMSCRIPTEN_KEEPALIVE make_move(const char *fen, const char *move) {
  return to_new_cstr(sovereign_chess::make_move_impl(fen, move));
}

const char *EMSCRIPTEN_KEEPALIVE get_owned_color(const char *fen,
                                                 bool activePlayer) {
  using namespace sovereign_chess;
  Board board = Game::Board::from_fen(fen);

  Player player = activePlayer ? board.player_to_move()
                               : other_player(board.player_to_move());

  Color owned_color = board.owned_color(player);
  std::string color_name{color_names.at(owned_color)};
  return to_new_cstr(color_name);
}

// Get colors controlled (but not owned) by a player
const char *EMSCRIPTEN_KEEPALIVE get_controlled_colors(const char *fen,
                                                       bool activePlayer) {
  using namespace sovereign_chess;
  Board board = Board::from_fen(fen);

  Player player = activePlayer ? board.player_to_move()
                               : other_player(board.player_to_move());

  std::ostringstream ss;
  for (const auto &[color, name] : color_names) {
    if (board.controlling_player(color) == player &&
        color != board.owned_color(player)) {
      if (!ss.str().empty())
        ss << " ";
      ss << name;
    }
  }

  return to_new_cstr(ss.str());
}
}