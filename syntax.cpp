#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "lexeme.hpp"
#include "lexer.hpp"
#include "report.hpp"
#include "rpn.hpp"
#include "syntax.hpp"


#ifdef DEBUG
    #define DPRINT(x) do {printf x;} while (0)
#else
    #define DPRINT(x) do {} while(0)
#endif

/*class EndOfFile{};

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
};*/

void Synt::GetRpn(RpnItem *&beg, RpnItem *&end)
{
    beg = rpn_beg;
    end = rpn_end;
}

Synt::Synt()
{
    rpn_beg = new RpnItem;
    rpn_beg->elem = new RpnNoOp;
    rpn_beg->next = 0;
    rpn_end = rpn_beg;
}

void Synt::PrintItem(FILE* fd, RpnItem *src) const
{
    RpnItem *tmp = src;
    while (tmp != 0){
        if (tmp->elem != 0)
            tmp->elem->Print(fd);
        else
            fprintf(fd, "(null elem)__");
        tmp = tmp->next;
    }
} 

void Synt::AddToRpn(RpnElem *elem_add)
{
    rpn_end->next = new RpnItem;
    rpn_end = rpn_end->next;
    rpn_end->next = 0;
    rpn_end->elem = elem_add;
}

bool Synt::FuncNoArg()
{
    return (strcmp(lex.str, "?rawAmount")==0) ||
            (strcmp(lex.str, "?rawPrice")==0) ||
            (strcmp(lex.str, "?prodAmount")==0) ||
            (strcmp(lex.str, "?prodPrice")==0) ||
            (strcmp(lex.str, "?myId")==0) ||
            (strcmp(lex.str, "?players")==0) ||
            (strcmp(lex.str, "?turn")==0);
}

bool Synt::CurLexType(lex_type type)
{
    bool ans = false;
    if (type == function && lex.type == function){
        ans = (strcmp(lex.str, "?rawAmount")==0) ||
                (strcmp(lex.str, "?rawPrice")==0) ||
                (strcmp(lex.str, "?prodAmount")==0) ||
                (strcmp(lex.str, "?prodPrice")==0) ||
                (strcmp(lex.str, "?myId")==0) ||
                (strcmp(lex.str, "?players")==0) ||
                (strcmp(lex.str, "?money")==0) ||
                (strcmp(lex.str, "?raw")==0) ||
                (strcmp(lex.str, "?prod")==0) ||
                (strcmp(lex.str, "?plants")==0) ||
                (strcmp(lex.str, "?aplants")==0) ||
                (strcmp(lex.str, "?manufactured")==0) ||
                (strcmp(lex.str, "?aucRawPrice")==0) ||
                (strcmp(lex.str, "?aucRawBought")==0) ||
                (strcmp(lex.str, "?aucProdPrice")==0) ||
                (strcmp(lex.str, "?aucProdSold")==0) ||
                (strcmp(lex.str, "?turn")==0) ||
                (strcmp(lex.str, "?random")==0);
        if (!ans)
            throw Report(function_err, lex);
    } 
    return lex.type == type;
}

bool Synt::CurLexEq(const char *str)
{
    return (strcmp(str, lex.str) == 0);
}

void Synt::S()
{
    DPRINT(("~~~S %s\n",lex.str));
    if (CurLexEq("begin")){
        GetNext();
        OP();
        GetNext();
        if (CurLexEq("end")){
            GetNext();
            throw Report(not_end_err, lex);
            return;
        }else
            throw Report(end_err, lex);    
    } else 
        throw Report(beg_err, lex);
}

void Synt::OP()
{
    DPRINT(("~~~OP %s\n",lex.str));
    if (CurLexType(variable)){
        AddToRpn(new RpnVar(lex.str));
        GetNext();
        R_ASGN();
        return;
    } else
    if (CurLexEq("if")){
        GetNext();
        R_IF();
        return;
    } else 
    if (CurLexEq("while")){
        GetNext();
        R_WHILE();
        return;
    } else
    if (CurLexEq("{")){
        GetNext();
        R_COM();
        return;
    } else
    if (CurLexEq("endturn")){
        AddToRpn(new RpnFuncGameOpNone);
        GetNext();
        if (!CurLexEq(";"))
            throw Report(semicolon_endt_err, lex);
        AddToRpn(new RpnSemicolon);
        return;
    } else 
    if (CurLexEq("prod") || CurLexEq("build") || CurLexEq("abuild")){
        char c = lex.str[0];
        GetNext();
        EXP();
        if (!CurLexEq(";")){
            throw Report(semicolon_prod_err, lex);
        }
        switch (c){
        case 'p':
            AddToRpn(new RpnFuncProd);
            break;
        case 'b':
            AddToRpn(new RpnFuncBuild);
            break;
        case 'a':
            AddToRpn(new RpnFuncABuild);
            break;
        }
        AddToRpn(new RpnSemicolon);
        return;
    } else
    if (CurLexEq("buy") || CurLexEq("sell")){
        char cr = lex.str[0];
        GetNext();
        EXP();
        EXP();
        if (!CurLexEq(";"))
            throw Report(semicolon_buy_err, lex);
        if (cr == 'b')
            AddToRpn(new RpnFuncBuy);
        else
            AddToRpn(new RpnFuncSell);
        AddToRpn(new RpnSemicolon);
        return;
    } else
    if (CurLexEq("print")){
        GetNext();
        R_PRINT();
        return;
    } else
        throw Report(expected_op_err, lex);
}        
        
void Synt::R_ASGN()
{
    if (CurLexEq("[")){
        GetNext();
        EXP();
        if (!CurLexEq("]"))
            throw Report(square_br_err, lex);
        GetNext();
        if (!CurLexEq(":="))
            throw Report(asgn_err, lex);
        GetNext();
        EXP();
        AddToRpn(new RpnVarIndex);
        AddToRpn(new RpnFuncAsgn);
        if (!CurLexEq(";"))
            throw Report(semicolon_asgn1_err, lex);
        AddToRpn(new RpnSemicolon);
        return;
    } else
    if (CurLexEq(":=")){
        GetNext();
        EXP();
        AddToRpn(new RpnFuncAsgn);
        if (!CurLexEq(";"))
            throw Report(semicolon_asgn2_err, lex);
        AddToRpn(new RpnSemicolon);
        return;
    } else
        throw Report(expected_s_asgn_err, lex);
}

void Synt::R_IF()
{
    DPRINT(("~~~R_IF %s\n",lex.str));
    if (!CurLexEq("("))
        throw Report(bracket_open_if_err, lex);
    GetNext();
    EXP();
    if (!CurLexEq(")"))
        throw Report(bracket_close_if_err, lex);
    RpnLabel *lab_if = new RpnLabel;
    AddToRpn(lab_if);
    AddToRpn(new RpnGoFalse);
    GetNext();
    OP();
    AddToRpn(new RpnNoOp);
    lab_if->Set(rpn_end);
}

void Synt::R_WHILE()
{
    DPRINT(("~~~R_WHILE %s\n",lex.str));
    if (!CurLexEq("("))
        throw Report(bracket_open_while_err, lex);
    RpnItem *bef_exp = rpn_end;
    GetNext();
    EXP();
    RpnLabel *lab_while = new RpnLabel;
    AddToRpn(lab_while);
    AddToRpn(new RpnGoFalse);
    if (!CurLexEq(")"))
        throw Report(bracket_close_while_err, lex);
    GetNext();
    OP();
    AddToRpn(new RpnLabel(bef_exp));
    AddToRpn(new RpnGo);
    AddToRpn(new RpnNoOp);
    lab_while->Set(rpn_end);
}    

void Synt::R_COM()
{
    DPRINT(("~~~R_COM %s\n",lex.str));
    OP();
    GetNext();
    if (CurLexEq("}"))
        return;
    else
        R_COM();
}
    
void Synt::R_PRINT()
{
    if (CurLexType(str_const)){
        AddToRpn(new RpnString(lex.str));
        AddToRpn(new RpnPrint);
        GetNext();
        if (!CurLexEq(";"))
            throw Report(semicolon_print, lex);
        AddToRpn(new RpnSemicolon);
    } else {
        EXP();
        AddToRpn(new RpnPrint);
        if (!CurLexEq(";"))
            throw Report(semicolon_print, lex);
        AddToRpn(new RpnSemicolon);
    }
}
        
void Synt::EXP()
{
    E_1();
    DPRINT(("~~~EXP %s\n",lex.str));
    if (CurLexEq("|")){
        GetNext();
        EXP();
        AddToRpn(new RpnFuncOr);
    }
}

void Synt::E_1()
{
    E_2();
    if (CurLexEq("&")){
        GetNext();
        E_1();
        AddToRpn(new RpnFuncAnd);
    }
}

void Synt::E_2()
{
    DPRINT(("~~~E_2 %s\n",lex.str));   
    E_3();
    if (CurLexEq("<") || CurLexEq(">") || CurLexEq("=")){
        char sign = lex.str[0];
        GetNext();
        E_2();
        if (sign == '<')
            AddToRpn(new RpnFuncLess);
        else {
            if (sign == '>')
                AddToRpn(new RpnFuncGreater);
            else 
                AddToRpn(new RpnFuncEqual);
        }
    }
}

void Synt::E_3()
{
    E_4();
    if (CurLexEq("+") || CurLexEq("-")){
        char sign = lex.str[0];
        GetNext();
        E_3();
        if (sign == '+')
            AddToRpn(new RpnFuncPlus);
        else
            AddToRpn(new RpnFuncMinus);
    }
}

void Synt::E_4()
{
    E_5();
    if (CurLexEq("*") || CurLexEq("/") || CurLexEq("%")){
        char sign = lex.str[0];
        GetNext();
        E_4();
        if (sign == '*')
            AddToRpn(new RpnFuncMult);
        else {
            if (sign == '/')
                AddToRpn(new RpnFuncDiv);
            else 
                AddToRpn(new RpnFuncMod);
        }
    }    
}

void Synt::E_5()
{
    if (CurLexEq("!") || CurLexEq("-")){
        char sign = lex.str[0];
        GetNext();
        E_5();
        if (sign == '!')
            AddToRpn(new RpnFuncNot);
        else
            AddToRpn(new RpnFuncUnMin);
    } else 
        E_6();
}

void Synt::E_6()
{
    DPRINT(("~~~E_6 %s\n",lex.str));
    if (CurLexType(variable)){
        AddToRpn(new RpnVar(lex.str));
        GetNext();
        R_VAR();
        AddToRpn(new RpnFuncVarVal);
        return;
    } else
    if (CurLexType(num_const)){
        AddToRpn(new RpnInt(lex.str));
        GetNext();
        return;
    } else
    if (CurLexEq("(")){
        GetNext();
        EXP();
        if (!CurLexEq(")"))
            throw Report(bracket_close_exp_err, lex);
        GetNext();
    } else 
    if (CurLexType(function)){
        char *name = new char [strlen(lex.str)+1];
        strcpy(name, lex.str);
        if(FuncNoArg()){
            AddToRpn(new RpnGameFuncNoArg(name));
            GetNext();
            return;
        }
        GetNext();
        if (!CurLexEq("("))
            throw Report(bracket_open_func_err, lex);
        GetNext();
        EXP();
        AddToRpn(new RpnGameFuncOneArg(name));
        if (!CurLexEq(")"))
            throw Report(bracket_close_func_err, lex);
        GetNext();
        delete [] name;
    } else
        throw Report(expected_exp_err, lex);
}

void Synt::R_VAR()
{
    if (CurLexEq("[")){
        GetNext();
        EXP();
        if (!CurLexEq("]"))
            throw Report(square_br_err, lex);
        AddToRpn(new RpnVarIndex);
        GetNext();
    }
}

void Synt::GetNext()
{
    int c;
    while (lex.answer != good){
        lex.Clear();
        c = getchar();
        avt.FeedChar(c);
        avt.CheckLexeme(lex);
        if (lex.IsError())
            throw Report(lex_err, lex);
        if (lex.answer == end_of_file)
            throw EndOfFile();
//#ifdef LEXPRINT
        lex.Print();
//#endif
    }
    lex.answer = empty;
}   
 
void Synt::Analyze()
{
    GetNext();
    S();
}   

/*class Rpn{
    RpnItem *beg, *end;
    RpnItem *stack;
    VarTable *v_table;
    Synt synt;
public:
    Rpn(){
        stack = 0;
        beg = 0;
        end = 0;
        v_table = new VarTable;
    }
    void Eval();
    void PrintItem(FILE *fd, RpnItem *src) const;
    ~Rpn();
    void Analyze(){
        synt.Analyze();
    }
};*/

Rpn::Rpn()
{
    stack = 0;
    beg = 0;
    end = 0;
    v_table = new VarTable;
}

void Rpn::Analyze()
{
    synt.Analyze();
}

Rpn::~Rpn()
{
    while (beg != 0){
        RpnItem *tmp = beg;
        beg = beg->next;
        delete tmp->elem;
        delete tmp;
    }
    delete v_table;
}

void Rpn::PrintItem(FILE* fd, RpnItem *src) const
{
    RpnItem *tmp = src;
    while (tmp != 0){
        if (tmp->elem != 0)
            tmp->elem->Print(fd);
        else
            fprintf(fd, "(null elem)__");
        tmp = tmp->next;
    }
}

//#define STACKPRINT
#define RPNPRINT

void Rpn::Eval(Bot *bot)
{
    synt.GetRpn(beg,end);
    RpnItem *cur_cmd = beg;
#ifdef RPNPRINT
        PrintItem(stdout,beg);
#endif
    try {
        while (cur_cmd != 0){
            cur_cmd->elem->Evaluate(stack,cur_cmd,v_table,bot);
#ifdef STACKPRINT
            PrintItem(stdout,stack);
            printf("\n\n");
#endif
        }
#ifdef VARTABLEPRINT
        v_table->Print(stdout);     
#endif
        printf("\nProgram completed\n\n");
    }
    catch (RpnExDivNull ex){
        ex.Print(stderr);
        ex.Free();
    }
    catch (RpnExNotVar ex){
        ex.Print(stderr);
        ex.Free();
    }
    catch (RpnExNotNum ex){
        ex.Print(stderr);
        ex.Free();
    }
    catch (RpnExNoValue ex){
        ex.Print(stderr);
        ex.Free();
    }
    catch (RpnExNotLabel ex){
        ex.Print(stderr);
        ex.Free();
    }
    catch (RpnExNotInt ex){
        ex.Print(stderr);
        ex.Free();
    }
    catch (RpnExPrint ex){
        ex.Print(stderr);
        ex.Free();
    }

};

void Rpn::Start(Bot *bot){
    try {
        if (beg == 0)
            Analyze();
        else
            throw EndOfFile();
    }
    catch (Report rep){
        rep.PrintReport();
    }
    catch (EndOfFile end){
        //printf("\nText meets the grammar requirements\n\n");
        Eval(bot);
    }
}

/*int main(int argc, char **argv)
{
    int fd;
    Rpn rpn;
    if (argc != 2){
        fprintf(stderr, "incorrect args: <file.txt>\n"); 
        return 0;
    }
    fd = open(argv[1], O_RDONLY);
    if (fd == -1){
        perror(argv[1]);
        return 0;
    }
    dup2(fd,0);
    close(fd);
    start(&rpn);
    return 0;
}*/
