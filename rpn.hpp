class Rpn;

#ifndef BOT_MACRO_SENTRY
#define BOT_MACRO_SENTRY
enum what_is_taken{
    not_taken, auction_enum, info_enum, market_enum, ok_enum,
    players_enum, endturn_enum, start_enum, str_taken, inc_enum, 
    dec_enum, endgame_enum
};

enum{
    max_name_len = 17
};

struct list_str{
    char *str;
    list_str *next;
};

struct auction_state{
    char *name;
    int amount;
    int price;
    auction_state(){
        int i;
        name = new char [max_name_len];
        for (i=0; i<max_name_len; i++)
            name[i] = 0;
    }
    ~auction_state(){
        delete [] name;
    }
};

struct info_state{
    char *name;
    int raw;
    int prod;
    int money;
    int plants;
    int auto_plants;
    info_state(){
        int i;
        name = new char [max_name_len];
        for (i=0; i<max_name_len; i++)
            name[i] = 0;
    }
    ~info_state(){
        delete [] name;
    }
};

struct market_state{
    int raw;
    int min;
    int prod;
    int max;
};

struct player_t{
    int num;
    char *name;
    bool check;
    player_t *next;
};

class PlayTable{
    player_t *beg, *end;
    int num;
    void Add(char *str);
    int CheckAndDel();
    void Renum();
public:
    PlayTable();
    void Fill(info_state *&info, int players_num);
    char *GetName(int num);
    int GetNum(char *str);
    void Print() const;
    ~PlayTable();
};

class Bot{
    int fd, buf_size, buf_ptr, players_num;
    int info_ptr, auction_ptr, turn, mem_buf_ptr;
    char *buf;
    market_state market;
    info_state *info, my_info;
    auction_state *auction;
    PlayTable pl_table;
    bool mass_maked;
    Rpn *rpn;
    ////
    void FillPlayers();
public:
    Bot();
    int SetName(const char *str);
    int ConnectServer(char *str_ip, char *str_port);
    int WriteToServ(const char *str, int n1=0, int n2=0);
    int ReadFromServ();
    int CheckBuf();
    int TakePlayersNum();
    void PrintState();
    int StrParsing(char *&str);
    int Fill(list_str *list, char *str);
    //int MainCycle();
    int Join(const char *where);
    int Create(int pl_num);

    market_state* GetMarket();
    auction_state* GetAuc(int num);
    info_state* GetInfo(int num); 
    int& InfoPtr();
    int& AucPtr();
    int& Players_num();
    int GetTurn();   
    int GetPlayersNum();
    int GetMyId();
    void PrintTable() const;
    void FillTable();
    void ClearAuc();
    void Turn();
    ~Bot();
};

#endif

class RpnItem;
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
};

class RpnExDivNull{
    RpnElem *elem;
public:
    RpnExDivNull(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;    
    void Free();
};

class RpnExNotVar{
    RpnElem *elem;
public:
    RpnExNotVar(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;
    void Free();
};

class RpnExNotNum{
    RpnElem *elem;
public:
    RpnExNotNum(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;
    void Free();
};

class RpnExNoValue{
    RpnElem *elem;
public:
    RpnExNoValue(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;
    void Free();
};

class RpnExNotLabel{
    RpnElem *elem;
public:
    RpnExNotLabel(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;
    void Free();
};

class RpnExNotInt{
    RpnElem *elem;
public:
    RpnExNotInt(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;
    void Free();
};

class RpnExPrint{
    RpnElem *elem;
public:
    RpnExPrint(RpnElem *op) : elem(op){}
    void Print(FILE* fd) const;
    void Free();
};

struct varList{
    double val;
    char *name;
    varList* next;
};

class VarTable{
    varList *beg, *end;
    void DelTable(varList *&ptr);
public: 
    VarTable() : beg(0), end(0){}
    void SetVal(char *str, double x);
    int GetVal(char *str, double &val);
    void Print(FILE* fd) const;
    ~VarTable();
}; 

struct RpnItem{
    RpnElem *elem;
    RpnItem *next;
};

class RpnNoOp : public RpnElem{
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
};

class RpnConst : public RpnElem{
public:
    virtual RpnElem* Clone() const = 0;
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const;
};

class RpnInt : public RpnConst{
    int val;
public:
    RpnInt(int a) : val(a) {}
    RpnInt(const char *str);    
    virtual ~RpnInt() {}
    virtual RpnElem* Clone() const;
    virtual void Print(FILE *fd);
    int Get() const;
};

class RpnDouble : public RpnConst{
    double val;
public:
    RpnDouble(double a) : val(a) {}
    virtual ~RpnDouble() {}
    virtual RpnElem* Clone() const;
    virtual void Print(FILE *fd);
    double Get() const;
};

class RpnLabel : public RpnConst{
    RpnItem *val;
public:
    RpnLabel(){}
    RpnLabel(RpnItem *a) : val(a) {}
    virtual ~RpnLabel() {}
    virtual RpnElem* Clone() const;
    virtual void Print(FILE *fd);
    RpnItem* Get() const;
    void Set(RpnItem *label);
};

class RpnString : public RpnConst{
    char *str;
public:
    RpnString(char *s);
    ~RpnString();
    virtual void Print(FILE *fd);
    virtual RpnElem* Clone() const;
    char* Get() const;
};

class RpnVar : public RpnConst{
        char *str;
public:
    RpnVar(char *s);
    ~RpnVar();
    virtual void Print(FILE *fd);
    virtual RpnElem* Clone() const;
    char* GetName() const;
};

class RpnSemicolon : public RpnElem{
public:
    RpnSemicolon(){}
    virtual ~RpnSemicolon(){}
    virtual void Print(FILE *fd);
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                    VarTable *var_table, Bot *bot) const;
};

////////FUNCTIONS////////

class RpnFunc : public RpnElem{
public:
    //return noop if func doesn't have result (asgn and game ops)
    virtual RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const = 0;
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
        VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd)=0;
    virtual ~RpnFunc();
};

class RpnFuncPlus : public RpnFunc{
public:
    RpnFuncPlus(){}
    virtual ~RpnFuncPlus(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncMinus : public RpnFunc{
public:
    RpnFuncMinus(){}
    virtual ~RpnFuncMinus(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack,VarTable *var_table, Bot *bot) const;
};

class RpnFuncMult : public RpnFunc{
public:
    RpnFuncMult(){}
    virtual ~RpnFuncMult(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncDiv : public RpnFunc{
public:
    RpnFuncDiv(){}
    virtual ~RpnFuncDiv(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncMod : public RpnFunc{
public:
    RpnFuncMod(){}
    virtual ~RpnFuncMod(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncOr : public RpnFunc{
public:
    RpnFuncOr(){}
    virtual ~RpnFuncOr(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncAnd : public RpnFunc{
public:
    RpnFuncAnd(){}
    virtual ~RpnFuncAnd(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncLess : public RpnFunc{
public:
    RpnFuncLess(){}
    virtual ~RpnFuncLess(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncGreater : public RpnFunc{
public:
    RpnFuncGreater(){}
    virtual ~RpnFuncGreater(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncEqual : public RpnFunc{
public:
    RpnFuncEqual(){}
    virtual ~RpnFuncEqual(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncNot : public RpnFunc{
public:
    RpnFuncNot(){}
    virtual ~RpnFuncNot(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncUnMin : public RpnFunc{
public:
    RpnFuncUnMin(){}
    virtual ~RpnFuncUnMin(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncAsgn : public RpnFunc{
public:
    RpnFuncAsgn(){}
    virtual ~RpnFuncAsgn(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncVarVal : public RpnFunc{
public:
    RpnFuncVarVal(){}
    virtual ~RpnFuncVarVal(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncGameOpOne : public RpnFunc{
    char *func_name;
public:
    RpnFuncGameOpOne(char *str);
    virtual ~RpnFuncGameOpOne();
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncProd : public RpnFunc{
public:
    virtual ~RpnFuncProd(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncBuild : public RpnFunc{
public:
    virtual ~RpnFuncBuild(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncABuild : public RpnFunc{
public:
    virtual ~RpnFuncABuild(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};


class RpnFuncGameOpNone : public RpnFunc{
public:
    RpnFuncGameOpNone(){}
    virtual ~RpnFuncGameOpNone(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncGameOpTwo : public RpnFunc{
    char *func_name;
public:
    RpnFuncGameOpTwo(char *str);
    virtual ~RpnFuncGameOpTwo();
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncBuy : public RpnFunc{
public:
    virtual ~RpnFuncBuy(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};

class RpnFuncSell : public RpnFunc{
public:
    virtual ~RpnFuncSell(){}
    virtual void Print(FILE *fd);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
};


class RpnPrint : public RpnFunc{ 
public:
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
};

class RpnVarIndex : public RpnFunc{
public:
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
};

class RpnGameFuncNoArg : public RpnFunc{ 
    char *name;
public:
    RpnGameFuncNoArg(char *s);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
    ~RpnGameFuncNoArg();
};

class RpnGameFuncOneArg : public RpnFunc{ 
    char *name;
public:
    RpnGameFuncOneArg(char *s);
    RpnElem* EvalFunc(RpnItem *&stack, VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
    ~RpnGameFuncOneArg();
};

/////////RPN_GO////////

class RpnGo : public RpnElem{
public:
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
};

class RpnGoFalse : public RpnElem{
public:
    virtual void Evaluate(RpnItem *&stack, RpnItem *&cur_cmd, 
                            VarTable *var_table, Bot *bot) const;
    virtual void Print(FILE *fd);
};



