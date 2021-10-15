#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include <QMediaPlayer>
#include "QLabelKeys.h"

extern QMediaPlayer* player;
extern QLabelKeys* qLabelKeys;
enum InGameGUI{IN_GAME_GUI_NONE, IN_GAME_GUI_FIX_WIRING};
extern InGameGUI currentInGameGUI;

#endif
