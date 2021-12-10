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

int current_digit = 0  ;

QPixmap* EnterIDCodeBackgroundPixmap = nullptr;
QPixmap* currEnterIDCodePixmap = nullptr;
QLabel* currEnterIDCodeLabel = nullptr;


void randomCode(){
    for (quint8 i = 0; i< DIGIT_NUMBER; i++){
        quint8 randomdigit = QRandomGenerator::global()->bounded(MAX_DIGIT+1);
        code[i]=randomdigit;
    }

}

QString getStringofcode(quint8* c){
    QString string_code ;
    for (quint8 i = 0; i< DIGIT_NUMBER; i++){
        if (c[i]<DIGIT_UNDEFINED){
            QTextStream(& string_code) << c[i] ;
        }
    }
    return string_code;
}



//generates the painter and draw the code on the card
QPair<QPixmap*, QPainter*> getEnterIDCodePixmapPainter()
{
    QPixmap* pixmap = new QPixmap(EnterIDCodeBackgroundPixmap->size());
    QPainter* painter = new QPainter(pixmap);
    painter->drawImage(0, 0, EnterIDCodeBackgroundPixmap->toImage());
    
    QString s_code = getStringofcode(code); 
    QFont font = painter->font();
    font.setPixelSize(28);
    painter->setFont(font);
    painter-> drawText(260, 660, 100, 50,0,s_code);
    return qMakePair(pixmap, painter);
}


QLabel* getEnterIDCode(){
    if(!EnterIDCodeBackgroundPixmap) {EnterIDCodeBackgroundPixmap = getQPixmap("EnterIdCode_resized.png");};

    playSound("fix_wiring_task_open_sound.wav");
    QLabel* qFrame = new QLabel;
    QHBoxLayout* hbox = new QHBoxLayout(qFrame);
    QLabel* qLabel = new QLabel(qFrame);
    hbox->addStretch();
    hbox->addWidget(qLabel);
    hbox->addStretch();
    qFrame->setLayout(hbox);

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

    return qFrame;

} 

//return the digit clicked on the panel, can return 10 an 11 for the cancel and validate buttons 
quint8 DigitClicked(quint16 x, quint16 y)
{
    if (x>X_TOP_LEFT_1 && x < X_TOP_LEFT_1 + X_LENGTH && y > Y_TOP_LEFT_1 && y <Y_TOP_LEFT_1 + Y_LENGTH){
        return 1;
    }
    else if(x>X_TOP_LEFT_1+ X_LENGTH +X_SPACE && x<X_TOP_LEFT_1+ 2*X_LENGTH +X_SPACE && y > Y_TOP_LEFT_1 && y <Y_TOP_LEFT_1 + Y_LENGTH){
        return 2;
    }
    else if(x>X_TOP_LEFT_1+ 2*X_LENGTH +2*X_SPACE && x<X_TOP_LEFT_1+ 3*X_LENGTH +2*X_SPACE && y > Y_TOP_LEFT_1 && y <Y_TOP_LEFT_1 + Y_LENGTH){
        return 3;
    }
    else if(x>X_TOP_LEFT_1 && x < X_TOP_LEFT_1 + X_LENGTH&& y > Y_TOP_LEFT_1+Y_SPACE+Y_LENGTH && y <Y_TOP_LEFT_1 + Y_SPACE + 2*Y_LENGTH){
        return 4;
    }
    else if(x>X_TOP_LEFT_1+ X_LENGTH +X_SPACE && x<X_TOP_LEFT_1+ 2*X_LENGTH +X_SPACE && y > Y_TOP_LEFT_1+Y_SPACE+Y_LENGTH && y <Y_TOP_LEFT_1 + Y_SPACE + 2*Y_LENGTH){
        return 5;
    }
    else if(x>X_TOP_LEFT_1+ 2*X_LENGTH +2*X_SPACE && x<X_TOP_LEFT_1+ 3*X_LENGTH +2*X_SPACE && y > Y_TOP_LEFT_1+Y_SPACE+Y_LENGTH && y <Y_TOP_LEFT_1 + Y_SPACE + 2*Y_LENGTH){
        return 6;
    }
    else if(x>X_TOP_LEFT_1 && x < X_TOP_LEFT_1 + X_LENGTH && y > Y_TOP_LEFT_1+2*Y_SPACE+2*Y_LENGTH && y <Y_TOP_LEFT_1 + 2*Y_SPACE + 3*Y_LENGTH){
        return 7;
    }
    else if(x>X_TOP_LEFT_1+ X_LENGTH +X_SPACE && x<X_TOP_LEFT_1+ 2*X_LENGTH +X_SPACE && y > Y_TOP_LEFT_1+2*Y_SPACE+2*Y_LENGTH && y <Y_TOP_LEFT_1 + 2*Y_SPACE + 3*Y_LENGTH){
        return 8;
    }
    else if(x>X_TOP_LEFT_1+ 2*X_LENGTH +2*X_SPACE && x<X_TOP_LEFT_1+ 3*X_LENGTH +2*X_SPACE && y > Y_TOP_LEFT_1+2*Y_SPACE+2*Y_LENGTH && y <Y_TOP_LEFT_1 + 2*Y_SPACE + 3*Y_LENGTH){
        return 9;
    }
    else if(x>X_TOP_LEFT_1 && x < X_TOP_LEFT_1 + X_LENGTH && y > Y_TOP_LEFT_1+3*Y_SPACE+3*Y_LENGTH && y <Y_TOP_LEFT_1 + 3*Y_SPACE + 4*Y_LENGTH){
        return 10;
    }
    else if(x>X_TOP_LEFT_1+ X_LENGTH +X_SPACE && x<X_TOP_LEFT_1+ 2*X_LENGTH +X_SPACE && y > Y_TOP_LEFT_1+3*Y_SPACE+3*Y_LENGTH && y <Y_TOP_LEFT_1 + 3*Y_SPACE + 4*Y_LENGTH){
        return 0;
    }
    else if(x>X_TOP_LEFT_1+ 2*X_LENGTH +2*X_SPACE && x<X_TOP_LEFT_1+ 3*X_LENGTH +2*X_SPACE && y > Y_TOP_LEFT_1+3*Y_SPACE+3*Y_LENGTH && y <Y_TOP_LEFT_1 + 3*Y_SPACE + 4*Y_LENGTH){
        return 11 ;
    }
    else return 12;
}




//draw the code answer
void drawAnswer(QPainter* painter){
    QString s = getStringofcode(answer);
    QFont font = painter->font();
    font.setPixelSize(56);
    painter->setFont(font);
    painter-> drawText(WRITE_X,WRITE_Y,s);
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
        return;
    
    bool correct = false;

    quint8 digit = DigitClicked(mouseX,mouseY);

    if (digit<DIGIT_UNDEFINED){
        playSound("Enter_Id_Code_entering_number_sound.wav");
        answer[current_digit]=digit;
        current_digit++;
    }

    else if (digit==DIGIT_UNDEFINED){
        for (quint8 i = 0;i<DIGIT_NUMBER;i++){
            answer[i]=DIGIT_UNDEFINED;
        }
        current_digit=0;
    }

    else if (digit == DIGIT_UNDEFINED +1 ){
        correct = true;
        for (quint8 i =0;i<DIGIT_NUMBER;i++){
            correct = (code[i]==answer[i])&&(answer[i]<DIGIT_UNDEFINED)&&correct;
        };
        if (correct){
            qInfo()<< "correct";
            playSound("Enter_Id_Code_accepted_sound.wav");
        }
        else {
            qInfo()<< "failed";
            playSound("Enter_Id_Code_failed_sound.wav");
        }
    }


    drawAnswer(painter);

    delete painter;

    if(currEnterIDCodeLabel)
        currEnterIDCodeLabel->setPixmap(*qBackgroundPixmap);
    if(currEnterIDCodePixmap)
        delete currEnterIDCodePixmap;
    currEnterIDCodePixmap = qBackgroundPixmap;
    if (correct){
        playSound("task_Complete.wav");
        inGameUI->finishTask();
        inGameUI->closeTask();
    };
}


void onCloseEnterIDCode() {
    playSound("fix_wiring_task_close_sound.wav");
    resetCode();
    if(currEnterIDCodeLabel) {
        delete currEnterIDCodeLabel;
        currEnterIDCodeLabel = nullptr;
    }
    if(currEnterIDCodePixmap) {
        delete currEnterIDCodePixmap;
        currEnterIDCodePixmap = nullptr;
    }
}


void resetCode() {
    for(quint8 i = 0; i< DIGIT_NUMBER; i++){
        code[i] = DIGIT_UNDEFINED;
    }
}

