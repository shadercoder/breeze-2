#ifndef COMPONENTLISTBINDER_H
#define COMPONENTLISTBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>
#include <beCore/beFileWatch.h>
#include <QtGui/QIcon>

#include <beCore/beComponentTypes.h>

class QTreeView;
class QStandardItemModel;
class QStandardItem;

class SceneDocument;

/// Component list binder utility class.
class ComponentListBinder : public QObject, public lean::noncopyable
{
	Q_OBJECT

private:
	QTreeView *m_tree;
	QStandardItem *m_parentItem;
	
	QIcon m_componentIcon;

	const beCore::ComponentTypeDesc *m_type;
	SceneDocument *m_document;

protected:
	/// Called whenever an observed directory has changed.
	void DirectoryChanged(const lean::utf8_ntri &directory);

public:
	/// Constructor.
	ComponentListBinder(const beCore::ComponentTypeDesc *type, SceneDocument *document, const QIcon &icon,
		QTreeView *tree, QStandardItem *pParentItem, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~ComponentListBinder();

	/// Sets the given tree view up for component listing.
	static QStandardItemModel* setupTree(QTreeView &view);
	/// Sets the given model up for component listing.
	static void setupModel(QStandardItemModel &model);

	/// Gets the name of the component currently selected.
	QString selectedName() const;

public Q_SLOTS:
	/// Selects the given name.
	void selectName(const QString &componentName);
	/// Moves the cursor to the current item.
	void moveToCurrent();
	/// Updates the file tree.
	void updateTree();
	/// The selection has changed.
	void updateSelection();
	/// The name has changed.
	void updateName(const QModelIndex &start, const QModelIndex &end);

Q_SIGNALS:
	/// The selection has changed.
	void selectionChanged(const QString &name);
	/// The name has changed.
	void nameChanged(const QString &oldName, const QString &newName, bool &bSuccess);
};

#endif
