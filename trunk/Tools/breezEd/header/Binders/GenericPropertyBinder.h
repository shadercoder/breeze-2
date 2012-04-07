#ifndef GENERICPROPERTYBINDER_H
#define GENERICPROPERTYBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>

#include <beCore/beReflectedComponent.h>
#include <beCore/bePropertyListener.h>

class QTreeView;
class QStandardItem;
class QStandardItemModel;

class SceneDocument;

/// Create entity command class.
class GenericPropertyBinder : public QObject, public beCore::PropertyListener, public lean::noncopyable
{
	Q_OBJECT

private:
	beCore::PropertyProvider *m_pPropertyProvider;
	beCore::ReflectedComponent *m_pComponent;

	SceneDocument *m_pDocument;

	QTreeView *m_pTree;
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

	/// Called when the given propery might have changed.
	virtual void PropertyChanged(const beCore::PropertyProvider &provider);

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
