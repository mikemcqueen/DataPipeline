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

enum class MessageId_t : unsigned {};

constexpr bool operator==(const MessageId_t lhs, const MessageId_t rhs) {
    return unsigned(lhs) == unsigned(rhs);
}

namespace Message
{
    enum class Type : unsigned
    {
        Unknown = 0,
        Message,
        Event,
        Transaction
    };

    /*struct Id {
        enum : unsigned
        {
            Undefined = 0,
            Screenshot, // TODO
            Message_First = 0x00010000,
            Event_First   = 0x00020000,
            Txn_First     = 0x00040000,
        };
    };*/

    /*enum Id : MessageId_t
    {
        Unknown = 0, // TODO: Undefined
        Screenshot,
        Message_First = 0x00010000,
        Event_First   = 0x00020000,
        Txn_First     = 0x00040000,
    };
    */
    
    namespace Id {
        constexpr auto Unknown           = MessageId_t(0);
        static const auto Screenshot        = MessageId_t(1);
        static const auto Message_First     = MessageId_t(0x00010000);
        static const auto Event_First       = MessageId_t(0x00020000);
        static const auto Transaction_First = MessageId_t(0x00030000);
    }
    
    constexpr auto MakeId(const unsigned id) -> MessageId_t {
        //const unsigned max = 0x10000;
        const auto first = static_cast<unsigned>(Message::Id::Message_First);
        //static_assert(id < max);
        return MessageId_t(first + id);
    }

    struct Data_t
    {
        static const size_t ClassLength = 32;

        Stage_t       Stage;
        MessageId_t   Id;
        size_t        Size;
        wchar_t       Class[ClassLength];
        Message::Type Type;

        Data_t(
            Stage_t        stage       = Stage::Any,
            MessageId_t    messageId   = Message::Id::Unknown,
            size_t         size        = sizeof(Data_t),
            const wchar_t* className   = nullptr,
            Message::Type  messageType = Type::Message)
            :
            Stage(stage),
            Type(messageType),
            Id(messageId),
            Size(size)
        {
            Class[0] = L'\0';
            if (nullptr != className) {
                wcscpy_s(Class, className);
            }
        }

        ~Data_t() = default; 
        
        /*
        virtual
        ~Data_t()
        {
        }
        */
    };

////////////////////////////////////////////////////////////////////////////////

} // Message
} // DP

#endif // Include_DPMESSAGE_H

////////////////////////////////////////////////////////////////////////////////
