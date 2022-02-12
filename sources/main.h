#ifndef MAIN_H
#define MAIN_H

#include <QString>
#include "ui/InGameUI.h"
#include "network/Client.h"
#include "network/Server.h"

extern InGameUI* inGameUI;
enum InGameGUI{IN_GAME_GUI_NONE, IN_GAME_GUI_FIX_WIRING, IN_GAME_GUI_ASTEROIDS,IN_GAME_GUI_ENTER_ID_CODE,IN_GAME_GUI_ALIGN_ENGINE, IN_GAME_GUI_MAP, IN_GAME_GUI_CAMERA, IN_GAME_GUI_VITALS, IN_GAME_GUI_MEETING, IN_GAME_GUI_WIN_CREWMATES, IN_GAME_GUI_WIN_IMPOSTORS, IN_GAME_GUI_VENT};
extern InGameGUI currentInGameGUI;
extern QList<Client*> clients;
extern Server* server;
extern QString myAddress;
extern bool isFirstToRun;
extern quint16 remotePort;
QString toString(InGameGUI inGameGUI);

#endif
