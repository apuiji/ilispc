#pragma once

#include"nodes1.hh"
#include"nodes2.hh"

namespace zlt::ilispc {
  struct MakeRef2 final: Operation<1> {
    using Operation<1>::Operation;
  };

  struct GetRef2Oper final: Operation<1> {
    using Operation<1>::Operation;
  };

  struct SetRef2Oper final: Operation<2> {
    using Operation<2>::Operation;
  };

  struct Function2 final: Node {
    using Defs = std::vector<const std::string *>;
    using ClosureDefs = Function1::ClosureDefs;
    Defs defs;
    ClosureDefs closureDefs;
    UniqNodes body;
    Function2(const Pos *pos, Defs &&defs, ClosureDefs &&closureDefs, UniqNodes &&body) noexcept:
    Node(pos), defs(std::move(defs)), closureDefs(std::move(closureDefs)), body(std::move(body)) {}
  };
}
