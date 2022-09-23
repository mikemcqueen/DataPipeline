///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
// UiWindowManager.h
//
///////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_UIWINDOWMANAGER_H
#define Include_UIWINDOWMANAGER_H

///////////////////////////////////////////////////////////////////////////////

namespace Ui::Window {

template<
    class Window_t,
    class Translator_t,
    class Interpreter_t>
class Manager_t {
public:
    Manager_t(
        const Window_t& Window)
        :
        m_Window(Window),
        m_Translator(*this),
        m_Interpreter(*this)
    { }

    Manager_t() = delete;
    Manager_t(const Manager_t&) = delete;
    Manager_t& operator=(const Manager_t&) = delete;

    // Accessors:
    const Window_t&        GetWindow() const       { return m_Window; }

    const Translator_t&    GetTranslator() const   { return m_Translator; }
    Translator_t&          GetTranslator()         { return m_Translator; }

    const Interpreter_t&   GetInterpreter() const  { return m_Interpreter; }
    Interpreter_t&         GetInterpreter()        { return m_Interpreter; }

private:
    const Window_t& m_Window;
    Translator_t    m_Translator;
    Interpreter_t   m_Interpreter;
};

} // Ui::Window

////////////////////////////////////////////////////////////////////////////////

#endif // Include_UIWINDOWMANAGER_H

////////////////////////////////////////////////////////////////////////////////
