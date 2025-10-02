/**
 * @brief Implements the Reversi game controller
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <algorithm>
#include <iostream>

#include "raylib.h"

#include "ai.h"
#include "view.h"
#include "controller.h"

extern unsigned int exploratedNodes;

bool updateView(GameModel &model)
{
    if (WindowShouldClose())
        return false;

    if (model.gameOver)
    {
        if (IsMouseButtonPressed(0))
        {
            if (isMousePointerOverPlayBlackButton())
            {
                model.humanPlayer = PLAYER_BLACK;

                startModel(model);
            }
            else if (isMousePointerOverPlayWhiteButton())
            {
                model.humanPlayer = PLAYER_WHITE;

                startModel(model);
            }
        }
    }
    else if (model.currentPlayer == model.humanPlayer)
    {
        if (IsMouseButtonPressed(0))
        {
            // Human player
            Square square = getSquareOnMousePointer();

            if (isSquareValid(square))
            {
                Moves validMoves;
                getValidMoves(model, validMoves);

                // Play move if valid
                for (auto move : validMoves)
                {
                    if ((square.x == move.x) &&
                        (square.y == move.y))
                        playMove(model, square);
                }
            }
        }
    }
    else
    {
        // AI player
        exploratedNodes = 0;
        Square square = getBestMove(model);
        std::cout << "Evaluacion del tablero (Jugador Humano): " << evaluateBoard(model.board, model.humanPlayer) << std::endl;
        std::cout << "Cantidad de nodos explorados: " << exploratedNodes << std::endl << std::endl;
        playMove(model, square);
    }

    if ((IsKeyDown(KEY_LEFT_ALT) ||
         IsKeyDown(KEY_RIGHT_ALT)) &&
        IsKeyPressed(KEY_ENTER))
        ToggleFullscreen();

    

    drawView(model);

    return true;
}
