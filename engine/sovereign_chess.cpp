#include "sovereign_chess.h"

namespace sovereign_chess {

using common::name_to_piece_type;
using PT = common::PieceType;
// ----------------------------- Move engine ---------------------------

bool in_range(int coord) { return coord >= 0 && coord <= 15; }
bool in_range(const Coord &coord) {
  return in_range(coord.rank) && in_range(coord.file);
}

// Precondition: square must be colored
// TODO optimize, constexpr
Coord other_square_of_same_color(const Coord &coord) {
  // Compute map of coords by color
  std::unordered_map<Color, std::vector<Coord>> coords_by_color;
  for (const auto &[coord, color] : colored_squares) {
    coords_by_color[color].push_back(coord);
  }

  // Compute map of square->square for all colors
  std::unordered_map<Coord, Coord, common::coord_hash> other_square;
  for (const auto &[color, coords] : coords_by_color) {
    other_square[coords[0]] = coords[1];
    other_square[coords[1]] = coords[0];
  }

  return other_square.at(coord);
}

// Return false if move violates coloring rules of target square.
// Precondition: piece exists at source square
bool check_target_square_color(const Board &board, const Coord &src,
                               const Coord &dest) {
  Color piece_color = board.piece_at(src).color;

  std::optional<Color> dest_color = square_color(dest);

  // No rules if target square is uncolored
  if (!dest_color)
    return true;

  // may not land on square of same color
  if (*dest_color == piece_color)
    return false;

  // If this is a capture, no further checking needed
  if (board.piece_at(dest).color != Color::Empty)
    return true;

  // Cannot land on colored square if other square of same color has piece
  if (board.piece_at(other_square_of_same_color(dest)).color != Color::Empty)
    return false;

  return true;
}

// Knight, king
void fill_possible_nonrepeating_moves(const Board &board,
                                      const std::vector<Coord> &relative_moves,
                                      const Coord &src,
                                      std::vector<Move> &moves) {
  for (const Coord &relative : relative_moves) {
    Coord target = src + relative;
    if (in_range(target) && check_target_square_color(board, src, target) &&
        (board.piece_at(target).color == Color::Empty ||
         is_enemy_color(board, board.piece_at(target).color))) {
      moves.push_back({src, target});
    }
  }
}

// Bishop, queen, rook
void fill_possible_repeating_moves(const Board &board,
                                   const std::vector<Coord> &relative_moves,
                                   const Coord &src, std::vector<Move> &moves) {
  for (const Coord &relative : relative_moves) {
    Coord target = src;
    while (true) {
      target = target + relative;
      // if we're off board, stop
      if (!in_range(target)) {
        break;
      } else if (board.piece_at(target).color == Color::Empty) {
        if (check_target_square_color(board, src, target)) {
          // Empty square that we can move to, continue
          moves.push_back({src, target});
        }
        // if it's empty but we fail color checks, keep going
      } else if (is_enemy_color(board, board.piece_at(target).color) &&
                 check_target_square_color(board, src, target)) {
        // Enemy piece, and we're not landing on our own color, so record move
        // but stop
        moves.push_back({src, target});
        break;
      } else {
        // Not empty and not capturable, stop
        break;
      }
    }
  }
}

void fill_possible_pawn_moves(const Board &board, const Coord &src,
                              std::vector<Move> &moves) {
  // Non-capture
  for (const auto &step : common::kOrthogonalSteps) {
    Coord dest = src + step;
    // Check that we're getting closer to the center
    if (std::abs(dest.rank - 7.5) < std::abs(src.rank - 7.5) ||
        std::abs(dest.file - 7.5) < std::abs(src.file - 7.5)) {
      if (board.piece_at(dest).color != Color::Empty)
        continue; // can't capture

      if (check_target_square_color(board, src, dest)) {
        moves.push_back(Move{src, dest});
      }

      // If we weren't blocked, try two-step advance
      Coord two_step = step + step;
      if (!in_range(src - two_step) && // Check if we're on outer two rings
          board.piece_at(src + two_step).color == Color::Empty &&
          check_target_square_color(board, src, src + two_step)) {
        moves.push_back(Move{src, src + two_step});
      }
    }
  }

  // Capture
  for (const auto &step : common::kDiagonalSteps) {
    Coord dest = src + step;
    const Piece &target_p = board.piece_at(dest);
    // Check that we're getting closer to a centerline
    if ((std::abs(dest.rank - 7.5) < std::abs(src.rank - 7.5) ||
         std::abs(dest.file - 7.5) < std::abs(src.file - 7.5)) &&
        board.controlling_player(target_p.color) ==
            other_player(board.player_to_move()) &&
        check_target_square_color(board, src, dest)) {
      moves.push_back(Move{src, dest});
    }
  }
}

Board::Board() {
  for (int rank = 15; rank >= 0; rank--) {
    for (int file = 0; file < 16; file++) {
      pieces_[rank][file] = Piece{PieceType::Invalid, Color::Empty};
    }
  }
}

void Board::place_piece(const Piece &piece, const Coord &coord) {
  pieces_[coord.rank][coord.file] = piece;
}

Board Board::from_fen(std::string_view fen) {
  Board board;
  int rank = 15;
  int file = 0;
  int segment = 0;
  int skip_accumulator = 0;
  std::optional<char> last_color = {};
  for (const char &c : fen) {
    if (segment == 0) {
      if (rank < 0)
        break;
      if (c == '/') { // new rank
        rank -= 1;
        file = 0;
        skip_accumulator = 0;
      } else if (c == ' ') { // end of piece placement
        segment = 1;
      } else if ('0' <= c && c <= '9') { // skip empty spaces
        skip_accumulator *= 10;
        skip_accumulator += static_cast<int>(c - '0');
      } else { // we have a piece
        file += skip_accumulator;
        skip_accumulator = 0;

        if (last_color) {

          Color color = name_to_color(*last_color);
          PieceType type = name_to_piece_type(c);
          board.place_piece(Piece{type, color}, Coord{rank, file});

          file++;
          last_color = {};
        } else {
          last_color = c;
        }
      }
    } else if (segment == 1) {
      if (c == 'b')
        // TODO handle colors
        board.player_to_move() = Player::Player2;
    }

    // TODO castle rights
  }
  return board;
}

std::optional<Player> Board::controlling_player(Color color) const {
  if (owned_color(Player::Player1) == color)
    return Player::Player1;
  if (owned_color(Player::Player2) == color)
    return Player::Player2;
  for (const auto &[coord, sq_color] : colored_squares) {
    if (sq_color == color) {
      const Piece &piece = piece_at(coord);
      if (piece.color != Color::Empty) {
        // Only one piece may occupy either colored square
        // Will overflow stack if there's a cycle, but cycles are not allowed
        return controlling_player(piece.color);
      }
    }
  }

  // Neutral color
  return {};
}

std::vector<Move> get_possible_moves(const Board &board) {

  std::vector<Move> moves;
  // Iterate over all pieces of current color //TODO(colors)
  for (int rank = 0; rank < 16; rank++) {
    for (int file = 0; file < 16; file++) {
      Coord coord{rank, file};
      const Piece &piece = board.piece_at(coord);
      if (piece.color != Color::Empty &&
          board.controlling_player(piece.color) == board.player_to_move()) {
        // Pawns
        if (piece.type == PieceType::Pawn) {
          fill_possible_pawn_moves(board, coord, moves);
        }

        // Kings
        else if (piece.type == PieceType::King) {
          fill_possible_nonrepeating_moves(board, common::kDiagonalSteps, coord,
                                           moves);
          fill_possible_nonrepeating_moves(board, common::kOrthogonalSteps,
                                           coord, moves);

          // TODO castling, regime change
        }

        // Knights
        else if (piece.type == PieceType::Knight) {
          fill_possible_nonrepeating_moves(board, common::kKnightSteps, coord,
                                           moves);
        }

        // Bishops
        else if (piece.type == PieceType::Bishop) {
          fill_possible_repeating_moves(board, common::kDiagonalSteps, coord,
                                        moves);
        }

        // Rook
        else if (piece.type == PieceType::Rook) {
          fill_possible_repeating_moves(board, common::kOrthogonalSteps, coord,
                                        moves);
        }

        // Queen
        else if (piece.type == PieceType::Queen) {
          fill_possible_repeating_moves(board, common::kOrthogonalSteps, coord,
                                        moves);
          fill_possible_repeating_moves(board, common::kDiagonalSteps, coord,
                                        moves);
        }
      }
    }
  }
  return moves;
}

std::vector<Move> Game::get_legal_moves(const Board &board) {
  // Todo move legality
  return get_possible_moves(board);
}

} // namespace sovereign_chess