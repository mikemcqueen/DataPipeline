///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// LonWindowManager_t.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_LONWINDOWMANAGER_T_H
#define Include_LONWINDOWMANAGER_T_H

///////////////////////////////////////////////////////////////////////////////

namespace Lon
{

template<
    class Translator_t,
    class Interpreter_t>
class WindowManager_t
{

private:

    Translator_t  m_Translator;
    Interpreter_t m_Interpreter;

public:

    WindowManager_t() :
        m_Interpreter(*this)
    { }

    const Translator_t&    GetTranslator() const   { return m_Translator; }
    Translator_t&          GetTranslator()         { return m_Translator; }

    const Interpreter_t&   GetInterpreter() const  { return m_Interpreter; }
    Interpreter_t&         GetInterpreter()        { return m_Interpreter; }

private:

    WindowManager_t(const WindowManager_t&) = delete;
    WindowManager_t& operator=(const WindowManager_t&) = delete;
};

} // Lon

///////////////////////////////////////////////////////////////////////////////

#endif // Include_LONWINDOWMANAGER_T_H

///////////////////////////////////////////////////////////////////////////////
