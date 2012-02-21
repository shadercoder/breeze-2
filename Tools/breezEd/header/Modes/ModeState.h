#ifndef MODESTATE_H
#define MODESTATE_H

#include "breezEd.h"
#include "AbstractModeState.h"

#include <QtCore/QByteArray>
#include <QtCore/QVariant>

/// ModeState class
class ModeState : public AbstractModeState
{
	Q_OBJECT

private:
	/// Property assignment
	struct PropertyAssignment
	{
		QObject *pObject;		///< Property owner.
		QByteArray name;		///< Property name.
		QVariant value;			///< Property value.
		QVariant recoverValue;	///< Property recovery value.

		/// Default constructor.
		PropertyAssignment() : pObject() { };
		/// Constructor. Fully initializes this struct.
		PropertyAssignment(QObject *pObject, const QByteArray &name, const QVariant &value)
			: pObject(pObject), name(name), value(value) { };
	};

	// Property assignments
	typedef QList<PropertyAssignment> property_assignment_list;
	property_assignment_list m_propertyAssignments;

	/// Connection
	struct Connection
	{
		const QObject *pSender;
		QByteArray signal;
		const QObject *pReciever;
		QByteArray method;
		Qt::ConnectionType type;

		/// Default constructor.
		Connection() : pSender(), pReciever(), type(Qt::AutoConnection) { };
		/// Constructor. Fully initializes this structure.
		Connection(const QObject *pSender, const QByteArray &signal,
			const QObject *pReciever, const QByteArray &method,
			Qt::ConnectionType type = Qt::AutoConnection)
				: pSender(pSender), signal(signal),
				pReciever(pReciever), method(method), type(type) { };
	};

	// Connections
	typedef QList<Connection> connection_list;
	connection_list m_connections;

	// Applies properties and establishes connections.
	void handleEntry();
	/// Reverts properties and breaks connections.
	void handleExit();

protected:
	/// Called when the mode is entered.
	virtual void onEntry(QEvent *pEvent);
	/// Called when the mode is exited.
	virtual void onExit(QEvent *pEvent);

public:
	/// Constructor.
	ModeState(QObject *pParent = nullptr);
	/// Destructor.
	virtual ~ModeState();

	/// Sets a property to be assigned when this mode is entered.
	void assignProperty(QObject *pObject, const char *name, const QVariant &value);
	/// Adds a connection to be established when the mode is entered.
	void addConnection(const QObject *pSender, const char *pSignal,
		const QObject *pReciever, const char *pMethod, Qt::ConnectionType type = Qt::AutoConnection);
};

#endif
