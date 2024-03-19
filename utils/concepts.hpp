#pragma once

#include <bit>
#include <concepts>
#include <type_traits>

// template <typename T>
// struct is_byte_type : std::disjunction<
//     std::is_same<T, byte>,
//     std::is_same<T, int8_t>,
//     std::is_same<T, uint8_t>,
//     std::is_same<T, unsigned char>,
//     std::is_same<T, char>
// > {};

template <typename T>
concept is_byte_type = std::is_same_v<T, std::byte> 
|| std::is_same_v<T, int8_t> 
|| std::is_same_v<T, uint8_t> 
|| std::is_same_v<T, char> 
|| std::is_same_v<T, unsigned char>;

template <typename T>
concept ByteType = requires { is_byte_type<std::remove_cv_t<T>>; };
