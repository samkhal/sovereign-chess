#pragma once
#include "chess.h"

namespace sovereign_chess {
using common::Coord;
using common::PieceType;

// Player 1 always starts with White, but may end up playing Black on their next
// turn.
enum class Player : bool { Player1, Player2 };
inline Player other_player(Player player) {
  return player == Player::Player1 ? Player::Player2 : Player::Player1;
};

enum class Color : uint8_t {
  Empty,
  White,
  Black,
  Ash,
  Slate,
  Pink,
  Red,
  Orange,
  Yellow,
  Green,
  Cyan,
  Navy,
  Violet
};

// clang-format off
const std::unordered_map<Coord, Color, common::coord_hash> colored_squares = {
  {{4, 4}, Color::Navy},
  {{11, 11}, Color::Navy},
  {{11, 4}, Color::Red},
  {{4, 11}, Color::Red},
  {{5, 5}, Color::Green},
  {{10, 10}, Color::Green},
  {{7, 5}, Color::Cyan},
  {{8, 10}, Color::Cyan},
  {{8, 5}, Color::Orange},
  {{7, 10}, Color::Orange},
  {{10, 5}, Color::Yellow},
  {{5, 10}, Color::Yellow},
  {{6, 6}, Color::Ash},
  {{9, 9}, Color::Ash},
  {{9, 6}, Color::Slate},
  {{6, 9}, Color::Slate},
  {{5, 7}, Color::Violet},
  {{10, 8}, Color::Violet},
  {{7, 7}, Color::Black},
  {{8, 8}, Color::Black},
  {{8, 7}, Color::White},
  {{7, 8}, Color::White},
  {{10, 7}, Color::Pink},
  {{5, 8}, Color::Pink}
};
// clang-format on
inline std::optional<Color> square_color(Coord coord) {
  auto it = colored_squares.find(coord);
  if (it != colored_squares.end())
    return it->second;
  return {};
}

const std::unordered_map<Color, char> color_names = {
    {Color::Empty, ' '}, {Color::White, 'w'},  {Color::Black, 'b'},
    {Color::Ash, 'a'},   {Color::Slate, 's'},  {Color::Pink, 'p'},
    {Color::Red, 'r'},   {Color::Orange, 'o'}, {Color::Yellow, 'y'},
    {Color::Green, 'g'}, {Color::Cyan, 'c'},   {Color::Navy, 'n'},
    {Color::Violet, 'v'}};

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

inline Coord from_algebraic(std::string_view s) {
  return Coord{s[1] < 58 ? s[1] - 49 : s[1] - 65 + 9, s[0] - 97};
}

struct Move {
  Coord src;
  Coord dest;
  PieceType promotion_type = PieceType::Invalid;

  Move(Coord n_src, Coord n_dest) : src(n_src), dest(n_dest) {}

  Move(std::string_view move_str)
      : Move(move_str.substr(0, 2), move_str.substr(2, 4)) {
    promotion_type = common::name_to_piece_type(move_str[4]);
  }

  Move(std::string_view n_src, std::string_view n_dest)
      : src(from_algebraic(n_src)), dest(from_algebraic(n_dest)) {}

  bool operator==(const Move &other) const {
    return src == other.src && dest == other.dest &&
           promotion_type == other.promotion_type;
  }
  std::string to_string() const {
    std::ostringstream ss;
    ss << to_algebraic(src) << to_algebraic(dest);
    if (promotion_type != PieceType::Invalid)
      ss << common::piece_names.at(promotion_type);
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

  void make_move(const Move &move);
  void place_piece(const Piece &piece, const Coord &coord);

  static Board from_fen(std::string_view fen);
  std::string to_fen();

  Player player_to_move() const { return player_to_move_; };
  Player &player_to_move() { return player_to_move_; };

  const Piece &piece_at(const Coord &coord) const {
    return pieces_[coord.rank][coord.file];
  }
  Piece &piece_at(const Coord &coord) {
    return pieces_[coord.rank][coord.file];
  }
  Color owned_color(Player player) const { return owned_color_.at(player); }

  // Which player controls a color, or empty if it's neutral
  std::optional<Player> controlling_player(Color color) const;

private:
  std::array<std::array<Piece, 16>, 16> pieces_;
  Player player_to_move_ = Player::Player1;
  std::unordered_map<Player, Color> owned_color_ = {
      {Player::Player1, Color::White}, {Player::Player2, Color::Black}};
};

inline bool is_enemy_color(const Board &board, Color color) {
  return board.controlling_player(color) ==
         other_player(board.player_to_move());
}

inline bool is_friendly_color(const Board &board, Color color) {
  return board.controlling_player(color) == board.player_to_move();
}

bool is_in_check(const Board &board);

std::vector<Move> get_possible_moves(const Board &board);

struct Game {
  using Board = sovereign_chess::Board;
  static std::vector<Move> get_legal_moves(const Board &board);
};

} // namespace sovereign_chess