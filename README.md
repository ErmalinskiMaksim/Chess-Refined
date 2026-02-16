# Description
A minimalistic LAN chess game in C++, RTWgui, and boost::asio.\
Runs on Linux. The other platforms have not yet been tested.\

## Dependencies
The project depends on RTWgui and boost::asio. RTWgui is handled internally, so no extra actions needed.\
***However***, boost::asio *must* be already installed to make the game work. If it's installed but the project doesn't compile, then change:
```cpp
#include "boost/asio.hpp" -> #include <boost/asio.hpp>
```
In two files:
* Server/headers/Connection.h 
* Client/headers/Connection.h

## How does the game work?
The game consists of a chess server and player clients. The server can handle many sessions at once. To run a single session, first launch the server, then launch two clients. The game doesn't support single player.\
*Pieces won't be visible until the second player joins the session.*\
***AT THE MOMENT, THE GAME ONLY SUPPORTS UNIX_SOCKET MODE. LAN IS COMING SOON***

## Player's guide
* Select a piece by LMB clicking it
* Unselect a piece by LMB clicking the selected piece
* Move a piece by clicking a suggested move tile
* Promote by choosing an option in the menu popup.
* Close "Game Over" popup by clicking on it
* Restart a game by pressing R 
* Quit the client by pressing CTRL+Q 

## How to run?
``` bash 
# fetch
git clone https://github.com/ErmalinskiMaksim/Chess-Refined.git
# build
cd Chess-Refined
cmake -S . -B build
cmake --build build
# run
# first, run the server
build/Server/ChessServer
# then run clients
build/Client/ChessClient
```
## Future updates:
* LAN network mode
* AI
