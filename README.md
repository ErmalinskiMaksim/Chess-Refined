# Description
A minimalistic LAN chess game in C++, RTWgui, and boost::asio.\
Runs on Linux. The other platforms have not yet been tested.

## Dependencies
**The project requires at least g++-14.2**. 
The project depends on RTWgui and boost::asio. 
* RTWgui is handled internally, however problems may arise if SDL3/SDL3_TTF/SDL3_IMG dependencies are missing.
* boost::asio must be preinstalled on the system.

## How does the game work?
The game consists of a chess server and player clients. The server can handle many sessions at once. To run a single session, first launch the server, then launch two clients. The game doesn't support single client mode.\
*Pieces won't be visible until the second client joins the session.*\
***THE GAME SUPPORTS LOCAL HOST AND LAN MODE***

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
build/Client/ChessClient [optional: ip address of the server] 
```
You can run the game in both the local host mode and a LAN mode. To run with local host settings, the clients must be run as:
```bash 
build/Client/ChessClient
```
Alternatively, to run in a LAN mode, provide ip address of the server:
```bash
build/Client/ChessClient [ipv4 address] 
```

## Testing connection
To test if the game runs properly, there are ready docker-compose files. Just run:
```bash
xhost +local:docker 
docker compose up --build
docker compose up --scale client=2
```
## Future updates:
* CppUTest integration
* AI
