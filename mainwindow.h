#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ImageLibrary.h"

#include <QMainWindow>
#include <QListView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStringListModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ImageLibrary *imageLibrary;
    QListView *fileListView;
    QStringListModel *fileListModel;
    QString currentFilePath;
    QWidget *imageWidget;
    QLabel *imageLabel;
    QVBoxLayout *vBoxImageWidget;
    QWidget *settingsWidget;
    QVBoxLayout *settingsLayout;
    QHBoxLayout *hBoxDirectoryInput;
    QWidget *saveImageWidget;
    QHBoxLayout *saveImageLayout;
    QSpacerItem *spacerItem;
    QLineEdit *extensionInput;
    QLineEdit *directoryInput;
    QPushButton *selectDirectoryButton;
    QPushButton *saveImageButton;
    QGridLayout *gridLayout;
    QFutureWatcher<QImage> *loadingWatcher;
    QFutureWatcher<bool> *savingWatcher;
    void updateModel(const QString &newDir);
    void updatePixmap(const QString &imagePath);
};

#endif // MAINWINDOW_H
