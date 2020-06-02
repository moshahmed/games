/*
  $Header: c:/cvs/repo/mosh/cc/games/solitaire.c,v 1.9 2020/06/02 09:56:26 User Exp $
  GPL(C) moshahmed
  Notes:
    Played on a board of size with colored pegs:
    +---+
    |.2.|   DUD  RED   DUD
    |232|   BLUE SPC   GREEN
    |.2.|   DUD  BROWN DUD
    +---+
*/
 
char USAGE[] =
"Pego solitaire - a game of pegs, GPL(C) moshahmed                  \n"
"  Reference: Peg Solitare, chp 11, pp 122-133. Further Mathematical\n"
"     Diversions, Martin Gardner, Penguin Books, 1969.              \n"
"  Usage: pego <gamenumber>, eg. pego 0.                            \n"
"  Playing: Use Arrow keys to kill or move pegs. Press 'q' to quit. \n"
"     To move without killing, use control arrow keys.              \n"
"     To kill a peg, jump over it onto a blank on the other side.   \n"
"     Leave Last peg in center for games 0..6.                      \n"
;


#ifndef __TURBOC__
/* 
  2020-06-02 conio.* from https://github.com/thradams/conio
  vc14> cl solitaire.c conio.c
*/
#include <stdio.h>
#include "conio.h"
#define gotoxy    c_gotoxy
#define textcolor c_textcolor
#define clrscr    c_clrscr
#define cprintf   printf

#define is_extended(c)   (c!=0)

#else
/*
  turboc2: getche(), textcolor(), gotoxy(), clrscr(), cprintf().
  compile> d:\tc\bin\tcc -Id:\tc\include -Ld:\tc\lib pego
*/
#include <conio.h>
#define is_extended(c)   (c==0)
#endif

#define K (2+3+2)
#define forin(i,N) for(i=0;i<N;i++)
#define CASE break;case
#define SPC    '_'
#define DUD    '#'
typedef unsigned char uchar;

int   curx, cury;
uchar pego[K][K];

// ' ' in games[][][] are SPC.
uchar games[][K][K] = {{
    // usual.
    "##444##",
    "##444##",
    "1133322",
    "113 322",
    "1133322",
    "##666##",
    "##666##",
},{    // latin cross
    "##   ##",
    "## 3 ##",
    "  333  ",
    "   3   ",
    "   3   ",
    "##   ##",
    "##   ##",
},{    // greek cross
    "##   ##",
    "## 3 ##",
    "   3   ",
    " 33333 ",
    "   3   ",
    "##   ##",
    "##   ##",
},{    // fireplace
    "##333##",
    "##333##",
    "  333  ",
    "  3 3  ",
    "       ",
    "##   ##",
    "##   ##",
},{    // pyramid
    "##   ##",
    "## 3 ##",
    "  333  ",
    " 33333 ",
    "3333333",
    "##   ##",
    "##   ##",
},{    // lamp
    "## 3 ##",
    "##333##",
    " 33333 ",
    "   3   ",
    "   3   ",
    "##333##",
    "##333##",
},{    // inclined square
    "## 3 ##",
    "##333##",
    "  3333 ",
    "3333333",
    " 33333 ",
    "##333##",
    "## 3 ##",
}
};

int i, j;
int maxgames = sizeof(games)/sizeof(games[0]);

void init(int game)
{
    curx = cury = K/2;

    if( game < 0 || game >= maxgames ){
        printf( USAGE );
        printf("No such game=%d, choose from 0..%d\n", game, maxgames-1);
        exit(1);
    }
    forin(i,K){
        forin(j,K){
            pego[i][j] = games[game][j][i];
            if( pego[i][j] == ' ')  // ' ' in games[][][] are SPC.
                pego[i][j] = SPC;
        }
    }
}

int onboard( int x, int y )
{
    if( x < 0 )   return 0;
    if( y < 0 )   return 0;
    if( x > K-1 ) return 0;
    if( y > K-1 ) return 0;
    if( pego[x][y] == DUD ) return 0;
    return 1;
}

void move(int kill, int dx, int dy )
{
    if( ! onboard( curx+dx, cury+dy ) ) return;
    if( kill ){
        if( ! onboard(curx+dx*2 ,cury+dy*2 ) )    goto justmove;
        if( pego[curx   ][cury      ] == SPC )    goto justmove;
        if( pego[curx+dx][cury+dy   ] == SPC )    goto justmove;
        if( pego[curx+dx*2 ][cury+dy*2 ] != SPC ) goto justmove;

        pego[curx+dx*2 ][cury+dy*2 ] = pego[curx][cury];
        pego[curx+dx][cury+dy   ] = SPC;
        pego[curx   ][cury      ] = SPC;

        curx += dx;
        cury += dy;
    }
  justmove:
    curx += dx;
    cury += dy;
}

void drawpeg(int i, int j, char ch, int putcur )
{
    if( putcur ){
        gotoxy(curx*3+1 +1,cury*2+1);
        return;
    }
    if( ! onboard( i, j ) ) return;
    if( pego[i][j] == SPC   )
        ch = SPC;
    gotoxy(i*3+1,j*2+1);
    textcolor( 8+pego[i][j] );
    if( curx == i && cury == j )
        cprintf("<%c>",ch);
    else
        cprintf(" %c ", ch);
}

void draw()
{
    clrscr();
    forin(i,K)
        forin(j,K)
            drawpeg(i,j,'x',0);
    drawpeg(0,0,0,1);
}

void main(int argc, char* argv[] )
{
    char ic;

    if( argc == 1 || !isdigit( ic=argv[1][0] ) ){
        printf( USAGE );
        exit(1);
    }

    init( ic-'0' );
    draw();

    while( (ic = getche()) != 'q'){
        if( is_extended(ic) ){  // extended char?
            uchar ec = getche();
            switch( ec ){
              case 80: ic = 'd'; CASE 145: ic = 'D';
              CASE 72: ic = 'u'; CASE 141: ic = 'U';
              CASE 75: ic = 'l'; CASE 115: ic = 'L';
              CASE 77: ic = 'r'; CASE 116: ic = 'R';
            }
            // printf("ec=%d, ic=%d, ", ec, ic); continue;
        }
        switch( ic ){
          case 'u': move(1,0,-1); CASE 'U': move(0,0,-1);
          CASE 'd': move(1,0,+1); CASE 'D': move(0,0,+1);
          CASE 'l': move(1,-1,0); CASE 'L': move(0,-1,0);
          CASE 'r': move(1,+1,0); CASE 'R': move(0,+1,0);
        }
        draw();
    }
    clrscr();
}
