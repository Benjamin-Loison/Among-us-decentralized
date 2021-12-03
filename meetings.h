#ifndef MEETINGS_H
#define MEETINGS_H

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include "Player.h"
#include <QMap>
#include <QGridLayout>
#include <QLabel>

const int MEETING_PLAYERS_PER_ROW = 3;

class InGameUI;

class MeetingUI: public QWidget {
    Q_OBJECT

public:
    QMap<QString, quint8> votes;
    quint8 skipVotes, waitingVotes;

private:
    void voteFor(Player* player);
    QVector<QPushButton*> playerButtons;
    QPushButton* skipButton;
    QGridLayout* layout;
    QLabel* titleLabel;
    QLabel* reportedPlayerLabel;
    Player* votedPlayer;
    bool voted;

public:
    MeetingUI(InGameUI* parent, Player* reportedPlayer, Player* reportingPlayer);
};

#endif
