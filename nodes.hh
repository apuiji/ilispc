#pragma once

#include"ilispc.hh"

namespace zlt::ilispc {
  struct RawNode: virtual Node {
    virtual std::string_view getRaw() const noexcept = 0;
  };

  struct NumberNode final: RawNode {
    double value;
    const std::string *raw;
    NumberNode(const Pos *pos, double value, const std::string *raw) noexcept: Node(pos), value(value), raw(raw) {}
    std::string_view getRaw() const noexcept override;
  };

  struct StringLiteral final: Node {
    const std::string *value;
    StringLiteral(const Pos *pos, const std::string *value) noexcept: Node(pos), value(value) {}
  };

  struct ID final: RawNode {
    const std::string *raw;
    ID(const Pos *pos, const std::string *raw) noexcept: Node(pos), raw(raw) {}
    std::string_view getRaw() const noexcept override;
  };

  struct ListNode final: Node {
    std::list<UniqNode> items;
    ListNode(const Pos *pos, std::list<UniqNode> &&items = {}) noexcept: Node(pos), items(std::move(items)) {}
  };

  struct TokenNode final: RawNode {
    int token;
    TokenNode(const Pos *pos, int token) noexcept: Node(pos), token(token) {}
    std::string_view getRaw() const noexcept override;
  };
}
