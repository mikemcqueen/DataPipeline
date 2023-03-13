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

namespace DP {
  using EventId_t = MessageId_t;

  namespace Event {
    typedef unsigned Flag_t;

    namespace Flag {
      constexpr unsigned Synchronous = 0x1;
    }

    template<int Id>
    constexpr auto MakeId() noexcept -> EventId_t {
      return Message::MakeId<Id, Message::Id::Event_First, Message::Id::Event_Last>();
    }

    namespace Id {
      constexpr auto Unknown = Message::Id::Unknown;
      constexpr auto Start = MakeId<0>();
      constexpr auto Stop = MakeId<1>();
      constexpr auto Ui_First = MakeId<0x1000>();
      constexpr auto User_First = MakeId<0x2000>();
    }

    struct Data_t : Message::Data_t {
      explicit Data_t(
        Stage_t       stage       /*= DP::Stage_t::Any*/,
        EventId_t     eventId = Id::Unknown, // TODO: remove default initializer
        size_t        size = sizeof(Data_t),
        Flag_t        flags = 0,
        std::string_view msg_name = {},
        Message::Type messageType = Message::Type::Event) :
        Message::Data_t(stage, eventId, size, msg_name, messageType), Flags(flags)
      {}

      Flag_t  Flags;
    };

    struct StartAcquire_t : Data_t {
      StartAcquire_t() :
        Data_t(Stage_t::Acquire, Event::Id::Start, sizeof(StartAcquire_t)) {}
    };

    struct StopAcquire_t : Data_t {
      StopAcquire_t() : Data_t(Stage_t::Acquire, Event::Id::Stop,
        sizeof(StopAcquire_t)) {}
    };
  }// Event

  using EventPtr_t = std::unique_ptr<Event::Data_t>;
} // DP

#endif // Include_DPEVENT_H
