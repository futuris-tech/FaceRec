#include "decodewidget.h"


DecodeWidget::DecodeWidget(QWidget *parent) :
    QWidget(parent),
    im0(this),
    im1(this),
    im2(this),
    im3(this),
    im4(this) {}

const int ImageNum = 5;
void DecodeWidget::layout() {
    QSize Sz = size();
    QSize sz =  { (int)dec->width, (int)dec->height };
    sz.setWidth((int)(sz.width() * 1.125f * ImageNum));
    float K = (float)Sz.width() / Sz.height();
    float k = (float)sz.width() / sz.height();

    if (K > k)
        sz = { (int)(Sz.height() * k), Sz.height() };
    else
        sz = { Sz.width(), (int)(Sz.width() / k) };
    QPoint s = {
        (Sz.width() - sz.width()) / 2,
        (Sz.height() - sz.height()) / 2
    };
    sz.setWidth((int)(sz.width() / 1.125f / ImageNum));
    int space = sz.width() / 8;

    im0.setPixmap(dec->get_image0(sz));
    im0.move(s += QPoint(space, 0));
    im0.resize(sz);

    im1.setPixmap(dec->get_image1(sz));
    im1.move(s += QPoint(space + sz.width(), 0));
    im1.resize(sz);

    im2.setPixmap(dec->get_image2(sz));
    im2.move(s += QPoint(space + sz.width(), 0));
    im2.resize(sz);

    im3.setPixmap(dec->get_image3(sz));
    im3.move(s += QPoint(space + sz.width(), 0));
    im3.resize(sz);

    im4.setPixmap(dec->get_image4(sz));
    im4.move(s += QPoint(space + sz.width(), 0));
    im4.resize(sz);
}
void DecodeWidget::set(decoded* dec) {
    this->dec = dec;
    layout();
}

void DecodeWidget::resizeEvent(QResizeEvent *event) {
    layout();
}
