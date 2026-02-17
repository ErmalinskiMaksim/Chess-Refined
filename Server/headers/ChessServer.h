#ifndef CHESS_SERVER_H
#define CHESS_SERVER_H

#include <unordered_set>
#include "Board.h"
#include "Connection.h"
#include "GameState.h"

struct Packet;
class ChessServer {
    using ConnectionList = std::unordered_set<ConnectionPtr>;
public:
    ChessServer(/*UNIX std::string_view*/);
    ServerView getView() noexcept;
    void doAccept();
    void run();
    void onConnect(const ConnectionPtr&);
    void onDisconnect(const ConnectionPtr&);
    void onPacket(const ConnectionPtr&, Packet);
private:
    // sessions
    void removeFromSession(const ConnectionPtr&);
    void createSession(const ConnectionPtr&, const ConnectionPtr&);

    // connections
    void addConnection(const ConnectionPtr&);
    void removeConnection(const ConnectionPtr&);

    //  client requests 
    void performShutdown(const ConnectionPtr&);
    void performRestart(const ConnectionPtr&);
    void performAvailableMoves(const ConnectionPtr&, Pos);
    void performPromotion(const ConnectionPtr&, Piece::Type, Pos);
    void performCommitMove(const ConnectionPtr&, Pos, Pos);
    void performGetCache(const ConnectionPtr&);
    
    // server actions
    void notifyColor(const ConnectionPtr&, Piece::Color) const;
    void notifyDisconnect(const ConnectionPtr&) const;

    // helpers
    std::optional<size_t> findSession(const ConnectionPtr&) const noexcept;

    struct GameSession {
        struct State {
            Board board = {};
            Score score = {0x00, 0x00};
            Piece::Color currentTeam = Piece::Color::WHITE;
        } state;    
        ConnectionPtr white;
        ConnectionPtr black;
    };

    std::vector<GameSession> m_sessions;
    Context m_io;
    Acceptor m_acceptor;
    ConnectionList m_connections;
    ConnectionPtr m_waitingPlayer;
};

#endif
