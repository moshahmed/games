/*
  $Header: c:/cvs/repo/mosh/cc/games/conio/solitaire.c,v 1.18 2020/06/23 05:01:11 User Exp $
  GPL(C) moshahmed
  Notes:
    Played on a board of size with colored pegs:
    +---+
    |.2.|   DUD  RED   DUD
    |232|   BLUE SPC   GREEN
    |.2.|   DUD  BROWN DUD
    +---+

vc++:
  2020-06-02 conio.* from https://github.com/thradams/conio
  vc6/vc14> cl solitaire.c conio.c

Turboc2: getche(), textcolor(), gotoxy(), clrscr(), cprintf().
  compile> d:\tc\bin\tcc -Id:\tc\include -Ld:\tc\lib pego
  -include <conio.h>
  -define gotoxy    c_gotoxy
  -define textcolor c_textcolor
  -define cprintf   printf
  -define clrscr    c_clrscr

*/

char USAGE[] =
"$Header: c:/cvs/repo/mosh/cc/games/conio/solitaire.c,v 1.18 2020/06/23 05:01:11 User Exp $\n"
"Pego solitaire - a game of pegs, GPL(C) moshahmed                  \n"
"  Reference: Peg Solitare, chp 11, pp 122-133. Further Mathematical\n"
"     Diversions, by Martin Gardner, Penguin Books, 1969.           \n"
"  Usage: solitaire gamenumber, eg. solitaire 0  .. Start game 0    \n"
"    solitaire solitaire.sol 0 33      .. Replay file lines 0..33   \n"
"  Writes solitaire.log                                             \n"
"  Playing: Use Arrow keys to kill or move pegs. Press 'q' to quit. \n"
"     To move without killing: use Control-arrow-keys.              \n"
"     To undo/redo history: use 'Backspace' '<' or '>'              \n"
"     To kill a peg: jump over it onto a blank on the other side.   \n"
"     Leave Last peg in center for games numbers 0..6.              \n"
;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>
#include "conio.h"

#define K (2+3+2)
#define forin(ii,N) for(ii=0;ii<N;ii++)
#define CASE break;case

#define SPC    '_'
#define DUD    '#'

#define BACKSPACE 8

typedef unsigned char uchar;

int gamenumber=0;
int   curx, cury;
uchar pego[K][K];

int replay_delayms=200;
int debug=1;

const char *game_names[] = {
    "Usual",
    "Latin cross",
    "Greek cross",
    "Fireplace",
    "Pyramid",
    "Lamp",
    "Inclined square"
  };

uchar games[][K][K] = {
  /* ' ' in games[][][] are SPC. */
  {
      /* usual. */
      "##444##",
      "##444##",
      "1133322",
      "113 322",
      "1133322",
      "##666##",
      "##666##",
  },{    /* latin cross */
      "##   ##",
      "## 1 ##",
      "  131  ",
      "   1   ",
      "   4   ",
      "##   ##",
      "##   ##",
  },{    /* greek cross */
      "##   ##",
      "## 3 ##",
      "   2   ",
      " 32323 ",
      "   2   ",
      "##   ##",
      "##   ##",
  },{    /* fireplace */
      "##444##",
      "##444##",
      "  333  ",
      "  3 3  ",
      "       ",
      "##   ##",
      "##   ##",
  },{    /* pyramid */
      "##   ##",
      "## 4 ##",
      "  333  ",
      " 13332 ",
      "1133322",
      "##   ##",
      "##   ##",
  },{    /* lamp */
      "## 4 ##",
      "##444##",
      " 13332 ",
      "   3   ",
      "   3   ",
      "##666##",
      "##666##",
  },{    /* inclined square */
      "## 4 ##",
      "##444##",
      "  3332 ",
      "1133322",
      " 13332 ",
      "##666##",
      "## 6 ##",
  }
};

const int MAXGAMES = sizeof(games)/sizeof(games[0]);
const char *logfile="solitaire.log";
FILE *logger = NULL;

#define HISTMAX 1000
typedef struct {
  uchar pego[HISTMAX][K][K];
  int at, have, curx[HISTMAX], cury[HISTMAX];
} HIST;
HIST hist;

char *ymd_now() {
    time_t t;
    struct tm *info;
    static char date_str[60];

    time(&t);
    info = localtime(&t);
    strftime(date_str, sizeof(date_str)-1, "%Y-%m-%d %H:%M:%S",info);
    return date_str;
}

void init(int game) {
    int ii, jj;

    curx = cury = K/2;

    if( game < 0 || game >= MAXGAMES ){
        int gn;
        printf( USAGE );
        printf("No such gamenumber=%d, choose from 0..%d\n", game, MAXGAMES-1);
        for(gn=0; gn < MAXGAMES; gn++){
          printf("  gamenumber[%d]=\"%s\"\n", gn, game_names[gn]);
        }
        exit(1);
    }

    logger = fopen(logfile, "a+" );
    if(logger)
      fprintf(logger,"{ Begin solitaire gamenumber=%d at %s\n", game, ymd_now());

    forin(ii,K){
        forin(jj,K){
            pego[ii][jj] = games[game][jj][ii];
            if( pego[ii][jj] == ' ')  /* ' ' in games[][][] are SPC. */
                pego[ii][jj] = SPC;
        }
    }
    /* start history */
    memset(hist.pego,SPC,sizeof(hist.pego));
    hist.at = hist.have= 1;
    hist.curx[hist.at] = curx;
    hist.cury[hist.at] = cury;
    forin(ii,K)
      forin(jj,K)
        hist.pego[hist.at][ii][jj]=pego[ii][jj];
}

void move_hist(int dir){
  int ii, jj;
  assert(dir==1 || dir==-1);
  if ((hist.at+dir) <= 0 || ((hist.at+dir) >= hist.have)){
    return;
  }
  hist.at += dir;
  curx = hist.curx[hist.at];
  cury = hist.cury[hist.at];
  forin(ii,K)
    forin(jj,K){
      pego[ii][jj]=hist.pego[hist.at][ii][jj];
    }
}

void hist_save(){
  int ii, jj;
  if ((hist.at+1) >= HISTMAX){
    printf("history overflow, more than %d steps",  HISTMAX);
    return;
  }
  hist.at++;
  hist.have=hist.at;
  hist.curx[hist.at] = curx;
  hist.cury[hist.at] = cury;
  forin(ii,K)
    forin(jj,K)
      hist.pego[hist.at][ii][jj]=pego[ii][jj];
}

int off_board( int x, int y ) {
    if( x < 0 )   return 1;
    if( y < 0 )   return 2;
    if( x > K-1 ) return 3;
    if( y > K-1 ) return 4;
    if( pego[x][y] == DUD )
      return 5;
    return 0;
}

void move(int kill, int dx, int dy ) {
    if( off_board( curx+dx, cury+dy ) ) return;
    if( kill ){
        if( off_board(curx+dx*2 ,cury+dy*2 ) )    goto justmove;
        if( pego[curx   ][cury      ] == SPC )    goto justmove;
        if( pego[curx+dx][cury+dy   ] == SPC )    goto justmove;
        if( pego[curx+dx*2 ][cury+dy*2 ] != SPC ) goto justmove;

        if(logger)
          fprintf(logger, "  %d %d > %d %d\n",
            curx, cury, curx+dx*2, cury+dy*2);

        pego[curx+dx*2 ][cury+dy*2 ] = pego[curx][cury];
        pego[curx+dx][cury+dy   ] = SPC;
        pego[curx   ][cury      ] = SPC;

        curx += dx;
        cury += dy;

    }
    hist_save();
  justmove:
    curx += dx;
    cury += dy;
}

void drawpeg(int ii, int jj, char ch, int putcur ) {
    int outside_square;
    if( putcur ){
        c_gotoxy(curx*3+1 +1,cury*2+1);
        return;
    }
    outside_square = off_board( ii, jj );
    if( 1 <= outside_square  && outside_square <= 4)
      return;
    if( pego[ii][jj] == SPC   )
        ch = SPC;
    else if( pego[ii][jj] == DUD )
        ch = '.';
    c_gotoxy(ii*3+1,jj*2+1);
    c_textcolor( 8+pego[ii][jj] );
    c_textbackground(0);
    if( curx == ii && cury == jj )
        printf("<%c>",ch);
    else
        printf(" %c ", ch);
}

void draw() {
    int ii, jj;
    c_clrscr();
    forin(ii,K)
        forin(jj,K)
            drawpeg(ii,jj,'x',0);
    drawpeg(0,0,0,1);
}

void replay_file(char* replay_file, int from_line, int to_line){
    char *gni;
    int ln;
    FILE *rpf;
    #define MAXLINE 1000
    char    aline[MAXLINE];

    printf("replay_file %s from lines %d..%d\n", replay_file, from_line, to_line);
    if (from_line < 0 || to_line < 0 || from_line > to_line){
        printf("Invalid line range %d to %d\n", from_line, to_line);
        exit(-1);
    }

    rpf = fopen(replay_file, "r" );
    if(!rpf){
        printf("Cannot read replay_file=%s\n", replay_file);
        printf("Usage: solitaire replay_file from_line to_line\n");
        exit(-1);
    }
    for(ln=0; ln <= to_line; ln++){
      if (!fgets( aline, sizeof(aline)-1, rpf )){
        printf("Cannot read replay_file=%s at line=%d\n", replay_file, ln);
        exit(-1);
      }
      if (ln < from_line)
        continue;
      /* parse and process line */
      gni = strstr(aline,"gamenumber=");
      if(gni){
        gamenumber=atoi(gni+strlen("gamenumber="));
        init( gamenumber );
        draw();
        _sleep(replay_delayms);
        continue;
      }
      if (strstr(aline," End "))
        break;
      if(strstr(aline,">")){
        int nextx, nexty, dx, dy, kill;
        if ( sscanf(aline, "  %d %d > %d %d", &curx, &cury,  &nextx, &nexty) != 4 ){
          printf("Cannot parse replay_file=%s:%d line=%s\n", replay_file, ln, aline);
          exit(1);
        }
        if (off_board(curx,cury) || off_board(nextx,nexty)){
          printf("replay_file=%s:%d:%s invalid move %d %d > %d %d\n",
            replay_file, ln, aline, curx,cury, nextx,nexty);
          exit(1);
        }
        dx = (nextx-curx)/2;
        dy = (nexty-cury)/2;

        move(kill=0, 0,  0);
        draw();
        _sleep(replay_delayms);

        move(kill=1,dx, dy);
        draw();
        _sleep(replay_delayms);
      }
      if (kbhit()) {
        fprintf(logger, "; keypressed stopping at step %d\n",ln);
        break;
      }
    }
}

int main(int argc, char* argv[] ) {
    uchar ic;
    int gn;

    if (argc>1 && isdigit(argv[1][0])){
        /* e.g. solitaire 1 */
        gamenumber = atoi(argv[1]);
        init( gamenumber );
        draw();
    } else if (argc>3){
        /* e.g. solitaire 2replay_file 3from_line 4to_line */
        replay_file( argv[1], atoi(argv[2]), atoi(argv[3]));
    } else {
        /* no args? */
        printf( USAGE );
        exit(1);
    }

    while( (ic = getche()) != 'q'){
        #ifdef _MSC_VER
        if( ic==224 ){ /* vc win32 console keycodes */
            uchar ec = getche();
            switch( ec ){
              /* Arrow-keys  and Control-arrow-keys */
              case 80: ic = 'd'; CASE 145: ic = 'D';
              CASE 72: ic = 'u'; CASE 141: ic = 'U';
              CASE 75: ic = 'l'; CASE 115: ic = 'L';
              CASE 77: ic = 'r'; CASE 116: ic = 'R';
            }
        }
        #endif
        switch( ic ){
          case 'u': move(1,0,-1); CASE 'U': move(0,0,-1);
          CASE 'd': move(1,0,+1); CASE 'D': move(0,0,+1);
          CASE 'l': move(1,-1,0); CASE 'L': move(0,-1,0);
          CASE 'r': move(1,+1,0); CASE 'R': move(0,+1,0);
          /* Replay history with [\b<.,>] */
          CASE '<': case ',': move_hist(-1);
          CASE '>': case '.': move_hist(+1);
          CASE  BACKSPACE : move_hist(-1);
        }
        draw();
    }

    if(logger){
      fprintf(logger,"} End at %s\n", ymd_now());
      fclose(logger);
    }
    c_clrscr();
    return 0;
}
