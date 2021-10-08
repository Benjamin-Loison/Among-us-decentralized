#include "MyWindow.h"

#include <QPushButton>

MyWindow::MyWindow() : QWidget() {
    setBaseSize(300,150);
    m_button = new QPushButton("Quitter", this);

    m_lcd = new QLCDNumber(this);
    m_lcd->setSegmentStyle(QLCDNumber::Flat);
    m_lcd->move(10,30);

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setGeometry(10, 60, 150, 20);

    m_progress = new QProgressBar(this);
    m_progress->setGeometry(10, 90, 200, 30);
    QObject::connect(m_slider, SIGNAL(valueChanged(int)), m_lcd, SLOT(display(int)));
    QObject::connect(m_slider, SIGNAL(valueChanged(int)), m_progress, SLOT(setValue(int)));
    QObject::connect(m_button, SIGNAL(clicked()), qApp, SLOT(quit()));

    m_image = new QPixmap("outline_search_black_24dp.png");
}
