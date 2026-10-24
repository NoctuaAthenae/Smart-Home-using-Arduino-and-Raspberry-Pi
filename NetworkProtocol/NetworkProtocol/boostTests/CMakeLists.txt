set (Boost_USE_STATIC_LIBS OFF)
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (${Boost_INCLUDE_DIRS})

add_executable(CreateRawPackageTest CreateRawPackageTest.cpp
        ../Messages/messageBuilder.h
        ../Messages/messageBuilder.cpp)
add_executable(MessageBuilderTest
        ../Messages/messageBuilder.h
        ../Messages/messageBuilder.cpp
        MessageBuilderTest.cpp)
add_executable(NetworkTest NetworkTest.cpp
        testNetworkDevice.cpp
        testNetworkDevice.h
        ../networkDevice.cpp
        ../networkDevice.h)
target_link_libraries(CreateRawPackageTest PRIVATE Boost::unit_test_framework stdc++ NetworkProtocol)
target_link_libraries(MessageBuilderTest PRIVATE Boost::unit_test_framework stdc++ NetworkProtocol)
target_link_libraries(NetworkTest PRIVATE Boost::unit_test_framework stdc++ NetworkProtocol)