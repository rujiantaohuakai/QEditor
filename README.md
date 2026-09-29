# QEditor

QEditor 是一个基于 C++17 和 Qt Widgets 的桌面文本编辑器，支持编辑纯文本和 HTML 富文本。

## 功能

- **文件操作**：新建、打开、保存和另存为 `.txt`、`.html`、`.htm` 文件；新建、打开或关闭窗口前检查未保存的修改。
- **文本编辑**：剪切、复制、粘贴和清空内容；“清空”操作可以通过撤销恢复。
- **文字格式**：设置字体族、字号、粗体、斜体和下划线；字体控件与格式按钮随光标位置更新。
- **界面状态**：窗口标题和状态栏显示当前文件名，状态栏进度条显示字号；可切换工具栏文字标签。

## 技术实现

- 使用 `QTextEdit` 和 `QTextDocument` 编辑内容、获取当前字符格式并跟踪文档修改状态。
- 使用 Qt 信号与槽连接菜单、工具栏和编辑器；通过 `QSignalBlocker` 避免同步字体控件时再次触发格式修改。
- 使用 `QFile` 读取文件，使用 `QSaveFile` 写入文件；通过 `QCloseEvent` 处理关闭窗口时的未保存内容。

## 构建与运行

项目使用 qmake，已在 **Qt 6.9.3、MinGW 64-bit、Windows** 环境下构建通过。

1. 在 Qt Creator 中打开 `QEditor.pro`。
2. 选择 Qt 6 的桌面开发套件，并确保编译器与所选 Qt 套件匹配。
3. 构建并运行项目。

也可以在已配置好 Qt 6 `qmake` 和对应编译器的终端中构建：

```powershell
mkdir build
cd build
qmake ..\QEditor.pro
mingw32-make -j
```

上面的命令适用于 MinGW 工具链。运行前请确认 `qmake -v` 显示的是 Qt 6；如果系统中装有多个 Qt 版本，应使用所选 Qt 6 套件中的 `qmake`。

## 文件格式说明

- `.txt` 以 UTF-8 纯文本读写，保存时不保留字体等富文本格式。
- `.html` 和 `.htm` 以 UTF-8 读写，使用 `QTextEdit` 的 HTML 导入和导出能力保留受支持的文字格式。
- 文件类型由扩展名决定；当前版本未提供其他字符编码的选择或自动识别。

## 项目结构

| 路径 | 作用 |
| --- | --- |
| `main.cpp` | 创建应用和主窗口 |
| `mainwindow.h`、`mainwindow.cpp` | 编辑器界面逻辑、格式设置及文件操作 |
| `mainwindow.ui` | Qt Designer 界面、菜单和工具栏 |
| `res.qrc`、`images/` | 界面图标资源 |
| `QEditor.pro` | qmake 项目配置 |
