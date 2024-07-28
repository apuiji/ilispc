#include"nodes.hh"

using namespace std;

namespace zlt::ilispc::preproc {
  using It = UniqNodes::const_iterator;

  struct Context1 {
    map<const string *, It> map;
    It end;
  };

  using ItParam = Macro::Params::const_iterator;

  static void makeContext1(Context1 &dest, ItParam itParam, ItParam endParam, It it, It end);
  static void expand1(UniqNodes &dest, Context &ctx, const Context1 &ctx1, const Pos *prevPos, const UniqNode &src);

  static inline void expands(UniqNodes &dest, Context &ctx, const Context1 &ctx1, const Pos *prevPos, It it, It end) {
    for (; it != end; ++it) {
      expand1(dest, ctx, ctx1, prevPos, *it);
    }
  }

  void expand(UniqNodes &dest, Context &ctx, const Pos *pos, const Macro &macro, UniqNodes &src) {
    Context1 ctx1;
    makeContext1(ctx1, macro.params.begin(), macro.params.end(), src.begin(), src.end());
    expands(dest, ctx, ctx1, pos, macro.body.begin(), macro.body.end());
  }

  void makeContext1(Context1 &dest, ItParam itParam, ItParam endParam, It it, It end) {
    for (; itParam != endParam && it != end; ++itParam, ++it) {
      if (*itParam) {
        dest.map[*itParam] = it;
      }
    }
    for (; itParam != endParam; ++itParam) {
      if (*itParam) {
        dest.map[*itParam] = end;
      }
    }
    dest.end = end;
  }

  void expand1(UniqNodes &dest, Context &ctx, const Context1 &ctx1, const Pos *prevPos, const UniqNode &src) {
    if (auto nl = dynamic_cast<const NumberNode *>(src.get()); nl) {
      auto itPos = ctx.posSet.insert(Pos(src->pos->file, src->pos->li, prevPos)).first;
      dest.push_back({});
      dest.back().reset(new NumberNode(&*itPos, nl->value, nl->raw));
      return;
    }
    if (auto sl = dynamic_cast<const StringLiteral *>(src.get()); sl) {
      auto itPos = ctx.posSet.insert(Pos(src->pos->file, src->pos->li, prevPos)).first;
      dest.push_back({});
      dest.back().reset(new StringLiteral(&*itPos, sl->value));
      return;
    }
    if (auto id = dynamic_cast<const ID *>(src.get()); id) {
      auto itIt = ctx1.map.find(id->raw);
      if (itIt == ctx1.map.end()) {
        auto itPos = ctx.posSet.insert(Pos(src->pos->file, src->pos->li, prevPos)).first;
        dest.push_back({});
        dest.back().reset(new ID(&*itPos, id->raw));
        return;
      }
      It it1 = itIt->second;
      if (string_view(*id->raw).starts_with("...")) {
        expands(dest, ctx, ctx1, prevPos, it1, ctx1.end);
      } else {
        expand1(dest, ctx, ctx1, prevPos, *it1);
      }
      return;
    }
    if (auto ls = dynamic_cast<const ListNode *>(src.get()); ls) {
      UniqNodes items;
      expands(items, ctx, ctx1, prevPos, ls->items.begin(), ls->items.end());
      auto itPos = ctx.posSet.insert(Pos(src->pos->file, src->pos->li, prevPos)).first;
      dest.push_back({});
      dest.back().reset(new ListNode(&*itPos, std::move(items)));
      return;
    }
    auto tn = dynamic_cast<const TokenNode *>(src.get());
    auto itPos = ctx.posSet.insert(Pos(src->pos->file, src->pos->li, prevPos)).first;
    dest.push_back({});
    dest.back().reset(new TokenNode(&*itPos, tn->token));
  }
}
