#ifndef FILETREEBINDER_H
#define FILETREEBINDER_H

#include <QtCore/QObject>
#include <lean/tags/noncopyable.h>
#include <beCore/beFileWatch.h>
#include <QtGui/QIcon>

class QTreeWidget;
class QTreeWidgetItem;
class QFileSystemWatcher;

/// File tree binder utility class.
class FileTreeBinder : public QObject, public lean::noncopyable, protected beCore::DirectoryObserver
{
	Q_OBJECT

private:
	QTreeWidget *m_tree;
	QTreeWidgetItem *m_parentItem;

	QString m_location;
	QStringList m_masks;
	QIcon m_folderIcon;
	QIcon m_fileIcon;

protected:
	/// Called whenever an observed directory has changed.
	void DirectoryChanged(const lean::utf8_ntri &directory);

public:
	/// Constructor.
	FileTreeBinder(const QString &location, const QStringList &masks, const QIcon &icon,
		QTreeWidget *tree, QTreeWidgetItem *pParentItem, QObject *pParent = nullptr);
	/// Destructor.
	virtual ~FileTreeBinder();

	/// Sets the given tree view up for property editing.
	static void setupTree(QTreeWidget &view);

public Q_SLOTS:
	/// Updates the file tree.
	void updateTree();

Q_SIGNALS:
	/// The file tree has changed.
	void treeChanged();
};

#endif
