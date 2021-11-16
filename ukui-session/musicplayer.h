#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QThread>
#include <QMediaPlayer>

/*继承QThread的写法*/
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

/*worker-object写法*/
class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(int volumn, QString source);
    ~Worker();
public slots:
    void doWork();
    void stateChanged(QMediaPlayer::State state);

signals:
    void playFinished();

private:
    QMediaPlayer *m_player;
};

class Controller : public QObject
{
    Q_OBJECT
    QThread workerThread;

public:
    Controller(int volumn, QString source) {
      Worker *worker = new Worker(volumn, source);
      worker->moveToThread(&workerThread);
      connect(worker, &Worker::playFinished, worker, &QObject::deleteLater);
      connect(this, &Controller::operate, worker, &Worker::doWork);
      workerThread.start();
    }

    ~Controller() {
      workerThread.quit();
      workerThread.wait();
    }

signals:
    void operate(const QString &);
  };

#endif // MUSICPLAYER_H
