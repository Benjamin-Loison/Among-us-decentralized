#include <QLabel>
#include <QRandomGenerator>
#include <QPainter>
#include <QMediaPlayer>
#include <QFileInfo>
#include <QDateTime>
#include <QHBoxLayout>
#include <QtMath>
#include <QPainterPath>
#include <QString>
#include <QtDebug>
#include "EnterIDCode.h"
#include "main.h"
#include "qPlus.h"

quint8 code[DIGIT_NUMBER] = {DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED};
quint8 answer[DIGIT_NUMBER] = {DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED,DIGIT_UNDEFINED};

int current_digit = 0;

QPixmap* EnterIDCodeBackgroundPixmap = nullptr;
QPixmap* currEnterIDCodePixmap = nullptr;
QLabel* currEnterIDCodeLabel = nullptr;

void randomCode(){
    for (quint8 i = 0; i< DIGIT_NUMBER; i++){
        quint8 randomdigit = QRandomGenerator::global()->bounded(MAX_DIGIT+1);
        code[i]=randomdigit;
    }
}

QString getStringOfCode(quint8* c){
    QString string_code ;
    for (quint8 i = 0; i< DIGIT_NUMBER; i++)
        if (c[i]<DIGIT_UNDEFINED)
            QTextStream(& string_code) << c[i] ;
    return string_code;
}

//generates the painter and draw the code on the card
QPair<QPixmap*, QPainter*> getEnterIDCodePixmapPainter()
{
    QPixmap* pixmap = new QPixmap(EnterIDCodeBackgroundPixmap->size());
    QPainter* painter = new QPainter(pixmap);
    painter->drawImage(0, 0, EnterIDCodeBackgroundPixmap->toImage());
    
    QString s_code = getStringOfCode(code);
    QFont font = painter->font();
    font.setPixelSize(28);
    painter->setFont(font);
    painter-> drawText(260, 660, 100, 50,0,s_code);
    return qMakePair(pixmap, painter);
}

QLabel* getEnterIDCode(){
    if(!EnterIDCodeBackgroundPixmap) {EnterIDCodeBackgroundPixmap = getQPixmap("EnterIdCode_resized.png");};

    playSound("fix_wiring_task_open");
    QLabel* qLabel = new QLabel;

    randomCode();

    QPair<QPixmap*, QPainter*> pixmapPainter = getEnterIDCodePixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;
    delete painter;
    qLabel->setPixmap(*pixmap);

    if(currEnterIDCodePixmap) {
        delete currEnterIDCodePixmap;
        currEnterIDCodePixmap = nullptr;
    }
    currEnterIDCodePixmap = pixmap;

    if(currEnterIDCodeLabel) {
        delete currEnterIDCodeLabel;
        currEnterIDCodeLabel = nullptr;
    }
    currEnterIDCodeLabel = qLabel;

    return qLabel;
} 

//return the digit clicked on the panel, can return CANCEL an VALIDATE for the cancel and validate buttons
quint8 digitClicked(quint16 x, quint16 y)
{
    for(quint8 yIndex = 0; yIndex < 4; yIndex++) // reversing the loop order would improve complexity ^^
    {
        quint16 topY = Y_TOP_LEFT_1 + yIndex * (Y_SPACE + Y_LENGTH),
                bottomY = topY + Y_LENGTH;
        if(topY < y && y < bottomY)
            for(quint8 xIndex = 0; xIndex < 3; xIndex++)
            {
                quint16 leftX = X_TOP_LEFT_1 + xIndex * (X_LENGTH + X_SPACE),
                        rightX = leftX + X_LENGTH;
                if(leftX < x && x < rightX)
                {
                    quint8 toReturn = yIndex * 3 + xIndex + 1;
                    if(toReturn >= 11)
                        if(toReturn-- == 11)
                            toReturn = 0;
                    return toReturn;
                }
            }
    }
    return DIGIT_UNDEFINED/*12*/;
}

//draw the code answer
void drawAnswer(QPainter* painter){
    QString s = getStringOfCode(answer);
    QFont font = painter->font();
    font.setPixelSize(56);
    painter->setFont(font);
    painter-> drawText(WRITE_X,WRITE_Y,s); /// using bold font may be a good idea
}

void onMouseEventEnterIDCode(QMouseEvent* mouseEvent)
{   
    QPair<QPixmap*, QPainter*> pixmapPainter = getEnterIDCodePixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QSize pixmapSize = qBackgroundPixmap->size(),
          windowSize = inGameUI->size();
    QPainter* painter = pixmapPainter.second;
    QPoint position = mouseEvent->pos();
    qint16 mouseY = position.y() - (windowSize.height() - pixmapSize.height()) / 2,
           mouseX = position.x() - (windowSize.width() - pixmapSize.width()) / 2;
    if(mouseX < 0 || mouseX >= pixmapSize.width() || mouseY < 0 || mouseY >= pixmapSize.height())
    {
        qInfo("get out");
        return;
    }
    
    bool correct = false;

    quint8 digit = digitClicked(mouseX,mouseY);

    if (digit<DIGIT_UNDEFINED){
        playSound("Enter_Id_Code_entering_number");
        answer[current_digit]=digit;
        current_digit++;
    }
    else if (digit==CANCEL){
        for (quint8 i = 0;i<DIGIT_NUMBER;i++)
            answer[i]=DIGIT_UNDEFINED;
        current_digit=0;
    }
    else if (digit == VALIDATE){
        correct = true;
        for (quint8 i =0;i<DIGIT_NUMBER;i++)
            if(!((code[i]==answer[i])&&(answer[i]<DIGIT_UNDEFINED)))
            {
                correct = false;
                break;
            }
        qInfo()<< (correct ? "correct" : "failed");
        playSound(correct ? "Enter_Id_Code_accepted" : "Enter_Id_Code_failed");
    }

    drawAnswer(painter);

    delete painter;

    if(currEnterIDCodeLabel)
        currEnterIDCodeLabel->setPixmap(*qBackgroundPixmap);
    if(currEnterIDCodePixmap)
        delete currEnterIDCodePixmap;
    currEnterIDCodePixmap = qBackgroundPixmap;
    if (correct){
        playSound("task_completed");
        inGameUI->finishTask();
        inGameUI->closeTask();
    }
}

void onCloseEnterIDCode() {
    playSound("fix_wiring_task_close");
    resetCode();
    if(currEnterIDCodeLabel)
        currEnterIDCodeLabel = nullptr;
    if(currEnterIDCodePixmap)
        currEnterIDCodePixmap = nullptr;
}

void resetCode() {
    for(quint8 i = 0; i< DIGIT_NUMBER; i++)
        code[i] = DIGIT_UNDEFINED;
}
