
class ASTElseStatementBuilder : public ASTBase {
private:
  static ASTElseStatementBuilder CB;
  static std::map<unsigned, ASTStatementList *> ElseMap;
  mutable unsigned ISC;

private:
  ASTElseStatementBuilder() = default;

public:
  using map_type = std::map<unsigned, ASTStatementList *>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  static ASTElseStatementBuilder &Instance() { return CB; }

  ASTStatementList *List() const {
    ASTStatementList *SL = new ASTStatementList(++ISC);
    assert(SL && "Could not create an ASTStatementList!");

    if (!ElseMap.insert(std::make_pair(ISC, SL)).second) {
      assert(0 && "Could not create a new ASTStatementList!");
      delete SL;
      --ISC;
      return nullptr;
    }

    return SL;
  }

  ASTStatementList *List(unsigned LI) const {
    const_iterator I = ElseMap.find(LI);
    return I == ElseMap.end() ? nullptr : (*I).second;
  }

  unsigned GetMapIndex() const { return ISC; }

  void Append(ASTStatementNode *SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (!SN->IsDirective())
      ElseMap[ISC]->push(SN);
  }

  virtual ASTType GetASTType() const override { return ASTTypeUndefined; }

  virtual void print() const override {}

  virtual void push(ASTBase * /* unused */) override {}
};
