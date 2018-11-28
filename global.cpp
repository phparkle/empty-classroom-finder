#include "global.h"

#include <QStandardPaths>

static const QDir appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
static const QDir root = appDataLocation.filePath("empty-classroom-finder");

QDir getHtmlDir() {
    return root.filePath("html");
}
