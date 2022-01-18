/****************************************************************************
** Meta object code from reading C++ file 'sessionmanagercontext.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "sessionmanagercontext.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sessionmanagercontext.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SessionManagerDBusContext_t {
    QByteArrayData data[46];
    char stringdata0[561];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SessionManagerDBusContext_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SessionManagerDBusContext_t qt_meta_stringdata_SessionManagerDBusContext = {
    {
QT_MOC_LITERAL(0, 0, 25), // "SessionManagerDBusContext"
QT_MOC_LITERAL(1, 26, 18), // "moduleStateChanged"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 10), // "moduleName"
QT_MOC_LITERAL(4, 57, 5), // "state"
QT_MOC_LITERAL(5, 63, 12), // "inhibitadded"
QT_MOC_LITERAL(6, 76, 5), // "flags"
QT_MOC_LITERAL(7, 82, 13), // "inhibitremove"
QT_MOC_LITERAL(8, 96, 11), // "StartLogout"
QT_MOC_LITERAL(9, 108, 20), // "PrepareForSwitchuser"
QT_MOC_LITERAL(10, 129, 16), // "PrepareForPhase2"
QT_MOC_LITERAL(11, 146, 15), // "startupfinished"
QT_MOC_LITERAL(12, 162, 9), // "Q_NOREPLY"
QT_MOC_LITERAL(13, 172, 7), // "appName"
QT_MOC_LITERAL(14, 180, 6), // "string"
QT_MOC_LITERAL(15, 187, 9), // "canLogout"
QT_MOC_LITERAL(16, 197, 9), // "canSwitch"
QT_MOC_LITERAL(17, 207, 12), // "canHibernate"
QT_MOC_LITERAL(18, 220, 10), // "canSuspend"
QT_MOC_LITERAL(19, 231, 9), // "canReboot"
QT_MOC_LITERAL(20, 241, 11), // "canPowerOff"
QT_MOC_LITERAL(21, 253, 10), // "switchUser"
QT_MOC_LITERAL(22, 264, 9), // "hibernate"
QT_MOC_LITERAL(23, 274, 7), // "suspend"
QT_MOC_LITERAL(24, 282, 6), // "logout"
QT_MOC_LITERAL(25, 289, 6), // "reboot"
QT_MOC_LITERAL(26, 296, 8), // "powerOff"
QT_MOC_LITERAL(27, 305, 11), // "startModule"
QT_MOC_LITERAL(28, 317, 4), // "name"
QT_MOC_LITERAL(29, 322, 10), // "stopModule"
QT_MOC_LITERAL(30, 333, 7), // "Inhibit"
QT_MOC_LITERAL(31, 341, 6), // "app_id"
QT_MOC_LITERAL(32, 348, 12), // "toplevel_xid"
QT_MOC_LITERAL(33, 361, 6), // "reason"
QT_MOC_LITERAL(34, 368, 9), // "Uninhibit"
QT_MOC_LITERAL(35, 378, 6), // "cookie"
QT_MOC_LITERAL(36, 385, 13), // "GetInhibitors"
QT_MOC_LITERAL(37, 399, 16), // "IsSessionRunning"
QT_MOC_LITERAL(38, 416, 14), // "GetSessionName"
QT_MOC_LITERAL(39, 431, 11), // "IsInhibited"
QT_MOC_LITERAL(40, 443, 15), // "emitStartLogout"
QT_MOC_LITERAL(41, 459, 24), // "emitPrepareForSwitchuser"
QT_MOC_LITERAL(42, 484, 20), // "emitPrepareForPhase2"
QT_MOC_LITERAL(43, 505, 20), // "simulateUserActivity"
QT_MOC_LITERAL(44, 526, 22), // "on_serviceUnregistered"
QT_MOC_LITERAL(45, 549, 11) // "serviceName"

    },
    "SessionManagerDBusContext\0moduleStateChanged\0"
    "\0moduleName\0state\0inhibitadded\0flags\0"
    "inhibitremove\0StartLogout\0"
    "PrepareForSwitchuser\0PrepareForPhase2\0"
    "startupfinished\0Q_NOREPLY\0appName\0"
    "string\0canLogout\0canSwitch\0canHibernate\0"
    "canSuspend\0canReboot\0canPowerOff\0"
    "switchUser\0hibernate\0suspend\0logout\0"
    "reboot\0powerOff\0startModule\0name\0"
    "stopModule\0Inhibit\0app_id\0toplevel_xid\0"
    "reason\0Uninhibit\0cookie\0GetInhibitors\0"
    "IsSessionRunning\0GetSessionName\0"
    "IsInhibited\0emitStartLogout\0"
    "emitPrepareForSwitchuser\0emitPrepareForPhase2\0"
    "simulateUserActivity\0on_serviceUnregistered\0"
    "serviceName"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SessionManagerDBusContext[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      32,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,  174,    2, 0x06 /* Public */,
       5,    1,  179,    2, 0x06 /* Public */,
       7,    1,  182,    2, 0x06 /* Public */,
       8,    0,  185,    2, 0x06 /* Public */,
       9,    0,  186,    2, 0x06 /* Public */,
      10,    0,  187,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    2,  188,   12, 0x0a /* Public */,
      15,    0,  193,    2, 0x0a /* Public */,
      16,    0,  194,    2, 0x0a /* Public */,
      17,    0,  195,    2, 0x0a /* Public */,
      18,    0,  196,    2, 0x0a /* Public */,
      19,    0,  197,    2, 0x0a /* Public */,
      20,    0,  198,    2, 0x0a /* Public */,
      21,    0,  199,   12, 0x0a /* Public */,
      22,    0,  200,   12, 0x0a /* Public */,
      23,    0,  201,   12, 0x0a /* Public */,
      24,    0,  202,   12, 0x0a /* Public */,
      25,    0,  203,   12, 0x0a /* Public */,
      26,    0,  204,   12, 0x0a /* Public */,
      27,    1,  205,   12, 0x0a /* Public */,
      29,    1,  208,   12, 0x0a /* Public */,
      30,    4,  211,    2, 0x0a /* Public */,
      34,    1,  220,   12, 0x0a /* Public */,
      36,    0,  223,    2, 0x0a /* Public */,
      37,    0,  224,    2, 0x0a /* Public */,
      38,    0,  225,    2, 0x0a /* Public */,
      39,    1,  226,    2, 0x0a /* Public */,
      40,    0,  229,   12, 0x0a /* Public */,
      41,    0,  230,   12, 0x0a /* Public */,
      42,    0,  231,   12, 0x0a /* Public */,
      43,    0,  232,   12, 0x0a /* Public */,
      44,    1,  233,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::UInt,    6,
    QMetaType::Void, QMetaType::UInt,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   13,   14,
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
    QMetaType::Void, QMetaType::QString,   28,
    QMetaType::Void, QMetaType::QString,   28,
    QMetaType::UInt, QMetaType::QString, QMetaType::UInt, QMetaType::QString, QMetaType::UInt,   31,   32,   33,    6,
    QMetaType::Void, QMetaType::UInt,   35,
    QMetaType::QStringList,
    QMetaType::Bool,
    QMetaType::QString,
    QMetaType::Bool, QMetaType::UInt,    6,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   45,

       0        // eod
};

void SessionManagerDBusContext::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SessionManagerDBusContext *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->moduleStateChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->inhibitadded((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 2: _t->inhibitremove((*reinterpret_cast< quint32(*)>(_a[1]))); break;
        case 3: _t->StartLogout(); break;
        case 4: _t->PrepareForSwitchuser(); break;
        case 5: _t->PrepareForPhase2(); break;
        case 6: _t->startupfinished((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< const QString(*)>(_a[2]))); break;
        case 7: { bool _r = _t->canLogout();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 8: { bool _r = _t->canSwitch();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 9: { bool _r = _t->canHibernate();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 10: { bool _r = _t->canSuspend();
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
        case 31: _t->on_serviceUnregistered((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SessionManagerDBusContext::*)(QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionManagerDBusContext::moduleStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SessionManagerDBusContext::*)(quint32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionManagerDBusContext::inhibitadded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SessionManagerDBusContext::*)(quint32 );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionManagerDBusContext::inhibitremove)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SessionManagerDBusContext::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionManagerDBusContext::StartLogout)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SessionManagerDBusContext::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionManagerDBusContext::PrepareForSwitchuser)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (SessionManagerDBusContext::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SessionManagerDBusContext::PrepareForPhase2)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SessionManagerDBusContext::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_SessionManagerDBusContext.data,
    qt_meta_data_SessionManagerDBusContext,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SessionManagerDBusContext::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SessionManagerDBusContext::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SessionManagerDBusContext.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QDBusContext"))
        return static_cast< QDBusContext*>(this);
    return QObject::qt_metacast(_clname);
}

int SessionManagerDBusContext::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 32)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 32;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 32)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 32;
    }
    return _id;
}

// SIGNAL 0
void SessionManagerDBusContext::moduleStateChanged(QString _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SessionManagerDBusContext::inhibitadded(quint32 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SessionManagerDBusContext::inhibitremove(quint32 _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void SessionManagerDBusContext::StartLogout()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SessionManagerDBusContext::PrepareForSwitchuser()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SessionManagerDBusContext::PrepareForPhase2()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
