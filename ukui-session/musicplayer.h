#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QThread>
#include <QMediaPlayer>

class musicplayer :public QThread
{
    Q_OBJECT
    void run() override;

public:
    musicplayer();
    ~musicplayer();

    void setVolumn(int volumn);
    void setSource(QString source);
public slots:
    void stateChanged(QMediaPlayer::State state);

signals:
    void playFinished();

private:
    QMediaPlayer *m_player;
};

#endif // MUSICPLAYER_H
