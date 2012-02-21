#include "stdafx.h"
#include "Widgets/MeshCreationParameterWidget.h"

#include "Windows/MeshImportDialog.h"

#include "Editor.h"
#include <QtCore/QSettings>

#include <QtGui/QFileDialog>

// Constructor.
MeshCreationParameterWidget::MeshCreationParameterWidget(const QString &name, Editor *pEditor, QWidget *pParent)
	: FileCreationParameterWidget(name, pEditor, pParent)
{
}

// Destructor.
MeshCreationParameterWidget::~MeshCreationParameterWidget()
{
}

// Browses for a file.
QString MeshCreationParameterWidget::onBrowse(const QString &path)
{
	QSettings &settings = *m_pEditor->settings();

	QString location = path;

	// Default location
	if (location.isEmpty())
		location = settings.value("meshImportDialog/outputPath", QDir::currentPath()).toString();

	QString selectedFilter;

	// Open either breeze mesh or importable 3rd-party mesh format
	QString file = QFileDialog::getOpenFileName( widget(),
		MeshImportDialog::tr("Select a mesh resource for '%1'").arg(name()),
		location,
		QString("%1 (*.mesh);;%2 (*.dae *.obj *.3ds *.dxf *.x *.mdl *.*);;%3 (*.*)")
			.arg( MeshImportDialog::tr("breeze Meshes") )
			.arg( MeshImportDialog::tr("Importable Meshes") )
			.arg( MeshImportDialog::tr("All Files") ),
		&selectedFilter );

	if (!file.isEmpty())
	{
		// Launch import dialog for 3rd-party mesh formats
		if (!file.endsWith(".mesh", Qt::CaseInsensitive) && selectedFilter.contains("*.dae"))
		{
			MeshImportDialog meshImportDialog(m_pEditor, widget());
			meshImportDialog.setInput(file);
		
			file = (MeshImportDialog::Accepted == meshImportDialog.exec())
				? meshImportDialog.output()
				: "";
		}
		else
			// Update default location
			settings.setValue("meshImportDialog/outputPath", QFileInfo(file).absolutePath());
	}

	return file;
}

#include "Widgets/CreationParameterWidgetFactory.h"
#include "Plugins/WidgetFactoryManager.h"

namespace
{

/// Plugin class.
struct MeshCreationParameterWidgetPlugin : public CreationParameterWidgetFactory
{
	/// Constructor.
	MeshCreationParameterWidgetPlugin()
	{
		getCreationParameterWidgetFactory().addFactory("Mesh", this);
	}

	/// Destructor.
	~MeshCreationParameterWidgetPlugin()
	{
		getCreationParameterWidgetFactory().removeFactory("Mesh");
	}

	/// Creates a creation parameter widget.
	CreationParameterWidget* createWidget(const QString &parameterName, Editor *pEditor, QWidget *pParent) const
	{
		return new MeshCreationParameterWidget(parameterName, pEditor, pParent);
	}
};

const MeshCreationParameterWidgetPlugin MeshCreationParameterWidgetPlugin;

} // namespace
