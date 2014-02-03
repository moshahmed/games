// $Header: /cygdrive/c/cvs/repo/mosh/qt/qt_mastermind/qt_mastermind.h,v 1.13 2013-07-22 02:04:43 a Exp $
// AUTHOR: GPL(C) moshahmed 2005-04-29 Fri 21:15

#ifndef MASTERMIND_H
#define MASTERMIND_H

#include <qwidget.h>
#include <qcanvas.h>
#include <qwmatrix.h>
#include <qmessagebox.h>
#include <qmainwindow.h>
#include <qsound.h>
#include <qdatetime.h>
#include <qtooltip.h>
#include <qfile.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qtimer.h>

// All colored Pegs on the board are derived from QCanvasSprite,
// they only differ in rtti (runtime types are used instead of dynamic_cast
// to support various compilers besides gcc, bcc, vc).

class MMPeg : public QCanvasSprite {
private:
    int gx,gy,phint;
    int peg_color;
public:
    MMPeg(QCanvas*,int peg_color=0);
    int get_color(void){ return peg_color; }
    void place(int x,int y){ gx=x;gy=y; }
    int bx(void){ return gx; }
    int by(void){ return gy; }
    enum { max_pegs=6, RTTI=1234 }; // we have peg1..peg6.xpm
    virtual int rtti() const { return RTTI; }
    void TooTip(QWidget*,QString);
};

class MMHintPeg : public QCanvasSprite {
public:
    enum { RTTI=1236 };
    enum { eWhite=0,eBlack=1 };
    virtual int rtti() const { return RTTI; }
    MMHintPeg(QCanvas *c, int hint, int x, int y);
    void TooTip(QWidget*w,QString t);
};

class MMPegPicker : public MMPeg { // A peg with a different type.
public:
    enum { RTTI=1235 };
    virtual int rtti() const { return RTTI; }
    MMPegPicker(QCanvas*c,int peg_color);
};

// A singleton class.

class SoundPlayer : QMainWindow {
    Q_OBJECT
private:
    int enabled;
public:
    SoundPlayer();
    void gong(int i);
    void voice(QString filename);
    void Toggle(void);
    int is_enabled(void){return enabled; }
public slots:
    void doPlay1(void);
    void timeout(void);
};

// The abstract board where pegs are places.

class MMBoard {
  public:
    MMBoard(void);
    enum { GX=4, GY=11 };
    void ComputeHints(int more_hints,int& winning_ways);
    MMBoard *instance(void);
    void scale( int xx, int yy, int &sx, int &sy );
    void rscale( int gx, int gy, int& snapx,int& snapy );
    bool putpeg(int xx, int yy, MMPeg *pp, int& snapx, int& snapy );
    bool putpeg(int xx, int yy, MMPeg *pp);
    void putpeg_grid(int gx, int gy, MMPeg *pp );
    int pegat(int sx, int sy );
    void Randomize(void);
    void Clear(void);
    void AutoPlay(QCanvas *canvas);

    int hint_whites[GY],hint_blacks[GY];
    void ReadScore(SoundPlayer *player);
  private:
    int row_done[GY],final_row;
    int grid[GX][GY], gx,gy;
};

// Main widget is the CanvasView.

class Mastermind : public QCanvasView {
    Q_OBJECT
public:
    Mastermind(QCanvasView *parent=0, const char* name=0 );
    ~Mastermind();
private:
    void ShowHints(void);
    void ShowSolution(void);
    void MakeSolutionPegs(void);

    QCanvasItem *m_dragging;
    QCanvasText *m_mousepos, *m_score,*m_solution_label;
    int xoffset, yoffset;
    MMBoard *mmboard;
    MMHintPeg *HintSprite[2][MMBoard::GY][MMBoard::GX];
    QCanvasText *AddLabel(int x, int y, QString t, bool show);
    int more_hints, debugging, show_solution, winning_ways;
    SoundPlayer *m_gong;
    void ShowMousePosition( QMouseEvent *e, const char *t );
    MMPeg *m_SolutionPegs[MMBoard::GX];
protected:
    void contentsMousePressEvent( QMouseEvent* e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent * e );
    void wheelEvent(QWheelEvent *e );
private slots:
    void ToggleSolution();
    void ToggleSound();
    void ToggleMoreHints();
    void ToggleDebug();
    void ClearBoard();
    void NewGame();
    void AutoPlay();
};

#endif // MASTERMIND_H
// EOF
