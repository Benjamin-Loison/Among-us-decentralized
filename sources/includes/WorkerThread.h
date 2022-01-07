#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include "main.h"

// not working that much...
class WorkerThread : public QThread
{
    Q_OBJECT
    void run() override {
        inGameUI->checkEverybodyReady();
    }
};

#endif
