class ASTElseIfStatementBuilder : public ASTBase {
private:
  static ASTElseIfStatementBuilder CB;
  static std::map<unsigned, ASTStatementList*> ElseIfMap;
  mutable unsigned ISC;

private:
  ASTElseIfStatementBuilder() = default;

public:
  using map_type = std::map<unsigned, ASTStatementList*>;
  using iterator = typename map_type::iterator;
  using const_iterator = typename map_type::const_iterator;

public:
  static ASTElseIfStatementBuilder& Instance() {
    return CB;
  }

  ASTStatementList* List() const {
    ASTStatementList* SL = new ASTStatementList(++ISC);
    assert(SL && "Could not create an ASTStatementList!");

    if (!ElseIfMap.insert(std::make_pair(ISC, SL)).second) {
      assert(0 && "Could not create a new ASTStatementList!");
      delete SL;
      --ISC;
      return nullptr;
    }

    return SL;
  }

  ASTStatementList* List(unsigned LI) const {
    const_iterator I = ElseIfMap.find(LI);
    return I == ElseIfMap.end() ? nullptr : (*I).second;
  }

  unsigned GetMapIndex() const {
    return ISC;
  }

  void Append(ASTStatementNode* SN) {
    assert(SN && "Invalid ASTStatementNode argument!");
    if (!SN->IsDirective())
      ElseIfMap[ISC]->push(SN);
  }

  virtual ASTType GetASTType() const override {
    return ASTTypeUndefined;
  }

  virtual void print() const override { }

  virtual void push(ASTBase* /* unused */) override { }
};

