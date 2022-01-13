#include <QLabel>
#include <QRandomGenerator>
#include <QPainter>
#include <QFileInfo>
#include <QDateTime>
#include <QHBoxLayout>
#include <QVector>
#include <QtMath>
#include "asteroids.h"
#include "../main.h"
#include "../qPlus.h"
#include "../ui/InGameUI.h"
const double ASTEROID_VELOCITY_SEC = 200; // 200 pixels per second
const int TARGET_ASTEROID_COUNT = 10;
const int ASTEROID_SPAWN_INTERVAL_MS = 1000;
//qint16 asteroids[4][3],nbasteroids=2;
int nbAsteroids;
qint64 asteroidsLastSpawn = 0;
qint64 asteroidsLastUpdate = 0;
QVector<Asteroid> asteroids;
QVector<QPixmap*> asteroidPixmaps;
QPixmap* currAsteroidsPixmap = nullptr;
QLabel* currAsteroidsLabel = nullptr;

Asteroid::Asteroid():
    x(0), y(0), vx(0), vy(0), pixmap(nullptr) {}

Asteroid::Asteroid(int x, int y, double unit_x, double unit_y, QPixmap* pixmap):
    x(x), y(y), pixmap(pixmap) {
    double norm = sqrt(qPow(unit_x, 2)+qPow(unit_y, 2));
    unit_x /= norm;
    unit_y /= norm;
    vx = unit_x*ASTEROID_VELOCITY_SEC;
    vy = unit_y*ASTEROID_VELOCITY_SEC;
}

QPair<QPixmap*, QPainter*> getAsteroidsPixmapPainter()
{
    QPixmap* qBackgroundPixmap = getQPixmap(600,600);
    qBackgroundPixmap->fill(QColor(89, 206, 103, 153));
    QPainter* painter = new QPainter(qBackgroundPixmap);
    return qMakePair(qBackgroundPixmap, painter);
}

QLabel* getAsteroids(qint64 now)
{
    QLabel* qLabel = new QLabel;
    QPair<QPixmap*, QPainter*> pixmapPainter = getAsteroidsPixmapPainter();
    QPixmap* qBackgroundPixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;

    delete painter;
    qLabel->setPixmap(*qBackgroundPixmap);

    nbAsteroids = TARGET_ASTEROID_COUNT;
    if(currAsteroidsPixmap)
        delete currAsteroidsPixmap;
    currAsteroidsPixmap = qBackgroundPixmap;
    if(currAsteroidsLabel)
        delete currAsteroidsLabel;
    currAsteroidsLabel = qLabel;
    if(asteroidPixmaps.empty())
        asteroidPixmaps.push_back(getQPixmap("Asteroid.png"));
    asteroidsLastUpdate = now;
    return qLabel;
}

void replaceAsteroidsPixmap(QPixmap* pixmap) {
    if(currAsteroidsLabel)
        currAsteroidsLabel->setPixmap(*pixmap);
    if(currAsteroidsPixmap)
        delete currAsteroidsPixmap;
    currAsteroidsPixmap = pixmap;
}

void redrawAsteroids(qint64 now) {
    qint64 elapsed = now - asteroidsLastUpdate;
    asteroidsLastUpdate = now;
    QPair<QPixmap*, QPainter*> pixmapPainter = getAsteroidsPixmapPainter();
    QPixmap* pixmap = pixmapPainter.first;
    QPainter* painter = pixmapPainter.second;
    QVector<Asteroid> newAsteroids;
    for(Asteroid& asteroid : asteroids) {
        if(asteroid.destroyed)
            continue;
        asteroid.x += asteroid.vx * elapsed / 1000;
        asteroid.y += asteroid.vy * elapsed / 1000;
        int x = asteroid.x;
        int w = asteroid.pixmap->size().width();
        int asteroidLeft = x-w/2, asteroidRight = asteroidLeft+w;
        if(asteroidRight <= 0) // all asteroids move left
            continue;
        newAsteroids.push_back(asteroid);
    }
    asteroids = newAsteroids;

    if(now - asteroidsLastSpawn >= ASTEROID_SPAWN_INTERVAL_MS) {
        asteroidsLastSpawn = now;
        int xSpawn = 700;
        int ySpawn = QRandomGenerator::global()->bounded(-100, 700);
        int xDir = 300;
        int yDir = QRandomGenerator::global()->bounded(-100, 700);
        double unit_x = xDir - xSpawn, unit_y = yDir - ySpawn;
        int iPixmap = QRandomGenerator::global()->bounded(asteroidPixmaps.size());
        asteroids.push_back(Asteroid(xSpawn, ySpawn, unit_x, unit_y, asteroidPixmaps[iPixmap]));
    }

    for(Asteroid& asteroid : asteroids) {
        int x = asteroid.x, y = asteroid.y;
        QPixmap* pixmap = asteroid.pixmap;
        int w = pixmap->size().width(), h = pixmap->size().height();
        painter->drawPixmap(x-w/2, y-h/2, *pixmap);
    }

    int fontSizePt = 23;
    painter->setFont(QFont("Liberation Sans", fontSizePt));
    painter->setPen(Qt::white);
    painter->drawText(0, 13*fontSizePt/10, QString(QObject::tr("%1 left")).arg(nbAsteroids));

    delete painter;
    replaceAsteroidsPixmap(pixmap);
}

/**
 * Only pass mouse clicks (or double-clicks) to this function.
 */
void onMouseEventAsteroids(QMouseEvent* mouseEvent)
{
    int asteroidsWidth = currAsteroidsPixmap->size().width(),
        asteroidsHeight = currAsteroidsPixmap->size().height(),
        windowWidth = inGameUI->size().width(),
        windowHeight = inGameUI->size().height(),
        asteroidsLeft = (windowWidth - asteroidsWidth)/2,
        asteroidsTop = (windowHeight - asteroidsHeight)/2,
        mouseX = mouseEvent->x() - asteroidsLeft,
        mouseY = mouseEvent->y() - asteroidsTop;
    if(mouseX < 0 || mouseX >= asteroidsWidth || mouseY < 0 || mouseY >= asteroidsHeight)
        return;
    playSound("clear_asteroids_shooting");
    bool hit = false;
    for(Asteroid& asteroid : asteroids) {
        int x = asteroid.x, y = asteroid.y, w = asteroid.pixmap->size().width(), h = asteroid.pixmap->size().height();
        int left = x-w/2, top = y-h/2, right = left+w, bottom = top+h;
        if(mouseX >= left && mouseX < right && mouseY >= top && mouseY < bottom && !asteroid.destroyed) {
            asteroid.destroyed = true;
            nbAsteroids--;
            playSound(QString("clear_asteroids_asteroid_destroyed_%1").arg(QRandomGenerator::global()->bounded(3)));
            hit = true;
        }
    }
    if(nbAsteroids <= 0) {
        playSound("task_completed");
        inGameUI->finishTask();
        inGameUI->closeTask();
        inGameUI->checkEndOfTheGame();
    }
    else if(hit)
        playSound("task_in_progress");
}

void resetAsteroids() {
    nbAsteroids = TARGET_ASTEROID_COUNT;
    asteroids.clear();
}

void onCloseAsteroids() {
    resetAsteroids();
    if(currAsteroidsLabel)
        currAsteroidsLabel = nullptr;
    if(currAsteroidsPixmap) {
        delete currAsteroidsPixmap;
        currAsteroidsPixmap = nullptr;
    }
}
