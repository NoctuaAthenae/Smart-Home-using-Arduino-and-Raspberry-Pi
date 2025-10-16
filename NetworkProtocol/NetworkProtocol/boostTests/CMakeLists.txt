set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

add_executable(CreateRawPackageTest CreateRawPackageTest.cpp
        ../messageBuilder.h
        ../messageBuilder.cpp)
add_executable(MessageBuilderTest
        ../messageBuilder.h
        ../messageBuilder.cpp
        MessageBuilderTest.cpp)
target_link_libraries(CreateRawPackageTest PRIVATE Boost::unit_test_framework stdc++ NetworkProtocol)
target_link_libraries(MessageBuilderTest PRIVATE Boost::unit_test_framework stdc++ NetworkProtocol)