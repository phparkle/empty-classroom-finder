#include "global.h"

#include <QStandardPaths>

static const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
static const QString subdir = "empty-classroom-finder";

const QDir dataDir = QDir(dir).filePath(subdir);
