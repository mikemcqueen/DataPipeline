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
namespace Event
{
    namespace Id
    {
        enum : unsigned
        {
            ThumbPosition,
            Scroll,
            Click,
            Collection,
            SendChars,
            AddTrade
        };
    }

    struct Data_t :
        DP::Event::Data_t
    {
        Lon::Window::Type_e WindowType;

        Data_t(
            DP::Stage_t         Stage      = DP::Stage::Any,
            DP::MessageId_t     Id         = DP::Message::Id::Unknown,
            Lon::Window::Type_e InitWindowType = Lon::Window::Unknown,
            DP::Event::Flag_t   Flags      = 0,
            size_t              Size       = sizeof(Data_t))
        :
            DP::Event::Data_t(
                Stage,
                Id,
                Flags,
                Size),
            WindowType(InitWindowType)
        { }        
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
