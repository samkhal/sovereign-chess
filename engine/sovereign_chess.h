#include "chess.h"

namespace sovereign_chess {
using common::Coord;
using common::PieceType;

// Player 1 always starts with White, but may end up playing Black on their next
// turn.
enum class Player : uint8_t { Player1, Player2 };
enum class Color : uint8_t { Empty, White, Black }; // TODO add colors

const std::unordered_map<Color, char> color_names = {
    {Color::Empty, ' '}, {Color::White, 'w'}, {Color::Black, 'b'}};

inline Color name_to_color(char name) {
  for (auto &[c, c_name] : color_names) {
    if (c_name == name)
      return c;
  }
  return Color::Empty;
}

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

  static Board from_fen(std::string_view &fen);

  Player player_to_move() const { return player_to_move_; };
  Player &player_to_move() { return player_to_move_; };

  const Piece &piece_at(const Coord &coord) const {
    return pieces_[coord.rank][coord.file];
  }
  Piece &piece_at(const Coord &coord) {
    return pieces_[coord.rank][coord.file];
  }
  bool is_enemy_color(Color color) const;
  Color owned_color(Player player) const { return owned_color_.at(player); }

private:
  std::array<std::array<Piece, 16>, 16> pieces_;
  Player player_to_move_ = Player::Player1;
  std::unordered_map<Player, Color> owned_color_ = {
      {Player::Player1, Color::White}, {Player::Player2, Color::Black}};
};

std::vector<Move> get_possible_moves(const Board &board);

struct Game {
  using Board = sovereign_chess::Board;
  static std::vector<Move> get_legal_moves(const Board &board);
};

} // namespace sovereign_chess