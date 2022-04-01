/****************************************************************************
** Meta object code from reading C++ file 'modulemanager.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "modulemanager.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'modulemanager.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ModuleManager_t {
    QByteArrayData data[21];
    char stringdata0[227];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ModuleManager_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ModuleManager_t qt_meta_stringdata_ModuleManager = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ModuleManager"
QT_MOC_LITERAL(1, 14, 18), // "moduleStateChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 10), // "moduleName"
QT_MOC_LITERAL(4, 45, 5), // "state"
QT_MOC_LITERAL(5, 51, 8), // "finished"
QT_MOC_LITERAL(6, 60, 11), // "usdfinished"
QT_MOC_LITERAL(7, 72, 10), // "wmfinished"
QT_MOC_LITERAL(8, 83, 13), // "panelfinished"
QT_MOC_LITERAL(9, 97, 15), // "desktopfinished"
QT_MOC_LITERAL(10, 113, 13), // "startCompsite"
QT_MOC_LITERAL(11, 127, 6), // "logout"
QT_MOC_LITERAL(12, 134, 6), // "doExit"
QT_MOC_LITERAL(13, 141, 11), // "timerUpdate"
QT_MOC_LITERAL(14, 153, 6), // "timeup"
QT_MOC_LITERAL(15, 160, 6), // "weakup"
QT_MOC_LITERAL(16, 167, 3), // "arg"
QT_MOC_LITERAL(17, 171, 14), // "restartModules"
QT_MOC_LITERAL(18, 186, 8), // "exitCode"
QT_MOC_LITERAL(19, 195, 20), // "QProcess::ExitStatus"
QT_MOC_LITERAL(20, 216, 10) // "exitStatus"

    },
    "ModuleManager\0moduleStateChanged\0\0"
    "moduleName\0state\0finished\0usdfinished\0"
    "wmfinished\0panelfinished\0desktopfinished\0"
    "startCompsite\0logout\0doExit\0timerUpdate\0"
    "timeup\0weakup\0arg\0restartModules\0"
    "exitCode\0QProcess::ExitStatus\0exitStatus"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ModuleManager[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   74,    2, 0x06 /* Public */,
       5,    0,   79,    2, 0x06 /* Public */,
       6,    0,   80,    2, 0x06 /* Public */,
       7,    0,   81,    2, 0x06 /* Public */,
       8,    0,   82,    2, 0x06 /* Public */,
       9,    0,   83,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   84,    2, 0x0a /* Public */,
      11,    1,   85,    2, 0x0a /* Public */,
      13,    0,   88,    2, 0x0a /* Public */,
      14,    0,   89,    2, 0x0a /* Public */,
      15,    1,   90,    2, 0x0a /* Public */,
      17,    2,   93,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   16,
    QMetaType::Void, QMetaType::Int, 0x80000000 | 19,   18,   20,

       0        // eod
};

void ModuleManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ModuleManager *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->moduleStateChanged((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->finished(); break;
        case 2: _t->usdfinished(); break;
        case 3: _t->wmfinished(); break;
        case 4: _t->panelfinished(); break;
        case 5: _t->desktopfinished(); break;
        case 6: _t->startCompsite(); break;
        case 7: _t->logout((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->timerUpdate(); break;
        case 9: _t->timeup(); break;
        case 10: _t->weakup((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->restartModules((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< QProcess::ExitStatus(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ModuleManager::*)(QString , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ModuleManager::moduleStateChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ModuleManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ModuleManager::finished)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (ModuleManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ModuleManager::usdfinished)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (ModuleManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ModuleManager::wmfinished)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (ModuleManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ModuleManager::panelfinished)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (ModuleManager::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ModuleManager::desktopfinished)) {
                *result = 5;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ModuleManager::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_ModuleManager.data,
    qt_meta_data_ModuleManager,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ModuleManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ModuleManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ModuleManager.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QAbstractNativeEventFilter"))
        return static_cast< QAbstractNativeEventFilter*>(this);
    return QObject::qt_metacast(_clname);
}

int ModuleManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void ModuleManager::moduleStateChanged(QString _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ModuleManager::finished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void ModuleManager::usdfinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void ModuleManager::wmfinished()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void ModuleManager::panelfinished()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void ModuleManager::desktopfinished()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
