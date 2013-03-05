#ifndef CHANGEPROPERTYCOMMAND_H
#define CHANGEPROPERTYCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <beCore/bePropertyProvider.h>
#include <lean/properties/property.h>

#include <lean/smart/com_ptr.h>

/// Change property command class.
class ChangePropertyCommand : public QUndoCommand
{
private:
	lean::com_ptr<beCore::PropertyProvider> m_pPropertyProvider;
	uint4 m_propertyID;

	lean::scoped_property_data<> m_previousData;
	lean::scoped_property_data<> m_data;

	bool m_bIgnoreOnce;

public:
	/// Constructor.
	ChangePropertyCommand(beCore::PropertyProvider *pPropertyProvider, uint4 propertyID, QUndoCommand *pParent = nullptr);
	/// Destructor.
	virtual ~ChangePropertyCommand();

	/// Captures the property.
	void capture();

	/// Reverts the property.
	void undo();
	/// Changes the property.
    void redo();

	// Ignores the next call to redo().
	QUndoCommand* pushOnly()
	{
		m_bIgnoreOnce = true;
		return this;
	}

	/// Property provider.
	beCore::PropertyProvider* propertyProvider() const { return m_pPropertyProvider; }
	/// Property.
	uint4 propertyID() const { return m_propertyID; }
};

#endif
