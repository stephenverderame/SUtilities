#pragma once
#ifndef _SINGLETON_H
#define _SINGLETON_H
#include <exception>
#include <functional>
#include <mutex>
#include <compare>
#include <algorithm>
/**
 * Singleton Utility:
 * Policies:
 *	-	DeadReferencePolicy - what to do when you access a singleton after it has been destroyed
 *		- ReviveDeadRefPolicy - recreate itself
 *		- ErrorDeadRefPolicy - error
 *	-	DestructionPolicy - how to schedule the destruction of the singleton
 *		- Standard: use c++ automatic cleanup semantics
 *		- Lifetime: manually set a numeric lifetime number. Singletons with larger lifetimes are destroyed later
 *	-	LockingPolicy - how to lock the singleton (not the actual object in the singleton)
 *		- LockGuard - use a lock
 *		- NoLock - don't use a lock
 *	-	CreatePolicy - how to delete/create the object in the singleton
 *		- FreeStore - use the free store (C++ new/delete)
 */
namespace SUtil {
	template<typename T>
	concept SingletonDeadReferencePolicy = requires(T a) {
		T::onDeadReference();
	};

	template<template <typename> typename T, typename R>
	concept SingletonCreatePolicy = requires(T<R> a) {
		{T<R>::create()} -> std::same_as<R*>;
		{T<R>::free(std::declval<R*>())} noexcept;
	};
	/**
	 * Requires the parameter passed to scheduleDestruction be a function pointer that does not throw
	 */
	template<typename T>
	concept SingletonDestructionPolicy = requires(T a) {
		{T::scheduleDestruction(std::declval<void(*)()>())};
	};

	template<typename T>
	concept SingletonLockPolicy = requires(T a) {
		T::lockSingleton();
	};

	class DeadReferenceException : public std::exception {
	public:
		const char* what() const noexcept override {
			return "A previously destroyed singleton has been accessed";
		}
	};
	struct ErrorDeadRefPolicy {
		static void onDeadReference() {
			throw DeadReferenceException();
		}
	};
	struct ReviveDeadRefPolicy {
		static void onDeadReference() {
			//will fall through to the creation code
		}
	};

	struct StandardDestructionPolicy {
		static void scheduleDestruction(void(*dtor)(void)) noexcept {
			std::atexit(dtor);
		}
	};
	/**
	 * Not for external use
	 */
	namespace SingletonLongevityTracker {
		// We have to use C memory allocation since order of destruction of static/globals is not defined
		// C heap will still work however
		struct SingletonLife {
			/// Invariant: destroyer does not throw
			void(*destroyer)(void); 
			unsigned longevity;
		};
		auto operator<=>(const SingletonLife& s1, const SingletonLife& s2) {
			return s1.longevity <=> s2.longevity;
		}
		/// array of SingletonLife* ordered highest to shortest longevity
		inline SingletonLife** lifetimeManager = nullptr;
		/// number of elements in lifetimeManager
		inline int lifetimeElements = 0;

		/**
		 * Pops the last element in the array
		 */
		inline void popLifetimeArray() noexcept {
			auto last = lifetimeManager[lifetimeElements - 1];
			if (--lifetimeElements > 0) {
				lifetimeManager = static_cast<SingletonLife**>(
					realloc(lifetimeManager, sizeof(SingletonLife*) * (lifetimeElements)));
				// this should not fail because lifetimeElements is less than it was
			}
			else {
				free(lifetimeManager);
				// realloc is not guartunteed to act like free when size is 0, although it does on many implementations
			}
			last->destroyer();
			free(last);
		}
		/**
		 * @return the position the element with the specified longevity should go
		 */
		inline auto posForLongevity(unsigned longevity) {
			auto i = 0;
			for (; i < lifetimeElements; ++i) {
				if (lifetimeManager[i]->longevity <= longevity)
					break;
			}
			return i;
		}
		/**
		 * Creates space in the lifetimeManager array for an element at posToClear
		 * Shifts all elements after the given position to the right
		 * requires the last element to be empty and there to be lifeElements + 1 spaces in the array
		 */
		inline void shiftLifetimeElements(int posToClear) {
			if (posToClear < lifetimeElements) { // if not the last element (nothing to shift)
				for (auto i = lifetimeElements; i >= posToClear; --i) {
					lifetimeManager[i + 1] = lifetimeManager[i];
				}
			}
		}
		/**
		 * Adds the singleton life manager to the array and cleans up on error
		 */
		inline void safeAddManagerToArray(SingletonLife* manager) {
			int pos = 0;
			try {
				int p = manager->longevity;
				pos = posForLongevity(manager->longevity);
				shiftLifetimeElements(pos);
			}
			catch (const std::exception& e) {
				free(manager);
				throw e;
			}
			++lifetimeElements;
			lifetimeManager[pos] = manager;
			atexit(&popLifetimeArray);
		}
		inline void _scheduleDestruction(const SingletonLife & singleton) {
			auto newArray = 
				static_cast<SingletonLife**>(
					realloc(lifetimeManager, sizeof(SingletonLife*) * (lifetimeElements + 1)));
			// not incrementing lifetimeElements yet so that if this call fails, retain the strong guaruntee
			if (newArray == nullptr) throw std::bad_alloc();
			else lifetimeManager = newArray;

			auto newLifeManager = static_cast<SingletonLife*>(malloc(sizeof(SingletonLife)));
			newLifeManager->destroyer = singleton.destroyer;
			newLifeManager->longevity = singleton.longevity;
			safeAddManagerToArray(newLifeManager);
			
		}
		inline void scheduleDestruction(SingletonLife singleton) {
			try {
				_scheduleDestruction(singleton);
			}
			catch (...) {
				singleton.destroyer();
			}
		}
	}

	template<unsigned longevity>
	struct LongevityDestructionPolicy {
		static void scheduleDestruction(void(*destroyer)(void)) {
			SingletonLongevityTracker::scheduleDestruction({ destroyer, longevity });
		}
	};

	template<typename T>
	struct FreeStoreCreatePolicy {
		static T* create() {
			return new T();
		}
		static void free(T* instance) noexcept {
			delete instance;
		}
	};

	template<typename T>
	struct SingletonLockGuard {
		static std::mutex mu;
		static std::lock_guard<std::mutex> lockSingleton() {
			return { mu };
		}
	};

	template<typename T>
	struct SingletonNoLock {
		static int lockSingleton() { return 0; };
	};

	template<typename T,
		SingletonDeadReferencePolicy deathPolicy = ErrorDeadRefPolicy,
		template <typename> typename createPolicy = FreeStoreCreatePolicy,
		SingletonDestructionPolicy destructionPolicy = StandardDestructionPolicy,
		template<typename> typename coarseLockingPolicy = SingletonNoLock>
	requires SingletonCreatePolicy<createPolicy, T> &&
		SingletonLockPolicy<coarseLockingPolicy<T>>
	class Singleton {
	public:
		Singleton() {
			isLive = true;
		}
		~Singleton() {
			isLive = false;
		}
	private:
		static inline T* instance = nullptr;
		static inline bool isLive = false;
	public:
		static T& get() {
			if (!instance) {
				auto lk = coarseLockingPolicy<T>::lockSingleton();
				initializeSingleton();
			}
			return *instance;
		}
	private:
		static void onDestroy() noexcept {
			isLive = false;
			createPolicy<T>::free(instance);
		}
		static void initializeSingleton() {
			if (!instance) {
				if (!isLive) {
					deathPolicy::onDeadReference();
				}
				instance = createPolicy<T>::create();
				destructionPolicy::scheduleDestruction(&onDestroy);
				isLive = true;
			}
		}
	};

	template<typename T>
	using PheonixSingleton_t = Singleton<T, ReviveDeadRefPolicy,
		FreeStoreCreatePolicy, StandardDestructionPolicy,
		SingletonNoLock>;

	template<typename T>
	using MTSingleton_t = Singleton<T, ErrorDeadRefPolicy,
		FreeStoreCreatePolicy, StandardDestructionPolicy,
		SingletonLockGuard>;

	template<typename T>
	using MTPheonixSingleton_t = Singleton<T, ReviveDeadRefPolicy,
		FreeStoreCreatePolicy, StandardDestructionPolicy,
		SingletonLockGuard>;

	template<typename T, unsigned longevity>
	using LifetimeSingleton_t = Singleton<T, ReviveDeadRefPolicy,
		FreeStoreCreatePolicy, LongevityDestructionPolicy<longevity>,
		SingletonNoLock>;

	template<typename T, unsigned longevity>
	using MTLifetimeSingleton_t = Singleton<T, ReviveDeadRefPolicy,
		FreeStoreCreatePolicy, LongevityDestructionPolicy<longevity>,
		SingletonLockGuard>;
}
#endif
