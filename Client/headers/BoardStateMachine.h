#ifndef BOARD_STATE_MACHINE_H
#define BOARD_STATE_MACHINE_H

#include "RTWgui/Events.h"
#include "ChessClient.h"

class IdleBoardState;
class MoveWaitingState;
class MoveSelectionBoardState;
class MoveCommitWaitingState;
class PromotionCommitWaitingState;
class GameOverBoardState;

using States = std::variant<
              IdleBoardState
            , MoveWaitingState
            , MoveSelectionBoardState
            , MoveCommitWaitingState
            , PromotionCommitWaitingState
            , GameOverBoardState
>;
using OptState = std::optional<States>;

// Base state of the state machine. Does nothing by default
class BaseBoardState {
public:
    explicit BaseBoardState(DefaultBoardInteractorView context) 
        : m_context(context) {}
    template<NetworkMessageType Msg>
    OptState process(Msg);
protected:
    // Board interactor reference
    DefaultBoardInteractorView m_context;
};

template<typename T>
concept BoardStateType = std::is_base_of_v<BaseBoardState, T>;

class IdleBoardState : public BaseBoardState {
public:
    explicit IdleBoardState(DefaultBoardInteractorView context)
        : BaseBoardState(context) {}
    // supported events
    OptState process(const MouseLeftDownEvent&);
    using BaseBoardState::process;
    OptState process(GameState::Flags);
};

class MoveWaitingState : public BaseBoardState {
public:
    explicit MoveWaitingState(DefaultBoardInteractorView context)
        : BaseBoardState(context) {}
    // supported events
    OptState process(const MouseLeftDownEvent&);
    using BaseBoardState::process;
    OptState process(Moves);
};

class MoveSelectionBoardState : public BaseBoardState {
public:
    explicit MoveSelectionBoardState(DefaultBoardInteractorView context) 
        : BaseBoardState(context) {}
    // supported events
    OptState process(const MouseLeftDownEvent&);
    using BaseBoardState::process;
};

class MoveCommitWaitingState : public BaseBoardState {
public:
    explicit MoveCommitWaitingState(DefaultBoardInteractorView context)
        : BaseBoardState(context) {}
    // supported events
    OptState process(const MouseLeftDownEvent&);
    using BaseBoardState::process;
    OptState process(GameState::Flags);
};

class PromotionCommitWaitingState : public BaseBoardState {
public:
    explicit PromotionCommitWaitingState (DefaultBoardInteractorView context)
        : BaseBoardState(context) {}
    // supported events
    OptState process(const MouseLeftDownEvent&);
    using BaseBoardState::process;
    OptState process(PromotionMsg);
};

class GameOverBoardState : public BaseBoardState {
public:
    explicit GameOverBoardState(DefaultBoardInteractorView context)
        : BaseBoardState(context) {}
    OptState process(const MouseLeftDownEvent&);
    using BaseBoardState::process;
};

template<NetworkMessageType Msg>
OptState BaseBoardState::process(Msg) { return std::nullopt; }

// state machine 
template<BoardStateType... States>
class BoardStateMachine {
public:
    template<BoardStateType InitialState>
    explicit BoardStateMachine(InitialState&& state) 
        : m_state(std::forward<InitialState>(state)) {}
    // dispatches states 
    void process(const MouseLeftDownEvent& event) {
        auto optResult = std::visit([&](auto& state) { return state.process(event); }, m_state);
        if (optResult) m_state = std::move(*optResult);
    }
    template<NetworkMessageType Msg>
    void process(Msg msg) {
        auto optResult = std::visit([&](auto& state) { return state.process(std::move(msg)); }, m_state);
        if (optResult) m_state = std::move(*optResult);
    }
private:
    std::variant<States...> m_state;
};

#endif //
