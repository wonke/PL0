#include <stdio.h>

#define norw       11             // �ؼ��ֵĸ���
#define txmax      100            // ���ֱ������
#define nmax       14             // ���ֵ����λ��
#define al         10             // ���ŵ���󳤶�
#define amax       2047           // ��ַ�Ͻ�
#define levmax     3              // ��������Ƕ����������
#define cxmax      2000           // ���������������

//����������Լ��߼������

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
#define lparen     0x4000       //   ������
#define rparen     0x8000       //   ������
#define comma      0x10000      //   ����
#define semicolon  0x20000      //   �ֺ�
#define period     0x40000      //   ���
#define becomes    0x80000      //   ��ֵ

//ϵͳ������

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

//ö������

enum object {
    constant, variable, proc                        // ����������������
};

enum fct {
    lit, opr, lod, sto, cal, Int, jmp, jpc         // �뿴62-69�еľ����������
};

/*
    fΪ�����룻l��ʾ��β����������̱����õķֳ�����˵���ñ�������̵ķֳ���֮��Ĳ�β
    a�ĺ���Բ�ͬ��ָ���������𣬿����ǳ���ֵ��λ����������������ȡ�
*/

typedef struct{
    enum fct f;		// ���������ָ��
    long l; 		// ���ò���������Ĳ��
    long a; 		// λ�Ƶ�ַ
} instruction;
/*  lit 0, a : ���س��� a
    opr 0, a : ִ�в��� a
    lod l, a : ���ر��� l, a
    sto l, a : �洢���� l, a
    cal l, a : ��l����ò���a
    Int 0, a : ͨ��a����t�Ĵ����� -increment t-register by a
    jmp 0, a : ��ת��a
    jpc 0, a : ��ת������a�� -jump conditional to a      
 */

char ch;               // last character read -��������ַ�����ͬ
unsigned long sym;     // last symbol read
char id[al+1];         // last identifier read
long num;              // last number read
long cc;               // character count -��ǰ�ַ���λ�ã�
long ll;               // line length -һ�д�����ַ����ȣ�
long kk, err;
long cx;               // ���붨λ�������ţ� -code allocation index

char line[81];          // ��ȡ�л�����
char a[al+1];           // ��ʱ���� ,�ദ���ֽ����ڴ��;al�Ƿ��ŵ���󳤶�
instruction code[cxmax+1];            // cxmax���������������
char word[norw][al+1];                // ������
unsigned long wsym[norw];             // �����ֶ�Ӧ�ķ���ֵ
unsigned long ssym[256];              // �����ŵķ���ֵ

char mnemonic[8][3+1];                // ���������ָ�������
// ������ʼ�ķ��š���俪ʼ�ķ��š����ӿ�ʼ�ķ���
unsigned long declbegsys, statbegsys, facbegsys;       

// ���ֱ�
struct{
    char name[al+1];            // ����
    enum object kind;           // ����
    long val;                   // ��ֵ���� const ʹ��
    long level;                 // ������
    long addr;                  // ��ַ
}table[txmax+1];

char infilename[80];            //�����ļ���
FILE* infile;

// ����ı�����block�еģ�block�Ƿ������������
long dx;		// ���ݷ���������
long lev;		// ��ǰblockǶ�׵����
long tx;		// ��ǰtable��������

// ���������ռ���Ϊ������׼����
#define stacksize 50000
long s[stacksize];	// ���ݴ洢
