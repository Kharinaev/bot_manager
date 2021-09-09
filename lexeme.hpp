enum lex_type{
    unknown, delimiter, num_const, str_const, variable, label, 
    function, key_word, assign
}; 

enum lex_answer_t{
    good, empty, end_of_file,l_num_err, l_beg_err, l_iden_err, 
    l_asgn_err, l_str_err,
    l_key_err, l_label_err, l_endlab_err
};
 
struct lexeme{
    lex_answer_t answer;
    char *str;
    int line;
    bool str_maked;
    lex_type type;

    lexeme();
    void Print() const;
    lexeme& operator=(const lexeme& cur);
    ~lexeme();
    void Clear();
    lexeme(const lexeme &l);
    bool IsError() const;
    void PrintError() const;
};
