#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include <QMediaPlayer>
#include "InGameUI.h"

extern QMediaPlayer* player;
extern InGameUI* inGameUI;
enum InGameGUI{IN_GAME_GUI_NONE, IN_GAME_GUI_FIX_WIRING};
enum Task{TASK_FIX_WIRING,TASK_ASTEROIDS,TASK_SWIPE};
extern InGameGUI currentInGameGUI;

#endif
