#ifndef HANDLER_CONTEXT_H
#define HANDLER_CONTEXT_H

#include "RTWgui/Widgets/Widget.h"
#include "RTWgui/Requests.h"
#include "GameState.h"

struct EmptyOperation {};
struct PromotionOperation { Piece::Type type; };

using OperationRegister = std::variant<EmptyOperation, PromotionOperation>;
using OperationView = std::reference_wrapper<OperationRegister>;
using OptOperationView = std::optional<OperationView>;

// bare minimum context that is necessary for 
// an action handler
struct HandlerContext {
    std::reference_wrapper<const Widget> widget;
    RequestView req;
    OptOperationView op = std::nullopt;
    std::optional<GameState>
    gameOver = std::nullopt; // team name (0 = black, 1 = white),
                             // mate type (0 = stale, 1 = check),
                             // white score, black score
};

#endif
