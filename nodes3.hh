#pragma once

#include"nodes.hh"
#include"nodes1.hh"
#include"nodes2.hh"

namespace zlt::ilispc {
  struct GetRef2Oper final: Operation<1> {
    using Operation<1>::Operation;
  };

  struct SetRef2Oper final: Operation<2> {
    using Operation<2>::Operation;
  };

  struct Function2 final: Node {
    using Defs = Function1::Defs;
    using ClosureDefs = Function1::ClosureDefs;
    Defs defs;
    ClosureDefs closureDefs;
    size_t paramc;
    UniqNodes body;
    bool hasGuard;
    Function2(const Pos *pos, Defs &&defs, ClosureDefs &&closureDefs, size_t paramc, UniqNodes &&body, bool hasGuard) noexcept:
    Node(pos),
    defs(std::move(defs)),
    closureDefs(std::move(closureDefs)),
    paramc(paramc),
    body(std::move(body)),
    hasGuard(hasGuard) {}
  };
}
