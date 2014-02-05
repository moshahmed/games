// $Header: c:/cvs/repo/github/games/qt31/mastermind/qt_mastermind.cpp,v 1.1 2014-02-04 03:32:29 a Exp $

// AUTHOR: GPL(C) moshahmed/at/gmail
// DATE: 2005-04-29 Fri 21:14

// Compiled and tested with VC++ 6.0 SP4 with QT3.3 and borland bcc55 on W2K.
// ==================================================================
// The Mastermind Game:

// The computer will pick four random pegs out of six coloured pegs.
// You must place your guesses in each row by dragging and dropping the pegs.
// For each row, the computer will give you hints as follows:
// - A black peg means, your guess has a peg of correct colour in the right column.
// - A white peg means, your guess has a peg of correct colour in the wrong column.

// Notes:
// It takes about 5 rows of guesses to get the solution.
// Same coloured pegs may be repeated.
// Right click to toggle options:
//    - sound on/off, - more hints on/off, - show solution,
//    - debug shows board pixel coordinates.
// ==================================================================

#include "qt_mastermind.h"

// Main just instantiates the interactive singleton Mastermind widget.

int main( int argc, char **argv ) {
    QApplication app( argc, argv );
    Mastermind *m = new Mastermind();

    app.setMainWidget( m );
    m->show();
    return app.exec();
}

// The base class MMPeg, abstracts a colored peg on the board.

MMPeg::MMPeg(QCanvas *canvas, int peg_color) :
    QCanvasSprite(0,canvas), peg_color(peg_color) ,gx(-1),gy(-1),phint(0)
{
    QString xpmfile;
    xpmfile.sprintf("images/Marble-peg%d.png", 1 + peg_color % max_pegs );
    QFile xpmf( xpmfile );
    if(!xpmf.exists())
        qFatal("Cannot find %s",xpmfile);

    QCanvasPixmapArray *mypeg = new QCanvasPixmapArray(xpmfile);
    setSequence(mypeg);
    setAnimated(TRUE);
    setZ(1);
    show();
}

void MMPeg::TooTip(QWidget *w,QString t){
    QToolTip::add(w,QRect(x()-10,y()-10,20,20),t);
}

// class MMPegPicker, pegs shown in the top row of choices.

MMPegPicker::MMPegPicker(QCanvas*c,int peg_color)
        : MMPeg(c,peg_color)
{
    setX(30+peg_color*20);
    setY(10);
    setZ(0); // Ground level stuff will not be moved.
    show();
}

// Class MMHintPeg, the small black and white pegs shown as hints.

MMHintPeg::MMHintPeg(QCanvas *c, int hint, int x, int y)
    : QCanvasSprite(0,c)
{
    QString xpmfile;
    if( hint == eWhite)
        xpmfile = "images/white_hint.png";
    else if( hint == eBlack )
        xpmfile = "images/black_hint.png";
    QCanvasPixmapArray *hintpeg = new QCanvasPixmapArray(xpmfile);
    setSequence(hintpeg);
    setAnimated(TRUE);
    setX(x); setY(y); setZ(-1);
}

void MMHintPeg::TooTip(QWidget *w,QString t){
    QToolTip::add(w,QRect(x()-10,y()-10,20,20),t);
}

// class Mastermind, the main interactive widget, handles all signals
// and mouse events. Controls the GUI.

Mastermind::Mastermind( QCanvasView *parent, const char * name )
    : QCanvasView( parent, name ),
        more_hints(0), debugging(0),  show_solution(0), winning_ways(0),
        m_dragging(0), m_mousepos(0), m_score(0), m_solution_label(0),
        xoffset(0), yoffset(0), mmboard(0),m_gong(0)
{
    QCanvas* canvas = new QCanvas(300,500);
    setMinimumSize( 300, 500 );
    setMaximumSize( 350, 550 );
    canvas->setDoubleBuffering(TRUE);
    setCanvas(canvas);

    m_mousepos = AddLabel(4,385,tr("Mouse Position"),debugging);
    m_score = AddLabel(1,430,tr(""),false);
    m_gong = new SoundPlayer();

    setCaption( tr( "Mastermind" ) );
    setIconText( tr( "Mastermind" ) );
    setIcon( QPixmap("images/Marble-peg1.png") );
    QToolTip::add(this,QRect(50,60,180-50,380-60),tr("Think"));
    setName("Mastermind");
    QWhatsThis::add( this, tr( "Mastermind, the game." ) );
    QFont f( font() );
    f.setFamily( "Fixedsys" );
    setFont( f );
    setCursor( crossCursor );
    setMouseTracking( TRUE );
    setFocusPolicy( QWidget::NoFocus );

    canvas->setBackgroundPixmap( QPixmap("images/board2.png") );
    canvas->setBackgroundColor( QColor(200,100,60) ) ;

    // Create the hint pegs, shown only when the whole row is done.
    for(int gy=1;gy<mmboard->GY;gy++){
        for(int gx=0;gx<mmboard->GX;gx++){
            int xx,yy;
            mmboard->rscale(gx,gy,xx,yy);
            xx=gx*10;
            MMHintPeg *pp;
            pp = new MMHintPeg(canvas,MMHintPeg::eWhite,xx,yy);
            pp->TooTip(this,tr("hint: peg color is right, but position is wrong"));
            HintSprite[MMHintPeg::eWhite][gy][gx] = pp;

            pp = new MMHintPeg(canvas,MMHintPeg::eBlack,xx,yy+10);
            pp->TooTip(this,tr("hint: peg color and position is right"));
            HintSprite[MMHintPeg::eBlack][gy][gx] = pp;
        }
    }

    // Show the choices available for placement.
    int peg_color;
    for(peg_color=1;peg_color<6+1;peg_color++){
        MMPegPicker *pp = new MMPegPicker(canvas,peg_color);
        pp->TooTip(this,tr("Pick and Place this Peg below"));
    }
    AddLabel(1,10,tr("Take one"),true);

    NewGame();

    // refresh.
    resize(350,550);
    canvas->setAllChanged();
    show();
}

void Mastermind::ShowHints(void){
    if(!mmboard) return;
    mmboard->ComputeHints(more_hints,winning_ways);
    int r,gx,gy;
    for(gy=1;gy<mmboard->GY;gy++){
        for(gx=0;gx<mmboard->GX;gx++){
            HintSprite[MMHintPeg::eWhite][gy][gx]->hide();
            HintSprite[MMHintPeg::eBlack][gy][gx]->hide();
        }
        for(r=0;r<mmboard->hint_whites[gy];r++){
            HintSprite[MMHintPeg::eWhite][gy][r]->show();
        }
        for(r=0;r<mmboard->hint_blacks[gy];r++){
            HintSprite[MMHintPeg::eBlack][gy][r]->show();
        }
    }
    if( winning_ways )
        m_score->setText("You win");
    else
        m_score->setText("");
    m_score->show();
    canvas()->update();
}

void Mastermind::MakeSolutionPegs(void){
    ASSERT(mmboard);
    if( m_solution_label ){
        for(int gx=0;gx<mmboard->GX;gx++)
            delete m_SolutionPegs[gx];
    }else{
        m_solution_label = AddLabel(1,30,tr("Solution"),true);
    }
    for(int gx=0;gx<mmboard->GX;gx++)
        m_SolutionPegs[gx] = new MMPeg(canvas(),mmboard->pegat(gx,0) );
}

void Mastermind::ShowSolution(void){
    if(!mmboard) return;
    for(int gx=0;gx<mmboard->GX;gx++){
        MMPeg *sp = m_SolutionPegs[gx];
        sp->setX(30+(1+gx)*30);
        sp->setY(30);
        sp->setZ(-1);
        if( show_solution )
            sp->show();
        else
            sp->hide();
        sp->TooTip(this,tr("Solution"));
    }
    canvas()->update();
}

QCanvasText *Mastermind::AddLabel(int x, int y, QString t, bool show){
    QCanvasText *s1 = new QCanvasText(canvas());
    s1->setText(t);
    s1->setX(x);s1->setY(y);s1->setZ(-1);
    if( show )
        s1->show();
    else
        s1->hide();
    return s1;
}

Mastermind::~Mastermind(){
}

// Mastermind's slots handle events from the right mouse menu.

void Mastermind::ToggleDebug(){
    debugging = ! debugging;
    if( debugging )
        m_mousepos->show();
    else
        m_mousepos->hide();
    canvas()->update();
}

void Mastermind::ToggleSound(){
    m_gong->Toggle();
}

void Mastermind::ToggleMoreHints(){
    more_hints = ! more_hints;
    ShowHints();
}

void Mastermind::ToggleSolution(){
    show_solution = ! show_solution;
    ShowSolution();
}

void Mastermind::ClearBoard(void){
    QCanvasItemList il = canvas()->allItems();
    for( QCanvasItemList::Iterator it=il.begin(); it!=il.end(); ++it ) {
        QCanvasItem *ci = *it;
        if( ci->rtti() == MMPeg::RTTI ){
            MMPeg *pp = (MMPeg*)ci;
            if( pp->bx() >= 0 && pp->by() > 0 )
                delete pp;
        }
    }
    if(mmboard)
        mmboard->Clear();
    ShowHints();
    canvas()->update();
}

void Mastermind::AutoPlay(void){
    mmboard->AutoPlay(canvas());
    ShowHints();
    canvas()->update();
}

void Mastermind::NewGame(){
    ClearBoard();
    if( mmboard )
        delete mmboard;
    mmboard = new MMBoard();
    mmboard->Randomize();
    MakeSolutionPegs();
    ShowHints();
    ShowSolution();
}

// Mouse Handlers.

void Mastermind::contentsMousePressEvent( QMouseEvent* e ) {

    if (e->button() == RightButton) {
        QPopupMenu *popup = new QPopupMenu(0, "mypopup");
        if( m_gong->is_enabled() )
            popup->insertItem("Sound off", this, SLOT(ToggleSound()));
        else
            popup->insertItem("Sound on", this, SLOT(ToggleSound()));
        if( show_solution )
            popup->insertItem("Hide Solutions", this, SLOT(ToggleSolution()));
        else
            popup->insertItem("Show Solutions", this, SLOT(ToggleSolution()));
        popup->insertSeparator();
        if( debugging )
            popup->insertItem("Debug off", this, SLOT(ToggleDebug()));
        else
            popup->insertItem("Debug on", this, SLOT(ToggleDebug()));
        if( more_hints )
            popup->insertItem("More hints off", this, SLOT(ToggleMoreHints()));
        else
            popup->insertItem("More hints on", this, SLOT(ToggleMoreHints()));
        popup->insertItem("New Game", this, SLOT(NewGame()));
        popup->insertItem("Clear Board and continue", this, SLOT(ClearBoard()));
        popup->insertItem("Computer Play", this, SLOT(AutoPlay()));
        popup->popup(QCursor::pos());
        return;
    }

    QCanvasItemList il = canvas()->collisions( e->pos() );
    for( QCanvasItemList::Iterator it=il.begin(); it!=il.end(); ++it ) {
        QCanvasItem *ci = *it;
        if( ci->z() < 0 ) // don't touch the background
            continue;
        if( ci->rtti() == MMHintPeg::RTTI )
            continue;

        if( ci->rtti() == MMPegPicker::RTTI ){
            m_dragging = ci;
            MMPegPicker *pp = (MMPegPicker*) m_dragging;
            m_dragging = new MMPeg(canvas(),pp->get_color());
            m_dragging->setX((int)(e->x()));
            m_dragging->setY((int)(e->y()));
            m_gong->gong(3);
            break;
        }else if( ci->z() > 0 ) { // dont move stuff on ground.
            m_dragging = ci;
            xoffset = (int)(e->x() - m_dragging->x());
            yoffset = (int)(e->y() - m_dragging->y());
            break;
        }
    }
}

void Mastermind::contentsMouseReleaseEvent( QMouseEvent *e ) {
    // only peg being dragged is moved.
    if( m_dragging && m_dragging->rtti() == MMPeg::RTTI ){
        MMPeg *pp = (MMPeg*) m_dragging;
        int xx = e->x() - xoffset;
        int yy = e->y() - yoffset;

        int snapx,snapy; // snaps to grid.
        if( mmboard->putpeg(xx,yy,pp,snapx,snapy) ){
            m_dragging->setX(snapx);
            m_dragging->setY(snapy);
            m_dragging->setZ(0); // lock it on ground.
            m_dragging = 0;
            ShowHints();
            mmboard->ReadScore(m_gong);
        }
        canvas()->update();
    }
}

// GUI debug routines.
void Mastermind::ShowMousePosition( QMouseEvent *e, const char *t ){
    if( !debugging )
        return;
    QString mouse_at;
    int sx,sy;
    mmboard->scale(e->x(),e->y(),sx,sy);
    mouse_at.sprintf("%s x=%d,y=%d,xc=%d,yc=%d,color=%d",t,e->x(),e->y(),sx,sy,
        mmboard->pegat(sx,sy));
    m_mousepos->setText(mouse_at);
    canvas()->update();
}

void Mastermind::contentsMouseMoveEvent( QMouseEvent *e ) {

    ShowMousePosition(e,"MouseMove");
    if( m_dragging ) {
        m_dragging->setX( e->x() - xoffset );
        m_dragging->setY( e->y() - yoffset );
        canvas()->update();
    }
}

void Mastermind::contentsMouseDoubleClickEvent( QMouseEvent * e ){
    QCanvasItemList il = canvas()->collisions( e->pos() );
    if( m_dragging && m_dragging->rtti() == MMPeg::RTTI ){
        m_dragging->hide();
        delete m_dragging;
        m_dragging = 0;
        m_gong->gong(1);
    }
    canvas()->update();
    ShowHints();
}

void Mastermind::wheelEvent(QWheelEvent * e){ // not used for now.
    //canvas()->update();
    e->ignore();
}

// class MMBoard, the abstract grid of pegs.

MMBoard::MMBoard(void){
    Clear();
}

void MMBoard::Clear(void){
    final_row = 0;
    for(gy=1;gy<GY;gy++){
        row_done[gy] = hint_blacks[gy] = hint_whites[gy] = 0;
        for(gx=0;gx<GX;gx++)
            grid[gx][gy]=0;
    }
}

void MMBoard::Randomize(void){
    QTime midnight( 0, 0, 0 );
    srand( midnight.secsTo(QTime::currentTime()) );
    for(gx=0;gx<GX;gx++)
        grid[gx][0]= 1 + rand() % MMPeg::max_pegs;
}

void MMBoard::AutoPlay(QCanvas *canvas){
    int filled;
    for(gy=1;gy<GY;gy++){
        filled = 1;
        for(gx=0;gx<GX;gx++)
            if(!grid[gx][gy])
                filled=0;
        if(!filled){
            for(gx=0;gx<GX;gx++)
                if(!grid[gx][gy]){
                    grid[gx][gy] = 1 + rand() % MMPeg::max_pegs;
                    MMPeg *pp = new MMPeg(canvas,grid[gx][gy]);
                    putpeg_grid(gx,gy,pp);
                }
            break;
        }
    }
}

void MMBoard::ComputeHints(int more_hints, int& winning_ways){
    winning_ways = 0;
    for(gy=1;gy<GY;gy++){
        hint_whites[gy]=hint_blacks[gy]=0;
        int filled = 1; // find the rows that are fully done.
        for(gx=0;gx<GX;gx++)
            if(!grid[gx][gy])
                filled=0;
        if(!filled && !more_hints)
            continue; // no hints for half done row.
        final_row = gy;
        row_done[final_row]=1;
        int mark[GX],used[GX];
        for(gx=0;gx<GX;gx++){  // count exact matches
            used[gx]=mark[gx]=0;
            if( grid[gx][gy] == grid[gx][0] ){
                hint_blacks[gy]++;
                used[gx]=mark[gx]=1;
            }
        }
        if( hint_blacks[gy] == GX )
            winning_ways=1;
        // The only tricky part in the whole game.
        for(gx=0;gx<GX;gx++){ // count switched pegs.
            for(int gk=0;gk<GX;gk++){
                if( (gx != gk) &&
                    grid[gx][gy] == grid[gk][0] &&
                    !mark[gx] && !used[gk] ){
                        mark[gx]=used[gk]=1;
                        hint_whites[gy]++;
                }
            }
        }
    }
}

void MMBoard::ReadScore(SoundPlayer *player){
    int bb = hint_blacks[final_row];
    int ww = hint_whites[final_row];
    // TODO
    player->gong(2);
    return;
}

// scale board coordinates to grid.
void MMBoard::scale( int xx, int yy, int &sx, int &sy ){
    sx = -1; sy = -1; // outside the board.
    if( xx > 56 && xx < 170 )
        sx = (xx - 56)/ 30;
    if( yy > 35 && yy < 370 )   // row 0 has the solution.
        sy = (yy-35)/32;
}

// inverse scale grid to board coordinates.
void MMBoard::rscale( int gx, int gy, int& snapx,int& snapy ){
    if( gx < 0 || gx > GX || gy < 1 || gy > GY ){
        snapx = 0; snapy = 0;   // outside the board?
        return;
    }
    snapx = gx *30 + 60;
    if( snapx < 56 ) snapx = 56;
    if( snapx > 170 ) snapx = 170;

    snapy = (gy)*32 + 36;
    if( snapy < 36 ) snapy = 36;
    if( snapy > 370 ) snapy = 370;
}

bool MMBoard::putpeg(int xx, int yy, MMPeg *pp, int& snapx, int& snapy ){
    int gx,gy;
    scale(xx,yy,gx,gy);
    rscale(gx,gy,snapx,snapy);
    if( gx < 0 || gx >= GX || gy < 1 || gy >= GY ) // outside the board?
        return false;
    if( grid[gx][gy] && pp->get_color() > 0 ) // already occupied?
        return false;
    pp->place(gx,gy);
    grid[gx][gy] = pp->get_color(); // occupy it.
    return true;
}

void MMBoard::putpeg_grid(int gx, int gy, MMPeg *pp ){
    int xx=0,yy=0;
    rscale(gx,gy,xx,yy);
    pp->place(gx,gy);
    pp->setX(xx); pp->setY(yy); pp->setZ(-1);
}

bool MMBoard::putpeg(int xx, int yy, MMPeg *pp ){
    int snapx,snapy;
    return putpeg(xx,yy,pp,snapx,snapy);
}

// safe access to grid.
int MMBoard::pegat(int sx, int sy ){
    if( sx < 0 || sx >= GX || sy < 0 || sy >= GY )
        return 99;
    return grid[sx][sy];
}

// Class SoundPlayer

SoundPlayer::SoundPlayer() : enabled(1) {
    if(!QSound::isAvailable()){
        enabled = 0;
        QMessageBox::warning(this,"No soundsystem","No soundsystem");
    }
    QTime time = QTime::currentTime();
    QTimer *internalTimer = new QTimer( this );
    connect( internalTimer, SIGNAL(timeout()), SLOT(timeout()) );
    internalTimer->start( 2000 );

}

void SoundPlayer::timeout(void){
}

void SoundPlayer::voice(QString filename){
    QFile file(filename);
    if(!file.exists())
        qFatal("Sound file missing %s",filename);
    QSound::play(filename);
}

void SoundPlayer::gong(int i){
    if(!enabled)
        return;
    switch(1+i%4){
        case 1: QSound::play("sounds/1.wav"); break;
        case 2: QSound::play("sounds/2.wav"); break;
        case 3: QSound::play("sounds/3.wav"); break;
        case 4: QSound::play("sounds/4.wav"); break;
    }
}

void SoundPlayer::Toggle(void){
    gong(4);
    enabled = ! enabled;
    gong(3);
}

void SoundPlayer::doPlay1(void){
    if(!enabled)
        return;
    QSound::play("sounds/1.wav");
}

// EOF
