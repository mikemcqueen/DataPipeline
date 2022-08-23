////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonEventTypes.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_LONEVENTTYPES_H
#define Include_LONEVENTTYPES_H

#include "LonMessageTypes.h"
#include "LonTypes.h"
#include "CommonTypes.h"
#include "DpEvent.h"

namespace Lon
{

using EventId_t = DP::MessageId_t;

namespace Event
{

    constexpr auto MakeId(const unsigned id) {
        //const unsigned max = 0x10000;
        //if consteval {
        //    static_assert(id < max);
        //}
        const auto first = static_cast<unsigned>(DP::Event::Id::User_First);
        return EventId_t(first + id);
    }

    namespace Id
    {
        static const auto ThumbPosition = EventId_t(MakeId(0));
        static const auto Scroll        = EventId_t(MakeId(1));
        static const auto Click         = EventId_t(MakeId(2));
        static const auto Collection    = EventId_t(MakeId(3));
        static const auto SendChars     = EventId_t(MakeId(4));
        static const auto AddTrade      = EventId_t(MakeId(5));
    }

    struct Data_t :
        DP::Event::Data_t
    {
        Lon::Window::Type_e WindowType;

        Data_t(
            DP::Stage_t         Stage, /*    = DP::Stage::Any,*/
            DP::MessageId_t     Id,    /*    = DP::Message::Id::Unknown,*/
            Lon::Window::Type_e windowType = Lon::Window::Unknown,
            DP::Event::Flag_t   flags      = 0,
            size_t              size       = sizeof(Data_t))
            :
            DP::Event::Data_t(
                Stage,
                Id,
                flags,
                size),
            WindowType(windowType)
        { }

        Data_t() = default;
    };

    ////////////////////////////////////////////////////////////////////////////////

    struct Start_t : public Lon::Event::Data_t
    {
        Start_t() : Data_t(DP::Stage::Acquire, DP::Event::Id::Start) {}
    };

    ////////////////////////////////////////////////////////////////////////////////

    struct Stop_t : public Data_t
    {
        Stop_t() : Data_t(DP::Stage::Acquire, DP::Event::Id::Stop) {}
    };

    ////////////////////////////////////////////////////////////////////////////////

    /*
    struct EventScreenShot_t : public Event_t
    {
        EventScreenShot_t() : Event_t(Event::ScreenShot) {}
    };
    */

    ////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

class Collection_t
{
public:

    typedef std::vector<Data_t*> EventVector_t;
    // This is kinda hacky with the pointer usage.  What i really want is an
    // array of heterogeneous types.
    struct Data_t :
        public Event::Data_t
    {
        EventVector_t Events;

        Data_t(DP::Stage_t Stage) : 
            Event::Data_t(
                Stage,
                Id::Collection,
                Lon::Window::Unknown,
                0,
                sizeof(Data_t))
        { }
    } m_Data;

    Collection_t(DP::Stage_t Stage) : m_Data(Stage)
    { }

    void
    Add(Event::Data_t* pEventData)
    {
        m_Data.Events.push_back(pEventData);
    }
};


} // Event
} // Lon

////////////////////////////////////////////////////////////////////////////////

#endif // Include_LONEVENTTYPES_H

////////////////////////////////////////////////////////////////////////////////
