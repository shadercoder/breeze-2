#include "stdafx.h"
#include "Modes/ModeState.h"

#include "Utility/Checked.h"

// Constructor.
ModeState::ModeState(QObject *pParent)
	: AbstractModeState(pParent)
{
}

// Destructor.
ModeState::~ModeState()
{
}

// Called when the mode is entered.
void ModeState::onEntry(QEvent *pEvent)
{
	// Apply properties and establish connections
	handleEntry();
}

// Called when the mode is exited.
void ModeState::onExit(QEvent *pEvent)
{
	// Revert properties and break connections
	handleExit();
}

// Applies properties and establishes connections.
void ModeState::handleEntry()
{
	for (property_assignment_list::iterator itPropertyAssignment = m_propertyAssignments.begin();
		itPropertyAssignment != m_propertyAssignments.end(); itPropertyAssignment++)
	{
		// Store recovery value
		itPropertyAssignment->recoverValue = itPropertyAssignment->pObject->property(itPropertyAssignment->name);

		// Apply mode value
		lean::check( itPropertyAssignment->pObject->setProperty(itPropertyAssignment->name, itPropertyAssignment->value) );
	}

	Q_FOREACH (const Connection& connection, m_connections)
		// Establish connection
		checkedConnect(connection.pSender, connection.signal,
			connection.pReciever, connection.method,
			connection.type);
}

// Reverts properties and breaks connections.
void ModeState::handleExit()
{
	Q_FOREACH (const Connection& connection, m_connections)
		// Break connection
		disconnect(connection.pSender, connection.signal,
			connection.pReciever, connection.method);

	Q_FOREACH (const PropertyAssignment &propertyAssignment, m_propertyAssignments)
		// Recover value
		propertyAssignment.pObject->setProperty(propertyAssignment.name, propertyAssignment.recoverValue);
}

// Sets a property to be assigned when this mode is entered.
void ModeState::assignProperty(QObject *pObject, const char *name, const QVariant &value)
{
	if (!name)
	{
		qWarning("ModeState::assignProperty: cannot assign null property");
		return;
	}
	if (!pObject)
	{
		qWarning("ModeState::assignProperty: cannot assign property '%s' of null object", name);
		return;
	}

	for (property_assignment_list::iterator itPropertyAssignment = m_propertyAssignments.begin();
		itPropertyAssignment != m_propertyAssignments.end(); itPropertyAssignment++)
		// Check for redundant assignment calls
		if (itPropertyAssignment->pObject == pObject && itPropertyAssignment->name == name)
		{
			// Simply update the value
			itPropertyAssignment->value = value;
			return;
		}

	// Add new property assignment
	m_propertyAssignments.append( PropertyAssignment(pObject, name, value) );
}

// Adds a connection to be established when the mode is entered.
void ModeState::addConnection(const QObject *pSender, const char *pSignal,
	const QObject *pReciever, const char *pMethod, Qt::ConnectionType type)
{
	if (!pSignal)
	{
		qWarning("ModeState::addConnection: cannot connect null signal");
		return;
	}
	if (!pMethod)
	{
		qWarning("ModeState::addConnection: cannot connect to null slot");
		return;
	}
	if (!pSender)
	{
		qWarning("ModeState::addConnection: cannot recieve signal '%s' from null object", pSignal);
		return;
	}
	if (!pReciever)
	{
		qWarning("ModeState::addConnection: cannot connect to slot '%s' of null object", pMethod);
		return;
	}

	// Add connection to list
	m_connections.append( Connection(pSender, pSignal, pReciever, pMethod, type) );
}
