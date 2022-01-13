#ifndef MEETINGS_H
#define MEETINGS_H

#include <QWidget>
#include <QPushButton>
#include <QVector>
#include "../map/Player.h"
#include <QMap>
#include <QGridLayout>
#include <QLabel>

const int MEETING_PLAYERS_PER_ROW = 3;

class InGameUI;

class MeetingUI: public QWidget {
    Q_OBJECT

public:
    QMap<QString, quint8> votes;
    QMap<QString, Player*> votesByPlayer;
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

class MeetingResultsUI: public QWidget {
    Q_OBJECT

public:
    QMap<QString, quint8> votes;
    QMap<QString, Player*> votesByPlayer;
    quint8 skipVotes;

private:
    QGridLayout* layout;
    QLabel* titleLabel;
    QVector<QLabel*> playerLabels;
    QVector<QLabel*> voterLabels;
    QLabel* skipLabel;
    QLabel* skipVotersLabel;
    QLabel* verdictLabel;
    QPushButton* proceedButton;
    quint8 proceedsLeft;
    Player* whoToKill;
    void updateProceedButton();

public:
    MeetingResultsUI(InGameUI *parent, QMap<QString, quint8> votes, QMap<QString, Player*> votesByPlayer, quint8 skipVotes);
    void onProceed();

public slots:
    void onClickProceed();
};

#endif
