#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>
#include "floatingwindow.h"

class LineEdit_PinYin : public QLineEdit
{
    Q_OBJECT

public:
    explicit LineEdit_PinYin(QWidget *parent = 0);
    ~LineEdit_PinYin();

private:
    FloatingWindow* W_Floating;
    bool event(QEvent* e);
    void startFloatingWindow();
signals:
    void sigPutChar(const char c);
    void sigOther(QEvent*);
    void sigMoveLocation(int,int);
public slots:
    void slotRetEve(QEvent *e);
    void slotSetText(QString);
protected:
    void focusOutEvent(QFocusEvent *fe);
};

#endif // MYLINEEDIT_H
