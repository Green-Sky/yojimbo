cmake_minimum_required(VERSION 3.0)
project(yojimbo_serializer)

add_library(yojimbo_serializer
	yojimbo/yojimbo_serializer.h
	yojimbo/yojimbo_serializer.cpp

	tlsf/tlsf.h
	tlsf/tlsf.c
)

target_include_directories(yojimbo_serializer PUBLIC "yojimbo")

if(BUILD_TESTING)
	add_executable(yojimbo_serializer_test
		test.cpp
	)

	target_link_libraries(yojimbo_serializer_test
		yojimbo_serializer
	)
endif()

