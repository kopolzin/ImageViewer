#ifndef LIBRARYDEMO_H
#define LIBRARYDEMO_H

#include <QPixmap>
#include <QImage>
#include <QStringList>
#include <QFile>
#include <QFuture>
#include <QFileSystemWatcher>

class ImageLibrary
{
private:
    QStringList filters;
public:
    ImageLibrary();
    QFuture<QImage> loadImage(const QString &filename);
    QStringList filenames(const QString &directory);
    void watchDirectory(const QString &directory);
    QFileSystemWatcher *fileSystemWatcher;

public slots:
    void setExtensionFilter(const QString &newFilters);
    QFuture<bool> saveImage(const QPixmap *image, const QString &filename);
};

#endif // LIBRARYDEMO_H
