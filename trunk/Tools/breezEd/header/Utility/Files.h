#ifndef FILEUTILITIES_H
#define FILEUTILITIES_H

#include "breezEd.h"
#include <QtCore/QString>

#include <QtCore/QSettings>
#include <QtWidgets/QFileDialog>

/// Gets the path to the given directory or the directory containing the given file.
QString absolutePathToDir(const QString &dirOrFile);

/// Opens a file selection dialog.
QString openFileDialog(QWidget *parent, const QString &caption,
	QSettings &settings, const QString &locationSetting, const QString &currentFile,
	const QString &filter, QString *selectedFilter = nullptr, QFileDialog::Options options = 0);

/// Opens a file selection dialog.
QString saveFileDialog(QWidget *parent, const QString &caption,
	QSettings &settings, const QString &locationSetting, const QString &currentFile,
	const QString &filter, QString *selectedFilter = nullptr, QFileDialog::Options options = 0);

#endif