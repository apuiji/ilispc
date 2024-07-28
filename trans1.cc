#include"nodes.hh"
#include"nodes1.hh"
#include"nodes2.hh"

using namespace std;

namespace zlt::ilispc {
  struct Scope {
    Scope *parent;
    Function1::Defs defs;
    Function1::ClosureDefs closureDefs;
    Scope(Scope *parent, size_t defc) noexcept: parent(parent), defs(defc) {}
  };

  static void trans1(Scope *scope, UniqNode &src);

  using It = UniqNodes::iterator;

  static inline void trans1(Scope *scope, It it, It end) {
    for (; it != end; ++it) {
      trans1(scope, *it);
    }
  }

  void trans1(UniqNodes &src) {
    trans1(nullptr, src.begin(), src.end());
  }

  static Reference findRef(Scope *scope, bool closure, const string *name);
  static void transCall(Scope *scope, Call &call);

  using ItDef = Function1::Defs::iterator;

  static inline void copyDefs(Function::Defs::iterator it, Function::Defs::iterator end, ItDef outIt) noexcept {
    for (; it != end; ++it, ++outIt) {
      outIt->name = *it;
    }
  }

  void trans1(Scope *scope, UniqNode &src) {
    if (auto a = dynamic_cast<ID *>(src.get()); a) {
      auto ref = findRef(scope, false, a->raw);
      src.reset(new ReferenceNode(src->pos, ref));
    } else if (auto a = dynamic_cast<CallNode *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Defer *>(src.get()); a) {
      trans1(scope, a->value);
    } else if (auto a = dynamic_cast<Forward *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Function *>(src.get()); a) {
      Scope scope1(scope, a->defs.size());
      copyDefs(a->defs.begin(), a->defs.end(), scope1.defs.begin());
      trans1(&scope1, a->body.begin(), a->body.end());
      src.reset(new Function1(src->pos, std::move(scope1.defs), std::move(scope1.closureDefs), a->paramc, std::move(a->body)));
    } else if (auto a = dynamic_cast<Guard *>(src.get()); a) {
      trans1(scope, a->value);
    } else if (auto a = dynamic_cast<If *>(src.get()); a) {
      trans1(scope, a->cond);
      trans1(scope, a->then);
      trans1(scope, a->elze);
    } else if (auto a = dynamic_cast<Return *>(src.get()); a) {
      trans1(scope, a->value);
    } else if (auto a = dynamic_cast<Throw *>(src.get()); a) {
      trans1(scope, a->value);
    } else if (auto a = dynamic_cast<Try *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Operation<1> *>(src.get()); a) {
      trans1(scope, a->item);
    } else if (auto a = dynamic_cast<Operation<2> *>(src.get()); a) {
      trans1(scope, a->items[0]);
      trans1(scope, a->items[1]);
    } else if (auto a = dynamic_cast<Operation<3> *>(src.get()); a) {
      trans1(scope, a->items[0]);
      trans1(scope, a->items[1]);
      trans1(scope, a->items[2]);
    } else if (auto a = dynamic_cast<Operation<-1> *>(src.get()); a) {
      trans1(scope, a->items.begin(), a->items.end());
    }
  }

  static ItDef findDef(ItDef it, ItDef end, const string *name) noexcept;

  using ItClosureDef = Function1::ClosureDefs::iterator;

  static ItClosureDef findClosureDef(ItClosureDef it, ItClosureDef end, const string *name) noexcept;

  Reference findRef(Scope *scope, bool closure, const string *name) {
    if (!scope) [[unlikely]] {
      return Reference(Reference::GLOBAL_SCOPE, 0, name);
    }
    if (auto it = findDef(scope->defs.begin(), scope->defs.end(), name); it != scope->defs.end()) {
      if (closure) {
        it->closure = true;
      }
      return Reference(Reference::LOCAL_SCOPE, it - scope->defs.begin(), name);
    }
    auto &closureDefs = scope->closureDefs;
    if (auto it = findClosureDef(closureDefs.begin(), closureDefs.end(), name); it != closureDefs.end()) {
      return Reference(Reference::CLOSURE_SCOPE, it - scope->closureDefs.begin(), name);
    }
    Reference ref = findRef(scope->parent, true, name);
    if (ref.scope == Reference::GLOBAL_SCOPE) {
      return ref;
    }
    int i = (int) closureDefs.size();
    closureDefs.push_back(ref);
    return Reference(Reference::CLOSURE_SCOPE, i, name);
  }

  ItDef findDef(ItDef it, ItDef end, const string *name) noexcept {
    for (; it != end; ++it) {
      if (it->name == name) {
        return it;
      }
    }
    return end;
  }

  ItClosureDef findClosureDef(ItClosureDef it, ItClosureDef end, const string *name) noexcept {
    for (; it != end; ++it) {
      if (it->name == name) {
        return it;
      }
    }
    return end;
  }

  void transCall(Scope *scope, Call &call) {
    trans1(scope, call.callee);
    trans1(scope, call.args.begin(), call.args.end());
  }
}
