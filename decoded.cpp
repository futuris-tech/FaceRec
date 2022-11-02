#include "decoded.h"
#include <QDebug>
#include <QPainter>
#include <random>
#include <chrono>


/*scaled_c::scaled_c(QImage input) :
    QImage(input.size() / scaleK, QImage::Format::Format_Grayscale8)
{
    QPainter(this).drawImage(rect(), input);
    auto copy_line = [](uchar*& dst, uchar* src, int width) {
        const auto dst_end = dst + width;
        const auto src_end = src + width;
        auto src_begin = src;
       do {
            *dst = *src;
            src += K;
            if (src >= src_end) src = ++src_begin;
        } while (++dst != dst_end);
    };
    auto copy_image = [&copy_line](uchar* dst, uchar* src, int width, int height) {
        const int lineK = width * K;
        const int sqr = width * height;
        const auto dst_end = dst + sqr;
        const auto src_end = src + sqr;
        auto src_begin = src;
        do {
            copy_line(dst, src, width);
            src += lineK;
            if (src >= src_end) src = (src_begin += width);
        } while (dst != dst_end);
    };

    copy_image(bits(), input.bits(), width(), height());
}*/

random_points::random_points() {
    std::default_random_engine dre(
        std::chrono::system_clock::now().time_since_epoch().count());

    float k1 = 1.0f / (float)dre.max();
    for (auto& p : data) {
        p.x = dre() * k1;
        p.y = dre() * k1;
    }
    /*float k1 = 1.0f / (float)dre.max();
    float k0_2 = 0.2f * k1;
    point line[2];
    auto ptr = data;
    auto fill_data = [&]() {
        auto d = line[1] - line[0];
        for (int i = PointInLine; i; i--) {
            auto r = dre();
            auto r1 = r * k1;
            auto s = d * r1;
            *ptr++ = line[0] + s;
        }
    };

    int i = LineNum;
    while (true) {
        for (auto& p : line) {
            auto r = dre();
            p.x = r * k0_2;
            p.y = 0;
            if (r % 2) std::swap(p.x, p.y);
        }
        line[1] = (point)1 - line[1];
        fill_data();
        if (!--i) break;

        for (auto& p : line) {
            auto r = dre();
            p.x = r * k0_2;
            p.y = 0;
            if (r % 2) std::swap(p.x, p.y);
        }
        line[0].x = 1 - line[0].x;
        line[1].y = 1 - line[1].y;
        fill_data();
        if (!--i) break;

        for (auto& p : line)
            p.x = 0.45f + dre() * k0_2;
        line[0].y = 0;
        line[1].y = 1;
        fill_data();
        if (!--i) break;

        for (auto& p : line)
            p.y = 0.45f + dre() * k0_2;
        line[0].x = 0;
        line[1].x = 1;
        fill_data();
        if (!--i) break;
    }*/
}
random_points decoded::points;

void decoded::create_scaled() {
    auto src = (uchar*)scaled.data + sizeof(scaled.data) - ScaleS;
    QImage im(src, ScaleW, ScaleH, ScaleW, QImage::Format::Format_Grayscale8);
    QPainter p(&im);
    p.drawImage(im.rect(), get_image());

    float length = 0;
    for (float& f : scaled.data) {
        f = *src++ - 127.5f;
        length += f * f;
    }
    length = 1 / sqrt(length);
    for (float& f : scaled.data)
        f *= length;
}
void decoded::create_random() {
    auto dst = random.data;
    float w = width;
    float h = height;
    int max_w = width - 1;
    int max_h = height - 1;
    float length = 0;
    for (auto point : points.data) {
        int x = std::min<int>(point.x * w, max_w);
        int y = std::min<int>(point.y * h, max_h);
        float value = data[x + y * width] - 127.5f;
        *dst++ = value;
        length += value * value;
    }
    length = 1 / sqrt(length);
    for (float& f : random.data)
        f *= length;
}
void decoded::create_histogram() {
    memset(histogram.data, 0, sizeof(histogram.data));
    uint pixel_num = width * height;
    for (int i = 0; i < pixel_num; i++)
        histogram.data[data[i] / GroupSize]++;

    float s = 0.5f / GroupNum * pixel_num;
    float length = 0;
    for (float& f : histogram.data) {
        f -= s;
        length += f * f;
    }
    length = 1 / sqrt(length);
    for (float& f : histogram.data)
        f *= length;
}
void decoded::create_gradient() {
    auto src = data;
    auto get_line = [&](){
        int bright = 0;
        for (int i = width; i; i--)
            bright += *src++;
        return bright;
    };

    int brights[LineWidth * 2];
    for (int& b : brights)
        b = get_line();

    int line0 = 0, line1 = 0, y = 0;
    auto b0ptr = brights, b1ptr = brights + LineWidth;
    for (int i = LineWidth; i; i--)
        line0 += *b0ptr++;
    for (int i = LineWidth; i; i--)
        line1 += *b0ptr++;

    auto move = [&](){
        y++;
        if (b0ptr == brights + LineWidth * 2)
            b0ptr = brights;
        else if (b1ptr == brights + LineWidth * 2)
            b1ptr = brights;

        line0 -= *b0ptr;
        line0 += *b1ptr;
        *b0ptr = get_line();
        line1 -= *b1ptr++;
        line1 += *b0ptr++;
    };

    float length = 0;
    int range = height - LineWidth * 2;
    for (int i = 0; i < LineNum; i++) {
        int need_y = i * range / (LineNum-1);
        while (y < need_y)
            move();
        float value = line1 - line0;
        gradient.data[i] = value;
        length += value * value;
    }
    length = 1 / sqrt(length);
    for (float& f : gradient.data)
        f *= length;
}
void decoded::create_dft() {
    struct complex {
        double re;
        double im;
        complex& operator+=(complex&& b) {
            re += b.re;
            im += b.im;
            return *this;
        }
        complex operator*(double k) {
            return { re * k, im * k };
        }
        double length() {
            return sqrt(re * re + im * im);
        }
    };

    const double w = width;
    const double h = height;
    const double w_k = -2 * M_PI / width;
    const double h_k = -2 * M_PI / height;
    auto C = [this, w, h, w_k, h_k](double p, double r) {
        double k = p + r + 1;
        p *= w_k;
        r *= h_k;
        auto src = data;
        complex sum = { 0,0 };
        for (double y = 0; y < h; y++)
            for (double x = 0; x < w; x++) {
                double a = x * p + y * r;
                sum += complex { cos(a), sin(a) } * *src++;
            }
        return sum.length() * k;
    };

    float s = 0.1f * width * height;
    float length = 0;
    auto dst = dft.data;
    for (double r = 0; r < DFTSize; r++)
        for (double p = 0; p < DFTSize-r; p++) {
            double value = C(p, r) - s;
            *dst++ = value;
            length += value * value;
        }

    length = 1 / sqrt(length);
    for (float& f : dft.data)
        f *= length;
}

void decoded::set(uchar* input, uint w, uint h) {
    delete[] data;
    data = input;
    width = w;
    height = h;

    create_scaled();
    create_random();
    create_histogram();
    create_gradient();
    create_dft();
}
void decoded::set(QString file) {
    QImage raw(file);
    uchar* data = new uchar[raw.width() * raw.height()];
    QImage im(data, raw.width(), raw.height(), raw.width(), QImage::Format::Format_Grayscale8);
    QPainter p(&im);
    p.drawImage(QRect(0,0,raw.width(),raw.height()), raw);
    set(data, raw.width(), raw.height());
}

decoded::~decoded() {
    delete[] data;
}

float decoded::compare0(decoded &b) const {
    return scaled.compare(b.scaled);
}
float decoded::compare1(decoded &b) const {
    return random.compare(b.random);
}
float decoded::compare2(decoded &b) const {
    return histogram.compare(b.histogram);
}
float decoded::compare3(decoded &b) const {
    return gradient.compare(b.gradient);
}
float decoded::compare4(decoded &b) const {
    return dft.compare(b.dft);
    //return (dft0.compare(b.dft0) + dft1.compare(b.dft1)) * 0.5f;
}

QImage decoded::get_image() const {
    return QImage(data, width, height, width, QImage::Format::Format_Grayscale8);
}
QPixmap decoded::get_image0(QSize sz) const {
    /*return QPixmap::fromImage(
        QImage(scaled.data, ScaleW, ScaleH, ScaleW, QImage::Format::Format_Grayscale8))
            .scaled(sz);*/
    auto small = get_image().scaled(ScaleW, ScaleH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    auto big = QPixmap::fromImage(small).scaled(sz);
    return big;
}
QPixmap decoded::get_image1(QSize sz) const {
    const int r = 3;
    const int d = r * 2;
    QImage circle(d, d, QImage::Format_ARGB32);
    auto ptr = (int*)circle.bits();
    for (float y = 0; y < d; y++)
        for (float x = 0; x < d; x++) {
            float dx = x - r + 0.5f;
            float dy = y - r + 0.5f;
            float s = sqrt(dx*dx + dy*dy);
            float a = r - s;
            if (a < 0) a = 0;
            else if (a > 1) a = 1;
            int ai = a * 255;
            *ptr++ = (ai << 24) | 0xff00;
        }

    QPixmap ret = QPixmap::fromImage(get_image())
        .scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    QPainter p(&ret);
    float w = sz.width();
    float h = sz.height();
    int max_w = sz.width() - 1;
    int max_h = sz.height() - 1;
    for (auto point : points.data) {
        int x = std::min<int>(point.x * w, max_w);
        int y = std::min<int>(point.y * h, max_h);
        p.drawImage(x-r, y-r, circle);
    }
    return ret;
}
QPixmap decoded::get_image2(QSize sz) const {
    float minH = 0;
    float maxH = 0;
    for (float f : histogram.data) {
        minH = std::min(minH, f);
        maxH = std::max(maxH, f);
    }
    float s = minH;
    float k = 1 / (maxH - minH);

    QPixmap ret(sz);
    ret.fill();
    QPainter p(&ret);
    p.setBrush(QBrush(QColor(0,0,255), Qt::SolidPattern));
    for (int i = 0; i < GroupNum; i++) {
        int x0 = i * sz.width() / GroupNum;
        int x1 = (i+1) * sz.width() / GroupNum;
        int y0 = sz.height() * (1 - (histogram.data[i] - s) * k);
        int y1 = sz.height();
        p.drawRect(x0, y0, x1-x0, y1-y0);
    }
    return ret;
}
QPixmap decoded::get_image3(QSize sz) const {
    float minH = 0;
    float maxH = 0;
    for (float f : gradient.data) {
        minH = std::min(minH, f);
        maxH = std::max(maxH, f);
    }
    float s = minH;
    float k = 1 / (maxH - minH);
    float x_k = (float)sz.width() / (LineNum - 1);

    QPixmap ret(sz);
    ret.fill();
    QPainter p(&ret);
    QPointF p0, p1 = {
        0,
        (1 - (gradient.data[0] - s) * k) * sz.height()
    };
    for (int i = 1; i < LineNum; i++) {
        p0 = p1;
        p1 = {
            i * x_k,
            (1 - (gradient.data[i] - s) * k) * sz.height()
        };
        p.drawLine(p0, p1);
    }
    return ret;
}
QPixmap decoded::get_image4(QSize sz) const {
    /*float minH = 0;
    float maxH = 0;
    for (float f : dft0.data) {
        minH = std::min(minH, f);
        maxH = std::max(maxH, f);
    }
    float s = minH;
    float k = 255.0f / (maxH - minH);

    QImage im(DFTSize, DFTSize, QImage::Format::Format_Grayscale8);
    int diff = im.bytesPerLine() - DFTSize;
    auto dst = im.bits();
    auto src = dft.data;
    for (int y = DFTSize; y; y--) {
        for (int x = DFTSize; x; x--)
            *dst++ = (*src++ - s) * k;
        src += diff;
    }*
    return QPixmap::fromImage(im).scaled(sz);*/

    float minH = 0;
    float maxH = 0;
    for (float f : dft.data) {
        minH = std::min(minH, f);
        maxH = std::max(maxH, f);
    }
    float s = minH;
    float k = 255.99999f / (maxH - minH);

    QImage im(DFTSize, DFTSize+1, QImage::Format::Format_Grayscale8);
    auto dst = im.bits();
    auto y_k = im.bytesPerLine();
    auto src = dft.data;
    for (int y = 0; y < DFTSize; y++)
        for (int x = 0; x < DFTSize-y; x++) {
            /*int value0 = (*src0++ - s0) * k0;
            int value1 = (*src1++ - s1) * k1;
            im.setPixel(x, y, value0);
            im.setPixel(DFTSize-1-x, DFTSize-y, value1);*/
            dst[x + y * y_k] = (*src++ - s) * k;
            dst[DFTSize-1-x + (DFTSize-y) * y_k] = 255;
        }
    return QPixmap::fromImage(im).scaled(sz);

    //return QPixmap(sz);
}

template<int Size>
float attr_vector<Size>::compare(attr_vector<Size> &b) const {
    /*float sum = 0;
    auto b_ptr = b.data;
    for (auto value : data)
        sum += value * (*b_ptr++);
    return acos(sum);*/
    float sum = 0;
    auto b_ptr = b.data;
    for (auto value : data) {
        float s = value - *b_ptr++;
        sum += s * s;
    }
    return sqrt(sum);
}

