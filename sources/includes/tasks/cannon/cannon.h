#ifndef CANNON_H
#define CANNON_H

#include <QWidget>
#include <QTimer> 
#include <QTime>

class CannonField: public QWidget{
    Q_OBJECT //necessary keyword for signal and slot connection
public:
    CannonField(QWidget *parent=0, const char *name=0);
    int angle() const{return ang;}
    int force() const{return f;}
    QSizePolicy sizePolicy() const;

public slots:
    void setAngle(int degrees);
    void setForce(int newton);
    void shoot();
    void newTarget();
private slots:
    void moveShot();
signals:
    void angleChanged(int);
    void forceChanged(int);
    void hit();
    void end();
protected:
    void paintEvent(QPaintEvent *);
private:
    void paintShot(QPainter *);
    void paintCannon(QPainter *);
    void paintTarget(QPainter *);
    QRect cannonRect() const;
    QRect shotRect() const;
    QRect targetRect() const;
    int ang;
    int f;
    int timerCount;
    QTimer *autoShootTimer;
    QTimer *randomTargetTimer;
    float shoot_ang;
    float shoot_f;
    QPoint target;
    int nhit;
};

#endif