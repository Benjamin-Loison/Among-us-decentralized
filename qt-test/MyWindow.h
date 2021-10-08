#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QApplication>
#include <QLCDNumber>
#include <QSlider>
#include <QProgressBar>
#include <QPixmap>
#include <QLabel>
#include <QTimer>
#include <QDebug>

class MyWindow: public QWidget
{
    Q_OBJECT
public:
    MyWindow();
    void tick();

private:
    QPushButton *m_button;
    QLCDNumber *m_lcd;
    QProgressBar *m_progress;
    QSlider *m_slider;
    QLabel *m_image;
    QTimer *m_timer;
    int m_tick;
};
#endif // MYWINDOW_H
