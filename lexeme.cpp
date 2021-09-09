#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "lexeme.hpp"

lexeme::lexeme()
{
    line = 0;
    type = unknown;
    answer = empty;
    str_maked = false;
}

lexeme::lexeme(const lexeme &l)
{
    *this = l; 
    str = new char [strlen(l.str) + 1];
    strcpy(str, l.str);
}

lexeme& lexeme::operator=(const lexeme& cur)
{
    if (this == &cur)
        return *this;
    if (str_maked){
        delete [] str; 
    }
    str = new char [strlen(cur.str) + 1];
    strcpy(str, cur.str);
    answer = cur.answer;
    line = cur.line;
    str_maked = cur.str_maked;
    type = cur.type;
    return *this;
}

void lexeme::Clear()
{
    if (str_maked){
        delete [] str;
        str_maked = false;
    }
    line = 0;
    type = unknown;
    answer = empty;
}

void lexeme::Print() const
{
    if (answer == good){
        printf("LEXEME: %20s | LINE: %3.0d | TYPE: ", str, line);
        switch (type){
            case delimiter:
                printf("delimiter\n");
                break;
            case num_const:
                printf("number constant\n");
                break;
            case str_const:
                printf("string constant\n");
                break;
            case variable:
                printf("variable\n");
                break;
            case label:
                printf("label\n");
                break;
            case function:
                printf("function\n");
                break;
            case key_word:
                printf("key word\n");
                break;
            case assign:
                printf("assignation\n");
                break;
            case unknown:
                printf("unknown type ERROR\n");
        }
    }
}

lexeme::~lexeme()
{
    if (str_maked){
        delete [] str;
        str_maked = false;
    }
}

bool lexeme::IsError() const
{
    return answer == l_num_err || answer == l_beg_err || 
            answer == l_iden_err || answer == l_asgn_err ||
            answer == l_key_err || answer == l_label_err ||
            answer == l_endlab_err || answer == l_str_err;
}

void lexeme::PrintError() const
{
    printf("lexical error (line %d):\n\t",line);
    switch (answer){
        case l_num_err:
            printf("expected only number characters 0..9 in"
                    " Number Constant\n");
            break;
        case l_beg_err:
            printf("expected\n"
                    "\t       0..9 for Number Constant\n"
                    "\t         \"  for String Constant\n"
                    "\t          $ for Variable\n"
                    "\t          ? for Function\n"
                    "\t          @ for Label\n"
                    "\t          : for Assignation\n"
                    "\t a..z, A..Z for Key Word\n"
                    "\t + - * / %c < > = & | ! ( ) [ ] ; , { }\n"
                    "\t            for Delimiter\n",'%');
            break;
        case l_iden_err:
            printf("expected only letters and numbers in the name"
                    " of Functions and Variables\n");
            break;
        case l_asgn_err:
            printf("expected \"=\" after \":\" in Assignation\n");
            break;
        case l_key_err:
            printf("expected only letters in Key Word\n");
            break;
        case l_label_err:
            printf("expected only letters and numbers in Label"
                    " or \":\" as end of Label\n");
            break;
        case l_endlab_err:
            printf("expected *Space* or *Tab* or *Enter* or"
                    " Delimiter after label\n");
            break;
        case l_str_err:
            printf("expected closing \" in string constant\n");
            break;
        default:
            printf("there is no error, hmmmm....\n");
    }            
}

