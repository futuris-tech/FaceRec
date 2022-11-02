#ifndef DECODEWIDGET_H
#define DECODEWIDGET_H

#include <QLabel>
#include <QWidget>
#include "decoded.h"

class DecodeWidget : public QWidget {
    Q_OBJECT
    decoded* dec;
    QLabel im0;
    QLabel im1;
    QLabel im2;
    QLabel im3;
    QLabel im4;

    void layout();
    void resizeEvent(QResizeEvent *event) override;
public:
    explicit DecodeWidget(QWidget *parent);
    void set(decoded* dec);
signals:

};

#endif // DECODEWIDGET_H
