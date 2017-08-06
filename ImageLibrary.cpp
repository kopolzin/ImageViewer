#include "ImageLibrary.h"
#include <QDebug>
#include <QDir>
#include <QImageWriter>
#include <QPixmap>
#include <QtConcurrent>

ImageLibrary::ImageLibrary()
{
    fileSystemWatcher = new QFileSystemWatcher();
}

// Asynchronously load an image file and return a QImage
QFuture<QImage> ImageLibrary::loadImage(const QString &filename)
{
    auto loadImageWorker = [](const QString &filename) {
        QImage image;
        image.load(filename);
        return image;
    };

    return QtConcurrent::run(loadImageWorker, filename);
}

// Find image files in a directory
QStringList ImageLibrary::filenames(const QString &directory)
{
    QDir dir(directory);
    // Make sure directory exists
    if (dir.exists()) {
        return dir.entryList(filters,QDir::Files);
    } else {
        return { };
    }
}

// Monitor a directory for file changes
void ImageLibrary::watchDirectory(const QString &directory)
{
    QStringList existingPaths = fileSystemWatcher->directories();
    if (!existingPaths.isEmpty()) {
        fileSystemWatcher->removePaths(fileSystemWatcher->directories());
    }
    fileSystemWatcher->addPath(directory);
    // A signal is automatically emitted when files are changed
}

// Asynchronously save an image to a file. Depending on what image-modifying
// functions are implemented in the future, this may be changed to QImage::save
// rather than QPixmap::save. For now this works very quickly and effectively
// saves a scaled down version--the new file size depends on how large the Pixmap
// was displayed on the screen at the time of saving.
QFuture<bool> ImageLibrary::saveImage(const QPixmap *image, const QString &filename)
{
    auto saveImageWorker = [](const QPixmap *image, const QString &filename) {
        return image->save(filename);
    };

    return QtConcurrent::run(saveImageWorker, image, filename);
}

// Filter files by extension. Takes a space-separated list of wildcard-extensions.
// Example: *.tif *.tiff *.jpg
// Example 2: *   (all files will be shown)
void ImageLibrary::setExtensionFilter(const QString &newFilters)
{
    filters = newFilters.split(" ");
}
