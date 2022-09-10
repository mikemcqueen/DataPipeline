///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// BrokerUi.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#include "UiWindowId.h"

namespace Broker
{
    static const COLORREF Black                  = RGB(0,0,0);
    static const COLORREF DarkGrey10             = RGB(10,10,10);

    static const COLORREF BkLowColor             = Black;
    static const COLORREF BkHighColor            = RGB(50,50,50);

    static const COLORREF BorderLowColor         = RGB(50,50,50);
    static const COLORREF BorderHighColor        = RGB(255,255,255);

    namespace Table
    {
        static const COLORREF SelectedLowColor  = RGB(30,30,30);// RGB=0x252025
        static const COLORREF SelectedHighColor = RGB(39,39,39);

        constexpr int RowHeight            = 42;
        constexpr int QuantityTextHeight   = 14;
        constexpr int GapSizeY             = 2;
        constexpr int CharHeight           = 12;
        static const SIZE BorderSize           = { 2, 2 };
        static const SIZE DoubleBorderSize     = { BorderSize.cx * 2, BorderSize.cy * 2 };
    } // Table

    class Window_t;
    class MainWindow_t;

    namespace Window
    {

        constexpr auto MakeId(const int id) noexcept {
            constexpr auto first = static_cast<int>(Ui::Window::Id::User_First);
            return static_cast<Ui::WindowId_t>(first + id);
        }

        namespace Id {
            constexpr auto Eq2Login = MakeId(0);
            constexpr auto Eq2Loading = MakeId(1);
            constexpr auto Zoning = MakeId(2);
            constexpr auto MainChat = MakeId(3);
            constexpr auto BrokerFrame = MakeId(4);
            constexpr auto BrokerBuyTab = MakeId(5);
            constexpr auto BrokerSellTab = MakeId(6);
            constexpr auto BrokerSalesLogTab = MakeId(7);
            constexpr auto BrokerSetPricePopup = MakeId(8);
        } // Id

    } // Window

    namespace Tab {
        enum class Id {
            None,
            Buy,
            Sell,
            SalesLog,
        };
    } // Tab
    using Tab_t = Tab::Id;

    namespace Widget
    {
        namespace Id
        {
            enum : Ui::WidgetId_t
            {
                First                            = Ui::Widget::Id::User_First,
                Login_First                      = First,
                Eq2Login_First                   = First + 50,
                CharacterSelect_First            = First + 100,
                BrokerNpc_First                  = First + 200,
                BrokerFrame_First                = First + 300,
                BrokerBuy_First                  = First + 400,
                BrokerSell_First                 = First + 500,
                BrokerSalesLog_First             = First + 600,
                BrokerSetPrice_First             = First + 700,
                Last
            };
        }
    } // Widget

    namespace Login
    {
        namespace Widget
        {
            namespace Id
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::Login_First,
                    UsernameLabel                = First,
                    UsernameEdit,
                    PasswordLabel,
                    PasswordEdit,
                    LoginButton,
                    ExitButton,                  // 5
                    Last
                };
            }
        }
    } // Login

    namespace Eq2Login
    {
        static const size_t kCharacterButtonCount = 20;

        namespace Widget
        {
            namespace Id
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::Eq2Login_First,
                    CharacterEdit                = First,
                    ServerEdit,
                    ConnectButton,
                    FirstCharacterButton,
                    LastCharacterButton          = FirstCharacterButton + kCharacterButtonCount - 1,
                    Last
                };
            }
        }
    } // Login

    namespace Frame
    {
        namespace Layout
        {
            enum E : unsigned
            {
                Unknown,
                Broker,
                Market
            };
        }
        typedef Layout::E Layout_t;

        namespace Widget
        {
            namespace Id
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::BrokerFrame_First,
                    BuyTab                       = First,
                    SellTab,
                    SalesLogTab,
                    Last
                };
            }
        }
    } // Frame

    namespace Buy
    {
        namespace Widget
        {
            namespace Id
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::BrokerBuy_First,
                    FirstButton                  = First,
                    PreviousButton,
                    NextButton,
                    LastButton,
                    BuyButton,
                    PageNumber,                  //5
                    SearchLabel,
                    SearchEdit, 
                    FindButton,
                    SearchDropdown,
                    Last
                };
            }
        }
    } // Buy

    namespace Sell
    {
        namespace Widget
        {
            namespace Id
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::BrokerSell_First,
                    SetPriceButton               = First,
                    ListItemButton,
                    SearchButton,
                    RemoveItemButton,
                    DropToAddRect,               //5
                    Container1,
                    Container2,
                    Container3,
                    Container4,
                    Container5,                  //10
                    Container6,
                    Last
                };
            }
        }
    } // Sell

    namespace SalesLog
    {
        namespace Widget
        {
            namespace Id 
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::BrokerSalesLog_First,
                    Last                         = First
                };
            }
        }
    } // SalesLog

    namespace SetPrice
    {
        namespace Widget
        {
            namespace Id
            {
                enum : Ui::WidgetId_t
                {
                    First                        = Broker::Widget::Id::BrokerSetPrice_First,
                    OneButton                    = First,
                    TwoButton,
                    ThreeButton,
                    FourButton,
                    FiveButton,
                    SixButton,                   //5
                    SevenButton,
                    EightButton,
                    NineButton,
                    ZeroButton,
                    OkButton,                    //10
                    ClearButton,
                    PlatinumButton,
                    GoldButton,
                    SilverButton,
                    CopperButton,                //15
                    PriceText,
                    Last
                };
            }
        }
    }
} // Broker

