cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# add_executable(TypeListTest "TypeListTest.cpp" "${INCLUDE_DIR}/TypeList.hpp")
# target_include_directories(TypeListTest PRIVATE ${INCLUDE_DIR})
# target_link_libraries(TypeListTest PRIVATE gtest)
# add_test(TypeListTest TypeListTest)

# add_executable(SmallUtilitiesTest "SmallUtilitiesTest.cpp" 
# 	"${INCLUDE_DIR}/Visitable.hpp" 
# 	"${INCLUDE_DIR}/Visitor.hpp"
# 	"${INCLUDE_DIR}/Cast.hpp")
# target_include_directories(SmallUtilitiesTest PRIVATE ${INCLUDE_DIR})
# target_link_libraries(SmallUtilitiesTest PRIVATE gtest)
# add_test(SmallUtilitiesTest SmallUtilitiesTest)

# add_executable(SingletonTest "SingletonTest.cpp" 
# 	"${INCLUDE_DIR}/Singleton.hpp")
# target_include_directories(SingletonTest PRIVATE ${INCLUDE_DIR})
# target_link_libraries(SingletonTest PRIVATE gtest)
# add_test(SingletonTest SingletonTest)

add_executable(UnitsTest "units_test.cpp" 
	"${INCLUDE_DIR}/units.hpp")
target_include_directories(UnitsTest PRIVATE ${INCLUDE_DIR})
target_link_libraries(UnitsTest PRIVATE gtest)
add_test(UnitsTest UnitsTest)