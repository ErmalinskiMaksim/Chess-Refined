#ifndef BOARD_HANDLERS_H    
#define BOARD_HANDLERS_H

#include "Piece.h"
#include "RTWgui/Requests.h"
#include "Operations.h"

template<typename HandlerContext>
class BoardPromotionHandler {
    // actions of the column/row  
    enum class Actions : ActionID 
    { CHOOSE_ROOK = 0X00, CHOOSE_KNIGHT = 0x01, CHOOSE_BISHOP = 0x02, CHOOSE_QUEEN = 0x03 };

    static consteval auto buildPayload() {
        using MenuAction = MenuCreateRequest::Payload::MenuAction;
        return std::array<MenuAction, 4> {
              MenuAction{"Rook",    static_cast<ActionID>(Actions::CHOOSE_ROOK)} 
            , MenuAction{"Knight",  static_cast<ActionID>(Actions::CHOOSE_KNIGHT)}
            , MenuAction{"Bishop",  static_cast<ActionID>(Actions::CHOOSE_BISHOP)}
            , MenuAction{"Queen",   static_cast<ActionID>(Actions::CHOOSE_QUEEN)}
        };
    }

public:
    static constexpr std::string_view getID() { return "Board ops"; }

    static bool requestMainMenu(HandlerContext ctx) {
        constexpr auto payload = buildPayload();

        const auto& w = ctx.widget.get();
        auto hbox = w.getHitBox();
        auto menuWidth = w.getCharWidth() * 6;
        auto menuHeight = w.getCharHeight() * 4;
        ctx.req.get() = MenuCreateRequest {
            Widget {
                Rect{hbox.x + (hbox.w - menuWidth)/2, hbox.y + (hbox.h - menuHeight)/2, menuWidth, menuHeight}
                , {0x9A, 0xC5, 0xA2, 0xFF}
                , {0x00, 0xBB, 0xBB, 0xFF} 
            }
            , MenuCreateRequest::Payload {{ std::begin(payload), std::end(payload)}}};
        
        return true;
    }

    bool dispatch(MenuResponse&& resp, HandlerContext ctx) {
        if (!ctx.op) return false;
        switch (static_cast<Actions>(resp.code)) {
        case Actions::CHOOSE_ROOK:
            ctx.op->get() = ChooseOperation{ PieceType::ROOK };            
            break;
        case Actions::CHOOSE_KNIGHT:
            ctx.op->get() = ChooseOperation{ PieceType::KNIGHT };            
            break;
        case Actions::CHOOSE_BISHOP:
            ctx.op->get() = ChooseOperation{ PieceType::BISHOP };            
            break;
        case Actions::CHOOSE_QUEEN:
            ctx.op->get() = ChooseOperation{ PieceType::QUEEN };
            break;
        default:
            ctx.op->get() = EmptyOperation{};
            return false;
        }
        return true;
    }
};

#endif
