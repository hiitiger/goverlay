#pragma once
#include "overlay/session.h"
#include "message/gmessage.hpp"

struct IHookModule
{
    virtual ~IHookModule() { ; }
    virtual bool hook() = 0;
    virtual void unhook() = 0;
};

enum class Graphics
{
    D3d9 = 1,
    DXGI = 2,
    OpenGL = 16,
};

enum class Threads
{
    HookApp = 1,
    Graphics = 2,
    Window = 3,
};

inline bool checkThread(Threads type)
{
    std::uint32_t required = 0;
    switch (type)
    {
    case Threads::HookApp:
        required = session::hookAppThreadId();
        break;
    case Threads::Graphics:
        required = session::graphicsThreadId();
        break;
    case Threads::Window:
        required = session::windowThreadId();
        break;
    default:
        break;
    }

    return ::GetCurrentThreadId() == required;
}

#define CHECK_THREAD(type)         \
    do                             \
    {                              \
        assert(checkThread(type)); \
    } while (0);

namespace overlay_game
{

class ColorARGB
{

  public:
    ColorARGB() = default;
    ColorARGB(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
        : m_red(r), m_green(g), m_blue(b), m_alpha(a)

    {
    }

    explicit ColorARGB(uint32_t argb)
    {
        setColor(argb);
    }

    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        m_red = r;
        m_green = g;
        m_blue = b;
        m_alpha = a;
    }

    void setColor(uint32_t argb)
    {
        m_red = static_cast<uint8_t>((argb & sc_redMask) >> sc_redShift);
        m_green = static_cast<uint8_t>((argb & sc_greenMask) >> sc_greenShift);
        m_blue = static_cast<uint8_t>((argb & sc_blueMask) >> sc_blueShift);
        m_alpha = static_cast<uint8_t>((argb & sc_alphaMask) >> sc_alphaShitf);
    }

    void setRed(uint8_t r) { m_red = r; }
    uint8_t red() const { return m_red; }

    void setGreen(uint8_t g) { m_green = g; }
    uint8_t green() const { return m_green; }

    void setBlue(uint8_t b) { m_blue = b; }
    uint8_t blue() const { return m_blue; }

    void setAlpha(uint8_t a) { m_alpha = a; }
    uint8_t alpha() const { return m_alpha; }

    uint32_t value()
    {
        uint32_t v;
        v += static_cast<uint8_t>((alpha()) << sc_alphaShitf);
        v += static_cast<uint8_t>((red()) << sc_redShift);
        v += static_cast<uint8_t>((green()) << sc_greenShift);
        v += static_cast<uint8_t>((blue()) << sc_blueShift);
        return v;
    }

    bool operator==(const ColorARGB &color) const
    {
        return m_red == color.m_red && m_green == color.m_green && m_blue == color.m_blue && m_alpha == color.m_alpha;
    }

    bool operator!=(const ColorARGB &color) const
    {
        return m_red != color.m_red || m_green != color.m_green || m_blue != color.m_blue || m_alpha != color.m_alpha;
    }

  private:
    uint8_t m_blue = 0;
    uint8_t m_green = 0;
    uint8_t m_red = 0;
    uint8_t m_alpha = 0;

    static const uint32_t sc_alphaShitf = 24;
    static const uint32_t sc_redShift = 16;
    static const uint32_t sc_greenShift = 8;
    static const uint32_t sc_blueShift = 0;

    static const uint32_t sc_alphaMask = 0xff << sc_alphaShitf;
    static const uint32_t sc_redMask = 0xff << sc_redShift;
    static const uint32_t sc_greenMask = 0xff << sc_greenShift;
    static const uint32_t sc_blueMask = 0xff << sc_blueShift;
};

struct FrameBuffer
{
    int width = 0;
    int height = 0;
    int *data = 0;
    bool owned = true;

    FrameBuffer() = default;

    FrameBuffer(int w, int h, const int *mem, bool own = true)
        : width(w), height(h), data(nullptr), owned(own)
    {
        if (owned)
        {
            data = (int *)malloc(width * height * sizeof(std::uint32_t));
            memcpy_s(data, width * height * sizeof(int), mem, width * height * sizeof(int));
        }
        else
        {
            data = const_cast<int *>(mem);
        }
    }

    ~FrameBuffer()
    {
        if (owned)
        {
            free(data);
        }
    }

    FrameBuffer copy(Storm::Rect<int> rc) const
    {
        FrameBuffer img;
        img.width = rc.width();
        img.height = rc.height();
        img.data = (int *)malloc(width * height * sizeof(int));
        int x = rc.left();
        int y = rc.top();
        for (int i = 0; i != height; ++i)
        {
            int pY = y + i;
            const int *line = this->data + pY * width + x;
            int *dst = img.data + i + img.width;
            memcpy(dst, line, sizeof(int) * width);
        }

        return img;
    }

    ColorARGB pixel(int x, int y) const
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            int *pix = data + y * width + x;
            return ColorARGB((uint32_t)*pix);
        }
        else
        {
            return ColorARGB();
        }
    }

    void setPixel(int x, int y, ColorARGB argb)
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            int *pix = data + y * width + x;
            *pix = (int)argb.value();
        }
    }
};


inline bool pointInRect(const POINTS& pt, const  RECT& rc)
{
    return pt.x >= rc.left
        && pt.y >= rc.top
        && pt.x <= rc.right
        && pt.y <= rc.bottom;
}

inline bool pointInRect(const POINT& pt, const  RECT& rc)
{
    return pt.x >= rc.left
        && pt.y >= rc.top
        && pt.x <= rc.right
        && pt.y <= rc.bottom;
}


inline bool pointInRect(const POINTS& pt, const  overlay::WindowRect& rc)
{
    return pt.x >= rc.x
        && pt.y >= rc.y
        && pt.x <= rc.x + rc.width
        && pt.y <= rc.y + rc.height;
}

inline bool pointInRect(const POINT& pt, const  overlay::WindowRect& rc)
{
    return pt.x >= rc.x
        && pt.y >= rc.y
        && pt.x <= rc.x + rc.width
        && pt.y <= rc.y + rc.height;
}


inline int hitTest(POINT pt, const overlay::WindowRect& rect, bool resizable, const overlay::WindowCaptionMargin& caption, int borderWidth)
{
    Storm::Point<int> mousePt(pt.x, pt.y);

    Storm::Rect<int> dragBarRect(caption.left, caption.top, rect.width - caption.right, caption.top + caption.height);
    if (dragBarRect.isValid() && dragBarRect.contains(pt.x, pt.y))
    {
        return HTCAPTION;
    }

    if (!resizable)
    {
        return HTCLIENT;
    }

    Storm::Rect<int> topLeftBorder(0, 0, borderWidth, borderWidth);
    if (topLeftBorder.contains(mousePt))
    {
        return HTTOPLEFT;
    }

    Storm::Rect<int>  topBorder(borderWidth, 0, rect.width - borderWidth, borderWidth);
    if (topBorder.contains(mousePt))
    {
        return HTTOP;
    }

    Storm::Rect<int>  topRightBorder(rect.width - borderWidth, 0, rect.width, borderWidth);
    if (topRightBorder.contains(mousePt))
    {
        return HTTOPRIGHT;
    }

    Storm::Rect<int>  leftBorder(0, borderWidth, borderWidth, rect.height - borderWidth);
    if (leftBorder.contains(mousePt))
    {
        return HTLEFT;
    }

    Storm::Rect<int>  bottomLeftBorder(0, rect.height - borderWidth, borderWidth, rect.height);
    if (bottomLeftBorder.contains(mousePt))
    {
        return HTBOTTOMLEFT;
    }

    Storm::Rect<int>  rightBorder(rect.width - borderWidth, borderWidth, rect.width, rect.height - borderWidth);
    if (rightBorder.contains(mousePt))
    {
        return HTRIGHT;
    }

    Storm::Rect<int>  bottomRightBorder(rect.width - borderWidth, rect.height - borderWidth, rect.width, rect.height);
    if (bottomRightBorder.contains(mousePt))
    {
        return HTBOTTOMRIGHT;
    }


    Storm::Rect<int>  bottomBorder(borderWidth, rect.height - borderWidth, rect.width - borderWidth, rect.height);
    if (bottomBorder.contains(mousePt))
    {
        return HTBOTTOM;
    }

    return HTCLIENT;
}

} // namespace overlay_game