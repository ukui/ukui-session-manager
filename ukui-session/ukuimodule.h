#ifndef UKUIMODULE_H
#define UKUIMODULE_H

#include <QProcess>
#include "xdgautostart.h"
#include "xdgdesktopfile.h"
#include "xdgdirs.h"

class UkuiModule : public QProcess
{
    Q_OBJECT
public:
    UkuiModule(const XdgDesktopFile& file, QObject *parent = nullptr);
    void start();
    void terminate();
    bool isTerminating();

    int restartNum;

    const XdgDesktopFile file;
    const QString fileName;

signals:
    void moduleStateChanged(QString name, bool state);

private slots:
    void updateState(QProcess::ProcessState newState);

private:
    bool mIsTerminating;
};

#endif // UKUIMODULE_H
