/**
 * @brief Implements the Reversi game AI
 * @author Marc S. Ressl
 *
 * @copyright Copyright (c) 2023-2024
 */

#include <cstdlib>

#include "ai.h"
#include "controller.h"

Square getBestMove(GameModel &model)
{
    // To-do: your code goes here...



    // +++ TEST
    // Returns a random valid move...
    Moves validMoves;
    getValidMoves(model, validMoves);

    int index = rand() % validMoves.size();
    return validMoves[index];
    // --- TEST
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

int countLegalMoves(const GameModel* game, Player player)
{
    return 5; //OJO ESTO ESTA HARCODEADO, ACA VA LA FUNCION DE LUCCA Y NANO
}



int evaluateBoard(const GameModel* game, Player maxPlayer)
{
    // Convierto el jugador en pieza (esto es re criminal)
    Piece myPiece = playerToPiece(maxPlayer);
    Piece oppPiece = opponentPiece(maxPlayer);

    int myDiscs = 0, oppDiscs = 0;         // Cantidad de fichas propias y del otro
    int myScorePos = 0, oppScorePos = 0;   // Suma de los valores de la tabla de posiciones
    int myFrontier = 0, oppFrontier = 0;   // Fichas que están en adyacentes a casillas vacías
    int emptyCount = 0;                    // Cuantos espacios vacíos quedan

    // Aca guardo las direcciones a chequear
    const int dx[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    const int dy[8] = { -1,  0,  1,-1, 1,-1, 0, 1 };

    // Recorro todo el tablero
    for (int x = 0; x < BOARD_SIZE; x++) {
        for (int y = 0; y < BOARD_SIZE; y++) {
            Piece piece = game->board[y][x];

            // Cuento solo si la casilla esta vacia
            if (piece == PIECE_EMPTY) {
                emptyCount++;
                continue;
            }

            // Si es mi ficha
            if (piece == myPiece) {
                myDiscs++;                            // Sumo a mis fichas
                myScorePos += POSITIONAL_WEIGHTS[x][y]; // Aca mi score depende de donde este ubicada la ficha

                // Verifico si está en la frontera (pegada a un casillero vacio)
                for (int k = 0; k < 8; k++) {
                    int nx = x + dx[k], ny = y + dy[k];
                    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                        game->board[ny][nx] == PIECE_EMPTY) {
                        myFrontier++;
                        break; // Si encuentro un vacio salgo
                    }
                }
            }

            // Si es la ficha del otro
            else if (piece == oppPiece) {
                oppDiscs++;
                oppScorePos += POSITIONAL_WEIGHTS[x][y];

                for (int k = 0; k < 8; k++) {
                    int nx = x + dx[k], ny = y + dy[k];
                    if (nx >= 0 && nx < BOARD_SIZE && ny >= 0 && ny < BOARD_SIZE &&
                        game->board[ny][nx] == PIECE_EMPTY) {
                        oppFrontier++;
                        break;
                    }
                }
            }
        }
    }

    // Estas son las metricas que estoy tomando para evaluar una posicion

    // Paridad de fichas (diferencia porcentual entre mis fichas y las del rival)
    int parity = (myDiscs + oppDiscs > 0)
        ? 100 * (myDiscs - oppDiscs) / (myDiscs + oppDiscs)
        : 0;

    // Movilidad (quién tiene más movimientos legales disponibles)
    int myMoves = countLegalMoves(game, maxPlayer);
    int oppMoves = countLegalMoves(game, (maxPlayer == PLAYER_BLACK) ? PLAYER_WHITE : PLAYER_BLACK);
    int mobility = (myMoves + oppMoves > 0)
        ? 100 * (myMoves - oppMoves) / (myMoves + oppMoves)
        : 0;

    // Frontera (tener muchas piezas en frontera es malo -> por eso se resta)
    int frontier = (myFrontier + oppFrontier > 0)
        ? -100 * (myFrontier - oppFrontier) / (myFrontier + oppFrontier)
        : 0;

    // Puntaje posicional (sumo los valores de la tabla de pesos de mis fichas vs las del rival)
    int positional = myScorePos - oppScorePos;

    // Veo en que etapa del juego estoy
    int score = 0;
    if (emptyCount > 44) { // early game (muchos huecos todavía)
        score = 10 * parity + 80 * mobility + 30 * frontier + 100 * positional;
    }
    else if (emptyCount > 20) { // mid game
        score = 10 * parity + 70 * mobility + 50 * frontier + 50 * positional;
    }
    else { // late game (quedan pocas casillas)
        score = 100 * parity + 10 * mobility + 20 * frontier + 10 * positional;
    }

    return score; // devuelvo la evaluación final de la posición
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
