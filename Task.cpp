#include "Task.h"

Task::Task():
taskType(TASK_FIX_WIRING),
location(QPoint(0,0)),
finished(false) {}

Task::Task(TaskType taskType, QPoint location):
taskType(taskType),
location(location),
finished(false) {}