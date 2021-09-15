#include "musicplayer.h"

void musicplayer::run()
{
    m_player->play();
}

musicplayer::musicplayer()
{
    m_player = new QMediaPlayer;
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
    qDebug() << "the address of m_player is " << m_player;
}

musicplayer::~musicplayer()
{
    delete m_player;
}

void musicplayer::setVolumn(int volumn)
{
    m_player->setVolume(volumn);
}

void musicplayer::setSource(QString source)
{
    m_player->setMedia(QUrl::fromLocalFile(source));
}

void musicplayer::stateChanged(QMediaPlayer::State state)
{
    qDebug() << "Player state: " << state;
    if (state == QMediaPlayer::StoppedState) {
        qDebug() << "the address of m_player is " << m_player;
//        delete m_player;
//        qDebug() << "delete player";
        emit playFinished();
    }
}
