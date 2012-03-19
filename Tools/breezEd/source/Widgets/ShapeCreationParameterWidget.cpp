#include "stdafx.h"
#include "Widgets/ShapeCreationParameterWidget.h"

#include "Windows/ShapeImportDialog.h"

#include "Editor.h"
#include <QtCore/QSettings>

#include <QtGui/QFileDialog>

// Constructor.
ShapeCreationParameterWidget::ShapeCreationParameterWidget(const QString &name, Editor *pEditor, QWidget *pParent)
	: FileCreationParameterWidget(name, pEditor, pParent)
{
}

// Destructor.
ShapeCreationParameterWidget::~ShapeCreationParameterWidget()
{
}

// Browses for a file.
QString ShapeCreationParameterWidget::onBrowse(const QString &path)
{
	QSettings &settings = *m_pEditor->settings();

	QString location = path;

	// Default location
	if (location.isEmpty())
		location = settings.value("shapeImportDialog/outputPath", QDir::currentPath()).toString();

	QString selectedFilter;

	// Open either breeze shape or importable 3rd-party shape format
	QString file = QFileDialog::getOpenFileName( widget(),
		ShapeImportDialog::tr("Select a shape resource for '%1'").arg(name()),
		location,
		QString("%1 (*.shape);;%2 (*.dae *.obj *.3ds *.dxf *.x *.mdl *.*);;%3 (*.*)")
			.arg( ShapeImportDialog::tr("breeze Shapes") )
			.arg( ShapeImportDialog::tr("Importable Shapes") )
			.arg( ShapeImportDialog::tr("All Files") ),
		&selectedFilter );

	if (!file.isEmpty())
	{
		// Launch import dialog for 3rd-party shape formats
		if (!file.endsWith(".shape", Qt::CaseInsensitive) && selectedFilter.contains("*.dae"))
		{
			ShapeImportDialog shapeImportDialog(m_pEditor, widget());
			shapeImportDialog.setInput(file);
		
			file = (ShapeImportDialog::Accepted == shapeImportDialog.exec())
				? shapeImportDialog.output()
				: "";
		}
		else
			// Update default location
			settings.setValue("shapeImportDialog/outputPath", QFileInfo(file).absolutePath());
	}

	return file;
}

#include "Widgets/CreationParameterWidgetFactory.h"
#include "Plugins/WidgetFactoryManager.h"

namespace
{

/// Plugin class.
struct ShapeCreationParameterWidgetPlugin : public CreationParameterWidgetFactory
{
	/// Constructor.
	ShapeCreationParameterWidgetPlugin()
	{
		getCreationParameterWidgetFactory().addFactory("Shape", this);
	}

	/// Destructor.
	~ShapeCreationParameterWidgetPlugin()
	{
		getCreationParameterWidgetFactory().removeFactory("Shape");
	}

	/// Creates a creation parameter widget.
	CreationParameterWidget* createWidget(const QString &parameterName, Editor *pEditor, QWidget *pParent) const
	{
		return new ShapeCreationParameterWidget(parameterName, pEditor, pParent);
	}
};

const ShapeCreationParameterWidgetPlugin ShapeCreationParameterWidgetPlugin;

} // namespace
