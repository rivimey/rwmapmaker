/****************************************************************************
** Meta object code from reading C++ file 'drawboard.h'
**
** Created: Thu Jan 28 21:05:00 2010
**      by: The Qt Meta Object Compiler version 61 (Qt 4.5.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "drawboard.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'drawboard.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 61
#error "This file was generated using the moc from 4.5.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DrawBoard[] = {

 // content:
       2,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   12, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors

 // signals: signature, parameters, type, tag, flags
      15,   11,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      38,   36,   10,   10, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DrawBoard[] = {
    "DrawBoard\0\0col\0colorChanged(QColor)\0"
    "c\0setColor(QColor)\0"
};

const QMetaObject DrawBoard::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_DrawBoard,
      qt_meta_data_DrawBoard, 0 }
};

const QMetaObject *DrawBoard::metaObject() const
{
    return &staticMetaObject;
}

void *DrawBoard::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DrawBoard))
        return static_cast<void*>(const_cast< DrawBoard*>(this));
    return QWidget::qt_metacast(_clname);
}

int DrawBoard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: colorChanged((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 1: setColor((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void DrawBoard::colorChanged(const QColor & _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
