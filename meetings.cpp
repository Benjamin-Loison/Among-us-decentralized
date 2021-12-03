#include "meetings.h"
#include "Server.h"
#include "main.h"

MeetingUI::MeetingUI(InGameUI* parent, Player* reportedPlayer, Player* reportingPlayer): QWidget(parent), votedPlayer(nullptr), voted(false) {
    skipVotes = inGameUI->getAlivePlayersNumber();
    waitingVotes = skipVotes;
    setAutoFillBackground(true);
    layout = new QGridLayout(this);
    titleLabel = new QLabel("<b>Who is the Impostor?</b>");
    layout->addWidget(titleLabel, 0, 0, 1, -1);
    if(reportedPlayer)
        reportedPlayerLabel = new QLabel(QString("%1's body was found by %2").arg(reportedPlayer->nickname).arg(reportingPlayer->nickname)); /// should display who activate the meeting
    else
        reportedPlayerLabel = new QLabel(QString("Emergency meeting asked by %1").arg(reportingPlayer->nickname)); // No dead body reported
    layout->addWidget(reportedPlayerLabel, 1, 0, 1, -1);
    int iPlayer = 0;
    QVector<Player*> players;
    players.push_back(&parent->currPlayer);
    for(Player&/*ampersand is important here*/ player : inGameUI->otherPlayers)
        players.push_back(&player);
    for(Player* player : players) {
        QString playerLabel = player->isGhost ? QString("%1 (dead)").arg(player->nickname) : QString(player->nickname);
        QPushButton* button = new QPushButton(playerLabel);
        if(player->isGhost || inGameUI->currPlayer.isGhost)
            button->setDisabled(true);
        else
            connect(button, &QPushButton::released, this, [=/*=?*/]() {voteFor(player);});
        int row = 2+iPlayer/MEETING_PLAYERS_PER_ROW, col = iPlayer%MEETING_PLAYERS_PER_ROW;
        layout->addWidget(button, row, col);
        playerButtons.push_back(button);
        iPlayer++;
    }
    int skipRow = 2+iPlayer/MEETING_PLAYERS_PER_ROW;
    if(iPlayer % MEETING_PLAYERS_PER_ROW != 0)
        skipRow++;
    skipButton = new QPushButton("Skip vote");
    if(inGameUI->currPlayer.isGhost)
        skipButton->setDisabled(true);
    connect(skipButton, &QPushButton::clicked, this, [=]() {voteFor(nullptr);});
    layout->addWidget(skipButton, skipRow, 0, 1, -1);
}

void MeetingUI::voteFor(Player* player) {
    if(voted)
        return; // should never happen
    /*if(!player)
        qDebug() << "Skipped vote";
    else
        qDebug() << "Voted for" << player->nickname;*/
    votedPlayer = player;
    voted = true;
    for(QPushButton* button : playerButtons)
        button->setDisabled(true);
    skipButton->setDisabled(true);
    titleLabel->setText("<b>Waiting for other players...</b>");
    QString toSend = player ? "Voted " + player->nickname : "Skip";
    sendToAll(toSend); // TODO: delay votes till the end?
    inGameUI->executeVote(toSend);
    /*InGameUI* parentUI = static_cast<InGameUI*>(parent());
    parentUI->closeMeetingUI();*/
}
