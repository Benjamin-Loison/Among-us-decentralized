#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QPoint>

enum TaskType{TASK_FIX_WIRING,TASK_ASTEROIDS,TASK_SWIPE};

class Task : public QObject {
public:
    TaskType taskType;
    QPoint location;
    bool finished;
    Task();
    Task(TaskType taskType, QPoint location);
};

#endif