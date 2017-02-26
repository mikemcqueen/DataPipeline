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

typedef MessageId_t EventId_t;

namespace Event
{
    ////////////////////////////////////////////////////////////////////////////


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

    namespace Id
    {
        enum : EventId_t
        {
            // TODO: register in initpipelinemanager()
            Unknown    = 0,
            Start      = 1,
            Stop       = 2,
            Ui_First   = 0x00000200,
            User_First = 0x00020000,
        };
    }

    ////////////////////////////////////////////////////////////////////////////

    struct Data_t :
        public Message::Data_t
    {
        Flag_t  Flags;

        Data_t(
            Stage_t       Stage     = DP::Stage::Any,
            MessageId_t   Id        = Id::Unknown,
            size_t        Size      = sizeof(Data_t),
            MessageType_t Type      = Message::Type::Event,
            Flag_t        InitFlags = 0)
        :
            Message::Data_t(Stage, Id, Size, NULL, Type),
            Flags(InitFlags)
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
