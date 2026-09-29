#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSignalBlocker>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSaveFile>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    iniUI();
    iniSignalSlots();
    updateFileLabel();
    on_actToolbarLabel_triggered(ui->actToolbarLabel->isChecked());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::iniUI()
{
    fLabCurFile = new QLabel;
    fLabCurFile->setMinimumWidth(150);
    fLabCurFile->setText("当前文件：");
    ui->statusbar->addWidget(fLabCurFile); // 添加到状态栏

    // 进度条进度为当前字号
    progressBar1 = new QProgressBar;
    progressBar1->setMinimumWidth(200);
    progressBar1->setMinimum(5);
    progressBar1->setMaximum(50);
    progressBar1->setValue(ui->txtEdit->font().pointSize());
    ui->statusbar->addWidget(progressBar1);

    spinFontSize = new QSpinBox;
    spinFontSize->setMinimumWidth(50);
    spinFontSize->setMinimum(5);
    spinFontSize->setMaximum(50);
    spinFontSize->setValue(ui->txtEdit->font().pointSize());
    ui->mainToolBar->addWidget(new QLabel("字体大小"));
    ui->mainToolBar->addWidget((spinFontSize));

    ui->mainToolBar->addSeparator(); // 分隔条
    ui->mainToolBar->addWidget(new QLabel(" 字体 "));
    comboFont = new QFontComboBox;
    comboFont->setMinimumWidth(150);
    ui->mainToolBar->addWidget(comboFont);

    setCentralWidget(ui->txtEdit);
}

void MainWindow::iniSignalSlots()
{

    // connect(spinFontSize, SIGNAL(valueChanged(int)), this, SLOT(on_spinBoxFontSize_valueChanged(int)));
    // connect(comboFont, SIGNAL(currentFontChanged(QFont)), this, SLOT(on_comboFont_currentFontChanged(QFont)));

    // connect(spinFontSize, &QSpinBox::valueChanged, this, &MainWindow::on_spinBoxFontSize_valueChanged);
    // connect(comboFont, &QFontComboBox::currentFontChanged, this, &MainWindow::on_comboFont_currentFontChanged);

    // 如果有多个重载信号或者槽函数，可以用qOverload指定参数
    connect(spinFontSize, qOverload<int>(&QSpinBox::valueChanged), this, qOverload<int>(&MainWindow::on_spinBoxFontSize_valueChanged));
    connect(comboFont, qOverload<const QFont &>(&QFontComboBox::currentFontChanged),
            this, qOverload<const QFont &>(&MainWindow::on_comboFont_currentFontChanged));

    // 使用lambda作为槽函数
    // currentCharFormatChanged:
    // This signal is emitted if the current character format has changed, for example caused by a change of the cursor position.
    // 根据光标当前位置的字号，设置QSpinBox中的值；根据光标当前位置的字体，设置QFontComboBox中的值
    // 根据光标位置设置粗体、斜体、下划线
    connect(ui->txtEdit, &QTextEdit::currentCharFormatChanged, this, [this](const QTextCharFormat &fmt) {
        const QFont font = fmt.font();
        const int size = font.pointSize();
        if (size >= spinFontSize->minimum() &&
            size <= spinFontSize->maximum()) {
            // 同步控件显示时最好屏蔽信号
            // QSignalBlocker在它作用域中，临时屏蔽指定对象发出的信号
            // 防止触发valueChanged
            const QSignalBlocker blockSize(spinFontSize);
            spinFontSize->setValue(size);
            progressBar1->setValue(size);
        }

        if (!font.family().isEmpty()) {
            // 防止触发currentFontChanged
            const QSignalBlocker blockFont(comboFont);
            comboFont->setCurrentFont(font);
        }

        ui->actFontBold->setChecked(fmt.font().bold());
        ui->actFontItalic->setChecked(fmt.fontItalic());
        ui->actFontUnder->setChecked(fmt.fontUnderline());
    });

    // 初始化复制粘贴剪切
    on_txtEdit_copyAvailable(false);
}

void MainWindow::updateFileLabel()
{
    const QString name = currentFilePath.isEmpty()
    ? tr("未命名")
    : QFileInfo(currentFilePath).fileName();
    fLabCurFile->setText(tr("当前文件：%1").arg(name));
    setWindowTitle(tr("%1 - QEditor").arg(name));
}

bool MainWindow::maybeSave()
{
    // 在即将丢弃或替换当前文档前，判断能否继续
    // 新建、打开新文件、关闭窗口时调用

    // 无修改返回true
    if (!ui->txtEdit->document()->isModified()) return true;

    const auto answer = QMessageBox::warning(
        this,
        tr("未保存的内容"),
        tr("当前内容已修改，是否保存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
        QMessageBox::Save
    );

    if (answer == QMessageBox::Save) return saveFile();
    // else if (answer == QMessageBox::Discard) return true;
    // else return false;
    return answer == QMessageBox::Discard;

}

bool MainWindow::saveFile()
{
    if (currentFilePath.isEmpty()) return saveFileAs();

    return saveFileTo(currentFilePath);
}

bool MainWindow::saveFileTo(const QString &path)
{
    // 是否是纯文本(.txt)
    const bool plainText =
        QFileInfo(path).suffix().compare("txt", Qt::CaseInsensitive) == 0;

    const QByteArray data = (plainText ? ui->txtEdit->toPlainText()
                                       : ui->txtEdit->toHtml()).toUtf8();
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("保存失败"), file.errorString());
        return false;
    }

    if (file.write(data) != data.size() || !file.commit()) {
        QMessageBox::warning(this, tr("保存失败"), file.errorString());
        return false;
    }

    currentFilePath = path;
    ui->txtEdit->document()->setModified(false);
    updateFileLabel();
    return true;
}

bool MainWindow::saveFileAs()
{
    const QString htmlFilter = tr("HTML 文件 (*.html *.htm)");
    const QString textFilter = tr("文本文件 (*.txt)");

    // 默认类型，compare忽略大小写
    QString selectedFilter = QFileInfo(currentFilePath).suffix()
                            .compare("txt", Qt::CaseInsensitive) == 0 ? textFilter : htmlFilter;
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("另存为"),
        currentFilePath.isEmpty() ? tr("未命名.html") : currentFilePath,
        htmlFilter + ";;" + textFilter,
        &selectedFilter
    );

    if (path.isEmpty()) return false;

    // 没有输入拓展名时，按所选文件类型补上
    if (QFileInfo(path).suffix().isEmpty()) {
        path += (selectedFilter == textFilter ? ".txt" : ".html");

        if (QFileInfo::exists(path)) {
            const auto answer = QMessageBox::question(
                this,
                tr("确认覆盖"),
                tr("文件已存在，是否覆盖？"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
            );
            if (answer != QMessageBox::Yes) return false;
        }
    }
    return saveFileTo(path);
}

void MainWindow::on_actFontBold_triggered(bool checked)
{
    QTextCharFormat fmt;

    // currentCharFormat文档描述：
    // Returns the char format that is used when inserting new text.
    // 也就是返回当前光标位置格式

    // 使用currentCharFormat会有bug，如果同时选中两个不同字号的内容，点击粗体会统一成一个字号
    // fmt = ui->txtEdit->currentCharFormat();

    // 直接使用空格式然后合并即可解决
    if (checked) {
        fmt.setFontWeight(QFont::Bold);
    }
    else {
        fmt.setFontWeight(QFont::Normal);
    }
    ui->txtEdit->mergeCurrentCharFormat(fmt);
}


void MainWindow::on_actFontUnder_triggered(bool checked)
{
    QTextCharFormat fmt;
    //fmt = ui->txtEdit->currentCharFormat();
    fmt.setFontUnderline(checked);
    ui->txtEdit->mergeCurrentCharFormat(fmt);
}


void MainWindow::on_actFontItalic_triggered(bool checked)
{
    QTextCharFormat fmt;
    //fmt = ui->txtEdit->currentCharFormat();
    fmt.setFontItalic(checked);
    ui->txtEdit->mergeCurrentCharFormat(fmt);
}


void MainWindow::on_txtEdit_copyAvailable(bool b)
{
    // 更新cut,copy,paste的enabled属性
    ui->actCut->setEnabled(b);
    ui->actCopy->setEnabled(b);
    ui->actPaste->setEnabled(ui->txtEdit->canPaste());
}


void MainWindow::on_txtEdit_selectionChanged()
{
    // 更新粗体、斜体和下划线的checked属性
    QTextCharFormat fmt;
    fmt = ui->txtEdit->currentCharFormat();
    ui->actFontBold->setChecked(fmt.font().bold());
    ui->actFontItalic->setChecked(fmt.fontItalic());
    ui->actFontUnder->setChecked(fmt.fontUnderline());
}

void MainWindow::on_spinBoxFontSize_valueChanged(int aFontSize)
{
    // 更新字号
    QTextCharFormat fmt;
    fmt.setFontPointSize(aFontSize);
    ui->txtEdit->mergeCurrentCharFormat(fmt);
    progressBar1->setValue(aFontSize);
}

void MainWindow::on_comboFont_currentFontChanged(const QFont &font)
{
    // 更新字体
    QTextCharFormat fmt;
    fmt.setFontFamilies(QStringList{font.family()});
    ui->txtEdit->mergeCurrentCharFormat(fmt);
}


void MainWindow::on_actOpen_triggered()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("打开文件"),
        currentFilePath,
        tr("HTML 文件 (*.html *.htm);;文本文件 (*.txt)")
    );

    // 如果取消选择，path为空
    if (path.isEmpty() || !maybeSave()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("打开失败"), file.errorString());
        return;
    }

    const QByteArray data = file.readAll();
    if (file.error() != QFileDevice::NoError) {
        QMessageBox::warning(this, tr("读取失败"), file.errorString());
        return;
    }

    const QString content = QString::fromUtf8(data);
    // ui->txtEdit->setCurrentCharFormat(QTextCharFormat{});
    if (QFileInfo(path).suffix().compare("txt", Qt::CaseInsensitive) == 0)
        ui->txtEdit->setPlainText(content);
    else
        ui->txtEdit->setHtml(content);

    currentFilePath = path;
    ui->txtEdit->document()->setModified(false);
    updateFileLabel();
}


void MainWindow::on_actSave_triggered()
{
    saveFile();
}


void MainWindow::on_actSaveAs_triggered()
{
    saveFileAs();
}


void MainWindow::on_actToolbarLabel_triggered(bool checked)
{
    ui->mainToolBar->setToolButtonStyle(
        checked ? Qt::ToolButtonTextUnderIcon : Qt::ToolButtonIconOnly);
}

void MainWindow::on_actClear_triggered()
{
    QTextCursor cursor = ui->txtEdit->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
    ui->txtEdit->setTextCursor(cursor);
}


void MainWindow::on_actNew_triggered()
{
    if (!maybeSave()) return;

    ui->txtEdit->clear();
    ui->txtEdit->setCurrentCharFormat(QTextCharFormat{});
    currentFilePath.clear();
    ui->txtEdit->document()->setModified(false);
    updateFileLabel();

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}
