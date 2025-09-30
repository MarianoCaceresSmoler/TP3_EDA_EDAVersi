/**
 * @brief Implements the Reversi game model
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <cstring>

#include "raylib.h"

#include "model.h"

struct Direction
{
    int x;
    int y;
};

#define UP_RIGHT Direction{1, 1}
#define UP Direction{0, 1}
#define UP_LEFT Direction{-1, 1}
#define LEFT Direction{-1, 0}
#define LEFT_DOWN Direction{-1, -1}
#define DOWN Direction{0, -1}
#define RIGHT_DOWN Direction{1, -1}
#define RIGTH Direction{1, 0}

/**
 * @brief Recursive function to try eating pieces in a given direction
 *
 * @param model The game model.
 * @param source Where the new piece was just placed
 * @param dir Direction to try eating
 * 
 * @return Wether any pieces were eaten (true) or not (false).
 */
static bool eatPieces(GameModel &model, Square source, Direction dir);
static bool checkCurrentSquare(GameModel& model, Square source, Direction dir);

void initModel(GameModel &model)
{
    model.gameOver = true;

    model.playerTime[0] = 0;
    model.playerTime[1] = 0;

    memset(model.board, PIECE_EMPTY, sizeof(model.board));
}

void startModel(GameModel &model)
{
    model.gameOver = false;

    model.currentPlayer = PLAYER_BLACK;

    model.playerTime[0] = 0;
    model.playerTime[1] = 0;
    model.turnTimer = GetTime();

    memset(model.board, PIECE_EMPTY, sizeof(model.board));
    model.board[BOARD_SIZE / 2 - 1][BOARD_SIZE / 2 - 1] = PIECE_WHITE;
    model.board[BOARD_SIZE / 2 - 1][BOARD_SIZE / 2] = PIECE_BLACK;
    model.board[BOARD_SIZE / 2][BOARD_SIZE / 2] = PIECE_WHITE;
    model.board[BOARD_SIZE / 2][BOARD_SIZE / 2 - 1] = PIECE_BLACK;
}

Player getCurrentPlayer(GameModel &model)
{
    return model.currentPlayer;
}

int getScore(GameModel &model, Player player)
{
    int score = 0;

    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            if (((model.board[y][x] == PIECE_WHITE) &&
                 (player == PLAYER_WHITE)) ||
                ((model.board[y][x] == PIECE_BLACK) &&
                 (player == PLAYER_BLACK)))
                score++;
        }

    return score;
}

double getTimer(GameModel &model, Player player)
{
    double turnTime = 0;

    if (!model.gameOver && (player == model.currentPlayer))
        turnTime = GetTime() - model.turnTimer;

    return model.playerTime[player] + turnTime;
}

Piece getBoardPiece(GameModel &model, Square square)
{
    return model.board[square.y][square.x];
}

void setBoardPiece(GameModel &model, Square square, Piece piece)
{
    model.board[square.y][square.x] = piece;
}

bool isSquareValid(Square square)
{
    return (square.x >= 0) &&
           (square.x < BOARD_SIZE) &&
           (square.y >= 0) &&
           (square.y < BOARD_SIZE);
}

void getValidMoves(GameModel &model, Moves &validMoves)
{
	model.validMoves.clear();
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            Square move = {x, y};
			if (getBoardPiece(model, move) != PIECE_EMPTY) continue;
            for (Direction dir : {UP_RIGHT, UP, UP_LEFT, LEFT, LEFT_DOWN, DOWN, RIGHT_DOWN, RIGTH})
            {
                if(checkCurrentSquare(model, move, dir))
                {
                    model.validMoves.push_back(move);
                    validMoves.push_back(move);
                    break;
                }
			}

        }
}
static bool checkCurrentSquare(GameModel& model, Square source, Direction dir)
{
    if (!isSquareValid({ source.x + 2 * dir.x, source.y + 2 * dir.y })) return false;
    if (!isSquareValid({ source.x + dir.x, source.y + dir.y })) return false;
    Player currentPlayer = getCurrentPlayer(model);
    Piece currentPiece = currentPlayer == PLAYER_BLACK ? PIECE_BLACK : PIECE_WHITE;
    Piece oponentPiece = currentPiece == PIECE_BLACK ? PIECE_WHITE : PIECE_BLACK;

    if (model.board[source.y + dir.y][source.x + dir.x] == oponentPiece)
    {
        if (model.board[source.y + 2 * dir.y][source.x + 2 * dir.x] == currentPiece) // Base case
            return true;
        else if (model.board[source.y + 2 * dir.y][source.x + 2 * dir.x] == oponentPiece) // Recursive case
        {
            if (checkCurrentSquare(model, { source.x + dir.x, source.y + dir.y }, dir))
                return true;
        }

    }

    return false;
}

bool playMove(GameModel &model, Square move)
{
    // Set game piece
    Piece piece =
        (getCurrentPlayer(model) == PLAYER_WHITE)
            ? PIECE_WHITE
            : PIECE_BLACK;

    setBoardPiece(model, move, piece);


    eatPieces(model, move, UP_RIGHT);
    eatPieces(model, move, UP);
    eatPieces(model, move, UP_LEFT);
    eatPieces(model, move, LEFT);
    eatPieces(model, move, LEFT_DOWN);
    eatPieces(model, move, DOWN);
    eatPieces(model, move, RIGHT_DOWN);
    eatPieces(model, move, RIGTH);

    // Update timer
    double currentTime = GetTime();
    model.playerTime[model.currentPlayer] += currentTime - model.turnTimer;
    model.turnTimer = currentTime;

    // Swap player
    model.currentPlayer =
        (model.currentPlayer == PLAYER_WHITE)
            ? PLAYER_BLACK
            : PLAYER_WHITE;

    // Game over?
    Moves validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.size() == 0)
    {
        // Swap player
        model.currentPlayer =
            (model.currentPlayer == PLAYER_WHITE)
                ? PLAYER_BLACK
                : PLAYER_WHITE;

        Moves validMoves;
        getValidMoves(model, validMoves);

        if (validMoves.size() == 0)
            model.gameOver = true;
    }

    return true;
}


static bool eatPieces(GameModel &model, Square source, Direction dir)
{
    if(!isSquareValid({source.x+2*dir.x, source.y+2*dir.y})) return false;
    if(!isSquareValid({source.x+dir.x, source.y+dir.y})) return false;

    Player currentPlayer = getCurrentPlayer(model);
    Piece eatablePiece = currentPlayer==PLAYER_BLACK ? PIECE_WHITE : PIECE_BLACK;
    Piece eatingPiece = eatablePiece==PIECE_BLACK ? PIECE_WHITE : PIECE_BLACK;

    if(model.board[source.y+dir.y][source.x+dir.x] == eatablePiece)
    {
        if(model.board[source.y+2*dir.y][source.x+2*dir.x] == eatingPiece) // Base case
        {
            model.board[source.y+dir.y][source.x+dir.x] = eatingPiece;
            return true;
        }else if(model.board[source.y+2*dir.y][source.x+2*dir.x] == eatablePiece) // Recursive case
        {
            if(eatPieces(model, {source.x+dir.x, source.y+dir.y}, dir))
            {
                model.board[source.y+dir.y][source.x+dir.x] = eatingPiece;
                return true;
            }
        }
        
    }
    
    return false; // Reached when model.board[source.y+dir.y][source.x+dir.x] != eatablePiece || model.board[source.y+2*dir.y][source.x+2*dir.x] == PIECE_EMPTY
}
