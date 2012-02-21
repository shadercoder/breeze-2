#include "stdafx.h"
#include "Binders/GenericPropertyBinder.h"

#include <QtGui/QTreeView>

#include <beCore/bePropertyVisitor.h>
#include <beCore/beTextSerialization.h>
#include <sstream>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

struct PropertyValueReader : public beCore::DataVisitor
{
	QString value;
	QVector<QString> components;

	/// Visits the given values.
	void Visit(uint4 typeID, const lean::type_info &typeInfo, const void *values, size_t count)
	{
		const beCore::TextSerializer *pSerializer = beCore::GetTextSerialization().GetSerializer(typeID);

		if (pSerializer)
		{
			std::stringstream str;
			
			value = (pSerializer->Write(str, typeInfo.type, values, count))
				?  toQt( str.str() )
				: GenericPropertyBinder::tr("<error>");

			if (count > 1)
			{
				components.reserve( static_cast<int>(count) );

				for (size_t i = 0; i < count; ++i)
				{
					std::stringstream str;

					components.push_back( 
						(pSerializer->Write(str, typeInfo.type, lean::get_element(typeInfo.size, values, i), 1))
							?  toQt( str.str() )
							: GenericPropertyBinder::tr("<error>")
						);
				}
			}
		}
		else
			value = GenericPropertyBinder::tr("<error>");
	}
};

struct PropertyValueWriter : public beCore::DataVisitor
{
	QString value;
	uint4 component;

	PropertyValueWriter(const QString &value, uint4 component = static_cast<uint4>(-1))
		: value(value),
		component(component) { }

	/// Visits the given values.
	bool Visit(uint4 typeID, const lean::type_info &typeInfo, void *values, size_t count)
	{
		bool success = false;

		const beCore::TextSerializer *pSerializer = beCore::GetTextSerialization().GetSerializer(typeID);

		if (pSerializer)
		{
			if (component < count)
				success = pSerializer->Read(
					std::stringstream( toUtf8(value) ),
					typeInfo.type, lean::get_element(typeInfo.size, values, component), 1);
			else
				success = pSerializer->Read(
					std::stringstream( toUtf8(value) ),
					typeInfo.type, values, count);

			if (!success)
				QMessageBox::critical(
					nullptr,
					GenericPropertyBinder::tr("Invalid value"),
					GenericPropertyBinder::tr("An error occurred while trying to interpret value '%1'.").arg(value)
				);
		}
		else
			QMessageBox::critical(
					nullptr,
					GenericPropertyBinder::tr("Serializer unavailable"),
					GenericPropertyBinder::tr("No serializer available for property type '%1'.").arg( toQt(beCore::GetTypeIndex().GetName(typeID)) )
				);

		return success;
	}
};

} // namespace

// Constructor.
GenericPropertyBinder::GenericPropertyBinder(beCore::PropertyProvider *pPropertyProvider,
											 QTreeView *pTree, QStandardItem *pParentItem,
											 QObject *pParent)
	: QObject(pParent),
	m_pPropertyProvider( LEAN_ASSERT_NOT_NULL(pPropertyProvider) ),
	m_pParentItem(pParentItem)
{
	// Create new empty model, if none to be extended
	if (!m_pParentItem)
	{
		// WARNING: Don't attach to binder, binder might be attached to model
		QStandardItemModel *pModel = new QStandardItemModel(pTree);
		setupModel(*pModel);
		pTree->setModel(pModel);

		m_pParentItem = pModel->invisibleRootItem();
	}

	const uint4 propertyCount = m_pPropertyProvider->GetPropertyCount();

	for (uint4 i = 0; i < propertyCount; ++i)
	{
		QString propertyName = toQt( m_pPropertyProvider->GetPropertyName(i) );
		
		PropertyValueReader propertyReader;
		m_pPropertyProvider->ReadProperty(i, beCore::PropertyDataVisitor(propertyReader));

		// Property
		QStandardItem *pNameItem = new QStandardItem(propertyName);
		pNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		QStandardItem *pValueItem = new QStandardItem(propertyReader.value);
		pValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		m_pParentItem->appendRow( QList<QStandardItem*>() << pNameItem << pValueItem );

		beCore::PropertyDesc desc = m_pPropertyProvider->GetPropertyDesc(i);

		if (propertyReader.components.size() > 1)
		{
			const uint4 componentCount = propertyReader.components.size();

			// Components
			static const char *const vectorComponentNames[] = { "x", "y", "z", "w" };
			static const char *const colorComponenttNames[] = { "r", "g", "b", "a" };
			const char *const *componentNames = nullptr;

			// TODO: Widget?
			if (componentCount <= 4)
				componentNames = vectorComponentNames;

			for (uint4 c = 0; c < componentCount; ++c)
			{
				QStandardItem *pComponentNameItem = new QStandardItem( (componentNames) ? componentNames[c] : QString("[%1]").arg(c) );
				pComponentNameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
				QStandardItem *pComponentValueItem = new QStandardItem(propertyReader.components[c]);
				pComponentValueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
				pNameItem->appendRow( QList<QStandardItem*>() << pComponentNameItem << pComponentValueItem );
			}
		}
	}

	// WARNING: Direct connection is dangerous, crashes everything on inferred editor focus loss
	checkedConnect(m_pParentItem->model(), SIGNAL(itemChanged(QStandardItem*)), this, SLOT(propertyChanged(QStandardItem*)),
		Qt::QueuedConnection);
}

// Destructor.
GenericPropertyBinder::~GenericPropertyBinder()
{
}

// Property changed.
void GenericPropertyBinder::propertyChanged(QStandardItem *pValueItem)
{
	QStandardItem *pParentItem = pValueItem->parent();

	// Property
	if (pParentItem == m_pParentItem)
	{
		uint4 propertyID = pValueItem->index().row();

		PropertyValueWriter propertyWriter(pValueItem->text());
		m_pPropertyProvider->WriteProperty(propertyID, beCore::PropertyDataVisitor(propertyWriter));
	}
	// Property component
	else if (pParentItem && pParentItem->parent() == m_pParentItem)
	{
		uint4 propertyID = pParentItem->index().row();
		uint4 component = pValueItem->index().row();

		PropertyValueWriter propertyWriter(pValueItem->text(), component);
		m_pPropertyProvider->WriteProperty(propertyID, beCore::PropertyDataVisitor(propertyWriter), false);
	}
}

// Sets the given item model up for property display.
void GenericPropertyBinder::setupModel(QStandardItemModel &model)
{
	model.setHorizontalHeaderLabels( QStringList()
			<< GenericPropertyBinder::tr("Name")
			<< GenericPropertyBinder::tr("Value") );
}

// Correctly fills empty cells in the given row.
void GenericPropertyBinder::fillRow(QStandardItem &rowItem)
{
	QStandardItem *pParent = rowItem.parent();

	if (!pParent)
		pParent = rowItem.model()->invisibleRootItem();

	int row = rowItem.row();

	const int columnCount = pParent->columnCount();

	for (int i = 0; i < columnCount; ++i)
	{
		QStandardItem *pCellItem = pParent->child(row, i);

		if (!pCellItem)
		{
			pCellItem = new QStandardItem();
			pCellItem->setFlags(Qt::NoItemFlags);
			pCellItem->setEnabled( rowItem.isEnabled() );
			pCellItem->setSelectable( rowItem.isSelectable() );
			pParent->setChild(row, i, pCellItem);
		}
	}
}
