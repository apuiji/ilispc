#include"ilispc.hh"
#include"nodes.hh"
#include"token.hh"

using namespace std;

namespace zlt::ilispc {
  using Context = ParseContext;
  using It = const char *;

  static pair<It, const Pos *> nodes(UniqNodes &dest, Context &ctx, const Pos *pos, It it, It end);

  void parse(UniqNodes &dest, Context &ctx, const Pos *pos, It it, It end) {
    auto [end1, pos2] = nodes(dest, ctx, pos, it, end);
    It start2 = hit(end1, end);
    auto [t2, end2] = lexer(pos2, start2, end);
    if (t2 != token::E0F) {
      throw Bad(bad::UNEXPECTED_TOKEN, pos2);
    }
  }

  static pair<It, const Pos *> node(UniqNode &dest, Context &ctx, const Pos *pos, It it, It end);

  pair<It, const Pos *> nodes(UniqNodes &dest, Context &ctx, const Pos *pos, It it, It end) {
    UniqNode node1;
    auto [end1, pos1] = node(node1, ctx, pos, it, end);
    if (!end1) {
      return { it, pos };
    }
    dest.push_back(std::move(node1));
    return nodes(dest, ctx, pos1, end1, end);
  }

  static pair<It, const Pos *> lizt(UniqNode &dest, Context &ctx, const Pos *pos, It it, It end);

  pair<It, const Pos *> node(UniqNode &dest, Context &ctx, const Pos *pos, It it, It end) {
    It start1 = hit(it, end);
    double numval;
    string strval;
    string_view raw;
    auto [t1, end1] = lexer(numval, strval, raw, pos, start1, end);
    if (t1 == token::E0F || t1 == ")"_token) [[unlikely]] {
      return {};
    }
    if (t1 == token::EOL) {
      auto itPos = ctx.posSet.insert(Pos(pos->file, pos->li + 1, pos->prev)).first;
      return node(dest, ctx, &*itPos, end1, end);
    }
    if (t1 == token::NUMBER) {
      auto itRaw = ctx.strs.insert(string(raw)).first;
      dest.reset(new NumberNode(pos, numval, &*itRaw));
      return { end1, pos };
    }
    if (t1 == token::STRING) {
      auto itStr = ctx.strs.insert(std::move(strval)).first;
      dest.reset(new StringLiteral(pos, &*itStr));
      return { end1, pos };
    }
    if (t1 == token::ID) {
      auto itRaw = ctx.strs.insert(string(raw)).first;
      dest.reset(new ID(pos, &*itRaw));
      return { end1, pos };
    }
    if (t1 == "("_token) {
      return lizt(dest, ctx, pos, end1, end);
    }
    dest.reset(new TokenNode(pos, t1));
    return { end1, pos };
  }

  pair<It, const Pos *> lizt(UniqNode &dest, Context &ctx, const Pos *pos, It it, It end) {
    UniqNodes items;
    auto [end1, pos1] = nodes(items, ctx, pos, it, end);
    It start2 = hit(end1, end);
    auto [t2, end2] = lexer(pos1, start2, end);
    if (t2 != ")"_token) {
      throw Bad(bad::UNTERMINATED_LIST, pos);
    }
    dest.reset(new ListNode(pos, std::move(items)));
    return { end2, pos1 };
  }
}
