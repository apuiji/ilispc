#include"nodes.hh"
#include"nodes2.hh"

using namespace std;

namespace zlt::ilispc {
  struct Scope {
    Scope *parent;
    Function::Defs &defs;
    map<const string *, Reference> closureDefs;
    Scope(Scope *parent, Function::Defs &defs) noexcept: parent(parent), defs(defs) {}
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

  static int findRefScope(Scope *scope, const string *name) noexcept;
  static void transCall(Scope *scope, Call &call);

  void trans1(Scope *scope, UniqNode &src) {
    if (auto a = dynamic_cast<ID *>(src.get()); a) {
      int s = findRefScope(scope, a->raw);
      src.reset(new ReferenceNode(src->pos, Reference(s, a->raw)));
    } else if (auto a = dynamic_cast<CallNode *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Defer *>(src.get()); a) {
      trans1(scope, a->value);
    } else if (auto a = dynamic_cast<Forward *>(src.get()); a) {
      transCall(scope, *a);
    } else if (auto a = dynamic_cast<Function *>(src.get()); a) {
      Scope scope1(scope, a->defs);
      trans1(&scope1, a->body.begin(), a->body.end());
      src.reset(
        new Function1(src->pos, std::move(a->defs), std::move(scope1.closureDefs), std::move(a->params), std::move(a->body)));
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

  int findRefScope(Scope *scope, const string *name) noexcept {
    if (!scope) [[unlikely]] {
      return Reference::GLOBAL_SCOPE;
    }
    if (scope->defs.find(name) != scope->defs.end()) {
      return Reference::LOCAL_SCOPE;
    }
    if (scope->closureDefs.find(name) != scope->closureDefs.end()) {
      return Reference::CLOSURE_SCOPE;
    }
    int s = findRefScope(scope->parent, name);
    if (s == Reference::GLOBAL_SCOPE) {
      return Reference::GLOBAL_SCOPE;
    }
    scope->closureDefs[name] = Reference(s, name);
    return Reference::CLOSURE_SCOPE;
  }

  void transCall(Scope *scope, Call &call) {
    trans1(scope, call.callee);
    trans1(scope, call.args.begin(), call.args.end());
  }
}
