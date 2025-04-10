# CMakeList.txt : CMake project for Chess, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (Chess "executables/Chess.cpp" "headers/Constants.h"
						"executables/Window.cpp" "headers/Window.h"
						"executables/PieceType.cpp" "headers/PieceType.h" 
						"executables/Piece.cpp" "headers/Piece.h"
						"executables/Board.cpp" "headers/Board.h"
						"executables/Game.cpp" "headers/Game.h"
						 )

#target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-graphics.lib)
#target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-audio.lib)
#target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-system.lib)
#target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-window.lib)

target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-graphics-d.lib)
target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-audio-d.lib)
target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-system-d.lib)
target_link_libraries(Chess  ${CMAKE_SOURCE_DIR}/Dependencies/lib/sfml-window-d.lib)

include_directories(${CMAKE_SOURCE_DIR}/Dependencies/include)

set_property(TARGET Chess PROPERTY CXX_STANDARD 20)
set_property(TARGET Chess PROPERTY CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Chess PROPERTY CXX_STANDARD 23)
endif()

# TODO: Add tests and install targets if needed.
