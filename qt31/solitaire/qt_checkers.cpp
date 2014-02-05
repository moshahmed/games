// $Header: c:/cvs/repo/github/games/qt31/solitaire/qt_checkers.cpp,v 1.1 2014-02-04 03:32:59 a Exp $

// AUTHOR: GPL(C) moshahmed/at/gmail
// DATE 2005-05-07 Sat 21:58

// Compiled and tested with VC++ 6.0 SP4 with QT3.3 and borland bcc55 on W2K.
// ==================================================================

// The Marble Solitaire Game:
//
// The aim of this game is to remove the marbles from the board one by one
// until only one is left in the center of the board.
//
// A marble x can only move by jumping over another adjacent marble y
// with an empty spot z on the other side of y.  After moving x to z,
// the marble y is removed from the board.
//
// Notes: You can use history to move back and forth in the game
//   and even save and load a partial game to a file.

#include "qt_checkers.h"

int main( int argc, char **argv ) {
    QApplication app( argc, argv );
    TopWindow * bw = TopWindow::instance(0,"TopWindow");
    app.setMainWidget( bw );
    bw->setCaption("Solitaire Marbles");
    bw->show();
    return app.exec();
}

TopWindow::TopWindow( QWidget *parent, const char *name)
    : QWidget( parent, name )
{
    createStatusBar();

    // setup the quit button
    b_quit  = new QPushButton( "&Quit", this, "Quit" );
    b_sound = new QRadioButton( "Sound", this );
    b_new   = new QPushButton( "New", this );
    b_filesave = new QPushButton( "&Save", this );
    b_fileload = new QPushButton( "&Load", this );
    b_prev  = new QPushButton( "&Prev", this );
    b_next  = new QPushButton( "&Next", this );

    b_quit->setFlat(true);
    b_new->setFlat(true);
    b_filesave->setFlat(true);
    b_fileload->setFlat(true);

    canvas = new QCanvas(280,280);
    canvas->setDoubleBuffering(TRUE);
    CanvasView = new Maya_Marbles(canvas,this,"CanvasView",b_sound, b_prev, b_next);
    CanvasView->setFocusPolicy(QWidget::StrongFocus);

    QGridLayout *vlay = new QGridLayout( this, 1, 1 );
    vlay->setResizeMode( QLayout::Fixed );
    vlay->addMultiCellWidget( CanvasView,  10, 10, 0, 5, Qt::AlignCenter );
    vlay->addWidget(b_quit, 0, 0 );
    vlay->addWidget(b_new, 0, 1 );
    vlay->addWidget(b_prev, 0, 2 );
    vlay->addWidget(b_next, 0, 3 );
    vlay->addWidget(b_filesave, 1, 0 );
    vlay->addWidget(b_fileload, 1, 1 );
    vlay->addWidget(b_sound, 0, 5 );
    vlay->addMultiCellWidget(statusBar,11,12,0,5);

    connect( b_quit, SIGNAL( clicked() ), qApp, SLOT( quit() ) );
    connect( b_new, SIGNAL( clicked() ),  CanvasView, SLOT( new_game() ) );
    connect( b_filesave, SIGNAL( clicked() ),  CanvasView, SLOT( filesave() ) );
    connect( b_fileload, SIGNAL( clicked() ),  CanvasView, SLOT( fileload() ) );
}

TopWindow *TopWindow::instance( QWidget *parent, const char * name )
{
    static TopWindow *singleton_topwindow=0;
    if(!singleton_topwindow)
        singleton_topwindow = new TopWindow(parent,name);
    return singleton_topwindow;
}

void TopWindow::createStatusBar()
{
    statusBar = new QStatusBar(this,"");
    statusBar->message("QT Marble Solitaire, GPL(C) moshahmed/at/gmail");
}

void Maya_Marbles::new_game(void){
    TopWindow::instance()->statusBar->message("New Game");
    m_board->StartGame();
}

void Maya_Marbles::fileload(void){
    QStatusBar *qs = TopWindow::instance()->statusBar;

    QString filename = QFileDialog::getOpenFileName(".", "Checker moves (*.*)", this);
    if (filename.isEmpty())
        return;
    QFile f( filename );
    if ( !f.open( IO_ReadOnly ) ) {
        qs->message( QString("Could not read %1").arg(filename), 2000 );
        return;
    }
    QTextStream t( &f );
    m_board->ApplyFile(t);
    f.close();
}

void Maya_Marbles::filesave(void){
    QStatusBar *qs = TopWindow::instance()->statusBar;
    QString filename = QFileDialog::getSaveFileName(".", "Checker moves (*.*)", this);
    if (filename.isEmpty())
        return;

    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
        qs->message( QString("Could not write to %1").arg(filename), 2000 );
        return;
    }

    QTextStream t( &f );
    m_board->hist->SaveToFile(t);
    f.close();

    qs->message(QString("Saved to file %1").arg(filename),2000);
}

CPegBackground::CPegBackground(QCanvas *canvas,int gx, int gy, QCanvasPixmapArray *pm)
    : CPeg(canvas,pm,gx,gy,-1)
{
}

// Class Board

void Board::redo_undo( int dir ){
    int gx0,gy0,gx1,gy1;
    CPeg *c1, *d1;
    //Debug("x");
    if(! hist->redo_undo(gx0,gy0,gx1,gy1,c1,d1, dir ) )
        return;
    if( dir == -1 ){
        if( c_board[gx0][gy0] || ! c_board[gx1][gy1] ||
            !c1 || !d1
        ) return; // assert(0);
        d1->toggleshow();
        c_board[d1->m_gx][d1->m_gy] = d1;
        c_board[gx0][gy0] = c1;
        c_board[gx1][gy1] = 0;
        c1->m_gx = gx0;
        c1->m_gy = gy0;
        int xx,yy;
        c1->scale(gx0,gy0,xx,yy,true); // go back.
        c1->setX(xx);
        c1->setY(yy);
        m_canvas->update();
    }else if( dir == +1 ){
         if( !c_board[gx0][gy0] || c_board[gx1][gy1] ||
             !c1 ||  !d1
         ) return; // assert(0);
        int xx,yy;
        d1->hide();
        c_board[d1->m_gx][d1->m_gy] = 0;
        c_board[gx1][gy1] = c1;
        c_board[gx0][gy0] = 0;
        c1->m_gx = gx1;
        c1->m_gy = gy1;
        c1->scale(gx1,gy1,xx,yy,true); // go back.
        c1->setX(xx);
        c1->setY(yy);
        m_canvas->update();
    }
}

void Board::MakeBackground(void){
    for(gy=0;gy<GY;gy++){
        for(gx=0;gx<GX;gx++){
            QCanvasPixmapArray *pm;
            if( onboard(gx,gy) > 0 ) pm = m_back[0];
            else                     pm = m_back[1];
            background[gx][gy]= new CPegBackground(m_canvas,gx,gy,pm);
        }
    }
}

void Board::StartGame(void){

    if(hist)
        hist->new_hist();

    for(gy=0;gy<GY;gy++)
        for(gx=0;gx<GX;gx++){
            CPeg *cp = c_board[gx][gy];
            if( cp )
                delete cp;
        }

    for(gy=0;gy<GY;gy++){
        for(gx=0;gx<GX;gx++){
            int color = onboard(gx,gy);
            if( color > 0 && color <= 5 && !(gx==GX/2 && gy==GX/2) )
                c_board[gx][gy]= new CPeg(m_canvas,m_png[color],gx,gy,1);
            else
                c_board[gx][gy]=0;
        }
    }
    m_canvas->update();
}

Board::Board(QCanvas *canvas, QPushButton *b_prev, QPushButton *b_next )
: hist(0)
{
    m_canvas = canvas;

    m_back[0] = new QCanvasPixmapArray("images/board1.png");
    m_back[1] = new QCanvasPixmapArray("images/board2.png");

    MakeBackground();

    m_png[1] = new QCanvasPixmapArray("images/Marble-peg1.png");
    m_png[2] = new QCanvasPixmapArray("images/Marble-peg2.png");
    m_png[3] = new QCanvasPixmapArray("images/Marble-peg3.png");
    m_png[4] = new QCanvasPixmapArray("images/Marble-peg4.png");
    m_png[5] = new QCanvasPixmapArray("images/Marble-peg5.png");
    m_png[6] = new QCanvasPixmapArray("images/Marble-peg6.png");

    hist = new History( b_prev, b_next );

    connect( b_prev, SIGNAL(clicked()), this, SLOT(prev()) );
    connect( b_next, SIGNAL(clicked()), this, SLOT(next()) );

    for(gy=0;gy<GY;gy++)
        for(gx=0;gx<GX;gx++)
            c_board[gx][gy] = 0;

    StartGame();
}

CPeg::CPeg(QCanvas* canvas,QCanvasPixmapArray *pm,int gx, int gy, int z)
    : QCanvasSprite(0,canvas), m_gx(gx), m_gy(gy)
{
    setSequence(pm);
    setAnimated(TRUE);
    int xx, yy;
    scale(gx,gy,xx,yy,z>0);
    setX(xx); setY(yy); setZ(z);
    show();
}

/*
 The colored pegs on the board and the indices (0 is empty).

  \ gx 01234567=GX
 gy\
   0   0022200
   1   0022200
   2   1155533
   3   1155533
   4   1155533
   5   0044400
   6   0044400
   7=GY

*/

int Board::onboard(int gx,int gy) const {
    if( gx < 0 || gx > GX || gy < 0 || gy > GY )
        return 0;
    if( 2 <= gx  && gx < 5 )
        if( gy < 2 ) return 2;
        else if( gy < 5 ) return 5;
        else return 4;
    if( 2 <= gy  && gy < 5 )
        if( gx < 2 ) return 1;
        else if( gx < 5 ) return 5;
        else return 3;
    return 0;
}

Maya_Marbles::Maya_Marbles(  QCanvas *pcanvas, QWidget *parent, const char *name,
    QRadioButton *b_sound, QPushButton *b_prev, QPushButton *b_next  )
    : m_board(0),m_dragging(0),m_gong(0),
      QCanvasView( pcanvas, parent, name ),
      canvas(pcanvas)
{
    setCaption( tr( "Maya_Marbles" ) );
    setIconText( tr( "Maya_Marbles" ) );
    setIcon( QPixmap("images/Marble-peg2.png") );
    setName("Maya_Marbles");
    QWhatsThis::add( this, tr( "Maya_Marbles, the game." ) );
    QFont f( font() );
    f.setFamily( "Fixedsys" );
    setFont( f );
    setCursor( crossCursor );
    setMouseTracking( TRUE );
    setFocusPolicy( QWidget::NoFocus );
    canvas->setBackgroundColor( QColor(200,100,60) ) ;

    m_board = new Board(canvas, b_prev, b_next);

    m_gong = new SoundPlayer(b_sound);
    // refresh.
    resize(350,550);
    canvas->setAllChanged();
    show();
}

// Mouse Handlers.
void Maya_Marbles::contentsMousePressEvent( QMouseEvent * e ){
    if (e->button() == RightButton) {
        return;
    }
    QCanvasItemList il = canvas->collisions( e->pos() );
    for( QCanvasItemList::Iterator it=il.begin(); it!=il.end(); ++it ) {
        QCanvasItem *ci = *it;
        if( ci->z() < 0 ) // don't touch the background
            continue;

        if( ci->rtti() == CPeg::RTTI ){
            m_dragging = (CPeg*)ci;
            m_dragging->setX((int)e->x());
            m_dragging->setY((int)e->y());
            m_gong->gong(3);
            break;
        }
    }
}
void Maya_Marbles::contentsMouseReleaseEvent( QMouseEvent *e ) {
    if( m_dragging && m_dragging->rtti() == CPeg::RTTI ){
        m_board->gotoxy(m_dragging,e->x(),e->y(), m_gong);
        m_dragging = 0;
        canvas->update();
    }
}
void Maya_Marbles::contentsMouseMoveEvent( QMouseEvent *e ) {
    if( m_dragging ) {
        m_dragging->setX( e->x() );
        m_dragging->setY( e->y() );
        canvas->update();
    }
}
void Maya_Marbles::contentsMouseDoubleClickEvent( QMouseEvent * e ){
}
void Maya_Marbles::wheelEvent(QWheelEvent * e){ // not used for now.
    e->ignore();
}

void Maya_Marbles::keyPressEvent( QKeyEvent *e ){
    switch( e->key() ){
        case Key_Left : m_board->redo_undo( -1 ); break;
        case Key_Right: m_board->redo_undo( +1 ); break;
    }
}

void CPeg::scale(int gx,int gy, int& xx, int& yy, bool center){
    xx = gx *40;
    yy = gy *40;
    if( center )
        xx += 10, yy += 10;
}

void CPeg::rscale(int xx,int yy, int& gx, int& gy){
    gx = xx/40;
    gy = yy/40;
}

int CPeg::moveok(int gx0, int gy0, int gx1, int gy1, int& midx, int& midy, int& jump_len){
    jump_len = 0;
    if( gx0 == gx1 )
        jump_len = abs(gy0-gy1);
    if( gy0 == gy1 )
        jump_len = abs(gx0-gx1);
    midx = (gx0+gx1)/2;
    midy = (gy0+gy1)/2;
    if( jump_len == 1 || jump_len == 2 )
        return jump_len;
    return 0;
}

void Board::gotoxy(CPeg *cp, int xx, int yy, SoundPlayer *s_gong){
    int gx0,gy0,gx1,gy1,midx,midy,jump_len;
    gx0 = cp->m_gx;
    gy0 = cp->m_gy;
    ASSERT( c_board[gx0][gy0] == cp );
    cp->rscale(xx,yy,gx1,gy1);

    cp->moveok(gx0,gy0,gx1,gy1,midx,midy,jump_len);

    // already occupied? cannot move?
    if( !onboard(gx1,gy1) || c_board[gx1][gy1] != 0 ){
        TopWindow::instance()->statusBar->message("Cannot move there");
        cp->scale(gx0,gy0,xx,yy,true); // go back.
    }else if( jump_len==2 && c_board[midx][midy]){
        CPeg *cmid = c_board[midx][midy];
        cmid->hide();
        cp->scale(gx1,gy1,xx,yy,true);
        cp->m_gx = gx1;
        cp->m_gy = gy1;
        c_board[gx1][gy1] = cp;
        c_board[gx0][gy0] = 0;
        c_board[midx][midy]=0;

        hist->played(gx0,gy0,gx1,gy1,cp,cmid);
        s_gong->gong(1);
    }else{
        TopWindow::instance()->statusBar->message("Cannot move there");
        cp->scale(gx0,gy0,xx,yy,true); // go back.
    }
    cp->setX(xx);
    cp->setY(yy);
    m_canvas->update();
}

// Class SoundPlayer

SoundPlayer::SoundPlayer(QRadioButton *b_sound)
    : s_sound(b_sound)
{
    if(!QSound::isAvailable()){
        s_sound->setEnabled(false);
        QMessageBox::warning(this,"No soundsystem","No soundsystem");
    }
}

void SoundPlayer::gong(int i){
    if(!s_sound->isOn())
        return;
    switch(1+i%4){
        case 1: QSound::play("sounds/1.wav"); break;
        case 2: QSound::play("sounds/2.wav"); break;
        case 3: QSound::play("sounds/3.wav"); break;
        case 4: QSound::play("sounds/4.wav"); break;
    }
}

// Class History

History::History(QPushButton *b_prev, QPushButton *b_next) :
    cmoves(0), current(0), h_prev(b_prev), h_next(b_next)
{
    h_prev->setEnabled(false);
    h_next->setEnabled(false);

    h_prev->setFlat(true);
    h_next->setFlat(true);
}

void History::played(int gx0, int gy0, int gx1, int gy1, CPeg*c1, CPeg*d1 )
{
    cmoves.resize( current );
    cmoves.push_back( new CMove(gx0,gy0,gx1,gy1,c1,d1) );
    current = cmoves.size();

    h_prev->setEnabled(current>0);
    h_next->setEnabled(current<cmoves.size());

    QString status;
    status.sprintf("Move %d from (%d,%d) to (%d,%d).",
        current,gx0,gy0,gx1,gy1);
    TopWindow::instance()->statusBar->message(status);
}

bool History::redo_undo( int& gx0, int& gy0, int& gx1, int& gy1, CPeg*&c1, CPeg*&d1, int dir )
{
    int csize = cmoves.size();
    if( cmoves.empty() ){
        TopWindow::instance()->statusBar->message("No history");
        return false;
    }
    CMove *cm=0;

    QString status;
    if( dir == -1  && current > 0 ){
        cm = cmoves[--current];
        status.sprintf("History %d of %d.",current,csize);
    }else if( dir == +1 && current < cmoves.size()  ){
        cm = cmoves[current++];
        status.sprintf("History %d of %d.",current,csize);
    }else{
        if( dir == -1 )
            status.sprintf("Beginning of history %d of %d",current,csize);
        if( dir == +1 )
            status.sprintf("End of history %d of %d",current,csize);
    }
    TopWindow::instance()->statusBar->message(status);
    if(!cm)
        return false;
    gx0 = cm->fromx; gy0 = cm->fromy;
    gx1 = cm->tox;   gy1 = cm->toy;
    c1  = cm->moved; d1  = cm->under;

    h_prev->setEnabled(current>0);
    h_next->setEnabled(current<cmoves.size());

    return true;
}

void History::SaveToFile(QTextStream& t){
    for( std::vector<CMove*>::iterator it = cmoves.begin(); it != cmoves.end(); ++it ){
        QString line;
        line.sprintf("%d %d %d %d\n", (*it)->fromx, (*it)->fromy, (*it)->tox, (*it)->toy);
        t << line;
    }
}

void Board::mysleep(int ms_delay) const {
    QTime t;
    t.start();
    while (t.elapsed() < ms_delay)
        ;
}

void Board::ApplyFile(QTextStream& t){
    QStatusBar *qs = TopWindow::instance()->statusBar;
    int gx0,gy0,gx1,gy1,midx,midy,jump_len,moves=0;
    while( !t.atEnd() ){
        t >> gx0 >> gy0 >> gx1 >> gy1;

        if( !CPeg::moveok(gx0,gy0,gx1,gy1,midx,midy,jump_len) ||
            !onboard(gx0,gy0) || c_board[gx0][gy0]   == 0 ||
            !onboard(gx1,gy1) || c_board[gx1][gy1]   != 0 ||
            jump_len !=2      || c_board[midx][midy] == 0
        ){
            QString err;
            err.sprintf("Illegal move %d (%d,%d) to (%d,%d)", moves, gx0,gy0,gx1,gy1);
            qs->message(err);
            return;
        }
        moves++;
        CPeg *cp = c_board[gx0][gy0];
        CPeg *cmid = c_board[midx][midy];
        cmid->hide();

        cp->m_gx = gx1;
        cp->m_gy = gy1;

        c_board[gx1][gy1] = cp;
        c_board[gx0][gy0] = 0;
        c_board[midx][midy]=0;

        int xx,yy;
        cp->scale(gx1,gy1,xx,yy,true); // go back.

        int x0=cp->x(), y0=cp->y(),i;
        const int steps=10;
        for(i=0;i<steps+1;i++){
            int x2 = x0 + (xx-x0)*i/steps;
            int y2 = y0 + (yy-y0)*i/steps;
            cp->setX(x2);
            cp->setY(y2);
            m_canvas->update();
            mysleep(10);
        }
        hist->played(gx0,gy0,gx1,gy1,cp,cmid);
    }
    qs->message(QString("Loaded %1 moves").arg(moves));
}

// EOF
