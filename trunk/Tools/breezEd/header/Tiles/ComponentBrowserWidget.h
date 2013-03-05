#ifndef COMPONENTBROWSERWIDGET_H
#define COMPONENTBROWSERWIDGET_H

#include "ui_ComponentBrowserWidget.h"
#include "Utility/CollectionListWidget.h"

#include <beCore/beComponentTypes.h>

#include <lean/smart/scoped_ptr.h>
#include <lean/smart/cloneable_obj.h>
#include <lean/containers/any.h>

#include <vector>

class Editor;
class MainWindow;
class AbstractDocument;
class SceneDocument;
class ComponentListBinder;

/// Entity builder tool.
class ComponentBrowserWidget : public QWidget
{
	Q_OBJECT

private:
	Ui::ComponentBrowserWidget ui;

	Editor *m_editor;

	const beCore::ComponentTypeDesc *m_type;

	SceneDocument *m_pDocument;
	lean::scoped_ptr<ComponentListBinder> m_pListBinder;

private Q_SLOTS:
	/// Synchronization changed.
	void syncChanged();
	/// Selection changed.
	void selectionChanged(const QString &componentName);
	/// Name changed.
	void nameChanged(const QString &oldName, const QString &newName, bool &bSuccess);
	/// Processes component management changes.
	void processChanges();
	/// Focus changed.
	void focusChanged(QWidget *prev, QWidget *next);
	/// Adds a new component.
	void newComponent();
	/// Replaces the selected component.
	void replaceComponent();
	/// Loads a new component.
	void loadComponent();
	/// Toggles the mode.
	void toggleMode();
	/// Browses for a component file.
	void browse();
	/// Reacts to file or mode changes.
	void adapt();

public:
	/// Constructor.
	ComponentBrowserWidget(const beCore::ComponentTypeDesc *type, Editor *editor, QWidget *pParent = nullptr, Qt::WindowFlags flags = 0);
	/// Destructor.
	~ComponentBrowserWidget();

	/// Gets the selected component.
	lean::cloneable_obj<lean::any, lean::ptr_sem> selectedComponent() const;

	/// Sets the browser color.
	void setColor(QColor color);
	/// Gets the browser color.
	QColor color() const;

	/// Checks whether the browser is in use.
	bool inUse() const;

	/// Gets the editor.
	Editor* editor() const { return m_editor; }
	/// Gets the component type.
	const beCore::ComponentTypeDesc* componentType() const { return m_type; }

public Q_SLOTS:
	// Sets the current document.
	void setDocument(AbstractDocument *pDocument);
	/// Selects the given component.
	void selectComponent(const lean::any *pComponent);
	/// Moves the cursor to the current item.
	void moveToCurrent();
	/// Synchonizes the creation UI with the given component.
	void syncUI(const QString &componentName);

Q_SIGNALS:
	/// A component has been selected.
	void componentSelected(const lean::any *component, const QString &componentName);
	/// Focus has been lost.
	void focusLost(ComponentBrowserWidget *browser);
};

class ComponentCollectionWidget : public CollectionListWidget
{
	Q_OBJECT

private:
	Editor *m_editor;
	const beCore::ComponentTypeDesc *m_type;

	typedef QList< lean::cloneable_obj<lean::any, lean::ptr_sem> > components_t;
	components_t m_components;
	
private Q_SLOTS:
	/// Editing has started.
	void startEditing(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *&pEditor, bool &bHandled);
	/// Data currently being edited has changed.
	void updateEditor(QWidget *editor, const QModelIndex &index, bool &bHandled);
	/// Editor data is requested.
	void updateData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index, bool &bHandled);
	/// Editor to be relocated.
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index, bool &bHandled);
	/// Commits data and closes the editor.
	void selectionFinished();
	/// Item added.
	void prepareItem(QStandardItem *item);
	/// Item removed.
	void finalizeItem(QStandardItem *item);

public:
	/// Constructor.
	ComponentCollectionWidget(const beCore::ComponentTypeDesc *type, Editor *editor, QWidget *pParent = nullptr);
	/// Destructor.
	~ComponentCollectionWidget();

	/// Adds the given component.
	void addComponent(const lean::any *pComponent, const QString &componentName);

	typedef std::vector< lean::cloneable_obj<lean::any, lean::ptr_sem> > ComponentVector;
	/// Gets the components.
	ComponentVector components() const;
};

class ComponentSelectorWidget : public QLineEdit
{
	Q_OBJECT

private:
	Editor *m_editor;
	const beCore::ComponentTypeDesc *m_type;
	lean::cloneable_obj<lean::any, lean::ptr_sem> m_pComponent;
	ComponentBrowserWidget *m_pBrowser;
	
	bool m_bJustEnteredByMouse;

protected:
	void focusInEvent(QFocusEvent *event) LEAN_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *event) LEAN_OVERRIDE; 

public:
	/// Constructor.
	ComponentSelectorWidget(ComponentBrowserWidget *browser, QWidget *pParent = nullptr);
	/// Constructor.
	ComponentSelectorWidget(const beCore::ComponentTypeDesc *type, Editor *editor, QWidget *pParent = nullptr);
	/// Destructor.
	~ComponentSelectorWidget();

	/// Links this selector to the given component browser.
	void linkToBrowser(ComponentBrowserWidget *pBrowser, bool bVolatile = false);
	
	/// Links this selector to a component browser.
	void link();
	/// Unlinks this selector from its current browser.
	void unlink();
	/// Toggles linked state.
	void toggleLinked();

	/// Gets the component.
	const lean::any* component() const { return m_pComponent; }

public Q_SLOTS:
	/// Sets the given component.
	void setComponent(const lean::any *pComponent, const QString &componentName);

Q_SIGNALS:
	/// The selected component has changed.
	void componentChanged(const lean::any *pComponent);
	/// A component has been selected.
	void selectionFinished();
};

/// Opens a component browser for the given component type.
ComponentBrowserWidget* openBrowser(const QString &componentTypeName, MainWindow *mainWindow, QWidget *pAvoidOverlap = nullptr, bool bHidden = false);
/// Opens or retrieves an unused open component browser for the given component type.
ComponentBrowserWidget* retrieveBrowser(const bec::ComponentTypeDesc *componentType, MainWindow *mainWindow, QWidget *pAvoidOverlap = nullptr, bool *pLayoutChanged = nullptr);

#endif
