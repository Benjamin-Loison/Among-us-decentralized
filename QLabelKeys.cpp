#include "QLabelKeys.h"
#include "main.h"

QLabelKeys::QLabelKeys(QLabel* parent) : QLabel(parent)
{
    setWindowIcon(QIcon(assetsFolder + "logo.png")); // using an assets folder should be nice
    setWindowTitle("Among Us decentralized");

    x = 50;
    y = 0;
    display();
}

void QLabelKeys::display()
{
    QColor originalColors[2] = {QColor(0, 255, 0), QColor(255, 0, 0)},
           colors[7][2] = {{QColor(192, 201, 216), QColor(120, 135, 174)},
                           {QColor(20, 156, 20), QColor(8, 99, 64)},
                           {QColor(17, 43, 192), QColor(8, 19, 131)},
                           {QColor(102, 67, 27), QColor(87, 35, 21)},
                           {QColor(193, 17, 17), QColor(120, 8, 57)},
                           {QColor(62, 71, 78), QColor(30, 30, 38)},
                           {QColor(244, 244, 86), QColor(194, 134, 34)}};
    QPixmap* qPixmap = getQPixmap("player.png"),
           * qBackgroundPixmap = getQPixmap("mapCrop.png"); // "The Skeld"

    QImage tmp = qPixmap->toImage();
    quint8 colorsIndex = 2; // s'accorder sur de l'aléatoire en début de partie serait bien où bijection pseudo (s'il y en a un) au skin ?

    for(quint16 y = 0; y < tmp.height(); y++)
    {
        for(quint16 x = 0; x < tmp.width(); x++)
        {
            for(quint8 originalColorsIndex = 0; originalColorsIndex < 2; originalColorsIndex++)
            {
                if(tmp.pixelColor(x, y) == originalColors[originalColorsIndex])
                    tmp.setPixelColor(x, y, colors[colorsIndex][originalColorsIndex]);
            }
        }
    }

    *qPixmap = QPixmap::fromImage(tmp);

    QPainter* painter = new QPainter(qBackgroundPixmap);
    painter->drawPixmap(x, y, *qPixmap);

    painter->end();
    setPixmap(*qBackgroundPixmap);
    if(qLabel != nullptr && layout() == nullptr)
    {
        QHBoxLayout* hBox = new QHBoxLayout;
        hBox->addWidget(qLabel); // crash here ?
        setLayout(hBox);
    }
}

bool QLabelKeys::eventFilter(QObject* obj, QEvent* event)
{
    Q_UNUSED(obj)

    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        switch(key->key())
        {
            case Qt::Key_Down:
                y++;
                break;
            case Qt::Key_Up:
                y--;
                break;
            case Qt::Key_Left:
                x--;
                break;
            case Qt::Key_Right:
                x++;
                break;
            // https://nerdschalk.com/among-us-keyboard-controls/
            case Qt::Key_E:
                if(qLabel == nullptr)
                {
                    currentInGameGUI = IN_GAME_GUI_FIX_WIRING;
                    qLabel = getFixWiring();
                }
                else
                {
                    currentInGameGUI = IN_GAME_GUI_NONE;
                    //qLabel = new QLabel;
                    //qLabel->clear();
                    //free(qLabel);
                }
                break;
        }

        display();

        return true;
    }
    else if(event->type() == QEvent::MouseMove)
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if(qLabel != nullptr)
        {
            if(currentInGameGUI == IN_GAME_GUI_FIX_WIRING)
                onMouseEventFixWiring(mouseEvent);
        }
        return true;
    }
    return false;
}
