#include <qapplication.h>
#include <qpushbutton.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <QWidget> 
#include <QGridLayout> 
#include "lcdrange.h"
#include "cannon.h"
#include <QTimer> 
#include "mywidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>



MyWidget::MyWidget(QWidget *parent, const char *name) : QWidget(parent){
    quit = new QPushButton("quit");
    quit->setFont(QFont("Times", 18, QFont::Bold));
    
    LCDRange *angle = new LCDRange(this);
    angle->setRange(5,70);
    
    LCDRange *force = new LCDRange(this);
    force->setRange(10, 50);

    cannonField = new CannonField(this);

    connect(angle, SIGNAL(valueChanged(int)), cannonField, SLOT(setAngle(int)));
    connect(cannonField, SIGNAL(angleChanged(int)), angle, SLOT(setValue(int)));
    connect(force, SIGNAL(valueChanged(int)), cannonField, SLOT(setForce(int)));
    connect(cannonField, SIGNAL(forceChanged(int)), force, SLOT(setValue(int)));

    QPushButton *shoot = new QPushButton( "Shoot", this);
    shoot->setFont(QFont("Times", 18, QFont::Bold));

    connect(shoot, SIGNAL(clicked()), cannonField, SLOT(shoot()));

    QPushButton *new_target = new QPushButton("New Target", this);
    new_target->setFont(QFont("Times", 18, QFont::Bold));

    connect(new_target, SIGNAL(clicked()), cannonField, SLOT(newTarget()));





    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(quit, 0, 0);
    grid->addWidget(cannonField, 1, 1);
    grid->setColumnStretch(1, 5);

    QVBoxLayout *leftBox = new QVBoxLayout(this);
    grid->addLayout(leftBox, 1, 0);
    leftBox->addWidget(angle);
    leftBox->addWidget(force);

    QHBoxLayout *topBox = new QHBoxLayout(this);
    grid->addLayout(topBox, 0, 1);
    topBox->addWidget(shoot);
    topBox->addWidget(new_target);
    topBox->addStretch(1);

    angle->setValue(45);
    force->setValue(25);
    angle->setFocus();
}

