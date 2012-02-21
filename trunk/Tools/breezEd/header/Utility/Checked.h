#ifndef CHECKED_H
#define CHECKED_H

#include "breezEd.h"
#include <QtCore/QObject>

/// Connects the given objects & checks the results.
inline void checkedConnect(const QObject *sender, const char *signal, const QObject *receiver, const char *method, Qt::ConnectionType type = Qt::AutoConnection)
{
	lean::check( QObject::connect(sender, signal, receiver, method) );
}

#endif