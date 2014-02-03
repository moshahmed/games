{
 AUTHOR: GPL(C) moshahmed/at/gmail

 SYNOPSIS: Rubik's cube.

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
 21: applystr original.           19. stringhistory[0]
 22: apply (ipos).                20. stringhistory[0]
 23: messages.
 24: input.

 variables:

 s  == 6 faces of the 3x3 cube; face 7 is tmp.
 af == Adjacent face matrix.
}

uses crt;

var    s  : array [0..6,0..2,0..2] of integer;
const  af : array [0..5,0..5]      of integer
   = ((0,2,4,3,5,1),
      (1,3,4,2,5,0),
      (2,1,4,0,5,3),
      (3,4,1,5,0,2),
      (4,2,1,3,0,5),
      (5,3,1,2,0,4));

var wait : integer;
var stringhistory : array[0..10-1] of string;
var autocheckon : boolean;
const CONTROL_C :  char =  #3;
var controlc :  boolean;
var stepcount :  integer;

{var ask : integer;}
procedure message(s : string);
var ch :  char;
begin{message}
   textcolor(WHITE);
   gotoxy(1,23); write(s); clreol;
   {
   if( ask = 1 )then
     begin
     write(', press any key to continue:');
     ch := readkey;
     gotoxy(1,23); write(s); clreol;
   end;
   }
end{message};

{ Print a single piece of color e of face i, ie s[i][j][k]. }

procedure print(i,j,k, e :  integer);
const X :  array[0..5] of integer = (1,1,2,1,0,3);
const Y :  array[0..5] of integer = (7,0,7,14,7,7);
const cubecolors :  array[0..5] of integer
   =  ( WHITE, BLUE, LIGHTRED{orange}, RED, YELLOW, GREEN );
var r :  integer;
begin{print}
   textcolor( cubecolors[e] );
   for r := 0 to 1 do
      begin
         gotoxy(  1+X[i]*4*2 +2*k, 1+Y[i]+j*2+r );
         write( chr(219) {e} ); write( chr(219) {e} );
      end;
end{print};

{ Draw the cube, either in ascii, or in color. }

procedure draw;
var x, j, k : integer;
begin{draw}
   for j:=0 to 3-1 do
      for k :=0 to 3-1 do
         print(1,j,k, s[af[0,1],j,k] );
   for j :=0 to 3-1 do
   begin
      for k :=0 to 3-1 do print(4,j,k, s[af[0,4],j,k] );
      for k :=0 to 3-1 do print(0,j,k, s[af[0,0],j,k] );
      for k :=0 to 3-1 do print(2,j,k, s[af[0,2],j,k] );
      for k :=0 to 3-1 do print(5,j,k, s[af[0,5],j,k] );
   end;
   for j :=0 to 3-1 do
      for k :=0 to 3-1 do
         print(3,j,k, s[af[0,3],j,k] );
end{draw};

{ Just rotate the nine pieces on front face N times,
  doesn't touch the adjoining pieces.
}

procedure rotface(  N, f : integer);
var i,t0,t1 : integer;
begin{rotface}
   for i:=1 to N do
      begin
         t0       := s[f,0,0];
         t1       := s[f,0,1];
         s[f,0,0] := s[f,0,2];
         s[f,0,1] := s[f,1,2];
         s[f,0,2] := s[f,2,2];
         s[f,1,2] := s[f,2,1];
         s[f,2,2] := s[f,2,0];
         s[f,2,1] := s[f,1,0];
         s[f,2,0] := t0;
         s[f,1,0] := t1;
      end;
end{rotface};

{ When turning front face, rotate the rows of adjoining faces. }
procedure rotrows(f0,f1,f2,f3 :integer );
var t0,t1,t2 :  integer;
begin{rotrows}
    t0        := s[f0,2,0];
    t1        := s[f0,2,1];
    t2        := s[f0,2,2];
    s[f0,2,0] := s[f1,0,0];
    s[f0,2,1] := s[f1,1,0];
    s[f0,2,2] := s[f1,2,0];
    s[f1,0,0] := s[f2,0,2];
    s[f1,1,0] := s[f2,0,1];
    s[f1,2,0] := s[f2,0,0];
    s[f2,0,0] := s[f3,0,2];
    s[f2,0,1] := s[f3,1,2];
    s[f2,0,2] := s[f3,2,2];
    s[f3,0,2] := t2;
    s[f3,1,2] := t1;
    s[f3,2,2] := t0;
end{rotrows};

{ turn front face N times. }
procedure turn( N : integer );
var i : integer;
begin{turn}
   for i := 1 to N do
   begin
      rotface( 1, 0 );
      rotrows( af[0,1], af[0,2], af[0,3], af[0,4] );
   end;
end{turn};

{ copy nine pieces in face b to face a }
procedure cpf(a,b :  integer );
var i,j : integer;
begin{cpf}
   for i := 0 to 2 do
      for j := 0 to 2 do
         { Back side is actually flipped when turned up/down }
         if(((a = 1) and (b in [2,3]))  or
            ((b = 1) and (a in [2,3])) ) then
            s[a][i][j] := s[b][2-i][j]
         else
            s[a][i][j] := s[b][i][j];
end{cpf};

{ Roll faces a<-b<-c<-d<-a, Change the orientation of the cube. }

procedure roll( a, b, c, d :  integer );
begin{roll}
   cpf( 6,a ); cpf( a,b ); cpf( b,c ); cpf( c,d ); cpf( d,6 );
end{roll};

procedure leftroll;
begin{leftroll}
   roll(1,4,0,5); rotface( 1, 2); rotface( 3, 3);
end{leftroll};

procedure rightroll;
begin{rightroll}
   roll(5,0,4,1); rotface(3, 2); rotface(1, 3);
end{rightroll};

procedure uproll;
begin{uproll}
   roll(0,2,1,3); rotface( 3, 5); rotface( 1, 4);
end{uproll};

procedure downroll;
begin{downroll}
   roll(0,3,1,2); rotface( 3, 4); rotface( 1, 5);
end{downroll};


procedure showstringhistory;
var i :  integer;
begin
   for i:=0 to 1 do
   begin
      gotoxy(33,19+i);
      write(i,'="',stringhistory[i], '"');
      clreol;
   end;
end;


procedure help;
begin
   gotoxy( 33,1); write('Rubiks cube simulator.                  ');
   gotoxy( 33,2); write('(C) moshahmed/at/gmail                  ');
   gotoxy( 33,3); write('........................................');
   gotoxy( 33,4); write('---------------------KEYS-------------  ');
   gotoxy( 33,5); write('l,r,u,d,f,b      : rotate faces.        ');
   gotoxy( 33,6); write('L,R,U,D,arrows   : rolling cube.        ');
   gotoxy( 33,7); write('q: quit            i : initialize.      ');
   gotoxy( 33,9); write('w/W: +/- wait. Backspace/<: undo.       ');
   gotoxy( 33,8); write('a: apply string. 0..9: use string history');
   gotoxy( 33,9); write('A: autocheck toggle. C-c to break.       ');
   gotoxy( 33,12);write('---------------------STRINGs----------   ');
   gotoxy( 33,13);write('Action A ::= l|r|u|d|f|b|i|L|R|U|D|A-|AN.');
   gotoxy( 33,14);write('            where: A- is A3, N is digits.');
   gotoxy( 33,15);write('String S ::= A | N | (S)N                ');
   gotoxy( 33,16);write(' eg. "(ru-D(fU)21)715."                  ');

   showstringhistory;
end;

procedure init( firsttime : boolean ) ;
var  i, j, k : integer;
begin{init}
   clrscr;
   stepcount := 0;
   for i := 0 to 6-1 do
      for j :=0 to 3-1 do
         for k :=0 to 3-1 do
            s[i,j,k] := i;

   if( firsttime )then
      begin
         wait := 0;
         autocheckon := false;
         for i:= 0 to 10-1 do
            stringhistory[i] := '';
      end;

   help;
end{init};

function done(dummy : integer): boolean;
var i,j,k :  integer;
begin{done}
   done := true;
   for i := 0 to 6-1 do
      for j :=0 to 3-1 do
         for k :=0 to 3-1 do
            if s[i,j,k] <> i then
               done := false;
end{done};

procedure autocheck;
var ch :  char;
begin
   if( autocheckon and done(0) and (stepcount > 0 ))then
      repeat
         draw;
         message('done!, press C to continue.');
         ch := readkey;
         if( ch = CONTROL_C )then
            controlc := true;
      until( ch = 'C' ) or controlc;

   gotoxy( 33,18); write('step=',stepcount);
end;

const icol :  integer = 22;
const ipos :  integer =  1;

procedure apply(ch : char );
begin{apply}
   case ch of
        'f' : turn(3);
        'b' : begin leftroll;leftroll;turn(3);rightroll;rightroll; end;
        'u' : begin uproll;turn(3);downroll; end;
        'd' : begin downroll;turn(3);uproll; end;
        'r' : begin rightroll;turn(3);leftroll; end;
        'l' : begin leftroll;turn(3);rightroll; end;
        'U' : uproll;
        'D' : downroll;
        'L' : leftroll;
        'R' : rightroll;
        'i' : begin init(false); ipos := 1; end;
   end; { case }
end{apply};

{ apply ch, maintain history, wrap at 80. undo char is '<' }
var history  :  array[1..80] of char;
procedure appundo(ch : char);
var i :  integer;
begin{appundo}
   if( ch = '<' )then
   begin
      if( ipos > 1 )then
      begin
         ipos := ipos -1;
         for i:=1 to 3 do
            apply( history[ipos]);
         textcolor(CYAN);
         gotoxy(ipos,icol); write(history[ipos]);
      end
      else
         message('no history.');
   end
   else
   begin
      apply( ch );
      history[ipos] := ch;
      textcolor(WHITE);
      gotoxy(ipos,icol); write(ch);
      ipos := ipos +1; if ipos >= 80 then ipos := 1;
      if ch in ['f','b','u','d','r','l','U','D','L','R'] then
         stepcount := stepcount+1;
      autocheck;
   end
end{appundo};


{ backup to opening paren to reapply until N is zero }

function matchparen(var s : string; i, level : integer):integer;
begin{matchparen}
   matchparen := i+1; {ie. not found}
   while( (i>0) and (level>0))do
   begin
      case s[i] of
        '(' : level := level -1;
        ')' : level := level +1;
      end;
      i := i-1;
   end;
   if( level > 0 )then
   begin
      message('Matching "(" not found:');
      write(s);
   end
   else
      matchparen := i;
end{matchparen};


{ applies string =~ /([rludfb][-0-9]?)*/
 "X"         is in     [rludfb].
 "N"         is in     [-0-9].
 "XN"        applies "X"      N times.
 "X-"        applies "X"      3 times.
 "(string)N" applies "string" N times.
 }

procedure applystr(s : string; start, final, count : integer );
var i, j, k    :  integer;
var act, ch :  char;
var sum, oldi :  integer;

begin{applystr}

   if controlc then  {exits from recursive call.}
      exit;

   act := ' ';
   textcolor(WHITE);
   gotoxy(1,21); clreol;
   write('applystr("',s,'"[',start,',',final,'],',count,').');

   while( count > 0 )do
   begin
      i := start;
      while( i <= final )do
      begin
         ch := s[i];
         case ch of
           'i', 'f','b',
           'u','d','r','l',
           'U','D','R','L' : begin appundo(ch); act:=ch; end;
           '0'..'9': for k:=2 to ord(ch)-ord('0') do appundo(act);
           '-'             : begin appundo(act); appundo(act); end;
           ')'             :
           begin
              j     := matchparen( s, i-1, 1 );
              oldi  := i;
              sum := 0;
              while( i < final )and( s[i+1] in ['0'..'9'] )do
              begin
                 sum := sum * 10 + ord(s[i+1])-ord('0');
                 i := i+1;
              end;
              { s[j] = '(', and s[i] is ')', we have already
               applied the string once when we reached here, so -1. }
              applystr( s, j+1, oldi-1, sum-1 );
           end;

         end; { case }
         if( wait > 0 )then
         begin
            draw;
            delay(wait);
         end;

         if keypressed then
         begin
            ch := readkey;
            if( ch = CONTROL_C )then
               controlc := true;
         end;
         { autocheck can also set controlc }
         if controlc then
         begin
            message('CONTROL_C');
            exit;
         end;

         i := i+1;
      end;
      count := count-1;
   end;
end{applystr};

{ apply complete string once.
  if s == '1' to '9' == i, then apply i-1 th history string,
 '0' being the redo last.
 '123' apply last string 123 times.
}

procedure applystring(s : string; his : integer );
var i :  integer;
begin
   if length(s) = 0 then
      exit;

   if( his in [0..9] )then
      s := stringhistory[ his ]
   else if (s[1] in ['0'..'9']) then
   begin
      i := length(stringhistory[0]);
      while( (i > 0 ) and (stringhistory[0,i] in ['0'..'9']))do
         i := i-1;
      s := copy( stringhistory[0], 1, i ) + s;
      stringhistory[0] := s;
      showstringhistory;
   end
   else
   begin
      for i:= 9-1 downto 0 do
         stringhistory[i+1] := stringhistory[i];
      stringhistory[0] := s;
      showstringhistory;
   end;
   applystr( s, 1, length(s), 1 );
   autocheck;
end;

procedure test;
var k :  integer;
begin{test}
   message('testing.');
   draw;
   applystring( '(ruf)240', -1 );
   if done(0) then
      message('test passed')
   else
      message('test failed.');
   draw;
end{test};


var ch, che: char;
var instr :  string;
begin{main}
   clrscr; randomize;
   init(true);
   test;
   repeat
      controlc := false;
      gotoxy(1,24);  textcolor(WHITE);
      ch := readkey; clreol; write(ch,'_');
      message('ok.');

      if( ch in ['0'..'9'] )then
          applystring( '', ord(ch)-ord('0') );

      case ch of
        'Q','q'         : begin clrscr; exit; end;
        'i',
        'f','b',
        'u','d','r','l',
        'U','D','R','L' : appundo(ch);
        {bs  } #8,'<'   : appundo('<');
        'a'             : begin
           gotoxy(1,24); clreol;
           write('string to apply:');
           gotoxy(17,24); readln( instr );
           applystring( instr, -1 );
           gotoxy(1,24); clreol;
        end;
        'A'             : begin
           autocheckon := not autocheckon;
           if( autocheckon )then
              begin
              message( 'autocheckon='); write(true)
              end
              else
              begin
              message( 'autocheckon='); write(false);
              end;
        end;
        'w'             : begin
           wait := wait +500;
           message('delay=');write(wait);
        end;
        'W'             : begin
           wait := wait -500;
           if( wait < 0 )then
              wait := 0;
           message('delay=');
           write(wait);
        end;
        #0 : {extended key}
      begin
         che := readkey;
         case che of
           {l-ar}#75 : appundo('R');
           {r-ar}#77 : appundo('L');
           {u-ar}#72 : appundo('D');
           {d-ar}#80 : appundo('U');
         end; { case }
      end;
      end;
      draw;
   until false;
end{main}.


