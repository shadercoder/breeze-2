#ifndef SLOTOBJECT_H
#define SLOTOBJECT_H

#include <QtCore/QObject>

class SlotObject : public QObject
{
	Q_OBJECT

protected Q_SLOTS:
	virtual void slot() = 0;

public:
	SlotObject(QObject *parent)
		: QObject(parent) { }
};

#endif