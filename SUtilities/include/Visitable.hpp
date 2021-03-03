#pragma once
#ifndef _VISITABLE_H
#define _VISITABLE_H
#include "Visitor.hpp"
#include <concepts>
#include <exception>
/**
 * UnknownVisitorPolicy:
 *	- the policy that dictates the behavior when an unknown type is visited by a visitors
 * Usage:
 *  - make any class you wish to be able to visit inherit from BaseVisitable<>
 *  - add the macro MAKE_VISITABLE(ReturnType) to the public section of the class definition
 *  - for visitors that do not mutate, inherit from Vistitor with the const modifier applied to the class to visit
 *  - for visitables that can accept const visitors, inherit from ImmutableBaseVisitable<> and use the macro MAKE_CONST_VISITABLE
 */
namespace SUtil {
	template<template <typename> typename T, typename ReturnType>
	concept UnknownVisitorPolicy = requires(T<ReturnType> a) {
		{T<ReturnType>::onUnknownVisitor()} -> std::same_as<ReturnType>;
	};

	template<typename ReturnType>
	class DefaultConstructUnknownPolicy {
	public:
		static ReturnType onUnknownVisitor() {
			return ReturnType();
		}
	};

	class UnknownVisitorException : public std::exception {
	public:
		const char* what() const override {
			return "Visitor has visited an unknown type";
		}
	};

	template<typename ReturnType>
	class ExceptionUnknownPolicy {
	public:
		static ReturnType onUnknownVisitor() {
			throw UnknownVisitorException();
		}
	};

	/**
	 * The visitable class can only accept mutable visitors and cannot be declared const
	 */
	template<typename ReturnType>
	class MutableVisitablePolicy {
	public:
		virtual ReturnType accept(BaseVisitor&) = 0;
		virtual ~MutableVisitablePolicy() = default;
	};
	/**
	 * The visitable class can only accept const visitors 
	 */
	template<typename ReturnType>
	class ConstVisitablePolicy {
	public:
		virtual ReturnType accept(BaseVisitor&) const = 0;
		virtual ~ConstVisitablePolicy() = default;
	};
	/**
	 * The visitable class can accept const visitors and mutable visitors provided it isn't declared const
	 */
	template<typename ReturnType>
	class MutableAndConstVisitablePolicy {
	public:
		virtual ReturnType accept(BaseVisitor&) const = 0;
		virtual ReturnType accept(BaseVisitor&) = 0;
		virtual ~MutableAndConstVisitablePolicy() = default;
	};

	/**
	 * The parent class of all types you wish to be able to visit
	 * @param <ReturnType> the return type of the accept function
	 * @param <up> the Unknown visitor Policy that dictates what to do when a visitor visits an object it isn't supposed to
	 * @param <accessPolicy> the mutability level of accepted visitors
	 */
	template<typename ReturnType = void,
		template<typename> typename up = ExceptionUnknownPolicy,
		template<typename> typename accessPolicy = MutableAndConstVisitablePolicy
	>
		requires UnknownVisitorPolicy<up, ReturnType>
	class BaseVisitable : public accessPolicy<ReturnType> {
	public:
		virtual ~BaseVisitable() = default;
	protected:
		template<typename T>
		static ReturnType acceptImpl(T& visited, class BaseVisitor& base) {
			if (auto* v =
				dynamic_cast<VisitorSingle<T, ReturnType>*>(&base)) {
				return v->visit(visited);
			}
			else if (auto* v =
				dynamic_cast<VisitorSingle<std::add_const_t<T>, ReturnType>*>(&base)) {
				// cast should work if the visitor is const but the visited is mutable
				return v->visit(visited);
			}
			return up<ReturnType>::onUnknownVisitor();
		}
	};

	template<typename ReturnType = void,
		template <typename> typename up = ExceptionUnknownPolicy>
	using ImmutableBaseVisitable = BaseVisitable<ReturnType, up, ConstVisitablePolicy>;
#define MAKE_MUTABLE_VISITABLE(ReturnType) \
	virtual ReturnType accept(BaseVisitor& visit) override \
	{ return acceptImpl(*this, visit);}

#define MAKE_CONST_VISITABLE(ReturnType) \
	virtual ReturnType accept(BaseVisitor& visit) const override \
	{ return acceptImpl(*this, visit);}
}
#define MAKE_VISITABLE(ReturnType) \
	MAKE_CONST_VISITABLE(ReturnType) \
	MAKE_MUTABLE_VISITABLE(ReturnType)
#endif
