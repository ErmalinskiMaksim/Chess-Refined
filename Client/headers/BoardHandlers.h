#ifndef BOARD_HANDLERS_H    
#define BOARD_HANDLERS_H

#include "RTWgui/Requests.h"
#include "HandlerContext.h"

#include <charconv>

template<typename HandlerContext>
class BoardPromotionHandler {
    // actions of the promotion menu 
    enum class Actions : ActionID 
    { CHOOSE_ROOK = 0X00, CHOOSE_KNIGHT = 0x01, CHOOSE_BISHOP = 0x02, CHOOSE_QUEEN = 0x03 };

    // Responsibility: define a menu layout
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

    // Responsibility: build a menu widget and package it
    static bool requestMainMenu(HandlerContext ctx) {
        constexpr auto payload = buildPayload();

        const auto& w = ctx.widget.get();
        auto hbox = w.getHitBox();
        auto menuWidth = w.getCharWidth() * 6;
        auto menuHeight = w.getCharHeight() * 4;
        ctx.req.get() = MenuCreateRequest {
            Widget {
                Rect{hbox.x + (hbox.w - menuWidth)/2, hbox.y + (hbox.h - menuHeight)/2, menuWidth, menuHeight}
                , {0xDD, 0xDD, 0xDD, 0xFF}
                , {0x00, 0xBB, 0xBB, 0xFF} 
            }
            , MenuCreateRequest::Payload {{ std::begin(payload), std::end(payload)}}};
        
        return true;
    }

    // Responsibility: If a response is directed at it,
    // * then, set a pending operation for the BoardInteractor 
    //   and consume the response by returning true
    // * else, indicate that the response was not consumed by rerturning false
    bool dispatch(MenuResponse&& resp, HandlerContext ctx) {
        if (!ctx.op) return false;
        switch (static_cast<Actions>(resp.code)) {
        case Actions::CHOOSE_ROOK:
            ctx.op->get() = PromotionOperation{ Piece::Type::ROOK };            
            break;
        case Actions::CHOOSE_KNIGHT:
            ctx.op->get() = PromotionOperation{ Piece::Type::KNIGHT };            
            break;
        case Actions::CHOOSE_BISHOP:
            ctx.op->get() = PromotionOperation{ Piece::Type::BISHOP };            
            break;
        case Actions::CHOOSE_QUEEN:
            ctx.op->get() = PromotionOperation{ Piece::Type::QUEEN };
            break;
        default:
            ctx.op->get() = EmptyOperation{};
            return false;
        }
        return true;
    }
};

template<typename HandlerContext>
class GameOverHandler {
    // Responsibility: convert the score to a string of a format: 
    // [WHITE_SCORE:BLACK_SCORE]
    static std::string_view buildScoreString(Score score) {
        char* begin = s_score;
        char* end = s_score + sizeof(s_score);

        auto whiteScore = std::to_chars(begin, end, static_cast<unsigned>(score.white));
        begin = whiteScore.ptr;

        *begin++ = ':';

        auto blackScore = std::to_chars(begin, end, static_cast<unsigned>(score.black));
        begin = blackScore.ptr;
        *begin++ = '\0'; 

        return std::string_view(s_score, static_cast<size_t>(begin - s_score));
    }
public:
    static constexpr std::string_view getID() { return "GameOver"; }

    // Responsibility: build a game over menu and package it into a request
    static bool requestMainMenu(HandlerContext ctx) {
        const auto& w = ctx.widget.get();
        auto hbox = w.getHitBox();
        auto popupW = w.getCharWidth() * 10;
        auto popupH = w.getCharHeight() * 4;

        if(!ctx.gameOver) return false;
        auto go = *ctx.gameOver;

        ctx.req.get() = PopupCreateRequest {
            Widget { 
                Rect {hbox.x+(hbox.w-popupW)/2, hbox.y+(hbox.h-popupH)/2, popupW, popupH}
                , Color {0xDD, 0xDD, 0xDD, 0xFF}
                , Color {0x44, 0x44, 0x44, 0xFF}
            }
            , {{  (go.currentTeam == Piece::Color::BLACK) ? "WHITE WIN!" : "BLACK WIN!"
                , (go.flag == GameState::Flags::CHECK_MATE)  ? "CHECK MATE" : "STALE MATE"
                , "SCORE: WvB"
                , buildScoreString(go.score)
            }}
        };
        return true;
    }
    // Responsibility: indicate that a response can't be directed at it
    static bool dispatch(PopupResponse&&, HandlerContext) { return false; }
private:
    // PopupCreateRequest accepts string_views
    // so the s_scores must be here for lifetiem
    inline static char s_score[8] = {}; 
};

#endif
