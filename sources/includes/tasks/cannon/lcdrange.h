#ifndef LCDRANGE_H
#define LCDRANGE_H
#include <QWidget> 
#include <QSlider>
#include <QLabel>


class LCDRange: public QWidget{
    Q_OBJECT
public:
    LCDRange(QWidget *parent=0, const char *name=0);
    LCDRange(const char *s, QWidget *parent=0, const char *name=0);
    QString text() const;
    int value() const;
public slots:
    void setValue(int);
    void setRange(int minVal, int maxVal);
    void setText(const char *);
signals:
    void valueChanged(int);
private:
    void init();
    QLabel *label;
    QSlider *slider;
};


#endif // LCDRANGE_H
