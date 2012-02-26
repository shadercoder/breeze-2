#include "stdafx.h"
#include "Widgets/FileCreationParameterWidget.h"

#include "Widgets/BrowseWidget.h"

#include <QtGui/QFileDialog>

#include <beCore/beExchangeContainers.h>

#include "Utility/Strings.h"
#include "Utility/Checked.h"

// Constructor.
FileCreationParameterWidget::FileCreationParameterWidget(const QString &name, Editor *pEditor, QWidget *pParent)
	: CreationParameterWidget( pEditor, pParent ),
	m_pWidget( new BrowseWidget(pParent) ),
	m_name(name)
{
	checkedConnect(m_pWidget, SIGNAL(browse()), this, SLOT(browse()));
}

// Destructor.
FileCreationParameterWidget::~FileCreationParameterWidget()
{
}

// Browses for a file.
void FileCreationParameterWidget::browse()
{
	QString path = onBrowse(m_pWidget->path());

	if (!path.isEmpty())
		m_pWidget->setPath(path);
}

// Browses for a file.
QString FileCreationParameterWidget::onBrowse(const QString &path)
{
	return QFileDialog::getOpenFileName(m_pWidget, tr("Select a resource for '%1'").arg(name()), path);
}

// Sets the creation parameter in the given set.
void FileCreationParameterWidget::setParameters(beCore::Parameters &parameters, SceneDocument &document) const
{
	// TODO: Improve parameter set?
	parameters.SetValue( parameters.Add( toUtf8Range(m_name) ), toUtf8Range(m_pWidget->path()).get().to<beCore::Exchange::utf8_string>() );
}

// Gets the widget.
BrowseWidget* FileCreationParameterWidget::widget()
{
	return m_pWidget;
}

// Gets the widget.
const BrowseWidget* FileCreationParameterWidget::widget() const
{
	return m_pWidget;
}

#include "Widgets/CreationParameterWidgetFactory.h"
#include "Plugins/WidgetFactoryManager.h"

namespace
{

/// Plugin class.
struct FileCreationParameterWidgetPlugin : public CreationParameterWidgetFactory
{
	/// Constructor.
	FileCreationParameterWidgetPlugin()
	{
		getCreationParameterWidgetFactory().addFactory("File", this);
		getCreationParameterWidgetFactory().addFactory("Resource", this);
	}

	/// Destructor.
	~FileCreationParameterWidgetPlugin()
	{
		getCreationParameterWidgetFactory().removeFactory("File");
		getCreationParameterWidgetFactory().removeFactory("Resource");
	}

	/// Creates a creation parameter widget.
	CreationParameterWidget* createWidget(const QString &parameterName, Editor *pEditor, QWidget *pParent) const
	{
		return new FileCreationParameterWidget(parameterName, pEditor, pParent);
	}
};

const FileCreationParameterWidgetPlugin FileCreationParameterWidgetPlugin;

} // namespace
