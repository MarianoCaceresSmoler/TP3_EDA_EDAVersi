/**
 * @brief Implements the Reversi game AI
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#ifndef AI_H
#define AI_H

#include "model.h"
#include <unordered_map>

enum Bound
{
	EXACT,
	LOWER_BOUND,
	UPPER_BOUND
};

struct NodePunctuation
{
	int eval;
	Bound bound;
	int depth;
};

typedef std::unordered_map<uint64_t, NodePunctuation> NodesTable; // Keys are Zobrist hash

/**
 * @brief Returns the best move for a certain position.
 *
 * @return The best move.
 */
Square getBestMove(GameModel &model);

int evaluateBoard(const Board board, Player maxPlayer);

Square findBestMove(const GameModel& model, int depth);
int minimax(const GameModel& model, NodesTable& table, int depth, int alpha, int beta, bool maximizingPlayer, Player maxPlayer);

#endif
