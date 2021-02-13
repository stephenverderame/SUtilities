#include <stdio.h>
#include <sstream>
#include <gtest/gtest.h>
struct DeinitChecker {
	inline static std::vector<unsigned> order;
	static void assertInOrder() {
		if (order.size() > 1) {
			for (int i = 0u; i < order.size() - 1; ++i) {
				if (order[i] > order[i + 1u])
					throw std::runtime_error("Singletons not in order!");
				// can't use gtest because that is probably deinitializes at this time
				std::cout << order[i] << "\n";
			}
		}
	}
	~DeinitChecker() {
		assertInOrder();
	}
};
static DeinitChecker lifetimeChecker;
#include <Singleton.hpp>

using namespace SUtil;
std::stringstream ss;
struct Logger {
	Logger() {

	}
	void log(const char* msg) {
		ss << msg << "\n";
	}
};


TEST(SingletonTest, basicTest) {
	Singleton<Logger> logger;
	struct SingletonUser {
		Logger& logger;
		SingletonUser(Logger& logger) : logger(logger) {
			logger.log("Hello there");
		}
		~SingletonUser() {
			logger.log("Goodbye");
		}
	};
	static SingletonUser user(logger.get());
//	EXPECT_THROW(user.~SingletonUser(), DeadReferenceException);
	user.~SingletonUser();
	ASSERT_EQ(ss.str(), "Hello there\nGoodbye\n");
}
template<unsigned i>
struct S {
	S() {
		std::cout << "Hello " << i << "\n";
	}
	~S() {
		lifetimeChecker.order.push_back(i);
		std::cout << i << "\n";
		
	}
};
TEST(SingletonTest, longevityTest) {

	LifetimeSingleton_t<S<200>, 200> single200;
	LifetimeSingleton_t<S<10>, 10> single1;
	LifetimeSingleton_t<S<20>, 20> single2;
	LifetimeSingleton_t<S<5>, 5> single3;
	LifetimeSingleton_t<S<100>, 100> single100;
	LifetimeSingleton_t<S<25>, 25> single25;
	LifetimeSingleton_t<S<25>, 25> single25_2;
	LifetimeSingleton_t<S<5000>, 5000> single5000;
	single25_2.get();
	single200.get();
	single2.get();
	single1.get();
	single100.get();
	single3.get();
	single25.get();
	single5000.get();
}

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}