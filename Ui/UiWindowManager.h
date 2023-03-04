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

#include "UiWindow.h"

namespace Ui::Window {
  template<typename T>
  concept UiWindow = std::derived_from<T, Base_t>;

  template<UiWindow windowT, class Translator_t, class Interpreter_t>
  class Manager_t {
  public:
    Manager_t(const Base_t& window) :
      m_Window(window),
      m_Translator(GetWindow()), // *this),
      m_Interpreter(GetWindow()) // *this)
    { }

    Manager_t() = delete;
    Manager_t(const Manager_t&) = delete;
    Manager_t& operator=(const Manager_t&) = delete;

    // Accessors:
    constexpr const windowT& GetWindow() const {
      return static_cast<const windowT&>(m_Window);
    }

    constexpr Translator_t& GetTranslator() const { return m_Translator; }
    Translator_t& GetTranslator() { return m_Translator; }

    constexpr Interpreter_t& GetInterpreter() const { return m_Interpreter; }
    Interpreter_t& GetInterpreter() { return m_Interpreter; }

  private:
    const Base_t& m_Window;
    Translator_t m_Translator;
    Interpreter_t m_Interpreter;
  };
} // Ui::Window

#endif // Include_UIWINDOWMANAGER_H
