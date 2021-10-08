#ifndef MYWINDOW_H
#define MYWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QApplication>
#include <QLCDNumber>
#include <QSlider>
#include <QProgressBar>
#include <QPixmap>

class MyWindow: public QWidget
{
    Q_OBJECT
public:
    MyWindow();

private:
    QPushButton *m_button;
    QLCDNumber *m_lcd;
    QProgressBar *m_progress;
    QSlider *m_slider;
    QPixmap *m_image;
};
#endif // MYWINDOW_H
