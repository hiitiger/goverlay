#pragma once

namespace Storm
{

template <class T = int>
struct Point
{
    typedef T ValueType;

    Point()
    {
        point_.fill(T());
    }

    Point(const Point &other)
        : point_{other.x(), other.y()}
    {
        ;
    }

    Point(Point &&other)
        : point_(std::move(other.point_))
    {
    }

    Point(ValueType xx, ValueType yy)
        : point_{xx, yy}
    {
        ;
    }

    Point &operator=(const Point &other)
    {
        copyArray(point_, other.point_);
        return *this;
    }

    Point &operator=(Point &&other)
    {
        point_ = std::move(other.point_);
        return *this;
    }

    void setX(ValueType xx)
    {
        point_[0] = xx;
    }

    ValueType x() const
    {
        return point_[0];
    }

    void setY(ValueType yy)
    {
        point_[1] = yy;
    }

    ValueType y() const
    {
        return point_[1];
    }

    bool operator==(const Point &other) const
    {
        return compareValue(point_[0], other.point_[0]) && compareValue(point_[1], other.point_[1]);
    }

    bool operator!=(const Point &other) const
    {
        return !compareValue(point_[0], other.point_[0]) || !compareValue(point_[1], other.point_[1]);
    }

    Point operator-() const
    {
        return Point(-point_[0], -point_[1]);
    }

    Point operator+(const Point &other) const
    {
        return Point(point_[0] + other.x(), point_[1] + other.y());
    }

    Point operator-(const Point &other) const
    {
        return Point(point_[0] - other.x(), point_[1] - other.y());
    }

    Point &operator+=(const Point &pt)
    {
        point_[0] += pt.point_[0];
        point_[1] += pt.point_[1];
        return *this;
    }

    Point &operator-=(const Point &pt)
    {
        point_[0] -= pt.point_[0];
        point_[1] -= pt.point_[1];
        return *this;
    }

    Point operator*(const double factor) const
    {
        return Point((ValueType)(point_[0] * factor), (ValueType)(point_[1] * factor));
    }

    Point operator/(const double divisor) const
    {
        return Point((ValueType)(point_[0] / divisor), (ValueType)(point_[1] / divisor));
    }

    Point &operator*=(const double factor)
    {
        point_[0] = (ValueType)(point_[0] * factor);
        point_[1] = (ValueType)(point_[1] * factor);
        return *this;
    }

    Point &operator/=(const double divisor)
    {
        point_[0] = (ValueType)(point_[0] / divisor);
        point_[1] = (ValueType)(point_[1] / divisor);
        return *this;
    }

  private:
    std::array<ValueType, 2> point_;
};

typedef Point<int> PointI;
typedef Point<float> PointF;

template <class T>
std::ostream &operator<<(std::ostream &os, Point<T> pt)
{
    os << "{"
       << "x:" << pt.x() << ",y:" << pt.y() << "}" << std::endl;
    return os;
}

template <class T>
POINT toWinPoint(const Point<T> &pt)
{
    POINT point = {(int)pt.x(), (int)pt.y()};
    return point;
}

template <class T = int>
struct Size
{
    typedef T ValueType;

    Size()
    {
        size_.fill(T());
    }

    Size(const Size &other)
        : size_{other.width(), other.height()}
    {
        ;
    }

    Size(Size &&other)
        : size_(std::move(other.size_))
    {
    }

    Size(ValueType w, ValueType h)
        : size_{w, h}
    {
        ;
    }

    Size &operator=(const Size &other)
    {
        copyArray(size_, other.size_);
        return *this;
    }

    Size &operator=(Size &&other)
    {
        size_ = std::move(other.size_);
        return *this;
    }

    void setWidth(ValueType w) { size_[0] = w; }

    ValueType width() const { return size_[0]; }

    void setHeight(ValueType h) { size_[1] = h; }

    ValueType height() const { return size_[1]; }

    Size resize(ValueType w, ValueType h)
    {
        size_[0] = w;
        size_[1] = h;
    }

    bool isZero() const
    {
        return Storm::isZero(size_[0]) && Storm::isZero(size_[1]);
    }

    bool isValid() const
    {
        return size_[0] >= ValueType() && size_[1] >= ValueType();
    }

    bool operator==(const Size &other) const
    {
        return compareValue(size_[0], other.width()) && compareValue(size_[1], other.height());
    }
    bool operator!=(const Size &other) const
    {
        return !compareValue(size_[0], other.width()) || !compareValue(size_[1], other.height());
    }

    bool operator<(const Size &other) const
    {
        return size_[0] < other.width() || size_[1] < other.height();
    }

    bool operator>(const Size &other) const
    {
        return size_[0] > other.width() && size_[1] > other.height();
    }

  private:
    std::array<ValueType, 2> size_;
};

typedef Size<int> SizeI;
typedef Size<float> SizeF;

template <class T>
std::ostream &operator<<(std::ostream &os, Size<T> sz)
{
    os << "{"
       << "width:" << sz.w() << ",height:" << sz.height() << "}" << std::endl;
    return os;
}

template <class T>
SIZE toWinSize(const Size<T> &sz)
{
    SIZE size = {(int)sz.width(), (int)sz.height()};
    return size;
}

template <class T = int>
struct Margin
{
    typedef T ValueType;

    Margin()
    {
        margin_.assign(T());
    }

    Margin(const Margin &other)
    {
        copyArray(margin_, other.margin_);
    }

    Margin(Margin &&other)
        : margin_(std::move(other.margin_))
    {
    }

    Margin(ValueType left, ValueType top, ValueType right, ValueType bottom)
        : margin_{left, top, right, bottom}
    {
    }

    Margin &operator=(const Margin &other)
    {
        copyArray(margin_, other.margin_);
        return *this;
    }

    Margin &operator=(Margin &&other)
    {
        margin_ = std::move(other.margin_);
        return *this;
    }

    ValueType left() const
    {
        return margin_[0];
    }

    ValueType top() const
    {
        return margin_[1];
    }

    ValueType right() const
    {
        return margin_[2];
    }

    ValueType bottom() const
    {
        return margin_[3];
    }

  private:
    std::array<ValueType, 4> margin_;
};

typedef Margin<int> MarginI;
typedef Margin<float> MarginF;

template <class T = int>
struct Rect
{
    typedef T ValueType;

    Rect()
    {
        rect_.fill(T());
    }

    Rect(const Rect &other)
    {
        copyArray(rect_, other.rect_);
    }

    Rect(Rect &&other)
        : rect_(std::move(other.rect_))
    {
    }

    Rect(const Point<ValueType> &pt, const Size<ValueType> sz)
        : rect_{pt.x(), pt.y(), pt.x() + sz.width(), pt.y() + sz.height()}
    {
        ;
    }

    Rect(const Point<ValueType> &ptTopLeft, const Point<ValueType> &ptBottomRight)
        : rect_{ptTopLeft.x(), ptTopLeft.y(), ptBottomRight.x(), ptBottomRight.y()}
    {
        ;
    }

    Rect(ValueType left, ValueType top, ValueType right, ValueType bottom)
        : rect_{left, top, right, bottom}
    {
        ;
    }

    Rect &operator=(const Rect &other)
    {
        copyArray(rect_, other.rect_);
        return *this;
    }

    Rect &operator=(Rect &&other)
    {
        rect_ = std::move(other.rect_);
        return *this;
    }

    bool isValid() const
    {
        return rect_[0] < rect_[2] && rect_[1] < rect_[3];
    }

    Point<ValueType> topLeft() const
    {
        return Point<ValueType>(rect_[0], rect_[1]);
    }

    Point<ValueType> bottomRight() const
    {
        return Point<ValueType>(rect_[2], rect_[3]);
    }

    ValueType left() const
    {
        return rect_[0];
    }

    ValueType right() const
    {
        return rect_[2];
    }

    ValueType top() const
    {
        return rect_[1];
    }

    ValueType bottom() const
    {
        return rect_[3];
    }

    ValueType width() const
    {
        return rect_[2] - rect_[0];
    }

    ValueType height() const
    {
        return rect_[3] - rect_[1];
    }

    Size<ValueType> size() const
    {
        return Size<ValueType>(rect_[2] - rect_[0], rect_[3] - rect_[1]);
    }

    Rect &resize(const Size<ValueType> &sz)
    {
        setWidth(sz.width());
        setHeight(sz.height());
        return *this;
    }

    Rect &resize(ValueType w, ValueType h)
    {
        setWidth(w);
        setHeight(h);
        return *this;
    }

    Rect &enlarge(ValueType w, ValueType h)
    {
        rect_[2] += w;
        rect_[3] += h;
        return *this;
    }

    Rect &setWidth(ValueType w)
    {
        rect_[2] = rect_[0] + w;
        return *this;
    }

    Rect &enlargeWidth(ValueType w)
    {
        rect_[2] += w;
        return *this;
    }

    Rect &setHeight(ValueType h)
    {
        rect_[3] = rect_[1] + h;
        return *this;
    }

    Rect &enlargeHeight(ValueType h)
    {
        rect_[3] += h;
        return *this;
    }

    Rect &setLeft(ValueType left)
    {
        rect_[0] = left;
        return *this;
    }

    Rect &setTop(ValueType top)
    {
        rect_[1] = top;
        return *this;
    }

    Rect &setRight(ValueType right)
    {
        rect_[2] = right;
        return *this;
    }

    Rect &setBottom(ValueType bottom)
    {
        rect_[3] = bottom;
        return *this;
    }

    Rect &moveLeft(ValueType left)
    {
        ValueType w = width();
        rect_[0] = left;
        rect_[2] = left + w;
    }

    Rect moveTop(ValueType top)
    {
        ValueType h = height();
        rect_[1] = top;
        rect_[3] = top + h;
    }

    Rect &moveTo(ValueType x, ValueType y)
    {
        ValueType w = width();
        ValueType h = height();
        rect_[0] = x;
        rect_[1] = y;
        rect_[2] = rect_[0] + w;
        rect_[3] = rect_[1] + h;
        return *this;
    }

    Rect &moveTo(const Point<ValueType> &pt)
    {
        return moveTo(pt.x(), pt.y());
    }

    Rect movedTo(ValueType x, ValueType y) const
    {
        Rect rc = *this;

        return rc.moveTo(x, y);
    }

    Rect movedTo(const Point<ValueType> &pt) const
    {
        Rect rc = *this;
        return rc.moveTo(pt);
    }

    Rect &adjust(ValueType dx)
    {
        rect_[0] += dx;
        rect_[1] += dx;
        rect_[2] -= dx;
        rect_[3] -= dx;
        return *this;
    }

    Rect adjusted(ValueType dx) const
    {
        Rect rc = *this;
        rc.rect_[0] += dx;
        rc.rect_[1] += dx;
        rc.rect_[2] -= dx;
        rc.rect_[3] -= dx;
        return rc;
    }

    Rect &adjust(const Margin<ValueType> &margin)
    {
        rect_[0] += margin.left();
        rect_[1] += margin.top();
        rect_[2] -= margin.right();
        rect_[3] -= margin.bottom();
        return *this;
    }

    Rect adjusted(const Margin<ValueType> &margin)
    {
        Rect rc = *this;
        rc.rect_[0] += margin.left();
        rc.rect_[1] += margin.top();
        rc.rect_[2] -= margin.right();
        rc.rect_[3] -= margin.bottom();
        return rc;
    }

    bool contains(ValueType x, ValueType y) const
    {
        return x >= rect_[0] && y >= rect_[1] && x <= rect_[2] && y <= rect_[3];
    }

    bool contains(const Point<ValueType> &pt) const
    {
        return contains(pt.x(), pt.y());
    }

    bool isIntersectWith(const Rect &rect) const
    {
        Rect rc = intersectedWith(rect);
        return rc.isValid();
    }

    Rect &intersectWith(const Rect &rect)
    {
        rect_[0] = std::max(rect_[0], rect.rect_[0]);
        rect_[2] = std::min(rect_[2], rect.rect_[2]);
        rect_[1] = std::max(rect_[1], rect.rect_[1]);
        rect_[3] = std::min(rect_[3], rect.rect_[3]);
        return *this;
    }

    Rect intersectedWith(const Rect &rect) const
    {
        ValueType ix1 = std::max(rect_[0], rect.rect_[0]);
        ValueType ix2 = std::min(rect_[2], rect.rect_[2]);
        ValueType iy1 = std::max(rect_[1], rect.rect_[1]);
        ValueType iy2 = std::min(rect_[3], rect.rect_[3]);

        return Rect(ix1, iy1, ix2, iy2);
    }

    bool operator==(const Rect &other) const
    {
        return compareValue(rect_[0], other.rect_[0]) && compareValue(rect_[1], other.rect_[1]) && compareValue(rect_[2], other.rect_[2]) && compareValue(rect_[3], other.rect_[3]);
    }

    bool operator!=(const Rect &other) const
    {
        return !compareValue(rect_[0], other.rect_[0]) || !compareValue(rect_[1], other.rect_[1]) || !compareValue(rect_[2], other.rect_[2]) || !compareValue(rect_[3], other.rect_[3]);
    }

  private:
    //x1, y1, x2, y2
    std::array<ValueType, 4> rect_;
};

typedef Rect<int> RectI;
typedef Rect<float> RectF;

template <class T>
std::ostream &operator<<(std::ostream &os, Rect<T> rect)
{
    os << "{"
       << "left:" << rect.left() << ",top:" << rect.top() << ",width:" << rect.width() << ",height:" << rect.height() << "}" << std::endl;
    return os;
}

template <class T>
RECT toWinRect(const Rect<T> &rc)
{
    RECT rect = {(int)rc.left(), (int)rc.top(), (int)rc.right(), (int)rc.bottom()};
    return rect;
}

} // namespace Storm
