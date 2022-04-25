#pragma once

#include <array>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace common {
struct Coord {
  int rank;
  int file;

  bool operator==(const Coord &other) const {
    return rank == other.rank && file == other.file;
  }
  Coord operator+(const Coord &other) const {
    return Coord{rank + other.rank, file + other.file};
  }
};
inline std::ostream &operator<<(std::ostream &out, const Coord &c) {
  out << "Coord{" << c.rank << "," << c.file << "}";
  return out;
}
struct coord_hash {
  std::size_t operator()(const Coord &c) const {
    return std::hash<int>()(c.rank) ^ std::hash<int>()(c.file);
  }
};

const std::vector<Coord> kKnightSteps = {{1, 2}, {-1, 2}, {1, -2}, {-1, -2},
                                         {2, 1}, {2, -1}, {-2, 1}, {-2, -1}};

const std::vector<Coord> kOrthogonalSteps = {{1, 0}, {0, 1}, {0, -1}, {-1, 0}};

const std::vector<Coord> kDiagonalSteps = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

enum class PieceType : uint8_t {
  Invalid,
  Pawn,
  Knight,
  Bishop,
  Rook,
  Queen,
  King
};

const std::unordered_map<PieceType, char> piece_names = {
    {PieceType::Pawn, 'p'}, {PieceType::Knight, 'n'}, {PieceType::Bishop, 'b'},
    {PieceType::Rook, 'r'}, {PieceType::Queen, 'q'},  {PieceType::King, 'k'}};

inline PieceType name_to_piece_type(char name) {
  for (auto &[p, p_name] : piece_names) {
    if (p_name == name)
      return p;
  }
  return PieceType{};
}

} // namespace common

namespace chess {

using common::Coord;
using common::PieceType;

extern bool print_captures;
extern bool print_moves;
extern bool print_all_moves;

// ----------------------------- Core types ---------------------------

enum class Color : uint8_t { Empty, White, Black };

enum class Castle { Kingside, Queenside };

struct Piece {
  PieceType type;
  Color color;

  bool operator==(const Piece &other) const {
    return type == other.type && color == other.color;
  }
  bool operator!=(const Piece &other) const { return !(*this == other); }
};

inline std::string to_algebraic(const Coord &c) {
  char file = 'a' + c.file;
  char rank = '1' + c.rank;
  return {file, rank};
}

struct Move {
  Coord src;
  Coord dest;
  PieceType promotion_type = PieceType::Invalid;

  bool operator==(const Move &other) const {
    return src == other.src && dest == other.dest &&
           promotion_type == other.promotion_type;
  }
  std::string to_string() const {
    std::ostringstream ss;
    ss << to_algebraic(src) << to_algebraic(dest);
    return ss.str();
  }
};

inline std::ostream &operator<<(std::ostream &out, const Move &m) {
  out << "Move{" << m.src << "," << m.dest << "," << (int)m.promotion_type
      << "}";
  return out;
}

class Board {
public:
  Board();

  void make_move(const Move &move, bool count = false);
  void place_piece(const Piece &piece, const Coord &coord);

  friend std::ostream &operator<<(std::ostream &out, const Board &board);

  static Board from_fen(std::string_view fen);

  Color side_to_move() const { return side_to_move_; };
  Color &side_to_move() { return side_to_move_; };
  Color not_side_to_move() const {
    return side_to_move_ == Color::White ? Color::Black : Color::White;
  };

  const Piece &piece_at(const Coord &coord) const {
    return pieces_[coord.rank][coord.file];
  }
  Piece &piece_at(const Coord &coord) {
    return pieces_[coord.rank][coord.file];
  }

  const bool &castle_right(Color color, Castle side) const {
    return castle_rights_[(static_cast<int>(color) - 1) * 2 +
                          static_cast<int>(side)];
  }
  bool &castle_right(Color color, Castle side) {
    return castle_rights_[(static_cast<int>(color) - 1) * 2 +
                          static_cast<int>(side)];
  }

  std::optional<Coord> en_passant_target() const { return en_passant_target_; }

private:
  std::array<std::array<Piece, 8>, 8> pieces_;
  Color side_to_move_ = Color::White;
  std::array<bool, 4> castle_rights_ = {true, true, true, true};
  std::optional<Coord> en_passant_target_ = {};
};

class Counters {
public:
  // call counter methods before applying move
  void move(const Board &board, const Move &move);
  void capture(const Board &board, const Move &move);
  void promotion(const Board &board, const Move &move);
  void castle(const Board &board, const Move &move);
  void en_passant(const Board &board, const Move &move);

  int moves() const { return moves_; }
  int captures() const { return captures_; }
  int promotions() const { return promotions_; }
  int castles() const { return castles_; }
  int en_passants() const { return en_passants_; }

private:
  int moves_ = 0;
  int captures_ = 0;
  int promotions_ = 0;
  int castles_ = 0;
  int en_passants_ = 0;
};
extern Counters global_counters;
std::ostream &operator<<(std::ostream &out, const Counters &c);

// ---- Check utilities ----
bool move_kills_king(const Board &board, const Move &move);
bool is_in_check(const Board &board);
bool move_into_check(const Board &board, const Move &move);

// ---- Castle utilities ----
bool is_castle(const Board &board, const Move &move);
Move castle_intermediate_king_move(const Move &move);

// ---- En passant utilities ----
bool is_en_passant(const Board &board, const Move &move);

/** Castling requirements:
 * - castle rights, updated in Board::make_move. Rights are lost when:
 *   - any king moves
 *   - rook moves from origin
 *   - enemy piece moves onto rook origin square
 * - no pieces blocking, checked in get_possible_moves
 * - not starting from or passing through check, checked in get_legal_moves
 * - not ending in check (same as any other move)
 */

void prettyprint_move(const Board &board, const Move &move);

std::vector<Move> get_possible_moves(const Board &board);

struct Game {
  using Board = Board;
  static std::vector<Move> get_legal_moves(const Board &board);
};
} // namespace chess