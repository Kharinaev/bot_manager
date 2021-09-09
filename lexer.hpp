class AutoCl{
    int line_num, buf_size, buf_ptr;
    lexeme lex;
    char *buf, res;
    bool lex_ready;
    enum state_t {
        s_err, s_beg, s_num, s_str, s_iden, s_asgn, s_key, s_label, 
        s_end_label
    } state;
    lex_type type;
    void ChangeState(int c);
    void ChangeFromBegin(int c);
    void ChangeFromNum(int c);
    void ChangeFromStr(int c);
    void ChangeFromIden(int c);
    void ChangeFromAssign(int c);
    void ChangeFromKey(int c);
    void ChangeFromLabel(int c);
    void ChangeFromEndLabel(int c);
    void PutInBuf(int c);
    void DefineType(int c, lex_type &type);
    void MakeLexSymb(int c);
    void MakeLexBuf();
public:
    AutoCl();
    void FeedChar(int c);
    void CheckLexeme(lexeme &l);
    ~AutoCl();
};
