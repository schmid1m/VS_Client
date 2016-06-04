/****************************************************************************
** Meta object code from reading C++ file 'vs_client.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "vs_client.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'vs_client.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_VS_Client_t {
    QByteArrayData data[14];
    char stringdata0[125];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VS_Client_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VS_Client_t qt_meta_stringdata_VS_Client = {
    {
QT_MOC_LITERAL(0, 0, 9), // "VS_Client"
QT_MOC_LITERAL(1, 10, 14), // "startReceiving"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 6), // "newMsg"
QT_MOC_LITERAL(4, 33, 3), // "msg"
QT_MOC_LITERAL(5, 37, 6), // "packet"
QT_MOC_LITERAL(6, 44, 8), // "u_int8_t"
QT_MOC_LITERAL(7, 53, 3), // "err"
QT_MOC_LITERAL(8, 57, 9), // "u_int32_t"
QT_MOC_LITERAL(9, 67, 6), // "src_ip"
QT_MOC_LITERAL(10, 74, 13), // "sendBroadcast"
QT_MOC_LITERAL(11, 88, 15), // "cleanServerList"
QT_MOC_LITERAL(12, 104, 10), // "msgReceive"
QT_MOC_LITERAL(13, 115, 9) // "decodeMsg"

    },
    "VS_Client\0startReceiving\0\0newMsg\0msg\0"
    "packet\0u_int8_t\0err\0u_int32_t\0src_ip\0"
    "sendBroadcast\0cleanServerList\0msgReceive\0"
    "decodeMsg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VS_Client[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    3,   45,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      10,    0,   52,    2, 0x08 /* Private */,
      11,    0,   53,    2, 0x08 /* Private */,
      12,    0,   54,    2, 0x08 /* Private */,
      13,    3,   55,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 6, 0x80000000 | 8,    5,    7,    9,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 6, 0x80000000 | 8,    5,    7,    9,

       0        // eod
};

void VS_Client::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        VS_Client *_t = static_cast<VS_Client *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->startReceiving(); break;
        case 1: _t->newMsg((*reinterpret_cast< msg(*)>(_a[1])),(*reinterpret_cast< u_int8_t(*)>(_a[2])),(*reinterpret_cast< u_int32_t(*)>(_a[3]))); break;
        case 2: _t->sendBroadcast(); break;
        case 3: _t->cleanServerList(); break;
        case 4: _t->msgReceive(); break;
        case 5: _t->decodeMsg((*reinterpret_cast< msg(*)>(_a[1])),(*reinterpret_cast< u_int8_t(*)>(_a[2])),(*reinterpret_cast< u_int32_t(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (VS_Client::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&VS_Client::startReceiving)) {
                *result = 0;
            }
        }
        {
            typedef void (VS_Client::*_t)(msg , u_int8_t , u_int32_t );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&VS_Client::newMsg)) {
                *result = 1;
            }
        }
    }
}

const QMetaObject VS_Client::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_VS_Client.data,
      qt_meta_data_VS_Client,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *VS_Client::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VS_Client::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_VS_Client.stringdata0))
        return static_cast<void*>(const_cast< VS_Client*>(this));
    return QObject::qt_metacast(_clname);
}

int VS_Client::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void VS_Client::startReceiving()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void VS_Client::newMsg(msg _t1, u_int8_t _t2, u_int32_t _t3)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
