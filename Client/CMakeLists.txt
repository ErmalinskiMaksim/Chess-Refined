# executables
file(GLOB SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable (ChessClient ${SRC})

# features 
target_compile_features(ChessClient PUBLIC cxx_std_23)

# options
target_compile_options(ChessClient PRIVATE
    -Wstrict-aliasing
    -Wall 
    -Wextra 
    -Werror
    -Wpedantic 
    -Wwrite-strings
    -Wmissing-declarations
    -Wmissing-include-dirs
    -Wfloat-equal
    -Wcast-align
    -Wimplicit-fallthrough
    -Wempty-body
    -Wuninitialized
    -Wmisleading-indentation
    -Wshadow
    -Wconversion 
    -Wsign-compare
    -Wswitch-default 
    -Wundef
)

if (GUI_ENABLE_LTO)
    set_property(TARGET ChessClient PROPERTY
        INTERPROCEDURAL_OPTIMIZATION_RELEASE true)
endif()

target_compile_options(ChessClient PRIVATE
    $<$<CONFIG:Debug>:
        -O0
        -g3
        -ggdb3
        -fno-elide-constructors
        -fno-strict-aliasing
        -fno-omit-frame-pointer
        -ftrapv
        -fstack-protector-strong
    >
)

target_compile_options(ChessClient PRIVATE
    $<$<CONFIG:Release>:
        -O3
        -march=native
        -fno-plt
    >
)

# definitions
target_compile_definitions(ChessClient PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(ChessClient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/headers
)

add_compile_options(-fdebug-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=.)   

target_link_libraries(ChessClient PRIVATE RTWgui Boost::boost Threads::Threads)

