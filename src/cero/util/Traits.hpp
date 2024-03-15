#pragma once

#include <type_traits>

namespace cero {

template<typename...>
constexpr inline bool always_false = false;

template<typename... Ts>
constexpr inline bool are_trivially_destructible = (... && std::is_trivially_destructible_v<Ts>);

template<typename... Ts>
constexpr inline bool are_trivially_copy_constructible = (... && std::is_trivially_copy_constructible_v<Ts>);

template<typename... Ts>
constexpr inline bool are_trivially_copy_assignable = (... && std::is_trivially_copy_assignable_v<Ts>);

template<typename... Ts>
constexpr inline bool are_trivially_move_constructible = (... && std::is_trivially_move_constructible_v<Ts>);

template<typename... Ts>
constexpr inline bool are_trivially_move_assignable = (... && std::is_trivially_move_assignable_v<Ts>);

} // namespace cero
