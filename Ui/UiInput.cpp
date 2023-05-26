////////////////////////////////////////////////////////////////////////////////
//
// UiInput.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UiInput.h"
#include "Rect.h"
#include "Log.h"

namespace Ui
{

Input_t::
Input_t()
{
}

////////////////////////////////////////////////////////////////////////////////

bool
Input_t::
Click(
    HWND            hWnd,
    POINT           point,
    Mouse::Button_t Button /*= Mouse::Button::Left*/)
{
    MoveToAbsolute(hWnd, point);
    Pause(DefaultMoveClickDelay);
    return Click(Button);
}

////////////////////////////////////////////////////////////////////////////////

bool
Input_t::
Click(
    Mouse::Button_t Button /*= Mouse::Button::Left*/)
{
    MouseDown(Button);
    MouseUp(Button);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Input_t::
MoveToAbsolute(
    HWND hWnd,
    POINT point,
    size_t count /*= 1*/)
{
    ClientToScreen(hWnd, &point);
    Normalize(point);
    INPUT Input      = { 0 };
    Input.type       = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE;
    Input.mi.dx      = point.x;
    Input.mi.dy      = point.y;
    while (0 < count--)
    {
        INPUT copyInput = Input;
        Send(copyInput);
        Pause(DefaultMoveDelay);
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

#if 0
/* static */
bool Input_t::SendChars(
   std::string_view text)
{
  text;
#if 0
    for (int index = 0; L'\0' != pText[index]; ++index)
    {
        if (!SendChar(pText[index]))
        {
            LogError(L"Input_t::SendChars(): Could not send string (%ls) char (%c)",
                     pText, pText[index]);
            return false;
        }
    }
#endif
    return true;
}
#endif

////////////////////////////////////////////////////////////////////////////////

/* static */
bool Input_t::SendChar(
  char ch,
  size_t count /*= 1*/,
  size_t delay /*= kDefaultSendcharDelay */)
{
  short vkScan = GetVkCode(ch);
  BYTE vkCode = LOBYTE(vkScan);
  if (vkInvalid != vkCode)
  {
    INPUT shiftDown = { 0 };
    shiftDown.type = INPUT_KEYBOARD;
    shiftDown.ki.wVk = VK_SHIFT;
    INPUT shiftUp = shiftDown;
    shiftUp.ki.dwFlags = KEYEVENTF_KEYUP;

    bool shift = 0 != (HIBYTE(vkScan) & 0x01); // shift key
    while (0 < count--)
    {
      INPUT keyDown = { 0 };
      keyDown.type = INPUT_KEYBOARD;
      keyDown.ki.wVk = vkCode;
      INPUT keyUp = keyDown;
      keyUp.ki.dwFlags = KEYEVENTF_KEYUP;
      if (shift)
      {
        Send(shiftDown);
        Pause(delay);
      }
      Send(keyDown);
      Pause(delay);
      Send(keyUp);
      Pause(delay);
      if (shift)
      {
        Send(shiftUp);
        Pause(delay);
      }
    }
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

/* static */
bool
Input_t::
SendKey(
  WORD vkKey,
  bool keyUp /*= false */)
{
  INPUT key = { 0 };
  key.type = INPUT_KEYBOARD;
  key.ki.wVk = vkKey;
  if (keyUp)
  {
    key.ki.dwFlags = KEYEVENTF_KEYUP;
  }
  Send(key);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

/* static */
short Input_t::GetVkCode(char ch)
{
  short vkScan = vkInvalid;
  switch (ch)
  {
  case '\r':    // fall through
  case '\n':    vkScan = VK_RETURN; break;
    //    case '\\':    vkCode = VK_DIVIDE; break;
    //    case '\010':  vkCode = VK_BACK;   break;
    //    case '\027':  vkCode = VK_ESCAPE; break;
  default:
    vkScan = ::VkKeyScanA(ch);
    break;
  }
  if (vkInvalid == LOBYTE(vkScan)) {
    LogError(L"Unknown vk code for (%c)", ch);
    throw std::runtime_error("Unknown vk code");
  }
  return vkScan;
}

////////////////////////////////////////////////////////////////////////////////

void Input_t::Normalize(    POINT& Point)
{
    double ScreenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1.0;
    double ScreenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1.0;
    Point.x = int(Point.x * (65535.0 / ScreenWidth));
    Point.y = int(Point.y * (65535.0 / ScreenHeight));
}

////////////////////////////////////////////////////////////////////////////////

bool
Input_t::
MouseDown(
    Mouse::Button_t /*Button*/)
{
    INPUT Input      = { 0 };
    Input.type       = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    Send(Input);
    Pause(DefaultClickDelay);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

bool
Input_t::
MouseUp(
    Mouse::Button_t /*Button*/)
{
    INPUT Input      = { 0 };
    Input.type       = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    Send(Input);
    Pause(DefaultClickDelay);
    return true;
}

////////////////////////////////////////////////////////////////////////////////

/* static */
bool Input_t::Send(INPUT& input) {
  UINT EventCount = SendInput(1, &input, sizeof(INPUT));
  if (EventCount != 1) {
    LogError(L"Input_t::Send() Size(%d) Sent(%d) GLE(%d)",
      1, EventCount, GetLastError());
    return false;
  }
  return true;
}

} // Ui
