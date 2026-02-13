#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <variant>
#include <optional>
#include "Piece.h"

struct EmptyOperation {};
struct ChooseOperation { PieceType type; };

using OperationRegister = std::variant<EmptyOperation, ChooseOperation>;
using OperationView = std::reference_wrapper<OperationRegister>;
using OptOperationView = std::optional<OperationView>;

#endif
