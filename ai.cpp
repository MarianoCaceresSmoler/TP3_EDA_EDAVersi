/**
 * @brief Implements the Reversi game AI
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <cstdlib>
#include <iostream>
#include "ai.h"
#include "controller.h"
#define INF 10000

unsigned int exploratedNodes = 0;

Square getBestMove(GameModel &model)
{
    // To-do: your code goes here...
    
    return findBestMove(model, 7);

    // +++ TEST
    // Returns a random valid move...
    Moves validMoves;
    getValidMoves(model, validMoves);

    int index = rand() % validMoves.size();
    return validMoves[index];
    // --- TEST
}



Square findBestMove(GameModel model, int depth) {
    int bestValue = -INF;
    Square bestMove = { -1, -1 };

    Moves validMoves;
    getValidMoves(model, validMoves);

    for (const auto& move : validMoves) {
        GameModel nextModel = model;
        playMove(nextModel, move);

        int moveValue = minimax(nextModel, depth - 1,-INF,INF, false, model.currentPlayer);

        if (moveValue > bestValue) {
            bestValue = moveValue;
            bestMove = move;
        }
    }

    return bestMove;
}

//alpha-beta pruning minimax, alpha setea el mejor valor que el maximizador puede asegurar en ese nivel o niveles superiores y beta el mejor valor que el minimizador puede asegurar en ese nivel o niveles superiores
int minimax(GameModel model, int depth, int alpha, int beta, bool maximizingPlayer, Player maxPlayer) {
    // Caso base
    if (depth == 0 || model.gameOver) {
        return evaluateBoard(model.board, maxPlayer);
    }

    Moves validMoves;
    getValidMoves(model, validMoves);

    if (validMoves.empty()) {
        // Si no hay jugadas válidas, el jugador pasa turno automáticamente
        GameModel nextModel = model;
        nextModel.currentPlayer = (model.currentPlayer == PLAYER_BLACK ? PLAYER_WHITE : PLAYER_BLACK);
        return minimax(nextModel, depth - 1,alpha,beta, !maximizingPlayer, maxPlayer);
    }

    if (maximizingPlayer) {
        int maxEval = -INF;
        for (const auto& move : validMoves) {
            GameModel nextModel = model;       // copia del estado
            playMove(nextModel, move);         // aplica la jugada
            int eval = minimax(nextModel, depth - 1,alpha,beta, false, maxPlayer);
			if (eval > maxEval) maxEval = eval;
			if (eval > alpha) alpha = eval; // actualiza alpha
			if (beta <= alpha) break; // poda beta
        }
        return maxEval;
    }
    else {
        int minEval = INF;
        for (const auto& move : validMoves) {
            GameModel nextModel = model;
            playMove(nextModel, move);
            int eval = minimax(nextModel, depth - 1,alpha,beta, true, maxPlayer);
			if (eval < minEval) minEval = eval;
			if (eval < beta) beta = eval; // actualiza beta
			if (beta <= alpha) break; // poda alpha
        }
        return minEval;
    }
}





// Tabla de pesos estáticos para Reversi
static const int POSITIONAL_WEIGHTS[BOARD_SIZE][BOARD_SIZE] = {
    {120, -20, 20,  5,  5, 20, -20, 120},
    {-20, -40, -5, -5, -5, -5, -40, -20},
    { 20,  -5, 15,  3,  3, 15,  -5,  20},
    {  5,  -5,  3,  3,  3,  3,  -5,   5},
    {  5,  -5,  3,  3,  3,  3,  -5,   5},
    { 20,  -5, 15,  3,  3, 15,  -5,  20},
    {-20, -40, -5, -5, -5, -5, -40, -20},
    {120, -20, 20,  5,  5, 20, -20, 120}
};

// Esto es una porqueria hay que cambiarlo
inline Piece playerToPiece(Player p) {
    return (p == PLAYER_BLACK) ? PIECE_BLACK : PIECE_WHITE;
}

inline Piece opponentPiece(Player p) {
    return (p == PLAYER_BLACK) ? PIECE_WHITE : PIECE_BLACK;
}

int evaluateBoard(Board board, Player maxPlayer)
{
    //Cuento la cantidad de nodos explorados
    exploratedNodes++;

    // Convierto el jugador en pieza
    Piece myPiece = playerToPiece(maxPlayer);
    Piece oppPiece = opponentPiece(maxPlayer);

    // Matricas basicas
    int myDiscs = 0, oppDiscs = 0;         // Cantidad de fichas propias y del rival
    int myScorePos = 0, oppScorePos = 0;   // Suma de los valores de la tabla de posiciones
    int myFrontier = 0, oppFrontier = 0;   // Cantidad de fichas adyacentes a casillas vacías
    int emptyCount = 0;                    // Cuántos espacios vacíos quedan en el tablero

    // Direcciones posibles (8 alrededor de una casilla)
    const int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    const int dy[8] = { -1,  0,  1,-1, 1,-1, 0, 1 };

    // Recorro todo el tablero
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            Piece piece = board[y][x];

            // Si la casilla está vacia
            if (piece == PIECE_EMPTY) {
                emptyCount++;
                continue;
            }

            // Si es mi ficha
            if (piece == myPiece) {
                myDiscs++;                            // Sumo a mis fichas
                myScorePos += POSITIONAL_WEIGHTS[x][y]; // Sumo el valor posicional

                // Verifico si está en la frontera (adyacente a un casillero vacio)
                for (int k = 0; k < 8; k++) {
                    int nx = x + dx[k], ny = y + dy[k];
                    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                        board[ny][nx] == PIECE_EMPTY) {
                        myFrontier++;
                        break; // Si encuentro un vacío salgo
                    }
                }
            }

            // Si es ficha del rival
            else if (piece == oppPiece) {
                oppDiscs++;                             // Sumo a las fichas del rival
                oppScorePos += POSITIONAL_WEIGHTS[x][y]; // Sumo el valor posicional

                // Verifico si esta en la frontera
                for (int k = 0; k < 8; k++) {
                    int nx = x + dx[k], ny = y + dy[k];
                    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                        board[ny][nx] == PIECE_EMPTY) {
                        oppFrontier++;
                        break;
                    }
                }
            }
        }
    }


    // Normalización de métricas

    // Paridad de fichas (diferencia relativa de cantidad de fichas)
    double parity = (myDiscs + oppDiscs > 0)
        ? (double)(myDiscs - oppDiscs) / (myDiscs + oppDiscs)
        : 0.0;

    // Movilidad (quién tiene más movimientos legales disponibles)
    int myMoves = getValidMovesNumber(board, maxPlayer);
    int oppMoves = getValidMovesNumber(board, (maxPlayer == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK);
    double mobility = (myMoves + oppMoves > 0)
        ? (double)(myMoves - oppMoves) / (myMoves + oppMoves)
        : 0.0;

    // Frontier (tener más fichas en frontera es malo entonces se resta)
    double frontier = (myFrontier + oppFrontier > 0)
        ? -(double)(myFrontier - oppFrontier) / (myFrontier + oppFrontier)
        : 0.0;

    // Puntaje posicional (normalizado por el máximo absoluto posible 7680 = 64 * 120)
    double positional = (double)(myScorePos - oppScorePos) / 7680.0;

    // Evaluación según etapa del juego
    double score = 0.0;
    if (emptyCount > 44) { // early game (muchos espacios vacíos todavía)
        score = 0.1 * parity + 0.8 * mobility + 0.3 * frontier + 1.0 * positional;
    }
    else if (emptyCount > 20) { // mid game
        score = 0.1 * parity + 0.7 * mobility + 0.5 * frontier + 0.5 * positional;
    }
    else { // late game (quedan pocas casillas)
        score = 1.0 * parity + 0.1 * mobility + 0.2 * frontier + 0.1 * positional;
    }

    // Escalo el score a un rango legible [-100, 100]
    return (int)(score * 100);
}



/*
Principios de una buena función de evaluación en Reversi
--------------------------------------------------------
- No alcanza con contar fichas: en las etapas tempranas/medias del juego el conteo bruto engaña.
- Lo importante es controlar la movilidad y la estabilidad.
- Conviene ponderar distinto según la etapa del juego (early, mid, late).

Heurísticas clásicas
--------------------
La función de evaluación puede ser una combinación ponderada:

   Eval(s) = w1*C(s) + w2*M(s) + w3*S(s) + w4*P(s) + w5*F(s)

Donde:

1) C(s) - Coin Parity (diferencia de fichas)
   C(s) = 100 * (myDiscs - oppDiscs) / (myDiscs + oppDiscs)
   ? Importante casi exclusivamente al final del juego.

2) M(s) - Mobility (movilidad)
   Diferencia en número de movimientos legales posibles.
   M(s) = 100 * (myMoves - oppMoves) / (myMoves + oppMoves + 1)

3) S(s) - Stability (estabilidad)
   Cuántas fichas no se pueden voltear jamás (esquinas, bordes y protegidas).
   Una ficha estable es una ventaja permanente.

4) P(s) - Positional Weights (tabla de pesos estática)
   Usar una tabla de evaluación típica 8x8:

      120 -20  20   5   5  20 -20 120
      -20 -40  -5  -5  -5  -5 -40 -20
       20  -5  15   3   3  15  -5  20
        5  -5   3   3   3   3  -5   5
        5  -5   3   3   3   3  -5   5
       20  -5  15   3   3  15  -5  20
      -20 -40  -5  -5  -5  -5 -40 -20
      120 -20  20   5   5  20 -20 120

   - Esquinas = oro (120).
   - Casillas adyacentes a esquina (X-squares y C-squares) = muy malas (-40, -20).
   - Bordes = intermedios.
   - Centro = neutral/ligeramente positivo.

5) F(s) - Frontier Discs (discos frontera)
   Discos adyacentes a un espacio vacío.
   Cuantos menos discos frontera, mejor (más solidez).
   F(s) = -100 * (myFrontier - oppFrontier) / (myFrontier + oppFrontier + 1)

Etapas del juego
----------------
Conviene ajustar los pesos según la cantidad de casillas vacías:

- Early game (0–20 movs): enfatizar movilidad, evitar esquinas malas, tabla posicional.
- Mid game (20–40 movs): movilidad + estabilidad + frontera.
- Late game (40+ movs): el conteo de fichas (coin parity) domina.

Ejemplo de pesos efectivos:

Factor        Early   Mid   Late
--------------------------------
Coin parity     10     10    100
Mobility        80     70     10
Stability       50    100    200
Positional     100     50     10
Frontier        30     50     20
*/
