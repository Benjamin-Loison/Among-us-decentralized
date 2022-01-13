#include <qapplication.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <QWidget> 
#include <QGridLayout> 
#include "lcdrange.h"
#include "cannon.h"
#include <QTimer> 
#include <QVBoxLayout>
#include <QHBoxLayout>

class MyWidget: public QWidget{
    public:
        MyWidget(QWidget *parent=0, const char *name=0);
        QPushButton *quit;
        CannonField *cannonField;
};

