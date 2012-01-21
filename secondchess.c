/*
secondchess - gpl, by Emilio Díaz, based on firstchess by Pham Hong Nguyen
Version: beta ta
*/
/*

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
* Make and Take back a move, IsInCheck *
* Search function - a typical alphabeta *
* Utility *
* Main program *
*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
/*
****************************************************************************
* Some definitions *
****************************************************************************
*/
#define PAWN 0
#define KNIGHT 1
#define BISHOP 2
#define ROOKC 3
#define ROOKU 4
#define QUEEN 5
#define KINGC 6
#define KINGU 7
#define EMPTY 8
#define WHITE 0
#define BLACK 1
#define false 0

#define VALUE_PAWN 100
#define VALUE_KNIGHT 300
#define VALUE_BISHOP 300
#define VALUE_ROOKU 500
#define VALUE_ROOKC 500
#define VALUE_QUEEN 900
#define VALUE_KINGU 10000
#define VALUE_KINGC 10000

#define MATE 10000 /* equal value of King, losing King==mate */

#define COL(pos) ((pos)&7)
#define ROW(pos) (((unsigned)pos)>>3)

/*
****************************************************************************
* Board representation and main varians *
****************************************************************************
*/
/* Board representation */
int piece[64] = {
    ROOKC, KNIGHT, BISHOP, QUEEN, KINGC, BISHOP, KNIGHT, ROOKC,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN,
    ROOKC, KNIGHT, BISHOP, QUEEN, KINGC, BISHOP, KNIGHT, ROOKC
};

int color[64] = {
    BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
    BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE
};

int side; /* side to move, value = BLACK or WHITE */

/* For move generation */
#define MOVE_TYPE_NONE 0
#define MOVE_TYPE_NORMAL 1
#define MOVE_TYPE_CASTLE 2
#define MOVE_TYPE_ENPASANT 3
#define MOVE_TYPE_PROMOTION_TO_QUEEN 4
#define MOVE_TYPE_PROMOTION_TO_ROOK 5
#define MOVE_TYPE_PROMOTION_TO_BISHOP 6
#define MOVE_TYPE_PROMOTION_TO_KNIGHT 7

/* A move is defined by its origin and final squares, and by the kind of
* move it's: normal, castle, enpasant... */
typedef struct tag_MOVE {
    int from;
    int dest;
    int type;
} MOVE;

/* For storing all moves of game */
typedef struct tag_HIST {
    MOVE m;
    int cap;
} HIST;

HIST hist[6000]; /* Game length < 6000 */

int hdp; /* Current move order */

/* For searching */
int nodes; /* Count all visited nodes when searching */
int ply; /* ply of search */

/*
****************************************************************************
* Move generator *
* Lack: no enpassant, no castle *
****************************************************************************
*/
void Gen_Push(int from, int dest, int type, MOVE * pBuf, int *pMCount)
{
    MOVE move;
    move.from = from;
    move.dest = dest;
    move.type = type;
    pBuf[*pMCount] = move;
    *pMCount = *pMCount + 1;
}

void Gen_PushNormal(int from, int dest, MOVE * pBuf, int *pMCount)
{
    Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
}

/* Pawn can promote */
void Gen_PushPawn(int from, int dest, MOVE * pBuf, int *pMCount)
{
/* The 7 and 56 are to limit pawns to the 2nd through 7th ranks, which
* means this isn't a promotion*/
    if (dest > 7 && dest < 56) /* this is just a normal move */
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

/* King*/
void Gen_PushKing(int from, int dest, MOVE * pBuf, int *pMCount)
{
/* Is it a castle?*/
    if (from == 60 && dest == 62) /* this is a short castle */
    {
		Gen_Push(from, dest, MOVE_TYPE_CASTLE, pBuf, pMCount);
	}
    else /* otherwise it's a normal move */
    {
		Gen_Push(from, dest, MOVE_TYPE_NORMAL, pBuf, pMCount);
        
    }
}

/* Gen all moves of current_side to move and push them to pBuf, return number of moves */
int Gen(int current_side, MOVE * pBuf)
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
					 * We use Gen_PushPawn
					 * because it can be a promotion */
                        Gen_PushPawn(i, i + 8, pBuf, &movecount);
                    if (row == 1 && color[i + 8] == EMPTY && color[i + 16] == EMPTY)
					/* Pawn advances two squares */
                        Gen_PushNormal(i, i + 16, pBuf, &movecount);
                    if (col && color[i + 7] == WHITE)
					/* Pawn captures */
                        Gen_PushNormal(i, i + 7, pBuf, &movecount);
                    if (col < 7 && color[i + 9] == WHITE)
					/* Pawn captures */
                        Gen_PushNormal(i, i + 9, pBuf, &movecount);
                }
                else
                {
                    if (color[i - 8] == EMPTY)
                        Gen_PushPawn(i, i - 8, pBuf, &movecount);
                    if (row == 6 && color[i - 8] == EMPTY && color[i - 16] == EMPTY)
                        Gen_PushNormal(i, i - 16, pBuf, &movecount);
                    if (col && color[i - 9] == BLACK)
                        Gen_PushNormal(i, i - 9, pBuf, &movecount);
                    if (col < 7 && color[i - 7] == BLACK)
                        Gen_PushNormal(i, i - 7, pBuf, &movecount);
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
            case ROOKU:
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
                
            case ROOKC:
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

            case KINGU:
				/* the column and rank checks are to make sure it is on the board*/
                col = COL(i);
                if (col && color[i - 1] != current_side)
                    Gen_PushNormal(i, i - 1, pBuf, &movecount); /* left */
                if (col < 7 && color[i + 1] != current_side)
                    Gen_PushNormal(i, i + 1, pBuf, &movecount); /* right */
                if (i > 7 && color[i - 8] != current_side)
                    Gen_PushNormal(i, i - 8, pBuf, &movecount); /* up */
                if (i < 56 && color[i + 8] != current_side)
                    Gen_PushNormal(i, i + 8, pBuf, &movecount); /* down */
                if (col && i > 7 && color[i - 9] != current_side)
                    Gen_PushNormal(i, i - 9, pBuf, &movecount); /* left up */
                if (col < 7 && i > 7 && color[i - 7] != current_side)
                    Gen_PushNormal(i, i - 7, pBuf, &movecount); /* right up */
                if (col && i < 56 && color[i + 7] != current_side)
                    Gen_PushNormal(i, i + 7, pBuf, &movecount); /* left down */
                if (col < 7 && i < 56 && color[i + 9] != current_side)
                    Gen_PushNormal(i, i + 9, pBuf, &movecount); /* right down */               
                break;
                
            case KINGC:
				/* the column and rank checks are to make sure it is on the board*/
                col = COL(i);
                if (col && color[i - 1] != current_side)
                    Gen_PushNormal(i, i - 1, pBuf, &movecount); /* left */
                if (col < 7 && color[i + 1] != current_side)
                    Gen_PushNormal(i, i + 1, pBuf, &movecount); /* right */
                if (i > 7 && color[i - 8] != current_side)
                    Gen_PushNormal(i, i - 8, pBuf, &movecount); /* up */
                if (i < 56 && color[i + 8] != current_side)
                    Gen_PushNormal(i, i + 8, pBuf, &movecount); /* down */
                if (col && i > 7 && color[i - 9] != current_side)
                    Gen_PushNormal(i, i - 9, pBuf, &movecount); /* left up */
                if (col < 7 && i > 7 && color[i - 7] != current_side)
                    Gen_PushNormal(i, i - 7, pBuf, &movecount); /* right up */
                if (col && i < 56 && color[i + 7] != current_side)
                    Gen_PushNormal(i, i + 7, pBuf, &movecount); /* left down */
                if (col < 7 && i < 56 && color[i + 9] != current_side)
                    Gen_PushNormal(i, i + 9, pBuf, &movecount); /* right down */
                /* Can we castle? First attempt without checking the legality */
                if (col && color[i + 1] == EMPTY && color[i + 2] == EMPTY)
                {
					/* The king goes 2 sq to the left */
					Gen_PushKing(i, i + 2, pBuf, &movecount);
				}
                break;
            default:
                puts("piece type unknown");
                assert(false);
            }
        }
    return movecount;
}

/*
****************************************************************************
* Evaluation for current position - main "brain" function *
* Lack: almost no knowlegde *
****************************************************************************
*/
int Eval()
{
    /* The values of the pieces in centipawns */
    int value_piece[10] = {VALUE_PAWN, VALUE_KNIGHT, VALUE_BISHOP, VALUE_ROOKU, VALUE_ROOKC, VALUE_QUEEN, VALUE_KINGU, VALUE_KINGC};
    /* A counter for the board squares */
    int i;
    /* The score of the position */
    int score = 0;
    
    for (i = 0; i < 64; i++)
    {
        if (color[i] == WHITE)
        {
			score += value_piece[piece[i]];
		}
        else if (color[i] == BLACK)
        {
			score -= value_piece[piece[i]];
		}
    }
    
    if (side == WHITE)
        return score;
    return -score;
}

/*
****************************************************************************
* Make and Take back a move, IsInCheck *
****************************************************************************
*/
/* Check and return 1 if side is in check, 0 otherwise */
int IsInCheck(int current_side)
{
    int k; /* The square where the king is placed */
    int h;
    int y;
    int row; /* Row where the king is placed */
    int col; /* Col where the king is placed */
    int xside;
    xside = (WHITE + BLACK) - current_side; /* opposite current_side, who may be checking */
    
    /* Find King */
    for (k = 0; k < 64; k++)
        if ( (piece[k] == KINGC || piece[k] == KINGU)   && color[k] == current_side )
            break;
/* Situation of the king */
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
        if (color[y] == xside && (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
            return 1;
        if (piece[y] == EMPTY)
            for (y += 8; y < 64; y += 8)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go left */
    y = k - 1;
    h = k - col;
    if (y >= h)
    {
        if (color[y] == xside && (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
            return 1;
        if (piece[y] == EMPTY)
            for (y--; y >= h; y--)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go right */
    y = k + 1;
    h = k - col + 7;
    if (y <= h)
    {
        if (color[y] == xside && (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
            return 1;
        if (piece[y] == EMPTY)
            for (y++; y <= h; y++)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go up */
    y = k - 8;
    if (y >= 0)
    {
        if (color[y] == xside && (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
            return 1;
        if (piece[y] == EMPTY)
            for (y -= 8; y >= 0; y -= 8)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == ROOKC || piece[y] == ROOKU))
                    return 1;
                if (piece[y] != EMPTY)
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
            if (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == BLACK && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y += 9; y < 64 && COL(y) != 0; y += 9)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    /* go left down */
    y = k + 7;
    if (y < 64 && COL(y) != 7)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == BLACK && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y += 7; y < 64 && COL(y) != 7; y += 7)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;

            }
    }
    /* go left up */
    y = k - 9;
    if (y >= 0 && COL(y) != 7)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == WHITE && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y -= 9; y >= 0 && COL(y) != 7; y -= 9)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;

            }
    }
    /* go right up */
    y = k - 7;
    if (y >= 0 && COL(y) != 0)
    {
        if (color[y] == xside)
        {
            if (piece[y] == KINGC || piece[y] == KINGU || piece[y] == QUEEN || piece[y] == BISHOP)
                return 1;
            if (current_side == WHITE && piece[y] == PAWN)
                return 1;
        }
        if (piece[y] == EMPTY)
            for (y -= 7; y >= 0 && COL(y) != 0; y -= 7)
            {
                if (color[y] == xside && (piece[y] == QUEEN || piece[y] == BISHOP))
                    return 1;
                if (piece[y] != EMPTY)
                    break;
            }
    }
    return 0;
}

int MakeMove(MOVE m)
{
    int r;
    
    hist[hdp].m = m;
    
    /* store in history the piece of the dest square */
    hist[hdp].cap = piece[m.dest];
    
    /* dest piece is the original piece */
    piece[m.dest] = piece[m.from];
    /* The original sqaure becomes empty */
    piece[m.from] = EMPTY;
    
    /* The dest square color is the one of the origin piece */
    color[m.dest] = color[m.from];
    /* The original color becomes empty */
    color[m.from] = EMPTY;
    
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
            piece[m.dest] = ROOKU;
            break;

        case MOVE_TYPE_PROMOTION_TO_BISHOP:
            piece[m.dest] = BISHOP;
            break;

        case MOVE_TYPE_PROMOTION_TO_KNIGHT:
            piece[m.dest] = KNIGHT;
            break;

        default:
            puts("impossible to get here...");
            assert(false);
        }
    }
    
    if (m.type == MOVE_TYPE_CASTLE)
    {
		puts("castle!");
		printf("%d\n", m.from);
		printf("%d\n", m.dest);
		piece[m.dest + 1] = EMPTY;
		color[m.dest + 1] = EMPTY;
		piece[m.from + 1] = ROOKU;
		color[m.from + 1] = WHITE;
	}
    
    /* Update ply and hdp */
    ply++;
    hdp++;
    
    r = !IsInCheck(side);
    side = (WHITE + BLACK) - side; /* After making move, give turn to
* opponent */
    return r;
}

void TakeBack() /* undo what MakeMove did */
{
    side = (WHITE + BLACK) - side;
    hdp--;
    ply--;
    piece[hist[hdp].m.from] = piece[hist[hdp].m.dest];
    piece[hist[hdp].m.dest] = hist[hdp].cap;
    color[hist[hdp].m.from] = side;
    if (hist[hdp].cap != EMPTY)
        color[hist[hdp].m.dest] = (WHITE + BLACK) - side;
    else
        color[hist[hdp].m.dest] = EMPTY;
    if (hist[hdp].m.type >= MOVE_TYPE_PROMOTION_TO_QUEEN) /* Promotion */
        piece[hist[hdp].m.from] = PAWN;
}

/*
****************************************************************************
* Search function - a typical alphabeta, main search function *
* Lack: no any technique for move ordering *
****************************************************************************
*/

int Search(int alpha, int beta, int depth, MOVE * pBestMove)
{
    int i;
    int value;
    int havemove;
int movecnt;

    MOVE moveBuf[200]; /* List of movements */
    MOVE tmpMove;
    
    nodes++; /* visiting a node, count it */
    havemove = 0; /* is there a move available? */
    pBestMove->type = MOVE_TYPE_NONE;
    
    /* Generate and count all moves for current position */
movecnt = Gen(side, moveBuf);

//printf("from and dest: %d, %d\n", tmpMove.from, tmpMove.dest);
//printf("# of moves: %d\n", movecnt);

    /* Once we have all the moves available, we loop through the posible
* moves and apply an alpha-beta search */
    for (i = 0; i < movecnt; ++i)
    {
        if (!MakeMove(moveBuf[i]))
        {
            TakeBack();
            continue;
        }
        havemove = 1;
        
        /* This 'if' takes us to the deep of the position */
        if (depth - 1 > 0) /* If depth is still, continue to search deeper */
{
            value = -Search(-beta, -alpha, depth - 1, &tmpMove);
}
        else /* If no depth left (leaf node), go to evalute that position
and apply the alpha-beta search*/
            {
            value = -Eval();
            //printf("eval:%d\n", value);
}
/* We go to the move that origins the move we're analyzing and
* apply the alpha-beta search */
        TakeBack();
        if (value > alpha)
        {
            /* This move is so good and caused a cutoff */
            if (value >= beta)
            {
                return beta;
}
            alpha = value;
            *pBestMove = moveBuf[i]; /* so far, current move is the best reaction
* for current position */
        }
    }
 
    /* If no legal moves, that is checkmate or stalemate */
    if (!havemove)
    {
        if (IsInCheck(side))
            return -MATE + ply; /* add ply to find the longest path to lose or shortest path to win */
        else
            return 0;
    }
    
    //printf("from and dest: %d, %d\n", tmpMove.from, tmpMove.dest);
    
    return alpha;
}

MOVE ComputerThink(int max_depth)
{
/* It returns the move the computer makes */

    MOVE m;
    int score;
    
/* reset some values before searching */
    ply = 0;
    nodes = 0;
    
/* search now */
    score = Search(-MATE, MATE, max_depth, &m);
    
/* after searching, print results */
    printf("Search result: move = %c%d%c%d; nodes = %d, depth = %d, score = %d\n",
           'a' + COL(m.from),
           8 - ROW(m.from),
           'a' + COL(m.dest),
           8 - ROW(m.dest),
           nodes,
           max_depth,
           score
        );
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
            printf(" +---+---+---+---+---+---+---+---+\n");
            if (i <= 56)
            {
                printf(" %d |", 8 - (((unsigned)i) >> 3));
            }
        }
        if (piece[i] == EMPTY)
            printf("   |"); 
        else
        {
            printf(" %c |", pieceName[piece[i] + (color[i] == WHITE ? 0 : 6)]);
        }
        if ((i & 7) == 7)
            printf("\n");
    }
    printf(" +---+---+---+---+---+---+---+---+\n a b c d e f g h\n");
}

/*
****************************************************************************
* Main program *
****************************************************************************
*/
void main()
{

/* It mainly calls ComputerThink(maxdepth) to the desired ply */

    char s[256];
    int from;
    int dest;
    int i;
    int computer_side;
    int max_depth; /* max depth to search */
    MOVE moveBuf[200];
    int movecnt;
    
    printf("First Chess, by Pham Hong Nguyen\n");
    printf("Help\n d: display board\n MOVE: make a move (e.g. b1c3, a7a8q)\n quit: exit\n\n");
    
    side = WHITE;
    computer_side = BLACK; /* Human is white side */
    max_depth = 1;
    hdp = 0; /* Current move order */
    for (;;)
    {
        if (side == computer_side)
        { /* Computer's turn */
            /* Find out the best move to react the current position */
            MOVE bestMove = ComputerThink(max_depth);
            MakeMove(bestMove);
            continue;
        }
        
        /* get user input */
        printf("fc> ");
        if (scanf("%s", s) == EOF) /* close program */
            return;
        if (!strcmp(s, "d"))
        {
            PrintBoard();
            continue;
        }
        if (!strcmp(s, "quit"))
        {
            printf("Good bye!\n");
            return;
        }
        
        /* maybe the user entered a move? */
        from = s[0] - 'a';
        from += 8 * (8 - (s[1] - '0'));
        dest = s[2] - 'a';
        dest += 8 * (8 - (s[3] - '0'));
        ply = 0;
        movecnt = Gen(side, moveBuf);
        
        /* loop through the moves to see if it's legal */
        for (i = 0; i < movecnt; i++)
        {
            if (moveBuf[i].from == from && moveBuf[i].dest == dest)
            {
                if (piece[from] == PAWN && (dest < 8 || dest > 55))
                { /* Promotion move? */
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
                        puts("promoting to a McGuffin..., I'll give you a queen");
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
}
