#ifndef DECODED_H
#define DECODED_H

#include <QPixmap>
#include <QString>

inline const int ScaleW = 92 / 4;                  //scaling
inline const int ScaleH = 112 / 4;                 //scaling
inline const int ScaleS = ScaleW * ScaleH;         //scaling
inline const int PointNum = 200;                   //random
inline const int GroupSize = 8;                    //histogram
inline const int GroupNum = 255 / GroupSize + 1;   //histogram
inline const int LineWidth = 8;                    //gradient
inline const int LineNum = 20;                     //gradient
inline const int DFTSize = 8;                      //dft

struct point {
    float x, y;
    point() {}
    point(float x, float y) : x(x), y(y) {}
    explicit point(float xy) : x(xy), y(xy) {}
    inline point operator+(point& b) {
        return { x + b.x, y + b.y };
    }
    inline point operator-(point& b) {
        return { x - b.x, y - b.y };
    }
    inline point operator*(float b) {
        return { x * b, y * b };
    }
    inline point operator/(float b) {
        return { x / b, y / b };
    }
};
struct random_points {
    point data[PointNum];
    random_points();
};

template <int Size>
struct attr_vector {
    float data[Size];
    float compare(attr_vector<Size>& b) const;
};

struct decoded {
    uchar* data = 0;
    uint width;
    uint height;
    static random_points points;

    attr_vector<ScaleS> scaled;
    attr_vector<PointNum> random;
    attr_vector<GroupNum> histogram;
    attr_vector<LineNum> gradient;
    attr_vector<DFTSize * (DFTSize + 1) / 2> dft;

    void set(uchar* input, uint w, uint h);
    void set(QString file);
    ~decoded();
private:
    void create_scaled();
    void create_random();
    void create_histogram();
    void create_gradient();
    void create_dft();
    QImage get_image() const;
public:
    float compare0(decoded& b) const;
    float compare1(decoded& b) const;
    float compare2(decoded& b) const;
    float compare3(decoded& b) const;
    float compare4(decoded& b) const;

    QPixmap get_image0(QSize sz) const;
    QPixmap get_image1(QSize sz) const;
    QPixmap get_image2(QSize sz) const;
    QPixmap get_image3(QSize sz) const;
    QPixmap get_image4(QSize sz) const;
};

#endif // DECODED_H
