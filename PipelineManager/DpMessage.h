////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008-2009 Mike McQueen.  All rights reserved.
//
// DpMessage.h
//
////////////////////////////////////////////////////////////////////////////////

#if _MSC_VER >= 1000
#pragma once
#endif

#ifndef Include_DPMESSAGE_H
#define Include_DPMESSAGE_H

#include "CommonTypes.h"

namespace DP {

////////////////////////////////////////////////////////////////////////////////

/*
typedef unsigned Stage_t;
namespace Stage
{
    enum : Stage_t
    {
        None       = 0,
        Acquire    = 0x00000001,
        Translate  = 0x00000002,
        Interpret  = 0x00000004,
        Analyze    = 0x00000008,
        Execute    = 0x00000010,
        Any        = 0x0000001F,
    };
}
*/
enum class Stage_t {
    None =      0,
    Acquire =   0x00000001,
    Translate = 0x00000002,
    Interpret = 0x00000004,
    Analyze =   0x00000008,
    Execute =   0x00000010,
    Any =       0x0000001F,
};

enum class MessageId_t : int {};

namespace Message {

    enum class Type
    {
        Unknown = 0,
        Message,
        Event,
        Transaction
    };

    template<int Id>
    constexpr auto MakeId() noexcept {
        return static_cast<MessageId_t>(Id);
    }

    template<int Id, MessageId_t First, MessageId_t Last>
    constexpr auto MakeId() noexcept {
        constexpr auto actualId = intValue(First) + Id;
        static_assert(Id >= 0 && actualId <= intValue(Last));
        return MakeId<actualId>();
    }

    namespace Id {
        constexpr auto Unknown =           MakeId<0>();
        constexpr auto Screenshot =        MakeId<1>();

        constexpr auto Message_First =     MakeId<0x10000>();
        constexpr auto Message_Last =      MakeId<0x1FFFF>();

        constexpr auto Event_First =       MakeId<0x20000>();
        constexpr auto Event_Last =        MakeId<0x2FFFF>();

        constexpr auto Transaction_First = MakeId<0x30000>();
        constexpr auto Transaction_Last =  MakeId<0x3FFFF>();
    }

    struct Data_t;
    using ReleaseFn_t = void (*)(Data_t& data);

    struct Data_t
    {
        static constexpr int ClassLength = 32;

        Stage_t       Stage;
        MessageId_t   Id;
        size_t        Size;
        wchar_t       Class[ClassLength];
        Message::Type Type;
        ReleaseFn_t   ReleaseFn;

        explicit
        Data_t(
            Stage_t        stage,
            MessageId_t    messageId = Message::Id::Unknown,
            size_t         size = sizeof(Data_t),
            const wchar_t* className = nullptr,
            Message::Type  messageType = Type::Message,
            ReleaseFn_t    releaseFn = nullptr)
            :
            Stage(stage),
            Type(messageType),
            Id(messageId),
            Size(size),
            ReleaseFn(releaseFn)
        {
            Class[0] = L'\0';
            if (nullptr != className) {
                wcscpy_s(Class, className);
            }
        }

        Data_t() final = default;
    };

}// Message
} // DP

#endif // Include_DPMESSAGE_H
