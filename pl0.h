#include <stdio.h>

#define norw       11             // 关键字的个数
#define txmax      100            // 名字表的容量
#define nmax       14             // 数字的最大位数
#define al         10             // 符号的最大长度
#define amax       2047           // 地址上界
#define levmax     3              // 最大允许的嵌套声明层数
#define cxmax      2000           // 最多的虚拟机代码数

//算数运算符以及逻辑运算符

#define nul	       0x1 
#define ident      0x2
#define number     0x4
#define plus       0x8          // +
#define minus      0x10         // -    
#define times      0x20         // *
#define slash      0x40         // /
#define oddsym     0x80         
#define eql        0x100        // =
#define neq        0x200        // #
#define lss        0x400        // <
#define leq        0x800        // <=
#define gtr        0x1000       // >
#define geq        0x2000       // >=
#define lparen     0x4000       //   左括号
#define rparen     0x8000       //   右括号
#define comma      0x10000      //   逗号
#define semicolon  0x20000      //   分号
#define period     0x40000      //   句号
#define becomes    0x80000      //   赋值

//系统保留字

#define beginsym   0x100000
#define endsym     0x200000
#define ifsym      0x400000
#define thensym    0x800000
#define whilesym   0x1000000
#define dosym      0x2000000
#define callsym    0x4000000      
#define constsym   0x8000000
#define varsym     0x10000000
#define procsym    0x20000000

//枚举类型

enum object {
    constant, variable, proc                        // 常量、变量、程序
};

enum fct {
    lit, opr, lod, sto, cal, Int, jmp, jpc         // 请看62-69行的具体操作翻译
};

/*
    f为功能码；l表示层次差，即变量或过程被引用的分程序与说明该变量或过程的分程序之间的层次差；
    a的含义对不同的指令有所区别，可以是常数值、位移量、操作符代码等。
*/

typedef struct{
    enum fct f;		// 虚拟机代码指令
    long l; 		// 引用层与声明层的层差
    long a; 		// 位移地址
} instruction;
/*  lit 0, a : 加载常量 a
    opr 0, a : 执行操作 a
    lod l, a : 加载变量 l, a
    sto l, a : 存储变量 l, a
    cal l, a : 在l层调用操作a
    Int 0, a : 通过a增加t寄存器？ -increment t-register by a
    jmp 0, a : 跳转到a
    jpc 0, a : 跳转条件到a？ -jump conditional to a      
 */

char ch;               // last character read -最后读入的字符，下同
unsigned long sym;     // last symbol read
char id[al+1];         // last identifier read
long num;              // last number read
long cc;               // character count -当前字符的位置？
long ll;               // line length -一行代码的字符长度？
long kk, err;
long cx;               // 代码定位的索引号？ -code allocation index

char line[81];          // 读取行缓冲区
char a[al+1];           // 临时符号 ,多处的字节用于存放;al是符号的最大长度
instruction code[cxmax+1];            // cxmax最多的虚拟机代码数
char word[norw][al+1];                // 保留字
unsigned long wsym[norw];             // 保留字对应的符号值
unsigned long ssym[256];              // 单符号的符号值

char mnemonic[8][3+1];                // 虚拟机代码指令的名称
// 声明开始的符号、语句开始的符号、因子开始的符号
unsigned long declbegsys, statbegsys, facbegsys;       

// 名字表
struct{
    char name[al+1];            // 名字
    enum object kind;           // 类型
    long val;                   // 数值，仅 const 使用
    long level;                 // 所处层
    long addr;                  // 地址
}table[txmax+1];

char infilename[80];            //导入文件名
FILE* infile;

// 下面的变量是block中的；block是分析程序处理过程
long dx;		// 数据分配索引号
long lev;		// 当前block嵌套的深度
long tx;		// 当前table的索引号

// 下面的数组空间是为解释器准备的
#define stacksize 50000
long s[stacksize];	// 数据存储
