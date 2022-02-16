#include "meetings.h"
#include "../network/Server.h"
#include "../main.h"

MeetingUI::MeetingUI(InGameUI* parent, Player* reportedPlayer, Player* reportingPlayer): QWidget(parent), skipVotes(0), votedPlayer(nullptr), voted(false) {
    waitingVotes = inGameUI->getAlivePlayersNumber();
    setAutoFillBackground(true);
    layout = new QGridLayout(this);

    titleLabel = new QLabel("<b>" + tr("Who is the Impostor ?") + "</b>");
    layout->addWidget(titleLabel, 0, 0, 1, -1);
    if(reportedPlayer)
        reportedPlayerLabel = new QLabel(QString(tr("%1's body was found by %2")).arg(reportedPlayer->nickname).arg(reportingPlayer->nickname)); /// should display who activate the meeting, can't .arg on tr result ?
    else
        reportedPlayerLabel = new QLabel(QString(tr("Emergency meeting requested by %1")).arg(reportingPlayer->nickname)); // No dead body reported
    layout->addWidget(reportedPlayerLabel, 1, 0, 1, -1);

    int iPlayer = 0;
    QVector<Player*> players;
    players.push_back(&parent->currPlayer);
    for(Player&/*ampersand is important here*/ player : inGameUI->otherPlayers)
        players.push_back(&player);
    for(Player* player : players) {
        QString playerLabel = player->isGhost ? QString(tr("%1 (dead)")).arg(player->nickname) : QString(player->nickname);
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
    skipButton = new QPushButton(tr("Skip vote"));
    if(inGameUI->currPlayer.isGhost)
        skipButton->setDisabled(true);
    connect(skipButton, &QPushButton::clicked, this, [=]() {voteFor(nullptr);});
    layout->addWidget(skipButton, skipRow, 0, 1, -1);
}

void MeetingUI::voteFor(Player* player) {
    if(voted)
        return; // should never happen
    votedPlayer = player;
    voted = true;
    for(QPushButton* button : playerButtons)
        button->setDisabled(true);
    skipButton->setDisabled(true);
    titleLabel->setText("<b>" + tr("Waiting for other players...") + "</b>");
    QString toSend = player ? "Voted " + player->nickname : "Skip";
    sendToAll(toSend); // TODO: delay votes till the end?
    inGameUI->executeVote(toSend, &inGameUI->currPlayer);
    /*InGameUI* parentUI = static_cast<InGameUI*>(parent());
    parentUI->closeMeetingUI();*/
}

MeetingResultsUI::MeetingResultsUI(InGameUI *parent, QMap<QString, quint8> votes, QMap<QString, Player*> votesByPlayer, quint8 skipVotes): QWidget(parent), votes(votes), votesByPlayer(votesByPlayer), skipVotes(skipVotes), proceedsLeft(parent->getAlivePlayersNumber()) {
    setAutoFillBackground(true);
    layout = new QGridLayout(this);

    titleLabel = new QLabel("<b>" + tr("Voting results") + "</b>");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel, 0, 0, 1, -1);

    QMap<QString, QStringList> whoVotedFor;
    QStringList whoVotedSkip;
    if(!parent->currPlayer.isGhost)
        whoVotedFor[parent->currPlayer.nickname] = QStringList();
    for(Player* player : parent->getAllPlayers())
        if(!player->isGhost)
            whoVotedFor[player->nickname] = QStringList();
    for(auto it = votesByPlayer.begin(); it != votesByPlayer.end(); ++it)
        if(it.value() != nullptr)
            whoVotedFor[it.value()->nickname].append(it.key());
        else
            whoVotedSkip.append(it.key());

    int iPlayer = 0;
    for(auto it = whoVotedFor.begin(); it != whoVotedFor.end(); ++it, ++iPlayer) {
        QLabel* playerLabel = new QLabel("<b>" + it.key() + "</b>");
        layout->addWidget(playerLabel, iPlayer+1, 0);
        playerLabels.append(playerLabel);

        QLabel* votersLabel = new QLabel(it.value().join(", "));
        layout->addWidget(votersLabel, iPlayer+1, 1);
        voterLabels.append(votersLabel);
    }
    skipLabel = new QLabel("<i>" + tr("Skip vote") + "</i>");
    layout->addWidget(skipLabel, iPlayer+1, 0);
    skipVotersLabel = new QLabel(whoVotedSkip.join(", "));
    layout->addWidget(skipVotersLabel, iPlayer+1, 1);

    QString verdictText = "";
    quint8 nbImpostors = parent->getAliveImpostorsNumber();
    if(2*skipVotes > inGameUI->getAlivePlayersNumber()) {
        verdictText = tr("Majority voted to skip vote. No player was killed.") + "<br>";
        whoToKill = nullptr;
    }
    else {
        quint8 maxVotes = 0;
        QString nicknameMostVoted = "";
        bool exAequo = false;
        for(auto it = votes.begin(); it != votes.end(); ++it)
        {
            quint8 currentVotes = it.value();
            if(currentVotes > maxVotes)
            {
                maxVotes = currentVotes;
                nicknameMostVoted = it.key();
                exAequo = false;
            }
            else if(currentVotes == maxVotes && currentVotes > 0)
                exAequo = true;
        }
        if(nicknameMostVoted != "" && !exAequo) {
            whoToKill = parent->getPlayer(nicknameMostVoted);
            verdictText = tr("%1 got the most votes and was killed.").arg(nicknameMostVoted) + "<br>";
			verdictText += (whoToKill->isImpostor ? tr("He was an Impostor.") : tr("He was not an Impostor.")) + "<br>"; // can't make the ternary within the tr otherwise translation detection doesn't work well, does it ?
            if(whoToKill->isImpostor)
                nbImpostors--;
        }
        else {
            whoToKill = nullptr;
            verdictText = tr("A tie occurred. No player was killed.") + "<br>";
        }
    }
    verdictText += nbImpostors <= 1 ? tr("%1 Impostor left.").arg(nbImpostors) : tr("%1 Impostors left.").arg(nbImpostors);
    verdictLabel = new QLabel(verdictText);
    verdictLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(verdictLabel, iPlayer+2, 0, 1, -1);

    proceedButton = new QPushButton(tr("Proceed"));
    if(parent->currPlayer.isGhost) {
        proceedButton->setEnabled(false);
        updateProceedButton();
    }
    connect(proceedButton, SIGNAL(clicked()), this, SLOT(onClickProceed()));
    layout->addWidget(proceedButton, iPlayer+3, 0, 1, -1);
}

void MeetingResultsUI::updateProceedButton() {
    if(!proceedButton->isEnabled())
        proceedButton->setText(proceedsLeft == 1 ? tr("Waiting for one other player...") : tr("Waiting for %1 other players...").arg(proceedsLeft));
}

void MeetingResultsUI::onProceed() {
    proceedsLeft--;
    if(proceedsLeft == 0)
        inGameUI->onAllProceeded(whoToKill);
    else
        updateProceedButton();
}

void MeetingResultsUI::onClickProceed() {
    sendToAll("Proceed_vote");
    proceedButton->setEnabled(false);
    onProceed();
}
