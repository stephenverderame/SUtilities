#include <gtest/gtest.h>
#include <Visitable.hpp>
#include <Visitor.hpp>
#include <sstream>
#include <Cast.hpp>

using namespace SUtil;

TEST(VisitorTest, visitorTest) {
	struct CanVisit : public BaseVisitable<> {
		MAKE_VISITABLE(void);
	};

	struct CantVisit : public BaseVisitable<> {
		MAKE_VISITABLE(void);
	};

	struct Other : public BaseVisitable<> {
		MAKE_VISITABLE(void);
	};



	class MyVisitor : public Visitor<void, CanVisit, Other> {
	public:
		std::stringstream ss;
		void visit(CanVisit&) override {
			ss << "Can Visit\n";
		}

		void visit(Other&) override {
			ss << "Other\n";
		}
	};

	Other o;
	CanVisit v;
	CantVisit cv;
	MyVisitor mv;
	v.accept(mv);
	o.accept(mv);
	v.accept(mv);
	ASSERT_EQ(mv.ss.str(), "Can Visit\nOther\nCan Visit\n");
	ASSERT_THROW(cv.accept(mv), UnknownVisitorException);



}

TEST(VisitorTest, constVisitorTest) {
	struct ConstVisit : public ImmutableBaseVisitable<> {
		MAKE_CONST_VISITABLE(void);
	};
	struct FluidVisitable :
		public BaseVisitable<> {

		MAKE_VISITABLE(void);
	};


	class ConstVisitor : public Visitor<void, const ConstVisit, const FluidVisitable> {
	public:
		std::stringstream ss;
		void visit(const ConstVisit&) override {
			ss << "Accepted ";
		}
		void visit(const FluidVisitable&) override {
			ss << "Fluid Accepted ";
		}
	};

	class NonConstVisitor : public Visitor<void, ConstVisit, FluidVisitable> {
	public:
		std::stringstream ss;
		void visit(ConstVisit&) override {
		}
		void visit(FluidVisitable&) override {
			ss << "Accepted";
		}
	};

	const ConstVisit v2;

	ConstVisitor constVisitor;
	FluidVisitable fluid;
	NonConstVisitor ncv;
	v2.accept(constVisitor);
	fluid.accept(constVisitor);
	ASSERT_EQ(constVisitor.ss.str(), "Accepted Fluid Accepted ");
	ASSERT_THROW(v2.accept(ncv), UnknownVisitorException);
	fluid.accept(ncv);
	ASSERT_EQ(ncv.ss.str(), "Accepted");


	const FluidVisitable cFluid;
	cFluid.accept(constVisitor);
	ASSERT_EQ(constVisitor.ss.str(), "Accepted Fluid Accepted Fluid Accepted ");
	ASSERT_THROW(cFluid.accept(ncv), UnknownVisitorException);
}

TEST(CastTest, castTest) {
	narrow_cast<char>(100);
	narrow_cast<short>(-5000);
	narrow_cast<double>(50);
	narrow_cast<double>(100.f);
	narrow_cast<int>(20.0);
	ASSERT_THROW(narrow_cast<char>(200), std::bad_cast);
	ASSERT_THROW(strict_narrow_cast<unsigned int>(-1), std::bad_cast);
	narrow_cast<unsigned int>(-1);
	narrow_cast<float>(20.0);
	narrow_cast<float>(10.5);
	narrow_cast<double>(10.436462637f);
	ASSERT_THROW(narrow_cast<float>(10.00000564), std::bad_cast);
	strict_narrow_cast<short>(125);
	strict_narrow_cast<char>(-100);
	ASSERT_THROW(strict_narrow_cast<char>(-200), std::bad_cast);

	enum class TestEnum {
		val1, val2
	};
	strict_narrow_cast<int>(TestEnum::val1);
	strict_narrow_cast<char>(TestEnum::val1);
	strict_narrow_cast<TestEnum>(500);
	strict_narrow_cast<TestEnum>(-500);

	enum class TestEnum2 : uint8_t {
		val1
	};
	ASSERT_THROW(strict_narrow_cast<TestEnum2>(-10), std::bad_cast);
	ASSERT_THROW(narrow_cast<TestEnum2>(5000), std::bad_cast);
}


int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}