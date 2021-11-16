#include "musicplayer.h"
#include "ukuisessiondebug.h"

void musicplayer::run()
{
    m_player->play();
}

musicplayer::musicplayer()
{
    m_player = new QMediaPlayer;
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
    qCDebug(UKUI_SESSION) << "the address of m_player is " << m_player;
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
    qCDebug(UKUI_SESSION) << "Player state: " << state;
    if (state == QMediaPlayer::StoppedState) {
        qCDebug(UKUI_SESSION) << "the address of m_player is " << m_player;
        emit playFinished();
    }
}

Worker::Worker(int volumn, QString source)
{
    m_player = new QMediaPlayer;
    m_player->setVolume(volumn);
    m_player->setMedia(QUrl::fromLocalFile(source));
    connect(m_player, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(stateChanged(QMediaPlayer::State)));
    qCDebug(UKUI_SESSION) << "the address of m_player is " << m_player;

}

Worker::~Worker()
{
    if (m_player) {
        delete m_player;
    }

}

void Worker::doWork()
{
    m_player->play();
}

void Worker::stateChanged(QMediaPlayer::State state)
{
    qCDebug(UKUI_SESSION) << "Player state: " << state;
    if (state == QMediaPlayer::StoppedState) {
        qCDebug(UKUI_SESSION) << "the address of m_player is " << m_player;
        emit playFinished();
    }
}
