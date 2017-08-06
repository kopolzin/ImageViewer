#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QStandardPaths>
#include <QDesktopWidget>
#include <QRect>
#include <QGridLayout>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include <QImage>
#include <QFutureWatcher>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageLibrary = new ImageLibrary();

    QString dir = QDir::homePath();
    QString defaultFilters = "*.tif";
    imageLibrary->setExtensionFilter(defaultFilters);

    // Create a widget that lists image files found in the current directory
    fileListView = new QListView();
    fileListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    fileListModel = new QStringListModel;
    auto list = imageLibrary->filenames(dir);
    fileListModel->setStringList(list);
    fileListView->setModel(fileListModel);

    // Create a widget that displays the image of the selected file
    imageWidget = new QWidget();
    vBoxImageWidget = new QVBoxLayout();
    imageLabel = new QLabel();

    // Set the label's maximum size so large images fit on the primary monitor
    QDesktopWidget desktopWidget;
    auto screenSize = desktopWidget.availableGeometry(desktopWidget.primaryScreen());
    imageLabel->setMaximumWidth(screenSize.width()-200);
    imageLabel->setMaximumHeight(screenSize.height()-200);

    // Create a button to save the current image to a file
    saveImageButton = new QPushButton();
    saveImageButton->setText("Save Image");
    connect(saveImageButton,
            &QPushButton::clicked,
            this,
            [=]() mutable {
                    auto saveTo = QFileDialog::getSaveFileName(this,"Save Image",
                            directoryInput->text() + "/" + fileListModel->data(fileListView->currentIndex()).toString(),
                            "Images (*.tif *.tiff");
                    if (saveTo == "" || imageLabel->pixmap() == 0)
                        return;

                    QFuture<bool> savingFuture = imageLibrary->saveImage(imageLabel->pixmap(), saveTo);
                    savingWatcher = new QFutureWatcher<bool>();

                    QObject::connect(savingWatcher,
                             &QFutureWatcher<bool>::finished,
                             [=]() {
                                    if (savingFuture.result()) {
                                        qDebug() << "Saving was successful.";
                                    } else {
                                        qDebug() << "Saving failed.";
                                    }
                             });

                    savingWatcher->setFuture(savingFuture);
                 }
            );
    saveImageWidget = new QWidget();
    saveImageWidget->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    saveImageLayout = new QHBoxLayout();
    saveImageLayout->addWidget(saveImageButton);
    saveImageWidget->setLayout(saveImageLayout);

    vBoxImageWidget->addWidget(imageLabel);
    vBoxImageWidget->addWidget(saveImageWidget);
    imageWidget->setLayout(vBoxImageWidget);

    // When a filename is clicked, display its image
    connect(fileListView,
            &QListView::clicked,
            this,
            [=]() mutable {
                    currentFilePath = directoryInput->text() + "/" +
                            fileListModel->data(fileListView->currentIndex()).toString();
                    updatePixmap(currentFilePath);
                 }
            );

    // When a filename is highlighted and the enter key is pressed, display its image
    connect(fileListView,
            &QListView::activated,
            this,
            [=]() mutable {
                    currentFilePath = directoryInput->text() + "/" +
                            fileListModel->data(fileListView->currentIndex()).toString();
                    updatePixmap(currentFilePath);
                 }
            );

    // Create a widget for editing settings
    settingsWidget = new QWidget();
    settingsLayout = new QVBoxLayout();
    settingsWidget->setLayout(settingsLayout);

    // Settings section

    // Select directory to watch
    hBoxDirectoryInput = new QHBoxLayout();
    directoryInput = new QLineEdit(dir);
    directoryInput->setReadOnly(true);
    selectDirectoryButton = new QPushButton();
    selectDirectoryButton->setText("Select Directory");
    connect(selectDirectoryButton,
            &QPushButton::clicked,
            this,
            [=]() mutable {
                    auto newDir = QFileDialog::getExistingDirectory(this,"Select directory containing images",
                                                                       dir); // ,QFileDialog::DontUseNativeDialog
                    if (newDir == "")
                        return;
                    directoryInput->setText(newDir);
                    updateModel(newDir);
                 }
            );
    hBoxDirectoryInput->addWidget(directoryInput);
    hBoxDirectoryInput->addWidget(selectDirectoryButton);
    settingsLayout->addLayout(hBoxDirectoryInput);

    // Select file extensions to look for, separated by a space
    // Example: *.tif *.jpg
    extensionInput = new QLineEdit(defaultFilters);
    settingsLayout->addWidget(extensionInput);
    connect(extensionInput,
            &QLineEdit::textEdited,
            this,
            [=]() mutable {
                    imageLibrary->setExtensionFilter(extensionInput->text());
                    updateModel(directoryInput->text());
                 }
            );

    // Create the overall screen layout as a grid
    gridLayout = new QGridLayout();
    ui->centralWidget->setLayout(gridLayout);
    gridLayout->addWidget(fileListView,0,0);
    gridLayout->addWidget(imageWidget,0,1);
    gridLayout->addWidget(settingsWidget,1,0,1,2);
    gridLayout->setColumnMinimumWidth(0,150);
    gridLayout->setColumnStretch(1,100);

    // Watch for file changes in the image directory
    imageLibrary->watchDirectory(directoryInput->text());
    connect(imageLibrary->fileSystemWatcher,
            &QFileSystemWatcher::directoryChanged,
            this,
            [=]() mutable {
                    updateModel(directoryInput->text());
                }
            );

    // View the first image in fileListModel, if any
    if (!list.isEmpty()) {
        currentFilePath = directoryInput->text() + "/" + list.first();
        updatePixmap(currentFilePath);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Update the model when the directory or files change
void MainWindow::updateModel(const QString &newDir)
{
    if (!QFileInfo::exists(newDir)) {
        return;
    }
    imageLibrary->watchDirectory(newDir);
    auto newList = imageLibrary->filenames(newDir);
    fileListModel->setStringList(newList);
    if (!newList.isEmpty()) {
        QString newImagePath = newDir + "/" + newList.first();
        QFileInfo currentFileInfo(currentFilePath);
        if (newList.contains(currentFileInfo.fileName())
                && newDir == currentFileInfo.absolutePath()) {
            return;
        } else if (newImagePath == currentFilePath) {
            return;
        } else {
            updatePixmap(newImagePath);
        }
    }
}

// Update the image in the UI
void MainWindow::updatePixmap(const QString &imagePath)
{
    if (!QFileInfo::exists(imagePath)) {
        return;
    }
    QFuture<QImage> loadingFuture = imageLibrary->loadImage(imagePath);
    loadingWatcher = new QFutureWatcher<QImage>();

    QObject::connect(loadingWatcher,
             &QFutureWatcher<QImage>::finished,
             [=]() {
                    QPixmap pixmap = QPixmap::fromImage(loadingFuture.result());
                    int x = imageLabel->width();
                    int y = imageLabel->height();
                    if (pixmap.width() > x || pixmap.height() > y) {
                        imageLabel->setPixmap(pixmap.scaled(x,y,Qt::KeepAspectRatio,Qt::SmoothTransformation));
                    } else {
                        imageLabel->setPixmap(pixmap);
                    }
             });

    loadingWatcher->setFuture(loadingFuture);
}
