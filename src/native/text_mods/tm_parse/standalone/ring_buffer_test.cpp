//
// Date       : 31/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#include "pch.h"
#include "parser/token_ring_buffer.h"

#include "catch.hpp"

namespace tm_parse_tests {

using namespace tm_parse;

// NOLINTBEGIN(*-magic-numbers, *-function-cognitive-complexity)

TEST_CASE("Ring Buffer Functionality") {
    constexpr size_t buffer_size = 16;
    constexpr Token in_token{TokenKind::Identifier, {}};

    SECTION("Primary Functions") {
        TokenRingBuffer<uint8_t, buffer_size> buffer{};

        // Default states
        REQUIRE(buffer.length() == 0);
        REQUIRE(buffer.buffer_size() == buffer_size);
        REQUIRE(buffer.is_empty());
        REQUIRE(!buffer.is_full());

        // This is non-standard and perhaps it should just throw
        REQUIRE(buffer.pop() == token_invalid);

        // Push all
        for (size_t i = 0; i < buffer.buffer_size() - 1; ++i) {
            REQUIRE(buffer.push(in_token));
            REQUIRE(buffer.length() == (i + 1));
            REQUIRE(!buffer.is_empty());
        }

        // Buffer is full
        REQUIRE(!buffer.push(in_token));
        REQUIRE(!buffer.is_empty());
        REQUIRE(buffer.is_full());
        REQUIRE(buffer.length() == buffer.buffer_size() - 1);

        // Pop all
        size_t len = buffer.length();
        for (size_t i = 0; i < buffer.buffer_size() - 1; ++i) {
            REQUIRE(buffer.pop() == in_token);
            REQUIRE(buffer.length() == (len - (i + 1)));
        }

        REQUIRE(buffer.length() == 0);
    }

    SECTION("Aggressive Push/Pop") {
        TokenRingBuffer<uint8_t, buffer_size> buffer{};

        for (size_t i = 0; i < buffer.buffer_size() * 5; ++i) {
            REQUIRE(buffer.push(in_token));
            REQUIRE(buffer.push(in_token));
            REQUIRE(buffer.length() == 2);
            REQUIRE(buffer.pop() == in_token);
            REQUIRE(buffer.length() == 1);
            REQUIRE(buffer.pop() == in_token);
            REQUIRE(buffer.length() == 0);
            REQUIRE(buffer.pop() == token_invalid);
            REQUIRE(buffer.is_empty());
        }
    }

    SECTION("Peek") {
        TokenRingBuffer<uint8_t, buffer_size> buffer{};
        REQUIRE(buffer.peek() == token_invalid);

        Token tokens[]{
            Token{TokenKind::Identifier, {}},
            Token{    TokenKind::Kw_Set, {}},
            Token{  TokenKind::Kw_Begin, {}},
            Token{   TokenKind::Kw_None, {}},
            Token{  TokenKind::Kw_False, {}},
        };
        constexpr size_t token_count = sizeof(tokens) / sizeof(tokens[0]);

        int index = 0;
        for (const Token& tok : tokens) {
            REQUIRE(buffer.push(tok));
            REQUIRE(buffer.peek(index) == tok);
            ++index;
        }

    }
}

// NOLINTEND(*-magic-numbers, *-function-cognitive-complexity)

}  // namespace tm_parse_tests