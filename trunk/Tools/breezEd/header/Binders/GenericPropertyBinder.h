#ifndef GENERICPROPERTYBINDER_H
#define GENERICPROPERTYBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>

#include <beCore/beReflectedComponent.h>
#include <beCore/beComponentObservation.h>

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class ItemDelegateEx;

class SceneDocument;

/// Create entity command class.
class GenericPropertyBinder : public QObject, public beCore::ComponentObserver, public lean::noncopyable
{
	Q_OBJECT

private:
	lean::com_ptr<beCore::PropertyProvider> m_pPropertyProvider;
	beCore::ReflectedComponent *m_pComponent;

	SceneDocument *m_pDocument;

	QTreeView *m_pTree;
	ItemDelegateEx *m_pDelegate;
	QStandardItem *m_pParentItem;

	int m_propertyStartIdx, m_propertyEndIdx;
	int m_componentStartIdx, m_componentEndIdx;

	bool m_bPropertiesChanged;

private Q_SLOTS:
	/// Editing has started.
	void startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled);
	/// Data currently being edited has changed.
	void updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled);
	/// Editor data is requested.
	void updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled);
	/// Data has changed.
	void dataUpdated(QAbstractItemModel *model, const QModelIndex &index);
	/// Editor to be relocated.
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled);

public:
	/// Constructor.
	GenericPropertyBinder(beCore::PropertyProvider *pPropertyProvider, beCore::ReflectedComponent *pComponent, SceneDocument *pDocument, 
		QTreeView *pTree, QStandardItem *pParentItem, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~GenericPropertyBinder();

	/// Called when properties in the given provider might have changed.
	void PropertyChanged(const beCore::PropertyProvider &provider) LEAN_OVERRIDE;
	/// Called when child components in the given provider might have changed.
	void ChildChanged(const beCore::ReflectedComponent &provider) LEAN_OVERRIDE;
	/// Called when the structure of the given component has changed.
	void StructureChanged(const beCore::Component &provider) LEAN_OVERRIDE;
	/// Called when the given component has been replaced.
	void ComponentReplaced(const beCore::Component &previous) LEAN_OVERRIDE;

	/// Sets the given item model up for property display.
	static void setupModel(QStandardItemModel &model);
	/// Sets the given tree view up for property editing.
	static void setupTree(QTreeView &view);
	/// Correctly fills empty cells in the given row.
	static void fillRow(QStandardItem &rowItem);

	/// Property provider.
	beCore::PropertyProvider* propertyProvider() const { return m_pPropertyProvider; }

public Q_SLOTS:
	/// Check for property changes.
	void updateProperties();

Q_SIGNALS:
	/// Properties have changed.
	void propertiesChanged();
	/// Propagates property update calls.
	void propagateUpdateProperties();
};

#endif
