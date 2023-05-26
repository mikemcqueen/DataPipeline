////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// UiWindow.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_UIWINDOW_H
#define Include_UIWINDOW_H

#include "UiTypes.h"
#include "UiWindowId.h"
#include "Rect.h"

class CSurface;

////////////////////////////////////////////////////////////////////////////////

namespace Ui::Window {
  namespace LocateBy {
    constexpr unsigned LastOriginMatch = 0x1;
    constexpr unsigned OriginSearch = 0x2;
    constexpr unsigned AnyMeans = (LastOriginMatch | OriginSearch);
  }

#if 0
  ////////////////////////////////////////////////////////////////////////////
  //
  // Ui::Window::Handle_t
  //
  ////////////////////////////////////////////////////////////////////////////

  struct Handle_t {
    HWND       hWnd;
    WindowId_t WindowId;

    Handle_t(
      HWND       InitHwnd = nullptr,
      WindowId_t InitWindowId = Id::Unknown)
      :
      hWnd(InitHwnd),
      WindowId(InitWindowId)
    { }
  };
#endif

  class Base_t;
  using Vector_t = std::vector<std::reference_wrapper<const Base_t>>;

  ////////////////////////////////////////////////////////////////////////////
  //
  // Ui::Window::Base_t
  //
  ////////////////////////////////////////////////////////////////////////////

  class Base_t {
    /*
      public:
        static Ui::Scroll::Position_t GetVertScrollPos(
          const CSurface& Surface,
          const Rect_t& ScrollUpRect,
          const Rect_t& ScrollDownRect);
    */
  public:
    Base_t(
      WindowId_t WindowId,
      const Base_t& ParentWindow,
      std::string_view windowName,
      Vector_t children = {},
      Flag_t Flags = 0,
      std::span<const Widget::Data_t> widgets = std::span<const Widget::Data_t>{});

    virtual ~Base_t();

    Base_t() = delete;
    Base_t(const Base_t&) = delete;
    const Base_t& operator=(const Base_t&) = delete;

    // Determine the active window Id and rectangle by looking
    // at pixels of the supplied surface:
    virtual WindowId_t GetWindowId(
      const CSurface& Surface,
      const POINT* pptHint) const;

    // Determine the rectangle of the supplied widget Id.
    // Return true if the widget was found, false otherwise.
    virtual bool GetWidgetRect(
      Ui::WidgetId_t WidgetId,
      Rect_t* pWidgetRect) const;

    virtual bool UpdateScrollPosition(
      Scroll::Bar_t   /*ScrollBar*/,
      const CSurface& /*Surface*/)
    {
      return false;
    }

    virtual bool IsLocatedOn(
      const CSurface& Surface,
      Flag_t    flags,
      POINT* pptOrigin = nullptr) const;

    virtual const CSurface* GetOriginSurface() const { return nullptr; }

    virtual void GetOriginSearchRect(
      const CSurface& surface,
      Rect_t& rect) const;

    bool HasChildWindow(WindowId_t WindowId) const;
    const Base_t& GetWindow(Ui::WindowId_t WindowId) const;

    bool GetWidgetRect(
      Ui::WidgetId_t  WidgetId,
      const Rect_t& RelativeRect,
      Rect_t* pWidgetRect,
      std::span<const Widget::Data_t> widgets) const;

    bool GetWidgetRect(
      Ui::WidgetId_t  WidgetId,
      const Rect_t& RelativeRect,
      Rect_t* pWidgetRect) const;

    WindowId_t  GetWindowId() const { return m_WindowId; }
    const char* GetWindowName() const { return m_windowName.c_str(); }
    const Base_t& GetParent() const { return m_ParentWindow; }
    Flag_t GetFlags() const { return m_Flags; }
    size_t GetWidgetCount() const { return widgets_.size(); }

    bool ValidateBorders(
      const CSurface& Surface,
      const Rect_t& Rect,
      const SIZE& BorderSize,
      const COLORREF  LowColor,
      const COLORREF  HighColor) const;

    bool ValidateBorder(
      const CSurface& Surface,
      const Rect_t& BorderRect,
      const wchar_t* pBorderName,
      const COLORREF  LowColor,
      const COLORREF  HighColor) const;

    void DumpWidgets(
      const CSurface& Surface,
      const Rect_t& RelativeRect) const;

    bool FindOrigin(
      const CSurface& Surface,
      const POINT& ptHint,
      POINT& ptOrigin) const;

    const POINT GetLastOrigin() const { return m_ptLastOrigin; }
    void SetLastOrigin(POINT ptOrigin) const { m_ptLastOrigin = ptOrigin; }

    bool CompareLastOrigin(
      const CSurface& surface,
      const CSurface& image,
      POINT* pptOrigin) const;

    bool OriginSearch(
      const CSurface& surface,
      const CSurface& image,
      POINT* pptOrigin) const;

  private:
    const Base_t& m_ParentWindow;
    WindowId_t    m_WindowId;
    std::string   m_windowName;
    Flag_t        m_Flags;
    mutable POINT m_ptLastOrigin;
    Vector_t children_;
    std::vector<Widget::Data_t> widgets_;
  };

  class WithHandle_t : public Base_t {
  public:
    WithHandle_t(
      WindowId_t WindowId,
      std::string_view class_name,
      std::string_view window_name,
      Vector_t children = {},
      Flag_t Flags = 0);

    HWND Hwnd() const { return hwnd_; }
    void GetClientRect(Rect_t& rect) const { ::GetClientRect(hwnd_, &rect); }
    bool IsVisibleTopWindow() const;

    HWND SyncHwnd() const;
    HWND SyncHwndGetClientRect(Rect_t& rect) const;

    bool TakeSnapshot(CSurface& surface);
      
    bool ClickWidget(
      const Base_t& Window,
      WidgetId_t WidgetId,
      const Rect_t* pRect = nullptr) const;

#if 0
    bool SetWidgetText(
      WidgetId_t    WidgetId,
      const std::wstring& text) const;

    bool ClearWidgetText(
      WidgetId_t widgetId,
      size_t     count) const;

    const Widget::Data_t& GetWidgetData(WidgetId_t widgetId) const;

    bool Scroll(Scroll::Direction_t Direction) const; // ,size_t Count)

    Scroll::Position_t GetScrollPosition(Scroll::Bar_t ScrollBar) const {
      // TODO: Lock
      using namespace Scroll::Bar;
      return (Vertical == ScrollBar) ? m_VertScrollPos : m_HorzScrollPos;
    }

    void SetScrollPosition(
      Scroll::Bar_t      ScrollBar,
      Scroll::Position_t ScrollPos)
    {
      // TODO: Lock
      using namespace Scroll::Bar;
      if (Vertical == ScrollBar) {
        m_VertScrollPos = ScrollPos;
      }
      else {
        m_HorzScrollPos = ScrollPos;
      }
    }
#endif

  private:
    HWND FindWindow(std::string_view class_name) const;
    HWND FindWindow() const { return FindWindow(class_name_); }

    std::string  class_name_;
    mutable HWND hwnd_;

    //Scroll::Position_t m_VertScrollPos = Scroll::Position::Unknown;
    //Scroll::Position_t m_HorzScrollPos = Scroll::Position::Unknown;
  };
} // namespace Ui::Window

#endif // Include_UIWINDOW_H
