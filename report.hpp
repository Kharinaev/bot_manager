enum err_type{
    lex_err, end_err, beg_err, semicolon_endt_err, //3
    semicolon_prod_err, semicolon_buy_err, //5
    expected_op_err, square_br_err, asgn_err, //8
    semicolon_asgn1_err, semicolon_asgn2_err, //10
    expected_s_asgn_err, bracket_open_if_err, //12
    bracket_close_if_err, bracket_open_while_err, //14
    bracket_close_while_err, expected_print_err, //16
    bracket_close_exp_err, bracket_open_func_err, //18
    expected_exp_err, bracket_close_func_err, //20
    expected_func_err, not_end_err, function_err, //23
    semicolon_print //24
};

class Report{ //not written
    lexeme lex;
    err_type type;
public:
    Report(err_type t, lexeme &l);
    Report(const Report &report);
    void PrintReport() const;
}; 