#include "sessionapplication.h"
#include "modulemanager.h"
#include "sessiondbusadaptor.h"
//#include <QGSettings/QGSettings>

SessionApplication::SessionApplication(int& argc, char** argv) :
    QApplication(argc, argv)
{
    modman = new ModuleManager;

    new SessionDBusAdaptor(modman);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(QStringLiteral("org.ukui.Session"));
    dbus.registerObject(("/org/ukui/Session"), modman);

    // Wait until the event loop starts
    QTimer::singleShot(0, this, SLOT(startup()));
}

SessionApplication::~SessionApplication()
{
    delete modman;
}

bool SessionApplication::startup()
{
    modman->startup();
//    getidledelay();
//    if(settings != nullptr){
//        qDebug()<<"111111111111111111 idle-delay = "<<idledelay;
//        connect(settings,SIGNAL(changed(QString)),this,SLOT(updatekey(QString)));
//        idlemonitor = new QTimer(this);
//        connect(idlemonitor,SIGNAL(timeout()), this, SLOT(onTimeout()));
//        idlemonitor->start(idledelay*60000);
//    }
    return true;
}

//bool SessionApplication::notify(QObject *obj, QEvent *e)
//{
//    qDebug()<<"nothing has done"<<"obj name :"<<obj->metaObject()->className()<<"event name: "<<e->type();
//    if(e->type() == QEvent::KeyPress){
//        qDebug()<<"KeyPress";
//        updateTimemonitor();
//    }else if(e->type() == QEvent::MouseButtonPress){
//        qDebug()<<"MouseButtonPress";
//        updateTimemonitor();
//    }else if(e->type() == QEvent::MouseMove){
//        qDebug()<<"MouseMove";
//        updateTimemonitor();
//    }

//    return QApplication::notify(obj,e);//其他事件仍然交给系统处理
//}

//void SessionApplication::getidledelay(){
//    if(QGSettings::isSchemaInstalled("org.ukui.session")) {
//        settings = new QGSettings("org.ukui.session");
//        idledelay = settings->get("idle-delay").toInt();
//        qDebug()<<"Gsettings's idle-delay value is :"<<idledelay;
//    }else{
//        qDebug()<<"Can not find the org.ukui.session Schema";
//    }
//}

//void SessionApplication::updateTimemonitor(){
//    idlemonitor->stop();
//    idlemonitor->start(idledelay*60000);
//}

//void SessionApplication::updatekey(const QString &key){
//    if (key == "idle-delay") {
//        idledelay = settings->get("idle-delay").toInt();
//        qDebug()<<"3333333333333333now the idle-delay = "<<idledelay;
//    }
//}

//void SessionApplication::onTimeout(){
//    idlemonitor->stop();
//    qDebug()<<"22222222222222222"<<"close screen";
//}
