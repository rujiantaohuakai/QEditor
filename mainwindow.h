#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QProgressBar>
#include <QSpinBox>
#include <QFontComboBox>
#include <QString>
#include <QCloseEvent>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actFontBold_triggered(bool checked);

    void on_actFontUnder_triggered(bool checked);

    void on_actFontItalic_triggered(bool checked);

    void on_txtEdit_copyAvailable(bool b);

    void on_txtEdit_selectionChanged();

    void on_spinBoxFontSize_valueChanged(int aFontSize);

    // void on_comboFont_currentIndexChanged(int index);

    void on_comboFont_currentFontChanged(const QFont &font);

    void on_actOpen_triggered();

    void on_actSave_triggered();

    void on_actSaveAs_triggered();

    void on_actToolbarLabel_triggered(bool checked);

    void on_actClear_triggered();

    void on_actNew_triggered();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::MainWindow *ui;
    QLabel *fLabCurFile;            // 状态栏里显示当前文件
    QProgressBar *progressBar1;     // 状态栏上的进度条
    QSpinBox *spinFontSize;         // 字体大小spinBox
    QFontComboBox *comboFont;       // 字体名称comboBox
    void iniUI();                   // 代码实现的UI的初始化
    void iniSignalSlots();          // 关联信号与槽

    QString currentFilePath;
    bool maybeSave();
    bool saveFile();
    bool saveFileAs();
    bool saveFileTo(const QString &path);
    void updateFileLabel();
};
#endif // MAINWINDOW_H
