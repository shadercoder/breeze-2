#include "stdafx.h"
#include "Windows/ShapeImportDialog.h"

#include <lean/io/filesystem.h>
#include <algorithm>

#include "Editor.h"
#include "Windows/MainWindow.h"

#include <QtWidgets/QFileDialog>
#include <QtCore/QSettings>

#include "Utility/Files.h"
#include "Utility/Strings.h"
#include "Utility/Checked.h"

namespace
{

/// Restores the default shape import settings.
void restoreDefaults(Ui::ShapeImportDialog &ui, const QSettings &settings)
{
	ui.scaleFactorLineEdit->setText(QString::number( settings.value("shapeImportDialog/scaleFactor", 1.0f).toFloat() ));

	ui.inputWidget->setPath( settings.value("shapeImportDialog/inputPath", QDir::currentPath()).toString() );
	ui.outputWidget->setPath( settings.value("shapeImportDialog/outputPath", QDir::currentPath()).toString() );
}

/// Saves the default shape import settings.
void saveDefaults(const Ui::ShapeImportDialog &ui, QSettings &settings)
{
	settings.setValue("shapeImportDialog/scaleFactor", ui.scaleFactorLineEdit->text().toFloat());

	settings.setValue("shapeImportDialog/inputPath", absolutePathToDir(ui.inputWidget->path()));
	settings.setValue("shapeImportDialog/outputPath", absolutePathToDir(ui.outputWidget->path()));
}

/// Generates an output file name from the given input file name.
QString outputFromInput(const QString &input, const QString &prevInput, const QString &prevOutput, const QString &outputExt)
{
	QString prevInputDir = absolutePathToDir(prevInput);
	QString inputDir = absolutePathToDir(input);

	QFileInfo outputDirInfo(
			absolutePathToDir(prevOutput),
			lean::relative_path<QString>(inputDir, prevInputDir)
		);
	QFileInfo outputInfo(
			outputDirInfo.absoluteFilePath(),
			QFileInfo(input).baseName() + outputExt
		);

	return outputInfo.absoluteFilePath();
}

} // namespace

// Constructor.
ShapeImportDialog::ShapeImportDialog(Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: QDialog(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	ui.setupUi(this);

	ui.scaleFactorLineEdit->setValidator( new QDoubleValidator(ui.scaleFactorLineEdit) );

	restoreDefaults(ui, *m_pEditor->settings());

	checkedConnect(ui.inputWidget, SIGNAL(browse()), this, SLOT(browseForInput()));
	checkedConnect(ui.outputWidget, SIGNAL(browse()), this, SLOT(browseForOutput()));
}

// Destructor.
ShapeImportDialog::~ShapeImportDialog()
{
}

// Browses for an input file.
void ShapeImportDialog::browseForInput()
{
	QString file = QFileDialog::getOpenFileName( this,
		tr("Select an importable collection of shapes"),
		input(),
		QString("%1 (*.dae *.obj *.3ds *.dxf *.x *.mdl *.*);;%2 (*.*)")
			.arg( ShapeImportDialog::tr("Importable Shape") )
			.arg( ShapeImportDialog::tr("All Files") ) );
	
	if (!file.isEmpty())
		setInput(file);
}

// Browses for an output file.
void ShapeImportDialog::browseForOutput()
{
	QString file = QFileDialog::getSaveFileName( this,
		tr("Select an output shape"),
		output(),
		QString("%1 (*.shape);;%2 (*.*)")
			.arg( ShapeImportDialog::tr("breeze Shapes") )
			.arg( ShapeImportDialog::tr("All Files") ) );

	if (!file.isEmpty())
		setOutput(file);
}

// Validates dialog data and accepts if valid.
void ShapeImportDialog::accept()
{
	saveDefaults(ui, *m_pEditor->settings());

	QString inputFile = input();
	QString outputFile = output();

	QDir outputDir = QFileInfo(outputFile).absoluteDir();
	QString outputDirPath = outputDir.absolutePath();

	// Check if output directory existent
	if (!outputDir.exists(outputDirPath))
	{
		QMessageBox::StandardButton result = QMessageBox::question( nullptr,
				ShapeImportDialog::tr("Create path?"),
				ShapeImportDialog::tr("The specified path '%1' does not exist. Create?").arg(outputDirPath),
				QMessageBox::Yes | QMessageBox::Cancel
			);

		// Prompt to create path
		if (result != QMessageBox::Yes)
			return;

		// Try to create path
		outputDir.mkpath(outputDirPath);

		// Check if path existent
		if (!outputDir.exists())
		{
			QMessageBox::critical( nullptr,
					tr("Error while creating path."),
					tr("The path '%1' could not be created.").arg(outputDirPath)
				);
			return;
		}
	}

	// Build resource compiler command
	QStringList rcCommand;
	rcCommand << "physics";
	
	rcCommand << QString("/Tsf:%1").arg(ui.scaleFactorLineEdit->text());

	rcCommand << inputFile;
	rcCommand << outputFile;

	QString berc = (ui.x64CheckBox->isChecked()) ? "berc_x64" : "berc";
	m_pEditor->write( berc + "\n\t" + rcCommand.join("\n\t") );

	// Run resource compiler
	QProcess rc;
	checkedConnect(&rc, SIGNAL(readyReadStandardOutput()), this, SLOT(forwardConsoleOutput()));
	rc.start(berc, rcCommand);
	
	// Allow for interruption
	while (!rc.waitForFinished(10000))
	{
		QMessageBox::StandardButton result = QMessageBox::question( nullptr,
				tr("Waiting"),
				tr("Import of shape collection '%1' is taking longer. Continue?").arg(inputFile),
				QMessageBox::Yes | QMessageBox::Cancel
			);

		if (result != QMessageBox::Yes)
			break;
	}

	m_pEditor->newLine();

	// Close dialog
	if (rc.exitCode() == 0)
		QDialog::accept();
	else
		QMessageBox::critical( nullptr,
				tr("Shape import failed"),
				tr("An error occurred while importing shape collection '%1', see console for details.").arg(inputFile)
			);
}

// Sets the input file.
void ShapeImportDialog::setInput(const QString &input)
{
	QString prevInput = this->input();
	QString prevOutput = this->output();

	ui.inputWidget->setPath(input);

	QFileInfo prevInputInfo(prevInput);
	QFileInfo prevOutputInfo(prevOutput);

	if (!prevOutputInfo.isFile() || prevInputInfo.baseName() == prevOutputInfo.baseName())
		setOutput( outputFromInput(input, prevInput, prevOutput, ".shape") );
}

// Gets the input file.
QString ShapeImportDialog::input() const
{
	return ui.inputWidget->path();
}

// Sets the output file.
void ShapeImportDialog::setOutput(const QString &output)
{
	ui.outputWidget->setPath(output);
}

// Gets the output file.
QString ShapeImportDialog::output() const
{
	return ui.outputWidget->path();
}

// Forwards console output to the editor's console.
void ShapeImportDialog::forwardConsoleOutput()
{
	QProcess *pProcess = qobject_cast<QProcess*>(sender());

	if (pProcess)
		m_pEditor->write( QString::fromUtf8(pProcess->readAllStandardOutput()) );
}


// Browses for a file.
QString browseForShape(const QString &currentPath, Editor &editor, QWidget *pParent)
{
	QSettings &settings = *editor.settings();

	QString location = (!currentPath.isEmpty())
		? currentPath
		// Default location
		: settings.value("shapeImportDialog/outputPath", QDir::currentPath()).toString();

	QString selectedFilter;

	// Open either breeze shape or importable 3rd-party shape format
	QString file = QFileDialog::getOpenFileName( pParent,
		ShapeImportDialog::tr("Select a shape resource for"),
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
			ShapeImportDialog shapeImportDialog(&editor, pParent);
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

#include "Widgets/GenericComponentPicker.h"
#include "Widgets/ComponentPickerFactory.h"
#include "Plugins/FactoryManager.h"

#include <bePhysics/beAssembledShape.h>

namespace
{

/// Plugin class.
struct ShapeComponentPickerPlugin : public ComponentPickerFactory
{
	/// Constructor.
	ShapeComponentPickerPlugin()
	{
		getComponentPickerFactories().addFactory(bepx::AssembledShape::GetComponentType()->Name, this);
	}

	/// Destructor.
	~ShapeComponentPickerPlugin()
	{
		getComponentPickerFactories().removeFactory(bepx::AssembledShape::GetComponentType()->Name);
	}

	/// Creates a component picker.
	ComponentPicker* createComponentPicker(const beCore::ComponentReflector *reflector, const lean::any *pCurrent, Editor *editor, QWidget *pParent) const
	{
		return new GenericComponentPicker(reflector, pCurrent, editor, pParent);
	}

	/// Browses for a component resource.
	virtual QString browseForComponent(const beCore::ComponentReflector &reflector, const QString &currentPath, Editor &editor, QWidget *pParent) const
	{
		return browseForShape(currentPath, editor, pParent);
	}
};

const ShapeComponentPickerPlugin ShapeComponentPickerPlugin;

} // namespace

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"
#include "Utility/SlotObject.h"

namespace
{

class ShapeImportTool : public SlotObject
{
	Editor *editor;

public:
	ShapeImportTool(Editor *editor, QWidget *parent)
		: SlotObject(parent),
		editor(editor) { }

	void slot()
	{
		ShapeImportDialog dlg( editor, qobject_cast<QWidget*>(this->parent()) );
		dlg.exec();
	}
};

/// Plugin class.
struct ShapeImportToolPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	ShapeImportToolPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~ShapeImportToolPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *mainWindow) const
	{
		ShapeImportTool *tool = new ShapeImportTool(mainWindow->editor(), mainWindow);

		checkedConnect(mainWindow->widgets().actionShape_Import, SIGNAL(triggered()), tool, SLOT(slot()));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const ShapeImportToolPlugin ShapeImportToolPlugin;

} // namespace