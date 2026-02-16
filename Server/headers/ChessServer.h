#ifndef CHESS_SERVER_H
#define CHESS_SERVER_H

#include "Common.h"
#include "Connection.h"
#include "Board.h"
#include <unordered_set>

class ChessServer {
    using AcceptorType = boost::asio::local::stream_protocol::acceptor;
    using EndpointType = boost::asio::local::stream_protocol::endpoint;
    using ConnectionList = std::unordered_set<ConnectionPtr>;
    using ContextType = boost::asio::io_context;
public:
    ChessServer(std::string_view);
    ServerView getView() noexcept;
    void doAccept();
    void run();
    void onConnect(ConnectionPtr);
    void onDisconnect(ConnectionPtr);
    void dispatchRequests(ConnectionPtr, Request);
private:
    // sessions
    void removeFromSession(ConnectionPtr);
    void createSession(ConnectionPtr, ConnectionPtr);

    // connections
    void addConnection(ConnectionPtr);
    void removeConnection(ConnectionPtr);

    //  client requests 
    void performShutdown(ConnectionPtr);
    void performRestart(ConnectionPtr);
    void performAvailableMoves(ConnectionPtr, Pos);
    void performPromotion(ConnectionPtr, Pos, PieceType);
    void performCommitMove(ConnectionPtr, Pos, Pos);
    void performGetCache(ConnectionPtr);
    
    // server actions
    void notifyColor(ConnectionPtr, PieceColor);
    void notifyDisconnect(ConnectionPtr);

    struct GameSession {
        struct GameState {
            Board board = {};
            Score score = {0x00, 0x00};
            PieceColor currentTeam = PieceColor::WHITE;
        } state;    
        ConnectionPtr white;
        ConnectionPtr black;
    };

    // helpers
    std::optional<size_t> findSession(ConnectionPtr);

    std::vector<GameSession> m_sessions;
    ContextType m_io;
    AcceptorType m_acceptor;
    ConnectionList m_connections;
    ConnectionPtr m_waitingPlayer;
};

#endif
