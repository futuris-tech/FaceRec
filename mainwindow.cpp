#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDir>
#include <QResizeEvent>
#include <QTimer>
#include <QDebug>


decoded dec;

Task::Task(MainWindow *wnd) : wnd(wnd) {
    moveToThread(&thr);
    connect(&thr, &QThread::started, this, &Task::do_work);
    connect(this, &Task::update, wnd, &MainWindow::update_image);
    thr.start();
    //connect(&thr, &QThread::finished, wnd, &MainWindow::begin_processing);
}
void Task::do_work() {
    auto e_ptr = wnd->ethalons;
    auto& dirs = wnd->dirs;
    for (int i = 0; i < dirs.size();) {
        QDir dir(dirs[i]);
        bool exist = true;
        auto e_ptr2 = e_ptr;
        for (auto name : wnd->ethalon_names) {
            if (!dir.exists(name)) {
                exist = false;
                break;
            }
            e_ptr2->set(dirs[i] + '/' + name);
            e_ptr2++;
        }
        if (exist) {
            e_ptr = e_ptr2;
            i++;
        }
        else dirs.removeAt(i);
    }

    struct result_s {
        float diff;
        uint id;
    } result[6];

    float cc[] = { 0,0,0,0,0 };
    float cc_num = 0;

    auto recognize = [&, this](QString name) {
        dec.set(name);
        for (auto& r : result)
            r.diff = INFINITY;

        auto e_ptr = wnd->ethalons;
        for (uint i = 0; i < dirs.size(); i++) {
            for (uint j = 0; j < wnd->ethalon_names.size; j++) {
                float c[6] = {
                    e_ptr->compare0(dec) / 1.06f,
                    e_ptr->compare1(dec) / 1.04f,
                    e_ptr->compare2(dec) / 0.79f,
                    e_ptr->compare3(dec) / 1.09f,
                    e_ptr->compare4(dec) / 0.35f
                };
                for (int k = 0; k < 5; k++)
                    cc[k] += c[k];
                cc_num++;
                c[5] = c[0] + c[1] + c[2] + c[3] + c[4];
                uint id = e_ptr - wnd->ethalons;
                e_ptr++;
                for (int k = 0; k < 6; k++)
                    if (result[k].diff > c[k])
                        result[k] = { c[k], id };
            }
        }
    };

    for (int i = 0; i < dirs.size(); i++) {
        uint begin = i * wnd->ethalon_names.size;
        uint end = (i+1) * wnd->ethalon_names.size;

        auto files = QDir(dirs[i]).entryList(QDir::Filter::Files);
        auto success_begin = wnd->success_num[5];
        for (auto& file : files) {
            bool is_ethalon = false;
            for (auto name : wnd->ethalon_names)
                if (file == name) {
                    is_ethalon = true;
                    break;
                }
            if (is_ethalon) continue;

            recognize(dirs[i] + '/' + file);

            for (int j = 0; j < 6; j++)
                if (result[j].id >= begin && result[j].id < end)
                    wnd->success_num[j]++;

            wnd->total_num++;
            emit update(result[5].id);
            //QThread::msleep(500);
        }
        qDebug() << dirs[i] << wnd->success_num[5] - success_begin;
    }

    qDebug() << cc[0] / cc_num;
    qDebug() << cc[1] / cc_num;
    qDebug() << cc[2] / cc_num;
    qDebug() << cc[3] / cc_num;
    qDebug() << cc[4] / cc_num;

    thr.exit();
}


MainWindow::MainWindow(str_array ethalon_names, bool auto_close, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    ethalon_names(ethalon_names),
    dirs(QDir().entryList(QDir::Filter::Dirs | QDir::NoDotAndDotDot)),
    ethalons(new decoded[dirs.size() * ethalon_names.size]),
    task(this),
    total_num(0),
    image0(this),
    image1(this),
    output(this)
{
    if (auto_close)
        connect(&task.thr, &QThread::finished, 0, &QApplication::quit);

    memset(success_num, 0, sizeof(success_num));
    ui->setupUi(this);
    dec.set(new uchar[1], 1, 1);
    image0.set(&dec);
    image1.set(&dec);
}
MainWindow::~MainWindow() {
    delete ui;
    delete[] ethalons;
}


void MainWindow::resizeEvent(QResizeEvent *event) {
    int w = event->size().width();
    int h = event->size().height();
    int space = h / 32;
    int y = space;
    h = (h - space * 2 - 150) / 2;

    image0.move(0, y);
    image0.resize(w, h);

    y += space + h;
    image1.move(0, y);
    image1.resize(w, h);

    y += space + h;
    output.move(20, y);
    output.resize(w, event->size().height() - y);
}
void MainWindow::update_image(int ethalon_id) {
    image0.set(&dec);
    image1.set(ethalons + ethalon_id);
    float k = 100.0f / total_num;
    output.setText(QString(
        "Scaling:\t\t%1%\n"
        "Random:\t%2%\n"
        "Histogram:\t%3%\n"
        "Gradient:\t%4%\n"
        "DFT:\t\t%5%\n"
        "Total:\t\t%6%")
        .arg(success_num[0] * k)
        .arg(success_num[1] * k)
        .arg(success_num[2] * k)
        .arg(success_num[3] * k)
        .arg(success_num[4] * k)
        .arg(success_num[5] * k));
}
