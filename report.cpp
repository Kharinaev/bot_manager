#include <stdio.h>

#include "lexeme.hpp"
#include "report.hpp"

Report::Report(err_type t, lexeme &l)
{
    lex = l;
    type = t;
}

Report::Report(const Report &report)
{
    lex = report.lex;
    type = report.type;
}

void Report::PrintReport() const
{
    if (type == lex_err){
        lex.PrintError();
        return;
    }
    printf("INCORRECT INPUT (line %d, your input: %s):\n\t",
            lex.line, lex.str);
    switch (type) {
        case end_err:
            printf("expected \"end\" at the end of your programm\n");
            break;
        case beg_err:
            printf("expected \"begin\" at the begin of your"
                    " programm\n");
            break;
        case semicolon_endt_err:
            printf("expected \";\" after statement \"endturn;\"\n");    
            break;
        case semicolon_prod_err:
            printf("expected \";\" after statement \"prod NUM;\""
                    " or \"build NUM;\" or \"abuild NUM;\"\n");
            break;
        case semicolon_buy_err:
            printf("expected \";\" after statement \"buy NUM PRICE;\""
                    " or \"sell NUM PRICE;\"\n");
            break;

        case square_br_err:
            printf("expected \"]\" after index of array\n");
            break;
        case asgn_err:
            printf("expected \":=\" after array element"
                    " in assignation\n");
            break;
        case semicolon_asgn1_err:
        case semicolon_asgn2_err:
            printf("expected \";\" after assignation\n");
            break;
        case expected_s_asgn_err:
            printf("expected \":=\" or \"[INDEX]\" after variable in"
                   " assignation\n");
            break;
        case bracket_open_if_err:
            printf("expected \"(\" opening bracket after if"
                    " in IF statement\n");
            break;
        case bracket_close_if_err:
            printf("expected \")\" closing bracket after expression"
                    " in IF statement\n");
            break;
        case bracket_open_while_err:
             printf("expected \"(\" opening bracket after while"
                    " in WHILE statement\n");
             break;
        case bracket_close_while_err:
            printf("expected \")\" closing bracket after expression"
                    " in WHILE statement\n");
            break;
        case expected_print_err:
            printf("expected \";\" or \",\" and next element in"
                    " PRINT statement\n");
            break;
        case bracket_close_exp_err:
            printf("expected \")\" closing bracket after"
                    " expression\n");
            break;
        case bracket_open_func_err:
            printf("expected \"(\" opening bracket after name of"
                    " function (for function with args)\n");
            break;
        case expected_exp_err:
            printf("expected Variable or Number or \"(expression)\""
                    " or Function in expression\n");
            break;
        case bracket_close_func_err:
            printf("expected \")\" closing bracket after argument of"
                    " function\n");
            break;
        case expected_func_err:
            printf("expected argument of function (expression or"
                    " another function) or \")\" if no arguments"
                    " aren needed\n");
            break;
        case not_end_err:
            printf("\"end\" encountered but program didn't end\n");
            break; 
        case expected_op_err:
            printf("expected: \n"
                    "\t-Assignation statement (Variable :="
                    " expression) or (Variable[index] :="
                    " expression)\n"
                    "\t-IF statement (if (expession) operator)\n"
                    "\t-WHILE statement (while (expression operator)\n"
                    "\t-Compound operator ({operator; ...})\n"
                    "\t-Game operators (endturn, prod, build, abuild, "
                    "buy, sell)\n"
                    "\t-Print statement (print [string|expression],...;)\n");
            break;
        case function_err:
            printf("not expected function\n"
                    "try:\n"
                    "?rawAmount\n"
                    "?rawPrice\n"
                    "?prodAmount\n"
                    "?prodPrice\n"
                    "?myId\n"
                    "?players\n"
                    "?money(num)\n"
                    "?raw(num)\n"
                    "?prod(num)\n"
                    "?plants(num)\n"
                    "?aplants(num)\n"
                    "?manufactured(num)\n"
                    "?aucRawPrice(num)\n"
                    "?aucRawBought(num)\n"
                    "?aucProdSold(num)\n"
                    "?aucProdPrice(num)\n"
                    "?turn\n");
            break;
        case semicolon_print:
            printf("expected \";\" after arg of print statement\n");
            break;
        default:
            printf("error num is %d\n",type);
    }
}
