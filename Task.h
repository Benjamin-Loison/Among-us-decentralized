#ifndef TASK_H
#define TASK_H

#include <QPoint>

enum TaskType{TASK_FIX_WIRING,TASK_ASTEROIDS,TASK_SWIPE};
enum TaskTime{TASK_COMMON, TASK_LONG, TASK_SHORT};

class Task {
public:
    TaskType taskType;
    QPoint location;
    bool finished;
    Task();
    Task(TaskType taskType, QPoint location);
    bool operator ==(Task const a) const;
};

QVector<Task> getRandomTasks(QString privateSaltedWithCommonRandom);
QVector<Task*> getTasksAsPointers(QVector<Task> tasks);

extern quint8 commonTasks, longTasks, shortTasks;

#endif
