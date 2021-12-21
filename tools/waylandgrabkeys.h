#ifndef WAYLANDGRABKEYS_H
#define WAYLANDGRABKEYS_H

#include <QMainWindow>
#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QFont>
#include <QAbstractNativeEventFilter>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/plasmashell.h>
class WaylandGrabKeys : public QObject
{
    Q_OBJECT

public:
    WaylandGrabKeys(QWidget *parent = nullptr);
    ~WaylandGrabKeys();

protected:
private:
    KWayland::Client::Registry *m_ksldRegistry = nullptr;
protected:

public:

signals:
};
#endif // WAYLANDGRABKEYS_H
