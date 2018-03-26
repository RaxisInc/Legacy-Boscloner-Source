/****************************************************************************
** Meta object code from reading C++ file 'proxguiqt.h'
**
** Created: Tue Nov 17 14:20:58 2015
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "proxguiqt.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'proxguiqt.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ProxWidget[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

static const char qt_meta_stringdata_ProxWidget[] = {
    "ProxWidget\0"
};

const QMetaObject ProxWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_ProxWidget,
      qt_meta_data_ProxWidget, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProxWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProxWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProxWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProxWidget))
        return static_cast<void*>(const_cast< ProxWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int ProxWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_ProxGuiQT[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      11,   10,   10,   10, 0x05,
      35,   10,   10,   10, 0x05,
      62,   10,   10,   10, 0x05,

 // slots: signature, parameters, type, tag, flags
      86,   10,   10,   10, 0x08,
     105,   10,   10,   10, 0x08,
     127,   10,   10,   10, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ProxGuiQT[] = {
    "ProxGuiQT\0\0ShowGraphWindowSignal()\0"
    "RepaintGraphWindowSignal()\0"
    "HideGraphWindowSignal()\0_ShowGraphWindow()\0"
    "_RepaintGraphWindow()\0_HideGraphWindow()\0"
};

const QMetaObject ProxGuiQT::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_ProxGuiQT,
      qt_meta_data_ProxGuiQT, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ProxGuiQT::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ProxGuiQT::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ProxGuiQT::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ProxGuiQT))
        return static_cast<void*>(const_cast< ProxGuiQT*>(this));
    return QObject::qt_metacast(_clname);
}

int ProxGuiQT::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: ShowGraphWindowSignal(); break;
        case 1: RepaintGraphWindowSignal(); break;
        case 2: HideGraphWindowSignal(); break;
        case 3: _ShowGraphWindow(); break;
        case 4: _RepaintGraphWindow(); break;
        case 5: _HideGraphWindow(); break;
        default: ;
        }
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void ProxGuiQT::ShowGraphWindowSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 0, 0);
}

// SIGNAL 1
void ProxGuiQT::RepaintGraphWindowSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 1, 0);
}

// SIGNAL 2
void ProxGuiQT::HideGraphWindowSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 2, 0);
}
QT_END_MOC_NAMESPACE
