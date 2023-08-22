// SUtilities.cpp : Defines the entry point for the application.
#include "SUtilities.h"
#include "include/TypeList.hpp"
#include "include/Visitor.hpp"
#include "include/Visitable.hpp"
#include <functional>
#include <array>

using namespace std;

struct Idk {
	void doIt() const {
		printf("Done\n");
	}
	Idk() = default;
	Idk(const Idk&) {
		printf("Copy!\n");
	}
	Idk(Idk&&) {
		printf("Move\n");
	}
	Idk& operator=(const Idk&) {
		printf("Copy! =\n");
		return *this;
	}
	Idk& operator=(Idk&&) {
		printf("Move =\n");
	}
};

void call(Idk& i) {
	i.doIt();
}
Idk get() {
	Idk i;
	return i;
}

void use(Idk&& o) {
	o.doIt();
}
void use2(const Idk& o) {
	o.doIt();
}
struct MyClass {
	char b[4];
	MyClass() {
		std::cout << "Class ctor\n";
	}
	MyClass(const MyClass&) {
		std::cout << "Class cpy\n";
	}
	MyClass(MyClass&&) {
		std::cout << "Class mv\n";
	}
	~MyClass() {
		std::cout << "Class dtor\n";
	}
};
struct CDeleter {
	void operator()(MyClass* res) {
		free(res);
	}
};
template<typename T>
void foo(const T& t) {

}
struct Swapper {
	friend void swap(Swapper& sw, Swapper& s2);
	Swapper() {};
	Swapper(Swapper&& o) {
		swap(*this, o);
	}
};
template<typename T>
void barf(T t) {
	std::cout << "Normal";
}
// overload
void barf(int num) {
	std::cout << "Int";
}

template<>
void barf<char>(char c) {
	std::cout << "CHAR";
}
template<unsigned int i>
struct Fibonacci {
	enum {
		value = Fibonacci<i - 1>::value +
		Fibonacci<i - 2>::value
	};
};

template<>
struct Fibonacci<0> {
	enum { value = 0 };
};
template<>
struct Fibonacci<1> {
	enum { value = 1 };
};
constexpr unsigned int fibo(unsigned int i) {
	if (i <= 1)
		return i;
	return fibo(i - 1) + fibo(i - 2);
}
template<typename T>
auto load(T&& t) {
	if constexpr (std::is_pointer_v<std::remove_reference_t<T>>)
		return *t;
	else
		return std::forward<T>(t);
}
struct Hide {
	virtual void foo(int) {
		printf("Base\n!");
	};
};
struct N : private Hide {
	void foo(int a) {
		printf("Int\n");
	};
	void foo(char c) {
		printf("Char\n");
	};
};
class A {
public:
	int aVar;
public:
	int doA() {
		return aVar;
	}
};
class B : public A {};
class C : public A {};
class D : public B, public C {};

template<typename ... T>
auto sum(T&& ... args) {
	return (... + args);
}

auto getTup() {
	MyClass m;
	m.b[2] = 10;
	return std::forward_as_tuple('c', 5);
}
constexpr auto argCount() {
	return 0;
}

template<typename T, typename ... Args>
constexpr auto argCount(T&&, Args&& ... args) {
	return 1 + argCount(std::forward<Args>(args)...);
}

template<unsigned index, typename Head, typename ... List>
struct NthType {
	using Type = typename NthType<index - 1, List...>::Type;
};

template<typename Head, typename ... List>
struct NthType<0, Head, List...> {
	using Type = Head;
};
struct Stringy {
	virtual ~Stringy() = default;
	virtual const char* data() = 0;
	virtual size_t size() = 0;
};

class AnyString : public Stringy {
private:
	template<typename T>
	struct StringHolder : public Stringy {
		T str;

		template<typename T>
		StringHolder(T&& str) : str(std::forward<T>(str)) {}

		const char* data() override {
			return str.data();
		}
		size_t size() override {
			return str.size();
		}
	};

	template<>
	struct StringHolder<const char*> : public Stringy {
		const char* str;

		StringHolder(const char* str) : str(str) {}

		const char* data() override {
			return str;
		}
		size_t size() override {
			return strlen(str);
		}
	};

	std::shared_ptr<Stringy> stringy;
public:
	template<typename T>
	AnyString(T&& t) 
		: stringy(std::make_shared<
			StringHolder<std::remove_reference_t<T>>>(std::forward<T>(t))) {}

	const char* data() override {
		return stringy->data();
	}
	size_t size() override {
		return stringy->size();
	}

};
template<typename T>
concept Any = true;
int main()
{
	{
		MyClass* mc = (MyClass*)malloc(sizeof(MyClass));
		auto ptr = std::unique_ptr<MyClass, CDeleter>(mc, CDeleter());
	}
//	Idk i;
	use(get());
	use(Idk{});
//	use(std::move(i));
	use2(get());
	Swapper s, w;
	swap(s, w);
	foo(5);
	barf(5.0);
	barf(5);
	barf('c');
	Fibonacci<6>::value;
	static_assert(fibo(6) == 8);
	auto& f = typeid(Swapper);

	std::cout << std::endl;
	int foo = 5;
	int* f2 = &foo;
	std::cout << load(foo);
	std::cout << std::endl;
	std::cout << load(f2);

	D d;
	d.B::aVar = 0;

	std::cout << sum(10, 40, 23.0, 10, 574) << std::endl;
	char arr[2] = { 5, 3 };
	auto [id1, id2] = arr;

	auto t = std::tie(id1, id2);
	auto t2 = std::forward_as_tuple(id1, 5);
	auto [mc, num] = getTup();

	constexpr auto c = argCount(5);
	static_assert(c == 1);

	using tt = typename NthType<3, int, char, void, char*>::Type;

	std::array v = { 54, 32, 10 };

	auto [num1, num2, num3] = v;
	std::cout << num2 << std::endl;

	std::vector<AnyString> strings;
	std::vector chars = { 'h', 'e', 'l', 'l', 'o' };
	std::string str = "hello world";
	std::string_view sv = { chars.begin() + 1, chars.end() };
	const char* cstr = "Hello there";
	strings.emplace_back(chars);
	strings.emplace_back(str);
	strings.emplace_back(sv);
	strings.emplace_back(cstr);
	

	std::string_view sv2 = { strings[0].data(), strings[0].size() };
	std::cout << sv2 << std::endl;

	auto r = SUtil::narrow_cast<int>(5);
	

}

void swap(Swapper& sw, Swapper& s2)
{
	printf("Friend swap\n");
}
