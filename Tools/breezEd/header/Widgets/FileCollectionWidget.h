#ifndef FILECOLLECTIONWIDGET_H
#define FILECOLLECTIONWIDGET_H

#include "Utility/CollectionListWidget.h"

#include <beCore/beComponentTypes.h>

#include <lean/smart/scoped_ptr.h>
#include <lean/smart/cloneable_obj.h>
#include <lean/containers/any.h>

#include <vector>

class Editor;

class FileCollectionWidget : public CollectionListWidget
{
	Q_OBJECT

private:
	Editor *m_editor;

private Q_SLOTS:
	/// Editing has started.
	void startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled);
	/// Data currently being edited has changed.
	void updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled);
	/// Editor data is requested.
	void updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled);
	/// Editor to be relocated.
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled);

public:
	/// Constructor.
	FileCollectionWidget(Editor *editor, QWidget *pParent = nullptr);
	/// Destructor.
	~FileCollectionWidget();

Q_SIGNALS:
	/// Browser requested.
	void browse();
};

#endif
