#pragma once

#include"nodes1.hh"

namespace zlt::ilispc {
  struct Reference {
    enum {
      CLOSURE_SCOPE,
      GLOBAL_SCOPE,
      LOCAL_SCOPE
    };
    int scope;
    const std::string *name;
    Reference() = default;
    Reference(int scope, const std::string *name) noexcept: scope(scope), name(name) {}
  };

  struct Function1 final: Node {
    using Defs = Function::Defs;
    using Params = Function::Params;
    using ClosureDefs = std::map<const std::string *, Reference>;
    Defs defs;
    ClosureDefs closureDefs;
    Params params;
    UniqNodes body;
    Function1(const Pos *pos, Defs &&defs, ClosureDefs &&closureDefs, Params &&params, UniqNodes &&body) noexcept:
    Node(pos), defs(std::move(defs)), closureDefs(std::move(closureDefs)), params(std::move(params)), body(std::move(body)) {}
  };

  struct ReferenceNode final: Node, Reference {
    ReferenceNode(const Pos *pos, const Reference &ref) noexcept: Node(pos), Reference(ref) {}
  };
}
