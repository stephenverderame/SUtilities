#pragma once
#ifndef _TYPELIST_H
#define _TYPELIST_H
#include <type_traits>
#include <typeinfo>
#include <concepts>
/**
 * Typelist facility
 * Concepts + Types:
 *	TList - the type list concept. Has Value and Next typedefs. Constructed via a paramater pack of types
 *  TListAny - a TList or EmptyType
 *	EmptyType - indicates end of type list
 *	TypeComparator + ComparisonResult - see below
 *	TemplateFunction - callable object with a templated operator() taking no arguments and returning void
 *	TypeInfoFunction - callable object accepting a const std::type_info& and returning void
 * 3 types of operations:
 *	- constexpr functions
 *		any operation that has a non-type result (ie size())
 *		can be used in other constexpr functions
 *		parameters passed via template arguments
 *	- type "functions":	
 *		use a capital letter named struct
 *		the "return value" is "stored" in a type alias called Type
 *		has type alias in the form snake_case_name_t<> for CamelCaseName<>::Type
 *  - type comparators:
 *		has a constexpr ComparisonResult member called Result
 *		"returns" the result of the comparison into their Result member
 *		used for finding the minimum, and sorting types
 */
namespace TL {
	struct EmptyType {};

	/**
	 * Compile time variable length "array" of types
	 */
	template<typename T, typename ... Args>
	struct TypeList;

	template<typename T>
	struct TypeList<T> {
		// paramater packs can be empty
		// specialization of TypeList with 1 size and an empty paramater pack
		using Value = T;
		using Next = EmptyType;
	};

	template<typename T, typename U, typename ... Args>
	struct TypeList<T, U, Args...> {
		// we need more than 2 template parameters to differentiate it from the default
		using Value = T;
		using Next = TypeList<U, Args...>;
	};

	/// a non empty type list
	template<typename T>
	concept TList = requires(T list) {
		typename T::Value;
		typename T::Next;
	};

	/// A Type list or Empty type list concept
	template<typename T>
	concept TListAny = TList<T> || std::is_same_v<T, EmptyType>;

	template<TList list>
	using next_t = typename list::Next;

	template<TList list>
	using value_t = typename list::Value;




	/// Gets the length of the type list
	template<TListAny List>
	constexpr unsigned size() {
		return 1 + size<typename List::Next>();
	}
	/// Specialization for empty list
	template<>
	constexpr unsigned size<EmptyType>() {
		return 0;
	}




	template<TList List, unsigned index>
	struct Get {
		using Type = typename Get<typename List::Next, index - 1>::Type;
	};

	template<TList List>
	struct Get<List, 0> {
		using Type = typename List::Value;
	};

	/// Gets the type from the type list at the given index
	template<TList List, unsigned index>
	using get_t = typename Get<List, index>::Type;




	/**
	 * Functional version of get_t
	 * Gets the std::type_info of the type at the specified index
	 * Requires index < size (enforced by compiler)
	 */
	template<TListAny list>
	constexpr const std::type_info& getInfo(unsigned index) {
		return index == 0 ? typeid(typename list::Value) : getInfo<typename list::Next>(index - 1);
	}
	template<>
	constexpr const std::type_info& getInfo<EmptyType>(unsigned index) {
		return typeid(EmptyType);
	}



	/// Indicator that a type wasn't found
	constexpr unsigned tl_npos = ~0;

	/// Overload for when the end of the list is reached
	template<typename T, typename V> requires std::is_same_v<EmptyType, T>
	constexpr unsigned find(unsigned index = 0) {
		return std::is_same_v<T, V> ? index : tl_npos;
	}
	/**
	 * Gets the index of the first occurance of type V in the list
	 * @param <T> the node of the type list to start searching
	 * @param <V> the type to search for
	 * @param index the index of the node passed to this function
	 * @return index of type <V> or tl_npos
	 */
	template<TList T, typename V>
	constexpr unsigned find(unsigned index = 0) {
		return std::is_same_v<typename T::Value, V> ? index :
			find<typename T::Next, V>(index + 1);
	}

	/**
	 * Determines if the type list contains the type V
	 */
	template<TList T, typename V>
	constexpr bool has() {
		return find<T, V>() != tl_npos;
	}




	template<TListAny List, typename ... Ts>
	struct Push {
		// push the value and the head onto the tail of the list
		// need to push recursively because we don't want to create a TypeList of TypeLists
		using Type = typename Push<typename List::Next, Ts..., typename List::Value>::Type;
	};
	/// Specialization when pushing onto a 1 element type list
	template<typename T, typename ... Ts>
	struct Push<TypeList<T>, Ts...> {
		using Type = TypeList<Ts..., T>;
	};

	template<typename ... Ts>
	struct Push<EmptyType, Ts...> {
		using Type = TypeList<Ts...>;
	};

	/**
	 * Pushes the values onto the front of the type list
	 */
	template<TListAny list, typename ... values>
	using push_t = typename Push<list, values...>::Type;




	template<TList listA, typename ... Ts>
	struct Concat;

	// Concatenates A onto B so that the final list becomes B, A
	template<TList listA, TList listB, typename ... Ts>
	struct Concat<listA, listB, Ts...> {
		// concat A and tail of B, push on head of B and all other "free types"
		using Type = typename Concat<listA, typename listB::Next, Ts..., typename listB::Value>::Type;
	};

	template<TList listA, typename ... Ts>
	struct Concat<listA, EmptyType, Ts...> {
		// second list is now empty, push all other "free types"
		using Type = typename Push<listA, Ts...>::Type;
	};

	/**
	 * Concatenates 2 types lists
	 */
	template<TList listA, TList listB>
	using concat_t = typename Concat<listB, listA>::Type;



	template<TList T, unsigned index>
	struct Erase {
		// pushes the head onto the tail with the specified index erased
		using Type = push_t<typename Erase<typename T::Next, index - 1>::Type, typename T::Value>;
	};

	template<TList T>
	struct Erase<T, 0> {
		// skips over the current node, bc its at the specified index
		using Type = typename T::Next;
	};

	/// Erases the type in the list at the specified index
	template<TList T, unsigned index>
	using erase_t = typename Erase<T, index>::Type;

	
	/**
	 * Erases the type from the list
	 * @param <T> the type list to erase from
	 * @param <V> the value to erase
	 * @param <eraseAll> true to erase all ocurrences, false to erase the first
	 */
	template<TListAny T, typename V, bool eraseAll>
	struct EraseType {
	private:
		template<typename T, typename V, bool found, bool eraseAll>
		struct EraseTypeHelper;

		template<typename T, typename V, bool eraseAll>
		struct EraseTypeHelper<T, V, false, eraseAll> {
			// this node is not the node to erase
			// push the head of the type list onto the body with the specified type erased
			using Type = push_t<typename EraseType<typename T::Next, V, eraseAll>::Type, typename T::Value>;
		};

		template<typename T, typename V>
		struct EraseTypeHelper<T, V, true, false> {
			// this node is the value to erase and we only want to erase the 
			// first occurence so return the rest of the list
			using Type = typename T::Next;
		};
		template<typename T, typename V>
		struct EraseTypeHelper<T, V, true, true> {
			// this node is the value to erase
			// return the rest of the list with the specified type removed
			using Type = typename EraseType<typename T::Next, V, true>::Type;
		};
		template<typename V, bool found, bool eraseAll>
		struct EraseTypeHelper<EmptyType, V, found, eraseAll> {
			// the list is empty
			using Type = EmptyType;
		};
	public:
		using Type = typename 
			EraseTypeHelper<T, V, std::is_same_v<typename T::Value, V>, eraseAll>::Type;
	};

	template<typename V>
	struct EraseType<EmptyType, V, true> {
		// base case for erase all
		using Type = EmptyType;
	};

	/**
	 * Removes the first occurrence of V in the type list
	 */
	template<TList T, typename V>
	using eraseType_t = typename EraseType<T, V, false>::Type;

	/**
	 * Erases all occurences of V in the type list
	 */
	template<TList T, typename V>
	using eraseAllType_t = typename EraseType<T, V, true>::Type;




	template<TListAny list, typename ... Ts>
	struct Reverse;

	template<TList list, typename ... Ts>
	struct Reverse<list, Ts...> {
		// strips the type list elements in reverse order
		using Type = typename Reverse<typename list::Next, typename list::Value, Ts...>::Type;
	};
	template<typename ... Ts>
	struct Reverse<EmptyType, Ts...> {
		using Type = TypeList<Ts...>;
	};

	/**
	 * Reverses the order of the elements in the given list
	 */
	template<TList list>
	using reverse_t = typename Reverse<list>::Type;

	/// A functor that can be instantiated with EmptyType and type V
	template<typename T, typename V>
	concept TemplateFunction = requires(T a, V b) {
		{a.template operator()<V>()} -> std::same_as<void>;
		{a.template operator()<EmptyType>() } -> std::same_as<void>;
	};

	/// A Functor that accepts a type_info
	template<typename T>
	concept TypeInfoFunction = requires(T a) {
		{a(typeid(int))} -> std::same_as<void>;
	};


	template<typename T, TemplateFunction<EmptyType> function> requires std::is_same_v<T, EmptyType>
	constexpr void for_each(function&& f) {}

	/**
	* Iterates through all types in the type list and calls the templated operator() of the function
	*/
	template<TList list, TemplateFunction<typename list::Value> function>
	constexpr void for_each(function&& f) {
		f.template operator() < typename list::Value > ();
		for_each<typename list::Next>(std::forward<function>(f));
	}

	template<typename T, TypeInfoFunction function> requires std::is_same_v<T, EmptyType>
	constexpr void for_each(function&& f) {}

	/**
	 * Iterates through all types in the type list passing the type_info to the specified callable object
	 */
	template<TList list, TypeInfoFunction function> 
	constexpr void for_each(function&& f) {
		f(typeid(typename list::Value));
		for_each<typename list::Next>(std::forward<function>(f));
	}

	/**
	 * Increments a counter each time operator()<T>() is called
	 * @param <T> the specified type to increment the counter for when it's passed to operator()
	 */
	template<typename T>
	class CountTypes {
	private:
		unsigned count = 0;
	public:
		constexpr unsigned getCount() { return count; }

		template<typename V>
		constexpr void operator()() { }

		template<>
		constexpr void operator() < T > () { ++count; }
	};
	/**
	 * Gets the amount of times T occurs in the list
	 */
	template<TList list, typename T>
	constexpr unsigned countTypes() {
		CountTypes<T> count;
		for_each<list>(count);
		return count.getCount();
	}

	/**
	 * Replaces T with R in list
	 * @param <replaceAll> true to replace all T's with R, false to replace the first ocurrence
	 */
	template<TListAny list, typename T, typename R, bool replaceAll>
	struct Replace {
	private:
		// don't replace this node
		template<bool, bool continueReplacing>
		struct ReplaceHelper {
			using Type = push_t<
				typename Replace<typename list::Next, T, R, continueReplacing>::Type,
				typename list::Value
			>;
		};

		// replace this node and don't replace all occurences
		template<>
		struct ReplaceHelper<true, false> {
			using Type = push_t<typename list::Next, R>;
		};
		// replace this node and replace all occurences
		template<>
		struct ReplaceHelper<true, true> {
			using Type = push_t<
				typename Replace<typename list::Next, T, R, true>::Type, 
				R
			>;
		};
	public:
		using Type = typename ReplaceHelper<std::is_same_v<typename list::Value, T>, replaceAll>::Type;
	};

	template<typename T, typename R, bool replaceAll>
	struct Replace<EmptyType, T, R, replaceAll> {
		using Type = EmptyType;
	};

	/**
	 * Replaces the first occurrence of Target with Replacement in list
	 */
	template<TList list, typename Target, typename Replacement>
	using replace_t = typename Replace<list, Target, Replacement, false>::Type;

	/**
	 * Replaces all occurrences of Target with Replacement in list
	 */
	template<TList list, typename Target, typename Replacement>
	using replaceAll_t = typename Replace<list, Target, Replacement, true>::Type;

	/**
	 * Resultant type of type comparison
	 * Returned by comparators where less indicates the first type should go first,
	 * greater indicates the first type should go after the second and equal indicating they are equal
	 */
	enum class ComparisonResult {
		less,
		equal,
		greater
	};

	template<typename T>
	concept TypeComparator = requires(T a) {
		{T::Result} -> std::same_as<const ComparisonResult&>;
		// has a constant member variable named Result of type ComparisonResult
	};

	/**
	 * Gets the TrueType if expr is true, otherwise the FalseType
	 */
	template<bool expr, typename TrueType, typename FalseType>
	struct IfThen {
		using Type = FalseType;
	};
	template<typename TrueType, typename FalseType>
	struct IfThen<true, TrueType, FalseType> {
		using Type = TrueType;
	};

	/**
	 * Gets the type that is ordered first in Type
	 * Stores the other type in Other
	 */
	template<typename T, typename U, template <typename, typename> class cmp> 
		requires TypeComparator<cmp<T, U>>
	struct Less {
	private:
		template<ComparisonResult res>
		struct LessHelper {
			using Type = T;
		};

		template<>
		struct LessHelper<ComparisonResult::greater> {
			using Type = U;
		};
	public:
		using Type = typename LessHelper<cmp<T, U>::Result>::Type;
		using Other = typename IfThen<std::is_same_v<Type, T>, U, T>::Type;
	};

	/**
	 * Gets the minimum type from the list based on a comparator
	 */
	template<TListAny list, typename CurrentMin, template <typename, typename> typename comp>
	struct Min {
	private:
		using min = typename Less<typename list::Value, CurrentMin, comp>::Type;
	public:
		using Type = typename Min<typename list::Next, min, comp>::Type;
	};

	template<typename CurrentMin, template <typename, typename> typename comp>
	struct Min<EmptyType, CurrentMin, comp> {
		using Type = CurrentMin;
	};

	/**
	 * Places type T into the correct place in list based on comp
	 */
	template<TListAny list, typename T, template <typename, typename> typename comp>
	struct OrderedInsert {
	private:
		// the body of the current list, ordered
		using prev = typename OrderedInsert<typename list::Next, typename list::Value, comp>::Type;

		// new value is less than current head, push it on the rest of the list
		template<ComparisonResult>
		struct InsertHelper {
			using Type = push_t<prev, T>;
		};

		// new value is greater than list head, insert it into proper place in the list
		template<>
		struct InsertHelper<ComparisonResult::greater> {
			using Type = typename Push<
				typename OrderedInsert<typename prev::Next, T, comp>::Type, 
				typename prev::Value
			>::Type;
		};
	public:
		using Type = typename InsertHelper<comp<T, typename prev::Value>::Result>::Type;
	};

	template<typename T, template <typename, typename> typename comp>
	struct OrderedInsert<EmptyType, T, comp> {
		using Type = TypeList<T>;
	};

	/**
	 * Orders a list based on a comparator
	 */
	template<TList list, template <typename, typename> typename comp>
	using order_t = typename OrderedInsert<typename list::Next, typename list::Value, comp>::Type;


	/**
	 * Comparator which brings smallest types to the front
	 */
	template<typename T, typename U>
	struct SizeSort {
	private:
		constexpr static ComparisonResult doCmp(int difference) {
			if (difference < 0)
				return ComparisonResult::less;
			else if (difference == 0)
				return ComparisonResult::equal;
			else
				return ComparisonResult::greater;
		}
	public:
		constexpr static ComparisonResult Result = doCmp(static_cast<int>(sizeof(T)) - 
			static_cast<int>(sizeof(U)));

	};

	/**
	 * Comparator which brings most derived classes to the front
	 */
	template<typename T, typename U>
	struct DerivedSort {
	private:
		constexpr static ComparisonResult doCmp() {
			if (std::is_base_of_v<U, T>)
				return ComparisonResult::less;
			else if(std::is_base_of_v<T, U>)
				return ComparisonResult::greater;
			else
				return ComparisonResult::equal;
		}
	public:
		constexpr static ComparisonResult Result = doCmp();

	};


}
#endif
