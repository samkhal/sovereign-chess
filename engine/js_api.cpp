#include <string>
#include <string.h>
#include <iostream>

#include "chess.h"

extern "C" const char *getLegalMoves(const char *fen)
{
    using Game = chess::Game;

    const Game::Board board = Game::Board::from_fen(fen);

    auto legal_moves = Game::get_legal_moves(board);
    std::ostringstream ss;
    for (int i = 0; i < legal_moves.size(); i++)
    {
        ss << legal_moves[i].to_string();
        if (i < legal_moves.size() - 1)
            ss << " ";
    }

    const std::string str = ss.str();
    char *c_str = strcpy(new char[str.length() + 1], str.c_str());
    return c_str;
}