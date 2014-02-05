/*
What: Rubiks cube simulator.
$Id: cube.cpp,v 1.8 2013-07-28 05:48:51 a Exp $
AUTHOR: GPL(C) moshahmed/at/gmail
Compiling:
 Turbo C 2.0 for MSDOS/Windows 95/NT. Ported and improved.
     c:\> tcc -Id:\tc\include -Ld:\tc\lib cube.cpp
     c:\> cube.EXE
 Linux: g++: Curses C version for Linux/Unix, compile with:
     $ g++ -Wall cube.cpp -lcurses -ltermcap -o cube
     $ TERM=console cube ruf Q # works.
 Cygwin:
     $ g++ -Wall cube.cpp -lcurses -lncurses -o cube
     $ cube ruf # works.
 Caveats: Don't check for input string overflow.

    +---+                            f1
    |222|                         00 01 02
    |222|                         10 11 12
    |222|                         20 21 22
+---+---+---+---+
|555|000|444|111|       00 01 02  00 01 02  00 01 02    00 01 02
|555|000|444|111|   f4  10 11 12  10 11 12  10 11 12 f2 10 11 12  f5
|555|000|444|111|       20 21 22  20 21 22  20 21 22    20 21 22
+---+---+---+---+
    |333|   |ttt|                 00 01 02
    |333|   |ttt|                 10 11 12
    |333|   |ttt|                 20 21 22
    +---+   +---+                    f3
     555

 ..................................1. Help screen
 .................................16. Help screen
 .................................................
 .................................18. stepcount;
 21: appstr original.             19. apphist[0]
 22: apply (ipos).                20. apphist[0]
 23: messages.
 24: input.
 ===========================================================
*/

const char *USAGE[] = {
  "Rubiks cube simulator, (C) moshahmed/at/gmail  ",
  "---------------------KEYS--------------------- ",
  "l,r,u,d,f,b      : rotate faces.               ",
  "L,R,U,D,arrows   : rolling cube.               ",
  "i,A              : init, autocheck,            ",
  "q,C-c,t,p        : quit, break, test, puzzle   ",
  "a,0..9           : apply string, history       ",
  "<,>,w,W:         : undo,redo, wait,nowait,     ",
  "---------------------STRINGs------------------ ",
  "Action A ::= l|r|u|d|f|b|i|L|R|U|D|A-|AN.      ",
  "            where: A- is A3, N is digits.      ",
  "String S ::= A | N | (S)N                      ",
  "Usage and details c:\\> cube -?                ",
  "---------------------------------------------- ",
  "References:                            HRW=Holt Rinehart & Winston,NY",
  "o Mastering Rubik's cube, Don Taylor, ISBN0-03-059941-5, HRW 1980.   ",
  "o Cube Games, Don Taylor and Leanne Rylands, HRW 1981.               ",
  "o http://ssie.binghamton.edu/~jirif/ptrns.html                       ",
  "o http://jacobi.math.brown.edu/~reid/rubik/patterns.html             ",
  "       Use tolower 'RLUDFB' for reid patterns.                       ",
  "---------------------------------------------- ",
  "Test patterns are read from file ./cube.tst at startup             ",
  "   each line of cube.tst =~ /name=action./,  line =~ s/# comments//  ",
  "   name =~ /bist/ => test cube returns to initial position.          ",
  "---------------------------------------------- ",
  "Other interactive commands:  (C- for control)                      ",
  "   C-UP/C-DOWN to rock/roll history.                                 ",
  "   C-LEFT/C-RIGHT to see test cases read from ./cube.tst             ",
  "   HOME/END - to warp single corner/side.                            ",
  "   BACKSPACE - same as '<' for undo.                                 ",
  "   Q: quit with cube on screen.                                      ",
  "---------------------------------------------- ",
  "Command line:                                                      ",
  "> cube             Use keys as above, t/T to run inbuilt patterns    ",
  "> cube 123 Q       Run testcase #123 and Quit leaving cube on screen ",
  "> cube i(ruf)240   Start with init and apply 'ruf' 240 times.        ",
};

// =========================================================

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define until(C)   while(!(C))
#define unless(C)  if(!(C))
#define CASE       break; case
#define DEFAULT    break; default
#define BACKSPACE  8
#define CONTROL_C  3
#define BLANK      32
#define WHITESPACE "\n\r \t\f"
#define ORANGE LIGHTRED

// =========================================================
// #include "tc2cur.h" // curses => turbo c.

#ifdef __TURBOC__
#include <conio.h>
#include <dos.h>    // for delay.

typedef unsigned char chtype;  // is ulong in curses.h!
enum { FALSE=0, TRUE=1 };
#define endwin  clrscr
#define clear   clrscr
#define initscr clrscr
void   refresh(){ }
void   clrtoeol(){ clreol(); }
void   noecho(){ }
void   echo(){ }
void   cbreak(){ }
void   move(int i,int j){ gotoxy(j+1,i+1); }
void   addch( chtype c){ cprintf("%c",c); }
void   mvaddch(  int i, int j, chtype c ){ move(i,j); addch(c); }
void   addstr(const char*s){ cprintf("%s",s); }
void   mvaddstr( int i, int j, const char*s ){ move(i,j); addstr(s); }
chtype mvgetch(  int i, int j ){ move(i,j); return getche();}
void mvgetstr( int i, int j, char*s ){ move(i,j); scanf("%s",s); }

// #define time(X) 0 -- tcc linker can't find _setdate in module STIME? */

#define ARROW_KEY 0
#define KEY_F1    59
#define KEY_F9    67

#else
#include <ncurses/ncurses.h>

enum { WHITE =0,  BLUE=1,  RED  =2, LIGHTRED=3, YELLOW=4,  GREEN=5, BLACK=6 };
#define DARKGRAY        BLUE     // not there in ncurses?
#define kbhit()         timeout(0)
#define getche()        getch()
#define textcolor(C)       ((void)0) // color_set(C,0)?
#define textbackground(C)  ((void)0) // color_set(,)?
#define delay              (void*)

#endif

enum { BGCOL = 0, FGCOL = 15 };

// end of tc2cur.h

#define BACKSPACE 8
#define CONTROL_C 3
#define ARROW_KEY 0
#define BLANK     32
#define KEY_F1    59
#define KEY_F9    67
#define ORANGE LIGHTRED

#define WHITESPACE "\n\r \t\f"

/* Global Variables:
   cubar    6 faces of the 3x3 cube; face 7 is tmp.
   adjface  Adjacent face matrix.
   apphist  String history.
   history  keyboard history.
*/

int   cubar[7][3][3];

const int  adjface[6][6] =
{ {0,2,4,3,5,1},
  {1,3,4,2,5,0},
  {2,1,4,0,5,3},
  {3,4,1,5,0,2},
  {4,2,1,3,0,5},
  {5,3,1,2,0,4}};

const int MAXLINE  = 256;

long int stepcount=0;
int      wait=0;        // delay in ms.
int      autocheckon=0; // check if done after each move.
int      controlc=0;    // to get out of recursive eg. apply ((ruf)3)3
int      puzzlemode=0;  // hides solution.
char     sprt[MAXLINE]; // for sprintf

// IO

void amessage(const char* message) {
    textcolor(FGCOL);
    mvaddstr(23,0,message );
    clrtoeol();
}

void drawone(int i, int j, int k, int e) {
    // Print a single piece of color e of face i,
    // ie cubar[i][j][k] in color e.

    static int X[6] = {1,1,2,1,0,3};
    static int Y[6] = {7,0,7,14,7,7};
    static int cubecolors[6] = {WHITE, BLUE, ORANGE, RED, YELLOW, GREEN };

    textcolor( cubecolors[e] );
    for( int r=0; r<=1; r++ ){
        move(  1+Y[i]+j*2+r, 1+X[i]*4*2 +2*k );
#ifdef __TURBOC__
        addch( 219 ); addch( 219 );  // two blobs
#else
        // On linux blob is \02, wont work
        // addch( cubecolors[e]+'0' |A_REVERSE| COLOR_PAIR(cubecolors[e]) );
        // addch( cubecolors[e]+'0' |A_REVERSE| COLOR_PAIR(cubecolors[e]) );
        addch( ' ' |A_REVERSE| COLOR_PAIR(cubecolors[e]) );
        addch( ' ' |A_REVERSE| COLOR_PAIR(cubecolors[e]) );
#endif
    }

#ifndef __TURBOC__
    refresh();
#endif
}

void drawcube(void) {
    // drawcube the cube, either in ascii, or in color.

    int j, k;
    for( j=0; j<=2;j++ )
        for( k=0; k<=2;k++ )
            drawone(1,j,k, cubar[adjface[0][1]][j][k] );

    for( j=0; j<=2; j++ ){
        for( k=0; k<=2;k++ ) drawone(4,j,k, cubar[ adjface[0][4] ][j][k] );
        for( k=0; k<=2;k++ ) drawone(0,j,k, cubar[ adjface[0][0] ][j][k] );
        for( k=0; k<=2;k++ ) drawone(2,j,k, cubar[ adjface[0][2] ][j][k] );
        for( k=0; k<=2;k++ ) drawone(5,j,k, cubar[ adjface[0][5] ][j][k] );
    }
    for( j=0; j<=2;j++ )
        for( k=0; k<=2; k++ )
            drawone(3,j,k, cubar[adjface[0][3]][j][k] );
}

// Cube operations

void rotface( int n, int f ) {
    // Just rotate the nine pieces on front face N times,
    // doesnot touch the adjoining pieces.

    int i;
    for( i=1; i<=n ; i++ )
    {
        int t0         = cubar[f][0][0];
        int t1         = cubar[f][0][1];
        cubar[f][0][0] = cubar[f][0][2];
        cubar[f][0][1] = cubar[f][1][2];
        cubar[f][0][2] = cubar[f][2][2];
        cubar[f][1][2] = cubar[f][2][1];
        cubar[f][2][2] = cubar[f][2][0];
        cubar[f][2][1] = cubar[f][1][0];
        cubar[f][2][0] = t0;
        cubar[f][1][0] = t1;
    }
}

void rotrows( int f0, int f1, int f2, int f3 ) {
    // When turning front face, rotate the
    // rows of adjoining faces.

    int t0          = cubar[f0][2][0];
    int t1          = cubar[f0][2][1];
    int t2          = cubar[f0][2][2];
    cubar[f0][2][0] = cubar[f1][0][0];
    cubar[f0][2][1] = cubar[f1][1][0];
    cubar[f0][2][2] = cubar[f1][2][0];
    cubar[f1][0][0] = cubar[f2][0][2];
    cubar[f1][1][0] = cubar[f2][0][1];
    cubar[f1][2][0] = cubar[f2][0][0];
    cubar[f2][0][0] = cubar[f3][0][2];
    cubar[f2][0][1] = cubar[f3][1][2];
    cubar[f2][0][2] = cubar[f3][2][2];
    cubar[f3][0][2] = t2;
    cubar[f3][1][2] = t1;
    cubar[f3][2][2] = t0;
}

void turnface( int n ) {
    // turn front face n times.

    for( int i=1; i<=n; i++ ){
        rotface( 1, 0 );
        rotrows( adjface[0][1], adjface[0][2],
                 adjface[0][3], adjface[0][4] );
    }
}

void copyface( int a, int b ) {
    // Copy nine pieces in face b to face a.
    // Back side is actually flipped when turned up/down (else part).

    for( int i=0; i<=2; i++ )
        for( int j=0; j<=2;j++ )
            if(((a == 1) && (b == 2 || b == 3))  ||
               ((b == 1) && (a == 2 || a == 3) ))
            {
                cubar[a][i][j] = cubar[b][2-i][j];
            }else{
                cubar[a][i][j] = cubar[b][i][j];
            }
}

void rollface( int a, int b, int c, int d) {
    // Roll faces a<-b<-c<-d<-a,
    // Change the orientation of the cube.

    copyface( 6,a ); copyface( a,b );
    copyface( b,c ); copyface( c,d ); copyface( d,6 );
}

#define leftroll  rollface(1,4,0,5); rotface( 1, 2); rotface( 3, 3)
#define rightroll rollface(5,0,4,1); rotface( 3, 2); rotface( 1, 3)
#define uproll    rollface(0,2,1,3); rotface( 3, 5); rotface( 1, 4)
#define downroll  rollface(0,3,1,2); rotface( 3, 4); rotface( 1, 5)

int mode_warpcubeside   = 0;
int mode_warpcubecorner = 0;

void warpcubeside() {
    // rotate f0[01] <- f2[21] <-, see pic at line =~ +---+ above.
    int          t = cubar[0][0][1];
    cubar[0][0][1] = cubar[2][2][1];
    cubar[2][2][1] = t;
    mode_warpcubeside++;
}

void warpcubecorner() {
    // rotate f0[02] <- f2[22] <- f4[00] <-
    int          t = cubar[0][0][2];
    cubar[0][0][2] = cubar[2][2][2];
    cubar[2][2][2] = cubar[4][0][0];
    cubar[4][0][0] = t;
    mode_warpcubecorner++;
}

const int MAXAPPHIST = 12;
char apphist[MAXAPPHIST][MAXLINE];

void showapphist(void) {
    textcolor(FGCOL);
    for( int i=0; i<=4;i++ ){
        sprintf(sprt, "%d='%s'", i, apphist[i] );
        mvaddstr( 16+i,25, sprt );
        clrtoeol();
    }
}

void init(int clearhist) {
    // Called from appundo('i'), who clears history[] also.

    int i;

    clear();
    stepcount = 0;
    puzzlemode = 0;

    for( i=0; i<=5;i++ )
        for( int j=0; j<=2;j++ )
            for( int k=0; k<=2;k++ )
                cubar[i][j][k] = i;

    mode_warpcubeside   = 0;
    mode_warpcubecorner = 0;

    if( clearhist ){
        for( i=0; i<MAXAPPHIST;i++ )
            apphist[i][0] = 0;
    }

    textcolor(FGCOL);
    for( i=0; i< 15; i++ )
        mvaddstr( i,33, USAGE[i] );
    showapphist();
}

int done(void) {
    // Check if cube is in original position.

    for( int i=0; i<=5;i++ )
        for( int j=0; j<=2;j++ )
            for( int k=0; k<=2;k++ )
                if( cubar[i][j][k] != i )
                    return 0;
    return 1;
}

char
waitkey(const char* message ) {
    // wait for user to press a key or control-c
    char ch;
    if( controlc )
        return CONTROL_C;
    amessage( message );
    ch = getche();
    if( ch == CONTROL_C )
        controlc = 1;
    return ch;
}

void
autocheck(void) {
    char ch;

    if( autocheckon && done() && (stepcount > 0 )){
        while(1){
            drawcube();
            ch = waitkey("done!, press c to continue, space to stop.");
            if( ch == CONTROL_C || ch == ' ' ){
                amessage("stopped.");
                controlc = 1;
                break;
            }else if( ch == 'c' ){
                amessage("good.");
                break;
            }
        }
    }
    sprintf(sprt, "step=%02ld", stepcount );

    int warp1 = mode_warpcubecorner % 3;
    int warp2 = mode_warpcubeside % 2;
    if( warp1 || warp2 ){
        strcat( sprt, ", warped:");
        if( warp1 ) strcat( sprt, " corner");
        if( warp2 ) strcat( sprt, " side");
    }
    mvaddstr( 15,25, sprt );  // need more space.
    clrtoeol();
}

const char* VALID_APP = "fbudrlUDLR";

void
apply(char ch) {
    switch( ch ){
      case 'i' : init(0);
      CASE 'f' : turnface(3);
      CASE 'b' : leftroll;  leftroll;  turnface(3); rightroll; rightroll;
      CASE 'u' : uproll;    turnface(3); downroll;
      CASE 'd' : downroll;  turnface(3); uproll;
      CASE 'r' : rightroll; turnface(3); leftroll;
      CASE 'l' : leftroll;  turnface(3); rightroll;
      CASE 'U' : uproll;
      CASE 'D' : downroll;
      CASE 'L' : leftroll;
      CASE 'R' : rightroll;
      DEFAULT  : ;
    }
}

void
appundo(char ch) {
    // apply ch, maintain history, wrap at 80.
    // undo char is '<', init is i.

    static char history[80];
    static int  icol = 22;
    static int  ipos =  0;

    if( ch == 'i' ){  // and fall thru.
        stepcount = 0;
        ipos = sizeof(history)-1;
        while( ipos > 0 )
            history[ipos--] = ' ';
    }

    if( ch == '<' ){
        if( ipos <= 0 ){
            amessage("no history.");
            return;
        }
        --ipos;
        ch = history[ipos];
        for( int i=0; i<3; i++ )
            apply( ch );
        textcolor(CYAN);
        mvaddch( icol,ipos, ch );
        if( strchr( VALID_APP, ch ) )
            stepcount--;
        return;
    }else if( ch == '>' ){
        if( ipos >= 80-1 ){
            amessage("beyond history.");
            return;
        }
        ch = history[ipos];
        apply( ch );
        textcolor(RED);
        mvaddch( icol,ipos, ch );
        if( strchr( VALID_APP, ch ) )
            stepcount++;
        ++ipos;
        return;
    }else{
        apply( ch );
        history[ipos] = ch;
        if( puzzlemode )
            textcolor(DARKGRAY);
        else
            textcolor(FGCOL);
        mvaddch(icol,ipos, ch );
        if( ++ipos >= 80 ){
            ipos = 1;
            move(icol,ipos);
            clrtoeol();
        }
        if( strchr( VALID_APP, ch ) )
            stepcount++;
        autocheck();
    }
}

int
matchparen( const char* astr ) {
    // start at open paren, find pos of closing paren.

    int i=0, level=0;

    assert( astr[i] == '(' );

    for( ; *astr ; i++ ){
        if( astr[i] == '(' )
            level++;
        else if( astr[i] == ')' )
            level--;
        if( level <= 0  ) // all matched?
            return i;
    }

    sprintf(sprt, "closing paren not found '%s'", astr );
    amessage(sprt);
    return 0;
}

void
appstr( const char* dost, int start, int final, int count ) {
    //  applies string =~ ,[rludfb][-0-9]?)*,
    //  "X"         is in     [rludfb].
    //  "N"         is in     [-0-9].
    //  "XN"        applies "X"      N times.
    //  "X-"        applies "X"      3 times.
    //  "(string)N" applies "string" N times.

    char act = ' ', ch;
    int  j, k, sum;

    if( controlc )  // exits from recursive call.
        return;

    if( puzzlemode ){
        // can't set background?
        textbackground(DARKGRAY);
        textcolor(DARKGRAY); // this doesn't work?
    }else{
        textcolor(FGCOL);
        sprintf( sprt, "appstr = %s[%d..%d] ^%d.",
                 dost, start, final, count );
        mvaddstr(21,0, sprt ); clrtoeol();
    }

    while( count-- > 0 ){
        int i = start;
        while( i <= final ){
            ch = dost[i];
            if(    isdigit( ch )
                && i>0
                && strchr( "ifbudrlUDRL", dost[i-1] )
            ){
                sum = ch-'0'-1;
                while( sum-- > 0 )
                    appundo( dost[i-1] );
            }else if( ch == '(' ){
                j   = i + matchparen( dost+i );

                sum = 0;
                for( k=j+1; k<=final && isdigit( dost[k] ); k++ ){
                    sum *= 10;
                    sum += dost[k]-'0';
                }

                if( sum == 0 ){  // eg. (ruf)(fud)0, => (ruf)1(fud)1
                    sum = 1;
                }

                appstr( dost, i+1, j-1, sum );
                i = k;
            }else if( strchr( "ifbudrlUDRL", ch ) ){
                act = ch;
                appundo(act);
            }else if( ch == '-' ){
                appundo(act);
                appundo(act);
            }

            if( ch == 'Q' ){
                clear();
                drawcube();
                endwin();
                exit(1);   // leave screen as is.
            }
#ifdef __TURBOC__
            if( wait > 0 ){
                drawcube();
                delay(wait);
                if( kbhit() && getche() )  // all keys stop.
                    controlc = 1;
            }else if( kbhit() && getche() == CONTROL_C )
                controlc = 1;
#else
            timeout(0);
            ch = getch();
            timeout(-1);
            if( wait > 0 ){
                drawcube();
                if( ch != ERR )
                     controlc = 1;
            }else if( ch == CONTROL_C )
                controlc = 1;
#endif

            if( controlc ){ // Autocheck can also set control c
                amessage("CONTROL_C");
                return;
            }
            i++;
        }
    }
}

void
applystring( const char* dost, char hst ) {
    // apply dost and rock and roll history.
    // if hst == 'p', then puzzlemode, history unaffected.
    // if hst == '-', rock history.
    // if hst == '+', roll history.
    // if hst == '0'..'9', apply history[hst].
    // if hst == 'x', apply dost.
    // if dost is numeric eg "123" then
    //     reapply history[0] with new number.
    //     apply( remove_numeric_suffix( apphist[0] )* 123 ).

    if( isdigit( hst ) ){
        dost = apphist[hst-'0'];
    }else if( hst == 'p' ){
        assert( puzzlemode == 1 );
    }else if( hst == '+' ){
        // roll: apphist[ 0 <- 1 <-   <- MAXAPPHIST-1 <- 0 ].
        char tmphist[MAXLINE];
        strcpy( tmphist, apphist[0] );
        for( int i=0; i<MAXAPPHIST-1; i++ )
            // Should we skip empty lines in apphist? No.
            strcpy( apphist[i], apphist[i+1] );
        strcpy( apphist[MAXAPPHIST-1], tmphist );
        showapphist();
        return;
    }else if( hst == '-' ){
        // rock: apphist[ 0 -> 1 ->   -> MAXAPPHIST-1 -> 0 ].
        char tmphist[MAXLINE];
        strcpy( tmphist, apphist[MAXAPPHIST-1] );
        for( int i=MAXAPPHIST-1; i>0; i--)
            strcpy( apphist[i], apphist[i-1] );
        strcpy( apphist[0], tmphist );
        showapphist();
        return;
    }else if( dost && isdigit( *dost ) ){
        int i = strlen( apphist[0] )-1;
        while( (i > 0 ) && isdigit( apphist[0][i] ) ){
            apphist[0][i] = 0; // remove num at end of st0.
            i--;
        }
        strcat( apphist[0], dost ); // suffix dost to st0.
        dost = apphist[0];
    }else{
        // push dost onto apphist.
        for( int i=MAXAPPHIST-1; i>0; i--)
            strcpy( apphist[i], apphist[i-1] );
        strcpy( apphist[0], dost );
    }

    showapphist();
    appstr( dost, 0, strlen(dost)-1, 1 );
    autocheck();
}

char pick1( const char* choices, int size ) {
    return choices[ rand() % size ];
}

void shufflestr( char * s, int len, int times ) {
    int i, j, k;
    char tm;
    for( i=0; i < times; i++ ){
        j = rand() % len;
        k = rand() % len;
        tm   = s[j];
        s[j] = s[k];
        s[k] = tm;
    }
}

char CHOICES[]    = "rludbf";
char SUBCHOICES[10];

const int maxpuzzlestring = 100;
char  puzzlestring[maxpuzzlestring];

void puzzle( int level ) {
    if( level < 0 ) level = 0;
    sprintf(sprt, "puzzle level=%d", level );
    addstr( sprt );

    // create a string to be applied.
    if( level <= 0 ){
        switch( rand()%3 ){
          case 0: strcpy( SUBCHOICES, "rl" );
          CASE 1: strcpy( SUBCHOICES, "ud" );
          CASE 2: strcpy( SUBCHOICES, "fb" );
        }
    }else{
        strcpy( SUBCHOICES, CHOICES );
        shufflestr( SUBCHOICES, strlen( SUBCHOICES ), 50 );
    }

    int i=0;
    while(  level-- >= 0 && i < (maxpuzzlestring+1) ){
        puzzlestring[i++]   = pick1( SUBCHOICES, strlen(SUBCHOICES));
        if( rand()%2 )
            puzzlestring[i++] = '-';
    }
    puzzlestring[i] = '\0';

    applystring( puzzlestring, 'p' );
}

// =========================================================
const int MAXTESTCASES = 2000;
char* testcases[MAXTESTCASES];
int NUMTESTCASES = 0;

void
readtest( const char* filename ) {
    FILE *fp = fopen(filename, "r" );
    char line[MAXLINE], *p, *q;
    int  linelen;

    unless( fp ){
        sprintf( sprt, "cannot read file=%s.", filename );
        waitkey( sprt );
        return;
    }
    while( 1 ){
        if( NUMTESTCASES >= MAXTESTCASES ){
            sprintf( sprt, "Too many testcases %d >= MAXTESTCASES=%d.",
                    NUMTESTCASES, MAXTESTCASES );
            waitkey(sprt);
            break;
        }

        if( fgets( line, sizeof(line), fp ) == NULL || feof( fp ) )
            break;

        p = strchr( line, '\n' );      // Remove trailing junk spaces.
        while( (p >= line) && strchr( WHITESPACE, *p ) ){
            *p = '\0';
            p--;
        }
        while( (p = strstr(line,"  ")) != NULL ){  // squeeze spaces.
            while( *p ){ *p = *(p+1); p++; }
        }

        // remove all spaces after '='.
        if( (q = strchr( line, '=' )) != NULL )
            while( (q = p = strchr(q,' ')) != NULL )
                while( *p ){ *p = *(p+1); p++; }

        p = strchr( line, '#' );       // remove trailing # comments.
        if( p )
            *p = '\0';

        unless( line[0] )          // skip blank lines/comments.
            continue;

        linelen = strlen( line );
        p = (char*) malloc( linelen+1 );
        unless( p ){
            waitkey("malloc failed");
            break;
        }
        strcpy( p, line );
        testcases[NUMTESTCASES] = p;
        NUMTESTCASES++;
    }
    sprintf(sprt, "read upto testcases[%d] from '%s'",
            NUMTESTCASES, filename );
    amessage(sprt);

    fclose( fp );
    return;
}

void
test(int testnumber, int doit ) {
    int  i;
    char *action;

    if( NUMTESTCASES <= 0 ){
        testcases[0] = (char*) "cube.txt missing/bist=(ruf)240";
        NUMTESTCASES = 1;
    }
    if( testnumber < 0 )
        testnumber = -testnumber;

    i = testnumber % NUMTESTCASES;

    action = strchr( testcases[i], '=' );

    unless( action ){
        sprintf(sprt,"testcases[%d] !~ /comment=action./ = '%s'\n",
                i, testcases[i] );
        amessage(sprt);
        return;
    }

    sprintf( sprt, "test[%d/%d] '%s'",
             i, NUMTESTCASES-1, testcases[i] );
    amessage(sprt);
    drawcube();

    unless( doit )
        return;

    appundo('i');
    applystring( action+1, 'x' );

    sprintf( sprt, "test[%d/%d] '%s'",
             i, NUMTESTCASES-1, testcases[i] );
    if( strstr( testcases[i], "bist" ) )
        strcat( sprt, done()?
                " passed, press i to initialize.":
                "failed, fix it yourself............." );
    amessage( sprt );
    drawcube();
}

int
main(int argc, char* argv[] ) {
    chtype ch;
    int  testnumber = 0;

    // USAGE

    if( argc == 2 && (argv[1][0]=='?' || strcmp( argv[1],"-?")==0 )){
        clear();
        for( const char**u = USAGE; *u; u++ )
            printf("%s\n",*u);
        exit(-1);
    }

#ifdef __TURBOC__
    initscr(); cbreak(); noecho(); refresh();
#else
    initscr(); start_color(); cbreak(); noecho(); refresh();
    nonl(); intrflush(stdscr, FALSE); keypad(stdscr, TRUE);

    if( has_colors() ){
      init_pair( WHITE,   COLOR_WHITE,   COLOR_BLACK );
      init_pair( BLUE,    COLOR_BLUE,    COLOR_BLACK );
      init_pair( ORANGE,  COLOR_RED,     COLOR_BLACK );
      init_pair( RED,     COLOR_MAGENTA, COLOR_BLACK );
      init_pair( YELLOW,  COLOR_YELLOW,  COLOR_BLACK );
      init_pair( GREEN,   COLOR_GREEN,   COLOR_BLACK );
      init_pair( BLACK,   COLOR_BLACK,   COLOR_WHITE );
      printf("has colors\n");
      // one usage: addch( 'X' | A_BLINK | COLOR_PAIR(RED)  );
      // bash$ g++ -Wall cube3.cpp -lncurses -ltermcap
      // bash$ TERM=console a.out
    }else{
      clear(); refresh(); endwin();
      printf("no colors\n"); exit(-1);
    }

#endif
    srand(time(NULL));
    init(1);
    readtest("cube.tst");

    // COMMAND LINE.

    if( argc <= 1 )
        test(testnumber++,1); // do some minimal testing.

    for(int ok=1; ok < argc; ok++ ){
        if( isdigit( argv[ok][0] ) ){        // 123 => test[123].
            test( atoi( argv[ok] ), 1 );
        }else{                               // else apply actions
            applystring( argv[ok], 'x' );
        }
    }

    drawcube();

    // INTERACTIVE.

    ch = ' ';
    do{
        controlc = FALSE;
        textcolor(FGCOL);
        mvaddch(24,0,':');
        clrtoeol();
        mvaddch(24,1,ch);
        ch = mvgetch(24,2);
        clrtoeol();
        amessage("ok.");

        if( ch == BACKSPACE )
            ch = '<';

        if( ch ==  ARROW_KEY ){
            ch = mvgetch(24,2);
            switch( ch ){
              case 75 : ch = 'R';               // Arrow keys
              CASE 77 : ch = 'L';
              CASE 72 : ch = 'D';
              CASE 80 : ch = 'U';
              CASE -111: applystring( 0, '-');  // C-DOWN
                         goto nextcommand;
              CASE -115: applystring( 0, '+');  // C-UP
                         goto nextcommand;
              CASE 71  : warpcubecorner();      // HOME
                         autocheck();
                         goto nextcommand;
              CASE 79  : warpcubeside();        // END.
                         autocheck();
                         goto nextcommand;
              CASE 116 : test(++testnumber,0);  // C-RIGHT
                         goto nextcommand;
              CASE 115 : test(--testnumber,0);  // C-LEFT
                         goto nextcommand;
              DEFAULT : sprintf( sprt, "extended char=%d=0x%02x=%c.",
                                 (int) ch, (int) ch, (int) ch );
                         amessage( sprt );
                         goto nextcommand;
            }
        }

        if( strchr( "ifbudrlUDRL<>", ch ) ){
            appundo(ch);
        }else if( isdigit( ch ) ){
            applystring( "", ch );
        }else if( ch == 'p' ){
            char instr[MAXLINE];
            mvaddstr(24,0,"level:");
            mvgetstr(24,7, instr );
            puzzlemode = 1;
            puzzle( atoi( instr ) );
            move(24,1); clrtoeol();
        }else if( ch == 'a' ){
            char instr[MAXLINE];
            mvaddstr(24,1,"string to apply:");
            echo();
            mvgetstr(24,17, instr );
            noecho();
            applystring( instr, 'x' );
            move(24,1); clrtoeol();
        }else if( ch == 't' ){
            test(testnumber++, 1);
        }else if( ch == 'T' ){
            test(--testnumber, 1);
        }else if( ch == 'A' ){
            autocheckon = !autocheckon;
            sprintf(sprt, "autocheckon=%d", autocheckon );
            amessage(sprt);
        }else if( ch == 'w' ){
            wait += 500;
            sprintf(sprt,"delay=%d, all keys will stop.",wait);
            amessage(sprt);
        }else if( ch == 'W' ){
            wait -= 500;
            if( wait < 0 )
                wait = 0;
            sprintf(sprt,"delay=%d",wait);
            amessage(sprt);
        }else if( ch == 'Q' ){
            // quit, but leave cube on screen.
            applystring("Q", 'x');
        }else if( ch == 'q' || ch == 'Q' || ch == CONTROL_C ){
            break;
        }else if( ch == ' ' ){
            // noop().
        }else{
            sprintf(sprt,"unknown command %c(char=%d)", (int) ch,(int) ch);
            amessage(sprt);
            continue;
        }
      nextcommand:
        drawcube();
    }while( 1 );

    clear(); refresh();
    endwin();
}
// EOF.
