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

namespace DP
{

////////////////////////////////////////////////////////////////////////////////

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

enum class MessageId_t : int {};

constexpr bool operator==(const MessageId_t lhs, const MessageId_t rhs) {
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}

namespace Message
{
    enum class Type
    {
        Unknown = 0,
        Message,
        Event,
        Transaction
    };

    namespace Id {

    constexpr auto Unknown           = static_cast<MessageId_t>(0);

    constexpr auto Screenshot        = static_cast<MessageId_t>(1);

    constexpr auto Message_First     = static_cast<MessageId_t>(0x10000);
    constexpr auto Message_Last      = static_cast<MessageId_t>(0x1FFFF);

    constexpr auto Event_First       = static_cast<MessageId_t>(0x20000);

    constexpr auto Transaction_First = static_cast<MessageId_t>(0x30000);

    }

    constexpr auto MakeId(const int id) noexcept {
        constexpr auto first = static_cast<int>(Message::Id::Message_First);
        //constexpr auto last = static_cast<int>(Message::Id::Message_Last);
        //if consteval { static_assert(first + id <= last); }
        return static_cast<MessageId_t>(first + id);
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

        Data_t(
            Stage_t        stage       = Stage::Any,
            MessageId_t    messageId   = Message::Id::Unknown,
            size_t         size        = sizeof(Data_t),
            const wchar_t* className   = nullptr,
            Message::Type  messageType = Type::Message,
            ReleaseFn_t    releaseFn   = nullptr)
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
        //~Data_t() = delete;
    };

////////////////////////////////////////////////////////////////////////////////

} // Message
} // DP

#endif // Include_DPMESSAGE_H

////////////////////////////////////////////////////////////////////////////////
