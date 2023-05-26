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

template<size_t N>
auto array_equal(const std::array<char, N>& array, std::string_view sv) {
  return std::strncmp(array.data(), sv.data(), array.size()) == 0;
}

template<size_t N>
auto array_equal(const std::array<char, N>& array1, std::array<char, N> array2) {
  return std::strncmp(array1.data(), array2.data(), array1.size()) == 0;
}

#endif // Include_COMMONTYPES_H_
