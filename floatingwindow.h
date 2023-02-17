#ifndef FLOATINGWINDOW_H
#define FLOATINGWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QToolButton>
#include <assert.h>
#include "googlepinyin/pinyinime.h"
#define MAX_ANNOUNCE_NUM 5      //最大显示待选字个数
#define MAX_A_ANNOUNCE_LEN 32   //最大待选汉字串长度
using namespace ime_pinyin;


namespace Ui {
class FloatingWindow;
}

class FloatingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingWindow(QWidget *parent = 0);
    ~FloatingWindow();

protected:
    bool m_move;
    QPoint m_startPoint;
    QPoint m_windowPoint;
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void hideEvent(QHideEvent *event);
private:
    Ui::FloatingWindow *ui;
    int i_High,i_Wide;
    bool b_CharOrOther;
    QString QS_CurrentText;
    QStringList QSL_CurrentHanZi;
    int i_QSL_CurrentCandidateText_Index;
    QStringList QSL_CurrentCandidateText;
    QList<QStringList> QL_QSL_CurrentCandidateText;
    QHBoxLayout *HBL;
    QHBoxLayout *HBL_L[MAX_ANNOUNCE_NUM];
    QLabel *Label[MAX_ANNOUNCE_NUM];
    QLabel *Label_2[MAX_ANNOUNCE_NUM];
    char16 c16_HanZiLen[MAX_A_ANNOUNCE_LEN];
    void upDate();
    void query();
    void delChar();
    void reSet_QSL_CurrentCandidateText(int &maxNum);
signals:
    void sigRetEve(QEvent *);
    void sigSetText(QString);
public slots:
    void slotPutChar(const char c);
    void slotBackward();
    void slotForward();
    void slotOther(QEvent *e);
    void slotMoveLocation(int,int);
};

#endif // FLOATINGWINDOW_H
