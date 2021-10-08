#include "MyWindow.h"

#include <QPushButton>

const int fps = 30;

MyWindow::MyWindow() : QWidget() {
    setBaseSize(1000,500);
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

    //QPixmap m_pixmap("outline_search_black_24dp.png");
    QPixmap m_pixmap("Permis.jpg");
    m_image = new QLabel(this);
    m_image->setPixmap(m_pixmap);
    m_image->move(10,120);

    int timer_interval_ms = 1000/fps;
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MyWindow::tick);
    m_timer->start(timer_interval_ms);

    m_tick = 0;
}

void MyWindow::tick() {
    m_tick = (m_tick + 1) % (2*fps);
    qInfo() << "Ticked: " << m_tick;
    int mul = 20;
    if(m_tick < fps)
        m_image->move(10+m_tick*mul, 120);
    else
        m_image->move(10+fps*mul-mul*(m_tick-fps), 120);
}
