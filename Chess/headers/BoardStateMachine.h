#ifndef BOARD_STATE_MACHINE_H
#define BOARD_STATE_MACHINE_H

#include "RTWgui/Events.h"
#include "BoardWidget.h"
#include "BoardHandlers.h"
#include "HandlerContext.h"

template<WidgetType, typename, typename>
class BoardInteractor;
using DefaultBoardInteractorView = std::reference_wrapper<
                               BoardInteractor<BoardWidget
                             , HandlerContext
                             , BoardPromotionHandler<HandlerContext>
>>;

class IdleBoardState;
class MoveSelectionBoardState;
class GameOverBoardState;

using States = std::variant<
              IdleBoardState
            , MoveSelectionBoardState
            , GameOverBoardState
>;
using OptState = std::optional<States>;

// Base state of the state machine. Does nothing by default
class BaseBoardState {
public:
    explicit BaseBoardState(DefaultBoardInteractorView context) 
        : m_context(context) {}
    // supported events
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
    // supported rendering logic
    void draw(const Renderer&, const Font&) const;
};

class MoveSelectionBoardState : public BaseBoardState {
public:
    explicit MoveSelectionBoardState(DefaultBoardInteractorView context) 
        : BaseBoardState(context) {}
    // supported events
    OptState process(const MouseLeftDownEvent&);
    // supported rendering logic
    void draw(const Renderer&, const Font&) const;
};

class GameOverBoardState : public BaseBoardState {
public:
    explicit GameOverBoardState(DefaultBoardInteractorView context)
        : BaseBoardState(context) {}
    OptState process(const MouseLeftDownEvent&);
    void draw(const Renderer&, const Font&) const;
};

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
    // dispatches states
    void draw(const Renderer& renderer, const Font& font) const {
        std::visit([&](auto&& state) { return state.draw(renderer, font); }, m_state);
    }
private:
    std::variant<States...> m_state;
};


#endif //
