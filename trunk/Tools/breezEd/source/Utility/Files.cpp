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
