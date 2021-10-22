#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include <QMediaPlayer>
#include "InGameUI.h"

extern QMediaPlayer* player;
extern InGameUI* qLabelKeys;
enum InGameGUI{IN_GAME_GUI_NONE, IN_GAME_GUI_FIX_WIRING};
extern InGameGUI currentInGameGUI;

#endif
