#include "meetings.h"
#include "Server.h"
#include <QDebug>

MeetingUI::MeetingUI(InGameUI* parent, Player* reportedPlayer): QWidget(parent), votedPlayer(nullptr), voted(false) {
    setAutoFillBackground(true);
    layout = new QGridLayout(this);
    titleLabel = new QLabel("<b>Who is the Impostor?</b>");
    layout->addWidget(titleLabel, 0, 0, 1, -1);
    if(reportedPlayer)
        reportedPlayerLabel = new QLabel(QString("%1's body was found").arg(reportedPlayer->nickname)); /// should display who activate the meeting
    else
        reportedPlayerLabel = new QLabel("No dead body reported");
    layout->addWidget(reportedPlayerLabel, 1, 0, 1, -1);
    int iPlayer = 0;
    QVector<Player*> players;
    players.push_back(&parent->currPlayer);
    for(Player &player : parent->otherPlayers.values())
        players.push_back(&player);
    for(Player* player : players) {
        QString playerLabel;
        if(player->isGhost)
            playerLabel = QString("%1 (dead)").arg(player->nickname);
        else
            playerLabel = QString(player->nickname);
        QPushButton* button = new QPushButton(playerLabel);
        if(player->isGhost)
            button->setDisabled(true);
        else
            connect(button, &QPushButton::released, this, [=]() {voteFor(player);});
        int row = 2+iPlayer/MEETING_PLAYERS_PER_ROW, col = iPlayer%MEETING_PLAYERS_PER_ROW;
        layout->addWidget(button, row, col);
        playerButtons.push_back(button);
        iPlayer++;
    }
    int skipRow = 2+iPlayer/MEETING_PLAYERS_PER_ROW;
    if(iPlayer % MEETING_PLAYERS_PER_ROW != 0)
        skipRow++;
    skipButton = new QPushButton("Skip vote");
    connect(skipButton, &QPushButton::clicked, this, [=]() {voteFor(nullptr);});
    layout->addWidget(skipButton, skipRow, 0, 1, -1);
}

void MeetingUI::voteFor(Player* player) {
    if(voted)
        return; // should never happen
    if(!player)
        qDebug() << "Skipped vote";
    else
        qDebug() << "Voted for" << player->nickname;
    votedPlayer = player;
    voted = true;
    for(QPushButton* button : playerButtons)
        button->setDisabled(true);
    skipButton->setDisabled(true);
    titleLabel->setText("<b>Waiting for other players...</b>");
    sendToAll("Voted"); // TODO: delay votes till the end?
    // TODO: perform voting logic
    /*InGameUI* parentUI = static_cast<InGameUI*>(parent());
    parentUI->closeMeetingUI();*/
}
