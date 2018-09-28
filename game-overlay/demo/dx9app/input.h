#pragma once

class Input
{
public:
    Input();
    ~Input();

    bool Initialize(HINSTANCE, HWND, int, int);
    void Shutdown();
    bool Frame();

    bool IsEscapePressed();
    void GetMouseLocation(int&, int&);

private:
    bool ReadKeyboard();
    bool ReadMouse();
    void ProcessInput();

    Windows::ComPtr<IDirectInput8>       m_pDirectInput;                  //IDirectInput接口对象
    Windows::ComPtr<IDirectInputDevice8> m_pMouse;            //IDirectInput设备对象
    Windows::ComPtr<IDirectInputDevice8> m_pKeyboard;            //IDirectInput设备对象

    unsigned char m_keyboardState[256];
    DIMOUSESTATE m_mouseState;

    int m_screenWidth, m_screenHeight;
    int m_mouseX, m_mouseY;
};