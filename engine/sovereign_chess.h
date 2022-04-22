#include "chess.h"

namespace sovereign_chess {
using chess::PieceType;
using common::Coord;

enum class Color : uint8_t { Empty, White, Black }; // TODO

struct Piece {
  PieceType type;
  Color color;

  bool operator==(const Piece &other) const {
    return type == other.type && color == other.color;
  }
  bool operator!=(const Piece &other) const { return !(*this == other); }
};

inline std::string to_algebraic(const Coord &c) {
  // TODO add tests
  char file = 'a' + c.file;
  char rank = c.rank < 9 ? '1' + c.rank : 'A' + (c.rank - 9);
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

  static Board from_fen(const std::string &fen);

  Color side_to_move() const { return side_to_move_; };
  Color &side_to_move() { return side_to_move_; };

  const Piece &piece_at(const Coord &coord) const {
    return pieces_[coord.rank][coord.file];
  }
  Piece &piece_at(const Coord &coord) {
    return pieces_[coord.rank][coord.file];
  }

private:
  std::array<std::array<Piece, 16>, 16> pieces_;
  Color side_to_move_ = Color::White;
};

struct Game {
  static std::vector<Move> get_legal_moves(const Board &board);
};
} // namespace sovereign_chess