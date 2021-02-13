#pragma once
#ifndef _VISITOR_H
#define _VISITOR_H
/**
 * Acyclic visitor
 * Usage:
 *	- Make your visitor class subtype Visitor<ReturnType, Ts...>
 *  - implement the necessary overrides for each type you wish to visit
 */
namespace SUtil {
	class BaseVisitor {
	public:
		virtual ~BaseVisitor() = default;
	};

	template<typename T, typename ReturnType>
	class VisitorSingle {
	public:
		virtual ReturnType visit(T&) = 0;
	};

	/**
	 * Base class for visitors
	 * @param <ReturnType> the return type of the visit() function
	 * @param <Ts> the types that you would like to be able to visit
	 */
	template<typename ReturnType, typename ... Ts>
	class Visitor : public BaseVisitor, public VisitorSingle<Ts, ReturnType>... {
	};

	template<typename ... Ts>
	using Visitor_v = Visitor<void, Ts...>;
}
#endif
