#ifndef GENERICPROPERTYBINDER_H
#define GENERICPROPERTYBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>

#include <beCore/bePropertyProvider.h>

class QTreeView;
class QStandardItem;
class QStandardItemModel;

/// Create entity command class.
class GenericPropertyBinder : public QObject, public lean::noncopyable
{
	Q_OBJECT

private:
	beCore::PropertyProvider *m_pPropertyProvider;

	QStandardItem *m_pParentItem;

public:
	/// Constructor.
	GenericPropertyBinder(beCore::PropertyProvider *pPropertyProvider, QTreeView *pTree, QStandardItem *pParentItem, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~GenericPropertyBinder();

	/// Sets the given item model up for property display.
	static void setupModel(QStandardItemModel &model);
	/// Correctly fills empty cells in the given row.
	static void fillRow(QStandardItem &rowItem);

	/// Property provider.
	beCore::PropertyProvider* propertyProvider() const { return m_pPropertyProvider; }

public Q_SLOTS:
	/// Property changed.
	void propertyChanged(QStandardItem *pValueItem);
};

#endif
