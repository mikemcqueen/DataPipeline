#pragma once

namespace Ui
{
    typedef unsigned WindowId_t;
    typedef unsigned WidgetId_t;

    namespace Window
    {
        namespace Id
        {
            enum WindowId_t
            {
                Unknown    = 0,
                MainWindow = 1,   // main application overlapped window

                User_First = 100
            };
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
