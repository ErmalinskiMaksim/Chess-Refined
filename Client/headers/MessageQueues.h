#ifndef MESSAGE_QUEUES_H
#define MESSAGE_QUEUES_H

#include "GameStateCache.h"
#include <condition_variable>
#include <mutex>
#include <queue>
#include <variant>

// NETWORK MESSAGES
struct StateUpdateMsg { GameStateCache cache; Piece::Color myColor; };
struct RestartMsg { GameStateCache cache; Piece::Color myColor; };
struct ShutdownMsg {};

using NetworkMessage = std::variant<StateUpdateMsg, RestartMsg, Moves, ShutdownMsg>;

template<typename T>
concept NetworkMessageType = std::is_same_v<T, StateUpdateMsg>
                            || std::is_same_v<T, RestartMsg>
                            || std::is_same_v<T, Moves>
                            || std::is_same_v<T, ShutdownMsg>;

// GUI MESSAGES 
struct AvailableMovesReq { Pos moveFrom; };
struct MoveCommitReq { Pos moveTo; };
struct RestartReq {};
struct PromotionReq { Piece::Type type; };

using GuiMessage = std::variant<AvailableMovesReq, MoveCommitReq, RestartReq, PromotionReq>;

// QUEUES
template <typename T>
concept MessageType = std::is_same_v<T, NetworkMessage> || std::is_same_v<T, GuiMessage>; 

// A thread-safe message queue implementation.
template<MessageType Msg>
class ThreadSafeMQ {
public:
    // push to the queue and notify
    void push(Msg msg) {
        {
            std::lock_guard lock(m_mtx);
            m_queue.push(std::move(msg));
        }
        m_cv.notify_one();
    }

    // blocking pop
    std::optional<Msg> waitPop() {
        std::unique_lock lock(m_mtx);
        m_cv.wait(lock, [&] { return !m_queue.empty() || m_stopped; });

        if (m_stopped) return std::nullopt;

        auto msg = std::move(m_queue.front());
        m_queue.pop();
        return msg;
    }

    // non-blocking pop
    std::optional<Msg> tryPop() {
        std::lock_guard lock(m_mtx);

        if (m_queue.empty()) return std::nullopt;

        auto out = std::optional<Msg>{std::move(m_queue.front())};
        m_queue.pop();
        
        return out;
    }

    // shut down the queue and notify
    void stop() {
        {
            std::lock_guard lock(m_mtx);
            m_stopped = true;
        }
        m_cv.notify_all();
    }
private:
    std::mutex m_mtx;
    std::condition_variable m_cv;
    std::queue<Msg> m_queue;
    bool m_stopped = false;
};

struct ThreadQueues {
    ThreadSafeMQ<NetworkMessage> net2gui;   
    ThreadSafeMQ<GuiMessage> gui2net;
};
// a global means of communication between the GUI thread 
// and the network thread
inline ThreadQueues g_MQ;

#endif
