////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// UiWindow.cpp
//
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UiWindow.h"
#include "PipelineManager.h"
#include "UiEvent.h"
#include "UiInput.h"
#include "DdUtil.h"
#include "Log.h"
#include "Macros.h"
#include "dp_msg.h"

namespace Ui::Window {

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Constructor for 'fake' child 'windows' that have no associated hWnd.
  //

  Base_t::Base_t(
    WindowId_t WindowId,
    const Base_t& ParentWindow,
    std::string_view windowName,
    Vector_t children,
    Flag_t Flags /*= 0*/,
    std::span<const Widget::Data_t> widgets) :
    m_WindowId(WindowId),
    m_ParentWindow(ParentWindow),
    m_windowName(windowName.empty() ? "Undefined" : windowName),
    m_Flags(Flags),
    children_(children),
    widgets_(widgets.begin(), widgets.end())
  {
  }

  Base_t::~Base_t() = default;

  bool Base_t::HasChildWindow(WindowId_t WindowId) const {
    for (auto child : children_) {
      if (child.get().GetWindowId() == WindowId) {
        return true;
      }
    }
    for (auto child : children_) {
      if (child.get().HasChildWindow(WindowId)) {
        return true;
      }
    }
    return false;
  }
    
  const Base_t& Base_t::GetWindow(WindowId_t WindowId) const {
    for (auto child : children_) {
      if (child.get().GetWindowId() == WindowId) {
        return child.get();
      }
    }
    for (auto child : children_) {
      if (child.get().HasChildWindow(WindowId)) {
        return child.get().GetWindow(WindowId);
      }
    }
    LogError(L"%S:GetWindow() failed, id: %d", GetWindowName(), WindowId);
    throw std::invalid_argument("GetWindow failed, call HasChildWindow first?");
  }

  WindowId_t Base_t::GetWindowId(
    const CSurface& /*Surface*/,
    const POINT*    /*pptHint*/) const
  {
    if (!children_.empty()) {
      throw std::runtime_error("Ui::Window_t::GetWindowId(CSurface, POINT) must be implemented");
    }
    return m_WindowId;
  }

  bool Base_t::GetWidgetRect(
    Ui::WidgetId_t WidgetId,
    Rect_t* WidgetRect) const
  {
    Rect_t originRect(m_ptLastOrigin, { 0, 0 });
    return GetWidgetRect(WidgetId, originRect, WidgetRect);
    // NOTE: we could support this, by using "windowrect" as default
    // but i don't need that functionality, and i want to be warned
    // when i haven't overridden this in a derived class, so it stays
    // like this.
    // TODO: should be a flag to select behavior
    //throw logic_error("Ui::Window_t::GetWidgetRect() not implemented");
  }

  bool Base_t::GetWidgetRect(
    Ui::WidgetId_t WidgetId,
    const Rect_t& RelativeRect,
    Rect_t* pWidgetRect) const
  {
    return GetWidgetRect(WidgetId, RelativeRect, pWidgetRect, std::span{ widgets_ });
  }

  bool Base_t::GetWidgetRect(
    Ui::WidgetId_t WidgetId,
    const Rect_t& RelativeRect,
    Rect_t* pWidgetRect,
    std::span<const Widget::Data_t> widgets) const
  {
    for (auto& widget : widgets) {
      if (widget.WidgetId == WidgetId) {
        RelativeRect_t rect(widget.RectData);
        *pWidgetRect = rect.GetRelativeRect(RelativeRect);
        return true;
      }
    }
    return false;
  }

  bool Base_t::IsLocatedOn(
    const CSurface& surface,
    Flag_t flags,
    POINT* pptOrigin /*= nullptr*/) const
  {
    const CSurface* pOriginSurface = GetOriginSurface();
    if (pOriginSurface) {
      using namespace Ui::Window;
      if (flags.Test(LocateBy::LastOriginMatch) &&
        CompareLastOrigin(surface, *pOriginSurface, pptOrigin))
      {
        return true;
      }
      if (flags.Test(LocateBy::OriginSearch) &&
        OriginSearch(surface, *pOriginSurface, pptOrigin))
      {
        return true;
      }
    }
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////

  void Base_t::GetOriginSearchRect(
    const CSurface& surface,
    Rect_t& rect) const
  {
    LogWarning(L"%S::GetOriginSearchRect() Using entire BltRect", GetWindowName());
    rect = surface.GetBltRect();
  }

  ////////////////////////////////////////////////////////////////////////////////

  bool Base_t::CompareLastOrigin(
    const CSurface& surface,
    const CSurface& image,
    POINT* pptOrigin) const
  {
    const POINT pt = GetLastOrigin();
    if ((0 < pt.x) || (0 < pt.y)) {
      if (surface.Compare(pt.x, pt.y, image)) {
        if (pptOrigin) {
          *pptOrigin = pt;
        }
        //LogInfo(L"%S::CompareLastOrigin() Match", GetWindowName());
        return true;
      }
    }
    return false;
  }

  ////////////////////////////////////////////////////////////////////////////////

  bool Base_t::OriginSearch(
    const CSurface& surface,
    const CSurface& image,
    POINT* pptOrigin) const
  {
    Rect_t searchRect;
    GetOriginSearchRect(surface, searchRect);
    // search for the supplied origin bitmap on the supplied surface
    POINT ptOrigin;
    if (surface.FindSurfaceInRect(image, searchRect, ptOrigin, nullptr)) {
      //LogInfo(L"%S::OriginSearch(): Found @ (%d, %d)", GetWindowName(),
      //  ptOrigin.x, ptOrigin.y);
      if (pptOrigin) {
        *pptOrigin = ptOrigin;
      }
      SetLastOrigin(ptOrigin);
      return true;
    }
    return false;
  }

  bool Window_t::ValidateBorders(
    const CSurface& Surface,
    const Rect_t& Rect,
    const SIZE& BorderSize,
    const COLORREF  LowColor,
    const COLORREF  HighColor) const
  {
    Rect_t BorderRect = Rect;
    BorderRect.bottom = BorderRect.top + BorderSize.cy;
    if (!ValidateBorder(Surface, BorderRect, L"Top", LowColor, HighColor)) {
      return false;
    }
    OffsetRect(&BorderRect, 0, Rect.Height() - BorderSize.cy);
    if (!ValidateBorder(Surface, BorderRect, L"Bottom", LowColor, HighColor)) {
      return false;
    }

    BorderRect = Rect;
    BorderRect.right = BorderRect.left + BorderSize.cx,
      BorderRect.top += BorderSize.cy;
    BorderRect.bottom -= BorderSize.cy;
    if (!ValidateBorder(Surface, BorderRect, L"Left", LowColor, HighColor)) {
      return false;
    }
    OffsetRect(&BorderRect, Rect.Width() - BorderSize.cx, 0);
    if (!ValidateBorder(Surface, BorderRect, L"Rignt", LowColor, HighColor)) {
      return false;
    }
    return true;
  }

  bool Window_t::ValidateBorder(
    const CSurface& Surface,
    const Rect_t& BorderRect,
    const wchar_t* pBorderName,
    const COLORREF  LowColor,
    const COLORREF  HighColor) const
  {
    if (!Surface.CompareColorRange(BorderRect, LowColor, HighColor)) {
      LogWarning(L"%S::ValidateBorder(): %ls border doesn't match @ (%d, %d)",
        GetWindowName(), pBorderName, BorderRect.left, BorderRect.top);
      return false;
    }
    return true;
  }

  void Window_t::DumpWidgets(
    const CSurface& Surface,
    const Rect_t& RelativeRect) const
  {
    for (size_t Widget = 0; Widget < GetWidgetCount(); ++Widget) {
      RelativeRect_t Rect(widgets_[Widget].RectData);
      Rect_t WidgetRect = Rect.GetRelativeRect(RelativeRect);
      wchar_t szBuf[255];
      swprintf_s(szBuf, L"Diag\\%S_widget%d.bmp", GetWindowName(), Widget);
      Surface.WriteBMP(szBuf, WidgetRect);
    }
  }

#if 0
  static
  Ui::Scroll::Position_t Window_t::GetVertScrollPos(
    const CSurface& Surface,
    const Rect_t& VScrollUpRect,
    const Rect_t& VScrollDownRect) //const
  {
    extern bool g_bWriteBmps;

    static const size_t ScrollIntensity = 0xb0;
    static const size_t ActiveCount = 20;

    if (g_bWriteBmps) {
      Surface.WriteBMP(L"Diag\\VScrollUpRect.bmp", VScrollUpRect);
      Surface.WriteBMP(L"Diag\\VScrollDownRect.bmp", VScrollDownRect);
    }
    bool bUpActive = ActiveCount <= Surface.GetIntensityCount(VScrollUpRect, ScrollIntensity);
    bool bDownActive = ActiveCount <= Surface.GetIntensityCount(VScrollDownRect, ScrollIntensity);
    using namespace Ui::Scroll;
    Position_t Pos = Position::Unknown;
    if (!bUpActive && !bDownActive) {
      Pos = Position::Unknown;
    }
    else if (bUpActive && bDownActive) {
      Pos = Position::Middle;
    }
    else if (bUpActive) {
      Pos = Position::Bottom;
    }
    else if (bDownActive) {
      Pos = Position::Top;
    }
    else {
      throw std::logic_error("UI::Window_t::GetVertScrollPos(): Impossible!");
    }
    //LogAlways(L"MainWindow_t::GertVertScrollPos() ScrollRect = { %d, %d, %d, %d }",
    //          ScrollRect.left, ScrollRect.top, ScrollRect.right, ScrollRect.bottom);
    return Pos;
  }
#endif

  ////////////////////////////////////////////////////////////////////////////////
  //
  //
  ////////////////////////////////////////////////////////////////////////////////

  WithHandle_t::WithHandle_t(
    WindowId_t WindowId,
    std::string_view class_name,
    std::string_view window_name,
    Vector_t children,
    Flag_t flags /* = 0 */) :
    Base_t(WindowId, *this, window_name, children, flags),
    class_name_(class_name),
    hwnd_(FindWindow(class_name))
  {
    if (!hwnd_) {
      LogError(L"Ui::Window::WithHandle_t(%S): Window not found, class_name(%S)",
        window_name.data(), class_name.data());
    }
  }

  HWND WithHandle_t::FindWindow(std::string_view class_name) const {
    HWND hwnd{};
    if (!class_name.empty()) {
      hwnd = ::FindWindowA(class_name.data(), nullptr);
    }
    return hwnd;
  }

  bool WithHandle_t::IsVisibleTopWindow() const {
    if (!hwnd_ || !::IsWindowVisible(hwnd_) || (hwnd_ != ::GetForegroundWindow())) {
      // noisy, extra log level maybe
      //LogError(L"IsVisibleTopWindow(%S) failed, hWnd (%08x)", GetWindowName(), hwnd_);
      return false;
    }
    return true;
  }

  HWND WithHandle_t::SyncHwnd() const {
    if (!hwnd_ || !::IsWindow(hwnd_)) {
      if (hwnd_) {
        LogInfo(L"SyncHwnd(): Window handle is invalid");
      }
      hwnd_ = FindWindow();
      if (hwnd_) {
        LogInfo(L"SyncHwnd(): Window handle has been re-establishd");
      }
    }
    return hwnd_;
  }

  HWND WithHandle_t::SyncHwndGetClientRect(Rect_t& rect) const {
    HWND hwnd = SyncHwnd();
    if (hwnd) {
      GetClientRect(rect);
    }
    return hwnd;
  }

  bool WithHandle_t::ClickWidget(
    const Base_t& Window,
    WidgetId_t WidgetId,
    const Rect_t* pRect /*= nullptr*/) const
  {
    Rect_t rect;
    if (!pRect) {
      if (!Window.GetWidgetRect(WidgetId, &rect)) {
        LogError(L"ClickWidget(%S): GetWidgetRect(%d) failed",
          Window.GetWindowName(), WidgetId);
        return false;
      }
      pRect = &rect;
    }
    Ui::Input_t::Click(hwnd_, pRect->Center());
    return true;
  }

#if 0
  bool WithHandle_t::ClearWidgetText(
    WidgetId_t widgetId,
    size_t count) const
  {
    Rect_t rect;
    if (!GetWidgetRect(widgetId, &rect)) {
      LogError(L"%S::ClearWidgetText(): GetWidgetRect(%d) failed",
        GetWindowName(), widgetId);
      return false;
    }
    // click right side of rectangle
    Rect_t clickRect(rect);
    clickRect.left = clickRect.right - 1;
    Input_t::Click(hwnd_, clickRect.Center());
    while (0 < count--) {
      Input_t::SendChar(VK_BACK);
    }
    return true;
  }

  bool WithHandle_t::SetWidgetText(
    WidgetId_t widgetId,
    const wstring& text)
  {
    // TODO: why this?
    Rect_t rect;
    if (!GetWidgetRect(widgetId, &rect)) {
      LogError(L"%S::SetWidgetText(direct): GetWidgetRect(%d) failed",
        GetWindowName(), widgetId);
      return false;
    }
    Ui::Input_t Chars; // huh?
    Chars.SendChars(text.c_str());
    return true;
  }

  const Widget::Data_t& Base_t::GetWidgetData(WidgetId_t widgetId) const {
    if (nullptr != m_pWidgets)
    {
      for (size_t Widget = 0; Widget < m_WidgetCount; ++Widget)
      {
        if (m_pWidgets[Widget].WidgetId == widgetId)
        {
          return m_pWidgets[Widget];
        }
      }
    }
    throw invalid_argument("Ui::Window_t::GetWidgetData()");
  }

  bool Base_t::Scroll(
    Scroll::Direction_t Direction) const
  {
    using namespace Scroll;
    Ui::WidgetId_t WidgetId = Ui::Widget::Id::Unknown;
    switch (Direction) {
    case Direction::Up:
    {
      const Position_t VertPos = GetScrollPosition(Bar::Vertical);
      if ((Position::Top == VertPos) || (Position::Unknown == VertPos))
      {
        return false;
      }
      WidgetId = Ui::Widget::Id::VScrollUp;
    }
    break;
    case Direction::Down:
    {
      const Position_t VertPos = GetScrollPosition(Bar::Vertical);
      if ((Position::Bottom == VertPos) || (Position::Unknown == VertPos))
      {
        return false;
      }
      WidgetId = Ui::Widget::Id::VScrollDown;
    }
    break;
    case Direction::Left:
    case Direction::Right:
      return false;
    default:
      break;
    }
    if (Ui::Widget::Id::Unknown == WidgetId) {
      throw logic_error("UiWindow::Scroll(): Invalid widget id");
    }
    return ClickWidget(WidgetId);
  }
#endif
} // namespace Ui::Window
