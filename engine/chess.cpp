#include "chess.h"
namespace chess {

bool print_captures = false;
bool print_moves = false;
bool print_all_moves = false;

// ----------------------------- Counters ---------------------------

std::ostream &operator<<(std::ostream &out, const Counters &c) {
  out << "Captures: " << c.captures() << " Promotions: " << c.promotions()
      << " Castles: " << c.castles() << " Simple moves: " << c.moves()
      << " En passant: " << c.en_passants();
  return out;
}

Counters global_counters;

// ----------------------------- Game board updates ---------------------------

Board::Board() {
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      pieces_[rank][file] = Piece{PieceType::Invalid, Color::Empty};
    }
  }
}

// assume move is legal
void Board::make_move(const Move &move, bool count) {
  // First, update castle rights
  if (piece_at(move.src).type ==
      PT::King) // castle rights lost when moving king
  {
    castle_right(side_to_move(), Castle::Kingside) = false;
    castle_right(side_to_move(), Castle::Queenside) = false;
  } else if (piece_at(move.src).type ==
             PT::Rook) // castle rights lost when rook is moved
  {
    if (side_to_move() == Color::White) {
      if (move.src == Coord{0, 0})
        castle_right(Color::White, Castle::Queenside) = false;
      if (move.src == Coord{0, 7})
        castle_right(Color::White, Castle::Kingside) = false;
    } else {
      if (move.src == Coord{7, 0})
        castle_right(Color::Black, Castle::Queenside) = false;
      if (move.src == Coord{7, 7})
        castle_right(Color::Black, Castle::Kingside) = false;
    }
  }
  // castle rights lost when enemy piece moves onto rook origin square
  if (side_to_move() == Color::White) {
    if (move.dest == Coord{7, 0})
      castle_right(Color::Black, Castle::Queenside) = false;
    if (move.dest == Coord{7, 7})
      castle_right(Color::Black, Castle::Kingside) = false;
  } else {
    if (move.dest == Coord{0, 0})
      castle_right(Color::White, Castle::Queenside) = false;
    if (move.dest == Coord{0, 7})
      castle_right(Color::White, Castle::Kingside) = false;
  }

  // Compute en-passant target square
  if (piece_at(move.src).type == PT::Pawn &&
      std::abs(move.dest.rank - move.src.rank) == 2) {
    int dir = (move.dest.rank - move.src.rank) / 2;
    en_passant_target_ = Coord{move.src.rank + dir, move.src.file};
  } else {
    en_passant_target_ = {};
  }

  // Pawn promotion
  if (move.promotion_type != PieceType::Invalid) {
    if (count) {
      global_counters.promotion(*this, move);
    }
    piece_at(move.dest) = Piece{move.promotion_type, piece_at(move.src).color};
  }
  // Castle
  else if (is_castle(*this, move)) {
    if (count)
      global_counters.castle(*this, move);
    piece_at(move.dest) = piece_at(move.src);

    // Move the rook
    if (move.dest.file > move.src.file) { // 0-0
      piece_at(Coord{move.src.rank, 7}) = Piece{};
      piece_at(Coord{move.src.rank, 5}) = Piece{PT::Rook, side_to_move()};
      castle_right(side_to_move(), Castle::Kingside) = false;
    } else { // 0-0-0
      piece_at(Coord{move.src.rank, 0}) = Piece{};
      piece_at(Coord{move.src.rank, 3}) = Piece{PT::Rook, side_to_move()};
      castle_right(side_to_move(), Castle::Queenside) = false;
    }
  } else if (is_en_passant(*this, move)) {
    if (count)
      global_counters.en_passant(*this, move);
    piece_at(move.dest) = piece_at(move.src);

    int dir = move.dest.rank - move.src.rank;
    piece_at(move.dest + Coord{-dir, 0}) = Piece{};
  } else if (piece_at(move.dest).color != Color::Empty) { // Normal capture
    if (count)
      global_counters.capture(*this, move);
    piece_at(move.dest) = piece_at(move.src);
  } else { // Normal move
    if (count)
      global_counters.move(*this, move);
    piece_at(move.dest) = piece_at(move.src);
  }
  piece_at(move.src) = Piece{};

  // swap player
  side_to_move_ = not_side_to_move();
}

void Board::place_piece(const Piece &piece, const Coord &coord) {
  // std::cout << "Placing " << (int)piece.type<< " "<<(int)piece.color<<" at
  // "<<(int)coord.rank<<" "<<(int)coord.file<<std::endl;
  pieces_[coord.rank][coord.file] = piece;
}

std::ostream &operator<<(std::ostream &out, const Board &board) {
  out << "Castle rights: " << board.castle_rights_[0] << board.castle_rights_[1]
      << board.castle_rights_[2] << board.castle_rights_[3] << "\n";
  for (int rank = 7; rank >= 0; rank--) {
    for (int file = 0; file < 8; file++) {
      const Piece &piece = board.pieces_[rank][file];
      if (piece.color == Color::Empty) {
        out << "_";
      } else if (piece.color == Color::White) {
        out << (char)(piece_names.at(piece.type) - 'a' + 'A');
      } else {
        out << piece_names.at(piece.type);
      }
    }
    out << "\n";
  }
  return out;
}

bool is_capture(const Board &board, const Move &move) {
  // TODO en passant
  return board.piece_at(move.dest).color != Color::Empty;
}

void prettyprint_move(const Board &board, const Move &move) {
  std::ostringstream ss;
  if (board.side_to_move() == Color::Black)
    ss << "... ";
  ss << to_algebraic(move.src);
  if (is_capture(board, move))
    ss << "x";
  else
    ss << "-";
  ss << to_algebraic(move.dest);

  std::cout << ss.str() << "\n";
}

// Assume format is correct, read until first space
Board Board::from_fen(const std::string &fen) {
  Board board;
  int rank = 7;
  int file = 0;
  int segment = 0;
  for (const char &c : fen) {
    if (segment == 0) {
      if (c == '/') { // new rank
        rank -= 1;
        file = 0;
      } else if (c == ' ') { // end of piece placement
        segment = 1;
      } else if ('1' <= c && c <= '8') { // skip empty spaces
        file += static_cast<int>(c - '0');
      } else { // we have a piece
        Color color = ('A' <= c && c <= 'Z') ? Color::White : Color::Black;
        char lowercase = c;
        if (color == Color::White)
          lowercase = c - 'A' + 'a';
        PieceType type = name_to_piece_type(lowercase);
        board.place_piece(Piece{type, color}, Coord{rank, file});

        file++;
      }
    } else if (segment == 1) {
      if (c == 'b')
        board.side_to_move() = Color::Black;
    }
  }
  // Remove castling rights if kings or rooks aren't in origin spaces
  if (board.piece_at({0, 4}) != Piece{PT::King, Color::White}) {
    board.castle_right(Color::White, Castle::Kingside) = false;
    board.castle_right(Color::White, Castle::Queenside) = false;
  }
  if (board.piece_at({7, 4}) != Piece{PT::King, Color::Black}) {
    board.castle_right(Color::Black, Castle::Kingside) = false;
    board.castle_right(Color::Black, Castle::Queenside) = false;
  }
  if (board.piece_at({0, 0}) != Piece{PT::Rook, Color::White}) {
    board.castle_right(Color::White, Castle::Queenside) = false;
  }
  if (board.piece_at({0, 7}) != Piece{PT::Rook, Color::White}) {
    board.castle_right(Color::White, Castle::Kingside) = false;
  }
  if (board.piece_at({7, 0}) != Piece{PT::Rook, Color::Black}) {
    board.castle_right(Color::Black, Castle::Queenside) = false;
  }
  if (board.piece_at({7, 7}) != Piece{PT::Rook, Color::Black}) {
    board.castle_right(Color::Black, Castle::Kingside) = false;
  }
  return board;
}
// ----------------------------- Counter methods ---------------------------

void Counters::move(const Board &board, const Move &move) {
  moves_++;
  if (print_moves || print_all_moves)
    prettyprint_move(board, move);
}
void Counters::capture(const Board &board, const Move &move) {
  captures_++;
  if (print_captures || print_all_moves)
    prettyprint_move(board, move);
}
void Counters::promotion(const Board &board, const Move &move) {
  promotions_++;
  if (print_all_moves)
    prettyprint_move(board, move);
}
void Counters::castle(const Board &board, const Move &move) {
  castles_++;
  if (print_all_moves)
    prettyprint_move(board, move);
}
void Counters::en_passant(const Board &board, const Move &move) {
  en_passants_++;
  if (print_all_moves)
    prettyprint_move(board, move);
}

// ----------------------------- Move engine ---------------------------

bool in_range(int coord) { return coord >= 0 && coord <= 7; }
bool in_range(const Coord &coord) {
  return in_range(coord.rank) && in_range(coord.file);
}

const std::vector<Coord> kKnightSteps = {{1, 2}, {-1, 2}, {1, -2}, {-1, -2},
                                         {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

const std::vector<Coord> kOrthogonalSteps = {{1, 0}, {0, 1}, {0, -1}, {-1, 0}};

const std::vector<Coord> kDiagonalSteps = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

// Knight, king
void fill_possible_nonrepeating_moves(const Board &board,
                                      const std::vector<Coord> &relative_moves,
                                      const Coord &src,
                                      std::vector<Move> &moves) {
  for (const Coord &relative : relative_moves) {
    Coord target = src + relative;
    if (in_range(target) &&
        (board.piece_at(target).color != board.side_to_move())) {
      moves.push_back({src, target, {}});
    }
  }
}

void fill_possible_repeating_moves(const Board &board,
                                   const std::vector<Coord> &relative_moves,
                                   const Coord &src, std::vector<Move> &moves) {
  for (const Coord &relative : relative_moves) {
    Coord target = src;
    while (true) {
      target = target + relative;
      // if we're off the board, or we hit a friendly piece, stop
      if (!in_range(target) ||
          (board.piece_at(target).color == board.side_to_move())) {
        break;
      } else if (board.piece_at(target).color ==
                 board.not_side_to_move()) { // hit enemy piece; good, but stop
        moves.push_back({src, target, {}});
        break;
      } else {
        moves.push_back({src, target, {}});
      }
    }
  }
}

std::vector<Move> get_possible_moves(const Board &board) {

  std::vector<Move> moves;
  // Iterate over all pieces of current color
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      Coord coord{rank, file};
      const Piece &piece = board.piece_at(coord);
      if (piece.type != PieceType::Invalid &&
          piece.color == board.side_to_move()) {
        // Pawns
        if (piece.type == PieceType::Pawn) {
          int dir = piece.color == Color::White ? 1 : -1;
          if (in_range(rank + dir) &&
              board.piece_at(Coord{rank + dir, file}).color == Color::Empty) {
            // Promotion
            if (rank + dir == 7 || rank + dir == 0) {
              moves.push_back(Move{coord, Coord{rank + dir, file}, PT::Bishop});
              moves.push_back(Move{coord, Coord{rank + dir, file}, PT::Knight});
              moves.push_back(Move{coord, Coord{rank + dir, file}, PT::Rook});
              moves.push_back(Move{coord, Coord{rank + dir, file}, PT::Queen});
            } else { // simple push
              moves.push_back(Move{coord, Coord{rank + dir, file}});
            }

            // double move (only possible if single move is)
            if ((rank == 1 || rank == 6) && in_range(rank + dir * 2) &&
                board.piece_at(Coord{rank + dir * 2, file}).color ==
                    Color::Empty) {
              moves.push_back(Move{coord, Coord{rank + dir * 2, file}});
            }
          }
          // Captures
          for (int offset : {-1, 1}) {
            Coord target{rank + dir, file + offset};
            if (in_range(target) &&
                (board.piece_at(target).color == board.not_side_to_move() ||
                 target == board.en_passant_target())) {
              // Promotion
              if (rank + dir == 7 || rank + dir == 0) {
                moves.push_back(Move{coord, target, PT::Bishop});
                moves.push_back(Move{coord, target, PT::Knight});
                moves.push_back(Move{coord, target, PT::Rook});
                moves.push_back(Move{coord, target, PT::Queen});
              } else { // simple push
                moves.push_back(Move{coord, target});
              }
            }
          }
        }

        // Kings
        else if (piece.type == PieceType::King) {
          fill_possible_nonrepeating_moves(board, kDiagonalSteps, coord, moves);
          fill_possible_nonrepeating_moves(board, kOrthogonalSteps, coord,
                                           moves);

          // 0-0
          if (board.castle_right(piece.color, Castle::Kingside) &&
              board.piece_at({coord.rank, coord.file + 1}).color ==
                  Color::Empty &&
              board.piece_at({coord.rank, coord.file + 2}).color ==
                  Color::Empty)
            moves.push_back(Move{coord, Coord{coord.rank, coord.file + 2}});
          // 0-0-0
          if (board.castle_right(piece.color, Castle::Queenside) &&
              board.piece_at({coord.rank, coord.file - 1}).color ==
                  Color::Empty &&
              board.piece_at({coord.rank, coord.file - 2}).color ==
                  Color::Empty &&
              board.piece_at({coord.rank, coord.file - 3}).color ==
                  Color::Empty)
            moves.push_back(Move{coord, Coord{coord.rank, coord.file - 2}});
        }

        // Knights
        else if (piece.type == PieceType::Knight) {
          fill_possible_nonrepeating_moves(board, kKnightSteps, coord, moves);
        }

        // Bishops
        else if (piece.type == PieceType::Bishop) {
          fill_possible_repeating_moves(board, kDiagonalSteps, coord, moves);
        }

        // Rook
        else if (piece.type == PieceType::Rook) {
          fill_possible_repeating_moves(board, kOrthogonalSteps, coord, moves);
        }

        // Queen
        else if (piece.type == PieceType::Queen) {
          fill_possible_repeating_moves(board, kOrthogonalSteps, coord, moves);
          fill_possible_repeating_moves(board, kDiagonalSteps, coord, moves);
        }
      }
    }
  }
  return moves;
}

bool move_kills_king(const Board &board, const Move &move) {
  return board.piece_at(move.dest).type == PT::King;
}

bool is_in_check(const Board &board) {
  Board next_board = board;
  next_board.side_to_move() = board.not_side_to_move();
  auto responses = get_possible_moves(next_board);
  for (const Move &response : responses) {
    if (move_kills_king(next_board, response))
      return true;
  }
  return false;
}

bool move_into_check(const Board &board, const Move &move) {
  Board next_board = board;
  next_board.make_move(move);
  auto responses = get_possible_moves(next_board);
  for (const Move &response : responses) {
    if (move_kills_king(next_board, response)) {
      return true;
    }
  }
  return false;
}

bool is_castle(const Board &board, const Move &move) {
  return board.piece_at(move.src).type == PT::King &&
         std::abs(move.src.file - move.dest.file) == 2;
}

bool is_en_passant(const Board &board, const Move &move) {
  return board.piece_at(move.src).type == PT::Pawn &&
         move.src.file != move.dest.file &&
         board.piece_at(move.dest).color == Color::Empty;
}

// Assuming this is a castle move, get the move representing a single square
// king move in the same direction
Move castle_intermediate_king_move(const Move &move) {
  if (move.dest.file > move.src.file)
    return Move{move.src, {move.src.rank, move.src.file + 1}, {}};
  else
    return Move{move.src, {move.src.rank, move.src.file - 1}, {}};
}

std::vector<Move> get_legal_moves(const Board &board) {
  std::vector<Move> moves = get_possible_moves(board);
  std::erase_if(moves, [&](const Move &m) {
    if (is_castle(board, m)) {
      if (is_in_check(board))
        return true;
      if (move_into_check(board, castle_intermediate_king_move(m)))
        return true;
    }
    return move_into_check(board, m);
  });
  return moves;
}

} // namespace chess