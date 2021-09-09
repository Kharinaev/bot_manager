#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "lexeme.hpp"
#include "lexer.hpp"
#include "report.hpp"
#include "rpn.hpp"
#include "syntax.hpp"

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

void Bot::Turn()
{
    turn++;
}
 
void Bot::FillTable()
{
    pl_table.Fill(info, players_num);
}

void Bot::PrintTable() const
{
    pl_table.Print();
}

int& Bot::Players_num()
{
    return players_num;
}

int& Bot::InfoPtr()
{
    return info_ptr;
}

int& Bot::AucPtr()
{
    return auction_ptr;
}


int Bot::GetMyId()
{
    return pl_table.GetNum(my_info.name);
}    

auction_state* Bot::GetAuc(int num)
{
    int i = 0;
    char *str = pl_table.GetName(num);
    for (i=0; i<auction_ptr; i++){
        if (strcmp(str, auction[i].name) == 0)
            return &auction[i]; 
    }
    return 0;
}  

info_state* Bot::GetInfo(int num)
{
    int i = 0;
    char *str = pl_table.GetName(num);
    //printf("-|-|- %s\n",str);
    for (i=0; i<info_ptr; i++){
        //printf("============ %d %s\n",i, info[i].name);
        if (strcmp(str, info[i].name) == 0)
            return &info[i]; 
    }
    return 0;
}  

PlayTable::PlayTable()
{
    beg = 0;
    end = 0;
    num = 0;
}

PlayTable::~PlayTable()
{
    while (beg != 0){
        player_t *tmp = beg;
        beg = beg->next;
        delete [] tmp->name;
        delete tmp;
    }
}

void PlayTable::Print() const
{
    player_t *tmp = beg;
    printf("\n\nplayers table\n");
    while (tmp != 0){
        printf("%s %d\n",tmp->name, tmp->num);
        tmp = tmp->next;
    }
    printf("\n");
}

int PlayTable::GetNum(char *str)
{
    player_t *tmp = beg;
    while (tmp != 0){
        if (strcmp(str, tmp->name) == 0)
            return tmp->num;
        tmp = tmp->next;
    }
    return -1;
}

char* PlayTable::GetName(int num)
{
    player_t *tmp=beg;
    while (tmp != 0){
        if (tmp->num == num)
            return tmp->name;
        tmp = tmp->next;
    }
    return 0;
}

void PlayTable::Add(char *str)
{
    //printf("PLAYTABLE ADD %s\n", str);
    player_t *tmp = beg;
    while (tmp != 0){
        if (strcmp(str, tmp->name) == 0){
            //printf("CHECK IS OK %s\n",tmp->name);
            tmp->check = true;
            return;
        }
        tmp = tmp->next;
    }
    if (end != 0){
        end->next = new player_t;
        end = end->next;
    } else 
        end = new player_t;
    end->name = new char [strlen(str)+1];
    strcpy(end->name, str);
    end->num = num++;
    end->check = true;
    end->next = 0;
    if (beg == 0)
        beg = end;
}

void PlayTable::Fill(info_state *&info, int players_num)
{
    int i;
    for (i=0; i<players_num; i++)
        Add(info[i].name);    
    while (CheckAndDel()!=0)
        ;
}

int PlayTable::CheckAndDel()
{
    player_t *tmp = beg;
    while (tmp != 0){
        if (!tmp->check)
            break;
        tmp = tmp->next;
    }
    if (tmp == 0)
        return 0;
    //printf("DELETING %s\n",tmp->name);
    if (tmp == beg){
        beg = beg->next;
        if (tmp == end)
            end = 0;
        delete [] tmp->name;
        delete tmp;
    } else  
        if (tmp == end){
            player_t *tmp2 = beg;
            while (tmp2->next != tmp)
                tmp2 = tmp2->next;
            end = tmp2;
            end->next = 0;
            delete [] tmp->name;
            delete tmp;
        } else{
            player_t *tmp2 = beg;
            while (tmp2->next != tmp)
                tmp2 = tmp2->next;
            tmp2->next = tmp->next;
            delete [] tmp->name;
            delete tmp;
        }
    Renum();
    return 1;
}

void PlayTable::Renum()
{
    num = 0;
    player_t *tmp = beg;
    while (tmp != 0){
        tmp->num = num++;
        tmp->check = false;
        tmp = tmp->next;
    }
}    
market_state* Bot::GetMarket()
{
    return &market;
}

int Bot::GetTurn()
{
    return turn;
}

int Bot::GetPlayersNum()
{
    return players_num;
}

Bot::Bot()
{
    int i;
    turn = 0;
    fd = -1;
    buf_size = 10;
    buf_ptr = 0;
    mem_buf_ptr = -1;
    players_num = 0;
    info_ptr = 0;
    auction_ptr = 0;
    mass_maked = false;
    buf = new char [buf_size];
    for (i=0; i<buf_size; i++)
        buf[i]=0;
}

Bot::~Bot()
{
    delete [] buf;
    if (fd != -1){
        shutdown(fd, 2);
        close(fd);
    }
    if (mass_maked){
        delete [] auction;
        delete [] info;
    }
}

void buf_incr(char *&buf, int &buf_size)
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

int str_to_int(const char *str, int &num)
{
    int i;
    num = 0;
    for (i=0; str[i]!=0; i++){
        if (str[i]<'0' || str[i]>'9')
            return -1;
        num = 10*num+(str[i]-'0');
    }
    return 0;
}

char *int_to_str(int num)
{
    int i=0, tmp = num, j;
    char *str;
    while (tmp != 0){
        i++;
        tmp /= 10;
    }
    tmp = num;
    str = new char [i+1];
    for (j=0; j<i; j++){
        str[i-j-1] = tmp % 10 + '0';
        tmp /= 10;
    }
    str[i] = 0;
    return str;
}
        

int Bot::TakePlayersNum()
{
    int ch;
    ch = WriteToServ("info");
    if (ch == -1)
        return -1;
    ch = 0;
    while (ch != players_enum){
        ch = ReadFromServ();
        if (ch == -1)
            return -1;
    }
    info = new info_state [players_num];
    auction = new auction_state [players_num * 2];
    mass_maked = true;
    return 0;
}     

void list_out(list_str *list)
{
    if (list != NULL){
        printf("%s\n",list->str);
        list_out(list->next);
    }
}
    
list_str *str_break(char *&str)
{
    list_str *list, *tmp;
    int i=0, j=1, k;
    list = NULL;
    do {
        tmp = new list_str;
        for(i=j+1; str[i]==0 || str[i]==' '; i++);
        for(j=i  ; str[j]!=0 && str[j]!=' '; j++);
        tmp->str = new char [j-i];
        for(k=i; k<j; k++){
            (tmp->str)[k-i] = str[k];
        }
        (tmp->str)[k-i] = 0;
        tmp->next = list;
        list = tmp;
    } while (str[i]!=0 && str[j]!=0);
    return list;
}

void delete_list(list_str **list)
{
    list_str *tmp;
    if (*list != NULL){
        tmp = *list;
        *list = (*list)->next;
        free(tmp->str);
        free(tmp);
        delete_list(list);
    }
}

void market_fill(market_state &market, list_str *list)
{
    list_str *tmp=list;
    str_to_int(tmp->str, market.max);
    tmp = tmp->next;
    str_to_int(tmp->str, market.prod);
    tmp = tmp->next;
    str_to_int(tmp->str, market.min);
    tmp = tmp->next;
    str_to_int(tmp->str, market.raw);
}

void auction_fill(auction_state &auction, list_str *list)
{
    list_str *tmp=list; 			
    str_to_int(tmp->str, auction.price);
    tmp = tmp->next;
    str_to_int(tmp->str, auction.amount);
    tmp = tmp->next;
    strcpy(auction.name, tmp->str);
}

void info_fill(info_state &info, list_str *list)
{
    list_str *tmp = list;
    str_to_int(tmp->str, info.auto_plants);
    tmp = tmp->next;
    str_to_int(tmp->str, info.plants);
    tmp = tmp->next;
    str_to_int(tmp->str, info.money);
    tmp = tmp->next;
    str_to_int(tmp->str, info.prod);
    tmp = tmp->next;
    str_to_int(tmp->str, info.raw);
    tmp = tmp->next;
    strcpy(info.name, tmp->str);
}

int Bot::Fill(list_str *list, char *str)
{
    int ret; 
    char *char_ptr;
    list_str *tmp=list;
    switch (str[2]){
        case 'M':
            market_fill(market, list);
            ret = market_enum;
            break;
        case 'P':
            if (str[5] != 'N'){
                tmp = list->next->next;
                str_to_int(tmp->str, players_num);
                ret = players_enum;
            }
            break;
        case 'O':
            ret = ok_enum;
            break;
        case 'B':
            if (str[3] == 'A'){
                if(strcmp(my_info.name, tmp->str) == 0){
                    ret = endgame_enum;
                    printf("bankrupt\n");
                }else 
                    ret = endturn_enum;
                break;
            } 
        case 'S': 
            if (str[3] == 'O'){
                auction_fill(auction[auction_ptr], list);
                auction_ptr++;
                ret = auction_enum;
            } else 
                ret = start_enum;
            break;
        case 'I':
            if (players_num == 0)
                break;
            info_fill(info[info_ptr], list);
            if (strcmp(info[info_ptr].name, my_info.name) == 0){
                char_ptr = my_info.name;
                my_info = info[info_ptr];
                my_info.name = char_ptr;
            }
            info_ptr++;
            ret = info_enum;
            break;
        case 'E':
            ret = endturn_enum;
            break;
        case 'W': 
        case 'N':
        case 'Y':
            ret = endgame_enum;        
    }
    return ret;
}                                        

int Bot::StrParsing(char *&str)
{
    int ch = str_taken;
    list_str *list; 
    if (str[0] == '&' && str[1] == ' '){
        list = str_break(str);
        ch = Fill(list, str);
        delete_list(&list);
    } 
    if (str[0] == '@'){
        if (str[1] == '+')
            return inc_enum;
        if (str[1] == '-')
            return dec_enum;    
    } 
    delete [] str;
    return ch;
}

int Bot::CheckBuf()
{
    int i, j;
    char *str;
    for (i=0; i<buf_ptr; i++)
        if (buf[i] == '\n')
            break;
    if (i == buf_ptr)
        return not_taken;
    str = new char [i+1];
    for (j=0; j<i; j++)
        str[j] = buf[j];
    str[j] = 0;
    for (j=0; j<(buf_ptr-i-1); j++)
        buf[j] = buf[j+i+1];
    buf_ptr -= i+1;
    printf("SERV: %s\n",str);
    return StrParsing(str);
}

int Bot::ReadFromServ()
{
    int ch,rd=0;
    if (mem_buf_ptr == buf_ptr){
        ch = read(fd, buf+buf_ptr, buf_size-buf_ptr);
        if (ch <= 0){
            perror("read");
            return -1;
        }
        buf_ptr += ch;
    }
    mem_buf_ptr = buf_ptr;
    do {
        ch = CheckBuf();
        if (ch != not_taken && ch != str_taken)
            rd = ch;
        if (ch == endgame_enum)
            return ch;
    } while (ch != not_taken && rd != market_enum 
            && rd != endturn_enum);
    if (buf_ptr >= buf_size)
        buf_incr(buf, buf_size);
    //printf("BUFFER SIZE %d PTR %d RD %d MPTR %d\n",buf_size,buf_ptr,rd,mem_buf_ptr);
    return rd;
}
    
int Bot::ConnectServer(char *str_ip, char *str_port)
{
    int ch, port;
    struct sockaddr_in addr;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1){
        perror("error: socket");
        return -1;
    }
    ch = str_to_int(str_port, port);
    if (ch == -1){
        perror("error: port");
        return -1;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (!inet_aton(str_ip, &(addr.sin_addr))){
        perror("error: ip");
        return -1;
    }
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr))){
        perror("error: connect");
        return -1;
    }
    return 0;
}
    
int Bot::WriteToServ(const char *str, int n1, int n2){
    //printf("===%s\n",str);
    int ch;
    char *tmp;
    ch = write(fd,str,strlen(str));
    if (ch == -1){
        perror("write"); 
        return -1;
    }
    if (n1 != 0){
        ch = write(fd," ",1);
        if (ch == -1){ 
            perror("write");
            return -1;
        }
        tmp = int_to_str(n1);
        ch = write(fd,tmp,strlen(tmp));
        if (ch == -1){
            perror("write"); 
            return -1;
        }
        delete [] tmp;
        if (n2 != 0){
            ch = write(fd," ",1);
            if (ch == -1){ 
                perror("write");
                return -1;
            }
            tmp = int_to_str(n2);
            ch = write(fd,tmp,strlen(tmp));
            if (ch == -1){
                perror("write");
                return -1;
            }
            delete [] tmp;
        }
    }
    ch = write(fd, "\n", 1);
    if (ch == -1){
        perror("write");
        return -1;
    }
    return 0;
}         

int Bot::SetName(const char *str)
{
    strcpy(my_info.name, str);
    return WriteToServ(my_info.name);
}

void Bot::PrintState()
{
     int i;
     printf("\n\n\n---market---\n%d %d %d %d\n", market.raw, market.min,
             market.prod, market.max);
     printf("---info---\n");
     for (i=0; i<players_num; i++)
         printf("%s %d %d %d %d %d\n", info[i].name, info[i].raw,
                 info[i].prod, info[i].money, info[i].plants,
                 info[i].auto_plants);
     printf("---auction---\n");
     for (i=0; i<2*players_num; i++)
         if (auction[i].name[0] != 0)
             printf("%s %d %d\n", auction[i].name, auction[i].amount, 
                     auction[i].price);
     printf("\n\n\n");
}                          

struct parent{
    Bot bot;
    Rpn rpn;
    int MainCycle();
};



void Bot::ClearAuc()
{
    int i,j;
    for (i=auction_ptr; i<2*players_num; i++){
        for (j=0; j<max_name_len; j++)
            auction[i].name[j] = 0;
        auction[i].price = 0;
        auction[i].amount = 0;
    }
}

int parent::MainCycle()
{
    int rd;
    for(;;){
        bot.Turn();
        if (bot.WriteToServ("market") == -1)
            return -1;
        do{
            //printf("cycle 1\n");
            rd = bot.ReadFromServ();
            if (rd == -1)
                return -1;
            if (rd == endgame_enum)
                return 0;
        } while (rd != market_enum);
        if (bot.WriteToServ("info") == -1)
            return -1;
        bot.InfoPtr() = 0;
        do{
            //printf("cycle 2\n");
            rd = bot.ReadFromServ();
            if (rd == -1)
                return -1;
            if (rd == endgame_enum)
                return 0;
            if (rd == dec_enum)
                bot.Players_num()--;
        } while (bot.InfoPtr() != bot.Players_num());
        
#if 0
        int wr;
        if (WriteToServ("buy", 2, market.min) == -1) 
            return -1;
        if (WriteToServ("sell", 2, market.max) == -1)
            return -1;
        if (my_info.raw == 1)
            wr = WriteToServ("prod", 1);
        if (my_info.raw >= 2)
            wr = WriteToServ("prod", 2);
        if (wr == -1)
            return -1;
        if (WriteToServ("turn") == -1)
            return -1;*/
#endif
        bot.FillTable();
        //bot.PrintTable(); 
        rpn.Start(&bot);

        bot.AucPtr() = 0;
        do {
            //printf("cycle 3\n");
            rd = bot.ReadFromServ();
            if (rd == -1)
                return -1;
            if (rd == endgame_enum)
                return 0;
        } while (rd != endturn_enum);
        bot.ClearAuc();
    }
    return 0;
}
            
int Bot::Join(const char *where)
{
    int rd, wr;
    const char *join=".join ";
    wr = write(fd, join, strlen(join));
    if (wr == -1){
        perror("write");
        return -1;
    }
    wr = WriteToServ(where);
    if (wr == -1)
        return -1;
    do {
        rd = ReadFromServ();
        if (rd == -1)
            return -1;
    } while (rd != start_enum);
    return 0;
}

int Bot::Create(int pl_num)
{
    int rd, wr, cur_num=0;
    const char *create=".create", *start="start";
    wr = WriteToServ(create);
    if (wr == -1)
        return -1;
    while(cur_num != pl_num){
        rd = ReadFromServ();
        if (rd == -1)
            return -1;
        if (rd == inc_enum)
            cur_num++;
        if (rd == dec_enum)
            cur_num--;
    };
    wr = WriteToServ(start);
    if (wr == -1)
        return -1;
    return 0;
}

int game(char **argv)
{
    int pl_num;
    parent par;
    //Bot bot;
    if (par.bot.ConnectServer(argv[1], argv[2]) == -1)
        return -1;
    if (par.bot.SetName(argv[5]) == -1)
        return -1;
    if (strcmp(argv[3], "join") == 0){
        if (par.bot.Join(argv[4]) == -1) 
            return -1;
    } else 
    if (strcmp(argv[3], "create") == 0){
        if (str_to_int(argv[4],  pl_num) == -1 || pl_num < 2){
            fprintf(stderr, "incorrect fifth arg: <num> ( >=2 )\n");
            return -1; 
        }
        if (par.bot.Create(pl_num) == -1)
            return -1;
    } else {
        fprintf(stderr,"incorrect fourth arg: \"join\" or \"create\"\n");
        return -1;
    }
    if (par.bot.TakePlayersNum() == -1)
        return -1;
    return par.MainCycle();
}

int main(int argc, char **argv)
{
    int fd;
    srand(time(NULL));
    if (argc != 7){
        fprintf(stderr, "incorrect args:\n"
                        "<ip> <port> \"join\" <num>/<name> <bot_name> <file.txt>\n"
                        "or\n"
                        "<ip> <port> \"create\" <players_num> <bot_name> <file.txt>\n");
        return 0;
    }
    fd = open(argv[6], O_RDONLY);
    if (fd == -1){
        perror(argv[6]);
        return 0;
    }
    dup2(fd,0);
    close(fd);
    return game(argv);
}
        