#include "lcdrange.h"
#include <qslider.h>
#include <qlcdnumber.h>
#include <QWidget> 
#include <QVBoxLayout>
#include <QLabel>

LCDRange::LCDRange(QWidget* parent, const char *name) : QWidget(parent){
    init();
}

LCDRange::LCDRange(const char *s, QWidget* parent, const char *name) : QWidget(parent){
    init();
    setText(s);
}

void LCDRange::init(){
    // define layout, lcd, slider and label
    QLCDNumber *lcd = new QLCDNumber(2, this);
    slider = new QSlider(Qt::Horizontal, this);
    QVBoxLayout *layout = new QVBoxLayout(this);
    label = new QLabel(this);

    //set layout, lcd, slider and label
    slider-> setRange(0, 99);
    slider-> setValue(0);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(lcd);
    layout->addWidget(slider);
    layout->addWidget(label);

    //connect slider and lcd
    connect(slider, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)));
    connect(slider, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)));
    
    //set slider as proxy
    setFocusProxy(slider);
}

int LCDRange::value() const {
    return slider->value();
}

QString LCDRange::text() const{
    return label->text();
}

void LCDRange::setValue(int value){
    slider->setValue(value);
}

void LCDRange::setRange(int minVal, int maxVal){
    if (minVal < 0 || maxVal > 99 || minVal > maxVal){
        qWarning( "LCDRange::setRange(%d,%d)\n"
               "\tRange must be 0..99\n"
               "\tand minVal must not be greater than maxVal",
               minVal, maxVal );
        return;
    }
    slider-> setRange(minVal, maxVal);
}

void LCDRange::setText(const char *s){
    label->setText(s);
}