/*
What: MASTERMIND game, guess the pin colors and position from clues.
$Id: mmind4.c,v 1.18 2018/10/22 20:23:59 User Exp $
AUTHOR: GPL(C) moshahmed/at/gmail.com
COMPILER: vc++ for windows console, turboc for dos, gcc with curses on linux.
Compiling:
    Turbo Pascal 5.0 version for MSDOS/Windows95/WindowsNT (Console mode)
    Turbo C 2.0 for MSDOS/Windows95/WindowsNT.
      c:\> tcc -Id:\tc\include -Ld:\tc\lib mmind4.c
    Linux: Curses C version, compile with:
      TODO: rebuild and test on linux
      $ gcc -Wall mmind4.c -DCURSES -lcurses -ltermcap -o mmind4
    VC++ 6.0:
      c:\> cl mmind4.c  (Runs in console mode)
    Cygwin ncurses (2011-04-26,2013-07-28)
      $ gcc -Wall mmind4.c -DNCURSES -l ncurses -o mmind4
*/

char USAGE[] = " USAGE: mmind4 [OPTIONS]	\n\n"
" OPTIONS:	\n"
"   -generate N  .. batch generate N problems and solutions to a file	\n"
"     -random    .. generate problems of random width and colors.	\n"
"   -output   PT .. path to prefix to output files.	\n"
"   -pegs     PN .. Number of pegs, default is 4.	\n"
"   -colors   CN .. Number of colors for pegs.	\n"
"   -solve    FN .. Read problem to solve from filename FN,	\n"
"                   input format is same as generated problem	\n"
"                   file, e.g. ' row 1. 1234 BW--'	\n"
"   -verbosity VN .. verbosity, 0 is terse, 1 is verbose, 2 for debug.	\n"
" 	\n"
" Interactive KEYS:	\n"
"   A   .. Play again	\n"
"   Q   .. Quit	\n"
"   S   .. Show solution and play	\n"
"   G   .. Let computer generate a new useful guess.	\n"
"   C   .. Show choices of all satisfiable permutations.	\n"
"          Won't show the solution if there is only one choice.	\n"
"   BS  .. Backspace to undo guess in current step.	\n\n"
" GAME:	\n"
"     Position of PN hidden pegs arranged by the computer in row zero.	\n"
"     The pegs are chosen from CN colors and may be repeated.	\n"
"     The computer gives hints with B/black and W/white pegs.	\n"
"     Hints:	\n"
"     B Each black peg hint means: that one of the guess is of	\n"
"       right color and at right position.	\n"
"     W Each white peg means: that a guessed color was right but in	\n"
"       the wrong position.	\n"
"     Using all the earlier hints the user tries again and again till	\n"
"     she guesses the colors and position of all the hidden pegs in top row.	\n"
;

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#define bool int

#ifdef __TURBOC__
  typedef unsigned char chtype;
  enum { FALSE=0, TRUE=1 };
  enum { BGCOL = 0, FGCOL = 15 };
  #define endwin  clrscr
  #define clear   clrscr
  #define initscr clrscr
  void   refresh(){ }
  void   clrtoeol(){ clreol(); }
  void   noecho(){ }
  void   cbreak(){ }
  void   move(int i,int j){ gotoxy(j+1,i+1); }
  void   setcolor( chtype c){
      // Our colored peg are sequentially numbered '1'..'6'
      // we remap colors here.
      if( c == 'B' ){ textcolor( 8     ); textbackground( 0); return; }
      if( c == '3' ){ textcolor( 14    ); textbackground( 0); return; }
      if( c == '8' ){ textcolor( 13    ); textbackground( 0); return; }
      if( c == '9' ){ textcolor( 12    ); textbackground( 0); return; }
      if(isdigit(c)){ textcolor(c-'0'+8); textbackground( 0); return; }
      textcolor(FGCOL); textbackground(BGCOL);
  }
  void   addch( chtype c){ setcolor( c ); cprintf("%c",c); }
  void   mvaddint( int i, int j, int n){ move(i,j); cprintf("%d",n); }
  void   mvaddch(  int i, int j, chtype c ){ move(i,j); addch(c); }
  void   addstr(chtype*s){ setcolor(0); cprintf("%s",s); }
  void   addint(chtype*f,int n){ cprintf(f,n); }
  void   mvaddstr( int i, int j, chtype*s ){ move(i,j); addstr(s); }
  chtype mvgetch(  int i, int j   ){ move(i,j); return getche();}
#elif  _MSC_VER

#include <windows.h>

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
    SetConsoleTitle("Mastermind (VC++) $Id: mmind4.c,v 1.18 2018/10/22 20:23:59 User Exp $");
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
  void addch( chtype c){
    printf("%c", c);
  }
  chtype getch() {
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
    return getch();
  }
#elif defined(NCURSES)
  #include <ncurses/curses.h>
#endif

enum {
    // Overriding Turbo c colors in <conio.h>
    NONE   = '-',  BLANK  = ' ',  BACKSPACE =  8 , CURSOR = '_',
    BLACK  = 'B',  WHITE  = 'W',
    //  BLACK=0,BLUE=1,GREEN=2,CYAN=3,RED=4,MAGENTA=5,BROWN=6,LIGHTGRAY=7,
    //  DARKGRAY=8,LIGHTBLUE=9,LIGHTGREEN=10,LIGHTCYAN=11,LIGHTRED=12,
    //  LIGHTMAGENTA=13,YELLOW=14,WHITE=15,
};

// Screen locations
static struct {
  const int SOLUTION; // row of grid containing the solution.
  const int STATUS;
  const int ASK;
  struct {
    const int MIN, MAX;
  } CONSISTENTS;
  const int BOT;
} ROWS = { 0, 22, 23, {12, 24}, 25 };

static struct {
  struct {
    const int MIN;
    const int MAX;
  } CONSISTENTS;
  const int HELP_LEFT;
  const int RIGHT;
} COLUMNS = { { 17, 75 }, 28, 80};

#define PEGS_MAX 6
static struct {
  int PN; // how many pegs to play with.
} PEGS = { 4 };

// Can't be computed at compile time.
#define COLORS_MAX 9
static struct {
  char   ALL[COLORS_MAX]; // must be contiguous chars for enumerating permutations
  char   A;  // First color
  char   B;  // Last color
  int    CN; // Number of colors to use.
  char   valid_colors[COLORS_MAX+1]; // for printing.
} MMCOLORS = { "123456789", '1', '6', 6, "?" };


#define MAXTRY 19
#define until(C) while(!(C))
#define STARTS_WITH(s, prefix)  strstr(s, prefix) == s

#define foreach(m,w) for( m=1; m<=w; m++ )

static struct {
  bool print_choices;
  bool batch_mode;
  bool debug; // numbers the rows and columns on screen.
  int  verbosity; // 0 is terse, 1 is normal, 2 is debug.
} opt = { FALSE, FALSE, FALSE, 1 };

static struct {
  char *solveme;
  FILE *fp;
  int step;
  const int show_perms; // how many solutions to show.
  int lineno;
} solver = { "", 0, 0, 3, 0 };

#define   PATH_MAX 80

static struct {
  int num_problems;
  bool random;
  double sigma;
  const int max_try;
  char       *output_path;
  const char *problem_file_name;
  const char *solution_file_name;
  const char *author;
  const char *cvs_id;
} generator = {
  0, // num_problems
  FALSE, // random
  1.0, // sigma
  100,  // max_try
  "mm", "p.txt", "s.txt",
  "GPL(C) moshahmed",
  "$Id: mmind4.c,v 1.18 2018/10/22 20:23:59 User Exp $"
  };

// All arrays are zero indexed, char[] are null terminated.
static struct {
  char guesses[PEGS_MAX+1], hints[PEGS_MAX+1];
  int  blacks, whites;
  int  consistents; // how many possible solutions with step hints.
} grids[MAXTRY];

/* PERM_MAX = pow(MMCOLORS.CN,PEGS.PN), turbo c has limited 16 bit memory */
#ifdef __TURBOC__
#define PERM_MAX 4096 /* 4K = 4096=pow(8,5) */
#else
#define PERM_MAX 2097152 /* 2M = 2097152=pow(8,7) */
#endif

static struct {
  char   generated[PEGS_MAX+1];
  int    blacker, whiter;
  bool   consistent;
} perm[PERM_MAX];
int perm_count=0; // perm[0..perm_count-1].generated

void assumptions(void) {
    int i;
    assert(PEGS.PN <= PEGS_MAX);
    assert(MMCOLORS.CN <= COLORS_MAX);
    // assert(pow(MMCOLORS.CN, PEGS.PN) <= PERM_MAX);
    assert(perm_count <= PERM_MAX);
    assert(MMCOLORS.CN == (MMCOLORS.B - MMCOLORS.A + 1));
    // Contiguous chars for enumerating permutations
    for(i=0; i< (int) strlen(MMCOLORS.ALL)-4;i++) {
      assert( (MMCOLORS.ALL[i] + 1) == MMCOLORS.ALL[i+1] );
    }
}

void clear_grid() {
  memset( &grids,0,sizeof(grids));
}

void clear_perm() {
  perm_count = 0;
  memset( &perm,0,sizeof(perm));
}

// Setup a new perm, copy from previous.
void perm_dup(int z) {
  int i;
  // Generated starts with MMCOLORS.A*
  for (i=0; i<PEGS.PN ; i++) {
    if (z>0) {
      perm[z].generated[i] = perm[z-1].generated[i];
    } else {
      perm[z].generated[i] = MMCOLORS.A;
    }
  }
  perm[z].generated[PEGS.PN] = '\0'; // Make it a string.
  perm[z].consistent = TRUE;
}

// Count blacker and whiter for grids[step].guesses assuming
// perm[z] as solution, update perm[z].
void perm_count_blacks_whites(int z, int step) {
  int j;
  static char mark[PEGS_MAX+1], used[PEGS_MAX+1];

  perm[z].blacker = perm[z].whiter = 0;

  // count the blacker.
  foreach(j,PEGS.PN) {
    mark[j] = used[j] = BLANK;
    if( perm[z].generated[j-1] == grids[step].guesses[j-1] ){
      mark[j] = used[j] = BLACK;
      perm[z].blacker++;
    }
  }

  // count the whites.
  foreach(j, PEGS.PN) {
    int m;
    foreach(m, PEGS.PN) {
        if( (  m != j  ) // wrong column but right color pin.
            && (perm[z].generated[j-1] == grids[step].guesses[m-1])
            && (mark[j] == BLANK ) // and not counted as white
            && (used[m] == BLANK ) // and not counted as black.
        ){
            mark[j] = used[m] = WHITE;
            perm[z].whiter++;
        }
    }
  }
}

void perm_next( char generated[]) {  // Add 1 (left to right) with carry.
  int i;
  for (i=0; i<PEGS.PN ; i++) {
      if ( generated[i] < MMCOLORS.B ) {
        generated[i]++;
        break;
      }
      generated[i] = MMCOLORS.A;
  }
}

bool perm_end( char generated[]) { // Generated ends with MMCOLORS.B*
  int i;
  for (i=0; i<PEGS.PN ; i++)
    if (generated[i] != MMCOLORS.B)
        return FALSE;
  return TRUE;
}

void show_choices(int step) {
  int z;
  int col = COLUMNS.CONSISTENTS.MIN, row = ROWS.CONSISTENTS.MIN;

  if (opt.batch_mode)
    return;

  // clear all lines, may have to clear more than we print.
  for( row = ROWS.CONSISTENTS.MIN; row < ROWS.CONSISTENTS.MAX; row++) {
    move(row,col); clrtoeol();
  }

  // Don't show single consistent, which is the solution.
  if (!opt.print_choices ||  grids[step].consistents <= 1) {
    return;
  }

  // print the consistent perm
  row = ROWS.CONSISTENTS.MIN;
  for(z=0; z<perm_count; z++) {
    if (!perm[z].consistent)
      continue;
    mvaddstr(row, col, perm[z].generated);
    col += PEGS.PN+1;
    if (col > COLUMNS.CONSISTENTS.MAX) {
      col = COLUMNS.CONSISTENTS.MIN;
      row++;
      if (row == ROWS.CONSISTENTS.MAX ) {
        addstr("..");
        row = ROWS.CONSISTENTS.MIN;
      }
    }
  }
}

// Generate all permutations, assume that perm is the solution
// and count how many guesses are consistent with it.
void consistent_perms(int step) {
  clear_perm();
  perm_dup(perm_count);
  grids[step].consistents = 0;
  do {
    int g;
    foreach(g, step) {
      perm_count_blacks_whites(perm_count, g);
      if (grids[g].blacks != perm[perm_count].blacker ||
          grids[g].whites != perm[perm_count].whiter ) {
        perm[perm_count].consistent = FALSE; // not consistent with guesses in g
        break;
      }
    }
    if (perm[perm_count].consistent) {
      grids[step].consistents++;
    }
    if (perm_end(perm[perm_count].generated))
      break;
    perm_count++;
    if (perm_count >= PERM_MAX) {
      if (opt.batch_mode) {
        printf("Out of memory: too many permutations %d\n", perm_count);
        exit(1);
      } else {
        mvaddstr(ROWS.STATUS,1,"No memory for choices.");
      }
      break;
    }
    perm_dup(perm_count);
    perm_next(perm[perm_count].generated);
  } while(1);

  show_choices(step);
}

// Return number of blacks = correct entries in right locations.
//                  whites = correct entries in wrong location.
// update grids[step].hints with black/white hints.

int checkgrid( int step ) {
    int  j;
    static char mark[PEGS_MAX+1], used[PEGS_MAX+1];
    int  hint_column = 0;

    grids[step].blacks = grids[step].whites = 0;

    foreach(j,PEGS.PN) {
        mark[j] = used[j] = BLANK;
        if( grids[step].guesses[j-1] == grids[ROWS.SOLUTION].guesses[j-1] ){
            grids[step].hints[hint_column++] = mark[j] = used[j] = BLACK;
            grids[step].blacks++;
        }
    }

    foreach(j,PEGS.PN) {
      int m;
      foreach(m,PEGS.PN) {
          if( (  m != j  )
              && (grids[step].guesses[j-1] == grids[ROWS.SOLUTION].guesses[m-1])
              && (mark[j] == BLANK )
              && (used[m] == BLANK )
          ){
              grids[step].hints[hint_column++] = mark[j] = used[m] = WHITE;
              grids[step].whites++;
          }
      }
    }

    // Pad the hints with dashes, sometimes there are zero B and W.
    for(;hint_column<PEGS.PN;hint_column++)
      grids[step].hints[hint_column] = '-';

    consistent_perms(step);

    return grids[step].blacks;
}

double rand01() { // random number between 0..1.
  return ((double)rand())/RAND_MAX;
}

// See pascal/random.pas:ranno(), normal random numbers around mean +- sigma.
double random_normal(double mean, double sigma) { // pascal/random.pas:ranno()
  int i;
  double sum = 0;
  const int samples = 12;
  for (i=0;i<samples;i++) {
    sum += rand01();
  }
  return mean + (sum/samples -  0.5) * sigma;
}

// return number in [a..b], normally distributed around (a+b)/2.
int random_normal_ab(int a, int b, double sigma) {
  double mean;
  int ab;
  if (b == a) return a;
  if (b < a ) { int A = a, B = b; b = A; a = B; } // swap.
  mean = 1.0 *(a + b)/2;
  sigma *= (b-a);
  ab = random_normal(mean,sigma) + 0.5;
  if (ab < a) return a;
  if (ab > b) return b;
  return ab;
}

int rand_color() {
  return MMCOLORS.ALL[ rand() % MMCOLORS.CN ];
}

void rand_problem() {
  int j;
  foreach(j,PEGS.PN)
    grids[ROWS.SOLUTION].guesses[j-1] = (char) rand_color();
}

void make_guess(int step) {
  int try;
  foreach(try, generator.max_try) {
    int j, k, s, dups=0;
    foreach(j,PEGS.PN)
      grids[step].guesses[j-1] = (char) rand_color();

    // Also guess must not equal the solution.
    for(s=0;s<step;s++) { // check grids.guesses[step] is a not a dup.
      int same = 0;
      foreach(k,PEGS.PN)
        same += (grids[s].guesses[k-1] == grids[step].guesses[k-1]);
      dups += (same == PEGS.PN);  // grids[s].guesses == grids[step].guesses
    }
    if (dups) // grids[step].guesses is same as some grids[0..step-1].guesses
      continue;
    checkgrid(step);
    if (step > 1 && !(grids[step].consistents < grids[step-1].consistents))
      continue; // skip, if no new hints in this guess.
    return;
  }
  if (opt.batch_mode)
    printf("dup guess after %d tries for step=%d\n", try, step);
}

void problem_generator_i(int pn, FILE *pf, FILE *sf) {
  int step;
  clear_grid();

  if (generator.random) {
      PEGS.PN = random_normal_ab(1, PEGS_MAX, generator.sigma);
      MMCOLORS.CN = random_normal_ab(2, COLORS_MAX, generator.sigma);
      MMCOLORS.A = MMCOLORS.ALL[0];
      MMCOLORS.B = MMCOLORS.ALL[MMCOLORS.CN-1];
      strncpy(MMCOLORS.valid_colors, MMCOLORS.ALL, MMCOLORS.CN);
      MMCOLORS.valid_colors[MMCOLORS.CN] = '\0';
  }

  rand_problem();

  fprintf(pf, "Puzzle %d, find %d pegs of %d colors [%s] {\n",
    pn, PEGS.PN, MMCOLORS.CN, MMCOLORS.valid_colors);

  fprintf(sf, "solution %3d: %s %s (", pn,
    grids[ROWS.SOLUTION].guesses, grids[ROWS.SOLUTION].hints);

  if (opt.verbosity) {
    printf("Puzzle %3d, find %d pegs of %d colors [%s]\n",
      pn, PEGS.PN, MMCOLORS.CN, MMCOLORS.valid_colors);
  }

  for (step=1; step < MAXTRY; step++) {
    make_guess(step);
    fprintf(pf, "  row %2d. %s %s\n",
      step, grids[step].guesses, grids[step].hints);
    fprintf(sf, "%d ", grids[step].consistents);
    if (grids[step].consistents <= 1 )
      break;
  }
  fprintf(sf,")\n");
  fprintf(pf,"}\n");
  return;
}

void print_header(FILE *pf) {
  struct tm *newtime;
  time_t aclock;
  time( &aclock );  /* time in seconds */
  newtime = localtime( &aclock );  /* time to struct tm form */
  fprintf(pf, "\n");
  fprintf(pf,"Generated by mmind4, built %s %s\n", __DATE__,__TIME__);
  fprintf(pf,"Source: %s\n",generator.cvs_id);
  fprintf(pf,"Date %s",asctime(newtime));
  fprintf(pf,"Author: %s\n", generator.author);
  fprintf(pf, "\n");
}

void problem_generator() {
    int i;
    // Open 2 files for writing problem and solution.
    static char problem_file[PATH_MAX];
    static char solution_file[PATH_MAX];
    FILE *pf, *sf;

    // output file names are output_path + ..file_name
    assert(strlen(generator.output_path) +
          strlen(generator.problem_file_name) < PATH_MAX);
    assert(strlen(generator.output_path) +
          strlen(generator.solution_file_name) < PATH_MAX);

    strcpy(problem_file, generator.output_path);
    strcat(problem_file, generator.problem_file_name);

    strcpy(solution_file, generator.output_path);
    strcat(solution_file, generator.solution_file_name);

    pf = fopen(problem_file, "w");
    sf = fopen(solution_file, "w");
    if (!pf) { printf("Cannot write %s\n",problem_file); exit(1); }
    if (!sf) { printf("Cannot write %s\n",solution_file); exit(1); }
    printf("Writing %d problems to problem_file=%s and solution_file=%s\n",
      generator.num_problems, problem_file, solution_file);

    // Problem file
    fprintf(pf,"Mastermind Puzzles\n");
    fprintf(pf,"Deduce the colors of hidden pegs in %d puzzles.\n",
      generator.num_problems);
    if (!generator.random)
      fprintf(pf,"There are %d hidden pegs of %d colors (%c..%c) in each puzzle.\n",
        PEGS.PN, MMCOLORS.CN, MMCOLORS.A, MMCOLORS.B);
    fprintf(pf,"Solutions are in the file '%s'\n", solution_file);
    fprintf(pf,"\n");
    fprintf(pf,"Hints provided for each row of guess are:\n");
    fprintf(pf,"  B: a correct color peg is in correct column\n");
    fprintf(pf,"  W: a correct color peg is in wrong column\n");
    fprintf(pf,"  The hints for each puzzle are necessary and sufficient.\n");
    fprintf(pf,"  The hints are not in the same order as the pegs.\n");
    fprintf(pf,"Each color can appear zero, one or more times.\n");
    print_header(pf);

    // Solution file
    fprintf(sf,"Solutions to %d Mastermind Puzzles in %s\n",
        generator.num_problems, problem_file);
    fprintf(sf,"(The numbers in parenthesis are the choices at each steps)\n");
    print_header(sf);

    foreach(i,generator.num_problems)
      problem_generator_i(i, pf, sf);

    fclose(pf); fclose(sf);
}

void label_grid() {
  int row, col;
  for( col=0; col < COLUMNS.RIGHT; col++ )
    mvaddch( 0, col, (char) ((col % 10) + '0')); // label the cols
  for( row=0; row < ROWS.BOT; row++ )
    mvaddch(row, 0, (char)((row % 10) + '0')); // label the rows
}

void drawgrid( int showsoln ) {
    int row;

    move(0,0); clrtoeol();

    if (opt.debug)
      label_grid();

    mvaddstr( 1, 1, "-\?\?\?\?-");
    for( row = 0 ; row<MAXTRY; row++ ) {
        int col, col2;

        for(col=0;col< (int)strlen(grids[row].guesses); col++) {
          mvaddch( row+1,  col+2,
            (char) ((row == 0 && !showsoln) ? '?' : grids[row].guesses[col]) );
        }

        if (row == 0) {
          mvaddstr(row+1, 3+  PEGS.PN, "Hints");
          mvaddstr(row+1, 4+2*PEGS.PN, "Choices");
        } else {
          for(col2=0; col2< (int) strlen(grids[row].hints); col2++)
            mvaddch( row+1, col2+3+col, grids[row].hints[col2] );
        }
    }
    refresh();
}

void help(void) {
    int col = COLUMNS.HELP_LEFT;
    int row = 1; // row 0 gets erased during play.
    int ch;
    mvaddstr( row++,col, "+----------- Master Mind Game ---------------------+");
    mvaddstr( row++,col, "| Guess the ");
      addint("%d ? hidden numbered pins [", PEGS.PN);
      for(ch=MMCOLORS.A; ch<=MMCOLORS.B; ch++)
        charcolor( (char) ch);
      addch( ']' );
    mvaddstr( row++,col, "| in the top row you will get hints, i.e.          |");
    mvaddstr( row++,col, "| Clues: " ); addch( BLACK );
                            addstr(": a pin is in right place                |");
    mvaddstr( row++,col, "|        " ); addch( WHITE );
                            addstr(": right pin color in wrong position      |");
    mvaddstr( row++,col, "| Q to Quit, BACKSPACE to erase.                   |");
    mvaddstr( row++,col, "| S to see solution and play, C to see choices.    |");
    mvaddstr( row++,col, "| G to let computer make a good guess.             |");
    mvaddstr( row++,col, "| (C) moshahmed                                    |");
    mvaddstr( row++,col, "|                                                  |");
    mvaddstr( row++,col, "+--------------------------------------------------+");
    refresh();
    getch();
    refresh();
}

char readuser( int s ) {
    int  i, j=1;
    char ch;

    do{
        do{
            if( j <= PEGS.PN )
                mvaddch(  s+1, j+1, CURSOR );

            ch = (char) toupper( mvgetch( s+1, j+1 ) );

            mvaddch( s+1, j+1, (char) ch );

            switch( ch ){
            case 'Q' : // Quit.
                return ch;
            case 'S' : // Solution.
                drawgrid( TRUE );
                break;
            case 'G' : // Generate guess.
                mvaddstr(ROWS.STATUS,1,"Guessing."); clrtoeol();
                make_guess(s);
                mvaddstr(ROWS.STATUS,1,"Guessed."); clrtoeol();
                return ch;
            case 'C' : // Possible choices.
                opt.print_choices = !opt.print_choices;
                checkgrid(s);
                mvaddint(s+1, 5+2*PEGS.PN, grids[s].consistents);
                break;
            case BACKSPACE :
                if( j > 1 ){
                    j--; mvaddch( s+1, j+1, ' ');
                }
                break;
            }

            assert( j <= PEGS.PN );

        }until( MMCOLORS.A <= ch && ch <= MMCOLORS.B );

        grids[s].guesses[j-1] = ch;
        mvaddch( s+1, j+1, ch );
        j++;

        if( j == PEGS.PN+1 ) { // Got all pin guesses?
            for( i=1; i<= s-1; i++ ){ // Compare row s with rows 1..s-1, for dup.
                int same = 0, k;
                foreach(k,PEGS.PN) // compare all 4 pegs in row s with row i.
                  same += (grids[s].guesses[k-1] == grids[i].guesses[k-1]);
                if (same == PEGS.PN) {
                    mvaddstr(ROWS.STATUS,1,"Already tried.");
                    j = 1; // start again.
                    break;
                }
            }
        }
    }while( j < PEGS.PN+1 );

    mvaddstr(ROWS.STATUS,1,"ok."); clrtoeol();

    return ch;
}

void parse_guess(int step) {
  char *guess = grids[step].guesses;
  for( ; *guess; guess++) {
    if (*guess < MMCOLORS.A  || *guess > MMCOLORS.B) {
        printf("Invalid char '%c' in guess '%s'\n",
          *guess, grids[step].guesses);
        exit(1);
    }
  }
}

void parse_hints(int step) {
  char *hints = grids[step].hints;
  for( ; *hints; hints++) {
    switch(toupper( (int)*hints)) {
      case 'B' : grids[step].blacks++; break;
      case 'W' : grids[step].whites++; break;
      case '-' : break;
      default  :
        printf("Invalid char '%c' in hint '%s'\n",
          *hints, grids[step].hints);
        exit(1);
    }
  }
}

void check_widths(char* guesses, char* hints) {
  // Check number of pegs and hints are constant.
  int gn, hn, pn;
  gn = strlen(guesses);
  hn = strlen(hints);
  pn = PEGS.PN;
  if (pn < 1 || pn > PEGS_MAX) {
    printf("Number of pegs PN=%d, not specified or out of range?\n", pn);
    exit(1);
  }
  if (gn < 1 || gn > PEGS_MAX) {
    printf("Bad input: expecting width %d between %d to %d\n",
      gn, 1, PEGS_MAX);
    exit(1);
  }
  if (gn != pn || hn != pn ) {
    printf("Bad input: expecting width %d for '%s' and '%s'\n",
      pn, guesses, hints);
    exit(1);
  }
}

void read_problem_file(void) {
  int step = 0;
  printf("Reading problem to solve from file '%s'\n", solver.solveme);
  clear_grid();
  do {
    int parsed;
    static char line[PATH_MAX], words[4][PATH_MAX];
    if (!fgets(line, PATH_MAX, solver.fp)) {
      printf("End of file %s\n", solver.solveme);
      fclose(solver.fp);
      solver.fp = 0; // so we can stop looping.
      break;
    }
    solver.lineno++;
    line[ strlen(line)-1 ] = '\0';

    // Look for puzzle parameters at the beginning of the puzzle.
    if (STARTS_WITH(line,"Puzzle")){
      int width=0, colors=0;
      printf("\nParsing new puzzle, on line %d:'%s'\n", solver.lineno, line);
      parsed = sscanf(line, "%*s %*s %*s %d %*s %*s %d %*s", &width, &colors);
      if( parsed != 2 ) {
        printf("Bad input, line %d:%s\n", solver.lineno, line);
        printf("Expected 'Puzzle N, find PN pegs of CN colors {'\n");
        exit(1);
      }
      if (opt.verbosity)
        printf("Parsed parameters %d pegs %d colors\n", width, colors);

      if (width < 1 || width > PEGS_MAX) {
        printf("Number of pegs PN=%d, out of range [1..%d].\n",
          width, PEGS_MAX);
        exit(1);
      }
      if (colors<2 || colors > COLORS_MAX) {
        printf("Number of colors CN=%d, out of range [2..%d]\n",
          colors, COLORS_MAX);
        exit(1);
      }

      PEGS.PN = width;
      MMCOLORS.CN = colors;
      MMCOLORS.A = MMCOLORS.ALL[0];
      MMCOLORS.B = MMCOLORS.ALL[MMCOLORS.CN-1];
      step = 0;
      continue;
    }

    // Puzzle end mark.
    if (line[0] == '}') {
      if (opt.verbosity)
        printf("Found end of puzzle mark '}'\n");
      break;
    }

    // Look for a guess, e.g. "  row  1. 4514 WW--"
    parsed = sscanf(line, "  %s %s %s %s",
      words[0],words[1],words[2],words[3]);
    if (parsed != 4 || strcmp(words[0],"row")) {
      printf("unparsed line %d:'%s'\n", solver.lineno, line);
      continue;
    }
    if (opt.verbosity)
      printf("  Line %d, parsed guess=%s, hints=%s\n",
        solver.lineno, words[2], words[3]);
    step++; // step is in 1..MAXTRY-1
    if (step >= MAXTRY) {
      printf("Too many steps %d >= %d.\n", step, MAXTRY);
      break;
    }
    check_widths(words[2], words[3]);
    strcpy(grids[step].guesses, words[2]);
    strcpy(grids[step].hints, words[3]);
    parse_guess(step);
    parse_hints(step);
  }while(1);

  printf("Read %d lines from %s, parsed %d steps\n\n",
    solver.lineno, solver.solveme, step);

  solver.step = step;
}

void solve_grids(void) {
  int g;
  foreach(g, solver.step) {
    int sc;
    int pc=0, printed=0;

    if (g==1)
      printf("Solving for %d pegs in %d colors\n", PEGS.PN, MMCOLORS.CN);
    consistent_perms(g);
    sc =  grids[g].consistents;
    printf("  Step %d, row %s, hint %s, %4d solutions.",
      g, grids[g].guesses, grids[g].hints, sc);
    if (sc == 0) {
      printf("\nInconsistent, no solution in %d colors\n", MMCOLORS.CN);
      return;
    }

    printf(" e.g. ");
    for(; pc <= perm_count; pc++) {
      if (printed > solver.show_perms)
        break;
      if (perm[pc].consistent) {
        printf("%s ", perm[pc].generated);
        printed++;
      }
    }
    if (sc == 1)
      printf("(found unique solution)");
    printf("\n");
  }
  printf("\n");
}

int hasprefix(char *s, char *prefix) {
  return strncmp(s, prefix, strlen(prefix)) == 0;
}

void hasarg(int i, int argc, char *opt, char* type) {
  if (i+1 < argc) return;
  printf("Option '%s' needs an argument (%s).\n", opt, type); exit(1);
}

void process_options(int argc, char* argv[]) {
  int i;
  for( i=1; i < argc; i++ ){
    if (strcmp(argv[i],"-help")==0 || strcmp(argv[i],"-h")==0) {
      printf("%s", USAGE);
      exit(0);
    } else if (strcmp(argv[i],"-generate")==0) {
      hasarg(i, argc, argv[i], "numeric>0"); i++;
      generator.num_problems = atoi(argv[i]);
      if (generator.num_problems < 1) {
        printf("Usage: %s -generate GN .. to generate GN problems, found GN=%d\n",
          argv[0], generator.num_problems);
        exit(1);
      }
    } else if (strcmp(argv[i],"-verbosity")==0) {
      hasarg(i, argc, argv[i], "numeric"); i++;
      opt.verbosity = atoi(argv[i]);
      if (opt.verbosity > 1)
        opt.debug = TRUE;
    } else if (strcmp(argv[i],"-random")==0) {
      generator.random = TRUE;
    } else if (strcmp(argv[i],"-pegs")==0) {
      hasarg(i, argc, argv[i], "numeric>0"); i++;
      PEGS.PN = atoi(argv[i]);
      if (PEGS.PN<1 || PEGS.PN > PEGS_MAX) {
        printf("Usage: %s -pegs PN .. PN in 1..%d \n", argv[0], PEGS_MAX);
        exit(1);
      }
    } else if (strcmp(argv[i],"-colors")==0) {
      hasarg(i, argc, argv[i], "numeric>0"); i++;
      MMCOLORS.CN = atoi(argv[i]);
      if (MMCOLORS.CN<2 || MMCOLORS.CN > COLORS_MAX) {
        printf("Usage: %s -colors CN .. CN in 2..%d \n", argv[0], COLORS_MAX);
        exit(1);
      }
      MMCOLORS.A = MMCOLORS.ALL[0];
      MMCOLORS.B = MMCOLORS.ALL[MMCOLORS.CN-1];
    } else if (strcmp(argv[i],"-output")==0) {
      hasarg(i, argc, argv[i], "output_path"); i++;
      generator.output_path = argv[i];
    } else if (strcmp(argv[i],"-solve")==0) {
      hasarg(i, argc, argv[i], "filename"); i++;
      solver.solveme = argv[i];
      solver.fp = fopen(solver.solveme, "r");
      solver.lineno = 0;
      if (!solver.fp) {
        printf("%s -solve '%s' .. cannot read file.\n", argv[0],solver.solveme);
        exit(1);
      }
      while (solver.fp) { // file can contain multiple problems.
        read_problem_file();
        solve_grids();
      }
      exit(0);
    } else if (hasprefix(argv[i], "-")) {
      printf("Invalid option '%s'\n", argv[i] );
      exit(1);
    }
  }
}

int main(int argc, char *argv[]){
    int  num_blacks, showsoln;
    int step;
    char ch;
    srand(time(NULL));

    assumptions();
    process_options(argc,argv);

    if (generator.num_problems > 0) {
      opt.batch_mode = TRUE;
      problem_generator();
      exit(0);
    }

    initscr(); cbreak(); noecho(); refresh();
    do{
      clear();
      help();
      showsoln = FALSE;
      clear_grid();
      rand_problem();
      step = 1;
      do{
        drawgrid( showsoln );
        ch = (char) toupper( (int) readuser( step ) );

        if( ch == 'Q' ) {
            goto quit;
        }

        num_blacks = checkgrid(step);
        mvaddint(step+1, 5+2*PEGS.PN, grids[step].consistents);
        step++;

        drawgrid( showsoln );

      }while( (step < MAXTRY) && (num_blacks < PEGS.PN) && (ch != 'Q') );

      drawgrid( TRUE );

      if( num_blacks == PEGS.PN )
          mvaddstr( ROWS.STATUS, 1, "Correct " );
      else if( step >= MAXTRY )
          mvaddstr( ROWS.STATUS, 1, "Too many tries. ");

      refresh();

    }while( ch != 'N' );

  quit:
    clear(); refresh();
    endwin();
    return 0;
}
