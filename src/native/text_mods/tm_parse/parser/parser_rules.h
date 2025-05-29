//
// Date       : 24/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

#include "parser_rule_enum.h"

namespace tm_parse {
class TextModLexer;
class TextModParser;
};  // namespace tm_parse

namespace tm_parse::rules {

// [[ParserDoc_DotIdentifier]]
struct DotIdentifierRule {
    size_t StartIndex{};  // Inclusive Start
    size_t EndIndex{};    // Inclusive End

    static DotIdentifierRule create(TextModParser* parser);
};

// [[ParserDoc_ObjectIdentifier]]
struct ObjectIdentifierRule {
    DotIdentifierRule PrimaryIdentifier{};
    std::optional<DotIdentifierRule> SecondaryIdentifier = std::nullopt;

    static ObjectIdentifierRule create(TextModParser* parser);
};

// [[ParserDoc_ArrayAccess]]
struct ArrayAccessRule {
    size_t StartTokenIndex{};
    size_t NumberTokenIndex{};
    size_t EndTokenIndex{};

    static ArrayAccessRule create(TextModParser* parser);
};

struct CompositeExprRule {};

// [[ParserDoc_SetCommand]]
struct SetCommandRule {
    size_t SetCommandIndex{};
    ObjectIdentifierRule ObjectIdentifier{};
    size_t PropertyIndex{};
    CompositeExprRule CompositeExpr{};

    str as_str(TextModParser* parser) const noexcept;
    static SetCommandRule create(TextModParser* parser);
};

struct ProgramRule {};

}  // namespace tm_parse::rules
