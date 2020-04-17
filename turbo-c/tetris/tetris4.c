/*  vim:path+=c\:/tc2/include:
    SYNOPSIS: Tetris -- a game to pack falling blocks.
    Source available at moshahmed
    GPL(C) moshahmed
    $Header: c:/cvs/repo/mosh/cc/games/tetris4.c,v 1.21 2018/04/08 20:56:35 User Exp $

    Compile with Turbo C.2 on Windows console/MSDOS:
        set TC=c:\tc2
        PATH=%PATH%;%TC%\bin
        tcc -I%TC%\include -L%TC%\lib tetris

    Compile with curses on Linux console:
        gcc -Wall tetris4.c -lcurses -ltermcap -o tetris4

    Cygwin: no curses?
    Mingw/Codebblock/win7, 2015-02-27:
        gcc -Wall -DNCURSES tetris4.c -lncursesw  -o tetris4 2>&1 | vi -

    Also see:
        DOS Programming FAQ http://www.bsdg.org/SWAG/FAQ/index.html
        kbhit http://www.bsdg.org/SWAG/FAQ/0002.PAS.html
        turboC to GCC http://www.sandroid.org/TurboC/functionlist.html

    Data Structures:
        board[][] - board on which the game is played.
        widget - shape of blocks that fall, directly looked up by indexes.
        fbox[] - widget falling from the top, each point into widget.
*/

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef unsigned char uchar;
typedef unsigned short ushort;

#define forin(I,N)  for(I=0;I<N;I++)
#define until(C) while(!(C))
enum { FALSE=0, TRUE=1, };
enum { BGCOL  = 0, FGCOL = 15, };

#ifdef __TURBOC__
//tmp// #include <conio.h>
//tmp// // #include <fnkeys.h> // found by yahoo.
//tmp// typedef unsigned char chtype;
//tmp//
//tmp// #define endwin  clrscr
//tmp// #define clear   clrscr
//tmp// #define initscr clrscr
//tmp// #define keypressed kbhit
//tmp//
//tmp// void   refresh(){ }
//tmp// void   clrtoeol(){ clreol(); }
//tmp// void   noecho(){ }
//tmp// void   cbreak(){ }
//tmp//
//tmp// void   move(int i,int j){ gotoxy(j+1,i+1); }
//tmp// void   setcolors(int fg,int bg){ textcolor(fg); textbackground(bg); }
//tmp//
//tmp// void   addch( chtype c){ cprintf("%c",c); }
//tmp// void   addstr(chtype*s){ setcolors(FGCOL,BGCOL); cprintf("%s",s); }
//tmp// void   mvaddstr( int i, int j, chtype*s ){ move(i,j); addstr(s); }
//tmp// chtype mvgetch(  int i, int j   ){ move(i,j); return getche();}
//tmp//
#elif  _MSC_VER
// c:\vc6\vc98\include\rpcasync.h(45) : warning C4115: '_RPC_ASYNC_STATE' : named type definition in parentheses
#pragma warning(disable:4115)
#include <windows.h>
#include <conio.h>
  #define chtype char

  void noop(){}
  #define cbreak noop
  #define noecho noop
  void refresh() { Sleep(100); }

  static HANDLE hStdout, hStdin;
  void initscr() {
    hStdout = GetStdHandle ( STD_OUTPUT_HANDLE );
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if ( INVALID_HANDLE_VALUE == hStdout ||
        INVALID_HANDLE_VALUE == hStdin) {
      printf("INVALID_HANDLE_VALUE\n");
      exit(1);
    }
    SetConsoleTitle("Mastermind (VC++) $Id: tetris4.c,v 1.21 2018/04/08 20:56:35 User Exp $");
  }

  void gotoxy(int x, int y) {
    COORD pos;
    pos.X = (short) y; // swap x y.
    pos.Y = (short) x;
    SetConsoleCursorPosition ( hStdout, pos );
  }
  void move(int x, int y) {
    gotoxy(x,y);
  }

  void clrtoeol() {
    int i=80;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    GetConsoleScreenBufferInfo( hStdout, &csbi );
    for(i=csbi.dwCursorPosition.X; i<csbi.srWindow.Right;i++) {
      printf(" ");
    }
  }

  // From microsoft support.
  /* Standard error macro for reporting API errors */
  #define PERR(ok, api){if(!(ok)) \
    printf("%s:Error %d from %s on line %d\n", \
      __FILE__, GetLastError(), api, __LINE__);}

  void clear() {
    // was: int i=40; while(i-->0){ gotoxy(i,0); clrtoeol(); }
    COORD home = { 0, 0 };  /* here's where we'll home the cursor */
    BOOL ok;
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    DWORD dwConSize; /* number of character cells in the current buffer */

    /* Get the number of character cells in the current buffer */
    ok = GetConsoleScreenBufferInfo( hStdout, &csbi );
    PERR( ok, "GetConsoleScreenBufferInfo" );
    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* Fill the entire screen with blanks */
    ok = FillConsoleOutputCharacter( hStdout, (TCHAR) ' ',
      dwConSize, home, &cCharsWritten );
    PERR( ok, "FillConsoleOutputCharacter" );

    /* Get the current text attribute */
    ok = GetConsoleScreenBufferInfo( hStdout, &csbi );
    PERR( ok, "ConsoleScreenBufferInfo" );

    /* Now set the buffer's attributes accordingly */
    ok = FillConsoleOutputAttribute( hStdout, csbi.wAttributes,
      dwConSize, home, &cCharsWritten );
    PERR( ok, "FillConsoleOutputAttribute" );

    /* Put the cursor at (0, 0) */
    ok = SetConsoleCursorPosition( hStdout, home );
    PERR( ok, "SetConsoleCursorPosition" );
    return;
  }

  void endwin(void) {
    // printf("\033[0m\033[2J\033[H");
    clear();
  }


  void mvaddstr( int i, int j, chtype*s ){
    gotoxy(i,j);
    printf("%s", s);
  }


// For console colors see C:/vc6/vc98/include/WINCON.H
#define F_RED      FOREGROUND_RED
#define F_BLUE     FOREGROUND_BLUE
#define F_GREEN    FOREGROUND_GREEN
#define B_RED      BACKGROUND_RED
#define B_BLUE     BACKGROUND_BLUE
#define B_GREEN    BACKGROUND_GREEN
#define F_BRIGHT   FOREGROUND_INTENSITY
#define B_BRIGHT   BACKGROUND_INTENSITY
#define F_YELLOW   FOREGROUND_RED|FOREGROUND_GREEN
#define F_MAGENTA  FOREGROUND_RED|FOREGROUND_BLUE
#define F_CYAN     FOREGROUND_BLUE|FOREGROUND_GREEN
#define F_WHITE    FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE

#define tcolor(c) SetConsoleTextAttribute(hStdout,c);

  void whiteOnBlack(void) { tcolor(F_WHITE); }

  void charcolor(char c) {
    whiteOnBlack();
    switch(c) {
      case 'W': break;
      case 'B': tcolor(F_BRIGHT); break;
      case '1': tcolor(F_RED|F_BRIGHT); break;
      case '2': tcolor(F_GREEN|F_BRIGHT); break;
      case '3': tcolor(F_YELLOW|F_BRIGHT); break;
      case '4': tcolor(F_BLUE|F_BRIGHT); break;
      case '5': tcolor(F_CYAN|F_BRIGHT); break;
      case '6': tcolor(F_MAGENTA|F_BRIGHT); break;
      case '7': tcolor(F_RED); break;
      case '8': tcolor(F_GREEN); break;
      case '9': tcolor(F_BLUE); break;
      case '0': tcolor(F_BLUE); break;
      default: ;
    }
    printf("%c", c);
    whiteOnBlack();
  }

  void mvaddch(  int i, int j, chtype c ){
    gotoxy(i,j);
    charcolor(c);
  }
  void mvaddint(  int i, int j, int n ){
    gotoxy(i,j);
    printf("%d", n);
  }
  void addstr(chtype*s){
    printf("%s",s);
  }
  void addint(chtype*f, int n){
    printf(f,n);
  }
  void addch( int c){
    printf("%c", c);
  }
  int /* chtype? */ getch() {
    // From http://www.adrianxw.dk/SoftwareSite/Consoles/Consoles5.html
    INPUT_RECORD InRec;
    DWORD NumRead;
    DWORD EventCount;

    Sleep(100);
    GetNumberOfConsoleInputEvents(hStdin, &EventCount);
    while (EventCount > 0) {
      ReadConsoleInput(hStdin, &InRec,1,&NumRead);
      if (InRec.EventType == KEY_EVENT && InRec.Event.KeyEvent.bKeyDown) {
        return InRec.Event.KeyEvent.uChar.AsciiChar;
      }
      GetNumberOfConsoleInputEvents(hStdin, &EventCount);
    }
    return '@'; // bad_char
  }
  chtype mvgetch(int i, int j){
    gotoxy(i,j);
    return (chtype) getch();
  }

#define textcolor(X) tcolor(X)
#define textbackground(X) 1/* todo.vc6 */
#define _setcursortype(X) 1/* todo.vc6
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.bVisible = visible;
    cursorInfo.dwSize = (size < 100) ? size : 100;
    SetConsoleCursorInfo(cConsole, &cursorInfo);
*/
#define delay(X) Sleep(X)
// #define sound(X) assert(!"use Beep")
// #define nosound() assert(!"use Beep")
#define clrscr()  clear()
#define keypressed()  _kbhit()

#elif defined(NCURSES)
#include <ncursesw/curses.h>
// enum { // Turbo C colors from <conio.h>
//     BLUE   = '1',  GREEN  = '2',  CYAN      = '3', RED    = '4',
//     MAGENTA= '5',  BROWN  = '6',  LIGHTGRAY = '7', DARKGRAY='8',
//     LIGHTBLUE=9, LIGHTGREEN=10, LIGHTCYAN=11, LIGHTRED=12,
//     LIGHTMAGENTA=13, YELLOW=14, WHITE=15,
// };
// TODO for ncurses.
// #define textcolor(X) 1/* todo.curses */
// #define textbackground(X) 1/* todo.curses */
// #define _setcursortype(X) 1/* todo.curses */
// #define sound(X) 1/*todo.curses*/
// #define delay(X) 1/*todo.curses*/
// // #define clear()  1/*todo.curses*/
// #define clrscr  clear
// #define nosound() /*todo.curses*/
// #define keypressed()  1/*todo.curses*/
// #define endwin()  1/*todo.curses*/
#endif

enum {
    // Overriding Turbo c colors in <conio.h>
    NONE   = '-',  BLANK  = ' ',  BACKSPACE =  8 , CURSOR = '_',
    BLACK=0,BLUE=1,GREEN=2,CYAN=3,RED=4,MAGENTA=5,BROWN=6,LIGHTGRAY=7,
    DARKGRAY=8,LIGHTBLUE=9,LIGHTGREEN=10,LIGHTCYAN=11,LIGHTRED=12,
    LIGHTMAGENTA=13,YELLOW=14,WHITE=15,
};

// hide cursor
void no_curor(void){
    textcolor(LIGHTGRAY);
    textbackground(LIGHTGRAY);
    _setcursortype(_NOCURSOR);
}

// is the char on board a part of fbox?
int is_solid(uchar cc){
    if( isalnum(cc) )
        return 1;
    return 0;
}

// draw a colored blob on the board.
void mvaddch2(  int i, int j, chtype c ){
    if(is_solid(c)){
        textcolor( (ushort) ((( c-'0')+7)%15+1));
        textbackground(LIGHTGRAY);
        c = (uchar) 219; // Comment this line to see fbox contents.
    }else if( c != '.' ){
        textcolor(LIGHTGRAY);
        textbackground(DARKGRAY);
    }else{
        textcolor(LIGHTGRAY);
        textbackground(DARKGRAY);
        c = (uchar) 219;
    }
    move(i,j);
    addch(c);
}

// sound - landing noise
void thump(void){
    int i;
    for(i=1;i<2;i++){
        int freq=20+i*3;
        #if defined(_MSC_VER)
        Beep(freq,1);
        #else
        sound(20+i*3);
        delay(1);
        nosound();
        #endif
    }
    delay(400);
}

// sound - clock tick.
void click(void){
    int i;
    for(i=1;i<2;i++){
        int freq=5000+2*i;
        #if defined(_MSC_VER)
        Beep(freq,1);
        #else
        sound(5000+2*i);
        delay(1);
        nosound();
        #endif
    }
}

// random number from a to b inclusive.
int randu(int a, int b){
    int r;
    if (a>=b)
        return a;
    r =  a + (abs(rand())%(b-a+1));
    return r;
}

//
// The Screen coordinates (row numbers and column numbers):
//
// 1    | x   x | r1 2   Messages
//      | x  xx |
//      + board + r2 6
//      |      x|        Time
//      |xxxx xx|        Speed
// RMAX +-------+ r3 22  Score
//      c1      c2       c3
//      20      52       60
//      1       CMAX
//
// The board is from [r1..r3 or 1..RMAX][c1..c2 or 1..CMAX].
//   index 0 is unused to be consistent with screen coordinates.

#define CMAX 32
#define RMAX 24
struct board {
    int colors[RMAX+1][CMAX+1];
} board;

// Screen offsets to draw the board.
int c1=1, c2=32, c3=34;
int r1=1, r2=5, r3=RMAX;

void clear_board(void){
    int r,c;
    memset(&board,'.',sizeof(board));
    for(r=1;r<=RMAX;r++){
        board.colors[r][1] = '|';
        board.colors[r][c2] = '|';
    }
    for(c=1;c<=c2;c++){
        board.colors[r3][c] = '-';
    }
    #ifndef NDEBUG
    board.colors[   1][   1] = '+';
    board.colors[RMAX][   1] = '+';
    board.colors[RMAX][  c2] = '+';
    board.colors[   1][  c2] = '+';
    #endif
}

void draw_board(void){
    int r,c;
    for(r=1;r<=RMAX;r++)
        for(c=1;c<=c2;c++)
            mvaddch2( r1+r-1, c1+c-1, (char) board.colors[r][c] );
}

// Different widget shape(s) are numbered and shaped.
// Make the shapes with is_solid() chars as below:
// 1,  22,  333,  4 ,  5 , 6  ,
//                44, 555, 666,

// widget is a 4-Dim array indexed by [shape][rotation][r][c].

uchar widget[][4][4][4] = {
 {{ "1...",
    "....",
    "....",
    "....", },
  { "1...",
    "....",
    "....",
    "....", },
  { "1...",
    "....",
    "....",
    "....", },
  { "1...",
    "....",
    "....",
    "....", }},
  {{"24..",
    "....",
    "....",
    "....", },
  { "4...",
    "2...",
    "....",
    "....", },
  { "42..",
    "....",
    "....",
    "....", },
  { "2...",
    "4...",
    "....",
    "....", }},
 {{ "334.",
    "....",
    "....",
    "....", },
  { "4...",
    "3...",
    "3...",
    "....", },
  { "433.",
    "....",
    "....",
    "....", },
  { "3...",
    "3...",
    "4...",
    "....", }},
 {{ "4...",
    "44..",
    "....",
    "....", },
  { ".4..",
    "44..",
    "....",
    "....", },
  { "44..",
    ".4..",
    "....",
    "....", },
  { "44..",
    "4...",
    "....",
    "....", }},
 {{ ".4..",
    "555.",
    "....",
    "....", },
  { ".5..",
    "45..",
    ".5..",
    "....", },
  { "555.",
    ".4..",
    "....",
    "....", },
  { "5...",
    "54..",
    "5...",
    "....", }},
  {{"4...",
    "666.",
    "....",
    "....", },
  { ".6..",
    ".6..",
    "46..",
    "....", },
  { "666.",
    "..4.",
    "....",
    "....", },
  { "64..",
    "6...",
    "6...",
    "....", }},
 {{ "777.",
    "4...",
    "....",
    "....", },
  { "7...",
    "7...",
    "74..",
    "....", },
  { "....",
    "..4.",
    "777.",
    "....", },
  { "47..",
    ".7..",
    ".7..",
    "....", }},
 {{ "8888",
    "77..",
    "....",
    "....", },
  { "8...",
    "8...",
    "87..",
    "87..", },
  { "..77",
    "8888",
    "....",
    "....", },
  { "..78",
    "..78",
    "...8",
    "...8", }},
};

#define MAX_SHAPES (sizeof(widget)/sizeof(widget[0]))
#define MAX_ROTATIONS (sizeof(widget[0])/sizeof(widget[0][0]))
#define MAX_WIDGET_WIDTH (int) (sizeof(widget[0][0])/sizeof(widget[0][0][0]))
#define MAX_WIDGET_HEIGHT (sizeof(widget[0][0][0])/sizeof(widget[0][0][0][0]))

#define MAX_FBOXES 16
struct fbox {
    int shape,rotation,r,c;
    int steps_moved, auto_moved, key_moved;
} fbox[MAX_FBOXES];

int fbox_in=MAX_FBOXES; // how many fboxes over the board.
int fbox_moving=0; // which of the fbox is moving down?

// create random fboxes over the board.
void fbox_make(int w){
    fbox[w].shape = randu(0,MAX_SHAPES-1);
    fbox[w].rotation = rand() % MAX_ROTATIONS;
    fbox[w].r = r1;
    fbox[w].c = 1 + c1 + (1+MAX_WIDGET_WIDTH)*w;
    fbox[w].steps_moved=0;
    fbox[w].auto_moved=0;
    fbox[w].key_moved=0;
}

// init the fboxes over the board.
void fbox_setup(void){
    int w;
    memset( fbox,0,sizeof(fbox));
    fbox_moving=0;
    for(w=0;w<MAX_FBOXES;w++){
        fbox_make(w);
        if( fbox[w].c + MAX_WIDGET_WIDTH < c2 ) // inside the box?
            fbox_in=w;
    }
}

int fbox_near_top(int w){
    return fbox[w].key_moved == 0 &&
          fbox[w].auto_moved < 2;
}

// draw all the fbox[] over the the board.
void fbox_draw(void){
    int w,r,c;
    for(w=0;w<fbox_in;w++){
        for(r=0;r<MAX_WIDGET_HEIGHT;r++){
            for(c=0;c<MAX_WIDGET_WIDTH;c++){
                uchar cc = widget[ fbox[w].shape ][ fbox[w].rotation ][r][c];
                if( is_solid(cc) )
                    mvaddch2( fbox[w].r+r, fbox[w].c+c, cc );
            }
        }
    }
}

// is the location on the board?
int in_board(int nr, int nc){
    return ( nr > 1 && nr < RMAX && nc > 1 && nc < c2 );
}

// is the board piece occupied?
int board_free(int nr, int nc ){
    return in_board(nr,nc) && !is_solid( (uchar) board.colors[nr][nc] );
}

// is the board free for the fbox[w] to move by dr,dc,drot?
int board_fbox_free( int w, int dr, int dc, int drot ){
    int r,c;
    int rot = (fbox[w].rotation + drot) % MAX_ROTATIONS;
    for(r=0;r<MAX_WIDGET_HEIGHT;r++){
        for(c=0;c<MAX_WIDGET_WIDTH;c++){
            uchar cc = widget[ fbox[w].shape ][ rot ][r][c];
            if( is_solid(cc) && !board_free( fbox[w].r + r + dr, fbox[w].c + c + dc) )
                return 0;
        }
    }
    return 1;
}

// find how many top rows of the board are empty.
int num_rows_empty_board(void){
    int r,c,empty=0;
    for(r=2;r<RMAX;r++){
        for(c=2;c<c2-1;c++){
            assert( in_board(r,c) );
            if( is_solid((uchar) board.colors[r][c]) )
                return empty;
        }
        empty++;
    }
    return empty;
}

// find how many contiguous bottom rows are full.
int num_bottom_rows_filled_board(void){
    int r,c,filled=0;
    for(r=RMAX-1;r>1;r--){
        for(c=2;c<c2-1;c++){
            assert( in_board(r,c) );
            if( !is_solid( (uchar) board.colors[r][c]) )
                return filled;
        }
        filled++;
    }
    return filled;
}

// move the board rows[botr and above] down by dr rows.
void clear_filled_rows_board(int dr, int botr){
    int r,c;
    assert(dr>0);
    assert(botr<=RMAX-1);  // botr was RMAX-1
    for(r=botr-dr;r>1;r--){
        for(c=2;c<=c2-1;c++){
            assert( in_board(r,c) );
            board.colors[r+dr][c] = board.colors[r][c];
        }
    }
}

// how much of row r is empty on the board.
int find_empty_count_in_row(int r){
    int c, empty=0;
    for(c=2;c<c2-1;c++){
        assert( in_board(r,c) );
        if( !is_solid((uchar) board.colors[r][c]) )
            empty++;
    }
    return empty;
}

// clear non contiguous full rows.
int clear_all_filled_rows_board(void){
    int r,rows_cleared=0;
    for(r=RMAX-1;r>1;r--){
        while( find_empty_count_in_row(r) == 0 ){
            clear_filled_rows_board(1,r);
            rows_cleared++;
            assert(rows_cleared<RMAX);
        }
    }
    return rows_cleared;
}

// is the fbox resting on the board?
int fbox_landed(int w){
    int r,c, dr=1, dc=0;
    for(r=0;r<MAX_WIDGET_HEIGHT;r++){
        for(c=0;c<MAX_WIDGET_WIDTH;c++){
            uchar cc = widget[ fbox[w].shape ][ fbox[w].rotation ][r][c];
            if( is_solid(cc) && !board_free( fbox[w].r + r + dr, fbox[w].c + c + dc) )
                return 1;
        }
    }
    return 0;
}

// place the fbox on the board.
void fbox_to_board(int w){
    int r,c;
    for(r=0;r<MAX_WIDGET_HEIGHT;r++){
        for(c=0;c<MAX_WIDGET_WIDTH;c++){
            uchar cc = widget[ fbox[w].shape ][ fbox[w].rotation ][r][c];
            if( is_solid(cc) )
                board.colors[ fbox[w].r + r ][ fbox[w].c + c ] = cc;
        }
    }
}

// move the fbox to the new location, report whether moved?
int move_fbox(int w, int dr, int dc, int drot, int ch){
    while( drot < 0 )
        drot += 4;
    if( !board_fbox_free(w,dr,dc,drot))
        return 0;
    //fbox_to_board(w,'.');
    fbox[w].r += dr;
    fbox[w].c += dc;
    fbox[w].rotation += drot;
    fbox[w].rotation %= MAX_ROTATIONS;
    if( ch == 0 )
        fbox[w].auto_moved++;
    else
        fbox[w].key_moved++;
    fbox[w].steps_moved++;
    //fbox_to_board(w,'a'+w);
    return 1;
}

// write a message at location, optionally sprintf i.
void mv_message(int r,int c, int havei, char *m, int i){
    char buffer[20];
    r = 1  + r;
    c = c3 + c;
    mvaddstr(r,c,m);
    if( havei ){
        mvaddstr(r,c+strlen(m),"........"); // clear it.
        sprintf(buffer,"%8d",i);
        mvaddstr(r,c+strlen(m),buffer);
    }
}

// get next keystroke, translate arrow keys.
uchar get_inkey(void){ // was getic()
    uchar ic=0;
    #if defined(__TURBOC__) || defined(_MSC_VER)
    uchar ec=0;
    if( (ic = (uchar) getche()) == 0 ) {
      ec = (uchar) getche();
      // TODO test this with vc6
      switch( ec ){
        case 71: ic = 'H'; break; // Home
        case 79: ic = 'E'; break; // End
        case 72: ic = 'U'; break; // Up
        case 75: ic = 'R'; break; // Right
        case 77: ic = 'L'; break; // Left
        case 80: ic = 'D'; break; // Down
        //default: ic = ec;  break;
      }
    }
    #elif defined(CURSES)
    ic=mvgetch( 22, 0 );
    switch( ic ){
      case KEY_UP   : ic = 'U'; break;
      case KEY_RIGHT: ic = 'R'; break;
      case KEY_LEFT : ic = 'L'; break;
      case KEY_DOWN : ic = 'D'; break;
      //default:        ic = ec;  break;
    }
    assert(!"TODO/test");
    #endif
    return ic;
}

// Global variables.
int delay_ms=600, delta_ms=30, delay_keyed_wait=20;
int timer=0,score=0,level=0,pause=0,empty=RMAX,grace=0;
int landing=-1;

// Update the delay_ms by dt.
void update_delay(int dt){
    delay_ms += dt;
    if( delay_ms <= 0 )
        delay_ms = 1;
    if( delay_ms <= 100 )
        delta_ms = 10;
    else
        delta_ms = 100;
}

// Show messages on the right side of the board.
void help(int debug){
    const int mrow=0;
    int r=0;
    mv_message(mrow+r++,0,0,"-------------Tetris Game ------------",0);
    mv_message(mrow+r++,0,0, "GPL(C) 2006 moshahmed@gmail",0);
    mv_message(mrow+r++,0,0,"                              ",0);
    mv_message(mrow+r++,0,0,"-------------------------------",0);
    mv_message(mrow+r++,0,0,"What: Pack the falling boxes.",0);
    mv_message(mrow+r++,0,0,"Press these keys to move the falling box:",0);
    mv_message(mrow+r++,0,0," Up=Rotate AntiClock, Down=Down Space=Drop",0);
    mv_message(mrow+r++,0,0," Left,Right,Home,End = Move Left,Right",0);
    mv_message(mrow+r++,0,0," q=quit, -=fast, +=slow p=pause",0);
    mv_message(mrow+r++,0,0," g=grace n=narrow w/wider m=magic(TODO)",0);
    mv_message(mrow+r++,0,0,"-------------------------------",0);
    mv_message(mrow+r++,0,1,"Timer:",++timer);
    mv_message(mrow+r++,0,1,"Delay:",delay_ms);
    mv_message(mrow+r++,0,1,"Level:",level);
    mv_message(mrow+r++,0,1,"Score:",score);
    mv_message(mrow+r++,0,1,"Pause:",pause);
    mv_message(mrow+r++,0,1,"Empty:",empty);
    mv_message(mrow+r++,0,1,"Grace:",grace);
    mv_message(mrow+r++,0,1,"Landing:",landing);
    mv_message(mrow+r++,0,1,"debug:",debug);
    #ifndef NDEBUG
    //mv_message(mrow+r++,0,1,"rot:", fbox[fbox_moving].rotation );
    //refresh();
    // mv_message(mrow+r++,0,0,"Press any key to begin.",0); getch();
    refresh();
    #endif
}

// Tetris main loop - read keyboard and play.
int main(void){
    chtype ch;
    int done_rows=0,landed=0,dropping=0,many=0,keyed=0;
    srand(time(0));
    initscr(); cbreak(); noecho(); refresh();
    clear_board();
    draw_board();

    fbox_setup();
    fbox_draw();

    do{
        ch = 0;
        no_curor();
        while( keypressed() ){ // clear the queue.
            ch = get_inkey();
            pause = 0;
            keyed = 10;
        }
        if( toupper(ch) == 'Q')
            break;
        help(ch);
        switch( ch ){
            case 'q':
            case 'Q': break;
            case 'U': move_fbox( fbox_moving,0,0,+1,ch); break; // Up/Rotate
            case 'L': move_fbox( fbox_moving,0,+1,0,ch); break; // Left
            case 'R': move_fbox( fbox_moving,0,-1,0,ch); break; // Right
            case 'D': move_fbox( fbox_moving,1,0,0,ch); break; // Down
            case 'E': while( move_fbox( fbox_moving,0,+1,0,ch) ) many++; break; // End/Right Edge
            case 'H': while( move_fbox( fbox_moving,0,-1,0,ch) ) many++; break; // Home/Left Edge
            case ' ': dropping=1 ;break;
            case '+': update_delay(+delta_ms); break;
            case '-': update_delay(-delta_ms); break;
            case 'p': pause=1; break;
            case 'w':
            case 'n': if( ch == 'w' && c2 < CMAX )
                            c2++;  // wide.
                        else if( ch == 'n' && c2 > c1 +MAX_WIDGET_WIDTH*4)
                            c2--;  // narrow.
                        clrscr();
                        clear_board();
                        fbox_setup();
                        draw_board();
                        break;
            case 'r': move_fbox( ++fbox_moving % fbox_in,
                            randu(-1,1),randu(-1,1), randu(-1,1),ch); break;
            case 'g': clear_filled_rows_board(1,RMAX-1); grace++; break;
            case 'm':  // fall for now, magic: auto move to empty spot.
            default:
                if( keyed > 0 ){ // wait for more keystrokes before falling.
                    keyed--;
                }else if(pause){
                    move_fbox(fbox_moving,0,randu(-1,1),randu(-1,1), 0);
                }else if( fbox_near_top(fbox_moving) ){
                    move_fbox(fbox_moving,1, randu(-1,2), randu(-1,1), 0);
                }else{
                    move_fbox(fbox_moving,1,0,0,0);
                }
        }

        if( dropping && ch != 0 && ch != ' ' && ch != 'D'){
            dropping = 0; // Any keypress deactivates dropping.
        }

        empty = num_rows_empty_board();
        if( empty <= MAX_WIDGET_HEIGHT ){
            grace++;
            clear_filled_rows_board(1,RMAX-1);
        }
        draw_board();
        fbox_draw();

        if( dropping || keyed ){
            delay(delay_keyed_wait);
        }else{
            delay(delay_ms);
            if( delay_ms > 100 )
                click();
        }

        landed = fbox_landed(fbox_moving);
        if( !landed ){
            landing=10;
            continue;
        }
        if( landing-- > 0 ) // wait for more keystrokes before settling.
            continue;
        if( landed == 1 ){
            fbox_to_board(fbox_moving);
            fbox_make(fbox_moving);
            fbox_moving = (fbox_moving +1) % fbox_in;

            dropping = 0;

            // done_rows = num_bottom_rows_filled_board();
            // clear_filled_rows_board(done_rows,RMAX-1);
            done_rows = clear_all_filled_rows_board();
            score += done_rows * c2;
            if( delay_ms > 5 )
                thump();
        }
    }while( ch != 'Q' );
    clear(); refresh();
    endwin();
    return 0;
}
// EOF
