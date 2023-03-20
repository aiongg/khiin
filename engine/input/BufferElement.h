#pragma once

#include <string>
#include <variant>
#include <vector>

#include "TaiText.h"
#include "UserToken.h"

namespace khiin::engine {
using utf8_size_t = size_t;

class BufferElement {
    using BufferElementType =
        std::variant<std::monostate, std::string, TaiText, Punctuation, VirtualSpace, UserToken>;

   private:
    class BufferElementProps {
       private:
        BufferElementProps() = default;
        SyllableParser* parser = nullptr;
        std::string input;
        std::optional<TaiToken> tai_token;
        std::optional<Punctuation> punctuation;
        std::optional<UserToken> user_token;

        bool virtual_space = false;
        bool set_converted = false;
        bool set_candidate = false;
        bool set_selected = false;
        friend class BufferElement;
        friend class Builder;
    };

   public:
    static bool ConvertedEq(BufferElement const& lhs, BufferElement const& rhs);
    // static BufferElement Build(
    //     SyllableParser* parser,
    //     std::string const& input,
    //     TaiToken const& match,
    //     bool set_candidate,
    //     bool set_converted);

    BufferElement();
    // explicit BufferElement(TaiText const& elem);
    // explicit BufferElement(TaiText&& elem);
    // explicit BufferElement(Punctuation const& elem);
    // explicit BufferElement(Punctuation&& elem);
    // explicit BufferElement(std::string const& elem);
    // explicit BufferElement(std::string&& elem);
    // explicit BufferElement(VirtualSpace elem);
    // explicit BufferElement(UserToken&& elem);

    bool operator==(BufferElement const& rhs) const;

    void Replace(TaiText const& elem);
    void Replace(std::string const& elem);
    void Replace(VirtualSpace elem);

    utf8_size_t size() const;
    utf8_size_t RawSize() const;

    std::string raw() const;
    std::string composed() const;
    std::string converted() const;
    std::optional<TaiToken> candidate() const;

    void Erase(utf8_size_t index);
    bool SetKhin(KhinKeyPosition khin_pos, char khin_key);

    utf8_size_t RawToComposedCaret(size_t raw_caret) const;
    size_t ComposedToRawCaret(utf8_size_t caret) const;

    bool IsVirtualSpace() const;
    bool IsVirtualSpace(utf8_size_t index) const;
    bool IsTaiText() const noexcept;
    bool IsUserToken() const noexcept;
    bool IsConverted() const noexcept;
    bool IsSelected() const noexcept;
    void SetConverted(bool converted) noexcept;
    void SetSelected(bool selected) noexcept;

   private:
    BufferElement(BufferElementProps const& properties);

    bool is_converted = false;
    bool is_selected = false;
    BufferElementType m_element;

   public:
    class Builder {
       public:
        Builder() {}

        Builder& Parser(SyllableParser* parser) {
            m_properties.parser = parser;
            return *this;
        }

        Builder& FromInput(std::string const& input) {
            m_properties.input = input;
            return *this;
        }

        Builder& WithTaiToken(TaiToken const& candidate) {
            m_properties.tai_token = candidate;
            return *this;
        }

        Builder& WithUserToken(UserToken const& user_token) {
            m_properties.user_token = user_token;
            return *this;
        }

        Builder& WithVirtualSpace() {
            m_properties.virtual_space = true;
            return *this;
        }

        Builder& WithPunctuation(Punctuation const& punctuation) {
            m_properties.punctuation = punctuation;
            return *this;
        }

        Builder& SetConverted() {
            m_properties.set_converted = true;
            return *this;
        }

        Builder& SetCandidate() {
            m_properties.set_candidate = true;
            return *this;
        }

        Builder& SetSelected() {
            m_properties.set_selected = true;
            return *this;
        }

        BufferElement Build() {
            BufferElement ret = BufferElement();

            return BufferElement(m_properties);
        }

       private:
        BufferElementProps m_properties;
    };
};

using BufferElementList = std::vector<BufferElement>;

}  // namespace khiin::engine
