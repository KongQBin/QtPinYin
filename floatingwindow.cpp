#include "floatingwindow.h"
#include "ui_floatingwindow.h"
#include <QDebug>


const char* cc_sys_dict = "/home/user/桌面/google_pinyin_lib/dict/dict_pinyin.dat";
const char* cc_usr_dict = "";
FloatingWindow::FloatingWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FloatingWindow)
{
    ui->setupUi(this);
    this->i_Wide = width();
    this->i_High = height();
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    connect(ui->TBtn_Backward,SIGNAL(clicked()),this,SLOT(slotBackward()));
    connect(ui->TBtn_Forward,SIGNAL(clicked()),this,SLOT(slotForward()));


    /**
     * 通过系统和用户字典文件名打开解码器引擎。
     * @param fn_sys_dict系统字典的文件名。
     * @param fn_usr_dict用户字典的文件名。
     * 如果成功打开解码器引擎，@return true。
     **/
    if(im_open_decoder(cc_sys_dict,cc_usr_dict))
    {
        qDebug()<<"输入法引擎打开成功";
        im_reset_search();
    }
    else
    {
        qDebug()<<"输入法引擎打开失败";
    }
    im_enable_shm_as_szm(true);
    im_enable_ym_as_szm(true);

    QS_CurrentText.clear();
    QSL_CurrentCandidateText.clear();
    i_QSL_CurrentCandidateText_Index = 0;
    this->HBL = new QHBoxLayout;
    this->ui->widget_2->setLayout(HBL);

    for(int i=0;i<MAX_ANNOUNCE_NUM;++i)
    {
        Label[i] = new QLabel;
        Label[i]->setMaximumWidth(15);
        Label[i]->setText(QString::number(i+1)+QString("."));

        Label_2[i]= new QLabel;

        HBL_L[i] = new QHBoxLayout;
        HBL_L[i]->addWidget(Label[i]);
        HBL_L[i]->addWidget(Label_2[i]);
        HBL_L[i]->setMargin(0);
        HBL->addLayout(HBL_L[i]);
    }
    HBL->setMargin(10);
}

FloatingWindow::~FloatingWindow()
{
    for(int i=0;i<MAX_ANNOUNCE_NUM;++i)
    {
        if(Label_2[i])
        {
            delete Label_2[i];
            Label_2[i] = NULL;
        }
        if(Label[i])
        {
            delete Label[i];
            Label[i] = NULL;
        }
        if(HBL_L[i])
        {
            delete HBL_L[i];
            HBL_L[i] = NULL;
        }
    }
    if(HBL)
    {
        delete HBL;
        HBL = NULL;
    }
    im_close_decoder();
    delete ui;
}
//增加字符
void FloatingWindow::slotPutChar(const char c)
{
    if(!this->isVisible())
        this->show();

    // 如果说用户的（右上角的）候选字已经选到一半，
    // 但用户又按下了字符键，则清空他的（右上角的）候选字。
    //（同Linux搜狗拼音做法保持一致）
    if(!b_CharOrOther)
    {
        QSL_CurrentHanZi.clear();
        ui->Label_CurrentHanZi->setText(QSL_CurrentHanZi.join(""));
        b_CharOrOther = true;
    }

    QS_CurrentText.append(c);

    QSL_CurrentCandidateText.clear();

    ui->Label_CurrentText->setText(QS_CurrentText);

    query();

    if(QSL_CurrentCandidateText.size())
        upDate();
}
//删除字符
void FloatingWindow::delChar()
{
    //    qDebug()<<this->isVisible();
    if(!this->isVisible()) return;

    QS_CurrentText.chop(1);

    QSL_CurrentCandidateText.clear();
    ui->Label_CurrentText->setText(QS_CurrentText);

    query();

    if(QSL_CurrentCandidateText.size())
        upDate();

    if(!QS_CurrentText.size())
    {
        this->hide();
    }
}
//上一页
void FloatingWindow::slotBackward()
{
    if(!i_QSL_CurrentCandidateText_Index)
        return;
    int backIndex = i_QSL_CurrentCandidateText_Index - MAX_ANNOUNCE_NUM;
    if(backIndex < 0) return;
    for(int i = backIndex;i<i_QSL_CurrentCandidateText_Index;++i)
    {
        Label_2[i - backIndex]->setText(QSL_CurrentCandidateText.at(i));
    }
    i_QSL_CurrentCandidateText_Index = backIndex;
}
//下一页
void FloatingWindow::slotForward()
{
    if(i_QSL_CurrentCandidateText_Index + MAX_ANNOUNCE_NUM >= QSL_CurrentCandidateText.size())
        return;
    int forIndex = i_QSL_CurrentCandidateText_Index + MAX_ANNOUNCE_NUM;
    for(int i = forIndex;i<forIndex+MAX_ANNOUNCE_NUM;++i)
    {
        if(i < QSL_CurrentCandidateText.size())
        {
            Label_2[i - forIndex]->setText(QSL_CurrentCandidateText.at(i));
        }
        else
            Label_2[i - forIndex]->setText("");
    }
    i_QSL_CurrentCandidateText_Index = forIndex;
}

void FloatingWindow::slotOther(QEvent *e)
{
    b_CharOrOther = false;
    if(!this->isVisible())
    {
        emit sigRetEve(e);
        return;
    }
    QKeyEvent *ev = static_cast<QKeyEvent *>(e);
    unsigned int keyNum = ev->key();
    if((keyNum <= Qt::Key_9 && keyNum >= Qt::Key_0)||keyNum == Qt::Key_Space)
    {
        int index;
        if(keyNum == Qt::Key_Space)
            index = 0;
        else
            index = keyNum - 48 - 1;//(-48转阿拉伯 -1转索引)
        if(index < QSL_CurrentCandidateText.size()-i_QSL_CurrentCandidateText_Index && index != -1)
        {
            //增加当前已固定候选字
            QSL_CurrentHanZi.append(QSL_CurrentCandidateText.at(i_QSL_CurrentCandidateText_Index+index));
            ui->Label_CurrentHanZi->setText(QSL_CurrentHanZi.join(""));
            //依照已固定候选字再进行搜索
            int num = im_choose(i_QSL_CurrentCandidateText_Index+index);

            QL_QSL_CurrentCandidateText.append(QStringList(QSL_CurrentCandidateText));
            QSL_CurrentCandidateText.clear();
            reSet_QSL_CurrentCandidateText(num);

            if(num == 1)
            {
                //                emit sigSetText(QSL_CurrentCandidateText.at(i_QSL_CurrentCandidateText_Index+0));
                emit sigSetText(QSL_CurrentCandidateText.at(0));
                this->hide();
            }
            if(QSL_CurrentCandidateText.size())
                upDate();
        }
    }
    else if(keyNum == Qt::Key_Equal)//=号键
        slotForward();
    else if(keyNum == Qt::Key_Minus)//_号键
        slotBackward();
    else if(keyNum == Qt::Key_Backspace)//backspace键
    {
        if(im_get_fixed_len())
        {
            //恢复历史（右上角）候选字
            QSL_CurrentHanZi.removeAt(QSL_CurrentHanZi.size()-1);
            ui->Label_CurrentHanZi->setText(QSL_CurrentHanZi.join(""));
            //恢复历史（下方的）待选字
            QSL_CurrentCandidateText = QL_QSL_CurrentCandidateText.at(QL_QSL_CurrentCandidateText.size()-1);
            QL_QSL_CurrentCandidateText.removeAt(QL_QSL_CurrentCandidateText.size()-1);
            if(QSL_CurrentCandidateText.size())
                upDate();
            int n = im_cancel_last_choice();
            if(!n)
                ui->Label_CurrentHanZi->setText("");
        }
        else
            delChar();
    }
    else if(keyNum == Qt::Key_Return)//Enter键
    {
        im_reset_search();
        QString tmp = QS_CurrentText;
        emit sigSetText(tmp);
        this->hide();
    }
    else
        return;

}

void FloatingWindow::slotMoveLocation(int x,int y)
{
    this->move(x,y);
}

void FloatingWindow::upDate()
{
    int currAnn = 0;
    if(QSL_CurrentCandidateText.size()>MAX_ANNOUNCE_NUM)
        currAnn = MAX_ANNOUNCE_NUM;
    else
        currAnn = QSL_CurrentCandidateText.size();
    for(int i=0;i<currAnn;++i)
    {
        Label_2[i]->setText(QSL_CurrentCandidateText.at(i));
    }
    HBL->setSpacing(1);
    i_QSL_CurrentCandidateText_Index = 0;
}

void FloatingWindow::query()
{
    im_reset_search();
    im_flush_cache();

    int num = im_search(QS_CurrentText.toLocal8Bit().data(),QS_CurrentText.size());
    reSet_QSL_CurrentCandidateText(num);
}

void FloatingWindow::reSet_QSL_CurrentCandidateText(int &maxNum)
{
    QString temp;
    int c16_HanZiLen_size = sizeof(c16_HanZiLen);
    for(int i=0;i<maxNum;++i)
    {
        temp.clear();
        memset(&c16_HanZiLen,0,c16_HanZiLen_size);
        if(im_get_candidate(i,c16_HanZiLen,c16_HanZiLen_size))
        {
            for(int j=0;j<MAX_A_ANNOUNCE_LEN;++j)
            {
                if(c16_HanZiLen[j])
                    temp.append(QString(QChar(*((ushort*)&c16_HanZiLen[j]))));
            }
            QSL_CurrentCandidateText.append(temp);
        }
        else
        {
            qDebug()<<"返回NULL";
        }
    }
}

void FloatingWindow::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton)
    {
        m_move = true; //记录鼠标的世界坐标.
        m_startPoint = event->globalPos(); //记录窗体的世界坐标.
        m_windowPoint = this->frameGeometry().topLeft();
    }
}

void FloatingWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        //移动中的鼠标位置相对于初始位置的相对位置.
        QPoint relativePos = event->globalPos() - m_startPoint; //然后移动窗体即可.
        this->move(m_windowPoint + relativePos );
    }
}

void FloatingWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        //改变移动状态.
        m_move = false;
    }
}

void FloatingWindow::hideEvent(QHideEvent *event)
{
    this->resize(QSize(i_Wide,i_High));
    QS_CurrentText.clear();
    QSL_CurrentHanZi.clear();
    QSL_CurrentCandidateText.clear();
    QL_QSL_CurrentCandidateText.clear();
    i_QSL_CurrentCandidateText_Index = 0;
    ui->Label_CurrentText->clear();
    ui->Label_CurrentHanZi->clear();
    QWidget::hideEvent(event);
}
