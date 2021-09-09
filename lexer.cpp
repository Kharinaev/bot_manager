#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "lexeme.hpp"
#include "lexer.hpp"

#ifdef DEBUG
    #define DPRINT(x) do {printf x;} while(0)
#else
    #define DPRINT(x) do {} while(0)
#endif

AutoCl::AutoCl()
{
    int i;
    state = s_beg;
    line_num = 1;
    buf_size = 10;
    buf_ptr = 0;
    res = 0;
    buf = new char [buf_size];
    for (i=0; i<buf_size; i++)
        buf[i]=0;
    lex_ready = false;
    type  = unknown;
} 
 
AutoCl::~AutoCl()
{
    delete [] buf;
}
 
#ifndef BUFFERINCREASE_LEXER_SENTRY
#define BUFFERINCREASE_LEXER_SENTRY
void buf_inc(char *&buf, int &buf_size)
{
    char *tmp; 
    int i;
    tmp = new char [2*buf_size];
    for (i=0; i < 2*buf_size; i++)
        tmp[i] = 0;
    for (i=0; i<buf_size; i++)
        tmp[i] = buf[i];
    delete [] buf;
    buf = tmp;
    buf_size *= 2;
}
  
bool is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == -1; 
}

bool is_delim(int c)
{
    return  c=='+' || c=='-' || c=='*' || c=='/' || c=='%' || c=='<' ||
            c=='>' || c=='=' || c=='&' || c=='|' || c=='!' || c=='(' ||
            c==')' || c=='[' || c==']' || c==';' || c==',' || c=='{' ||
            c=='}' ;
}

bool is_letter(int c)
{
    return (c>='a' && c<='z') || (c>='A' && c<='Z');
}

#endif

void AutoCl::ChangeFromBegin(int c)
{	
    if (is_space(c) || is_delim(c))
        return;
    else 
    if (c >= '0' && c <= '9')
        state = s_num;
    else
    if (c == '\"')
        state = s_str;
    else
    if (c == '$' || c == '?')
        state = s_iden;
    else
    if (c == '@')
        state = s_label;
    else 
    if (c == ':')
        state = s_asgn;
    else
    if (is_letter(c))
        state = s_key;
    else{
        state = s_err;
        lex.answer = l_beg_err;
    }
}

void AutoCl::ChangeFromNum(int c)
{
    if (c>='0' && c<='9')
        state = s_num;
    else 
        if (is_space(c) || is_delim(c) || c == ':')
            state = s_beg;
        else{
            state = s_err;
            lex.answer = l_num_err;
        }
}

void AutoCl::ChangeFromStr(int c)
{
    if (c == '\n'){
        state = s_err;
        lex.answer = l_str_err;
    }else
    if (c == '\"')
        state = s_beg;
    else 
        state = s_str;
}

void AutoCl::ChangeFromIden(int c)
{
    if ((c>='0' && c<='9') || is_letter(c))
        state = s_iden;
    else 
        if (is_space(c) || is_delim(c) || c == ':')
            state = s_beg;
        else {
            state = s_err;
            lex.answer = l_iden_err;
        }
}

void AutoCl::ChangeFromAssign(int c)
{
    if (c == '=')
        state = s_beg;
    else {
        state = s_err;
        lex.answer = l_asgn_err;
    }
}

void AutoCl::ChangeFromKey(int c)
{
    if (is_letter(c))
        state = s_key;
    else
        if (is_space(c) || is_delim(c))
            state = s_beg;
        else{
            state = s_err;
            lex.answer = l_key_err;
        }
}

void AutoCl::ChangeFromLabel(int c)
{
    if (is_letter(c) || (c<='9' && c>='0'))
        state = s_label;
    else
        if (c == ':')
            state = s_end_label;
        else {
            state = s_err;
            lex.answer = l_label_err;
        }
}    
                   
void AutoCl::ChangeFromEndLabel(int c)
{
    if (is_space(c) || is_delim(c))
        state = s_beg;
    else{
        state = s_err;
        lex.answer = l_endlab_err;
    }
}
 
void AutoCl::ChangeState(int c)
{
    switch(state){
        case s_beg:
            ChangeFromBegin(c);
            break;
        case s_num:
            ChangeFromNum(c);
            break;
        case s_str:
            ChangeFromStr(c); 
            break;
        case s_iden:
            ChangeFromIden(c);
            break;
        case s_asgn:
            ChangeFromAssign(c);
            break;
        case s_key:
            ChangeFromKey(c);
            break;
        case s_label:
            ChangeFromLabel(c);
            break;
        case s_end_label:
            ChangeFromEndLabel(c);
            break;
        case s_err:
            break;
    }
}

void AutoCl::PutInBuf(int c)
{
    buf[buf_ptr] = c;
    buf_ptr++;
    if (buf_ptr >= buf_size)
        buf_inc(buf, buf_size);
}
     
void AutoCl::DefineType(int c, lex_type &type)
{
    if (type != unknown && type != delimiter)
        return;
    if (c >= '0' && c <= '9')
        type = num_const;
    else
    if (c == '\"')
        type = str_const;
    else
    if (c == '$')
        type = variable;
    else
    if (c == '?')
        type = function;
    else
    if (c == '@')
        type = label;
    else
    if (c == ':')
        type = assign;
    else
    if (is_letter(c))
        type = key_word;
    else 
    if (is_delim(c))
        type = delimiter;
}

void AutoCl::MakeLexSymb(int c)
{
    lex.str = new char [2];
    lex.str[0] = c;
    lex.str[1] = 0;
    lex.line = line_num;    
    lex.answer = good;
    lex.type = delimiter;    
    lex.str_maked = true;
    lex_ready = true;
}

void AutoCl::MakeLexBuf()
{
    lex.str = new char [strlen(buf) + 1];
    strcpy(lex.str, buf);
    lex.line = line_num;    
    lex.answer = good;    
    lex.str_maked = true;
    lex_ready = true;
    lex.type = type;
    type = unknown;
}
    
void AutoCl::FeedChar(int c)
{
    int i;
    //printf("c is %c {%d} res %c state %d\n",c,c,res,state);
    if (c == -1 && res == 0 && buf_ptr == 0){
        lex.answer = end_of_file;
        lex_ready = true;
        return;
    }

    if (res != 0){
        ChangeState(res);
        if (state == s_err){
            lex.line = line_num;
            lex_ready = true;
            return;
        }
        if (state == s_beg)
            MakeLexSymb(res);     
        else 
            PutInBuf(res);
        res = 0;
    }        
    if (state == s_asgn && (c == '=' || res == '='))
        PutInBuf('=');
    if (state == s_str && (c == '\"' || res == '\"'))
        PutInBuf('c');
    ChangeState(c);
    DefineType(c,type);
    if (state == s_err){
        lex.line = line_num;
        lex_ready = true;
        return;
    }
    if (state == s_beg){
        if ((is_delim(c)||c==':') && buf[0] != ':')
            res = c;
        if (buf_ptr != 0){
            MakeLexBuf();
            for (i=0; i<buf_ptr; i++)
                buf[i] = 0;
            buf_ptr = 0;
        }
    } else 
        if (!is_space(c) || state == s_str)
            PutInBuf(c); 
    line_num += (c == '\n');
}

void AutoCl::CheckLexeme(lexeme &l)
{
    if (lex_ready){
        lex_ready = false;
        l = lex;
    } else
        l.answer = empty;
}
     
#if 0
int main(int argc, char **argv)
{
    int fd, c;
    lexeme lex;
    AutoCl avt;
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
    while (lex.answer != end_of_file){
        lex.Clear();
        c = getchar();
        avt.FeedChar(c);
        avt.CheckLexeme(lex);
        if (lex.IsError()){
            lex.PrintError();
            return 0;
        }
        lex.Print();
    }
    return 0;
}
#endif    