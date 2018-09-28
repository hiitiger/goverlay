#include "StdAfx.h"
#include "input.h"


Input::Input()
{
    memset(m_keyboardState, 0, sizeof(m_keyboardState));
}

Input::~Input()
{

}

bool Input::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
    HRESULT result;


    // Store the screen size which will be used for positioning the mouse cursor.
    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    // Initialize the location of the mouse on the screen.
    m_mouseX = 0;
    m_mouseY = 0;
    POINT pt;
    GetCursorPos(&pt);

    m_mouseX = pt.x;
    m_mouseY = pt.y;

    // Initialize the main direct input interface.
    result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)m_pDirectInput.resetAndGetPointerAddress(), NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the keyboard.
    result = m_pDirectInput->CreateDevice(GUID_SysKeyboard, m_pKeyboard.resetAndGetPointerAddress(), NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format.  In this case since it is a keyboard we can use the predefined data format.
    result = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result))
    {
        return false;
    }

    // Set the cooperative level of the keyboard to not share with other programs.
    result = m_pKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // Now acquire the keyboard.
    result = m_pKeyboard->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    // Initialize the direct input interface for the mouse.
    result = m_pDirectInput->CreateDevice(GUID_SysMouse, m_pMouse.resetAndGetPointerAddress(), NULL);
    if (FAILED(result))
    {
        return false;
    }

    // Set the data format for the mouse using the pre-defined mouse data format.
    result = m_pMouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result))
    {
        return false;
    }

    // Set the cooperative level of the mouse to share with other programs.
    result = m_pMouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result))
    {
        return false;
    }

    // Acquire the mouse.
    result = m_pMouse->Acquire();
    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void Input::Shutdown()
{
    m_pMouse = 0;
    m_pKeyboard = 0; 
    m_pDirectInput = 0;
}

bool Input::Frame()
{
    bool result;


    // Read the current state of the keyboard.
    result = ReadKeyboard();
    if (!result)
    {
        return false;
    }

    // Read the current state of the mouse.
    result = ReadMouse();
    if (!result)
    {
        return false;
    }

    // Process the changes in the mouse and keyboard.
    ProcessInput();

    return true;
}

bool Input::ReadKeyboard()
{
    HRESULT result;


    // Read the keyboard device.
    result = m_pKeyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_pKeyboard->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool Input::ReadMouse()
{
    HRESULT result;


    // Read the mouse device.
    result = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
    if (FAILED(result))
    {
        // If the mouse lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
        {
            m_pMouse->Acquire();
        }
        else
        {
            return false;
        }
    }

    return true;
}

void Input::ProcessInput()
{
    // Update the location of the mouse cursor based on the change of the mouse location during the frame.
    m_mouseX += m_mouseState.lX;
    m_mouseY += m_mouseState.lY;

    // Ensure the mouse location doesn't exceed the screen width or height.
    if (m_mouseX < 0)  { m_mouseX = 0; }
    if (m_mouseY < 0)  { m_mouseY = 0; }

    if (m_mouseX > m_screenWidth)  { m_mouseX = m_screenWidth; }
    if (m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }

    return;
}

bool Input::IsEscapePressed()
{
    // Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
    if (m_keyboardState[DIK_ESCAPE] & 0x80)
    {
        return true;
    }

    return false;
}

void Input::GetMouseLocation(int& mouseX, int& mouseY)
{
    mouseX = m_mouseX;
    mouseY = m_mouseY;
    return;
}