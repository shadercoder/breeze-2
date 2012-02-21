#ifndef FILEUTILITIES_H
#define FILEUTILITIES_H

#include "breezEd.h"
#include <QtCore/QString>

/// Gets the path to the given directory or the directory containing the given file.
QString absolutePathToDir(const QString &dirOrFile);

#endif