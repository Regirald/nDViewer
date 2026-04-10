#pragma once
#include <QFileInfo>
#include <QString>

QString getSolutionDirectory() 
{
	return QString(SOLUTION_DIR);
}
QString getSnapShotDirectory() 
{
	return QString(SOLUTION_DIR) + "snapshots\\";
}

QString getAvailableFilename(const QString& baseName)
{
    QFileInfo info(baseName);

    QString path = info.path();
    QString base = info.completeBaseName();
    QString ext = info.suffix();
    QString candidate = baseName;
    int counter = 1;

    while (QFileInfo::exists(candidate))
    {
        candidate = QString("%1/%2_%3.%4")
            .arg(path)
            .arg(base)
            .arg(counter++)
            .arg(ext);
    }

    return candidate;
}