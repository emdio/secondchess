/*
secondchess - gpl, by Emilio Díaz, based on firstchess by Pham Hong Nguyen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>

 * BASIC PARTS: *
 * Some definitions *
 * Board representation and main varians *
 * Move generator *
 * Evaluation for current position *
 * Make and Take back a move *
 * IsInCheck *
 * IsAttacked *
 * Search function - a typical alphabeta *
 * Quiescent search
 * Utilities *
 * Main program *
 */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

//#define NDEBUG
#include <assert.h>

/*
 ****************************************************************************
 * Some definitions *
 ****************************************************************************
 */
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOK 3
#define QUEEN 4
#define KING 5
#define EPS_SQUARE 6
#define EMPTY 7
#define WHITE 0
#define BLACK 1
#define false 0

/* The values of the pieces */
#define VALUE_PAWN 100
#define VALUE_KNIGHT 310
#define VALUE_BISHOP 320
#define VALUE_ROOK 500
#define VALUE_QUEEN 900
#define VALUE_KING 10000

#define MATE 10000 /* equal value of King, losing King==mate */

#define COL(pos) ((pos)&7)
#define ROW(pos) (((unsigned)pos)>>3)

/* For move generation */
#define MOVE_TYPE_NONE 0
#define MOVE_TYPE_NORMAL 1
#define MOVE_TYPE_CASTLE 2
#define MOVE_TYPE_PAWN_TWO 3
#define MOVE_TYPE_EPS 4
#define MOVE_TYPE_PROMOTION_TO_QUEEN 5
#define MOVE_TYPE_PROMOTION_TO_ROOK 6
#define MOVE_TYPE_PROMOTION_TO_BISHOP 7
#define MOVE_TYPE_PROMOTION_TO_KNIGHT 8

/* Some useful squares */
#define A1 56
#define B1 57
#define C1 58
#define D1 59
#define E1 60
#define F1 61
#define G1 62
#define H1 63
#define A8 0
#define B8 1
#define C8 2
#define D8 3
#define E8 4
#define F8 5
#define G8 6
#define H8 7

/*
 ****************************************************************************
 * Board representation and main variants *
 ****************************************************************************
 */
/* Board representation */

/* Piece in each square */
int piece[64] = {
        ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
        PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        PAWN, PAWN, PAWN, PAWN, PAWN,PAWN, PAWN, PAWN,
        ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT,ROOK };

/* Color of each square */
int color[64] = {
        BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
        BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
        WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
        WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE };

//int piece[64] = {
//        KNIGHT, EMPTY, KNIGHT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        PAWN, PAWN, PAWN, KING, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, KING, PAWN, PAWN, PAWN,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, KNIGHT, EMPTY, KNIGHT};

///* Color of each square */
//int color[64] = {
//        BLACK, EMPTY, BLACK, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        WHITE, WHITE, WHITE, BLACK, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, WHITE, BLACK, BLACK, BLACK,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, WHITE, EMPTY, WHITE };


int side; /* Side to move, value = BLACK or WHITE */
int computer_side;
int max_depth; /* max depth to search */

/* A move is defined by its origin and final squares, the castle rights and the kind of
 * move it's: normal, enpasant... */
typedef struct tag_MOVE
{
	int from;
	int dest;
	int castle;
	int type;
} MOVE;

/* For storing all moves of game */
typedef struct tag_HIST
{
	MOVE m;
	int cap;
} HIST;

HIST hist[6000]; /* Game length < 6000 */

/* For castle rights we use a bitfield, like in TSCP
 *
 * 0001 -> White can short castle
 * 0010 -> White can long castle
 * 0100 -> Black can short castle
 * 1000 -> Black can long castle
 *
 * 15 = 1111 = 1*2^3 + 1*2^2 + 1*2^1 + 1*2^0
 *
 */
int castle = 15; /* At start position all castle types ar available*/


/* This mask is applied like this
 *
 * castle &= castle_mask[from] & castle_mask[dest]
 *
 * When from and dest are whatever pieces, then nothing happens, otherwise
 * the values are chosen in such a way that if vg the white king moves
 * to F1 then
 *
 * castle = castle & (12 & 15)
 * 1111 & (1100 & 1111) == 1111 & 1100 == 1100
 *
 * and white's lost all its castle rights
 *
 * */
int castle_mask[64] = {
        7,  15, 15, 15,  3, 15, 15, 11,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		15, 15, 15, 15, 15, 15, 15, 15,
		13, 15, 15, 15, 12, 15, 15, 14 };

int hdp; /* Current move order */
int allmoves = 0;

/* For searching */
int nodes; /* Count all visited nodes when searching */
int ply; /* ply of search */
int count_evaluations;
int count_checks;
int count_MakeMove;
int countquiesCalls;
int countCapCalls;

/* The values of the pieces in centipawns */
int value_piece[6] =
{ VALUE_PAWN, VALUE_KNIGHT, VALUE_BISHOP, VALUE_ROOK, VALUE_QUEEN, VALUE_KING };

/* * * * * * * * * * * * *
 * Piece Square Tables
 * * * * * * * * * * * * */
/* When evaluating the position we'll add a bonus (or malus) to each piece
 * depending on the very square where it's placed. Vg, a knight in d4 will
 * be given an extra +15, whilst a knight in a1 will be penalized with -40.
 * This simple idea allows the engine to make more sensible moves */
int pst_pawn[64] ={
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0, 15, 15,  0,  0,  0,
        0,  0,  0, 10, 10,  0,  0,  0,
        0,  0,  0,  5,  5,  0,  0,  0,
        0,  0,  0,-25,-25,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0 };

int pst_knight[64] = {
		-40,-25,-25,-25,-25,-25,-25,-40,
        -30,  0,  0,  0,  0,  0,  0,-30,
        -30,  0,  0,  0,  0,  0,  0,-30,
        -30,  0,  0, 15, 15,  0,  0,-30,
        -30,  0,  0, 15, 15,  0,  0,-30,
        -30,  0, 10,  0,  0, 10,  0,-30,
        -30,  0,  0,  5,  5,  0,  0,-30,
		-40,-30,-25,-25,-25,-25,-30,-40 };

int pst_bishop[64] = {
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -10,  0,  5,  0,  0,  5,  0,-10,
        -10,  0,  0, 10, 10,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  0,  5,  0,  0,  5,  0,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
		-10,-20,-20,-20,-20,-20,-20,-10 };

int pst_rook[64] = {
         0,  0,  0,  0,  0,  0,  0,  0,
		10, 10, 10, 10, 10, 10, 10, 10,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  0,  0,  0,  0,  0,
         0,  0,  0,  5,  5,  0,  0,  0 };

int pst_king[64] = {
		-25,-25,-25,-25,-25,-25,-25,-25,
		-25,-25,-25,-25,-25,-25,-25,-25,
		-25,-25,-25,-25,-25,-25,-25,-25,
		-25,-25,-25,-25,-25,-25,-25,-25,
		-25,-25,-25,-25,-25,-25,-25,-25,
		-25,-25,-25,-25,-25,-25,-25,-25,
		-25,-25,-25,-25,-25,-25,-25,-25,
         10, 15,-15,-15,-15,-15, 15, 10 };

/* The flip array is used to calculate the piece/square
values for BLACKS pieces, without needing to write the
arrays for them (idea taken from TSCP).
The piece/square value of a white pawn is pst_pawn[sq]
and the value of a black pawn is pst_pawn[flip[sq]] */
int flip[64] = {
		56, 57, 58, 59, 60, 61, 62, 63,
		48, 49, 50, 51, 52, 53, 54, 55,
		40, 41, 42, 43, 44, 45, 46, 47,
		32, 33, 34, 35, 36, 37, 38, 39,
		24, 25, 26, 27, 28, 29, 30, 31,
		16, 17, 18, 19, 20, 21, 22, 23,
         8,  9, 10, 11, 12, 13, 14, 15,
         0,  1,  2,  3,  4,  5,  6,  7 };

/*
 ****************************************************************************
 * Move generator *
 ****************************************************************************
 */
void Gen_Push(int from, int dest, int type, MOVE * pBuf, int *pMCount)
{
	MOVE move;
	move.from = from;
	move.dest = dest;
	move.type = type;
	move.castle = castle;
	pBuf[*pMCount] = move;
	*pMCount = *pMCount + 1;
}

void Gen_PushNormal(int from, int dest, MOVE * pBuf, int *pMCount)
{
	Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
}

/* Especial cases for Pawn */

/* Pawn can promote */
void Gen_PushPawn(int from, int dest, MOVE * pBuf, int *pMCount)
{
	/* The 7 and 56 are to limit pawns to the 2nd through 7th ranks, which
	 * means this isn't a promotion, i.e., a normal pawn move */
	if (piece[dest] == EPS_SQUARE)
	{
		Gen_Push(from, dest, MOVE_TYPE_EPS, pBuf, pMCount);
	}
	else if (dest > 7 && dest < 56) /* this is just a normal move */
	{
		Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
	}
	else /* otherwise it's a promotion */
	{
		Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_QUEEN, pBuf, pMCount);
		Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_ROOK, pBuf, pMCount);
		Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_BISHOP, pBuf, pMCount);
		Gen_Push(from, dest, MOVE_TYPE_PROMOTION_TO_KNIGHT, pBuf, pMCount);
	}
}

/* When a pawn moves two squares then appears the possibility of the en passanta capture*/
void Gen_PushPawnTwo(int from, int dest, MOVE * pBuf, int *pMCount)
{
	Gen_Push(from, dest, MOVE_TYPE_PAWN_TWO, pBuf, pMCount);
}

/* Especial cases for King */
void Gen_PushKing(int from, int dest, MOVE * pBuf, int *pMCount)
{
	/* Is it a castle?*/
    if (from == E1 && (dest == G1 || dest == C1)) /* this is a white castle */
	{
		Gen_Push(from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
	}
    else if (from == E8 && (dest == G8 || dest == C8)) /* this is a black castle */
	{
		Gen_Push(from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
	}
	else /* otherwise it's a normal king's move */
	{
		Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
	}
}

/* Gen all moves of current_side to move and push them to pBuf, and return number of moves */
int GenMoves(int current_side, MOVE * pBuf)
{
	int i; /* Counter for the board squares */
	int k; /* Counter for cols */
	int y;
	int row;
	int col;
	int movecount;
	movecount = 0;

	for (i = 0; i < 64; i++) /* Scan all board */
		if (color[i] == current_side)
		{
			switch (piece[i])
			{

			case PAWN:
				col = COL(i);
				row = ROW(i);
				if (current_side == BLACK)
				{
					if (color[i + 8] == EMPTY)
						/* Pawn advances one square.
						 * We use Gen_PushPawn because it can be a promotion */
						Gen_PushPawn(i, i + 8, pBuf, &movecount);
					if (row == 1 && color[i + 8] == EMPTY && color[i + 16] == EMPTY)
						/* Pawn advances two squares */
						Gen_PushPawnTwo(i, i + 16, pBuf, &movecount);
					if (col && color[i + 7] == WHITE)
						/* Pawn captures and it can be a promotion*/
						Gen_PushPawn(i, i + 7, pBuf, &movecount);
					if (col < 7 && color[i + 9] == WHITE)
						/* Pawn captures and can be a promotion*/
						Gen_PushPawn(i, i + 9, pBuf, &movecount);
                        /* For en passant capture */
					if (col && piece[i + 7] == EPS_SQUARE)
						/* Pawn captures and it can be a promotion*/
						Gen_PushPawn(i, i + 7, pBuf, &movecount);
					if (col < 7 && piece[i + 9] == EPS_SQUARE)
						/* Pawn captures and can be a promotion*/
						Gen_PushPawn(i, i + 9, pBuf, &movecount);
				}
				else
				{
					if (color[i - 8] == EMPTY)
						Gen_PushPawn(i, i - 8, pBuf, &movecount);
					/* Pawn moves 2 squares */
					if (row == 6 && color[i - 8] == EMPTY && color[i - 16] == EMPTY)
						Gen_PushPawnTwo(i, i - 16, pBuf, &movecount);
					/* For captures */
					if (col && color[i - 9] == BLACK)
						Gen_PushPawn(i, i - 9, pBuf, &movecount);
					if (col < 7 && color[i - 7] == BLACK)
						Gen_PushPawn(i, i - 7, pBuf, &movecount);
					/* For en passant capture */
					if (col && piece[i - 9] == EPS_SQUARE)
						Gen_PushPawn(i, i - 9, pBuf, &movecount);
					if (col < 7 && piece[i - 7] == EPS_SQUARE)
						Gen_PushPawn(i, i - 7, pBuf, &movecount);
				}
				break;

			case QUEEN: /* == BISHOP+ROOK */

			case BISHOP:
				for (y = i - 9; y >= 0 && COL(y) != 7; y -= 9)
				{ /* go left up */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				for (y = i - 7; y >= 0 && COL(y) != 0; y -= 7)
				{ /* go right up */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				for (y = i + 9; y < 64 && COL(y) != 0; y += 9)
				{ /* go right down */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				for (y = i + 7; y < 64 && COL(y) != 7; y += 7)
				{ /* go left down */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				if (piece[i] == BISHOP) /* In the case of the bishop we're done */
					break;

				/* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
			case ROOK:
				col = COL(i);
				for (k = i - col, y = i - 1; y >= k; y--)
				{ /* go left */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				for (k = i - col + 7, y = i + 1; y <= k; y++)
				{ /* go right */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				for (y = i - 8; y >= 0; y -= 8)
				{ /* go up */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				for (y = i + 8; y < 64; y += 8)
				{ /* go down */
					if (color[y] != current_side)
						Gen_PushNormal(i, y, pBuf, &movecount);
					if (color[y] != EMPTY)
						break;
				}
				break;

			case KNIGHT:
				col = COL(i);
				y = i - 6;
				if (y >= 0 && col < 6 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i - 10;
				if (y >= 0 && col > 1 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i - 15;
				if (y >= 0 && col < 7 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i - 17;
				if (y >= 0 && col > 0 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i + 6;
				if (y < 64 && col > 1 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i + 10;
				if (y < 64 && col < 6 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i + 15;
				if (y < 64 && col > 0 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				y = i + 17;
				if (y < 64 && col < 7 && color[y] != current_side)
					Gen_PushNormal(i, y, pBuf, &movecount);
				break;

			case KING:
				/* the column and rank checks are to make sure it is on the board*/
                /* The 'normal' moves*/
				col = COL(i);
				if (col && color[i - 1] != current_side)
					Gen_PushKing(i, i - 1, pBuf, &movecount); /* left */
				if (col < 7 && color[i + 1] != current_side)
					Gen_PushKing(i, i + 1, pBuf, &movecount); /* right */
				if (i > 7 && color[i - 8] != current_side)
					Gen_PushKing(i, i - 8, pBuf, &movecount); /* up */
				if (i < 56 && color[i + 8] != current_side)
					Gen_PushKing(i, i + 8, pBuf, &movecount); /* down */
				if (col && i > 7 && color[i - 9] != current_side)
					Gen_PushKing(i, i - 9, pBuf, &movecount); /* left up */
				if (col < 7 && i > 7 && color[i - 7] != current_side)
					Gen_PushKing(i, i - 7, pBuf, &movecount); /* right up */
				if (col && i < 56 && color[i + 7] != current_side)
					Gen_PushKing(i, i + 7, pBuf, &movecount); /* left down */
				if (col < 7 && i < 56 && color[i + 9] != current_side)
					Gen_PushKing(i, i + 9, pBuf, &movecount); /* right down */

                /* The castle moves*/
				if (current_side == WHITE)
				{
					/* Can white short castle? */
					if (castle & 1)
					{
						/* If white can castle the white king has to be in square 60 */
						if (col &&
                            color[i + 1] == EMPTY &&
                            color[i + 2] == EMPTY &&
                            !IsInCheck(current_side) &&
                            !IsAttacked(current_side, i + 1))
						{
							/* The king goes 2 sq to the left */
							Gen_PushKing(i, i + 2, pBuf, &movecount);
						}
					}

					/* Can white long castle? */
					if (castle & 2)
					{
						if (col &&
                            color[i - 1] == EMPTY &&
                            color[i - 2] == EMPTY &&
                            color[i - 3] == EMPTY &&
                            !IsInCheck(current_side) &&
                            !IsAttacked(current_side, i - 1))
						{
							/* The king goes 2 sq to the left */
							Gen_PushKing(i, i - 2, pBuf, &movecount);
						}
					}
				}
				else if (current_side == BLACK)
				{
					/* Can black short castle? */
					if (castle & 4)
					{
						/* If white can castle the white king has to be in square 60 */
						if (col &&
								color[i + 1] == EMPTY &&
								color[i + 2] == EMPTY &&
								piece[i + 3] == ROOK &&
								!IsInCheck(current_side) &&
								!IsAttacked(current_side, i + 1))
						{
							/* The king goes 2 sq to the left */
							Gen_PushKing(i, i + 2, pBuf, &movecount);
						}
					}
					/* Can black long castle? */
					if (castle & 8)
					{
						if (col &&
								color[i - 1] == EMPTY &&
								color[i - 2] == EMPTY &&
								color[i - 3] == EMPTY &&
								piece[i - 4] == ROOK &&
								!IsInCheck(current_side) &&
								!IsAttacked(current_side, i - 1))
						{
							/* The king goes 2 sq to the left */
							Gen_PushKing(i, i - 2, pBuf, &movecount);
						}
					}
				}

				break;
//                default:
//                printf("Piece type unknown, %d", piece[i]);
				// assert(false);
			}
		}
	return movecount;
}

/* Gen all captures of current_side to move and push them to pBuf, return number of moves
 * It's necesary at least ir order to use quiescent in the search */
int GenCaps(int current_side, MOVE * pBuf)
{
	int i; /* Counter for the board squares */
	int k; /* Counter for cols */
	int y;
    int row;
    int col;
	int capscount; /* Counter for the posible captures */
	int xside;
	xside = (WHITE + BLACK) - current_side;
	capscount = 0;

	for (i = 0; i < 64; i++) /* Scan all board */
		if (color[i] == current_side)
		{
			switch (piece[i])
			{

			case PAWN:
				col = COL(i);
                row = ROW(i);
				if (current_side == BLACK)
				{
                    /* This isn't a capture, but it's necesary in order to
					 * not oversee promotions */
                    if (row > 7 && color[i + 8] == EMPTY)
                        /* Pawn advances one square.
                         * We use Gen_PushPawn because it can be a promotion */
                        Gen_PushPawn(i, i + 8, pBuf, &capscount);
					if (col && color[i + 7] == WHITE)
						/* Pawn captures and it can be a promotion*/
						Gen_PushPawn(i, i + 7, pBuf, &capscount);
					if (col < 7 && color[i + 9] == WHITE)
						/* Pawn captures and can be a promotion*/
						Gen_PushPawn(i, i + 9, pBuf, &capscount);
					/* For en passant capture */
					if (col && piece[i + 7] == EPS_SQUARE)
						/* Pawn captures and it can be a promotion*/
						Gen_PushPawn(i, i + 7, pBuf, &capscount);
					if (col < 7 && piece[i + 9] == EPS_SQUARE)
						/* Pawn captures and can be a promotion*/
						Gen_PushPawn(i, i + 9, pBuf, &capscount);
				}
				else if (current_side == WHITE)
				{
                    if (row < 2 && color[i - 8] == EMPTY)
                    /* This isn't a capture, but it's necesary in order to
                     * not oversee promotions */
                        Gen_PushPawn(i, i - 8, pBuf, &capscount);
					/* For captures */
					if (col && color[i - 9] == BLACK)
						Gen_PushPawn(i, i - 9, pBuf, &capscount);
					if (col < 7 && color[i - 7] == BLACK)
						Gen_PushPawn(i, i - 7, pBuf, &capscount);
					/* For en passant capture */
					if (col && piece[i - 9] == EPS_SQUARE)
						Gen_PushPawn(i, i - 9, pBuf, &capscount);
					if (col < 7 && piece[i - 7] == EPS_SQUARE)
						Gen_PushPawn(i, i - 7, pBuf, &capscount);
				}
				break;

			case KNIGHT:
				col = COL(i);
				y = i - 6;
				if (y >= 0 && col < 6 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i - 10;
				if (y >= 0 && col > 1 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i - 15;
				if (y >= 0 && col < 7 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i - 17;
				if (y >= 0 && col > 0 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i + 6;
				if (y < 64 && col > 1 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i + 10;
				if (y < 64 && col < 6 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i + 15;
				if (y < 64 && col > 0 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				y = i + 17;
				if (y < 64 && col < 7 && color[y] == xside)
					Gen_PushNormal(i, y, pBuf, &capscount);
				break;

			case QUEEN: /* == BISHOP+ROOK */

			case BISHOP:
				for (y = i - 9; y >= 0 && COL(y) != 7; y -= 9)
				{ /* go left up */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				for (y = i - 7; y >= 0 && COL(y) != 0; y -= 7)
				{ /* go right up */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				for (y = i + 9; y < 64 && COL(y) != 0; y += 9)
				{ /* go right down */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				for (y = i + 7; y < 64 && COL(y) != 7; y += 7)
				{ /* go left down */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				if (piece[i] == BISHOP) /* In the case of the bishop we're done */
					break;

				/* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
			case ROOK:
				col = COL(i);
				for (k = i - col, y = i - 1; y >= k; y--)
				{ /* go left */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				for (k = i - col + 7, y = i + 1; y <= k; y++)
				{ /* go right */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				for (y = i - 8; y >= 0; y -= 8)
				{ /* go up */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				for (y = i + 8; y < 64; y += 8)
				{ /* go down */
					if (color[y] != EMPTY)
					{
						if (color[y] != current_side)
							Gen_PushNormal(i, y, pBuf, &capscount);
						break;
					}
				}
				break;

			case KING:
				///* the column and rank checks are to make sure it is on the board*/
				col = COL(i);
				if (col && color[i - 1] == xside)
					Gen_PushKing(i, i - 1, pBuf, &capscount); /* left */
				if (col < 7 && color[i + 1] == xside)
					Gen_PushKing(i, i + 1, pBuf, &capscount); /* right */
				if (i > 7 && color[i - 8] == xside)
					Gen_PushKing(i, i - 8, pBuf, &capscount); /* up */
				if (i < 56 && color[i + 8] == xside)
					Gen_PushKing(i, i + 8, pBuf, &capscount); /* down */
				if (col && i > 7 && color[i - 9] == xside)
					Gen_PushKing(i, i - 9, pBuf, &capscount); /* left up */
				if (col < 7 && i > 7 && color[i - 7] == xside)
					Gen_PushKing(i, i - 7, pBuf, &capscount); /* right up */
				if (col && i < 56 && color[i + 7] == xside)
					Gen_PushKing(i, i + 7, pBuf, &capscount); /* left down */
				if (col < 7 && i < 56 && color[i + 9] == xside)
					Gen_PushKing(i, i + 9, pBuf, &capscount); /* right down */
				break;
//				 default:
//				 printf("Piece type unknown");
				// assert(false);
			}
		}
	return capscount;
}

/*
 ****************************************************************************
 * Evaluation for current position - main "brain" function *
 * Lack: almost no knowlegde; material value + piece square tables *
 ****************************************************************************
 */
int Eval()
{

	count_evaluations++;

	/* A counter for the board squares */
	int i;

	/* The score of the position */
	int score = 0;

	/* Check all the squares searching for the pieces */
	for (i = 0; i < 64; i++)
	{
		if (color[i] == WHITE)
		{
			/* In the current square, add the material
			 * value of the piece */
			score += value_piece[piece[i]];

			/* Now we add to the evaluation the value of the
			 * piece square tables */
			switch (piece[i])
			{
			case PAWN:
				score += pst_pawn[i];
				break;
			case KNIGHT:
				score += pst_knight[i];
				break;
			case BISHOP:
				score += pst_bishop[i];
				break;
			case ROOK:
				score += pst_rook[i];
				break;
			case KING:
				score += pst_king[i];
				break;
			}
		}
        /* Now the evaluation for black: note the change of
          the sign in the score*/
		else if (color[i] == BLACK)
		{
			score -= value_piece[piece[i]];

			switch (piece[i])
			{
			case PAWN:
				score -= pst_pawn[flip[i]];
				break;
			case KNIGHT:
				score -= pst_knight[flip[i]];
				break;
			case BISHOP:
				score -= pst_bishop[flip[i]];
				break;
			case ROOK:
				score -= pst_rook[flip[i]];
				break;
			case KING:
				score -= pst_king[flip[i]];
				break;
			}
		}
	}

    /* Finally we return the score, taking into account the side to move */
	if (side == WHITE)
		return score;
	return -score;
}

/*
 ****************************************************************************
 * Make and Take back a move, IsInCheck *
 ****************************************************************************
 */

/* Check if current side is in check. Necesary in order to check legality of moves
 and check if castle is allowed */
int IsInCheck(int current_side)
{
	int k; /* The square where the king is placed */

    /* Find the King of the side to move */
    for (k = 0; k < 64; k++)
        if ((piece[k] == KING) && color[k] == current_side)
            break;

    /* Use IsAttacked in order to know if current_side is under check */
    return IsAttacked(current_side, k);
}

/* Check and return 1 if square k is attacked by current_side, 0 otherwise. Necesary, vg, to check
 * castle rules (if king goes from e1 to g1, f1 can't be attacked by an enemy piece) */
int IsAttacked(int current_side, int k)
{
	int h;
	int y;
    int row; /* Row where the square is placed */
	int col; /* Col where the square is placed */
	int xside;
	xside = (WHITE + BLACK) - current_side; /* opposite current_side, who may be attacking */

	/* Situation of the square*/
	row = ROW(k);
	col = COL(k);

	/* Check Knight attack */
	if (col > 0 && row > 1 && color[k - 17] == xside && piece[k - 17] == KNIGHT)
		return 1;
	if (col < 7 && row > 1 && color[k - 15] == xside && piece[k - 15] == KNIGHT)
		return 1;
	if (col > 1 && row > 0 && color[k - 10] == xside && piece[k - 10] == KNIGHT)
		return 1;
	if (col < 6 && row > 0 && color[k - 6] == xside && piece[k - 6] == KNIGHT)
		return 1;
	if (col > 1 && row < 7 && color[k + 6] == xside && piece[k + 6] == KNIGHT)
		return 1;
	if (col < 6 && row < 7 && color[k + 10] == xside && piece[k + 10] == KNIGHT)
		return 1;
	if (col > 0 && row < 6 && color[k + 15] == xside && piece[k + 15] == KNIGHT)
		return 1;
	if (col < 7 && row < 6 && color[k + 17] == xside && piece[k + 17] == KNIGHT)
		return 1;

	/* Check horizontal and vertical lines for attacking of Queen, Rook, King */
	/* go down */
	y = k + 8;
	if (y < 64)
	{
		if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
				|| piece[y] == ROOK))
			return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y += 8; y < 64; y += 8)
			{
				if (color[y] == xside
						&& (piece[y] == QUEEN || piece[y] == ROOK))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;
			}
	}
	/* go left */
	y = k - 1;
	h = k - col;
	if (y >= h)
	{
		if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
				|| piece[y] == ROOK))
			return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y--; y >= h; y--)
			{
				if (color[y] == xside
						&& (piece[y] == QUEEN || piece[y] == ROOK))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;
			}
	}
	/* go right */
	y = k + 1;
	h = k - col + 7;
	if (y <= h)
	{
		if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
				|| piece[y] == ROOK))
			return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y++; y <= h; y++)
			{
				if (color[y] == xside
						&& (piece[y] == QUEEN || piece[y] == ROOK))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;
			}
	}
	/* go up */
	y = k - 8;
	if (y >= 0)
	{
		if (color[y] == xside && (piece[y] == KING || piece[y] == QUEEN
				|| piece[y] == ROOK))
			return 1;
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y -= 8; y >= 0; y -= 8)
			{
				if (color[y] == xside
						&& (piece[y] == QUEEN || piece[y] == ROOK))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;
			}
	}
	/* Check diagonal lines for attacking of Queen, Bishop, King, Pawn */
	/* go right down */
	y = k + 9;
	if (y < 64 && COL(y) != 0)
	{
		if (color[y] == xside)
		{
			if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
				return 1;
			if (current_side == BLACK && piece[y] == PAWN)
				return 1;
		}
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y += 9; y < 64 && COL(y) != 0; y += 9)
			{
				if (color[y] == xside && (piece[y] == QUEEN || piece[y]
				                                                     == BISHOP))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;
			}
	}
	/* go left down */
	y = k + 7;
	if (y < 64 && COL(y) != 7)
	{
		if (color[y] == xside)
		{
			if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
				return 1;
			if (current_side == BLACK && piece[y] == PAWN)
				return 1;
		}
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y += 7; y < 64 && COL(y) != 7; y += 7)
			{
				if (color[y] == xside && (piece[y] == QUEEN || piece[y]
				                                                     == BISHOP))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;

			}
	}
	/* go left up */
	y = k - 9;
	if (y >= 0 && COL(y) != 7)
	{
		if (color[y] == xside)
		{
			if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
				return 1;
			if (current_side == WHITE && piece[y] == PAWN)
				return 1;
		}
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y -= 9; y >= 0 && COL(y) != 7; y -= 9)
			{
				if (color[y] == xside && (piece[y] == QUEEN || piece[y]
				                                                     == BISHOP))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;

			}
	}
	/* go right up */
	y = k - 7;
	if (y >= 0 && COL(y) != 0)
	{
		if (color[y] == xside)
		{
			if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
				return 1;
			if (current_side == WHITE && piece[y] == PAWN)
				return 1;
		}
        if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
			for (y -= 7; y >= 0 && COL(y) != 0; y -= 7)
			{
				if (color[y] == xside && (piece[y] == QUEEN || piece[y]
				                                                     == BISHOP))
					return 1;
                if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
					break;
			}
	}
	return 0;
}

int MakeMove(MOVE m)
{
	int r;
	int i;

	count_MakeMove ++;

    hist[hdp].m = m;
    hist[hdp].cap = piece[m.dest]; /* store in history the piece of the dest square */
    piece[m.dest] = piece[m.from]; /* dest piece is the one in the original square */
    color[m.dest] = color[m.from]; /* The dest square color is the one of the origin piece */
    piece[m.from] = EMPTY;/* The original square becomes empty */
    color[m.from] = EMPTY; /* The original color becomes empty */
	
	/* en pasant capture */
	if (m.type == MOVE_TYPE_EPS)
	{
		if (side == WHITE)
		{
			piece[m.dest + 8] = EMPTY;
			color[m.dest + 8] = EMPTY;
		}
		else
		{
			piece[m.dest - 8] = EMPTY;
			color[m.dest - 8] = EMPTY;
		}
	}


	/* Once the move is done we check either this is a promotion */
	if (m.type >= MOVE_TYPE_PROMOTION_TO_QUEEN)
	{
		/* In this case we put in the destiny sq the chosen piece */
		switch (m.type)
		{
		case MOVE_TYPE_PROMOTION_TO_QUEEN:
			piece[m.dest] = QUEEN;
			break;

		case MOVE_TYPE_PROMOTION_TO_ROOK:
			piece[m.dest] = ROOK;
			break;

		case MOVE_TYPE_PROMOTION_TO_BISHOP:
			piece[m.dest] = BISHOP;
			break;

		case MOVE_TYPE_PROMOTION_TO_KNIGHT:
			piece[m.dest] = KNIGHT;
			break;

		default:
			puts("Impossible to get here...");
			assert(false);
		}
	}

	/* Remove possible eps piece, remaining from former move */
	if (hist[hdp-1].m.type == MOVE_TYPE_PAWN_TWO)
	{
		for (i = 16; i <= 23; i++)
		{
			if (piece[i] == EPS_SQUARE)
			{
				piece[i] = EMPTY;
				/* this seems unnecesary, but otherwise a bug occurs:
				 * after: a3 Nc6 d4 e6, white isn't allowed to play e4 */
                color[i] = EMPTY;
				break;
			}
		}
		for (i = 40; i <= 47; i++)
		{
			if (piece[i] == EPS_SQUARE)
			{
				piece[i] = EMPTY;
				color[i] = EMPTY;
				break;
			}
		}
	}

	/* Add the eps square when a pawn moves two squares */
	if (m.type == MOVE_TYPE_PAWN_TWO)
	{
		if (side == BLACK)
		{
			piece[m.from + 8] = EPS_SQUARE;
            color[m.from + 8] = EMPTY;
		}
		else if (side == WHITE)
		{
			piece[m.from - 8] = EPS_SQUARE;
            color[m.from - 8] = EMPTY;
		}
	}

	if (m.type == MOVE_TYPE_CASTLE)
	{
		if (m.dest == G1)
		{
			/* h1-h8 becomes empty */
			piece[m.from + 3] = EMPTY;
			color[m.from + 3] = EMPTY;
			/* rook to f1-f8 */
			piece[m.from + 1] = ROOK;
			color[m.from + 1] = WHITE;
		}
		else if (m.dest == C1)
		{
			/* h1-h8 becomes empty */
			piece[m.from - 4] = EMPTY;
			color[m.from - 4] = EMPTY;
			/* rook to f1-f8 */
			piece[m.from - 1] = ROOK;
			color[m.from - 1] = WHITE;
		}
		else if (m.dest == G8)
		{
			/* h1-h8 becomes empty */
			piece[m.from + 3] = EMPTY;
			color[m.from + 3] = EMPTY;
			/* rook to f1-f8 */
			piece[m.from + 1] = ROOK;
			color[m.from + 1] = BLACK;
		}
		else if (m.dest == C8)
		{
			/* h1-h8 becomes empty */
			piece[m.from - 4] = EMPTY;
			color[m.from - 4] = EMPTY;
			/* rook to f1-f8 */
			piece[m.from - 1] = ROOK;
			color[m.from - 1] = BLACK;
		}
	}

	/* Update ply and hdp */
	ply++;
	hdp++;

    /* Update the castle rights */
	castle &= castle_mask[m.from] & castle_mask[m.dest];

    /* Checking if after making the move we're in check*/
	r = !IsInCheck(side);

	/* After making move, give turn to opponent */
	side = (WHITE + BLACK) - side;

	return r;
}

/* Undo what MakeMove did */
void TakeBack()
{

	int i;

	side = (WHITE + BLACK) - side;
	hdp--;
	ply--;
	piece[hist[hdp].m.from] = piece[hist[hdp].m.dest];
	piece[hist[hdp].m.dest] = hist[hdp].cap;
	color[hist[hdp].m.from] = side;

	/* Update castle rights */
	castle = hist[hdp].m.castle;

	/* Return the captured material */
	if (hist[hdp].cap != EMPTY)
	{
		color[hist[hdp].m.dest] = (WHITE + BLACK) - side;
	}
	else
	{
		color[hist[hdp].m.dest] = EMPTY;
	}

	/* Promotion */
	if (hist[hdp].m.type >= MOVE_TYPE_PROMOTION_TO_QUEEN)
	{
		piece[hist[hdp].m.from] = PAWN;
	}

    /* If pawn moved two squares in the former move, we have to restore
	 * the eps square */
	if (hist[hdp-1].m.type == MOVE_TYPE_PAWN_TWO)
	{
        if (side == WHITE)
		{
            piece[hist[hdp-1].m.dest - 8] = EPS_SQUARE;
//            color[hist[hdp-1].m.dest - 8] = EMPTY;
		}
        else if (side == BLACK)
		{
            piece[hist[hdp-1].m.dest + 8] = EPS_SQUARE;
//            color[hist[hdp-1].m.dest + 8] = EMPTY;
		}
	}

	/* To remove the eps square after unmaking a pawn
	 * moving two squares*/
	if (hist[hdp].m.type == MOVE_TYPE_PAWN_TWO)
	{
		if (side == WHITE)
		{
			piece[hist[hdp].m.from - 8] = EMPTY;
            color[hist[hdp].m.from - 8] = EMPTY;
		}
		else
		{
			piece[hist[hdp].m.from + 8] = EMPTY;
            color[hist[hdp].m.from + 8] = EMPTY;
		}
	}
	
	/* Unmaking an en pasant capture */
	if (hist[hdp].m.type == MOVE_TYPE_EPS)
	{
		if (side == WHITE)
		{
			/* The pawn */
			piece[hist[hdp].m.dest + 8] = PAWN;
			color[hist[hdp].m.dest + 8] = BLACK;
			/* The eps square */
			piece[hist[hdp].m.dest] = EPS_SQUARE;
//            color[hist[hdp].m.dest] = EMPTY;
		}
		else
		{
            /* The pawn */
			piece[hist[hdp].m.dest - 8] = PAWN;
			color[hist[hdp].m.dest - 8] = WHITE;
            /* The eps square */
            piece[hist[hdp].m.dest] = EPS_SQUARE;
//            color[hist[hdp].m.dest] = EMPTY;
		}
	}

    /* Undo Castle: return rook to its original square */
	if (hist[hdp].m.type == MOVE_TYPE_CASTLE)
	{
		/* Take the tower to its poriginal place */
        if (hist[hdp].m.dest == G1 && side == WHITE)
		{
			piece[H1] = ROOK;
			color[H1] = WHITE;
			piece[F1] = EMPTY;
			color[F1] = EMPTY;
		}
        else if (hist[hdp].m.dest == C1 && side == WHITE)
		{
			piece[A1] = ROOK;
			color[A1] = WHITE;
			piece[D1] = EMPTY;
			color[D1] = EMPTY;
		}
        else if (hist[hdp].m.dest == G8 && side == BLACK)
		{
			piece[H8] = ROOK;
			color[H8] = BLACK;
			piece[F8] = EMPTY;
			color[F8] = EMPTY;
		}
        else if (hist[hdp].m.dest == C8 && side == BLACK)
		{
			piece[A8] = ROOK;
			color[A8] = BLACK;
			piece[D8] = EMPTY;
			color[D8] = EMPTY;
		}
	}
}

/*
 ****************************************************************************
 * Search function - a typical alphabeta, main search function *
 * Lack: no move ordering *
 ****************************************************************************
 */

int Search(int alpha, int beta, int depth, MOVE * pBestMove)
{
	int i;
	int value; /* To store the evaluation */
	int havemove; /* Either we have or not a legal move available */
	int movecnt; /* The number of available moves */

	MOVE moveBuf[200]; /* List of movements */
	MOVE tmpMove;

	// nodes++; /* visiting a node, count it */
	havemove = 0; /* is there a move available? */
	pBestMove->type = MOVE_TYPE_NONE;

	/* Generate and count all moves for current position */
    movecnt = GenMoves(side, moveBuf);
	assert (movecnt < 201);

	/* Once we have all the moves available, we loop through the posible
	 * moves and apply an alpha-beta search */
	for (i = 0; i < movecnt; ++i)
	{

		if (!MakeMove(moveBuf[i]))
		{
			/* If the current move isn't legal, we take it back
			 * and take the next move in the list */
			TakeBack();
			continue;
		}

        /* If we've reached this far, then we have a move available */
		havemove = 1;

		/* This 'if' takes us to the deep of the position, the leaf nodes */
		if (depth - 1 > 0)
		{
			value = -Search(-beta, -alpha, depth - 1, &tmpMove);
		}
		/* If no depth left (leaf node), we evalute the position
           and apply the alpha-beta search.
           In the case of existing a quiescent function, it should be
           called here instead of Eval() */
		else
		{
			value = -Quiescent(-beta, -alpha);
			// value = -Eval();
		}

        /* We've evaluated the position, so we return to the previous position in such a way
           that when we take the next move from moveBuf everything is in order */
		TakeBack();

		/* Once we have an evaluation, we use it in in an alpha-beta search */
		if (value > alpha)
		{
			/* This move is so good and caused a cutoff */
			if (value >= beta)
			{
				return beta;
			}
			alpha = value;
			/* So far, current move is the best reaction for current position */
			*pBestMove = moveBuf[i];
		}
	}

    /* Once we've checked all the moves, if we have no legal moves,
	 * then that's checkmate or stalemate */
	if (!havemove)
	{
		if (IsInCheck(side))
			return -MATE + ply; /* add ply to find the longest path to lose or shortest path to win */
		else
			return 0;
	}

	/* Finally we return alpha, the score value */
	return alpha;
}

int Quiescent(int alpha, int beta)
{
	int i;
	int capscnt;
	int stand_pat;
	int score;
	MOVE cBuf[200];

    countquiesCalls++;
	nodes++;

	/* First we just try the evaluation function */
    stand_pat = Eval();
    if( stand_pat >= beta )
        return beta;
    if( alpha < stand_pat )
        alpha = stand_pat;

	/* If we haven't got a cut off we generate the captures and
	 * store them in cBuf */
	capscnt = GenCaps(side, cBuf);

    countCapCalls++;
	
	for (i = 0; i < capscnt; ++i)
	{
		if (!MakeMove(cBuf[i]))
		{
			/* If the current move isn't legal, we take it back
			 * and take the next move in the list */
			TakeBack();
			continue;
		}
		score = -Quiescent(-beta, -alpha);
		TakeBack();
		if( score >= beta )
            return beta;
        if( score > alpha )
           alpha = score;
	}
	return alpha;
}



MOVE ComputerThink(int depth)
{
	/* It returns the move the computer makes */
	MOVE m;
	int score;
	double knps;

	/* Reset some values before searching */
	ply = 0;
	nodes = 0;
	count_evaluations = 0;
	count_MakeMove = 0;
    countquiesCalls  = 0;


	clock_t start;
	clock_t stop;
	double t = 0.0;

	/* Start timer */
	start = clock();
	assert(start != -1);

	/* Search now */
	score = Search(-MATE, MATE, depth, &m);

	/* Stop timer */
	stop = clock();
	t = (double) (stop - start) / CLOCKS_PER_SEC;
	knps = (nodes / t)/1000.;

	double decimal_score = ((double)score)/100.;
	if (side == BLACK)
	{
		decimal_score= -decimal_score;
	}

	/* After searching, print results */
	printf(
            "Search result: move = %c%d%c%d; nodes = %d, countCapCalls = %d, evaluations = %d, moves made = %d, depth = %d, score = %.2f, time = %.2fs, knps = %.2f\n",
			'a' + COL(m.from), 8 - ROW(m.from), 'a' + COL(m.dest), 8
            - ROW(m.dest), nodes, countCapCalls, count_evaluations, count_MakeMove, depth, decimal_score, t, knps);
	return m;
}

/*
 ****************************************************************************
 * Utilities *
 ****************************************************************************
 */

void PrintBoard()
{
	char pieceName[] = "PNBRQKpnbrqk";
	int i;
	for (i = 0; i < 64; i++)
	{
		if ((i & 7) == 0)
		{
			printf("   +---+---+---+---+---+---+---+---+\n");
			if (i <= 56)
			{
				printf(" %d |", 8 - (((unsigned) i) >> 3));
			}
		}
		if (piece[i] == EMPTY)
			printf("   |");
		else if (piece[i] == EPS_SQUARE)
			printf(" * |");
		else
		{
			if (color[i] == WHITE)
				printf(" %c |", pieceName[piece[i]]);
			else
				printf("<%c>|", pieceName[piece[i] + 6]);
		}
		if ((i & 7) == 7)
			printf("\n");
	}
	printf(
			"   +---+---+---+---+---+---+---+---+\n     a   b   c   d   e   f   g   h\n");
}


/* Returns the number of posible positions to a given depth. Based on the
 perft function on Danasah */
unsigned long long perft(depth)
{
    int i;
    int movecnt; /* The number of available moves */
    unsigned long long nodes = 0;

    if (!depth) return 1;

    MOVE moveBuf[200]; /* List of movements */

    /* Generate and count all moves for current position */
    movecnt = GenMoves(side, moveBuf);

    /* Once we have all the moves available, we loop through the posible
     * moves and apply an alpha-beta search */
    for (i = 0; i < movecnt; ++i)
    {
        if (!MakeMove(moveBuf[i]))
        {
            TakeBack();
            continue;
        }

//        if (IsInCheck(side))
//        {
//            count_checks++;
//        }

        /* This 'if' takes us to the deep of the position */
        nodes += perft(depth - 1);
        TakeBack();
    }

    return nodes;
}


/*
 ****************************************************************************
 * Main program *
 ****************************************************************************
 */

void startgame()
{
	int i;
	for (i = 0; i < 64; ++i) {
		piece[i] = piece[i];
		color[i] = color[i];
	}

	side = WHITE;
	computer_side = BLACK; /* Human is white side */
	hdp = 0;
}

void xboard()
{
	char line[256], command[256], c;
	int from, dest, i;
	MOVE moveBuf[200], bestMove;
	int movecnt;

	printf("\n");

	startgame();

	for (;;)
	{
		fflush(stdout);
		if (side == computer_side)
		{ /* computer's turn */
			/* Find out the best move to react the current position */
			bestMove = ComputerThink(max_depth);
			MakeMove(bestMove);
			/* send move */
			switch (bestMove.type)
			{
			case MOVE_TYPE_PROMOTION_TO_QUEEN:
				c = 'q';
				break;
			case MOVE_TYPE_PROMOTION_TO_ROOK:
				c = 'r';
				break;
			case MOVE_TYPE_PROMOTION_TO_BISHOP:
				c = 'b';
				break;
			case MOVE_TYPE_PROMOTION_TO_KNIGHT:
				c = 'n';
				break;
			default:
				c = ' ';
				printf("move %c%d%c%d%c\n", 'a' + COL(bestMove.from), 8
						- ROW(bestMove.from), 'a' + COL(bestMove.dest), 8
						- ROW(bestMove.dest), c);
			}
			continue;
		}

		if (!fgets(line, 256, stdin))
			return;
		if (line[0] == '\n')
			continue;
		sscanf(line, "%s", command);
		if (!strcmp(command, "xboard"))
		{
			continue;
		}
		if (!strcmp(command, "new"))
		{
			startgame();
			continue;
		}
		if (!strcmp(command, "quit"))
		{
			return;
		}
		if (!strcmp(command, "force"))
		{
			computer_side = EMPTY;
			continue;
		}
		if (!strcmp(command, "white"))
		{
			side = WHITE;
			computer_side = BLACK;
			continue;
		}
		if (!strcmp(command, "black"))
		{
			side = BLACK;
			computer_side = WHITE;
			continue;
		}
		if (!strcmp(command, "sd"))
		{
			sscanf(line, "sd %d", &max_depth);
			continue;
        }
		if (!strcmp(command, "go"))
		{
			computer_side = side;
			continue;
		}
		if (!strcmp(command, "undo"))
		{
			if (hdp == 0)
				continue;
			TakeBack();
			continue;
		}
		if (!strcmp(command, "remove"))
		{
			if (hdp <= 1)
				continue;
			TakeBack();
			TakeBack();
			continue;
		}

		/* maybe the user entered a move? */
		from = command[0] - 'a';
		from += 8 * (8 - (command[1] - '0'));
		dest = command[2] - 'a';
		dest += 8 * (8 - (command[3] - '0'));
		ply = 0;
        movecnt = GenMoves(side, moveBuf);
		/* loop through the moves to see if it's legal */
		for (i = 0; i < movecnt; i++)
			if (moveBuf[i].from == from && moveBuf[i].dest == dest)
			{
				if (piece[from] == PAWN && (dest < 8 || dest > 55))
				{ /* Promotion move? */
					switch (command[4])
					{
					case 'q':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
						break;
					case 'r':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_ROOK;
						break;
					case 'b':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_BISHOP;
						break;
					case 'n':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_KNIGHT;
						break;
					default:
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
					}
				}
				if (!MakeMove(moveBuf[i]))
				{
					TakeBack();
					printf("Illegal move.\n");
				}
				break;

			}
	}
}

int main()
{

	/* It mainly calls ComputerThink(maxdepth) to the desired ply */

	char s[256];
	int from;
	int dest;
	int i;
	//int computer_side;

	startgame();

	max_depth = 5; /* max depth to search */
	MOVE moveBuf[200];
	int movecnt;

	puts("Second Chess, by Emilio Diaz");
	puts(" Help");
	puts(" d: display board");
	puts(" MOVE: make a move (e.g. b1c3, a7a8q, e1g1)");
	puts(" on: force computer to move");
	puts(" quit: exit");
	puts(" sd n: set engine depth to n plies");
	puts(" undo: take back last move");

	side = WHITE;
	computer_side = BLACK; /* Human is white side */

	hdp = 0; /* Current move order */
	for (;;)
	{
		if (side == computer_side)
		{ /* Computer's turn */
			/* Find out the best move to react the current position */
			MOVE bestMove = ComputerThink(max_depth);
			MakeMove(bestMove);
			PrintBoard();
			printf("CASTLE: %d\n", castle);
			continue;
		}

		/* Get user input */
		printf("sc> ");
		if (scanf("%s", s) == EOF) /* close program */
			return 0;
		if (!strcmp(s, "d"))
		{
			PrintBoard();
			continue;
		}
		if (!strcmp(s, "undo"))
		{
			TakeBack();
			PrintBoard();
			computer_side = (WHITE + BLACK) - computer_side;
			continue;
		}
		if (!strcmp(s, "xboard"))
		{
			xboard();
			return 0;
		}
		if (!strcmp(s, "on"))
		{
			computer_side = side;
			continue;
		}
        if (!strcmp(s, "pass"))
        {
            side = (WHITE + BLACK) - side;
            continue;
        }
		if (!strcmp(s, "sd"))
		{
			scanf("%d", &max_depth);
			continue;
		}
		if (!strcmp(s, "perft"))
        {
            scanf("%d", &max_depth);
            clock_t start;
            clock_t stop;
            double t = 0.0;
            /* Start timer */
            start = clock();
            unsigned long long count = perft(max_depth);
            /* Stop timer */
            stop = clock();
            t = (double) (stop - start) / CLOCKS_PER_SEC;
            printf("nodes = %llu\n", count);
            printf("time = %f\n", t);
			continue;
		}
		if (!strcmp(s, "quit"))
		{
			printf("Good bye!\n");
			return 0;
		}

		/* Maybe the user entered a move? */
		from = s[0] - 'a';
		from += 8 * (8 - (s[1] - '0'));
		dest = s[2] - 'a';
		dest += 8 * (8 - (s[3] - '0'));
		ply = 0;
        movecnt = GenMoves(side, moveBuf);

		/* Loop through the moves to see if it's legal */
		for (i = 0; i < movecnt; i++)
			if (moveBuf[i].from == from && moveBuf[i].dest == dest)
			{
				/* Promotion move? */
				if (piece[from] == PAWN && (dest < 8 || dest > 55))
				{
					switch (s[4])
					{
					case 'q':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
						break;

					case 'r':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_ROOK;
						break;

					case 'b':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_BISHOP;
						break;

					case 'n':
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_KNIGHT;
						break;

					default:
						puts(
								"promoting to a McGuffin..., I'll give you a queen");
						moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
					}
				}
				if (!MakeMove(moveBuf[i]))
				{
					TakeBack();
					printf("Illegal move.\n");
				}
				break;
			}
		PrintBoard();
	}
}


