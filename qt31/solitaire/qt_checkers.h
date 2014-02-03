// $Header: /cygdrive/c/cvs/repo/mosh/qt/qt_checkers/qt_checkers.h,v 1.21 2013-07-22 02:04:43 a Exp $
// AUTHOR: GPL(C) moshahmed, 2005-05-07 Sat 21:59

#ifndef CHECKERS_H
#define CHECKERS_H
#include <qwidget.h>
#include <qcanvas.h>
#include <qwmatrix.h>
#include <qmessagebox.h>
#include <qmainwindow.h>
#include <qsound.h>
#include <qdatetime.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qmenubar.h>
#include <qtimer.h>
#include <vector>

class CPeg : public QCanvasSprite {
  private:
  public:
    enum { RTTI=1234 };
    virtual int rtti() const { return RTTI; }
    int m_gx,m_gy;
    CPeg(QCanvas* canvas,QCanvasPixmapArray *pm,int gx, int gy, int z);
    ~CPeg(){}
    void scale(int gx,int gy, int& xx, int& yy, bool center);
    void rscale(int xx,int yy, int& gx, int& gy);
    static int moveok(int gx0, int gy0, int gx1, int gy1, int& midx, int& midy, int& jump_len );
    void toggleshow(void){ if( isVisible() ) hide(); else show(); };
};

class CPegBackground : public CPeg {
    enum { RTTI=1235 };
  public:
    virtual int rtti() const { return RTTI; }
    CPegBackground(QCanvas *canvas,int gx, int gy, QCanvasPixmapArray *pm);
    ~CPegBackground(){ };
};

class SoundPlayer : QMainWindow {
    Q_OBJECT
  private:
    QRadioButton *s_sound;
  public:
    SoundPlayer(QRadioButton *b_sound);
    void gong(int i);
};

class History {

 class CMove {
   public:
    CPeg *moved, *under;
    int fromx,fromy,tox,toy;
    CMove(int gx0,int gy0, int gx1, int gy1, CPeg *c1, CPeg *d1 )
      : fromx(gx0),fromy(gy0),tox(gx1),toy(gy1),moved(c1),under(d1) { };
    ~CMove(){};
 };

private:
    std::vector<CMove*> cmoves;
    int current; // [0,1..size]
    QPushButton *h_prev, *h_next;
public:
    History(QPushButton *b_prev, QPushButton *b_next);
    ~History(){ };
    void new_hist(){ current=0; cmoves.clear(); }
    int get_current(void) const { return current; };
    void played(int gx0, int gy0, int gx1, int gy1, CPeg*c1, CPeg*d1 );
    bool redo_undo(int& gx0, int& gy0, int& gx1, int& gy1, CPeg*&c1, CPeg*&d1, int dir );
    void SaveToFile(QTextStream& t);
};

class Board : QObject {
    Q_OBJECT
  private:
    QCanvas *m_canvas;
    QCanvasPixmapArray *m_back[2], *m_png[7];
    int gx,gy;
    void MakeBackground(void);
    int  onboard(int gx, int gy) const;
    enum { GX=7,GY=7 };
    CPeg *c_board[GX][GY],*background[GX][GY];
    void mysleep(int ms_delay) const;
  public:
    History *hist;
    Board(QCanvas *canvas, QPushButton *b_prev, QPushButton *b_next);
    ~Board(){ };
    void Debug(QString mesg);
    void StartGame(void);
    void gotoxy(CPeg *cp, int xx, int yy, SoundPlayer *s_gong);
    void redo_undo( int dir );
    void ApplyFile(QTextStream& t);
  public slots:
    void next(void){ redo_undo(+1); };
    void prev(void){ redo_undo(-1); };
};

class Maya_Marbles : public QCanvasView {
    Q_OBJECT
  private:
    Board *m_board;
    CPeg *m_dragging;
    SoundPlayer *m_gong;
  public:
    QCanvas *canvas;
    Maya_Marbles(  QCanvas *pcanvas, QWidget *parent, const char *name,
            QRadioButton *b_sound, QPushButton *b_prev, QPushButton *b_next  );
    ~Maya_Marbles(){}
  protected:
    void contentsMousePressEvent( QMouseEvent* e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMouseDoubleClickEvent( QMouseEvent * e );
    void wheelEvent(QWheelEvent *e );
    void keyPressEvent( QKeyEvent *e );
  public slots:
    void new_game(void);
    void filesave(void);
    void fileload(void);
};

class TopWindow : public QWidget {
    Q_OBJECT
  public:
    TopWindow( QWidget *parent, const char * name );
    ~TopWindow(){}
    static TopWindow *instance( QWidget *parent=0, const char * name="" );

  private:
    QCanvas *canvas;
    QCanvasView *CanvasView;
    QPushButton *b_quit, *b_new, *b_prev, *b_next,*b_filesave,*b_fileload;
    QRadioButton *b_sound;
    void createStatusBar();
  public:
    QStatusBar *statusBar;
  public slots:

};

#endif /* CHECKERS_H */
