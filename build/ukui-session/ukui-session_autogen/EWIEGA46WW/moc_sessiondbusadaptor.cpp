/****************************************************************************
** Meta object code from reading C++ file 'sessiondbusadaptor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sessiondbusadaptor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sessiondbusadaptor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SessionDBusAdaptor_t {
    QByteArrayData data[46];
    char stringdata0[560];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SessionDBusAdaptor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SessionDBusAdaptor_t qt_meta_stringdata_SessionDBusAdaptor = {
    {
QT_MOC_LITERAL(0, 0, 18), // "SessionDBusAdaptor"
QT_MOC_LITERAL(1, 19, 15), // "D-Bus Interface"
QT_MOC_LITERAL(2, 35, 24), // "org.gnome.SessionManager"
QT_MOC_LITERAL(3, 60, 18), // "moduleStateChanged"
QT_MOC_LITERAL(4, 79, 0), // ""
QT_MOC_LITERAL(5, 80, 10), // "moduleName"
QT_MOC_LITERAL(6, 91, 5), // "state"
QT_MOC_LITERAL(7, 97, 12), // "inhibitadded"
QT_MOC_LITERAL(8, 110, 5), // "flags"
QT_MOC_LITERAL(9, 116, 13), // "inhibitremove"
QT_MOC_LITERAL(10, 130, 11), // "StartLogout"
QT_MOC_LITERAL(11, 142, 20), // "PrepareForSwitchuser"
QT_MOC_LITERAL(12, 163, 16), // "PrepareForPhase2"
QT_MOC_LITERAL(13, 180, 15), // "startupfinished"
QT_MOC_LITERAL(14, 196, 7), // "appName"
QT_MOC_LITERAL(15, 204, 6), // "string"
QT_MOC_LITERAL(16, 211, 9), // "canSwitch"
QT_MOC_LITERAL(17, 221, 12), // "canHibernate"
QT_MOC_LITERAL(18, 234, 10), // "canSuspend"
QT_MOC_LITERAL(19, 245, 9), // "canLogout"
QT_MOC_LITERAL(20, 255, 9), // "canReboot"
QT_MOC_LITERAL(21, 265, 11), // "canPowerOff"
QT_MOC_LITERAL(22, 277, 10), // "switchUser"
QT_MOC_LITERAL(23, 288, 9), // "Q_NOREPLY"
QT_MOC_LITERAL(24, 298, 9), // "hibernate"
QT_MOC_LITERAL(25, 308, 7), // "suspend"
QT_MOC_LITERAL(26, 316, 6), // "logout"
QT_MOC_LITERAL(27, 323, 6), // "reboot"
QT_MOC_LITERAL(28, 330, 8), // "powerOff"
QT_MOC_LITERAL(29, 339, 11), // "startModule"
QT_MOC_LITERAL(30, 351, 4), // "name"
QT_MOC_LITERAL(31, 356, 10), // "stopModule"
QT_MOC_LITERAL(32, 367, 7), // "Inhibit"
QT_MOC_LITERAL(33, 375, 6), // "app_id"
QT_MOC_LITERAL(34, 382, 12), // "toplevel_xid"
QT_MOC_LITERAL(35, 395, 6), // "reason"
QT_MOC_LITERAL(36, 402, 9), // "Uninhibit"
QT_MOC_LITERAL(37, 412, 6), // "cookie"
QT_MOC_LITERAL(38, 419, 13), // "GetInhibitors"
QT_MOC_LITERAL(39, 433, 16), // "IsSessionRunning"
QT_MOC_LITERAL(40, 450, 14), // "GetSessionName"
QT_MOC_LITERAL(41, 465, 11), // "IsInhibited"
QT_MOC_LITERAL(42, 477, 15), // "emitStartLogout"
QT_MOC_LITERAL(43, 493, 24), // "emitPrepareForSwitchuser"
QT_MOC_LITERAL(44, 518, 20), // "emitPrepareForPhase2"
QT_MOC_LITERAL(45, 539, 20) // "simulateUserActivity"

    },
    "SessionDBusAdaptor\0D-Bus Interface\0"
    "org.gnome.SessionManager\0moduleStateChanged\0"
    "\0moduleName\0state\0inhibitadded\0flags\0"
    "inhibitremove\0StartLogout\0"
    "PrepareForSwitchuser\0PrepareForPhase2\0"
    "startupfinished\0appName\0string\0canSwitch\0"
    "canHibernate\0canSuspend\0canLogout\0"
    "canReboot\0canPowerOff\0switchUser\0"
    "Q_NOREPLY\0hibernate\0suspend\0logout\0"
    "reboot\0powerOff\0startModule\0name\0"
    "stopModule\0Inhibit\0app_id\0toplevel_xid\0"
    "reason\0Uninhibit\0cookie\0GetInhibitors\0"
    "IsSessionRunning\0GetSessionName\0"
    "IsInhibited\0emitStartLogout\0"
    "emitPrepareForSwitchuser\0emitPrepareForPhase2\0"
    "simulateUserActivity"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SessionDBusAdaptor[] = {

 // content:
       8,       // revision
       0,       // classname
       1,   14, // classinfo
      31,   16, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // classinfo: key, value
       1,    2,

 // signals: name, argc, parameters, tag, flags
       3,    2,  171,    4, 0x06 /* Public */,
       7,    1,  176,    4, 0x06 /* Public */,
       9,    1,  179,    4, 0x06 /* Public */,
      10,    0,  182,    4, 0x06 /* Public */,
      11,    0,  183,    4, 0x06 /* Public */,
      12,    0,  184,    4, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      13,    2,  185,    4, 0x0a /* Public */,
      16,    0,  190,    4, 0x0a /* Public */,
      17,    0,  191,    4, 0x0a /* Public */,
      18,    0,  192,    4, 0x0a /* Public */,
      19,    0,  193,    4, 0x0a /* Public */,
      20,    0,  194,    4, 0x0a /* Public */,
      21,    0,  195,    4, 0x0a /* Public */,
      22,    0,  196,   23, 0x0a /* Public */,
      24,    0,  197,   23, 0x0a /* Public */,
      25,    0,  198,   23, 0x0a /* Public */,
      26,    0,  199,   23, 0x0a /* Public */,
      27,    0,  200,   23, 0x0a /* Public */,
      28,    0,  201,   23, 0x0a /* Public */,
      29,    1,  202,   23, 0x0a /* Public */,
      31,    1,  205,   23, 0x0a /* Public */,
      32,    4,  208,    4, 0x0a /* Public */,
      36,    1,  217,   23, 0x0a /* Public */,
      38,    0,  220,    4, 0x0a /* Public */,
      39,    0,  221,    4, 0x0a /* Public */,
      40,    0,  222,    4, 0x0a /* Public */,
      41,    1,  223,    4, 0x0a /* Public */,
      42,    0,  226,   23, 0x0a /* Public */,
      43,    0,  227,   23, 0x0a /* Public */,
      44,    0,  228,   23, 0x0a /* Public */,
      45,    0,  229,   23, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    5,    6,
    QMetaType::Void, QMetaType::UInt,    8,
    QMetaType::Void, QMetaType::UInt,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   14,   15,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   30,
    QMetaType::Void, QMetaType::QString,   30,
    QMetaType::UInt, QMetaType::QString, QMetaType::UInt, QMetaType::QString, QMetaType::UInt,   33,   34,   35,    8,
    QMetaType::Void, QMetaType::UInt,   37,
    QMetaType::QStringList,
    QMetaType::Bool,
    QMetaType::QString,
    QMetaType::Bool, QMetaType::UInt,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SessionDBusAdaptor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SessionDBusAdaptor *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->moduleStateChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->inhibitadded((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 2: _t->inhibitremove((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 3: _t->StartLogout(); break;
        case 4: _t->PrepareForSwitchuser(); break;
        case 5: _t->PrepareForPhase2(); break;
        case 6: _t->startupfinished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 7: { bool _r = _t->canSwitch();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 8: { bool _r = _t->canHibernate();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->canSuspend();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->canLogout();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 11: { bool _r = _t->canReboot();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 12: { bool _r = _t->canPowerOff();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 13: _t->switchUser(); break;
        case 14: _t->hibernate(); break;
        case 15: _t->suspend(); break;
        case 16: _t->logout(); break;
        case 17: _t->reboot(); break;
        case 18: _t->powerOff(); break;
        case 19: _t->startModule((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 20: _t->stopModule((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 21: { uint _r = _t->Inhibit((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< quint32(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< quint32(*)>(_a[4])));
            if (_a[0]) *reinterpret_cast< uint*>(_a[0]) = std::move(_r); }  break;
        case 22: _t->Uninhibit((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 23: { QStringList _r = _t->GetInhibitors();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        case 24: { bool _r = _t->IsSessionRunning();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 25: { QString _r = _t->GetSessionName();
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 26: { bool _r = _t->IsInhibited((*reinterpret_cast< quint32(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 27: _t->emitStartLogout(); break;
        case 28: _t->emitPrepareForSwitchuser(); break;
        case 29: _t->emitPrepareForPhase2(); break;
        case 30: _t->simulateUserActivity(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SessionDBusAdaptor::*)(QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionDBusAdaptor::moduleStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SessionDBusAdaptor::*)(quint32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionDBusAdaptor::inhibitadded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SessionDBusAdaptor::*)(quint32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionDBusAdaptor::inhibitremove)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SessionDBusAdaptor::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionDBusAdaptor::StartLogout)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SessionDBusAdaptor::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionDBusAdaptor::PrepareForSwitchuser)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (SessionDBusAdaptor::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionDBusAdaptor::PrepareForPhase2)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SessionDBusAdaptor::staticMetaObject = { {
    &QDBusAbstractAdaptor::staticMetaObject,
    qt_meta_stringdata_SessionDBusAdaptor.data,
    qt_meta_data_SessionDBusAdaptor,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SessionDBusAdaptor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SessionDBusAdaptor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SessionDBusAdaptor.stringdata0))
        return static_cast<void*>(this);
    return QDBusAbstractAdaptor::qt_metacast(_clname);
}

int SessionDBusAdaptor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDBusAbstractAdaptor::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 31)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 31;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 31)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 31;
    }
    return _id;
}

// SIGNAL 0
void SessionDBusAdaptor::moduleStateChanged(QString _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SessionDBusAdaptor::inhibitadded(quint32 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SessionDBusAdaptor::inhibitremove(quint32 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SessionDBusAdaptor::StartLogout()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SessionDBusAdaptor::PrepareForSwitchuser()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SessionDBusAdaptor::PrepareForPhase2()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
