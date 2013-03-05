#include "stdafx.h"
#include "Windows/MeshImportDialog.h"

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

/// Restores the default mesh import settings.
void restoreDefaults(Ui::MeshImportDialog &ui, const QSettings &settings)
{
	switch ( settings.value("meshImportDialog/normals", 2).toUInt() )
	{
	case 0: ui.noNormalsRadioButton->setChecked(true); break;
	case 1: ui.normalsRadioButton->setChecked(true); break;
	case 2: ui.regenerateNormalsRadioButton->setChecked(true); break;
	}

	ui.smoothingAngleLineEdit->setText(QString::number( settings.value("meshImportDialog/smoothingAngle", 60.0f).toFloat() ));

	ui.tangentsCheckBox->setChecked( settings.value("meshImportDialog/tangents", true).toBool() );
	ui.bitangentsCheckBox->setChecked( settings.value("meshImportDialog/bitangents", false).toBool() );

	ui.colorsCheckBox->setChecked( settings.value("meshImportDialog/colors", false).toBool() );
	ui.texCoordsCheckBox->setChecked( settings.value("meshImportDialog/texCoords", true).toBool() );
	ui.forceTexCoordsCheckBox->setChecked( settings.value("meshImportDialog/forceTexCoords", false).toBool() );

	ui.scaleFactorLineEdit->setText(QString::number( settings.value("meshImportDialog/scaleFactor", 1.0f).toFloat() ));

	ui.indicesCheckBox->setChecked( settings.value("meshImportDialog/wideIndices", false).toBool() );
	ui.optimizationCheckBox->setChecked( settings.value("meshImportDialog/cacheOptimization", true).toBool() );

	ui.inputWidget->setPath( settings.value("meshImportDialog/inputPath", QDir::currentPath()).toString() );
	ui.outputWidget->setPath( settings.value("meshImportDialog/outputPath", QDir::currentPath()).toString() );
}

/// Saves the default mesh import settings.
void saveDefaults(const Ui::MeshImportDialog &ui, QSettings &settings)
{
	settings.setValue(
			"meshImportDialog/normals",
			(ui.regenerateNormalsRadioButton->isChecked()) ? 2 : (ui.normalsRadioButton->isChecked()) ? 1 : 0
		);

	settings.setValue("meshImportDialog/smoothingAngle", ui.smoothingAngleLineEdit->text().toFloat());

	settings.setValue("meshImportDialog/tangents", ui.tangentsCheckBox->isChecked());
	settings.setValue("meshImportDialog/bitangents", ui.bitangentsCheckBox->isChecked());

	settings.setValue("meshImportDialog/colors", ui.colorsCheckBox->isChecked());
	settings.setValue("meshImportDialog/texCoords", ui.texCoordsCheckBox->isChecked());
	settings.setValue("meshImportDialog/forceTexCoords", ui.forceTexCoordsCheckBox->isChecked());

	settings.setValue("meshImportDialog/scaleFactor", ui.scaleFactorLineEdit->text().toFloat());

	settings.setValue("meshImportDialog/wideIndices", ui.indicesCheckBox->isChecked());
	settings.setValue("meshImportDialog/cacheOptimization", ui.optimizationCheckBox->isChecked());

	settings.setValue("meshImportDialog/inputPath", absolutePathToDir(ui.inputWidget->path()));
	settings.setValue("meshImportDialog/outputPath", absolutePathToDir(ui.outputWidget->path()));
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
MeshImportDialog::MeshImportDialog(Editor *pEditor, QWidget *pParent, Qt::WindowFlags flags)
	: QDialog(pParent, flags),
	m_pEditor( LEAN_ASSERT_NOT_NULL(pEditor) )
{
	ui.setupUi(this);

	ui.smoothingAngleLineEdit->setValidator( new QDoubleValidator(ui.smoothingAngleLineEdit) );
	ui.scaleFactorLineEdit->setValidator( new QDoubleValidator(ui.scaleFactorLineEdit) );

	restoreDefaults(ui, *m_pEditor->settings());

	checkedConnect(ui.inputWidget, SIGNAL(browse()), this, SLOT(browseForInput()));
	checkedConnect(ui.outputWidget, SIGNAL(browse()), this, SLOT(browseForOutput()));
}

// Destructor.
MeshImportDialog::~MeshImportDialog()
{
}

// Browses for an input file.
void MeshImportDialog::browseForInput()
{
	QString file = QFileDialog::getOpenFileName( this,
		tr("Select an importable mesh"),
		input(),
		QString("%1 (*.dae *.obj *.3ds *.dxf *.x *.mdl *.*);;%2 (*.*)")
			.arg( MeshImportDialog::tr("Importable Meshes") )
			.arg( MeshImportDialog::tr("All Files") ) );
	
	if (!file.isEmpty())
		setInput(file);
}

// Browses for an output file.
void MeshImportDialog::browseForOutput()
{
	QString file = QFileDialog::getSaveFileName( this,
		tr("Select an output mesh"),
		output(),
		QString("%1 (*.mesh);;%2 (*.*)")
			.arg( MeshImportDialog::tr("breeze Meshes") )
			.arg( MeshImportDialog::tr("All Files") ) );

	if (!file.isEmpty())
		setOutput(file);
}

// Validates dialog data and accepts if valid.
void MeshImportDialog::accept()
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
				MeshImportDialog::tr("Create path?"),
				MeshImportDialog::tr("The specified path '%1' does not exist. Create?").arg(outputDirPath),
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
	rcCommand << "mesh";
	
	if (ui.noNormalsRadioButton->isChecked())
		rcCommand << "/VDn";
	if (ui.regenerateNormalsRadioButton->isChecked())
		rcCommand << "/Vsn";

	rcCommand << QString("/Vsna:%1").arg(ui.smoothingAngleLineEdit->text());

	if (ui.tangentsCheckBox->isChecked())
		rcCommand << "/Vtan";
	if (ui.bitangentsCheckBox->isChecked())
		rcCommand << "/Vbtan";

	if (ui.colorsCheckBox->isChecked())
		rcCommand << "/Vc";
	if (!ui.texCoordsCheckBox->isChecked())
		rcCommand << "/VDt";
	if (ui.forceTexCoordsCheckBox->isChecked())
		rcCommand << "/VFt";

	if (ui.indicesCheckBox->isChecked())
		rcCommand << "/Iw";

	if (ui.optimizationCheckBox->isChecked())
		rcCommand << "/O";

	rcCommand << QString("/Tsf:%1").arg(ui.scaleFactorLineEdit->text());

	rcCommand << ui.argsLineEdit->text().split(" ");

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
				tr("Import of mesh '%1' is taking longer. Continue?").arg(inputFile),
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
				tr("Mesh import failed"),
				tr("An error occurred while importing mesh '%1', see console for details.").arg(inputFile)
			);
}

// Sets the input file.
void MeshImportDialog::setInput(const QString &input)
{
	QString prevInput = this->input();
	QString prevOutput = this->output();

	ui.inputWidget->setPath(input);

	QFileInfo prevInputInfo(prevInput);
	QFileInfo prevOutputInfo(prevOutput);

	if (!prevOutputInfo.isFile() || prevInputInfo.baseName() == prevOutputInfo.baseName())
		setOutput( outputFromInput(input, prevInput, prevOutput, ".mesh") );
}

// Gets the input file.
QString MeshImportDialog::input() const
{
	return ui.inputWidget->path();
}

// Sets the output file.
void MeshImportDialog::setOutput(const QString &output)
{
	ui.outputWidget->setPath(output);
}

// Gets the output file.
QString MeshImportDialog::output() const
{
	return ui.outputWidget->path();
}

// Forwards console output to the editor's console.
void MeshImportDialog::forwardConsoleOutput()
{
	QProcess *pProcess = qobject_cast<QProcess*>(sender());

	if (pProcess)
		m_pEditor->write( QString::fromUtf8(pProcess->readAllStandardOutput()) );
}

// Browses for a mesh file.
QString browseForMesh(const QString &currentPath, Editor &editor, QWidget *pParent)
{
	QSettings &settings = *editor.settings();

	QString location = (!currentPath.isEmpty())
		? currentPath
		// Default location
		: settings.value("meshImportDialog/outputPath", QDir::currentPath()).toString();

	QString selectedFilter;

	// Open either breeze mesh or importable 3rd-party mesh format
	QString file = QFileDialog::getOpenFileName( pParent,
		MeshImportDialog::tr("Select a mesh resource"),
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
			MeshImportDialog meshImportDialog(&editor, pParent);
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

#include "Widgets/GenericComponentPicker.h"
#include "Widgets/ComponentPickerFactory.h"
#include "Plugins/FactoryManager.h"

#include <beScene/beAssembledMesh.h>

namespace
{

/// Plugin class.
struct MeshComponentPickerPlugin : public ComponentPickerFactory
{
	/// Constructor.
	MeshComponentPickerPlugin()
	{
		getComponentPickerFactories().addFactory(besc::AssembledMesh::GetComponentType()->Name, this);
	}

	/// Destructor.
	~MeshComponentPickerPlugin()
	{
		getComponentPickerFactories().removeFactory(besc::AssembledMesh::GetComponentType()->Name);
	}

	/// Creates a component picker.
	ComponentPicker* createComponentPicker(const beCore::ComponentReflector *reflector, const lean::any *pCurrent, Editor *editor, QWidget *pParent) const
	{
		return new GenericComponentPicker(reflector, pCurrent, editor, pParent);
	}

	/// Browses for a component resource.
	virtual QString browseForComponent(const beCore::ComponentReflector &reflector, const QString &currentPath, Editor &editor, QWidget *pParent) const
	{
		return browseForMesh(currentPath, editor, pParent);
	}
};

const MeshComponentPickerPlugin MeshComponentPickerPlugin;

} // namespace

#include "Plugins/AbstractPlugin.h"
#include "Plugins/PluginManager.h"
#include "Windows/MainWindow.h"
#include "Utility/SlotObject.h"

namespace
{

class MeshImportTool : public SlotObject
{
	Editor *editor;

public:
	MeshImportTool(Editor *editor, QWidget *parent)
		: SlotObject(parent),
		editor(editor) { }

	void slot()
	{
		MeshImportDialog dlg( editor, qobject_cast<QWidget*>(this->parent()) );
		dlg.exec();
	}
};

/// Plugin class.
struct MeshImportToolPlugin : public AbstractPlugin<MainWindow*>
{
	/// Constructor.
	MeshImportToolPlugin()
	{
		mainWindowPlugins().addPlugin(this);
	}

	/// Destructor.
	~MeshImportToolPlugin()
	{
		mainWindowPlugins().removePlugin(this);
	}

	/// Initializes the plugin.
	void initialize(MainWindow *mainWindow) const
	{
		MeshImportTool *tool = new MeshImportTool(mainWindow->editor(), mainWindow);

		checkedConnect(mainWindow->widgets().actionMesh_Import, SIGNAL(triggered()), tool, SLOT(slot()));
	}
	/// Finalizes the plugin.
	void finalize(MainWindow *pWindow) const { }
};

const MeshImportToolPlugin MeshImportToolPlugin;

} // namespace
