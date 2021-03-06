#ifndef TASK_H
#define TASK_H

#include <QPoint>
#include "../map/Player.h"

enum TaskType{TASK_FIX_WIRING,TASK_ASTEROIDS,TASK_SWIPE, TASK_ENTER_ID_CODE,TASK_ALIGN_ENGINE};
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

QVector<Task> getRandomTasks(QString privateSaltedWithCommonRandom, Map map);
QVector<Task*> getTasksAsPointers(QVector<Task> tasks);
QString taskTimeToString(TaskTime taskTime);
TaskTime getTaskTime(QString taskTimeStr);

extern QMap<TaskType, TaskTime> taskTimes;
extern quint8 commonTasks, longTasks, shortTasks;

#endif
