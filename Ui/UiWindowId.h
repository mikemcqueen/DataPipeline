#pragma once

#ifndef INCLUDE_UIWINDOWID_H
#define INCLUDE_UIWINDOWID_H

namespace Ui
{
    typedef unsigned WidgetId_t;

    enum class WindowId_t : int {};

    constexpr bool operator==(const WindowId_t lhs, const WindowId_t rhs) noexcept {
        return static_cast<int>(lhs) == static_cast<int>(rhs);
    }

    constexpr bool operator!=(const WindowId_t lhs, const WindowId_t rhs) noexcept {
        return !(lhs == rhs);
    }

    namespace Window
    {

    namespace Id
    {
        constexpr auto Unknown = static_cast<WindowId_t>(0);
        constexpr auto MainWindow = static_cast<WindowId_t>(1);   // main application overlapped window
        constexpr auto User_First = static_cast<WindowId_t>(100);
    }

    } // Window

    namespace Widget
    {
        namespace Id
        {
            enum WidgetId_t
            {
                Unknown        = 0,
                VScrollUp,
                VScrollDown,
                HScrollUp,
                HScrollDown,

                User_First     = 1000
            };
        }
    } // Widget

} // Ui

#endif // INCLUDE_UIWINDOWID_H