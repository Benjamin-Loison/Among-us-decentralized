#include "cannon.h"
#include <QPainter> 
#include <QPaintEvent> 
#include <iostream>
#include <QPixmap>
#include <QTimer> 
#include <cmath>
#include <QTime>


const QRect barrelRect(33, -4, 15, 8);

CannonField::CannonField(QWidget *parent, const char *name): QWidget(parent){
    //original value assignment
    ang = 45;
    f = 0;
    timerCount = 0;
    shoot_ang = 0;
    shoot_f = 0;
    target = QPoint(200, 0);
    nhit = 0;
    // define and connect the shooting delay(time interval between two painting update) 
    autoShootTimer = new QTimer(this);// the shooting delay is designed to connect with shoot button and moveshot function.
    connect(autoShootTimer, SIGNAL(timeout()), this, SLOT(moveShot()));//be cautious of the fact that this timer will update forever.
    connect(this, SIGNAL(hit()),this, SLOT(newTarget()));

/*
    randomTargetTimer = new QTimer(this);
    randomTargetTimer->setInterval(1000);
    connect(randomTargetTimer, SIGNAL(timeout()), this, SLOT(newTarget()));
    randomTargetTimer->start(1000);
*/
    // fill the background color
    setAutoFillBackground(true);
    setPalette(QPalette(Qt::white));
}

void CannonField::setAngle(int degrees){
    if (degrees < 5){
        degrees = 5;
    } else if (degrees > 70){
        degrees = 70;
    } else if (ang == degrees){
        return;
    }
    ang = degrees;
    repaint(cannonRect());
    emit angleChanged(ang);
}

void CannonField::setForce(int newton){
    if (newton < 0){
        newton = 0;
    }
    if (f == newton){
        return;
    }
    f = newton;
    emit forceChanged(f);
}

void CannonField::shoot(){
    if (autoShootTimer->isActive())
        return;
    timerCount = 0;
    shoot_ang = ang;
    shoot_f = f;
    autoShootTimer->start(20);
}

void CannonField::newTarget(){
    static bool first_time = true;
    if (first_time){
        first_time = false;
        QTime midnight(0, 0, 0);
        srand(midnight.secsTo(QTime::currentTime()));
    }
    QRegion *r = new QRegion(targetRect());
    target = QPoint(200 + rand() % 20, 10 +rand() % 20);
    repaint(r->united(targetRect()));
}

void CannonField::moveShot(){
    QRegion r(shotRect());//old region
    timerCount++;

    QRect shotR = shotRect();//new shot rectangle 
    // emit slots of shooting result
    
    if (shotR.intersects(targetRect())){
        autoShootTimer->stop();
        if (nhit == 2) {
            emit end();
        }
        emit hit();
        nhit++;
    } else if (shotR.x() > width() || shotR.y() > height()){
        autoShootTimer->stop();
    } else {
        //smoothen the trajectory
        //r = r.united(QRegion(shotR));
    }

    if (nhit != 3){
        repaint(r);
    }
}

void CannonField::paintEvent(QPaintEvent *e){
    QRect updateR = e->rect();
    QPainter p(this);

    //paint cannon, shots and target
    if (updateR.intersects(cannonRect())){
        paintCannon(&p);
    }
    if (autoShootTimer->isActive() && updateR.intersects(shotRect())){
        paintShot(&p);
    }
    if (updateR.intersects(targetRect())){
        paintTarget(&p);
    }
}

void CannonField::paintShot(QPainter *p){
    p->setBrush(Qt::black);
    p->setPen(Qt::NoPen);
    p->drawRect(shotRect());
}

void CannonField::paintTarget(QPainter *p){
    p->setBrush(Qt::red);
    p->setPen(Qt::black);
    p->drawRect(targetRect());
}

void CannonField::paintCannon(QPainter *p){
    QRect cr = cannonRect();
    QPixmap *pix = new QPixmap(cr.size());
    pix->fill();

    //begin to paint on pixmap
    QPainter tmp(pix);
    tmp.setBrush(Qt::green);
    tmp.setPen(Qt::blue);
    tmp.translate(0, pix->height());
    tmp.drawPie(QRect( -35,-35, 70, 70 ), 0, 90*16);
    tmp.rotate(-ang);
    tmp.drawRect(barrelRect);
    tmp.end();

    //begin to paint on widget
    p->drawPixmap( cr.topLeft(), *pix);
    
}

QRect CannonField::cannonRect() const{
    QRect *r= new QRect(0, 0, 50, 50);
    r->moveBottomLeft(this->rect().bottomLeft());
    return *r;
}

QRect CannonField::shotRect() const{
    //assign the basic factors of trajectory calculation.
    
    double gravity = 9.8;
    double time = timerCount/10.0; //division by 4 to slow down the process.
    double velocity = shoot_f;
    double radians = shoot_ang*3.14159265/180;

    double velx = velocity*cos(radians);
    double vely = velocity*sin(radians);
    double x0 = barrelRect.right()*cos(radians);
    double y0 = barrelRect.right()*sin(radians);
    
    //trajectory formula
    double x = x0 + velx*time;
    double y = y0 + vely*time - 0.5*gravity*time*time;

    QRect r = QRect(0, 0, 6, 6);
    r.moveCenter(QPoint(qRound(x), height() - 1 - qRound(y)));
    return r;
}


QRect CannonField::targetRect() const{
    QRect r(0, 0, 20, 10);
    r.moveCenter(QPoint(target.x(), height() - 1 - target.y()));
    return r;
}

QSizePolicy CannonField::sizePolicy() const{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

