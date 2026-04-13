#ifndef CHESS_SERVER_H
#define CHESS_SERVER_H

#include "Board.h"
#include "Connection.h"

#include <unordered_set>

struct Packet;

// Main server-side game logic and connection manager
class ChessServer {
    using ConnectionList = std::unordered_set<ConnectionPtr>;
public:
    ChessServer();
    void doAccept();
    void run();

    // callbacks
    void onConnect      (const ConnectionPtr&);
    void onDisconnect   (const ConnectionPtr&);
    void onPacket       (const ConnectionPtr&, Packet);
private:
    ServerView getView() noexcept;
    // sessions
    void removeSession  (const ConnectionPtr&);
    void createSession      (const ConnectionPtr&, const ConnectionPtr&);

    // connections
    void addConnection      (const ConnectionPtr&);
    void removeConnection   (const ConnectionPtr&);

    //  client requests 
    struct GameSession;
    void performShutdown        ();
    void performRestart         (GameSession&);
    void performAvailableMoves  (GameSession&, const ConnectionPtr&, Pos);
    void performPromotion       (GameSession&, Piece::Type, Pos);
    void performCommitMove      (GameSession&, Pos, Pos);
    
    // server actions
    void notifyColor        (const ConnectionPtr&, Piece::Color) const;
    void notifyDisconnect   (const ConnectionPtr&) const;

    struct GameSession {
        struct State {
            Board board = {};
            Score score = {0x00, 0x00};
            Piece::Color currentTeam = Piece::Color::WHITE;
        } state;    
        ConnectionPtr white;
        ConnectionPtr black;
    };

    // all active sessiosn
    std::vector<GameSession> m_sessions;
    // network context
    Context m_io;
    // acceptor socket
    Acceptor m_acceptor;
    // all active connections
    ConnectionList m_connections;
    // an active connection with a player who hasn't joined a session
    ConnectionPtr m_waitingPlayer;
};

#endif
