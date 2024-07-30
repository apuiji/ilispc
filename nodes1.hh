#pragma once

#include<array>
#include"ilispc.hh"
#include"token.hh"

namespace zlt::ilispc {
  struct Call {
    UniqNode callee;
    UniqNodes args;
    Call(UniqNode &&callee = {}, UniqNodes &&args = {}) noexcept: callee(std::move(callee)), args(std::move(args)) {}
  };

  struct CallNode final: Node, Call {
    CallNode(const Pos *pos, Call &&call) noexcept: Node(pos), Call(std::move(call)) {}
  };

  struct Callee final: Node {
    using Node::Node;
  };

  struct Defer final: Node {
    UniqNode value;
    Defer(const Pos *pos, UniqNode &&value) noexcept: Node(pos), value(std::move(value)) {}
  };

  struct Forward final: Node, Call {
    Forward(const Pos *pos, Call &&call) noexcept: Node(pos), Call(std::move(call)) {}
  };

  struct Function final: Node {
    using Defs = std::vector<const std::string *>;
    Defs defs;
    size_t paramc;
    UniqNodes body;
    Function(const Pos *pos, Defs &&defs, size_t paramc, UniqNodes &&body) noexcept:
    Node(pos), defs(std::move(defs)), paramc(paramc), body(std::move(body)) {}
  };

  struct Guard final: Node {
    UniqNode value;
    Guard(const Pos *pos, UniqNode &&value) noexcept: Node(pos), value(std::move(value)) {}
  };

  struct If final: Node {
    UniqNode cond;
    UniqNode then;
    UniqNode elze;
    If(const Pos *pos, UniqNode &&cond, UniqNode &&then, UniqNode &&elze) noexcept:
    Node(pos), cond(std::move(cond)), then(std::move(then)), elze(std::move(elze)) {}
  };

  struct Null final: Node {
    using Node::Node;
  };

  struct NumberLiteral final: Node {
    double value;
    NumberLiteral(const Pos *pos, double value) noexcept: Node(pos), value(value) {}
  };

  struct Return final: Node {
    UniqNode value;
    Return(const Pos *pos, UniqNode &&value) noexcept: Node(pos), value(std::move(value)) {}
  };

  struct Throw final: Node {
    UniqNode value;
    Throw(const Pos *pos, UniqNode &&value) noexcept: Node(pos), value(std::move(value)) {}
  };

  struct Try final: Node, Call {
    Try(const Pos *pos, Call &&call) noexcept: Node(pos), Call(std::move(call)) {}
  };

  template<int N>
  struct Operation: Node {
    std::array<UniqNode, N> items;
    Operation(const Pos *pos, std::array<UniqNode, N> &&items) noexcept: Node(pos), items(std::move(items)) {}
  };

  template<>
  struct Operation<1>: Node {
    UniqNode item;
    Operation(const Pos *pos, UniqNode &&item) noexcept: Node(pos), item(std::move(item)) {}
  };

  template<>
  struct Operation<-1>: Node {
    UniqNodes items;
    Operation(const Pos *pos, UniqNodes &&items) noexcept: Node(pos), items(std::move(items)) {}
  };

  template<int N, int Op>
  struct Operation1 final: Operation<N> {
    static constexpr int operat0r = Op;
    using Operation<N>::Operation;
  };

  // arithmetical operations begin
  using ArithAddOper = Operation1<-1, "+"_token>;
  using ArithSubOper = Operation1<-1, "-"_token>;
  using ArithMulOper = Operation1<-1, "*"_token>;
  using ArithDivOper = Operation1<-1, "/"_token>;
  using ArithModOper = Operation1<-1, "%"_token>;
  using ArithPowOper = Operation1<-1, "**"_token>;
  // arithmetical operations end
  // logical operations begin
  using LogicAndOper = Operation1<-1, "&&"_token>;
  using LogicOrOper = Operation1<-1, "||"_token>;
  using LogicNotOper = Operation1<1, "!"_token>;
  using LogicXorOper = Operation1<-1, "^^"_token>;
  // logical operations end
  // bitwise operations begin
  using BitwsAndOper = Operation1<-1, "&"_token>;
  using BitwsOrOper = Operation1<-1, "|"_token>;
  using BitwsNotOper = Operation1<1, "~"_token>;
  using BitwsXorOper = Operation1<-1, "^"_token>;
  using LshOper = Operation1<-1, "<<"_token>;
  using RshOper = Operation1<-1, ">>"_token>;
  using UshOper = Operation1<-1, ">>>"_token>;
  // bitwise operations end
  // comparisons begin
  using CmpEqOper = Operation1<2, "=="_token>;
  using CmpLtOper = Operation1<2, "<"_token>;
  using CmpGtOper = Operation1<2, ">"_token>;
  using CmpLteqOper = Operation1<2, "<="_token>;
  using CmpGteqOper = Operation1<2, ">="_token>;
  using Cmp3wayOper = Operation1<2, "<=>"_token>;
  // comparisons end
  // signed operations begin
  using PositiveOper = Operation1<1, "+"_token>;
  using NegativeOper = Operation1<1, "-"_token>;
  // signed operations end
  using AssignOper = Operation1<2, "="_token>;
  using GetMembOper = Operation1<-1, "."_token>;
  using LengthOper = Operation1<1, "length"_token>;
  using Sequence = Operation1<-1, ","_token>;

  struct SetMembOper final: Operation<3> {
    using Operation<3>::Operation;
  };
}
