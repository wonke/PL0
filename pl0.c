// pl/0 compiler with code generation
#include <stdlib.h>
#include <string.h>
#include "pl0.h"


/*

函数功能说明表：

过程或函数名			功能简要说明

p10					主程序
error 				出错处理，打印出错位置和错误编码
getsym				词法分析，读取一个单词
getch				漏掉空格，读取一个字符
gen 				生成目标代码，并送入目标程序区
test 				测试当前单词符是否合法
block 				分析程序处理过程
enter				登录名字表
position(函数)		查找标识符在名字表中的位置
constdeclaration	常量定义处理
vardeclaration		变量定义处理
listcode			列出目标代码清单
statement			语句部分处理
expression			表达式处理
term				项处理
factor				因子处理
condition			条件处理
interpret			对目标代码的解释执行程序
base(函数)			通过静态链求出数据区的基地址

*/





void error(long n){
    long i;

    printf(" ****");
    for (i=1; i<=cc-1; i++){
	printf(" ");
    }
    printf("^%2d\n",n);
    err++;
}

// 读取一个字符
void getch() {
	// 该if语句并不是主要执行的
    if(cc==ll){
	if(feof(infile)){	// feof()检测流上的文件结束符，如果文件结束，则返回非0值，否则返回0
	    printf("************************************\n");
	    printf("      program incomplete\n");
	    printf("************************************\n");
	    exit(1);
	}
	ll=0; cc=0;
	printf("%5d ", cx);
	// 循环读出一行代码并输出，读到换行符\n便结束循环
	while((!feof(infile))&&((ch=getc(infile))!='\n')){		// getc()从指定的流获取下一个字符，并把位置标识符往前移动
	    printf("%c",ch);
	    ll=ll+1; line[ll]=ch;								// 存储一行代码的数组
	}
	printf("\n");
	ll=ll+1; line[ll]=' ';
    }
    // 主要执行语句。字符位置加1，往line数组中加入读到的字符
    cc=cc+1; ch=line[cc];
}

// getsym()函数的功能就是取一个数据单位（单词），通过调用该函数可以一次取出一个数字或字符串或运算符
// 每调用一次该函数，该函数就会去调用 getch 函数
// sym 记录的是语句单位的类型， num中存放的是数字的大小

void getsym(){
    long i,j,k;

    // 忽略空格，换行符，制表符
    while(ch==' '||ch=='\t'){
	getch();						// 取一个字符
    }
    if(isalpha(ch)){ 				// isalpha()判断字符ch是否为英文字母，若为英文字母，返回非0（小写字母为2，大写字母为1）。若不是字母，返回0。
	k=0;
	do{
	    if(k<al){					// al是符号的最大长度
		a[k]=ch; k=k+1;				// 将一个单词符号放入 a[] 数组中
	    }
	    getch();
	}while(isalpha(ch)||isdigit(ch));	// isdigit()若参数c为阿拉伯数字0~9，则返回非0值，否则返回0
	if(k>=kk){
	    kk=k;
	}else{
	    do{
		kk=kk-1; a[kk]=' ';
	    }while(k<kk);
	}
	strcpy(id,a); i=0; j=norw-1;		// 将该单词符号复制到 id[] 数组中
	// 执行二分查找，搜索该单词符号是否是保留字
	do{
	    k=(i+j)/2;
	    //strcmp() 比较两个字符串，相同则为0，第一个小为负，第一个大为正数
	    if(strcmp(id,word[k])<=0){
		j=k-1;
	    }
	    if(strcmp(id,word[k])>=0){
		i=k+1;
	    }
	}while(i<=j);
	if(i-1>j){
	    sym=wsym[k];			// 如果是保留字，则置为保留字的数字, wsym是保留字对应的符号值
	}else{
	    sym=ident;				// 如果不是保留字，则置 sym为ident
	}
    }else if(isdigit(ch)){ 		// 如果字符是数字（0-9）
	k=0; num=0; sym=number;
	do{
	    num=num*10+(ch-'0');
	    k=k+1; getch();
	}while(isdigit(ch));		// 通过循环得到数字的值
	if(k>nmax){					// 数字位数不能超过规定的 数字的最大位数
	    error(31);
	}
    }else if(ch==':'){
	getch();
	if(ch=='='){				//读取到'='，意味着赋值
	    sym=becomes; getch();
	}else{
	    sym=nul;
	}
    }else if(ch=='<'){
	getch();
	if(ch=='='){
	    sym=leq; getch(); 		
	}else if(ch=='>'){
	    sym=neq; getch();
	}else{
	    sym=lss;
	}
    }else if(ch=='>'){
	getch();
	if(ch=='='){
	    sym=geq; getch();
	}else{
	    sym=gtr;
	}
    }else{
	sym=ssym[(unsigned char)ch]; getch();
    }
}

// gen()生成目标代码，并送入目标程序区
/*
PL/0语言的代码生成是由GEN完成。GEN由三个参数，分别代表目标代码的功能码、层差和位
移量（或常数值、或操作符编码）。生成的代码顺序放在数组CODE中。CODE为一维数组，数组元
数为记录型数据。的一条记录是一个目录指令。CX为指令指针，由0开始顺序增加。
目标代码的顺序是内层过程的排在前边，主程序的目标代码排在最后。
*/
void gen(enum fct x, long y, long z){
    if(cx>cxmax){
	printf("program too long\n");
	exit(1);
    }
    code[cx].f=x; code[cx].l=y; code[cx].a=z;
    cx=cx+1;
}

/*
当语法分析进入某些关键字或终结符号集合为开始符号的语法单元时，在其入口或出口调用一
个测试程序TEST。

S1：当语法分析进入或退出某一语法单元时当前单词
符号应属于的集合，他可能是一个语法单元的开始符号集
合，也可能是一个语法单元的后继符号集合。
S2：在某一出错状态时，可恢复语法分析继续工作的
补充单词符号集合。因为当语法分析出错时，即当前单词
符号不再集合S1中，为了继续编译，需跳过后遍输入的一
写单词符号，直到当前输入的单词符号属于S1和S2其和。
n：出错信息编号。
*/

void test(unsigned long s1, unsigned long s2, long n){
    if (!(sym & s1)){
	error(n);
	s1=s1|s2;
	while(!(sym & s1)){
	    getsym();
	}
    }
}

/*
登录名字表
该函数的功能是将源程序中的常量，变量，分程序符号串输入到 table 表中

所造表放在全程量一维数组TABLE中，TABLE的元素为记录型数据。TX为索引表的指针，LEV
给出层次，DX给出每层局部变量当前已分配到的相对位置，可称位地址指示器，每说明完一个变量
后DX加1。
*/

void enter(enum object k){		
    tx=tx+1;
    strcpy(table[tx].name,id);
    table[tx].kind=k;
    switch(k){
	case constant:
	    if(num>amax){
		error(31);
		num = 0;
	    }
	    table[tx].val=num;
	    break;
	case variable:
	    table[tx].level=lev; table[tx].addr=dx; dx=dx+1;
	    break;
	case proc:
	    table[tx].level=lev;
	    break;
    }
}

// 在table 表中查找id的位置，并返回其所在位置

long position(char* id){	
    long i;

    strcpy(table[0].name,id);
    i=tx;
    while(strcmp(table[i].name,id)!=0){
	i=i-1;
    }
    return i;
}

// 常量定义处理

void constdeclaration(){
	// 如果当前的符号是一个变量，则继续执行
    if(sym==ident){
	getsym();
	if(sym==eql||sym==becomes){
	    if(sym==becomes){
		error(1);
	    }
	    getsym();
	    if(sym==number){
		enter(constant); getsym();
	    }else{
		error(2);
	    }
	}else{
	    error(3);
	}
    }else{
	error(4);
    }
}

// 变量定义处理

void vardeclaration(){
    if(sym==ident){
	enter(variable); getsym(); 			// 将变量放到table表中，再读入下一个单词
    }else{
	error(4);
    }
}

// 列出目标代码清单

void listcode(long cx0){	
    long i;

    for(i=cx0; i<=cx-1; i++){
    /* 打印出索引号，命令符，引用层与声明层的层差，唯一地址。
    f为功能码；l表示层次差，即变量或过程被引用的分程序与说明该变量或过程的分程序之间的层次差；
    a的含义对不同的指令有所区别，可以是常数值、位移量、操作符代码等。
   	*/
   	/*例：
		 2  int  0    5
         3  lod  1    3
         4  sto  0    3
   	*/
	printf("%10d%5s%3d%5d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
    }

}

void expression(unsigned long);

// 处理因子
/*
因子的开始符是“(”, ident, number。当语法分析进入这样的语法单元前，可
用测试程序检查当前单词符号是否属于他们开始符号的集合，若不是则出错。
*/

void factor(unsigned long fsys){
    long i;
	// 判断当前是否是因子的语法开始符号，如果不是，则继续调用getsysm()找到合适的符号
    test(facbegsys,fsys,24);
    while(sym & facbegsys){ 			// 当符号是因子的开始符号时
	if(sym==ident){
	    i=position(id); 				// 在table 表中查找id的位置，并返回其所在位置
	    if(i==0){
		error(11);
	    }else{
		switch(table[i].kind){
		    case constant:
			gen(lit,0,table[i].val); 		// 如果是常量，把数值送进栈
			break;
		    case variable:
			gen(lod,lev-table[i].level,table[i].addr); 		// 如果是变量，把变量放到数据栈栈顶，层次差为level，相对地址为addr
			break;
		    case proc:						// 如果是程序，则报错
			error(21);
			break;
		}
	    }
	    getsym();
	}else if(sym==number){
	    if(num>amax){
		error(31); num=0;
	    }
	    gen(lit,0,num);
	    getsym();
	}else if(sym==lparen){
	    getsym();
	    expression(rparen|fsys);
	    if(sym==rparen){
		getsym();
	    }else{
		error(22);
	    }
	}
	test(fsys,lparen,23);
    }
}

// 项处理

void term(unsigned long fsys){
    unsigned long mulop;

    factor(fsys|times|slash);
    while(sym==times||sym==slash){
	mulop=sym; getsym();
	factor(fsys|times|slash);
	if(mulop==times){
	    gen(opr,0,4);
	}else{
	    gen(opr,0,5);
	}
    }
}

// 表达式处理

void expression(unsigned long fsys){
    unsigned long addop;

    if(sym==plus||sym==minus){
	addop=sym; getsym();
	term(fsys|plus|minus);
	if(addop==minus){
	    gen(opr,0,1);
	}
    }else{
	term(fsys|plus|minus);
    }
    while(sym==plus||sym==minus){
	addop=sym; getsym();
	term(fsys|plus|minus);
	if(addop==plus){
	    gen(opr,0,2);
	}else{
	    gen(opr,0,3);
	}
    }
}

//  条件处理

void condition(unsigned long fsys){
    unsigned long relop;

    if(sym==oddsym){
	getsym(); expression(fsys);
	gen(opr,0,6);
    }else{
	expression(fsys|eql|neq|lss|gtr|leq|geq);
	if(!(sym&(eql|neq|lss|gtr|leq|geq))){
	    error(20);
	}else{
	    relop=sym; getsym();

	    expression(fsys);
	    switch(relop){
		case eql:
		    gen(opr,0,8);
		    break;
		case neq:
		    gen(opr,0,9);
		    break;
		case lss:
		    gen(opr,0,10);
		    break;
		case geq:
		    gen(opr,0,11);
		    break;
		case gtr:
		    gen(opr,0,12);
		    break;
		case leq:
		    gen(opr,0,13);
		    break;
	    }
	}
    }
}

// 语句部分处理;递归读取分析每一个语句

void statement(unsigned long fsys){
    long i,cx1,cx2;
	
	// 如果当前字符串是一个变量，则其后跟的是一个表达式

    if(sym==ident){
	i=position(id);
	if(i==0){
	    error(11);
	}else if(table[i].kind!=variable){	// 如果该字符串不是变量，则出错
	    error(12); i=0;
	}
	getsym();
	if(sym==becomes){
	    getsym();
	}else{
	    error(13);
	}
	expression(fsys);
	if(i!=0){
	    gen(sto,lev-table[i].level,table[i].addr);
	}
    }else if(sym==callsym){
	getsym();
	if(sym!=ident){
	    error(14);
	}else{
	    i=position(id);
	    if(i==0){
		error(11);
	    }else if(table[i].kind==proc){
		gen(cal,lev-table[i].level,table[i].addr);
	    }else{
		error(15);
	    }
	    getsym();
	}
    }else if(sym==ifsym){
	getsym(); condition(fsys|thensym|dosym);
	if(sym==thensym){
	    getsym();
	}else{
	    error(16);
	}
	cx1=cx;	gen(jpc,0,0);
	statement(fsys);
	code[cx1].a=cx;	
    }else if(sym==beginsym){
	getsym(); statement(fsys|semicolon|endsym);
	while(sym==semicolon||(sym&statbegsys)){
	    if(sym==semicolon){
		getsym();
	    }else{
		error(10);
	    }
	    statement(fsys|semicolon|endsym);
	}
	if(sym==endsym){
	    getsym();
	}else{
	    error(17);
	}
    }else if(sym==whilesym){
	cx1=cx; getsym();
	condition(fsys|dosym);
	cx2=cx;	gen(jpc,0,0);
	if(sym==dosym){
	    getsym();
	}else{
	    error(18);
	}
	statement(fsys); gen(jmp,0,cx1);
	code[cx2].a=cx;
    }
    test(fsys,0,19);
}
			
void block(unsigned long fsys){
    long tx0;		// initial table index
    long cx0; 		// initial code index
    long tx1;		// save current table index before processing nested procedures
    long dx1;		// save data allocation index

    dx=3; tx0=tx; table[tx].addr=cx; gen(jmp,0,0);
    if(lev>levmax){
	error(32);
    }
    do{
	if(sym==constsym){
	    getsym();
	    do{
		constdeclaration();
		while(sym==comma){
		    getsym(); constdeclaration();
		}
		if(sym==semicolon){
		    getsym();
		}else{
		    error(5);
		}
	    }while(sym==ident);
	}
	if(sym==varsym){
	    getsym();
	    do{
		vardeclaration();
		while(sym==comma){
		    getsym(); vardeclaration();
		}
		if(sym==semicolon) {
		    getsym();
		}else{
		    error(5);
		}
	    }while(sym==ident);
	}
	while(sym==procsym){
	    getsym();
	    if(sym==ident){
		enter(proc); getsym();
	    }else{
		error(4);
	    }
	    if(sym==semicolon){
		getsym();
	    }else{
		error(5);
	    }
	    lev=lev+1; tx1=tx; dx1=dx;
	    block(fsys|semicolon);
	    lev=lev-1; tx=tx1; dx=dx1;
	    if(sym==semicolon){
		getsym();
		test(statbegsys|ident|procsym,fsys,6);
	    }else{
		error(5);
	    }
	}
	test(statbegsys|ident,declbegsys,7); 		// 测试当前符号是否是后继符号集中的元素
    }while(sym&declbegsys);
    code[table[tx0].addr].a=cx;
    table[tx0].addr=cx;		// start addr of code
    cx0=cx; gen(Int,0,dx);
    statement(fsys|semicolon|endsym);
    gen(opr,0,0); // return
    test(fsys,0,8);
    listcode(cx0);
}

long base(long b, long l){
    long b1;

    b1=b;
    while (l>0){	// find base l levels down
	b1=s[b1]; l=l-1;
    }
    return b1;
}

// 对目标代码的解释执行程序

void interpret(){
    long p,b,t;		// 程序-, 基址-, 寄存器的栈顶
    instruction i;	// instruction register

    printf("start PL/0\n");
    t=0; b=1; p=0;
    s[1]=0; s[2]=0; s[3]=0;
    do{
	i=code[p]; p=p+1; 		// code 中存放的就是源程序的模拟程序，取出每一条指令
	switch(i.f){
	    case lit:
		t=t+1; s[t]=i.a;
		break;
	    case opr:
		switch(i.a){ 	// operator
		    case 0:	// return
			t=b-1; p=s[t+3]; b=s[t+2];
			break;
		    case 1:
			s[t]=-s[t];
			break;
		    case 2:
			t=t-1; s[t]=s[t]+s[t+1];
			break;
		    case 3:
			t=t-1; s[t]=s[t]-s[t+1];
			break;
		    case 4:
			t=t-1; s[t]=s[t]*s[t+1];
			break;
		    case 5:
			t=t-1; s[t]=s[t]/s[t+1];
			break;
		    case 6:
			s[t]=s[t]%2;
			break;
		    case 8:
			t=t-1; s[t]=(s[t]==s[t+1]);
			break;
		    case 9:
			t=t-1; s[t]=(s[t]!=s[t+1]);
			break;
		    case 10:
			t=t-1; s[t]=(s[t]<s[t+1]);
			break;
		    case 11:
			t=t-1; s[t]=(s[t]>=s[t+1]);
			break;
		    case 12:
			t=t-1; s[t]=(s[t]>s[t+1]);
			break;
		    case 13:
			t=t-1; s[t]=(s[t]<=s[t+1]);
		}
		break;
	    case lod:
		t=t+1; s[t]=s[base(b,i.l)+i.a];
		break;
	    case sto:
		s[base(b,i.l)+i.a]=s[t]; printf("%10d\n", s[t]); t=t-1;
		break;
	    case cal:		// generate new block mark
		s[t+1]=base(b,i.l); s[t+2]=b; s[t+3]=p;
		b=t+1; p=i.a;
		break;
	    case Int:
		t=t+i.a;
		break;
	    case jmp:
		p=i.a;
		break;
	    case jpc:
		if(s[t]==0){
		    p=i.a;
		}
		t=t-1;
	}
    }while(p!=0);
    printf("end PL/0\n");
}

main(){

	//初始化，单符号ssym为nul
    long i;
    for(i=0; i<256; i++){
	ssym[i]=nul;
    }
    // 将保留字输入 word数组中
    strcpy(word[0],  "begin     ");
    strcpy(word[1],  "call      ");
    strcpy(word[2],  "const     ");
    strcpy(word[3],  "do        ");
    strcpy(word[4],  "end       ");
    strcpy(word[5],  "if        ");
    strcpy(word[6],  "odd       ");
    strcpy(word[7],  "procedure ");
    strcpy(word[8],  "then      ");
    strcpy(word[9],  "var       ");
    strcpy(word[10], "while     ");
    //保留字，wsym内放有保留字所对应的枚举变量的值
    wsym[0]=beginsym;
    wsym[1]=callsym;
    wsym[2]=constsym;
    wsym[3]=dosym;
    wsym[4]=endsym;
    wsym[5]=ifsym;
    wsym[6]=oddsym;
    wsym[7]=procsym;
    wsym[8]=thensym;
    wsym[9]=varsym;
    wsym[10]=whilesym;
    //单符号
    ssym['+']=plus;
    ssym['-']=minus;
    ssym['*']=times;
    ssym['/']=slash;
    ssym['(']=lparen;
    ssym[')']=rparen;
    ssym['=']=eql;
    ssym[',']=comma;
    ssym['.']=period;
    ssym[';']=semicolon;
    //mnemonic 中存放的是模拟程序的命令符
    strcpy(mnemonic[lit],"lit");
    strcpy(mnemonic[opr],"opr");
    strcpy(mnemonic[lod],"lod");
    strcpy(mnemonic[sto],"sto");
    strcpy(mnemonic[cal],"cal");
    strcpy(mnemonic[Int],"int");
    strcpy(mnemonic[jmp],"jmp");
    strcpy(mnemonic[jpc],"jpc");
    declbegsys=constsym|varsym|procsym;							//声明开始的符号集
    statbegsys=beginsym|callsym|ifsym|whilesym;					//语句开始的符号集
    facbegsys=ident|number|lparen;								//因子开始的符号集

    printf("please input source program file name: ");
    scanf("%s",infilename);
    printf("\n");
    if((infile=fopen(infilename,"r"))==NULL){
	printf("File %s can't be opened.\n", infilename);
	exit(1);
    }
    
    err=0;
    cc=0; cx=0; ll=0; ch=' '; kk=al; getsym();
    lev=0; tx=0;
    block(declbegsys|statbegsys|period);
    if(sym!=period){
	error(9);
    }
    if(err==0){
	interpret();
    }else{
	printf("errors in PL/0 program\n");
    }
    fclose(infile);
}
