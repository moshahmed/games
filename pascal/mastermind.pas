program mastermind;
{
  GPL(C) moshahmed/at/gmail
  $Header: c:/cvs/repo/github/games/pascal/mastermind.pas,v 1.1 2014-02-04 03:31:59 a Exp $ 

 SYNOPSIS: MASTERMIND game, Player has to guess the colour and
    position of four hidden pegs arranged by the computer in row zero.
    The pegs are chosen from six colours and may be repeated.
    The computer gives hints with black and white pegs.
     Each black peg hint means, that one of the four guess is of
      right colour and at right position
     Each white peg means, that a guessed colour was right but in
      the wrong position.
    Using all the earlier hints the user tries again and again till
    he guesses all the colours and position of the pegs.
    Compile with Turbo Pascal 5.0

}
uses crt;

const maxtry = 20;
var grid : array [0..maxtry,1..10] of -1..5;
var soundon : boolean;

{ -------------------------------------------------------- }
procedure ring( i : integer );
   var j : integer;
  begin
  for j := 1 to 4 do
    begin
    sound( i*100+j*2 ); delay( 1 ); nosound; delay( 1 );
    end;
  delay( 100 );
  end;

procedure ringchar( i : integer );
   var j : integer;
  begin
  if soundon then
    begin
    for j := 1 to 15 do
      begin
      sound( 800+i ); delay( 1 ); nosound; delay( 1 );
      end;
     delay( 100 );
     end;
  end;

{ -------------------------------------------------------- }
function altcolor( c : integer ) : integer;
   var d : integer;
  begin
  case c of
    -1: d := black;
    0 : d := DarkGray; { Black }
    1 : d := Cyan;
    2 : d := Green;
    3 : d := Yellow;
    4 : d := Red;
    5 : d := White;
    end;
  altcolor := d;
  end;
{ -------------------------------------------------------- }
procedure drawrow( i : integer );
   var j : integer;
   begin
   textcolor( white );
   write( i:4, ' ' );
   for j := 1 to 4 do
       begin
       textcolor( altcolor( grid[i,j] ) ); write( grid[i,j]:2 );
       end;
   write('  ');
   for j := 6 to 9 do
       begin
       textcolor( altcolor( grid[i,j] ) ); write( 'o' );
       end;
   writeln;
   end;

procedure draw( showsoln : boolean );
   var i,j:integer;
   begin
   gotoxy(1,1); clreol;
   if showsoln then
      drawrow( 0 );
   gotoxy(1,2);
   for i:=1 to maxtry do
      drawrow(i);
   end;

{ -------------------------------------------------------- }
function checkgrid(i:integer):boolean;
   var j, k, m : integer;
       mark : array[1..4] of integer;
       solved,flag : boolean;
   begin

   for j:=1 to 4 do
     mark[j] := 0;

   for j:=1 to 4 do
      if grid[i,j] = grid[0,j] then
         mark[j] := 2;

   for j:=1 to 4 do
     if mark[j] <> 2 then
       begin
       flag := true;
       for m := 1 to 4 do
          if (grid[i,j] = grid[0,m]) and (mark[m] = 0) and flag then
             begin
             mark[m]:=1;
             flag := false;
             end;
       end;

   k := 6;
   for j := 1 to 4 do
      if mark[j] = 2 then
         begin
         grid[i,k] := 0; { black-pin }
         ring( 5 );
         k := k+1;
         end;

   for j := 1 to 4 do
      if mark[j] = 1 then
         begin
         grid[i,k] := 5;   { white-pin }
         ring( 4 );
         k := k+1;
         end;

   repeat
     ring( 8 );
     k := k+1;
     until k > 9;

   solved := true;
   for j:=1 to 4 do
      solved := solved and (mark[j]=2);

   checkgrid := solved;
   end;
{ -------------------------------------------------------- }
procedure help;
   var i : integer;
   begin
   textcolor( LightGray );
   gotoxy( 22,02 ); write('----------- Master Mind Game -----------');
   gotoxy( 22,04 ); write('Guess the four hidden colored pins [');
      for i := 0 to 5 do
         begin
         textcolor(altcolor(i)); write(i:1);
         end;
      textcolor( LightGray );
      write(']');
   gotoxy( 22,05 ); write('in row 0, you will get');
   gotoxy( 22,06 ); write('Clues: black : pin is in right place.');
   gotoxy( 22,07 ); write('       white : pin color ok, wrong position.');
   gotoxy( 22,08 ); write('A for Answers, Q to Quit, BACKSPACE to erase.');
   gotoxy( 22,09 ); write('Enter 4 colors (0..5), RETURN to submit.');
   gotoxy( 22,11 ); write('GPL(C) moshahmed/at/gmail                 ');
   end;
{ -------------------------------------------------------- }
function readuser( s : integer ):char;
    var j, d : integer;
        ch : char;
    label quit;
  begin
  textcolor( white );
  j := 1;
  repeat
     repeat
       gotoxy( 5+j*2 {row}, s+1 {col} );
       if j <= 4 then write('_');
       gotoxy( 5+j*2 {row}, s+1 {col} );
       ch := readkey;
       d := ord(ch) - ord('0');   { random(6)+1; }
       readuser := ch;
       ringchar( ord(ch) );
       case ch of
          'q','Q','a','A' : goto quit;
          #8 : begin
               if j>1 then j := j-1; { Backspace }
               write(' ');
               end;
          end;
       if j > 4 then goto quit;
       until ch in ['0'..'5'];
     grid[s,j] := d mod 6;
     textcolor( altcolor( grid[s,j] ) ); write( ch );
     j := j+1;
     until j > 5;
  quit:
  end;
{ -------------------------------------------------------- }
var done : boolean;
    i,j,step  : integer;
    ch        : char;
    showsoln  : boolean;

begin
repeat
  clrscr;
  randomize;
  help;
  soundon := false;
  showsoln := false;

  for i:=0 to maxtry do
      for j := 1 to 10 do
          grid[i,j] := -1;

  for j := 1 to 4 do
      grid[0,j] := random(6);

  step := 1;
  repeat
     draw( showsoln );
     ch := readuser( step );
     if ch in ['a','A'] then
       showsoln := true;

     done := checkgrid(step);
     draw( showsoln );
     if not( ch in ['a','A'] ) then
        step := step+1;
     until (step>=maxtry) or done or (ch in ['q','Q']);
   draw( true );

   gotoxy( 1, 22 ); clreol;
   textcolor( White );
   if done then
      begin
      write( ' correct ' );
      ring( 1 );
      end
   else
      if step >= maxtry then
        write(' too many tries. ');
   write(' play again [y/n]? ');
   ch := readkey;
   until ch in ['q','n','N'];
end.
{ -------------------------------------------------------- }

