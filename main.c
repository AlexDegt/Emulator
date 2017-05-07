#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <conio.h>
#include <stdbool.h>

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned short int adr;

struct SSDD
{
    byte num_reg;
    word val;
    adr _adr;
    bool in_reg;
    bool inc;
}ss, dd;

byte mem[1024 * 64];
word reg[8];
#define pc reg[7]

byte b_read (adr a)
{
    return mem[a];
}

void b_write (adr a, byte val)
{
    mem[a] = val;
}

word w_read (adr a)
{
    return (word)mem[a] + (word)(256*mem[a+1]);
}

void w_write(adr a, word val)
{
    mem[a] = (byte)(val % 256);
    mem[a+1] = (byte)(val / 256);
}

void load_file()
{
    FILE * f = fopen("sum.txt", "r");
    unsigned int a, n, x, i;
    int eof = 0;
    while (eof != EOF)
    {
        eof = fscanf(f, "%x", &a);
        eof = fscanf(f, "%x", &n);
        if (eof == EOF) break;
        for (i = 0; i < n; i++)
        {
            fscanf(f, "%x", &x);
            b_write((adr)(a+i), x);
           //printf("%x\n", x);
        }
    }
}

void mem_dump(adr start, word n)
{
    int i;
    /*for (i = 0; i < n/2; i++)
    {
        printf("%06o : %06o\n", start+(i*2), w_read(start+(i*2)));
    }*/
    for (i = 0; i < 7; i++)
    {
        printf("reg[%i] = %o\n", i, reg[i]);
    }
}

void do_halt()
{
    printf("\nTHE END!\n");
    mem_dump(01000, 0x0c);
    getch();
    exit(0);
}

void do_move()
{
    printf(" reg[%i] = %o", ss.num_reg, reg[ss.num_reg]);
    reg[dd.num_reg] = ss.val;
    word is_byte_cmd = w_read(pc-2);
    //printf("\nis_byte_cmd = %o", is_byte_cmd);
    is_byte_cmd = is_byte_cmd & 1;
    //printf("\nis_byte_cmd = %o\n", is_byte_cmd);
    if ((ss.inc) && (ss.num_reg != 7))
    {
        if (is_byte_cmd)
            reg[ss.num_reg]++;
        else
            reg[ss.num_reg] += 2;
    }
    printf(" reg[%i] = %o", ss.num_reg, reg[ss.num_reg]);
}

void do_add()
{
    reg[dd.num_reg] += reg[ss.num_reg];
    printf(" reg[%i] = %o", dd.num_reg, reg[dd.num_reg]);
}

void do_clr()
{
    byte is_byte_cmd = b_read(mem[pc-2]);
    is_byte_cmd == is_byte_cmd >> 13;
    is_byte_cmd == is_byte_cmd & 0x7;
    if (is_byte_cmd)
        b_write(reg[dd.num_reg], 0);
    else
        w_write(reg[dd.num_reg], 0);
    printf(" reg[%i] = %o", dd.num_reg, reg[dd.num_reg]);
}

void do_sob()
{
    byte num_reg = mem[pc-2] & 0x1C0;
    byte num = mem[pc-2] & 0x3F;
    num_reg = num_reg >> 6;
    if (--reg[num_reg])
        pc = pc - 2*(num);
}

void do_nothing()
{}

void get_arg_dd(word w)
{
    unsigned int n1 = w & 0x7;
    unsigned int mode1 = w >> 3;
    mode1 = mode1 & 0x7;
    //printf("\n%o", n1);
    dd.in_reg = false;
    dd.inc = false;
    switch(mode1)
    {
        case 0:   //R1
        {
            dd.in_reg = true;
            dd.num_reg = (byte) n1;
            //printf("\n%o", dd.num_reg);
            break;
        }
        case 1:   //(R1)
        {
            dd.num_reg = (byte) n1;
            dd.val = mem[reg[dd.num_reg]];
            dd._adr = reg[dd.num_reg];
            break;
        }
        case 2:
        {
            //printf("\n!");
            dd.num_reg = (byte) n1;
            dd.inc = true;
            dd._adr = reg[dd.num_reg];
            dd.val = mem[reg[dd.num_reg]];
            break;
        }
        case 3:
        {
            dd.num_reg = (byte) n1;
            dd._adr = mem[pc];
            dd.val = mem[dd._adr];
            break;
        }
        case 4:
        {
            dd.num_reg = (byte) n1;
            reg[n1]--;
            reg[n1]--;
            dd.val = mem[reg[n1]];
            dd._adr = reg[n1];
            break;
        }
        case 5:
        {
            dd.num_reg = (byte) n1;
            reg[n1]--;
            reg[n1]--;
            dd._adr = mem[reg[n1]];
            dd.val = mem[dd._adr];
            break;
        }
        case 6:
        {
            dd.num_reg = (byte) n1;
            dd._adr = mem[pc];
            pc += 2;
            dd._adr += pc;
            dd.val = mem[dd._adr];
            break;
        }
        case 7:
        {
            dd.num_reg = (byte) n1;
            dd._adr = reg[n1] + mem[pc];
            dd.val = mem[dd._adr];
            break;
        }
    }
}

void get_arg_ss(word w)
{
    unsigned int n0 = w >> 6;
    n0 = n0 & 0x7;
    unsigned int mode0 = w >> 9;
    mode0 = mode0 & 0x7;
    //printf("\n%o", mode0);
    ss.in_reg = false;
    ss.inc = false;
    switch(mode0)
    {
        case 0:   //R1
        {
            ss.in_reg = true;
            ss.num_reg = (byte) n0;
            //printf("\n%u", qq.val);
            break;
        }
        case 1:   //(R1)
        {
            ss.num_reg = (byte) n0;
            ss.val = mem[reg[ss.num_reg]];
            ss._adr = reg[ss.num_reg];
            break;
        }
        case 2:
        {
            //printf("\n!");
            ss.num_reg = (byte) n0;
            ss.inc = true;
            ss._adr = reg[ss.num_reg];
            ss.val = mem[reg[ss.num_reg]];
            //printf("\n%o", ss.val);
            break;
        }
        case 3:
        {
            ss.num_reg = (byte) n0;
            ss._adr = mem[pc];
            ss.val = mem[ss._adr];
            break;
        }
        case 4:
        {
            ss.num_reg = (byte) n0;
            reg[n0]--;
            reg[n0]--;
            ss.val = mem[reg[n0]];
            ss._adr = reg[n0];
            break;
        }
        case 5:
        {
            ss.num_reg = (byte) n0;
            reg[n0]--;
            reg[n0]--;
            ss._adr = mem[reg[n0]];
            ss.val = mem[ss._adr];
            break;
        }
        case 6:
        {
            ss.num_reg = (byte) n0;
            ss._adr = mem[pc];
            pc += 2;
            ss._adr += pc;
            ss.val = mem[ss._adr];
            break;
        }
        case 7:
        {
            ss.num_reg = (byte) n0;
            ss._adr = reg[n0] + mem[pc];
            ss.val = mem[ss._adr];
            break;
        }
    }
}

struct Command
{
    word mask;
    word opcode;
    char * name;
    void (*do_action)();
    int has_ss;
    int has_dd;
    int has_nn;
    int has_xx;
    int has_r;
};

struct Command command_list[] = {
    {0xFFFF,    0,      "HALT", do_halt, 0, 0, 0, 0, 0},
    {070000, 0010000,  "MOV",  do_move, 1, 1, 0, 0, 0},
    {0170000, 0060000,  "ADD",  do_add , 1, 1, 0, 0, 0},
    {077700, 05000,  "CLR",  do_clr , 0, 1, 0, 0, 0},
    {0x7E00, 0x7E00,  "SOB",  do_sob , 0, 0, 1, 0, 1},
    {0,    0,   "nothing", do_nothing, 0, 0, 0, 0, 0}
};

void run_program()
{
    pc = 01000;
    int i;
    while(1)
    {
        word w = w_read(pc);
        printf("%06o : %06o ", pc, w);
        pc += 2;

        for (i = 0; i < 6 ; i++)
        {
            struct Command cmd = command_list[i];
            if ((w & cmd.mask) == cmd.opcode)
            {
                printf("%s ", cmd.name);
                if (cmd.has_ss)
                    get_arg_ss(w);
                if (cmd.has_dd)
                    get_arg_dd(w);
                cmd.do_action();
                break;
            }
        }
        printf("\n");
        //printf("%i\n", reg[1]);
    }
}

int main()
{
    load_file();
    int i;
    //mem_dump(01000, 0x0c);
    run_program();
    system("PAUSE");
    return 0;
}
