#include "Task.h"
#include <QMap>
#include <QVector>
#include <QRandomGenerator>
using namespace std;

QMap<TaskType, TaskTime> taskTimes{{TASK_ASTEROIDS, TASK_SHORT}, {TASK_FIX_WIRING, TASK_COMMON}};
QMap<TaskType, QList<QPoint>> taskLocations{{TASK_ASTEROIDS, {QPoint(6653, 900)}}, {TASK_FIX_WIRING, {QPoint(4060, 360), QPoint(5433,2444), QPoint(7455,2055)}}};

quint8 commonTasks = 1, longTasks = /*1*/0, shortTasks = /*2*/1;

Task::Task():
taskType(TASK_FIX_WIRING),
location(QPoint(0,0)),
finished(false) {}

Task::Task(TaskType taskType, QPoint location):
taskType(taskType),
//taskTime(taskTimes[taskType]), // order seems to depend here for compilor weird
location(location),
finished(false) {}

bool Task::operator ==(Task const taskBis) const
{
    return this->taskType == taskBis.taskType && this->location == taskBis.location;
}

tuple<quint8, quint8, quint8> getTasksTypes(QVector<Task> tasks)
{
    quint8 commonTasksGot = 0, longTasksGot = 0, shortTasksGot = 0, tasksSize = tasks.size();
    for(quint8 tasksIndex = 0; tasksIndex < tasksSize; tasksIndex++)
    {
        Task* task = &tasks[tasksIndex];
        TaskType taskType = task->taskType;
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
    quint8 commonTasksGot = get<0>(tasksTypes), longTasksGot = get<1>(tasksTypes), shortTasksGot = get<2>(tasksTypes);
    return commonTasksGot == commonTasks && longTasksGot == longTasks && shortTasksGot == shortTasks;
}

// assume one is needed
TaskTime getTaskTimeNeeded(QVector<Task> tasks)
{
    tuple<quint8, quint8, quint8> tasksTypes = getTasksTypes(tasks);
    quint8 commonTasksGot = get<0>(tasksTypes), longTasksGot = get<1>(tasksTypes)/*, shortTasksGot = get<2>(tasksTypes)*/;
    if(commonTasksGot != commonTasks)
        return TASK_COMMON;
    else if(longTasksGot != longTasks)
        return TASK_LONG;
    return TASK_SHORT;
}

Task pickRandomTask(QRandomGenerator* qRandomGenerator, TaskTime taskTime)
{
    QVector<Task> tasks;
    QVector<TaskType> taskTypes;
    QList<TaskType> taskTypesKeys = taskTimes.keys();
    quint8 taskTypesKeysSize = taskTypesKeys.size();
    for(quint8 taskTypesKeysIndex = 0; taskTypesKeysIndex < taskTypesKeysSize; taskTypesKeysIndex++)
    {
        TaskType taskTypeKey = taskTypesKeys[taskTypesKeysIndex];
        TaskTime taskTimeGot = taskTimes[taskTypeKey];
        if(taskTimeGot == taskTime)
        {
            taskTypes.push_back(taskTypeKey);
        }
    }
    quint8 taskTypesSize = taskTypes.size();
    for(quint8 taskTypesIndex = 0; taskTypesIndex < taskTypesSize; taskTypesIndex++)
    {
        TaskType taskType = taskTypes[taskTypesIndex];
        QList<QPoint> taskTypeLocations = taskLocations[taskType];
        quint8 taskTypeLocationsSize = taskTypeLocations.size();
        for(quint8 taskTypeLocationsIndex = 0; taskTypeLocationsIndex < taskTypeLocationsSize; taskTypeLocationsIndex++)
        {
            QPoint taskTypeLocation = taskTypeLocations[taskTypeLocationsIndex];
            tasks.push_back(Task(taskType, taskTypeLocation));
        }
    }

    quint8 tasksSize = tasks.size(), randomIndex = qRandomGenerator->bounded(tasksSize);
    Task task = tasks[randomIndex];
    return task;
}

QVector<Task*> getTasksAsPointers(QVector<Task> tasks)
{
    QVector<Task*> tasksPointers;
    quint8 tasksSize = tasks.size();
    for(quint8 tasksIndex = 0; tasksIndex < tasksSize; tasksIndex++)
    {
        Task task = tasks[tasksIndex];
        tasksPointers.push_back(new Task(task.taskType, task.location));
    }
    return tasksPointers;
}

QVector<Task> getRandomTasks(QString privateSaltedWithCommonRandom) // can't only depend on private random, it also have to depends on common random
{
    quint32 privateSaltedWithCommonRandomUint32 = privateSaltedWithCommonRandom.toUInt(nullptr, 16); /// warning cryptographic here 32 bits only...
    QRandomGenerator qRandomGenerator = QRandomGenerator(privateSaltedWithCommonRandomUint32);
    QVector<Task> tasks;
    while(!doTasksFitRequirements(tasks))
    {
        TaskTime taskTimeNeeded = getTaskTimeNeeded(tasks);
        const Task randomTask = pickRandomTask(&qRandomGenerator, taskTimeNeeded);
        if(!tasks.contains(randomTask))
        {
            tasks.push_back(randomTask);
        }
    }
    return tasks;
}
