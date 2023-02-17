#include "lineedit_pinyin.h"
#include <QDebug>
#include <cstdio>
#include <QPoint>
#include <QKeyEvent>

using namespace std;


LineEdit_PinYin::LineEdit_PinYin(QWidget *parent) :
    QLineEdit(parent)
{
    this->installEventFilter(this);//注册捕获事件

    startFloatingWindow();
}

LineEdit_PinYin::~LineEdit_PinYin()
{
    this->W_Floating->close();
}

void LineEdit_PinYin::startFloatingWindow()
{
    W_Floating = new FloatingWindow;
    //输入字符（小写）
    connect(this,SIGNAL(sigPutChar(const char)),this->W_Floating,SLOT(slotPutChar(const char)));
    //其他（例：阿拉伯数字、空格、加号、减号、回车、删除）
    connect(this,SIGNAL(sigOther(QEvent *)),this->W_Floating,SLOT(slotOther(QEvent *)));
    //返回输入法不关心的信息
    connect(this->W_Floating,SIGNAL(sigRetEve(QEvent *)),this,SLOT(slotRetEve(QEvent *)));
    //返回用户最终选中的信息
    connect(this->W_Floating,SIGNAL(sigSetText(QString)),this,SLOT(slotSetText(QString)));
    //通知悬浮窗移动位置（相对显示器）
    connect(this,SIGNAL(sigMoveLocation(int,int)),this->W_Floating,SLOT(slotMoveLocation(int,int)));
}

bool LineEdit_PinYin::event(QEvent *e)
{



    if(e->type() == QEvent::Type::KeyPress)
    {
        QKeyEvent *ev = static_cast<QKeyEvent *>(e);
        unsigned int keyNum = ev->key();

        //        unsigned int maxOrMin = ev->nativeModifiers();
        //        if(maxOrMin==0x12 || maxOrMin==0x11)
        //        {
        //            qDebug()<<"大写";
        //            //大写
        //            if(keyNum <= 0x5A && keyNum >= 0x41)
        //                qDebug()<<(char)(keyNum);
        //            else
        //            {
        //                qDebug()<<(char)(keyNum);
        //            }
        //        }

        if(ev->nativeModifiers() == Qt::SquareCap)
        {
            //            qDebug()<<"小写";
            //小写
            if(keyNum <= Qt::Key_Z && keyNum >= Qt::Key_A)
            {
                QPoint point = this->mapToGlobal(this->pos());
                emit sigMoveLocation(point.x(),point.y()+this->height());
                const char key = (char)(keyNum + 32);
                emit this->sigPutChar(key);

            }
            else
            {
                //阿拉伯数字、空格、加号、减号、回车
                if((keyNum <= Qt::Key_9 && keyNum >= Qt::Key_0)
                        ||(keyNum == Qt::Key_Space)
                        ||(keyNum == Qt::Key_Equal)
                        ||(keyNum == Qt::Key_Minus)
                        ||(keyNum == Qt::Key_Backspace)
                        ||(keyNum == Qt::Key_Return))
                    emit sigOther(e);
                else
                {
                    QWidget::event(e);
                }
            }
        }
        else
            QWidget::event(e);
    }
    else
        QWidget::event(e);

    //    e->accept();
}

void LineEdit_PinYin::slotRetEve(QEvent *e)
{
    QWidget::event(e);
}

void LineEdit_PinYin::slotSetText(QString str)
{
    insert(str);
}

void LineEdit_PinYin::focusOutEvent(QFocusEvent *fe)
{
    emit W_Floating->hide();
}
