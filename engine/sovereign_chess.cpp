#include "sovereign_chess.h"

namespace sovereign_chess {

using common::name_to_piece_type;
using PT = common::PieceType;
// ----------------------------- Move engine ---------------------------

bool in_range(int coord) { return coord >= 0 && coord <= 15; }
bool in_range(const Coord &coord) {
  return in_range(coord.rank) && in_range(coord.file);
}

// Knight, king
void fill_possible_nonrepeating_moves(const Board &board,
                                      const std::vector<Coord> &relative_moves,
                                      const Coord &src,
                                      std::vector<Move> &moves) {
  for (const Coord &relative : relative_moves) {
    Coord target = src + relative;
    if (in_range(target) &&
        (board.piece_at(target).color == Color::Empty ||
         board.is_enemy_color(board.piece_at(target).color))) {
      moves.push_back({src, target, {}});
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
      if (!in_range(target))
        break;
      else if (board.piece_at(target).color == Color::Empty) {
        // Empty square, continue
        moves.push_back({src, target});
      } else if (board.is_enemy_color(board.piece_at(target).color)) {
        // Enemy piece, record move but stop
        moves.push_back({src, target});
        break;
      } else {
        // Not empty and not capturable, stop
        break;
      }
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

Board Board::from_fen(std::string_view &fen) {
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

bool Board::is_enemy_color(Color color) const {
  // TODO handle controlled colors
  if (color == Color::Empty)
    return false;
  if (color == owned_color(player_to_move()))
    return false;
  return true;
}

std::vector<Move> get_possible_moves(const Board &board) {

  std::vector<Move> moves;
  // Iterate over all pieces of current color //TODO(colors)
  for (int rank = 0; rank < 16; rank++) {
    for (int file = 0; file < 16; file++) {
      Coord coord{rank, file};
      const Piece &piece = board.piece_at(coord);
      if (piece.type != PieceType::Invalid &&
          piece.color == board.owned_color(board.player_to_move())) {
        // Pawns
        if (piece.type == PieceType::Pawn) {
          // TODO pawn moves
        }

        // Kings
        else if (piece.type == PieceType::King) {
          fill_possible_nonrepeating_moves(board, common::kDiagonalSteps, coord,
                                           moves);
          fill_possible_nonrepeating_moves(board, common::kOrthogonalSteps,
                                           coord, moves);

          // TODO castling
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