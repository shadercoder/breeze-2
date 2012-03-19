#include "stdafx.h"
#include "Binders/FileTreeBinder.h"

#include <QtGui/QTreeView>

#include <QtCore/QFileSystemWatcher>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

/// Adds items in the given directory to the given parent item.
QTreeWidgetItem* addFile(const QFileInfo &file, const QIcon &icon, QTreeWidgetItem *directoryItem)
{
	QTreeWidgetItem *fileItem = new QTreeWidgetItem();
	fileItem->setText(0, file.baseName());
	fileItem->setIcon(0, icon);
	fileItem->setToolTip(0, file.absoluteFilePath());
	fileItem->setText(1, file.completeSuffix());
	directoryItem->addChild(fileItem);
	return fileItem;
}

/// Adds items in the given directory to the given parent item.
QTreeWidgetItem* addDirectory(const QString &location, const QStringList &masks, const QIcon &icon, QTreeWidgetItem *parentItem, const QIcon &folderIcon)
{
	QDir dir(location);

	QTreeWidgetItem *directoryItem = new QTreeWidgetItem();
	directoryItem->setText(0, dir.dirName());
	directoryItem->setIcon(0, folderIcon);
	directoryItem->setToolTip(0, dir.absolutePath());
	directoryItem->setFlags(Qt::ItemIsEnabled);
	parentItem->addChild(directoryItem);

	QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

	Q_FOREACH (const QString &subdir, subdirs)
	{
		QTreeWidgetItem *subdirItem = addDirectory(dir.absoluteFilePath(subdir), masks, icon, directoryItem, folderIcon);
		
		// Skip irrelevant directories
		if (subdirItem->childCount() == 0)
			delete subdirItem;
	}

	QFileInfoList entries = dir.entryInfoList(masks, QDir::Files, QDir::Name);

	Q_FOREACH (const QFileInfo &entry, entries)
		addFile(entry, icon, directoryItem);

	return directoryItem;
}

/// Updates items in the given directory.
void updateDirectory(const QStringList &masks, const QIcon &icon, QTreeWidgetItem *directoryItem, const QIcon &folderIcon)
{
	// Delete all files
	for (int i = 0; i < directoryItem->childCount(); )
	{
		QTreeWidgetItem *item = directoryItem->child(i);

		if (item->flags() & Qt::ItemIsSelectable)
			delete item;
		else
			++i;
	}

	// ASSERT: Only directory skeleton left

	QDir dir(directoryItem->toolTip(0));

	QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	QTreeWidgetItem *pNextSubdirItem = directoryItem->child(0);

	for (QStringList::ConstIterator itSubdir = subdirs.begin(); itSubdir != subdirs.end(); )
	{
		const QString &subdir = *itSubdir;

		// Add new sub-directories
		if (!pNextSubdirItem || subdir < pNextSubdirItem->text(0))
		{
			QTreeWidgetItem *subdirItem = addDirectory(dir.absoluteFilePath(subdir), masks, icon, directoryItem, folderIcon);
			
			// Skip irrelevant directories
			if (subdirItem->childCount() == 0)
				delete subdirItem;
			// Move new directory in front of the next directory
			else if (pNextSubdirItem)
			{
				directoryItem->removeChild(subdirItem);
				directoryItem->insertChild(directoryItem->indexOfChild(pNextSubdirItem), subdirItem);
			}

			++itSubdir;
		}
		else
		{
			QTreeWidgetItem *subdirItem = pNextSubdirItem;

			// ORDER: Already move to next item (current might be deleted)
			pNextSubdirItem = directoryItem->child(directoryItem->indexOfChild(pNextSubdirItem) + 1);

			// Update existing sub-directories
			if (subdir == subdirItem->text(0))
			{
				updateDirectory(masks, icon, subdirItem, folderIcon);

				++itSubdir;
			}
			// Delete obsolete sub-directories
			else
				delete subdirItem;
		}
	}

	QFileInfoList entries = dir.entryInfoList(masks, QDir::Files, QDir::Name);

	Q_FOREACH (const QFileInfo &entry, entries)
		addFile(entry, icon, directoryItem);
}

} // namespace

// Constructor.
FileTreeBinder::FileTreeBinder(const QString &location, const QStringList &masks, const QIcon &icon,
		QTreeWidget *pTree, QTreeWidgetItem *pParentItem, QObject *pParent)
	: QObject(pParent),
	m_pTree( LEAN_ASSERT_NOT_NULL(pTree) ),
	m_location(location),
	m_masks(masks),
	m_fileIcon(icon)
{
	// Create new empty model, if none to be extended
	if (!pParentItem)
	{
		setupTree(*pTree);

		pParentItem = pTree->invisibleRootItem();
	}

	m_folderIcon.addFile(QString::fromUtf8(":/breezEd/icons/tree/folder"), QSize(), QIcon::Normal, QIcon::Off);
	m_folderIcon.addFile(QString::fromUtf8(":/breezEd/icons/tree/openFolder"), QSize(), QIcon::Normal, QIcon::On);

	addDirectory(m_location, m_masks, m_fileIcon, pParentItem, m_folderIcon);

	// Asynchronous
	connect(this, SIGNAL(treeChanged()), this, SLOT(updateTree()), Qt::QueuedConnection);

	// ORDER: Always last, exception safety!
	beCore::GetFileWatch().AddObserver(toUtf8(m_location), this);
}

// Destructor.
FileTreeBinder::~FileTreeBinder()
{
	// ORDER: Always first, exception safety!
	beCore::GetFileWatch().RemoveObserver(toUtf8(m_location), this);
}

// Sets the given tree view up for property editing.
void FileTreeBinder::setupTree(QTreeWidget &view)
{
	view.setHeaderLabels( QStringList()
			<< FileTreeBinder::tr("Name")
			<< FileTreeBinder::tr("Type")
		);
}

// Updates the file tree.
void FileTreeBinder::updateTree()
{
	updateDirectory(m_masks, m_fileIcon, m_pTree->invisibleRootItem()->child(0), m_folderIcon);
}

// Called whenever an observed directory has changed.
void FileTreeBinder::DirectoryChanged(const lean::utf8_ntri &directory)
{
	Q_EMIT treeChanged();
}
