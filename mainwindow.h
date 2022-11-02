#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "decodewidget.h"
#include "qtimer.h"

#include <QLabel>
#include <QMainWindow>
#include <QThread>


/*inline std::array ethalon_names = {
    "1.pgm",
    "2.pgm"
};*/
struct str_array {
    const char* const* str;
    int size;
    const char* const* begin() const { return str; }
    const char* const* end()   const { return str + size; }
};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow;
class Task : public QObject {
Q_OBJECT
    MainWindow* const wnd;
public:
    QThread thr;
    Task(MainWindow* wnd);
public slots:
    void do_work();
signals:
    void update(int ethalon_id);
};

class MainWindow : public QMainWindow {
    friend class Task;
    Q_OBJECT

    str_array ethalon_names;
    QStringList dirs;
    decoded* ethalons;
    Task task;
    float success_num[6];
    float total_num;

    DecodeWidget image0;
    DecodeWidget image1;
    QLabel output;
    Ui::MainWindow *ui;

    void resizeEvent(QResizeEvent *event) override;
public:
    MainWindow(str_array ethalon_names, bool auto_close = false, QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void update_image(int ethalon_id);
};

#endif // MAINWINDOW_H
