#include "stdafx.h"
#include "Utility/Files.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>

// Gets the path to the given directory or the directory containing the given file.
QString absolutePathToDir(const QString &dirOrFile)
{
	QFileInfo info(dirOrFile);

	return (info.isFile() || !info.suffix().isEmpty() && !info.isDir())
		? info.absolutePath()
		: info.absoluteFilePath();
}

// Opens a file selection dialog.
QString openFileDialog(QWidget *parent, const QString &caption,
	QSettings &settings, const QString &locationSetting, const QString &currentFile,
	const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
	QString result;

	QString location = currentFile;

	// Fetch default location
	if (location.isEmpty())
		location = settings.value(locationSetting, QDir::currentPath()).toString();

	result = QFileDialog::getOpenFileName(
			parent, caption, location,
			filter, selectedFilter,
			options
		);

	// Update default location
	if (!result.isEmpty())
		settings.setValue(locationSetting, QFileInfo(result).absolutePath());

	return result;
}

// Opens a file selection dialog.
QString saveFileDialog(QWidget *parent, const QString &caption,
	QSettings &settings, const QString &locationSetting, const QString &currentFile,
	const QString &filter, QString *selectedFilter, QFileDialog::Options options)
{
	QString result;

	QString location = currentFile;

	// Fetch default location
	if (location.isEmpty())
		location = settings.value(locationSetting, QDir::currentPath()).toString();

	result = QFileDialog::getSaveFileName(
			parent, caption, location,
			filter, selectedFilter,
			options
		);

	// Update default location
	if (!result.isEmpty())
		settings.setValue(locationSetting, QFileInfo(result).absolutePath());

	return result;
}
