#pragma once
#ifndef _CAST_H
#define _CAST_H
#include <typeinfo>
#include <type_traits>
#include <concepts>
namespace SUtil {
	template<typename R, typename T>
	concept StaticCastableTo = requires(T a) {
		static_cast<R>(a);
	};

	/**
	 * Safely casts <T> to <R> if value can fit inside <R>
	 */
	template<typename R, StaticCastableTo<R> T>
		requires StaticCastableTo<T, R>
	R narrow_cast(T value) {
		if (static_cast<T>(static_cast<R>(value)) != value)
			throw std::bad_cast();
		return static_cast<R>(value);
	}

	template<typename R, typename T>
		requires (std::is_arithmetic_v<R> && (std::is_arithmetic_v<T> || std::is_enum_v<T>))
	bool isSignMismatch(T val) {
		return val < static_cast<T>(0) && !std::is_signed_v<R>;
	}
	template<typename R, typename T>
		requires (std::is_enum_v<R> && (std::is_arithmetic_v<T> || std::is_enum_v<T>))
	bool isSignMismatch(T val) {
		return (val < static_cast<T>(0) && !std::is_signed_v<std::underlying_type_t<R>>);
	}
	template<typename R, typename T>
	bool isSignMismatch(T val) {
		return false;
	}

	/**
	 * Safely casts <T> to <R> if value can fit inside <R>
	 * Fails if value is < 0 and R is unsigned
	 */
	template<typename R, StaticCastableTo<R> T>
		requires StaticCastableTo<T, R>
	R strict_narrow_cast(T value) {
		if (isSignMismatch<R>(value)) throw std::bad_cast();
		return narrow_cast<R>(value);
	}
}
#endif
