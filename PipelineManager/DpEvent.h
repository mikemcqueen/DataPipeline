////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// DpEvent.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_DPEVENT_H
#define Include_DPEVENT_H

#include "DpMessage.h"

namespace DP
{

using EventId_t = MessageId_t;
namespace Event
{
    ////////////////////////////////////////////////////////////////////////////

    //  TODO: enum class
    typedef unsigned Flag_t;
#if 0
    namespace Flag
    {
        enum : Flag_t
        {
            Query = 0x00000001,
        };
    }
#endif

    constexpr auto MakeId(const unsigned id) {
        //const unsigned max = 0x10000;
        //if consteval {
        //    static_assert(id < max);
        //}
        const auto first = static_cast<unsigned>(Message::Id::Event_First);
        return EventId_t(first + id);
    }

    namespace Id
    {
        //using namespace Message;
        static const EventId_t Unknown    = EventId_t(0);
        static const EventId_t Start      = EventId_t(MakeId(0)); // 0x00020000
        static const EventId_t Stop       = EventId_t(MakeId(1));
        static const EventId_t Ui_First   = EventId_t(MakeId(0x1000));
        static const EventId_t User_First = EventId_t(MakeId(0x2000));
    }

    ////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public Message::Data_t
    {
        Flag_t  Flags;

        explicit Data_t(
            Stage_t       stage       = DP::Stage::Any,
            EventId_t     eventId     = Id::Unknown, // TODO: remove default initializer
            size_t        size        = sizeof(Data_t),
            Flag_t        flags       = 0,
            Message::Type messageType = Message::Type::Event)
            :
            Message::Data_t(
                stage,
                eventId,
                size,
                nullptr, 
                messageType),
            Flags(flags)
        { }
    };

    ////////////////////////////////////////////////////////////////////////////

    struct StartAcquire_t :
        public Data_t
    {
        StartAcquire_t() :
            Data_t(
                Stage::Acquire,
                Event::Id::Start,
                sizeof(StartAcquire_t))
        { }
    };

    ////////////////////////////////////////////////////////////////////////////

    struct StopAcquire_t :
        public Data_t
    {
        StopAcquire_t() :
            Data_t(
                Stage::Acquire,
                Event::Id::Stop,
                sizeof(StopAcquire_t))
        { }
    };

    ////////////////////////////////////////////////////////////////////////////

} // Event
} // DP

////////////////////////////////////////////////////////////////////////////////

#endif // Include_DPEVENT_H

////////////////////////////////////////////////////////////////////////////////
