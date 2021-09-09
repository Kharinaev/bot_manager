class Bot;
class EndOfFile{};

class Synt{
    lexeme lex;
    AutoCl avt;
    RpnItem *rpn_beg, *rpn_end;

    void GetNext();
    void S();
    void OP();
    void R_ASGN();
    void R_IF();
    void R_WHILE();
    void R_COM();
    void R_PRINT();
    void EXP();
    void E_1();
    void E_2();
    void E_3();
    void E_4();
    void E_5();
    void E_6();
    void R_FUNC();
    void R_VAR();
    bool CurLexType(lex_type type);
    bool CurLexEq(const char *str);
    bool FuncNoArg();
    void AddToRpn(RpnElem *elem_add);
public:
    Synt();
    void Analyze();
    void PrintItem(FILE* fd, RpnItem *src) const;
    void GetRpn(RpnItem *&beg, RpnItem *&end);
};

class Rpn{
    RpnItem *beg, *end;
    RpnItem *stack;
    VarTable *v_table;
    Synt synt;
public:
    Rpn();
    void Eval(Bot *bot);
    void PrintItem(FILE *fd, RpnItem *src) const;
    ~Rpn();
    void Analyze();
    void Start(Bot *bot);
};

//void start(Rpn *rpn);