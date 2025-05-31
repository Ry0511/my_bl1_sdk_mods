//
// Date       : 31/05/2025
// Project    : bl1_sdk_mods
// Author     : -Ry
//

#pragma once

#include "pch.h"

namespace tm_parse {

/**

Basic Ring Buffer

; Basic structure
XS   := { A, B, ?, ?, ? }
Head := { ^, _, _, _, _ }
Tail := { _, _, ^, _, _ }

; Logic for a full ring buffer
Full   := { A, B, C, D, E }
Head   := { ^, _, _, _, _ }
Tail   := { _, _, _, _, ^ }

; This affects maximum size of the buffer
IsFull := ((Tail + 1) % Length) == Head
Empty  := ( Head == Tail )

; Empty Buffer
XS   := { ?, ?, ?, ?, ? }
Head := { ^, _, _, _, _ }
Tail := { ^, _, _, _, _ }


*/

template <typename IntegralType, IntegralType Length>
    requires(std::is_integral_v<IntegralType> && std::is_unsigned_v<IntegralType>)
class TokenRingBuffer {
   private:
    std::array<Token, Length> m_Buffer{token_invalid, {}};
    IntegralType m_Head{0};
    IntegralType m_Tail{0};

   public:
    explicit TokenRingBuffer() noexcept(true) = default;
    ~TokenRingBuffer() noexcept(true) = default;

   public:
    [[nodiscard]] IntegralType head() const { return m_Head; }
    [[nodiscard]] IntegralType tail() const { return m_Tail; }
    [[nodiscard]] constexpr IntegralType buffer_size() const { return Length; }

   public:
    bool is_full() const noexcept(true) { return ((m_Tail + 1) % Length) == m_Head; }
    bool is_empty() const noexcept(true) { return m_Head == m_Tail; }

    size_t length() const noexcept(true) {
        constexpr auto length = static_cast<size_t>(Length);
        auto head = static_cast<size_t>(m_Head);
        auto tail = static_cast<size_t>(m_Tail);

        if (tail >= head) {
            return tail - head;
        }

        return Length - (head - tail);
    }

   public:
    /**
     * Attempts to push the provided token into the ring buffer if there is space.
     * @param token The token to push.
     * @return True if there was space for the token.
     */
    bool push(const Token& token) noexcept(true) {
        if (is_full()) {
            return false;
        }
        // Set & Advance tail
        m_Buffer[m_Tail] = token;
        m_Tail = (m_Tail + 1) % Length;
        return true;
    }

    /**
     * Pops the item from the front of the buffer and returns it.
     * @return The item at the fron or token_eof if we are empty.
     * @note Peek(-1) == token_invalid, after this operation.
     */
    Token pop() noexcept(true) {
        if (is_empty()) {
            return token_invalid;
        }

        Token token = m_Buffer[m_Head];
        m_Buffer[m_Head] = token_invalid;
        m_Head = (m_Head + 1) % Length;
        return token;
    }

    const Token& peek(IntegralType offset = 0) noexcept(true) {
        if (is_empty()) {
            return token_invalid;
        }

        if (offset == 0) {
            return m_Buffer[m_Head];
        }

        return m_Buffer[(m_Head + offset) % Length];
    }
};

}  // namespace tm_parse
