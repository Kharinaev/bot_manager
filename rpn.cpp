#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rpn.hpp"

/*class RpnItem;
class VarTable;

class RpnElem{  
public:
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const = 0;
    virtual ~RpnElem() {}  
    virtual void Print(FILE* fd) = 0;
protected:
    static void Push(RpnItem *&stack, RpnElem *elem);
    static RpnElem* Pop(RpnItem *&stack);
};*/

////////EXCEPTIONS////////

void RpnExDivNull::Print(FILE* fd) const
{
    fprintf(fd, "RPN ERROR: divide by null\n");
}
    
void RpnExDivNull::Free()
{
    delete elem;
}

void RpnExNotVar::Print(FILE* fd) const{
    fprintf(fd, "RPN STACK ERROR: expected variable, got: ");
    elem->Print(fd);
    fprintf(fd, "\n");
}

void RpnExNotVar::Free(){
    delete elem;
}

void RpnExNotNum::Print(FILE* fd) const
{
    fprintf(fd, "RPN STACK ERROR: expected num, got: ");
    elem->Print(fd);
    fprintf(fd, "\n");
}

void RpnExNotNum::Free()
{
    delete elem;
}

void RpnExNoValue::Print(FILE* fd) const
{
    fprintf(fd, "RPN ERROR: variable ");
    elem->Print(fd);
    fprintf(fd, " not declared\n");
}

void RpnExNoValue::Free()
{
    delete elem;
}

void RpnExNotLabel::Print(FILE* fd) const
{
    fprintf(fd, "RPN ERROR: expected label, got: ");
    elem->Print(fd);
    fprintf(fd, "\n");
}

void RpnExNotLabel::Free()
{
    delete elem;
}

void RpnExNotInt::Print(FILE* fd) const
{
    fprintf(fd, "RPN STACK ERROR: expected int, got: ");
    elem->Print(fd);
    fprintf(fd, "\n");
}

void RpnExNotInt::Free()
{
    delete elem;
}

void RpnExPrint::Print(FILE* fd) const
{
    fprintf(fd, "RPN STACK ERROR: expected exp or string in print statement, got: ");
    elem->Print(fd);
    fprintf(fd,"\n");
}

void RpnExPrint::Free()
{
    delete elem;
}

///////VAR_TABLE////////

VarTable::~VarTable()
{
    DelTable(beg);
}

void VarTable::Print(FILE* fd) const
{
    varList *tmp = beg;
    fprintf(fd,"\nVAR TABLE\n");
    while (tmp != 0){
        fprintf(fd,"%s = %0.2lf\n",tmp->name, tmp->val);
        tmp = tmp->next;
    }
    fprintf(fd,"\n");
}

int VarTable::GetVal(char *str, double &val)
{
    varList *tmp=beg;
    while (tmp != 0){
        if (strcmp(tmp->name, str) == 0){
            val = tmp->val;
            return 0;
        }
        tmp = tmp->next;
    }
    return -1;
}

void VarTable::SetVal(char *str, double x)
{
    varList *tmp=beg;
    while (tmp != 0){
        if (strcmp(tmp->name, str) == 0){
            tmp->val = x;
            return;
        }
        tmp = tmp->next;
    }
    if (end != 0){
        end->next = new varList;
        end = end->next;
    } else 
        end = new varList;
    end->name = new char [strlen(str)+1];
    strcpy(end->name, str);
    end->val = x;
    end->next = 0;
    if (beg == 0)
        beg = end;
}

void VarTable::DelTable(varList *&ptr)
{
    if (ptr != 0){
        varList *tmp = ptr;
        delete [] ptr->name;
        delete tmp;
        DelTable(ptr->next);
    }
}

////////RPN_ITEM////////
    
////////RPN_ELEM////////


void RpnElem::Push(RpnItem *&stack, RpnElem *elem)
{
    if (stack == 0){
        stack = new RpnItem;
        stack->next = 0;
        stack->elem = elem;
    } else {
        RpnItem *tmp;
        tmp = new RpnItem;
        tmp->elem = elem;
        tmp->next = stack;
        stack = tmp;
    }
}

RpnElem* RpnElem::Pop(RpnItem *&stack)
{
    if (stack == 0)
        return 0;
    RpnElem *elem=stack->elem;
    RpnItem *tmp=stack;
    stack = stack->next;
    delete tmp;
    return elem;
} 
        
void RpnNoOp::Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const
{
    cur_cmd = cur_cmd->next;
}    

void RpnNoOp::Print(FILE *fd)
{
    fprintf(fd,"(no op)__");
}

////////CONST////////

void RpnConst::Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
            VarTable *var_table, Bot *bot) const
{
    Push(stack, Clone());
    cur_cmd = cur_cmd->next;
}    

int strToInt(const char *str, int &num)
{
    int i;
    num=0;
    for (i=0; str[i]!=0; i++){
        if (str[i]<'0' || str[i]>'9')
            return -1;
        num = 10*num+(str[i]-'0');
    }
    return 0;
}


RpnInt::RpnInt(const char *str)
{
    strToInt(str, val);
}

RpnElem* RpnInt::Clone() const
{
    return new RpnInt(val);
}

void RpnInt::Print(FILE *fd)
{
    fprintf(fd, "(int)%d__",val);
}

int RpnInt::Get() const 
{
    return val;
}

RpnElem* RpnDouble::Clone() const
{
    return new RpnDouble(val);
}

void RpnDouble::Print(FILE *fd)
{
    fprintf(fd,"(double)%0.2lf__",val);
}

double RpnDouble::Get() const 
{
    return val;
}

RpnElem* RpnLabel::Clone() const
{
    return new RpnLabel(val);
}

void RpnLabel::Print(FILE *fd)
{
    fprintf(fd,"(label){");
    val->elem->Print(fd);
    fprintf(fd,"}__");
}

RpnItem* RpnLabel::Get() const 
{
    return val;
}

void RpnLabel::Set(RpnItem *label)
{
    val = label;
}

RpnString::RpnString(char *s)
{
    str = new char [strlen(s)+1];
    strcpy(str, s);
}

RpnString::~RpnString()
{
    delete [] str;
}

void RpnString::Print(FILE *fd)
{
    fprintf(fd, "(str)%s__",str);
}

RpnElem* RpnString::Clone() const
{
    return new RpnString(str);
}

char* RpnString::Get() const
{
    return str;
}

RpnVar::RpnVar(char *s)
{
    str = new char [strlen(s)+1];
    strcpy(str, s);
}

RpnVar::~RpnVar()
{
    delete [] str;
}

void RpnVar::Print(FILE *fd)
{
    fprintf(fd, "(var)%s__",str);
}

RpnElem* RpnVar::Clone() const
{
    return new RpnVar(str);
}

char* RpnVar::GetName() const
{
    return str;
}

////////SEMICOLON////////

    void RpnSemicolon::Print(FILE *fd){
        fprintf(fd, "(;)__");
    }
    void RpnSemicolon::Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                    VarTable *var_table, Bot *bot) const{
        RpnElem *op = Pop(stack);
        delete op;
        cur_cmd = cur_cmd->next;
    }

////////FUNCTIONS////////

    void RpnFunc::Evaluate(RpnItem *&stack, 
                RpnItem *&cur_cmd, VarTable *var_table, Bot *bot) const{
        RpnElem *res = EvalFunc(stack, var_table, bot);
        if (res)
            Push(stack, res);
        cur_cmd = cur_cmd->next;
    }

RpnFunc::~RpnFunc()
{}
 
    void RpnFuncPlus::Print(FILE *fd){
        fprintf(fd, "(+)__");
    }

    RpnElem* RpnFuncPlus::EvalFunc(RpnItem *&stack, 
                    VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1); 
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        double res = i3->Get() + i4->Get();
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnDouble(res);
    }

    void RpnFuncMinus::Print(FILE *fd){
        fprintf(fd, "(-)__");
    }
    RpnElem* RpnFuncMinus::EvalFunc(RpnItem *&stack,VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        double res = i4->Get() - i3->Get();
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnDouble(res);
    }

    void RpnFuncMult::Print(FILE *fd){
        fprintf(fd,"(*)__");
    }
    RpnElem* RpnFuncMult::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        double res = i4->Get() * i3->Get();
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnDouble(res);

    }

    void RpnFuncDiv::Print(FILE *fd){
        fprintf(fd,"(/)__");
    }
    RpnElem* RpnFuncDiv::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        if (i3->Get() == 0)
            throw RpnExDivNull(op2);
        double res = i4->Get() / i3->Get();
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnDouble(res);
    }

    void RpnFuncMod::Print(FILE *fd){
        fprintf(fd, "(%%)__");
    }
    RpnElem* RpnFuncMod::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        if (i3->Get() == 0)
            throw RpnExDivNull(op2);
        double res = (int)i4->Get() % (int)i3->Get();
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnDouble(res);
    }

    void RpnFuncOr::Print(FILE *fd){
        fprintf(fd, "(|)__");
    }
RpnElem* RpnFuncOr::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        int res;
        if (i4->Get())
            res = 1;
        else {
            RpnDouble *i3;
            RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
            if (!i1){
                RpnDouble *i3 = dynamic_cast<RpnDouble*>(op1);
                if (!i3)
                    throw RpnExNotNum(op1);
            } else
                i3 = new RpnDouble(i1->Get());
            res = i3->Get() == 1;
            if (i1)
                delete i3;
        }
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnInt(res);
    }

    void RpnFuncAnd::Print(FILE *fd){
        fprintf(fd, "(&)__");
    }
    RpnElem* RpnFuncAnd::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        int res;
        if (!i4->Get())
            res = 0;
        else {
            RpnDouble *i3;
            RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
            if (!i1){
                RpnDouble *i3 = dynamic_cast<RpnDouble*>(op1);
                if (!i3)
                    throw RpnExNotNum(op1);
            } else
                i3 = new RpnDouble(i1->Get());
            res = i3->Get() == 1;
            if (i1)
                delete i3;
        }
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnInt(res);
    }

    void RpnFuncLess::Print(FILE *fd){
        fprintf(fd,"(<)__");
    }
    RpnElem* RpnFuncLess::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        int res = (i4->Get()) < (i3->Get());
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnInt(res);
    }

    void RpnFuncGreater::Print(FILE *fd){
        fprintf(fd,"(>)__");
    }
    RpnElem* RpnFuncGreater::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        int res = (i4->Get()) > (i3->Get());
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnInt(res);
    }

    void RpnFuncEqual::Print(FILE *fd){
        fprintf(fd, "(=)__");
    }
    RpnElem* RpnFuncEqual::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else 
            i3 = new RpnDouble(i1->Get());
        RpnElem *op2 = Pop(stack);
        RpnDouble *i4;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i4 = dynamic_cast<RpnDouble*>(op2);
            if (!i4)
                throw RpnExNotNum(op2);
        } else
            i4 = new RpnDouble(i2->Get());
        int res = i4->Get() == i3->Get();
        if (i1)
            delete i3;
        if (i2)
            delete i4;
        delete op1;
        delete op2;
        return new RpnInt(res);
    }

    void RpnFuncNot::Print(FILE *fd){
        fprintf(fd,"(!)__");
    }
    RpnElem* RpnFuncNot::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i2;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i2 = dynamic_cast<RpnDouble*>(op1);
            if (!i2)
                throw RpnExNotNum(op1);
        } else
            i2 = new RpnDouble(i1->Get());
        int res = !i2->Get();
        if (i1)
            delete i2;
        delete op1;
        return new RpnInt(res);
    }

    void RpnFuncUnMin::Print(FILE *fd){
        fprintf(fd,"(un min)__");
    }
    RpnElem* RpnFuncUnMin::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
    RpnElem *op1 = Pop(stack);
        RpnDouble *i2;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i2 = dynamic_cast<RpnDouble*>(op1);
            if (!i2)
                throw RpnExNotNum(op1);
        } else
            i2 = new RpnDouble(i1->Get());
        int res = -(i2->Get());
        if (i1)
            delete i2;
        delete op1;
        return new RpnInt(res);
    }

    void RpnFuncAsgn::Print(FILE *fd){
        fprintf(fd,"(asgn)__");
    }
    RpnElem* RpnFuncAsgn::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op2 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
        if (!i2){
            i3 = dynamic_cast<RpnDouble*>(op2);
            if (!i3)
                throw RpnExNotNum(op2);
        } else
            i3 = new RpnDouble(i2->Get());
        RpnElem *op1 = Pop(stack);
        RpnVar *i1 = dynamic_cast<RpnVar*>(op1);
        if (!i1)
            throw RpnExNotVar(op1);
        var_table->SetVal(i1->GetName(), i3->Get());
        if (i2)
            delete i3;
        delete op1;
        delete op2;
        return new RpnNoOp;
    }
        
    void RpnFuncVarVal::Print(FILE *fd){
        fprintf(fd, "(var val)__");
    }
    RpnElem* RpnFuncVarVal::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnVar *i1 = dynamic_cast<RpnVar*>(op1); 
        if (!i1)
            throw RpnExNotVar(op1);
        double val; 
        int res = var_table->GetVal(i1->GetName(), val);
        if (res == -1){
            var_table->SetVal(i1->GetName(), 0); 
            val = 0;
        }
            //throw RpnExNoValue(op1);    
        delete op1;
        //printf("check %0.2lf\n",val);
        return new RpnDouble(val);
    }

RpnFuncGameOpOne::RpnFuncGameOpOne(char *str)
{
    func_name = new char [strlen(str) + 1];
    strcpy(func_name, str);
}

RpnFuncGameOpOne::~RpnFuncGameOpOne()
{
    delete [] func_name;
}
void RpnFuncGameOpOne::Print(FILE *fd)
{
    fprintf(fd,"(game op one)%s__",func_name);
}

RpnElem* RpnFuncGameOpOne::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    printf("%s\n", func_name);
    RpnElem *op = Pop(stack);
    delete op;
    return new RpnNoOp;
}

void RpnFuncProd::Print(FILE* fd)
{
    fprintf(fd,"(prod)__");
}

RpnElem* RpnFuncProd::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op = Pop(stack);
    RpnDouble *i1;
    RpnInt *i=dynamic_cast<RpnInt*>(op);
    if (!i){
        i1 = dynamic_cast<RpnDouble*>(op);
        if (!i1)
            throw RpnExNotNum(op);
    }else
        i1 = new RpnDouble(i->Get());
    bot->WriteToServ("prod",(int)i1->Get());
    if (i)
        delete i1;
    delete op;
    return new RpnNoOp;
}
 
void RpnFuncBuild::Print(FILE* fd)
{
    fprintf(fd,"(build)__");
}

RpnElem* RpnFuncBuild::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op = Pop(stack);
    RpnDouble *i1;
    RpnInt *i=dynamic_cast<RpnInt*>(op);
    if (!i){
        i1 = dynamic_cast<RpnDouble*>(op);
        if (!i1)
            throw RpnExNotNum(op);
    }else
        i1 = new RpnDouble(i->Get());
    bot->WriteToServ("build");
    if (i)
        delete i1;
    delete op;
    return new RpnNoOp;

}
 
void RpnFuncABuild::Print(FILE* fd)
{
    fprintf(fd,"(abuild)__");
}

RpnElem* RpnFuncABuild::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op = Pop(stack);
    RpnDouble *i1;
    RpnInt *i=dynamic_cast<RpnInt*>(op);
    if (!i){
        i1 = dynamic_cast<RpnDouble*>(op);
        if (!i1)
            throw RpnExNotNum(op);
    }else
        i1 = new RpnDouble(i->Get());
    bot->WriteToServ("abuild");
    if (i)
        delete i1;
    delete op;
    return new RpnNoOp;

}
    
    
void RpnFuncGameOpNone::Print(FILE *fd)
{
    fprintf(fd,"(endturn)__");
}

RpnElem* RpnFuncGameOpNone::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    if (bot->WriteToServ("turn") == -1){
        fprintf(stderr, "ERROR write");
    }
    return new RpnNoOp;
}

RpnFuncGameOpTwo::RpnFuncGameOpTwo(char *str)
{
   func_name = new char [strlen(str) + 1];
   strcpy(func_name, str);
}

RpnFuncGameOpTwo::~RpnFuncGameOpTwo()
{
    delete [] func_name;   
}

void RpnFuncGameOpTwo::Print(FILE *fd)
{
    fprintf(fd, "(game op two)%s__",func_name);
}
RpnElem* RpnFuncGameOpTwo::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    printf("%s\n", func_name);
    RpnElem *op = Pop(stack);
    delete op;
    op = Pop(stack);
    delete op;
    return new RpnNoOp;
}
    
void RpnFuncBuy::Print(FILE *fd)
{
    fprintf(fd,"(buy)__");
}  
 
RpnElem* RpnFuncBuy::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op1 = Pop(stack);
    RpnDouble *i3;
    RpnInt *i1=dynamic_cast<RpnInt*>(op1);
    if (!i1){
        i3 = dynamic_cast<RpnDouble*>(op1);
        if (!i3)
            throw RpnExNotNum(op1);
    }else
        i3 = new RpnDouble(i1->Get());
    RpnElem *op2 = Pop(stack);
    RpnDouble *i4;
    RpnInt *i2=dynamic_cast<RpnInt*>(op2);
    if (!i2){
        i4 = dynamic_cast<RpnDouble*>(op2);
        if (!i4)
            throw RpnExNotNum(op2);
    }else
        i4 = new RpnDouble(i2->Get());
    bot->WriteToServ("buy",(int)i4->Get(),(int)i3->Get());
    if (i1)
        delete i3;
    if (i2)
        delete i4;
    delete op1;
    delete op2;
    return new RpnNoOp; 
}

void RpnFuncSell::Print(FILE *fd)
{
    fprintf(fd,"(sell)__");
}  

RpnElem* RpnFuncSell::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op1 = Pop(stack);
    RpnDouble *i3;
    RpnInt *i1=dynamic_cast<RpnInt*>(op1);
    if (!i1){
        i3 = dynamic_cast<RpnDouble*>(op1);
        if (!i3)
            throw RpnExNotNum(op1);
    }else
        i3 = new RpnDouble(i1->Get());
    RpnElem *op2 = Pop(stack);
    RpnDouble *i4;
    RpnInt *i2=dynamic_cast<RpnInt*>(op2);
    if (!i2){
        i4 = dynamic_cast<RpnDouble*>(op2);
        if (!i4)
            throw RpnExNotNum(op2);
    }else
        i4 = new RpnDouble(i2->Get());
    bot->WriteToServ("sell",(int)i4->Get(),(int)i3->Get());
    if (i1)
        delete i3;
    if (i2)
        delete i4;
    delete op1;
    delete op2;
    return new RpnNoOp; 
}

RpnElem* RpnPrint::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op = Pop(stack);
    RpnInt *i = dynamic_cast<RpnInt*>(op); 
    if (!i){
        RpnDouble *i1 = dynamic_cast<RpnDouble*>(op);
        if (!i1){
            RpnString *s = dynamic_cast<RpnString*>(op);
            if (s)
                printf("%s\n",s->Get());
            else
                throw RpnExPrint(op); 
        } else
            printf("%0.2lf\n",i1->Get());
    } else
    printf("%d\n",i->Get());
    delete op;
    return new RpnNoOp;
}    

void RpnPrint::Print(FILE *fd)
{
    fprintf(fd,"(print)__");
}
    
char *intToStr(int num)
{
    int i=0, len, n=num;
    char *str;
    if (n == 0)
        len = 1;
    else{
        while (n != 0){
            i++;
            n /= 10;
        }
        len = i;
    }
    str = new char [len+1];
    str[len] = 0;
    n = num;
    for (i=len-1; i>=0; i--){
        str[i] = (n % 10) + '0';
        n /= 10;
    } 
    return str;
}
    
    
    RpnElem* RpnVarIndex::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const{
        RpnElem *op1 = Pop(stack);
        RpnDouble *i3;
        RpnInt *i1 = dynamic_cast<RpnInt*>(op1);
        if (!i1){
            i3 = dynamic_cast<RpnDouble*>(op1);
            if (!i3)
                throw RpnExNotNum(op1);
        } else
            i3 = new RpnDouble(i1->Get());
        int exp = (int)i3->Get();
        char *exp_str = intToStr(exp);
        RpnElem *op2 = Pop(stack);
        RpnVar *i2 = dynamic_cast<RpnVar*>(op2);
        if (!i2)
            throw RpnExNotVar(op2);
        char *var_name = new char [strlen(i2->GetName()) + 2 
                + strlen(exp_str) + 1];
        sprintf(var_name,"%s[%s]",i2->GetName(),exp_str);
        delete [] exp_str;
        if (i1)
            delete i3;
        delete op1;
        delete op2;
        return new RpnVar(var_name);
    }    
    void RpnVarIndex::Print(FILE *fd){
        fprintf(fd,"(var index)__");
    }

RpnElem* RpnGameFuncNoArg::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    int res;
    if (strcmp(name, "?rawAmount") == 0){
        res = bot->GetMarket()->raw;
    } else
    if (strcmp(name, "?rawPrice") == 0){
        res = bot->GetMarket()->min;
    } else
    if (strcmp(name, "?prodPrice") == 0){
        res = bot->GetMarket()->max;
    } else
    if (strcmp(name, "?prodAmount") == 0){
        res = bot->GetMarket()->prod;
    } else
    if (strcmp(name, "?myId") == 0){
        res = bot->GetMyId();
    } else
    if (strcmp(name, "?players") == 0){
        res = bot->GetPlayersNum();
    } else
    if (strcmp(name, "?turn") == 0){
        res = bot->GetTurn();
    }
    return new RpnInt(res);
};
     
RpnGameFuncNoArg::RpnGameFuncNoArg(char *s)
{
    name = new char [strlen(s)+1];
    strcpy(name, s);
}

RpnGameFuncOneArg::RpnGameFuncOneArg(char *s)
{
    name = new char [strlen(s)+1];
    strcpy(name, s);
}
    
RpnGameFuncNoArg::~RpnGameFuncNoArg()
{
    delete [] name;
}

RpnGameFuncOneArg::~RpnGameFuncOneArg()
{
    delete [] name;
}

void RpnGameFuncNoArg::Print(FILE *fd)
{
    fprintf(fd,"(func no arg)__");
}

RpnElem* RpnGameFuncOneArg::EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const
{
    RpnElem *op = Pop(stack);
    RpnDouble *i1;
    RpnInt *i = dynamic_cast<RpnInt*>(op);
    if (!i){
        i1 = dynamic_cast<RpnDouble*>(op);
        if (!i1)
            throw RpnExNotNum(op);
    }else
        i1 = new RpnDouble(i->Get());
    int res;
    if (strcmp(name, "?money") == 0){
        if (bot->GetInfo((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetInfo((int)i1->Get())->money;    
    } else 
    if (strcmp(name, "?raw") == 0){
        if (bot->GetInfo((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetInfo((int)i1->Get())->raw;
    } else 
    if (strcmp(name, "?prod") == 0){
        if (bot->GetInfo((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetInfo((int)i1->Get())->prod;
    } else 
    if (strcmp(name, "?plants") == 0){
        if (bot->GetInfo((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetInfo((int)i1->Get())->plants;
    } else 
    if (strcmp(name, "?aplants") == 0){
        if (bot->GetInfo((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetInfo((int)i1->Get())->auto_plants;
    } else 
    if (strcmp(name, "?aucRawPrice") == 0){
        if (bot->GetAuc((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetAuc((int)i1->Get())->price;
    } else 
    if (strcmp(name, "?aucProdPrice") == 0){
        if (bot->GetAuc((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetAuc((int)i1->Get())->price;
    } else 
    if (strcmp(name, "?aucRawBought") == 0){
        if (bot->GetAuc((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetAuc((int)i1->Get())->amount;
    } else 
    if (strcmp(name, "?aucProdSold") == 0){
        if (bot->GetAuc((int)i1->Get()) == 0)
            res = 0;
        else
            res = bot->GetAuc((int)i1->Get())->amount;
    } else
    if (strcmp(name, "?random") == 0){
        res = rand() % (int)i1->Get() ;
    }     
    if (i)
        delete i1;
    delete op;
    return new RpnInt(res);
}

void RpnGameFuncOneArg::Print(FILE *fd)
{
    fprintf(fd,"(func one arg)__");
}

/////////RPN_GO////////

void RpnGo::Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const
{
    RpnElem *op1 = Pop(stack);
    RpnLabel *i1 = dynamic_cast<RpnLabel*>(op1);
    if (!i1)
        throw RpnExNotLabel(op1);
    RpnItem *addr = i1->Get();
    cur_cmd = addr;
    delete op1;
}
 
void RpnGo::Print(FILE *fd)
{
    fprintf(fd,"(go)__");
}

void RpnGoFalse::Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const{
    RpnElem *op1 = Pop(stack);
    RpnLabel *i1 = dynamic_cast<RpnLabel*>(op1);
    if (!i1)
        throw RpnExNotLabel(op1);
    RpnElem *op2 = Pop(stack);
    RpnInt *i2 = dynamic_cast<RpnInt*>(op2);
    if (!i2)
        throw RpnExNotInt(op2);
    if (i2->Get())
        cur_cmd = cur_cmd->next;
    else{
        RpnItem *addr = i1->Get();
        cur_cmd = addr;
    }
    delete op1;
    delete op2;
}

void RpnGoFalse::Print(FILE *fd){
    fprintf(fd,"(false go)__");
}
