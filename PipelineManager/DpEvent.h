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

    constexpr auto MakeId(const int id) noexcept {
        const auto first = static_cast<int>(Message::Id::Event_First);
        return Message::MakeId(first + id);
    }

    namespace Id
    {
        constexpr auto Unknown    = Message::Id::Unknown;
        constexpr auto Start      = MakeId(0); // 0x00020000
        constexpr auto Stop       = MakeId(1);
        constexpr auto Ui_First   = MakeId(0x1000);
        constexpr auto User_First = MakeId(0x2000);
    }

    ////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public Message::Data_t
    {
        Flag_t  Flags;

        explicit Data_t(
            Stage_t       stage       /*= DP::Stage_t::Any*/,
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
                Stage_t::Acquire,
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
                Stage_t::Acquire,
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
