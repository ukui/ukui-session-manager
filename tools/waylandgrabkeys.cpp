#include "waylandgrabkeys.h"
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/surface.h>
#include <KWayland/Client/plasmashell.h>
#include <KGlobalAccel>
#include <QDebug>
#include <QAction>
using namespace  KWayland::Client;
WaylandGrabKeys::WaylandGrabKeys(QWidget *parent)
{
    // Global keys
    if (true) {
        qDebug() << "Configuring Lock Action";
        QAction *a = new QAction(this);
        a->setObjectName(QStringLiteral("Lock Session"));
        a->setProperty("componentName", QStringLiteral("ksmserver"));
        a->setText(("Lock Session"));
        QList<QKeySequence> keyList;
        keyList = {Qt::META | Qt::Key_L, Qt::ALT | Qt::CTRL | Qt::Key_L, Qt::Key_ScreenSaver};
        KGlobalAccel::self()->setGlobalShortcut(a, keyList);
        connect(a, &QAction::triggered, this, [this]() {
            qDebug() << "triggered...";
        });
    }

//    m_ksldRegistry = new Registry();
//    EventQueue *queue = new EventQueue(m_ksldRegistry);

//    connect(m_ksldRegistry, &Registry::interfaceAnnounced, this, [this, queue](QByteArray interface, quint32 name, quint32 version) {
//        Q_UNUSED(version)
//        if (interface != QByteArrayLiteral("org_kde_ksld")) {
//            return;
//        }

//        qDebug() << "interfaceAnnounced....." << interface << name;
//    });


}

WaylandGrabKeys::~WaylandGrabKeys()
{
}
