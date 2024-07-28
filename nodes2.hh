#pragma once

#include"ilispc.hh"

namespace zlt::ilispc {
  struct Reference {
    enum {
      CLOSURE_SCOPE,
      GLOBAL_SCOPE,
      LOCAL_SCOPE
    };
    int scope;
    int index;
    const std::string *name;
    Reference() = default;
    Reference(int scope, int index, const std::string *name) noexcept: scope(scope), index(index), name(name) {}
  };

  struct Function1 final: Node {
    struct Def {
      const std::string *name;
      bool closure;
    };
    using Defs = std::vector<Def>;
    using ClosureDefs = std::vector<Reference>;
    Defs defs;
    ClosureDefs closureDefs;
    size_t paramc;
    UniqNodes body;
    Function1(const Pos *pos, Defs &&defs, ClosureDefs &&closureDefs, size_t paramc, UniqNodes &&body) noexcept:
    Node(pos), defs(std::move(defs)), closureDefs(std::move(closureDefs)), paramc(paramc), body(std::move(body)) {}
  };

  struct ReferenceNode final: Node, Reference {
    ReferenceNode(const Pos *pos, const Reference &ref) noexcept: Node(pos), Reference(ref) {}
  };
}
