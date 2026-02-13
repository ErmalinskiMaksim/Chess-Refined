#ifndef HANDLER_CONTEXT_H
#define HANDLER_CONTEXT_H

#include "RTWgui/Widgets/Widget.h"
#include "RTWgui/Requests.h"
#include "Operations.h"
#include <functional>
     
// bare minimum context that is necessary for 
// an action handler
struct HandlerContext {
    std::reference_wrapper<const Widget> widget;
    RequestView req;
    OptOperationView op = std::nullopt;
};

#endif
