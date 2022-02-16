#include "Task.h"
#include <QMap>
#include <QVector>
#include <QRandomGenerator>
#include "../map/Player.h"
using namespace std;

QMap<TaskType, TaskTime> taskTimes{{TASK_ASTEROIDS, TASK_SHORT}, {TASK_FIX_WIRING, TASK_COMMON}, {TASK_ENTER_ID_CODE, TASK_COMMON},{TASK_ALIGN_ENGINE,TASK_LONG}};
QMap<Map, QMap<TaskType, QList<QPoint>>> taskLocations{
	{
		MAP_THE_SKELD,
		{
			{TASK_ASTEROIDS, {QPoint(6653, 900)}},
   			{TASK_FIX_WIRING, {QPoint(4060, 360), QPoint(5433,2444), QPoint(7455,2055)}},
   			{TASK_ENTER_ID_CODE, {QPoint(2645, 1820)}},
   			{TASK_ALIGN_ENGINE, {QPoint(1550,3600)}}
		}
	},
	{
		MAP_POLUS,
		{
			{TASK_ASTEROIDS, {QPoint(1413, 1650)}},
   			{TASK_FIX_WIRING, {QPoint(2610, 3400), QPoint(9231,2860), QPoint(6012,5890)}},
   			{TASK_ENTER_ID_CODE, {QPoint(3090, 4480)}},
   			{TASK_ALIGN_ENGINE, {QPoint(573,5410)}}
		}
	}
};

quint8 commonTasks = /*1*/2, longTasks = /*1*/1, shortTasks = /*2*/1;

Task::Task():
taskType(TASK_FIX_WIRING),
location(QPoint(0,0)),
finished(false) {}

Task::Task(TaskType taskType, QPoint location):
taskType(taskType),
//taskTime(taskTimes[taskType]), // order seems to depend here for compilator weird
location(location),
finished(false) {}

bool Task::operator ==(Task const taskBis) const
{
    return taskType == taskBis.taskType && location == taskBis.location;
}

tuple<quint8, quint8, quint8> getTasksTypes(QVector<Task> tasks)
{
    quint8 commonTasksGot = 0, longTasksGot = 0, shortTasksGot = 0;
    for(Task task : tasks)
    {
        TaskType taskType = task.taskType;
        TaskTime taskTime = taskTimes[taskType];
        switch(taskTime)
        {
            case TASK_COMMON:
                commonTasksGot++;
                break;
            case TASK_LONG:
                longTasksGot++;
                break;
            default: // TASK_SHORT
                shortTasksGot++;
        }
    }
    return make_tuple(commonTasksGot, longTasksGot, shortTasksGot);
}

bool doTasksFitRequirements(QVector<Task> tasks)
{
    tuple<quint8, quint8, quint8> tasksTypes = getTasksTypes(tasks);
    quint8 commonTasksGot = get<0>(tasksTypes),
           longTasksGot = get<1>(tasksTypes),
           shortTasksGot = get<2>(tasksTypes);
    return commonTasksGot == commonTasks && longTasksGot == longTasks && shortTasksGot == shortTasks;
}

// assume one is needed
TaskTime getTaskTimeNeeded(QVector<Task> tasks)
{
    tuple<quint8, quint8, quint8> tasksTypes = getTasksTypes(tasks);
    quint8 commonTasksGot = get<0>(tasksTypes),
           longTasksGot = get<1>(tasksTypes);
    if(commonTasksGot != commonTasks)
        return TASK_COMMON;
    else if(longTasksGot != longTasks)
        return TASK_LONG;
    return TASK_SHORT;
}

Task pickRandomTask(QRandomGenerator* qRandomGenerator, TaskTime taskTime, Map map)
{
    QVector<Task> tasks;
    QVector<TaskType> taskTypes;
    QList<TaskType> taskTimesKeys = taskTimes.keys();
    for(TaskType taskTypeKey : taskTimesKeys)
        if(taskTimes[taskTypeKey] == taskTime)
            taskTypes.push_back(taskTypeKey);
    for(TaskType taskType : taskTypes)
       	for(QPoint taskTypeLocation : taskLocations[map][taskType])
           	tasks.push_back(Task(taskType, taskTypeLocation));

    quint8 tasksSize = tasks.size(),
           randomIndex = qRandomGenerator->bounded(tasksSize);
    Task task = tasks[randomIndex];
    return task;
}

QVector<Task*> getTasksAsPointers(QVector<Task> tasks)
{
    QVector<Task*> tasksPointers;
    transform(tasks.begin(), tasks.end(), back_inserter(tasksPointers), [](const Task task){ return new Task(task.taskType, task.location); });
    return tasksPointers;
}

QVector<Task> getRandomTasks(QString privateSaltedWithCommonRandom, Map map) // can't only depend on private random, it also have to depends on common random
{
    bool ok;
    privateSaltedWithCommonRandom = privateSaltedWithCommonRandom.toUpper();
    privateSaltedWithCommonRandom.chop(120); // otherwise toUInt not working
    quint32 privateSaltedWithCommonRandomUint32 = privateSaltedWithCommonRandom.toUInt(/*nullptr*/&ok, 16); /// warning cryptographic here 32 bits only...
    //qInfo() << "ok:" << ok;
    //qInfo() << "privateSaltedWithCommonRandomUint32 " << privateSaltedWithCommonRandomUint32 << privateSaltedWithCommonRandom;
    QRandomGenerator qRandomGenerator = QRandomGenerator(privateSaltedWithCommonRandomUint32);
    QVector<Task> tasks;
    while(!doTasksFitRequirements(tasks))
    {
        TaskTime taskTimeNeeded = getTaskTimeNeeded(tasks);
        const Task randomTask = pickRandomTask(&qRandomGenerator, taskTimeNeeded, map);
        if(!tasks.contains(randomTask))
            tasks.push_back(randomTask);
    }
    return tasks;
}

QString taskTimeToString(TaskTime taskTime)
{
    switch(taskTime)
    {
        case TASK_COMMON:
            return "common";
        case TASK_LONG:
            return "long";
        default: // TASK_SHORT
            return "short";
    }
}

TaskTime getTaskTime(QString taskTimeStr)
{
    if(taskTimeStr == "common")
        return TASK_COMMON;
    else if(taskTimeStr == "long")
        return TASK_LONG;
    return TASK_SHORT;
}
