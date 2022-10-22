/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 Mike McQueen.  All rights reserved.
//
/////////////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef Include_COMMONTYPES_H_
#define Include_COMMONTYPES_H_


template<typename T>
concept underlying_type_int = std::is_same_v<std::underlying_type_t<T>, int>;

constexpr int intValue(underlying_type_int auto val) noexcept {
    return static_cast<int>(val);
}

constexpr bool operator==(underlying_type_int auto lhs, underlying_type_int auto rhs) noexcept {
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}

#endif // Include_COMMONTYPES_H_
