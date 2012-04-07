#include "stdafx.h"
#include "Windows/MeshImportDialog.h"

#include <lean/io/filesystem.h>
#include <algorithm>

#include "Editor.h"
#include "Windows/MainWindow.h"

#include <QtGui/QFileDialog>
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

	ui.scaleFactorLineEdit->setText(QString::number( settings.value("meshImportDialog/scaleFactor", 1.0f).toFloat() ));

	ui.indicesCheckBox->setChecked( settings.value("meshImportDialog/wideIndices", false).toBool() );

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

	settings.setValue("meshImportDialog/scaleFactor", ui.scaleFactorLineEdit->text().toFloat());

	settings.setValue("meshImportDialog/wideIndices", ui.indicesCheckBox->isChecked());

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
MeshImportDialog::MeshImportDialog(Editor *pEditor, QWidget *pParent, Qt::WFlags flags)
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

	if (!ui.indicesCheckBox->isChecked())
		rcCommand << "/Iw";

	rcCommand << QString("/Tsf:%1").arg(ui.scaleFactorLineEdit->text());

	rcCommand << inputFile;
	rcCommand << outputFile;

	// Run resource compiler
	QProcess rc;
	checkedConnect(&rc, SIGNAL(readyReadStandardOutput()), this, SLOT(forwardConsoleOutput()));
	rc.start("berc", rcCommand);
	
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
