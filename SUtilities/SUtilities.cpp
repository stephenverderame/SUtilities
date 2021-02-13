// SUtilities.cpp : Defines the entry point for the application.
//

#include "SUtilities.h"
#include "include/TypeList.hpp"
#include "include/Visitor.hpp"
#include "include/Visitable.hpp"

using namespace std;




template<TL::TList list>
constexpr void names() {
	for (unsigned i = 0; i < TL::size<list>(); ++i) {
		cout << TL::getInfo<list>(i).name() << " ";
	}
	cout << "\n";
}


int main()
{
	return 0;
}
