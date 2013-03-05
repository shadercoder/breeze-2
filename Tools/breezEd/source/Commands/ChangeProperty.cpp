#include "stdafx.h"
#include "Commands/ChangeProperty.h"

#include <QtCore/QCoreApplication>
#include "Utility/Strings.h"

namespace
{

/// Creates a property data object for the given property.
lean::scoped_property_data<> createPropertyData(const beCore::PropertyProvider &provider, uint4 propertyID)
{
	beCore::PropertyDesc desc = provider.GetPropertyDesc(propertyID);
	return lean::scoped_property_data<>(*desc.TypeDesc->Info.property_type, desc.Count);
}

} // namespace

// Constructor.
ChangePropertyCommand::ChangePropertyCommand(beCore::PropertyProvider *pPropertyProvider, uint4 propertyID, QUndoCommand *pParent)
	: QUndoCommand(
		QCoreApplication::translate("ChangePropertyCommand", "Changed property '%1'").arg(
				makeName(toQt( LEAN_ASSERT_NOT_NULL(pPropertyProvider)->GetPropertyName(propertyID) ))
			),
		pParent ),
	m_pPropertyProvider( LEAN_ASSERT_NOT_NULL(pPropertyProvider) ),
	m_propertyID( propertyID ),
	m_previousData( createPropertyData(*m_pPropertyProvider, m_propertyID), lean::consume ),
	m_data( createPropertyData(*m_pPropertyProvider, m_propertyID), lean::consume ),
	m_bIgnoreOnce(false)
{
	m_pPropertyProvider->GetProperty(m_propertyID, m_previousData.property_type().type_info().type, m_previousData.data(), m_previousData.count());
	m_pPropertyProvider->GetProperty(m_propertyID, m_data.property_type().type_info().type, m_data.data(), m_data.count());
}

// Destructor.
ChangePropertyCommand::~ChangePropertyCommand()
{
}

// Captures the property.
void ChangePropertyCommand::capture()
{
	m_pPropertyProvider->GetProperty(m_propertyID, m_data.property_type().type_info().type, m_data.data(), m_data.count());
}

// Reverts the property.
void ChangePropertyCommand::undo()
{
	m_pPropertyProvider->SetProperty(m_propertyID, m_previousData.property_type().type_info().type, m_previousData.data(), m_previousData.count());
}

// Changes the property.
void ChangePropertyCommand::redo()
{
	if (!m_bIgnoreOnce)
		m_pPropertyProvider->SetProperty(m_propertyID, m_data.property_type().type_info().type, m_data.data(), m_data.count());
	else
		m_bIgnoreOnce = false;
}
