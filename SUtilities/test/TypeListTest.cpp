#include <TypeList.hpp>
#include <gtest/gtest.h>
#include <sstream>

using namespace TL;

using list = TL::TypeList<int, char, short, long, long long>;
using list2 = TL::TypeList<int, short>;
using list3 = TL::TypeList<int, short, long>; 

TEST(TypeListTest, sizeTest) {

	static_assert(TL::size<TL::EmptyType>() == 0);
	static_assert(TL::size<TL::TypeList<char>>() == 1);
	static_assert(TL::size<list>() == 5);
	static_assert(TL::size<list2>() == 2);
	static_assert(TL::size<list3>() == 3);
	static_assert(size<TL::TypeList<TL::TypeList<int>>>() == 1);
	static_assert(size<next_t<list>>() == size<list>() - 1);
	static_assert(size<TypeList<int, void, char*, const char*, int, int, EmptyType>>() == 7);
	static_assert(size<TypeList<EmptyType, int, EmptyType, char>>() == 4);
	ASSERT_TRUE(true);
}

TEST(TypeListTest, getTest) {
	static_assert(sizeof(TL::get_t<list, 4>) == sizeof(long long));
	static_assert(sizeof(TL::get_t<list, 3>) == sizeof(long));
	static_assert(sizeof(TL::get_t<list, 2>) == sizeof(short));
	static_assert(sizeof(TL::get_t<list, 1>) == sizeof(char));
	static_assert(sizeof(TL::get_t<list, 0>) == sizeof(int));

	// this may be a false failure on some compilers
	ASSERT_EQ(typeid(get_t<list, 4>), typeid(long long)); 
	ASSERT_EQ(typeid(get_t<list2, 0>), typeid(int));
	ASSERT_EQ(typeid(get_t<list2, 1>), typeid(short));
	ASSERT_EQ(typeid(get_t<list3, 2>), typeid(long));
}

TEST(TypeListTest, findTest) {
	static_assert(TL::has<list, int>());
	static_assert(TL::has<list2, int>());
	static_assert(TL::has<list3, short>());
	static_assert(TL::has<list2, char*>() == false);
	static_assert(TL::has<list, void*>() == false);
	static_assert(TL::find<list, char>() == 1);
	static_assert(TL::find<list, long long>() == 4);
	static_assert(TL::find<list, int>() == 0);
	static_assert(find<list3, short>() == 1);
	static_assert(find<list3, int>() == 0);
	ASSERT_TRUE(true);
}

TEST(TypeListTest, eraseTest) {
	static_assert(sizeof(TL::get_t<TL::eraseType_t<list, int>, 0>) == sizeof(char));
	using et = TL::eraseType_t<list, long>;
	using erased = TL::erase_t<list, 3>;
	static_assert(TL::has<erased, long>() == false);
	static_assert(TL::has<erased, long long>() && TL::has<erased, int>() && TL::has<erased, char>());
	static_assert(TL::find<erased, long long>() == 3 &&
		TL::find<erased, int>() == 0 && TL::find<erased, short>() == 2);
	static_assert(TL::has<et, long>() == false);
	static_assert(TL::has<et, long long>() && TL::has<et, int>() && TL::has<et, char>());
	static_assert(TL::find<et, long long>() == 3 &&
		TL::find<et, int>() == 0 && TL::find<et, short>() == 2);
	using test = TL::concat_t<TL::TypeList<char, short>, TL::TypeList<short, int, long>>;
	static_assert(TL::size<test>() == 5);
	static_assert(TL::find<test, int>() == 3);
	static_assert(TL::find<test, void*>() == TL::tl_npos);
	using testErased = TL::eraseAllType_t<test, short>;
	static_assert(TL::size<testErased>() == 3);
	ASSERT_TRUE(true);
}

TEST(TypeListTest, countReplaceTest) {
	using test = TL::concat_t<TL::TypeList<char, short>, TL::TypeList<short, int, long>>;
	static_assert(TL::countTypes<TL::TypeList<int, short, short, char*, long long, short, short>, short>() == 4);
	static_assert(TL::countTypes<test, short>() == 2);


	using testR = TL::replace_t<TL::TypeList<int>, int, char>;
	static_assert(TL::has<testR, int>() == false);
	using listReplaced = TL::replace_t<list, long long, char*>;
	static_assert(TL::find<listReplaced, char*>() == 4);
	static_assert(TL::has<listReplaced, long long>() == false);
	using replaceTest = TL::replaceAll_t<test, short, char*>;
	static_assert(TL::has<replaceTest, short>() == false);
	static_assert(TL::size<replaceTest>() == TL::size<test>());
	static_assert(TL::countTypes<replaceTest, char*>() == 2);
	const auto sort = TL::SizeSort<int, long long>::Result;
	static_assert(TL::TypeComparator<TL::SizeSort<int, long>>);
	ASSERT_TRUE(true);
}

TEST(TypeListTest, sortTest) {
	struct big {
		char t[12];
	};

	struct Base {};

	struct Derived : public Base {};
	struct MoreDerived : public Derived {};
	struct MoreDerived2 : public Derived {};
	struct MoreDerived3 : public MoreDerived2 {};


	using min = typename TL::Min<list, TL::get_t<list, 0>, TL::SizeSort>::Type;
	static_assert(sizeof(min) == 1);
	ASSERT_EQ(typeid(min), typeid(char));
	using min2 = typename TL::Min<list3, TL::get_t<list3, 0>, TL::SizeSort>::Type;
	static_assert(sizeof(min2) == 2);
	ASSERT_EQ(typeid(min2), typeid(short));

	using ord = TL::order_t<list, TL::SizeSort>;
	using ins = TL::OrderedInsert<list, big, TL::SizeSort>::Type;
	static_assert(sizeof(TL::get_t<ins, 0>) == 1);
	static_assert(sizeof(TL::get_t<ins, 5>) == sizeof(big));
	static_assert(sizeof(TL::get_t<ins, 4>) == sizeof(long long));
	static_assert(sizeof(TL::get_t<ins, 1>) == 2);

	using hList = TL::TypeList<Base, MoreDerived3, Derived, MoreDerived2, MoreDerived3, MoreDerived>;
	using hOrd = TL::order_t<hList, TL::DerivedSort>;
	ASSERT_EQ(typeid(TL::get_t<hOrd, 5>), typeid(Base));
	static_assert(!std::is_base_of_v<get_t<hOrd, 1>, get_t<hOrd, 2>>);
	static_assert(!std::is_base_of_v<get_t<hOrd, 2>, get_t<hOrd, 3>>);
	static_assert(!std::is_base_of_v<get_t<hOrd, 3>, get_t<hOrd, 4>>);
}
/// Warning: EXTREMELY Platform + Compiler DEPENDENT
TEST(TypeListTest, iterationTest) {
	std::stringstream ss;
	for_each<list>([&ss](const std::type_info& info) {
		ss << info.name() << " ";
	});
	ASSERT_EQ(ss.str(), "int char short long __int64 ");
	using rev = reverse_t<list>;
	ss.str("");
	for_each<rev>([&ss](const std::type_info& info) {
		ss << info.name() << " ";
	});
	ASSERT_EQ(ss.str(), "__int64 long short char int ");
}

int main(int argc, char** argv) {
	std::cout << "Any failure in the TypeListTest case (with the exception of compilation fails) "
		<< "\n should be further inspected as some test cases are compiler + platform dependent" << std::endl;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}