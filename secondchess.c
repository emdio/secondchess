/*
secondchess - gpl, by Emilio D�az, based on firstchess by Pham Hong Nguyen

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
#include <locale.h>


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

#define MATE 10000		/* equal value of King, losing King==mate */

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
#define A2 48
#define A3 40
#define A4 32
#define A5 24
#define A6 16
#define A7 8
#define A8 0
#define B1 57
#define B2 49
#define B3 41
#define B4 33
#define B5 25
#define B6 17
#define B7 9
#define B8 1
#define C1 58
#define C2 50
#define C3 42
#define C4 34
#define C5 26
#define C6 18
#define C7 10
#define C8 2
#define D1 59
#define D2 51
#define D3 43
#define D4 35
#define D5 27
#define D6 19
#define D7 11
#define D8 3
#define E1 60
#define E2 52
#define E3 44
#define E4 36
#define E5 28
#define E6 20
#define E7 12
#define E8 4
#define F1 61
#define F2 53
#define F3 45
#define F4 37
#define F5 29
#define F6 21
#define F7 13
#define F8 5
#define G1 62
#define G2 54
#define G3 46
#define G4 38
#define G5 30
#define G6 22
#define G7 14
#define G8 6
#define H1 63
#define H2 55
#define H3 47
#define H4 39
#define H5 31
#define H6 23
#define H7 15
#define H8 7

/*
 ****************************************************************************
 * Board representation and main variants *
 ****************************************************************************
 */
/* Board representation */
int piece[64];
int color[64];

/* Piece in each square */
int init_piece[64] = {
  ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
  PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
  ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
};

/* Color of each square */
int init_color[64] = {
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
  WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE
};

//int piece[64] = {
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, ROOK, PAWN, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, PAWN, KING, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, PAWN, EMPTY, EMPTY, PAWN, PAWN,
//        EMPTY, EMPTY, EMPTY, EMPTY, KING, EMPTY, EMPTY, ROOK };
///* Color of each square */
//int color[64] = {
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY,  EMPTY, EMPTY, EMPTY, BLACK, WHITE, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, BLACK, BLACK, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
//        EMPTY, EMPTY, EMPTY, WHITE, EMPTY, EMPTY, WHITE, WHITE,
//        EMPTY, EMPTY, EMPTY, EMPTY, WHITE, EMPTY, EMPTY, WHITE };


int side;			/* Side to move, value = BLACK or WHITE */
int computer_side;
int max_depth;			/* max depth to search */

/* A move is defined by its origin and final squares, the castle rights and the kind of
 * move it's: normal, enpasant... */
typedef struct tag_MOVE
{
  int from;
  int dest;
//      int castle;
  int type;
} MOVE;

/* For storing all moves of game */
typedef struct tag_HIST
{
  MOVE m;
  int castle;
  int cap;
} HIST;

HIST hist[6000];		/* Game length < 6000 */

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
int castle_rights = 15;		/* At start position all castle types ar available */


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
  7, 15, 15, 15, 3, 15, 15, 11,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  15, 15, 15, 15, 15, 15, 15, 15,
  13, 15, 15, 15, 12, 15, 15, 14
};

int hdp;			/* Current move order */
//int allmoves = 0;

/* For searching */
int nodes;			/* Count all visited nodes when searching */
int ply;			/* ply of search */
int count_evaluations;
int count_checks;
int count_MakeMove;
int count_quies_calls;
int count_cap_calls;

/* The values of the pieces in centipawns */
int value_piece[6] =
  { VALUE_PAWN, VALUE_KNIGHT, VALUE_BISHOP, VALUE_ROOK, VALUE_QUEEN,
VALUE_KING };

/* * * * * * * * * * * * *
 * Piece Square Tables
 * * * * * * * * * * * * */
/* When evaluating the position we'll add a bonus (or malus) to each piece
 * depending on the very square where it's placed. Vg, a knight in d4 will
 * be given an extra +15, whilst a knight in a1 will be penalized with -40.
 * This simple idea allows the engine to make more sensible moves */
int pst_pawn[64] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 15, 15, 0, 0, 0,
  0, 0, 0, 10, 10, 0, 0, 0,
  0, 0, 0, 5, 5, 0, 0, 0,
  0, 0, 0, -25, -25, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

int pst_knight[64] = {
  -40, -25, -25, -25, -25, -25, -25, -40,
  -30, 0, 0, 0, 0, 0, 0, -30,
  -30, 0, 0, 0, 0, 0, 0, -30,
  -30, 0, 0, 15, 15, 0, 0, -30,
  -30, 0, 0, 15, 15, 0, 0, -30,
  -30, 0, 10, 0, 0, 10, 0, -30,
  -30, 0, 0, 5, 5, 0, 0, -30,
  -40, -30, -25, -25, -25, -25, -30, -40
};

int pst_bishop[64] = {
  -10, 0, 0, 0, 0, 0, 0, -10,
  -10, 5, 0, 0, 0, 0, 5, -10,
  -10, 0, 5, 0, 0, 5, 0, -10,
  -10, 0, 0, 10, 10, 0, 0, -10,
  -10, 0, 5, 10, 10, 5, 0, -10,
  -10, 0, 5, 0, 0, 5, 0, -10,
  -10, 5, 0, 0, 0, 0, 5, -10,
  -10, -20, -20, -20, -20, -20, -20, -10
};

int pst_rook[64] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  10, 10, 10, 10, 10, 10, 10, 10,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 5, 5, 0, 0, 0
};

int pst_king[64] = {
  -25, -25, -25, -25, -25, -25, -25, -25,
  -25, -25, -25, -25, -25, -25, -25, -25,
  -25, -25, -25, -25, -25, -25, -25, -25,
  -25, -25, -25, -25, -25, -25, -25, -25,
  -25, -25, -25, -25, -25, -25, -25, -25,
  -25, -25, -25, -25, -25, -25, -25, -25,
  -25, -25, -25, -25, -25, -25, -25, -25,
  10, 15, -15, -15, -15, -15, 15, 10
};

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
  8, 9, 10, 11, 12, 13, 14, 15,
  0, 1, 2, 3, 4, 5, 6, 7
};

/*
 ****************************************************************************
 * Move generator *
 ****************************************************************************
 */
void
Gen_Push (int from, int dest, int type, MOVE * pBuf, int *pMCount)
{
  MOVE move;
  move.from = from;
  move.dest = dest;
  move.type = type;
//      move.castle = castle;
  pBuf[*pMCount] = move;
  *pMCount = *pMCount + 1;
}

void
Gen_PushNormal (int from, int dest, MOVE * pBuf, int *pMCount)
{
  Gen_Push (from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
}

/* Especial cases for Pawn */

/* Pawn can promote */
void
Gen_PushPawn (int from, int dest, MOVE * pBuf, int *pMCount)
{
  /* The 7 and 56 are to limit pawns to the 2nd through 7th ranks, which
   * means this isn't a promotion, i.e., a normal pawn move */
  if (piece[dest] == EPS_SQUARE)
    {
      Gen_Push (from, dest, MOVE_TYPE_EPS, pBuf, pMCount);
    }
  else if (dest > 7 && dest < 56)	/* this is just a normal move */
    {
      Gen_Push (from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
    }
  else				/* otherwise it's a promotion */
    {
      Gen_Push (from, dest, MOVE_TYPE_PROMOTION_TO_QUEEN, pBuf, pMCount);
      Gen_Push (from, dest, MOVE_TYPE_PROMOTION_TO_ROOK, pBuf, pMCount);
      Gen_Push (from, dest, MOVE_TYPE_PROMOTION_TO_BISHOP, pBuf, pMCount);
      Gen_Push (from, dest, MOVE_TYPE_PROMOTION_TO_KNIGHT, pBuf, pMCount);
    }
}

/* When a pawn moves two squares then appears the possibility of the en passanta capture*/
void
Gen_PushPawnTwo (int from, int dest, MOVE * pBuf, int *pMCount)
{
  Gen_Push (from, dest, MOVE_TYPE_PAWN_TWO, pBuf, pMCount);
}

/* Especial cases for King */
void
Gen_PushKing (int from, int dest, MOVE * pBuf, int *pMCount)
{
  /* Is it a castle? */
  if (from == E1 && (dest == G1 || dest == C1))	/* this is a white castle */
    {
      Gen_Push (from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
    }
  else if (from == E8 && (dest == G8 || dest == C8))	/* this is a black castle */
    {
      Gen_Push (from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
    }
  else				/* otherwise it's a normal king's move */
    {
      Gen_Push (from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
    }
}

/* Gen all moves of current_side to move and push them to pBuf, and return number of moves */
int
GenMoves (int current_side, MOVE * pBuf)
{
  int i;			/* Counter for the board squares */
  int k;			/* Counter for cols */
  int y;
  int row;
  int col;
  int movecount;
  movecount = 0;

  for (i = 0; i < 64; i++)	/* Scan all board */
    if (color[i] == current_side)
      {
	switch (piece[i])
	  {

	  case PAWN:
	    col = COL (i);
	    row = ROW (i);
	    if (current_side == BLACK)
	      {
		if (color[i + 8] == EMPTY)
		  /* Pawn advances one square.
		   * We use Gen_PushPawn because it can be a promotion */
		  Gen_PushPawn (i, i + 8, pBuf, &movecount);
		if (row == 1 && color[i + 8] == EMPTY
		    && color[i + 16] == EMPTY)
		  /* Pawn advances two squares */
		  Gen_PushPawnTwo (i, i + 16, pBuf, &movecount);
		if (col && color[i + 7] == WHITE)
		  /* Pawn captures and it can be a promotion */
		  Gen_PushPawn (i, i + 7, pBuf, &movecount);
		if (col < 7 && color[i + 9] == WHITE)
		  /* Pawn captures and can be a promotion */
		  Gen_PushPawn (i, i + 9, pBuf, &movecount);
		/* For en passant capture */
		if (col && piece[i + 7] == EPS_SQUARE)
		  /* Pawn captures and it can be a promotion */
		  Gen_PushPawn (i, i + 7, pBuf, &movecount);
		if (col < 7 && piece[i + 9] == EPS_SQUARE)
		  /* Pawn captures and can be a promotion */
		  Gen_PushPawn (i, i + 9, pBuf, &movecount);
	      }
	    else
	      {
		if (color[i - 8] == EMPTY)
		  Gen_PushPawn (i, i - 8, pBuf, &movecount);
		/* Pawn moves 2 squares */
		if (row == 6 && color[i - 8] == EMPTY
		    && color[i - 16] == EMPTY)
		  Gen_PushPawnTwo (i, i - 16, pBuf, &movecount);
		/* For captures */
		if (col && color[i - 9] == BLACK)
		  Gen_PushPawn (i, i - 9, pBuf, &movecount);
		if (col < 7 && color[i - 7] == BLACK)
		  Gen_PushPawn (i, i - 7, pBuf, &movecount);
		/* For en passant capture */
		if (col && piece[i - 9] == EPS_SQUARE)
		  Gen_PushPawn (i, i - 9, pBuf, &movecount);
		if (col < 7 && piece[i - 7] == EPS_SQUARE)
		  Gen_PushPawn (i, i - 7, pBuf, &movecount);
	      }
	    break;

	  case QUEEN:		/* == BISHOP+ROOK */

	  case BISHOP:
	    for (y = i - 9; y >= 0 && COL (y) != 7; y -= 9)
	      {			/* go left up */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    for (y = i - 7; y >= 0 && COL (y) != 0; y -= 7)
	      {			/* go right up */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    for (y = i + 9; y < 64 && COL (y) != 0; y += 9)
	      {			/* go right down */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    for (y = i + 7; y < 64 && COL (y) != 7; y += 7)
	      {			/* go left down */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    if (piece[i] == BISHOP)	/* In the case of the bishop we're done */
	      break;

	    /* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
	  case ROOK:
	    col = COL (i);
	    for (k = i - col, y = i - 1; y >= k; y--)
	      {			/* go left */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    for (k = i - col + 7, y = i + 1; y <= k; y++)
	      {			/* go right */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    for (y = i - 8; y >= 0; y -= 8)
	      {			/* go up */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    for (y = i + 8; y < 64; y += 8)
	      {			/* go down */
		if (color[y] != current_side)
		  Gen_PushNormal (i, y, pBuf, &movecount);
		if (color[y] != EMPTY)
		  break;
	      }
	    break;

	  case KNIGHT:
              switch(i)
              {
                  case A8:
                      if (color[B6] != current_side)
                      Gen_PushNormal(A8, B6, pBuf, &movecount);
                      if (color[C7] != current_side)
                      Gen_PushNormal(A8, C7, pBuf, &movecount);
                      break;
                  case B8:
                      if (color[A6] != current_side)
                      Gen_PushNormal(B8, A6, pBuf, &movecount);
                      if (color[C6] != current_side)
                      Gen_PushNormal(B8, C6, pBuf, &movecount);
                      if (color[D7] != current_side)
                      Gen_PushNormal(B8, D7, pBuf, &movecount);
                      break;
                  case C8:
                      if (color[A7] != current_side)
                      Gen_PushNormal(C8, A7, pBuf, &movecount);
                      if (color[B6] != current_side)
                      Gen_PushNormal(C8, B6, pBuf, &movecount);
                      if (color[D6] != current_side)
                      Gen_PushNormal(C8, D6, pBuf, &movecount);
                      if (color[E7] != current_side)
                      Gen_PushNormal(C8, E7, pBuf, &movecount);
                      break;
                  case D8:
                      if (color[B7] != current_side)
                      Gen_PushNormal(D8, B7, pBuf, &movecount);
                      if (color[C6] != current_side)
                      Gen_PushNormal(D8, C6, pBuf, &movecount);
                      if (color[E6] != current_side)
                      Gen_PushNormal(D8, E6, pBuf, &movecount);
                      if (color[F7] != current_side)
                      Gen_PushNormal(D8, F7, pBuf, &movecount);
                      break;
                  case E8:
                      if (color[C7] != current_side)
                      Gen_PushNormal(E8, C7, pBuf, &movecount);
                      if (color[D6] != current_side)
                      Gen_PushNormal(E8, D6, pBuf, &movecount);
                      if (color[F6] != current_side)
                      Gen_PushNormal(E8, F6, pBuf, &movecount);
                      if (color[G7] != current_side)
                      Gen_PushNormal(E8, G7, pBuf, &movecount);
                      break;
                  case F8:
                      if (color[D7] != current_side)
                      Gen_PushNormal(F8, D7, pBuf, &movecount);
                      if (color[E6] != current_side)
                      Gen_PushNormal(F8, E6, pBuf, &movecount);
                      if (color[G6] != current_side)
                      Gen_PushNormal(F8, G6, pBuf, &movecount);
                      if (color[H7] != current_side)
                      Gen_PushNormal(F8, H7, pBuf, &movecount);
                      break;
                  case G8:
                      if (color[E7] != current_side)
                      Gen_PushNormal(G8, E7, pBuf, &movecount);
                      if (color[F6] != current_side)
                      Gen_PushNormal(G8, F6, pBuf, &movecount);
                      if (color[H6] != current_side)
                      Gen_PushNormal(G8, H6, pBuf, &movecount);
                      break;
                  case H8:
                      if (color[F7] != current_side)
                      Gen_PushNormal(G8, F7, pBuf, &movecount);
                      if (color[G6] != current_side)
                      Gen_PushNormal(G8, G6, pBuf, &movecount);
                      break;
                  case A7:
                      if (color[C8] != current_side)
                      Gen_PushNormal(A7, C8, pBuf, &movecount);
                      if (color[C6] != current_side)
                      Gen_PushNormal(A7, C6, pBuf, &movecount);
                      if (color[B5] != current_side)
                      Gen_PushNormal(A7, B5, pBuf, &movecount);
                      break;
                  case B7:
                      if (color[D8] != current_side)
                      Gen_PushNormal(B7, D8, pBuf, &movecount);
                      if (color[D6] != current_side)
                      Gen_PushNormal(B7, D6, pBuf, &movecount);
                      if (color[A5] != current_side)
                      Gen_PushNormal(B7, A5, pBuf, &movecount);
                      if (color[C5] != current_side)
                      Gen_PushNormal(B7, C5, pBuf, &movecount);
                      break;
                  case C7:
                      if (color[A8] != current_side)
                      Gen_PushNormal(C7, A8, pBuf, &movecount);
                      if (color[A6] != current_side)
                      Gen_PushNormal(C7, A6, pBuf, &movecount);
                      if (color[B5] != current_side)
                      Gen_PushNormal(C7, B5, pBuf, &movecount);
                      if (color[D5] != current_side)
                      Gen_PushNormal(C7, D5, pBuf, &movecount);
                      if (color[E6] != current_side)
                      Gen_PushNormal(C7, E6, pBuf, &movecount);
                      if (color[E8] != current_side)
                      Gen_PushNormal(C7, E8, pBuf, &movecount);
                      break;
                  case D7:
                      if (color[B8] != current_side)
                      Gen_PushNormal(D7, B8, pBuf, &movecount);
                      if (color[B6] != current_side)
                      Gen_PushNormal(D7, B6, pBuf, &movecount);
                      if (color[C5] != current_side)
                      Gen_PushNormal(D7, C5, pBuf, &movecount);
                      if (color[E5] != current_side)
                      Gen_PushNormal(D7, E5, pBuf, &movecount);
                      if (color[F6] != current_side)
                      Gen_PushNormal(D7, F6, pBuf, &movecount);
                      if (color[F8] != current_side)
                      Gen_PushNormal(D7, F8, pBuf, &movecount);
                      break;
                  case E7:
                      if (color[C8] != current_side)
                      Gen_PushNormal(E7, C8, pBuf, &movecount);
                      if (color[C6] != current_side)
                      Gen_PushNormal(E7, C6, pBuf, &movecount);
                      if (color[D5] != current_side)
                      Gen_PushNormal(E7, D5, pBuf, &movecount);
                      if (color[F5] != current_side)
                      Gen_PushNormal(E7, F5, pBuf, &movecount);
                      if (color[G6] != current_side)
                      Gen_PushNormal(E7, G6, pBuf, &movecount);
                      if (color[G8] != current_side)
                      Gen_PushNormal(E7, G8, pBuf, &movecount);
                      break;
                  case F7:
                      if (color[D8] != current_side)
                      Gen_PushNormal(F7, D8, pBuf, &movecount);
                      if (color[D6] != current_side)
                      Gen_PushNormal(F7, D6, pBuf, &movecount);
                      if (color[E5] != current_side)
                      Gen_PushNormal(F7, E5, pBuf, &movecount);
                      if (color[G5] != current_side)
                      Gen_PushNormal(F7, G5, pBuf, &movecount);
                      if (color[H6] != current_side)
                      Gen_PushNormal(F7, H6, pBuf, &movecount);
                      if (color[H8] != current_side)
                      Gen_PushNormal(F7, H8, pBuf, &movecount);
                      break;
                  case G7:
                      if (color[E8] != current_side)
                      Gen_PushNormal(G7, E8, pBuf, &movecount);
                      if (color[E6] != current_side)
                      Gen_PushNormal(G7, E6, pBuf, &movecount);
                      if (color[F5] != current_side)
                      Gen_PushNormal(G7, F5, pBuf, &movecount);
                      if (color[H5] != current_side)
                      Gen_PushNormal(G7, H5, pBuf, &movecount);
                      break;
                  case H7:
                      Gen_PushNormal(H7, F8, pBuf, &movecount);
                      Gen_PushNormal(H7, F6, pBuf, &movecount);
                      Gen_PushNormal(H7, G5, pBuf, &movecount);
                      break;
                  case A6:
                      Gen_PushNormal(A6, B8, pBuf, &movecount);
                      Gen_PushNormal(A6, B4, pBuf, &movecount);
                      Gen_PushNormal(A6, C7, pBuf, &movecount);
                      Gen_PushNormal(A6, C5, pBuf, &movecount);
                      break;
                  case B6:
                      Gen_PushNormal(B6, A8, pBuf, &movecount);
                      Gen_PushNormal(B6, A4, pBuf, &movecount);
                      Gen_PushNormal(B6, C8, pBuf, &movecount);
                      Gen_PushNormal(B6, C4, pBuf, &movecount);
                      Gen_PushNormal(B6, D7, pBuf, &movecount);
                      Gen_PushNormal(B6, D5,pBuf, &movecount);
                      break;
                  case C6:
                      Gen_PushNormal(C6, A7, pBuf, &movecount);
                      Gen_PushNormal(C6, A5, pBuf, &movecount);
                      Gen_PushNormal(C6, B8, pBuf, &movecount);
                      Gen_PushNormal(C6, B4, pBuf, &movecount);
                      Gen_PushNormal(C6, D8, pBuf, &movecount);
                      Gen_PushNormal(C6, D4, pBuf, &movecount);
                      Gen_PushNormal(C6, E7, pBuf, &movecount);
                      Gen_PushNormal(C6, E5, pBuf, &movecount);
                      break;
                  case D6:
                      Gen_PushNormal(D6, B7, pBuf, &movecount);
                      Gen_PushNormal(D6, B5, pBuf, &movecount);
                      Gen_PushNormal(D6, C8, pBuf, &movecount);
                      Gen_PushNormal(D6, C4, pBuf, &movecount);
                      Gen_PushNormal(D6, D7, pBuf, &movecount);
                      Gen_PushNormal(D6, D5, pBuf, &movecount);
                      Gen_PushNormal(D6, F7, pBuf, &movecount);
                      Gen_PushNormal(D6, F5, pBuf, &movecount);
                      break;
                  case E6:
                      Gen_PushNormal(E6, C7, pBuf, &movecount);
                      Gen_PushNormal(E6, C5, pBuf, &movecount);
                      Gen_PushNormal(E6, D8, pBuf, &movecount);
                      Gen_PushNormal(E6, D4, pBuf, &movecount);
                      Gen_PushNormal(E6, F7, pBuf, &movecount);
                      Gen_PushNormal(E6, F4, pBuf, &movecount);
                      Gen_PushNormal(E6, G7, pBuf, &movecount);
                      Gen_PushNormal(E6, G5, pBuf, &movecount);
                      break;
                  case F6:
                      Gen_PushNormal(F6, D7, pBuf, &movecount);
                      Gen_PushNormal(F6, D5, pBuf, &movecount);
                      Gen_PushNormal(F6, E8, pBuf, &movecount);
                      Gen_PushNormal(F6, E4, pBuf, &movecount);
                      Gen_PushNormal(F6, G8, pBuf, &movecount);
                      Gen_PushNormal(F6, G4, pBuf, &movecount);
                      Gen_PushNormal(F6, H7, pBuf, &movecount);
                      Gen_PushNormal(F6, H5, pBuf, &movecount);
                      break;
                  case G6:
                      Gen_PushNormal(G6, E7, pBuf, &movecount);
                      Gen_PushNormal(G6, E5, pBuf, &movecount);
                      Gen_PushNormal(G6, F8, pBuf, &movecount);
                      Gen_PushNormal(G6, F4, pBuf, &movecount);
                      Gen_PushNormal(G6, H4, pBuf, &movecount);
                      Gen_PushNormal(G6, H8, pBuf, &movecount);
                      break;
                  case H6:
                      Gen_PushNormal(H6, F7, pBuf, &movecount);
                      Gen_PushNormal(H6, F5, pBuf, &movecount);
                      Gen_PushNormal(H6, G8, pBuf, &movecount);
                      Gen_PushNormal(H6, G4, pBuf, &movecount);
                      break;
                  case A5:
                      Gen_PushNormal(A5, B7, pBuf, &movecount);
                      Gen_PushNormal(A5, B3, pBuf, &movecount);
                      Gen_PushNormal(A5, C4, pBuf, &movecount);
                      Gen_PushNormal(A5, C6, pBuf, &movecount);
                      break;
                  case B5:
                      Gen_PushNormal(B5, A7, pBuf, &movecount);
                      Gen_PushNormal(B5, A3, pBuf, &movecount);
                      Gen_PushNormal(B5, C7, pBuf, &movecount);
                      Gen_PushNormal(B5, C3, pBuf, &movecount);
                      Gen_PushNormal(B5, D6, pBuf, &movecount);
                      Gen_PushNormal(B5, D4, pBuf, &movecount);
                      break;
                  case C5:
                      Gen_PushNormal(C5, A6, pBuf, &movecount);
                      Gen_PushNormal(C5, A4, pBuf, &movecount);
                      Gen_PushNormal(C5, B7, pBuf, &movecount);
                      Gen_PushNormal(C5, B3, pBuf, &movecount);
                      Gen_PushNormal(C5, D7, pBuf, &movecount);
                      Gen_PushNormal(C5, D3, pBuf, &movecount);
                      Gen_PushNormal(C5, E6, pBuf, &movecount);
                      Gen_PushNormal(C5, E4, pBuf, &movecount);
                      break;
                  case D5:
                      Gen_PushNormal(D5, B6, pBuf, &movecount);
                      Gen_PushNormal(D5, B4, pBuf, &movecount);
                      Gen_PushNormal(D5, C7, pBuf, &movecount);
                      Gen_PushNormal(D5, C3, pBuf, &movecount);
                      Gen_PushNormal(D5, E7, pBuf, &movecount);
                      Gen_PushNormal(D5, E3, pBuf, &movecount);
                      Gen_PushNormal(D5, F6, pBuf, &movecount);
                      Gen_PushNormal(D5, F4, pBuf, &movecount);
                      break;
                  case E5:
                      Gen_PushNormal(E5, C6, pBuf, &movecount);
                      Gen_PushNormal(E5, C4, pBuf, &movecount);
                      Gen_PushNormal(E5, D7, pBuf, &movecount);
                      Gen_PushNormal(E5, D3, pBuf, &movecount);
                      Gen_PushNormal(E5, F7, pBuf, &movecount);
                      Gen_PushNormal(E5, F3, pBuf, &movecount);
                      Gen_PushNormal(E5, G6, pBuf, &movecount);
                      Gen_PushNormal(E5, G4, pBuf, &movecount);
                      break;
                  case F5:
                      Gen_PushNormal(F5, D6, pBuf, &movecount);
                      Gen_PushNormal(F5, D4, pBuf, &movecount);
                      Gen_PushNormal(F5, E7, pBuf, &movecount);
                      Gen_PushNormal(F5, E3, pBuf, &movecount);
                      Gen_PushNormal(F5, G7, pBuf, &movecount);
                      Gen_PushNormal(F5, G3, pBuf, &movecount);
                      Gen_PushNormal(F5, H6, pBuf, &movecount);
                      Gen_PushNormal(F5, H4, pBuf, &movecount);
                      break;
                  case G5:
                      Gen_PushNormal(G5, E6, pBuf, &movecount);
                      Gen_PushNormal(G5, E4, pBuf, &movecount);
                      Gen_PushNormal(G5, F7, pBuf, &movecount);
                      Gen_PushNormal(G5, F3, pBuf, &movecount);
                      Gen_PushNormal(G5, H7, pBuf, &movecount);
                      Gen_PushNormal(G5, H3, pBuf, &movecount);
                      break;
                  case H5:
                      Gen_PushNormal(H5, F6, pBuf, &movecount);
                      Gen_PushNormal(H5, F4, pBuf, &movecount);
                      Gen_PushNormal(H5, G7, pBuf, &movecount);
                      Gen_PushNormal(H5, G3, pBuf, &movecount);
                      break;
                  case A4:
                      Gen_PushNormal(A4, B6, pBuf, &movecount);
                      Gen_PushNormal(A4, B2, pBuf, &movecount);
                      Gen_PushNormal(A4, C5, pBuf, &movecount);
                      Gen_PushNormal(A4, C3, pBuf, &movecount);
                      break;
                  case B4:
                      Gen_PushNormal(B4, A6, pBuf, &movecount);
                      Gen_PushNormal(B4, A2, pBuf, &movecount);
                      Gen_PushNormal(B4, C6, pBuf, &movecount);
                      Gen_PushNormal(B4, C2, pBuf, &movecount);
                      Gen_PushNormal(B4, D5, pBuf, &movecount);
                      Gen_PushNormal(B4, D3, pBuf, &movecount);
                      break;
                  case C4:
                      Gen_PushNormal(C4, A5, pBuf, &movecount);
                      Gen_PushNormal(C4, A3, pBuf, &movecount);
                      Gen_PushNormal(C4, B6, pBuf, &movecount);
                      Gen_PushNormal(C4, B2, pBuf, &movecount);
                      Gen_PushNormal(C4, D6, pBuf, &movecount);
                      Gen_PushNormal(C4, D2, pBuf, &movecount);
                      Gen_PushNormal(C4, E5, pBuf, &movecount);
                      Gen_PushNormal(C4, E3, pBuf, &movecount);
                      break;
                  case D4:
                      Gen_PushNormal(D4, B5, pBuf, &movecount);
                      Gen_PushNormal(D4, B3, pBuf, &movecount);
                      Gen_PushNormal(D4, C6, pBuf, &movecount);
                      Gen_PushNormal(D4, C2, pBuf, &movecount);
                      Gen_PushNormal(D4, E6, pBuf, &movecount);
                      Gen_PushNormal(D4, E2, pBuf, &movecount);
                      Gen_PushNormal(D4, F5, pBuf, &movecount);
                      Gen_PushNormal(D4, F3, pBuf, &movecount);
                      break;
                  case E4:
                      Gen_PushNormal(E4, C5, pBuf, &movecount);
                      Gen_PushNormal(E4, C3, pBuf, &movecount);
                      Gen_PushNormal(E4, D6, pBuf, &movecount);
                      Gen_PushNormal(E4, D2, pBuf, &movecount);
                      Gen_PushNormal(E4, F6, pBuf, &movecount);
                      Gen_PushNormal(E4, F2, pBuf, &movecount);
                      Gen_PushNormal(E4, G5, pBuf, &movecount);
                      Gen_PushNormal(E4, G3, pBuf, &movecount);
                      break;
                  case F4:
                      Gen_PushNormal(F4, D5, pBuf, &movecount);
                      Gen_PushNormal(F4, D3, pBuf, &movecount);
                      Gen_PushNormal(F4, E6, pBuf, &movecount);
                      Gen_PushNormal(F4, E2, pBuf, &movecount);
                      Gen_PushNormal(F4, G6, pBuf, &movecount);
                      Gen_PushNormal(F4, G2, pBuf, &movecount);
                      Gen_PushNormal(F4, H5, pBuf, &movecount);
                      Gen_PushNormal(F4, H3, pBuf, &movecount);
                      break;
                  case G4:
                      Gen_PushNormal(G4, E5, pBuf, &movecount);
                      Gen_PushNormal(G4, E3, pBuf, &movecount);
                      Gen_PushNormal(G4, F6, pBuf, &movecount);
                      Gen_PushNormal(G4, F2, pBuf, &movecount);
                      Gen_PushNormal(G4, H6, pBuf, &movecount);
                      Gen_PushNormal(G4, H2, pBuf, &movecount);
                      break;
                  case H4:
                      Gen_PushNormal(H4, F5, pBuf, &movecount);
                      Gen_PushNormal(H4, F3, pBuf, &movecount);
                      Gen_PushNormal(H4, G6, pBuf, &movecount);
                      Gen_PushNormal(H4, G2, pBuf, &movecount);
                      break;
                  case A3:
                      Gen_PushNormal(A3, B5, pBuf, &movecount);
                      Gen_PushNormal(A3, B1, pBuf, &movecount);
                      Gen_PushNormal(A3, C4, pBuf, &movecount);
                      Gen_PushNormal(A3, C2, pBuf, &movecount);
                      break;
                  case B3:
                      Gen_PushNormal(B3, A5, pBuf, &movecount);
                      Gen_PushNormal(B3, A1, pBuf, &movecount);
                      Gen_PushNormal(B3, C5, pBuf, &movecount);
                      Gen_PushNormal(B3, C1, pBuf, &movecount);
                      Gen_PushNormal(B3, D4, pBuf, &movecount);
                      Gen_PushNormal(B3, D2, pBuf, &movecount);
                      break;
                  case C3:
                      Gen_PushNormal(C3, A4, pBuf, &movecount);
                      Gen_PushNormal(C3, A2, pBuf, &movecount);
                      Gen_PushNormal(C3, B5, pBuf, &movecount);
                      Gen_PushNormal(C3, B1, pBuf, &movecount);
                      Gen_PushNormal(C3, D5, pBuf, &movecount);
                      Gen_PushNormal(C3, D1, pBuf, &movecount);
                      Gen_PushNormal(C3, E4, pBuf, &movecount);
                      Gen_PushNormal(C3, E2, pBuf, &movecount);
                      break;
                  case D3:
                      Gen_PushNormal(D3, B4, pBuf, &movecount);
                      Gen_PushNormal(D3, B2, pBuf, &movecount);
                      Gen_PushNormal(D3, C5, pBuf, &movecount);
                      Gen_PushNormal(D3, C1, pBuf, &movecount);
                      Gen_PushNormal(D3, E5, pBuf, &movecount);
                      Gen_PushNormal(D3, E1, pBuf, &movecount);
                      Gen_PushNormal(D3, F4, pBuf, &movecount);
                      Gen_PushNormal(D3, F2, pBuf, &movecount);
                      break;
                  case E3:
                      Gen_PushNormal(E3, C4 , pBuf, &movecount);
                      Gen_PushNormal(E3, C2, pBuf, &movecount);
                      Gen_PushNormal(E3, D5, pBuf, &movecount);
                      Gen_PushNormal(E3, D1, pBuf, &movecount);
                      Gen_PushNormal(E3, F5, pBuf, &movecount);
                      Gen_PushNormal(E3, F1, pBuf, &movecount);
                      Gen_PushNormal(E3, G4, pBuf, &movecount);
                      Gen_PushNormal(E3, G2, pBuf, &movecount);
                      break;
                  case F3:
                      Gen_PushNormal(F3, D4, pBuf, &movecount);
                      Gen_PushNormal(F3, D2, pBuf, &movecount);
                      Gen_PushNormal(F3, E5, pBuf, &movecount);
                      Gen_PushNormal(F3, E1, pBuf, &movecount);
                      Gen_PushNormal(F3, G5, pBuf, &movecount);
                      Gen_PushNormal(F3, G1, pBuf, &movecount);
                      Gen_PushNormal(F3, H4, pBuf, &movecount);
                      Gen_PushNormal(F3, H2, pBuf, &movecount);
                      break;
                  case G3:
                      Gen_PushNormal(G3, E4, pBuf, &movecount);
                      Gen_PushNormal(G3, E2, pBuf, &movecount);
                      Gen_PushNormal(G3, F5, pBuf, &movecount);
                      Gen_PushNormal(G3, F1, pBuf, &movecount);
                      Gen_PushNormal(G3, H5, pBuf, &movecount);
                      Gen_PushNormal(G3, H1, pBuf, &movecount);
                      break;
                  case H3:
                      Gen_PushNormal(H3, F4, pBuf, &movecount);
                      Gen_PushNormal(H3, F2, pBuf, &movecount);
                      Gen_PushNormal(H3, G5, pBuf, &movecount);
                      Gen_PushNormal(H3, G1, pBuf, &movecount);
                      break;
                  case A2:
                      Gen_PushNormal(A2, B4, pBuf, &movecount);
                      Gen_PushNormal(A2, C3, pBuf, &movecount);
                      Gen_PushNormal(A2, C1, pBuf, &movecount);
                      break;
                  case B2:
                      Gen_PushNormal(B2, A4, pBuf, &movecount);
                      Gen_PushNormal(B2, C4, pBuf, &movecount);
                      Gen_PushNormal(B2, D3, pBuf, &movecount);
                      Gen_PushNormal(B2, D1, pBuf, &movecount);
                      break;
                  case C2:
                      Gen_PushNormal(C2, A3, pBuf, &movecount);
                      Gen_PushNormal(C2, A1, pBuf, &movecount);
                      Gen_PushNormal(C2, B4, pBuf, &movecount);
                      Gen_PushNormal(C2, D4, pBuf, &movecount);
                      Gen_PushNormal(C2, E3, pBuf, &movecount);
                      Gen_PushNormal(C2, E1, pBuf, &movecount);
                      break;
                  case D2:
                      Gen_PushNormal(D2, B3, pBuf, &movecount);
                      Gen_PushNormal(D2, B1, pBuf, &movecount);
                      Gen_PushNormal(D2, C4, pBuf, &movecount);
                      Gen_PushNormal(D2, E4, pBuf, &movecount);
                      Gen_PushNormal(D2, F3, pBuf, &movecount);
                      Gen_PushNormal(D2, F1, pBuf, &movecount);
                      break;
                  case E2:
                      Gen_PushNormal(E2, C3, pBuf, &movecount);
                      Gen_PushNormal(E2, C1, pBuf, &movecount);
                      Gen_PushNormal(E2, D4, pBuf, &movecount);
                      Gen_PushNormal(E2, F4, pBuf, &movecount);
                      Gen_PushNormal(E2, G3, pBuf, &movecount);
                      Gen_PushNormal(E2, G1, pBuf, &movecount);
                      break;
                  case F2:
                      Gen_PushNormal(F2, D3, pBuf, &movecount);
                      Gen_PushNormal(F2, D1, pBuf, &movecount);
                      Gen_PushNormal(F2, E4, pBuf, &movecount);
                      Gen_PushNormal(F2, G4, pBuf, &movecount);
                      Gen_PushNormal(F2, H3, pBuf, &movecount);
                      Gen_PushNormal(F2, H1, pBuf, &movecount);
                      break;
                  case G2:
                      Gen_PushNormal(G2, E3, pBuf, &movecount);
                      Gen_PushNormal(G2, E1, pBuf, &movecount);
                      Gen_PushNormal(G2, F4, pBuf, &movecount);
                      Gen_PushNormal(G2, H4, pBuf, &movecount);
                      break;
                  case H2:
                      Gen_PushNormal(H2, F3, pBuf, &movecount);
                      Gen_PushNormal(H2, F1, pBuf, &movecount);
                      Gen_PushNormal(H2, G4, pBuf, &movecount);
                      break;
                  case A1:
                      Gen_PushNormal(A1, B3, pBuf, &movecount);
                      Gen_PushNormal(A1, C2, pBuf, &movecount);
                      break;
                  case B1:
                      if (color[A3] != current_side)
                      Gen_PushNormal(B1, A3, pBuf, &movecount);
                      if (color[C3] != current_side)
                      Gen_PushNormal(B1, C3, pBuf, &movecount);
                      if (color[D2] != current_side)
                      Gen_PushNormal(B1, D2, pBuf, &movecount);
                      break;
                  case C1:
                      Gen_PushNormal(C1, A2, pBuf, &movecount);
                      Gen_PushNormal(C1, B3, pBuf, &movecount);
                      Gen_PushNormal(C1, D3, pBuf, &movecount);
                      Gen_PushNormal(C1, E2, pBuf, &movecount);
                      break;
                  case D1:
                      Gen_PushNormal(D1, B2, pBuf, &movecount);
                      Gen_PushNormal(D1, C3, pBuf, &movecount);
                      Gen_PushNormal(D1, E3, pBuf, &movecount);
                      Gen_PushNormal(D1, F2, pBuf, &movecount);
                      break;
                  case E1:
                      Gen_PushNormal(E1, C2, pBuf, &movecount);
                      Gen_PushNormal(E1, D3, pBuf, &movecount);
                      Gen_PushNormal(E1, F3, pBuf, &movecount);
                      Gen_PushNormal(E1, G2, pBuf, &movecount);
                      break;
                  case F1:
                      Gen_PushNormal(F1, D2, pBuf, &movecount);
                      Gen_PushNormal(F1, E3, pBuf, &movecount);
                      Gen_PushNormal(F1, G3, pBuf, &movecount);
                      Gen_PushNormal(F1, H2, pBuf, &movecount);
                      break;
                  case G1:
                      if (color[E2] != current_side)
                      Gen_PushNormal(G1, E2, pBuf, &movecount);
                      if (color[F3] != current_side)
                      Gen_PushNormal(G1, F3, pBuf, &movecount);
                      if (color[H3] != current_side)
                      Gen_PushNormal(G1, H3, pBuf, &movecount);
                      break;
                  case H1:
                      Gen_PushNormal(H1, F2, pBuf, &movecount);
                      Gen_PushNormal(H1, G3, pBuf, &movecount);
                      break;
              }
	    break;
            

	  case KING:
	    /* the column and rank checks are to make sure it is on the board */
	    /* The 'normal' moves */
	    col = COL (i);
	    if (col && color[i - 1] != current_side)
	      Gen_PushKing (i, i - 1, pBuf, &movecount);	/* left */
	    if (col < 7 && color[i + 1] != current_side)
	      Gen_PushKing (i, i + 1, pBuf, &movecount);	/* right */
	    if (i > 7 && color[i - 8] != current_side)
	      Gen_PushKing (i, i - 8, pBuf, &movecount);	/* up */
	    if (i < 56 && color[i + 8] != current_side)
	      Gen_PushKing (i, i + 8, pBuf, &movecount);	/* down */
	    if (col && i > 7 && color[i - 9] != current_side)
	      Gen_PushKing (i, i - 9, pBuf, &movecount);	/* left up */
	    if (col < 7 && i > 7 && color[i - 7] != current_side)
	      Gen_PushKing (i, i - 7, pBuf, &movecount);	/* right up */
	    if (col && i < 56 && color[i + 7] != current_side)
	      Gen_PushKing (i, i + 7, pBuf, &movecount);	/* left down */
	    if (col < 7 && i < 56 && color[i + 9] != current_side)
	      Gen_PushKing (i, i + 9, pBuf, &movecount);	/* right down */

	    /* The castle moves */
	    if (current_side == WHITE)
	      {
		/* Can white short castle? */
		if (castle_rights & 1)
		  {
		    /* If white can castle the white king has to be in square 60 */
		    if (col &&
			color[i + 1] == EMPTY &&
			color[i + 2] == EMPTY &&
			!IsInCheck (current_side) &&
			!IsAttacked (current_side, i + 1))
		      {
			/* The king goes 2 sq to the left */
			Gen_PushKing (i, i + 2, pBuf, &movecount);
		      }
		  }

		/* Can white long castle? */
		if (castle_rights & 2)
		  {
		    if (col &&
			color[i - 1] == EMPTY &&
			color[i - 2] == EMPTY &&
			color[i - 3] == EMPTY &&
			!IsInCheck (current_side) &&
			!IsAttacked (current_side, i - 1))
		      {
			/* The king goes 2 sq to the left */
			Gen_PushKing (i, i - 2, pBuf, &movecount);
		      }
		  }
	      }
	    else if (current_side == BLACK)
	      {
		/* Can black short castle? */
		if (castle_rights & 4)
		  {
		    /* If white can castle the white king has to be in square 60 */
		    if (col &&
			color[i + 1] == EMPTY &&
			color[i + 2] == EMPTY &&
			piece[i + 3] == ROOK &&
			!IsInCheck (current_side) &&
			!IsAttacked (current_side, i + 1))
		      {
			/* The king goes 2 sq to the left */
			Gen_PushKing (i, i + 2, pBuf, &movecount);
		      }
		  }
		/* Can black long castle? */
		if (castle_rights & 8)
		  {
		    if (col &&
			color[i - 1] == EMPTY &&
			color[i - 2] == EMPTY &&
			color[i - 3] == EMPTY &&
			piece[i - 4] == ROOK &&
			!IsInCheck (current_side) &&
			!IsAttacked (current_side, i - 1))
		      {
			/* The king goes 2 sq to the left */
			Gen_PushKing (i, i - 2, pBuf, &movecount);
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
int
GenCaps (int current_side, MOVE * pBuf)
{
  int i;			/* Counter for the board squares */
  int k;			/* Counter for cols */
  int y;
  int row;
  int col;
  int capscount;		/* Counter for the posible captures */
  int xside;
  xside = (WHITE + BLACK) - current_side;
  capscount = 0;

  for (i = 0; i < 64; i++)	/* Scan all board */
    if (color[i] == current_side)
      {
	switch (piece[i])
	  {

	  case PAWN:
	    col = COL (i);
	    row = ROW (i);
	    if (current_side == BLACK)
	      {
		/* This isn't a capture, but it's necesary in order to
		 * not oversee promotions */
		if (row > 7 && color[i + 8] == EMPTY)
		  /* Pawn advances one square.
		   * We use Gen_PushPawn because it can be a promotion */
		  Gen_PushPawn (i, i + 8, pBuf, &capscount);
		if (col && color[i + 7] == WHITE)
		  /* Pawn captures and it can be a promotion */
		  Gen_PushPawn (i, i + 7, pBuf, &capscount);
		if (col < 7 && color[i + 9] == WHITE)
		  /* Pawn captures and can be a promotion */
		  Gen_PushPawn (i, i + 9, pBuf, &capscount);
		/* For en passant capture */
		if (col && piece[i + 7] == EPS_SQUARE)
		  /* Pawn captures and it can be a promotion */
		  Gen_PushPawn (i, i + 7, pBuf, &capscount);
		if (col < 7 && piece[i + 9] == EPS_SQUARE)
		  /* Pawn captures and can be a promotion */
		  Gen_PushPawn (i, i + 9, pBuf, &capscount);
	      }
	    else if (current_side == WHITE)
	      {
		if (row < 2 && color[i - 8] == EMPTY)
		  /* This isn't a capture, but it's necesary in order to
		   * not oversee promotions */
		  Gen_PushPawn (i, i - 8, pBuf, &capscount);
		/* For captures */
		if (col && color[i - 9] == BLACK)
		  Gen_PushPawn (i, i - 9, pBuf, &capscount);
		if (col < 7 && color[i - 7] == BLACK)
		  Gen_PushPawn (i, i - 7, pBuf, &capscount);
		/* For en passant capture */
		if (col && piece[i - 9] == EPS_SQUARE)
		  Gen_PushPawn (i, i - 9, pBuf, &capscount);
		if (col < 7 && piece[i - 7] == EPS_SQUARE)
		  Gen_PushPawn (i, i - 7, pBuf, &capscount);
	      }
	    break;

	  case KNIGHT:
	    col = COL (i);
	    y = i - 6;
	    if (y >= 0 && col < 6 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i - 10;
	    if (y >= 0 && col > 1 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i - 15;
	    if (y >= 0 && col < 7 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i - 17;
	    if (y >= 0 && col > 0 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i + 6;
	    if (y < 64 && col > 1 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i + 10;
	    if (y < 64 && col < 6 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i + 15;
	    if (y < 64 && col > 0 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    y = i + 17;
	    if (y < 64 && col < 7 && color[y] == xside)
	      Gen_PushNormal (i, y, pBuf, &capscount);
	    break;

	  case QUEEN:		/* == BISHOP+ROOK */

	  case BISHOP:
	    for (y = i - 9; y >= 0 && COL (y) != 7; y -= 9)
	      {			/* go left up */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    for (y = i - 7; y >= 0 && COL (y) != 0; y -= 7)
	      {			/* go right up */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    for (y = i + 9; y < 64 && COL (y) != 0; y += 9)
	      {			/* go right down */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    for (y = i + 7; y < 64 && COL (y) != 7; y += 7)
	      {			/* go left down */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    if (piece[i] == BISHOP)	/* In the case of the bishop we're done */
	      break;

	    /* FALL THROUGH FOR QUEEN {I meant to do that!} ;-) */
	  case ROOK:
	    col = COL (i);
	    for (k = i - col, y = i - 1; y >= k; y--)
	      {			/* go left */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    for (k = i - col + 7, y = i + 1; y <= k; y++)
	      {			/* go right */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    for (y = i - 8; y >= 0; y -= 8)
	      {			/* go up */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    for (y = i + 8; y < 64; y += 8)
	      {			/* go down */
		if (color[y] != EMPTY)
		  {
		    if (color[y] != current_side)
		      Gen_PushNormal (i, y, pBuf, &capscount);
		    break;
		  }
	      }
	    break;

	  case KING:
	    ///* the column and rank checks are to make sure it is on the board*/
	    col = COL (i);
	    if (col && color[i - 1] == xside)
	      Gen_PushKing (i, i - 1, pBuf, &capscount);	/* left */
	    if (col < 7 && color[i + 1] == xside)
	      Gen_PushKing (i, i + 1, pBuf, &capscount);	/* right */
	    if (i > 7 && color[i - 8] == xside)
	      Gen_PushKing (i, i - 8, pBuf, &capscount);	/* up */
	    if (i < 56 && color[i + 8] == xside)
	      Gen_PushKing (i, i + 8, pBuf, &capscount);	/* down */
	    if (col && i > 7 && color[i - 9] == xside)
	      Gen_PushKing (i, i - 9, pBuf, &capscount);	/* left up */
	    if (col < 7 && i > 7 && color[i - 7] == xside)
	      Gen_PushKing (i, i - 7, pBuf, &capscount);	/* right up */
	    if (col && i < 56 && color[i + 7] == xside)
	      Gen_PushKing (i, i + 7, pBuf, &capscount);	/* left down */
	    if (col < 7 && i < 56 && color[i + 9] == xside)
	      Gen_PushKing (i, i + 9, pBuf, &capscount);	/* right down */
	    break;
//                               default:
//                               printf("Piece type unknown");
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
int
Eval ()
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
         the sign in the score */
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
int
IsInCheck (int current_side)
{
  int k;			/* The square where the king is placed */

  /* Find the King of the side to move */
  for (k = 0; k < 64; k++)
    if ((piece[k] == KING) && color[k] == current_side)
      break;

  /* Use IsAttacked in order to know if current_side is under check */
  return IsAttacked (current_side, k);
}

/* Returns 1 if square k is attacked by current_side, 0 otherwise. Necesary, v.g., to check
 * castle rules (if king goes from e1 to g1, f1 can't be attacked by an enemy piece) */
int
IsAttacked (int current_side, int k)
{
  int h;
  int y;
  int row;			/* Row where the square k is placed */
  int col;			/* Col where the square k is placed */
  int xside;
  xside = (WHITE + BLACK) - current_side;	/* opposite current_side, who may be attacking */

  /* Situation of the square */
  row = ROW (k);
  col = COL (k);

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
	    if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
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
	    if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
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
	    if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
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
	    if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOK))
	      return 1;
	    if (piece[y] != EMPTY && piece[y] != EPS_SQUARE)
	      break;
	  }
    }
  /* Check diagonal lines for attacking of Queen, Bishop, King, Pawn */
  /* go right down */
  y = k + 9;
  if (y < 64 && COL (y) != 0)
    {
      if (color[y] == xside)
	{
	  if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
	    return 1;
	  if (current_side == BLACK && piece[y] == PAWN)
	    return 1;
	}
      if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
	for (y += 9; y < 64 && COL (y) != 0; y += 9)
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
  if (y < 64 && COL (y) != 7)
    {
      if (color[y] == xside)
	{
	  if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
	    return 1;
	  if (current_side == BLACK && piece[y] == PAWN)
	    return 1;
	}
      if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
	for (y += 7; y < 64 && COL (y) != 7; y += 7)
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
  if (y >= 0 && COL (y) != 7)
    {
      if (color[y] == xside)
	{
	  if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
	    return 1;
	  if (current_side == WHITE && piece[y] == PAWN)
	    return 1;
	}
      if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
	for (y -= 9; y >= 0 && COL (y) != 7; y -= 9)
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
  if (y >= 0 && COL (y) != 0)
    {
      if (color[y] == xside)
	{
	  if (piece[y] == KING || piece[y] == QUEEN || piece[y] == BISHOP)
	    return 1;
	  if (current_side == WHITE && piece[y] == PAWN)
	    return 1;
	}
      if (piece[y] == EMPTY || piece[y] == EPS_SQUARE)
	for (y -= 7; y >= 0 && COL (y) != 0; y -= 7)
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

int
MakeMove (MOVE m)
{
  int r;
  int i;
  
  count_MakeMove++;

  hist[hdp].m = m;
  hist[hdp].cap = piece[m.dest];	/* store in history the piece of the dest square */
  hist[hdp].castle = castle_rights;

  piece[m.dest] = piece[m.from];	/* dest piece is the one in the original square */
  color[m.dest] = color[m.from];	/* The dest square color is the one of the origin piece */
  piece[m.from] = EMPTY;	/* The original square becomes empty */
  color[m.from] = EMPTY;	/* The original color becomes empty */

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

  /* Remove possible eps piece, remaining from former move */
  if (hist[hdp - 1].m.type == MOVE_TYPE_PAWN_TWO)
    {
      for (i = 16; i <= 23; i++)
	{
	  if (piece[i] == EPS_SQUARE)
	    {
	      piece[i] = EMPTY;
	      /* this seems unnecesary, but otherwise a bug occurs:
	       * after: a3 Nc6 d4 e6, white isn't allowed to play e4 */
//                color[i] = EMPTY;
	      break;
	    }
	}
      for (i = 40; i <= 47; i++)
	{
	  if (piece[i] == EPS_SQUARE)
	    {
	      piece[i] = EMPTY;
//                              color[i] = EMPTY;
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
	  puts ("Impossible to get here...");
	  assert (false);
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
  castle_rights &= castle_mask[m.from] & castle_mask[m.dest];

  /* Checking if after making the move we're in check */
  r = !IsInCheck (side);

  /* After making move, give turn to opponent */
  side = (WHITE + BLACK) - side;

  return r;
}

/* Undo what MakeMove did */
void
TakeBack ()
{

  int i;

  side = (WHITE + BLACK) - side;
  hdp--;
  ply--;
  piece[hist[hdp].m.from] = piece[hist[hdp].m.dest];
  piece[hist[hdp].m.dest] = hist[hdp].cap;
  color[hist[hdp].m.from] = side;

  /* Update castle rights */
  castle_rights = hist[hdp].castle;

  /* Return the captured material */
  if (hist[hdp].cap != EMPTY && hist[hdp].cap != EPS_SQUARE)
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
  if (hist[hdp - 1].m.type == MOVE_TYPE_PAWN_TWO)
    {
      if (side == WHITE)
	{
	  piece[hist[hdp - 1].m.dest - 8] = EPS_SQUARE;
//            color[hist[hdp-1].m.dest - 8] = EMPTY;
	}
      else if (side == BLACK)
	{
	  piece[hist[hdp - 1].m.dest + 8] = EPS_SQUARE;
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

int
Search (int alpha, int beta, int depth, MOVE * pBestMove)
{
  int i;
  int value;			/* To store the evaluation */
  int havemove;			/* Either we have or not a legal move available */
  int movecnt;			/* The number of available moves */

  MOVE moveBuf[200];		/* List of movements */
  MOVE tmpMove;

  // nodes++; /* visiting a node, count it */
  havemove = 0;			/* is there a move available? */
  pBestMove->type = MOVE_TYPE_NONE;

  /* Generate and count all moves for current position */
  movecnt = GenMoves (side, moveBuf);
  assert (movecnt < 201);

  /* Once we have all the moves available, we loop through the posible
   * moves and apply an alpha-beta search */
  for (i = 0; i < movecnt; ++i)
    {

      if (!MakeMove (moveBuf[i]))
	{
	  /* If the current move isn't legal, we take it back
	   * and take the next move in the list */
	  TakeBack ();
	  continue;
	}

      /* If we've reached this far, then we have a move available */
      havemove = 1;

      /* This 'if' takes us to the deep of the position, the leaf nodes */
      if (depth - 1 > 0)
	{
	  value = -Search (-beta, -alpha, depth - 1, &tmpMove);
	}
      /* If no depth left (leaf node), we evalute the position
         and apply the alpha-beta search.
         In the case of existing a quiescent function, it should be
         called here instead of Eval() */
      else
	{
	  value = -Quiescent (-beta, -alpha);
	  // value = -Eval();
	}

      /* We've evaluated the position, so we return to the previous position in such a way
         that when we take the next move from moveBuf everything is in order */
      TakeBack ();

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
      if (IsInCheck (side))
	return -MATE + ply;	/* add ply to find the longest path to lose or shortest path to win */
      else
	return 0;
    }

  /* Finally we return alpha, the score value */
  return alpha;
}

int
Quiescent (int alpha, int beta)
{
  int i;
  int capscnt;
  int stand_pat;
  int score;
  MOVE cBuf[200];

  count_quies_calls++;

  /* First we just try the evaluation function */
  stand_pat = Eval ();
  if (stand_pat >= beta)
    return beta;
  if (alpha < stand_pat)
    alpha = stand_pat;

  /* If we haven't got a cut off we generate the captures and
   * store them in cBuf */
  capscnt = GenCaps (side, cBuf);

  count_cap_calls++;

  for (i = 0; i < capscnt; ++i)
    {
      if (!MakeMove (cBuf[i]))
	{
	  /* If the current move isn't legal, we take it back
	   * and take the next move in the list */
	  TakeBack ();
	  continue;
	}
      score = -Quiescent (-beta, -alpha);
      TakeBack ();
      if (score >= beta)
	return beta;
      if (score > alpha)
	alpha = score;
    }
  return alpha;
}



MOVE
ComputerThink (int depth)
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
  count_quies_calls = 0;
  count_cap_calls = 0;

  clock_t start;
  clock_t stop;
  double t = 0.0;

  /* Start timer */
  start = clock ();
  assert (start != -1);

  /* Search now */
  score = Search (-MATE, MATE, depth, &m);

  /* Stop timer */
  stop = clock ();
  t = (double) (stop - start) / CLOCKS_PER_SEC;
  knps = ((double) count_quies_calls / t) / 1000.;

  double ratio_Qsearc_Capcalls =
    (double) count_quies_calls / (double) count_cap_calls;

  double decimal_score = ((double) score) / 100.;
  if (side == BLACK)
    {
      decimal_score = -decimal_score;
    }

  /* After searching, print results */
  printf
    ("Search result: move = %c%d%c%d; depth = %d, score = %.2f, time = %.2fs knps = %.2f\n countCapCalls = %d\n countQSearch = %d\n moves made = %d\n ratio_Qsearc_Capcalls = %.2f\n",
     'a' + COL (m.from), 8 - ROW (m.from), 'a' + COL (m.dest),
     8 - ROW (m.dest), depth, decimal_score, t, knps, count_cap_calls,
     count_quies_calls, count_MakeMove, ratio_Qsearc_Capcalls);
  return m;
}

/*
 ****************************************************************************
 * Utilities *
 ****************************************************************************
 */

void
PrintBoard ()
{
  char pieceName[] = "PNBRQKpnbrqk";
  int i;
  for (i = 0; i < 64; i++)
    {
      if ((i & 7) == 0)
	{
	  printf ("   +---+---+---+---+---+---+---+---+\n");
	  if (i <= 56)
	    {
	      printf (" %d |", 8 - (((unsigned) i) >> 3));
	    }
	}

      if (piece[i] == EMPTY && ((((unsigned) i) >> 3) % 2 == 0 && i % 2 == 0))
	printf ("   |");
      else if (piece[i] == EMPTY
	       && ((((unsigned) i) >> 3) % 2 != 0 && i % 2 != 0))
	printf ("   |");
      else if (piece[i] == EMPTY)
	printf ("   |");
      else if (piece[i] == EPS_SQUARE)
	printf (" * |");
      else
	{
	  if (color[i] == WHITE)
	    printf (" %c |", pieceName[piece[i]]);
	  else
	    printf ("<%c>|", pieceName[piece[i] + 6]);
	}
      if ((i & 7) == 7)
	printf ("\n");
    }
  printf
    ("   +---+---+---+---+---+---+---+---+\n     a   b   c   d   e   f   g   h\n");
}


/* Returns the number of posible positions to a given depth. Based on the
 perft function on Danasah */
unsigned long long
perft (int depth)
{
  int i;
  int movecnt;			/* The number of available moves */
  unsigned long long nodes = 0;

  if (!depth)
    return 1;

  MOVE moveBuf[200];		/* List of movements */

  /* Generate and count all moves for current position */
  movecnt = GenMoves (side, moveBuf);

  /* Once we have all the moves available, we loop through them */
  for (i = 0; i < movecnt; ++i)
    {
      /* Not a legal move? Then we unmake it and continue to the next one in the list */
      if (!MakeMove (moveBuf[i]))
	{
	  TakeBack ();
	  continue;
	}

      /* Just in case we want to count for checks */
//        if (IsInCheck(side))
//        {
//            count_checks++;
//        }

      /* This 'if' takes us to the deep of the position */
      nodes += perft (depth - 1);
      TakeBack ();
    }

  return nodes;
}


/*
 ****************************************************************************
 * Main program *
 ****************************************************************************
 */

void
startgame ()
{
  int i;
  for (i = 0; i < 64; ++i)
    {
      piece[i] = init_piece[i];
      color[i] = init_color[i];
    }

  side = WHITE;
  computer_side = BLACK;	/* Human is white side */
  hdp = 0;
  castle_rights = 15;
}

void
xboard ()
{
  char line[256], command[256], c;
  int from, dest, i;
  MOVE moveBuf[200], bestMove;
  int movecnt;
  //int illegal_king = 0;

  printf ("\n");

  startgame ();

  for (;;)
    {
      fflush (stdout);
      if (side == computer_side)
	{			/* computer's turn */
	  /* Find out the best move to react the current position */
	  bestMove = ComputerThink (max_depth);
	  MakeMove (bestMove);
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
	    }
	  printf ("move %c%d%c%d%c\n", 'a' + COL (bestMove.from), 8
		      - ROW (bestMove.from), 'a' + COL (bestMove.dest), 8
		      - ROW (bestMove.dest), c);
	  continue;
	}

      if (!fgets (line, 256, stdin))
	return;
      if (line[0] == '\n')
	continue;
      sscanf (line, "%s", command);
      if (!strcmp (command, "xboard"))
	{
	  continue;
	}
      if (!strcmp (command, "new"))
	{
	  startgame ();
	  continue;
	}
      if (!strcmp (command, "quit"))
	{
	  return;
	}
      if (!strcmp (command, "force"))
	{
	  computer_side = EMPTY;
	  continue;
	}
      if (!strcmp (command, "white"))
	{
	  side = WHITE;
	  computer_side = BLACK;
	  continue;
	}
      if (!strcmp (command, "black"))
	{
	  side = BLACK;
	  computer_side = WHITE;
	  continue;
	}
      if (!strcmp (command, "sd"))
	{
	  sscanf (line, "sd %d", &max_depth);
	  continue;
	}
      if (!strcmp (command, "go"))
	{
	  computer_side = side;
	  continue;
	}
      if (!strcmp (command, "undo"))
	{
	  if (hdp == 0)
	    continue;
	  TakeBack ();
	  continue;
	}
      if (!strcmp (command, "remove"))
	{
	  if (hdp <= 1)
	    continue;
	  TakeBack ();
	  TakeBack ();
	  continue;
	}

      /* maybe the user entered a move? */
      
	  /* is a move? */
	  if (command[0] < 'a' || command[0] > 'h' ||
		  command[1] < '0' || command[1] > '9' ||
		  command[2] < 'a' || command[2] > 'h' ||
		  command[3] < '0' || command[3] > '9') 
	  {
	   	printf("Error (unknown command): %s\n", command); /*no move, unknown command */
		continue;
	  }
      
      from = command[0] - 'a';
      from += 8 * (8 - (command[1] - '0'));
      dest = command[2] - 'a';
      dest += 8 * (8 - (command[3] - '0'));
      ply = 0;
      movecnt = GenMoves (side, moveBuf);

      /* loop through the moves to see if it's legal */
      for (i = 0; i < movecnt; ++i) {
		if (moveBuf[i].from == from && moveBuf[i].dest == dest)
	  	{
			if (piece[from] == PAWN && (dest < 8 || dest > 55)) 
			{
				if (command[4] != 'q' && command[4] != 'r' && command[4] != 'b' && command[4] != 'n')
				{
					printf ("Illegal move. Bad letter for promo\n");
					goto goon;
				}
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
				}
			}
		
	    	if (MakeMove (moveBuf[i]))
	    	{
				goto goon;	/* legal move */
			}
			else {
				printf ("Illegal move. King is in check\n");
				goto goon;
			}	
		}
	  }
      printf ("Illegal move.\n");  /* illegal move */
      
goon:
	  continue;
	}
}

int
main ()
{

  setlocale (LC_ALL, "");

  /* It mainly calls ComputerThink(maxdepth) to the desired ply */

  char s[256];
  int from;
  int dest;
  int i;
  //int computer_side;

  startgame ();

  max_depth = 4;		/* max depth to search */
  MOVE moveBuf[200];
  int movecnt;

  puts ("Second Chess, by Emilio Diaz");
  puts (" Help");
  puts (" d: display board");
  puts (" MOVE: make a move (e.g. b1c3, a7a8q, e1g1)");
  puts (" on: force computer to move");
  puts (" quit: exit");
  puts (" sd n: set engine depth to n plies (default 4)");
  puts (" undo: take back last move");

  side = WHITE;
  computer_side = BLACK;	/* Human is white side */

  hdp = 0;			/* Current move order */
  for (;;)
    {
      if (side == computer_side)
	{			/* Computer's turn */
	  /* Find out the best move to react the current position */
	  MOVE bestMove = ComputerThink (max_depth);
	  MakeMove (bestMove);
	  PrintBoard ();
	  printf ("CASTLE: %d\n", castle_rights);
	  continue;
	}

      /* Get user input */
      printf ("sc> ");
      if (scanf ("%s", s) == EOF)	/* close program */
	return 0;
      if (!strcmp (s, "d"))
	{
	  PrintBoard ();
	  continue;
	}
      if (!strcmp (s, "undo"))
	{
	  TakeBack ();
	  PrintBoard ();
	  computer_side = (WHITE + BLACK) - computer_side;
	  continue;
	}
      if (!strcmp (s, "xboard"))
	{
	  xboard ();
	  return 0;
	}
      if (!strcmp (s, "on"))
	{
	  computer_side = side;
	  continue;
	}
      if (!strcmp (s, "pass"))
	{
	  side = (WHITE + BLACK) - side;
	  continue;
	}
      if (!strcmp (s, "sd"))
	{
	  scanf ("%d", &max_depth);
	  continue;
	}
      if (!strcmp (s, "perft"))
	{
	  scanf ("%d", &max_depth);
	  clock_t start;
	  clock_t stop;
	  double t = 0.0;
	  /* Start timer */
	  start = clock ();
	  unsigned long long count = perft (max_depth);
	  /* Stop timer */
	  stop = clock ();
	  t = (double) (stop - start) / CLOCKS_PER_SEC;
	  printf ("nodes = %llu\n", count);
	  printf ("time = %.2f s\n", t);
          double Mnps = (count/t)/1000000;
          printf ("MegaNodes/second = %.2f Mnps\n", Mnps);
	  continue;
	}
      if (!strcmp (s, "quit"))
	{
	  printf ("Good bye!\n");
	  return 0;
	}

      /* Maybe the user entered a move? */
      from = s[0] - 'a';
      from += 8 * (8 - (s[1] - '0'));
      dest = s[2] - 'a';
      dest += 8 * (8 - (s[3] - '0'));
      ply = 0;
      movecnt = GenMoves (side, moveBuf);

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
		    puts
		      ("promoting to a McGuffin..., I'll give you a queen");
		    moveBuf[i].type = MOVE_TYPE_PROMOTION_TO_QUEEN;
		  }
	      }
	    if (!MakeMove (moveBuf[i]))
	      {
		TakeBack ();
		printf ("Illegal move.\n");
	      }
	    break;
	  }
      PrintBoard ();
    }
}
