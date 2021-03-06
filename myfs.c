/******************************************
**             미완성부분                **
**          절대경로 상대경로            **
**      프로그램 종료 후 데이터 복원     **
**     정수영, 차윤성, 김누리, 이인후    **
******************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct bootblock{ // 부트블록
    short bootblock;
} bootblock;

typedef struct superblock{ // 슈퍼블록
    unsigned freeinode[16];
    unsigned freedatablock[32];
} superblock;

typedef struct inode{ // 아이노드
    char file_type;
    int file_creation_time[6];
    int file_size;
    int datablock_info[3];
} inode;

typedef struct datablock{ // 데이터블록
    char data[128];
} datablock;

typedef struct tree{ // 트리구조
    char name[5];
    int inum;
    struct tree * up;       //부모 트리
    struct tree * left;     //왼쪽 트리
    struct tree * right;    //오른쪽 트리
}tree;

tree * root;       //루트 트리
tree * now;        //현재 트리
tree * next;       //작업 트리
tree * work;       //작업 트리

int copy2(int, int, int); // 128 byte 초과하는 파일 복사 함수
int copy1(int, int); // 128 byte 이하의 파일 복사 함수
void mycp(char *, char *); // mycp
void mycpto(char *, char *); // mycpto
void freeinode(inode * in); // 아이노드 정보 제거
void freedatablock(datablock *); // 데이터블록 정보 제거
void myrm(char *); // myrm
void makedouble(char *, int , FILE *, tree *); // double indirect 생성
void makesingle(char *, int, FILE *, tree *); // single indirect 생성
void makedirect(char *, FILE *, tree *); // direct 생성
int find_free_superblock(unsigned); // 비트열 중 0 검색(가용 아이노드 및 데이터블록 검색)
void mytouch(char *); // mytouch
void mycpfrom(char *, char *); //mycpfrom
void mymkdir(char *); // mymkdir
void show_inode_info(inode in); // 아이노드 정보 출력
void myshowinode(char *); // myshowinode
void mycd(char *, int); // mycd
void myshowblock(char *); // myshowblock
void getroot(void); // 루트 디렉터리 할당
void get_directory(char *); // 디렉터리 할당
void myshowfile(char *, char *, char *); // myshowfile
void mystate(void); // mystate
void myshowfile(char *, char *, char *); // myshowfile
void classify(char *,char *,char *,char *,char *);
int getblock(datablock *, FILE *); // 데이터블록 할당
void set_type(inode *); // 아이노드에 파일의 정보(디렉터리 / 일반 파일) 저장
void set_time(inode *); // 아이노드에 파일 생성 시간 저장
void freebit(unsigned *, int); // 비트 1에서 0으로 바꿔주는 함수(아이노드, 데이터블록 제거 시 사용)
int free_print(unsigned); // mystate 비트 계산 함수
void freesuperblock_i(int, superblock *); // 슈퍼블록 사용 정보 제거(아이노드)
void freesuperblock_d(int, superblock *); // 슈퍼블록 사용 정보 제거(데이터블록)
void set_size(inode *, char *); // 파일 사이즈 저장
void set_datablock_info(int *, int); // 데이터블록 정보(direct, single, double) 저장
void get_single(char *, int, int); // 싱글에 10비트씩 데이터 인덱스 저장
int bit_print(char *, int);
int num_bit(char *, int);
void get_double(char *, int, int); //더블에 10비트씩 싱글 인덱스 저장
void getdata(datablock *, FILE *); //파일로부터 데이터 읽어서 데이터블록에 저장
int set_inode_info_d(char *); // 아이노드 할당(디렉터리)
int set_inode_info_f(char *); // 아이노드 할당(일반 파일)
int find_free_i(); // 가용 아이노드 검색
int find_free_d(); // 가용 데이터블록 검색
void set_superblock_i(unsigned *); // 슈퍼블록에 아이노드 사용 정보 저장
void set_superblock_d(unsigned *); // 슈퍼블록에 데이터블록 사용 정보 저장
void init();  // 트리구조 시작
void mypwd(); // mypwd
void mycd(char *, int); // mycd
void myls(char*, int);  // myls
void myrmdir(char *); // myrmdir
void mytree(char *); // mytree
void mymv(char *, char *); // mymv
void myfs_shell();  //쉘
void filesave();    //파일저장
void fileload();    //파일로드

bootblock b[1] = {0};
inode in[512] = {0};
datablock db[1024] = {0};
superblock sb[1] = {0};

int main(void){
    myfs_shell();
    return 0;
}

void mycpfrom(char * tmp2, char * tmp3)
{
    int a, b, count = 0;
    char file_name[5] = {0};
    int single_member[102] = {0};
    int single_block = 0;
    FILE * ifp;
    int blocks, remain, n, size = 0;
    strncpy(file_name, tmp3, 4);
    tree * file = NULL;
    file = (tree *)malloc(sizeof(tree));
    strncpy(file -> name,tmp3,4);
    
    ifp = fopen(tmp2, "rb");
    fseek(ifp, 0, SEEK_END);
    
    size = ftell(ifp);
    rewind(ifp);
    
    blocks = size / 128;
    remain = size % 128;
    
    
    if(size <= 128){
        makedirect(tmp2, ifp, file);
    }
    
    else if(size > 128 && size <= 128 * 102){
        makesingle(tmp2, blocks, ifp,file);
    }
    else{
        makedouble(tmp2, blocks, ifp,file);
    }
}

void mycpto(char * sourcefile, char * destfile)
{
    FILE *ofp;
    ofp = fopen(destfile, "wb+");
    int num, first_db_num, single_db_num, double_db_num, size;
    if(now->left==NULL)
        return;
    if(!strncmp(now->left->name,sourcefile,4))
        num = now->left->inum-1;
    else{
        next = now->left;
        while(next->right!=NULL){
            if(!strncmp(next->right->name,sourcefile,4)){
                num = next->right->inum-1;
                break;
            }
            next= next->right;
        }
    }
    first_db_num = in[num].datablock_info[0];
    single_db_num = in[num].datablock_info[1];
    double_db_num = in[num].datablock_info[2];
    
    while(1){
        if(single_db_num != 0 && double_db_num == 0){
            for(int i = 0; i < single_db_num - 2; i++){
                fwrite(&(db[first_db_num - 1 + i].data), 128, 1, ofp);
            }
            fclose(ofp);
            break;
        }
        else if(single_db_num != 0 && double_db_num != 0){
            for(int i = 0; i < single_db_num - 2; i++){
                fwrite(&(db[first_db_num - 1 + i].data), 128, 1, ofp);
            }
            for(int i = single_db_num - 1; i < double_db_num - 1; i++){
                fwrite(&(db[i].data), 128, 1, ofp);
            }
            fclose(ofp);
            break;
        }
        else{
            fwrite(&(db[first_db_num - 1].data), 128, 1, ofp);
            fclose(ofp);
            break;
        }
    }
}

void mycp(char * sourcefile, char * destfile) // mycp
{
    int num, size, blocks, first_db_num, double_db_num, single_db_num, tmp;
    tree * newfile;
    newfile = (tree *)malloc(sizeof(tree));
    if(now->left==NULL)
        return;
    if(!strncmp(now->left->name,sourcefile,4))
        num = now->left->inum-1;
    else{
        next = now->left;
        while(next->right!=NULL){
            if(!strncmp(next->right->name,sourcefile,4)){
                num = next->right->inum-1;
                break;
            }
            next= next->right;
        }
    }
    size = in[num].file_size;
    blocks = size / 128;
    
    first_db_num = in[num].datablock_info[0];
    single_db_num = in[num].datablock_info[1];
    double_db_num = in[num].datablock_info[2];
    
    if(size <= 128)
        tmp = copy1(size, first_db_num);
    else if(size > 128 && size <= 128 * 102)
        tmp = copy2(size, first_db_num, single_db_num);
    strncpy(newfile -> name, destfile, 4);
    newfile->inum = tmp;
    newfile->up = now;
    newfile->left=NULL;
    newfile->right=NULL;
    if(now->left==NULL)
        now->left = newfile;
    else{
        next = now->left;
        while(next->right!=NULL)
            next= next->right;
        next->right = newfile;
    }
    
}

int copy1(int size, int num){ // 128 byte 이하 파일 복사 함수
    int a, b, len;
    a = find_free_i();
    b = find_free_d();
    in[a].datablock_info[0] = b + 1;
    in[a].file_type = '-';
    set_time(&in[a]);
    in[a].file_size = size;
    len = strlen(db[num-1].data);
    strncpy(db[b].data, db[num-1].data, len);
    return a;
}

int copy2(int size, int num, int num2){ // 이외 파일 복사 함수
    int a, b, len, c;
    a = find_free_i();
    b = find_free_d();
    in[a].datablock_info[0] = b + 1;
    in[a].file_type = '-';
    set_time(&in[a]);
    in[a].file_size = size;
    
    len = strlen(db[num-1].data);
    strncpy(db[b].data, db[num-1].data, len);
    
    for(int i = num; i < num2; i++){
        c = find_free_d();
        len = strlen(db[num-1+i].data);
        strncpy(db[b].data, db[num-1+i].data, len);
    }
    in[a].datablock_info[1] = num2 + b - 1;
    return a;
}

void makedirect(char * filename, FILE * ifp, tree * file) //direct 생성
{
    int a, b;
    a = set_inode_info_f(filename);
    b = getblock(db, ifp);
    set_datablock_info(&(in[a].datablock_info[0]), b);
    
    if(now -> left == NULL){
        now -> left = file;
        now -> left -> right=NULL;
        now -> left -> left=NULL;
        now -> left -> inum = a+1;
    }
    else{
        next = now -> left;
        while(!(next -> right == NULL)){
            next = next -> right;}
        next -> right = file;
        next -> right -> left =NULL;
        next -> right -> right = NULL;
        next -> right -> inum = a+1;
    }
}

void makesingle(char * filename, int blocks, FILE * ifp, tree * file) // single indirect 생성
{
    int a, b, single_block, count = 0;
    int single_member[102] = {0};
    a = set_inode_info_f(filename);
    b = getblock(db, ifp);
    set_datablock_info(&(in[a].datablock_info[0]), b);
    for(int i = 0; i < blocks; i++){
        single_member[i] = getblock(db, ifp);
    }
    single_block = find_free_d();
    for(int i = 0; i < blocks; i++){
        if(count % 5 == 4){
            count += 1;
            i = i - 1;
        }
        get_single(db[single_block].data, count, single_member[i]);
        count++;
    }
    set_datablock_info(&(in[a].datablock_info[1]), single_block);
    if(now -> left == NULL){
        now -> left = file;
        now -> left -> right=NULL;
        now -> left -> left=NULL;
        now -> left -> inum = a+1;
    }
    else{
        next = now -> left;
        while(!(next -> right == NULL)){
            next = next -> right;}
        next -> right = file;
        next -> right -> left =NULL;
        next -> right -> right = NULL;
        next -> right -> inum = a+1;
    }
}

void makedouble(char * filename, int blocks, FILE * ifp,tree * file)
{  // double indirect 생성
    int a, b, single_block, double_block, count = 0, count2 = 0, j = 0;
    int single_member[102] = {0};
    int single_member2[102] = {0};
    int double_member[102] = {0};
    a = set_inode_info_f(filename);
    b = getblock(db, ifp);
    set_datablock_info(&(in[a].datablock_info[0]), b);
    for(int i = 0; i < 102; i++){
        single_member[i] = getblock(db, ifp);
    }
    single_block = find_free_d();
    for(int i = 0; i < 102; i++){
        if(count % 5 == 4){
            count += 1;
            i = i - 1;
        }
        get_single(db[single_block].data, count, single_member[i]);
        count++;
    }
    set_datablock_info(&(in[a].datablock_info[1]), single_block);
    for(int i = 0; i < (blocks - 102); i++){
        single_member2[i] = getblock(db, ifp);
        if(i % 102 == 0){
            single_block = find_free_d();
            for(int i = 0; i < 102; i++){
                if(count % 5 == 4){
                    count += 1;
                    i = i - 1;
                }
                get_single(db[single_block].data, count, single_member2[i]);
                single_member2[i] = 0;
                count++;
            }
            double_member[j] = single_block;
            j++;
        }
    }
    double_block = find_free_d();
    for(int i = 0; i < j; i++){
        if(count2 % 5 == 4){
            count2 += 1;
            i = i - 1;
        }
        get_double(db[double_block].data, count2, double_member[i]);
        count++;
    }
    set_datablock_info(&(in[a].datablock_info[2]), double_block);
    if(now -> left == NULL){
        now -> left = file;
        now -> left -> right=NULL;
        now -> left -> left=NULL;
        now -> left -> inum = a+1;
    }
    else{
        next = now -> left;
        while(!(next -> right == NULL)){
            next = next -> right;}
        next -> right = file;
        next -> right -> left =NULL;
        next -> right -> right = NULL;
        next -> right -> inum = a+1;
    }
}

void getroot(void){ // 루트 디렉터리 생성
    set_inode_info_d("/");
}

void myshowfile(char *tmp2, char *tmp3, char *filename) // myshowfile
{
    int argv_1, argv_2, c, inode_num = 0, db_num = 0;
    
    argv_1 = atoi(tmp2);
    argv_2 = atoi(tmp3);
    if(now->left==NULL)
        return;
    if(!strncmp(now->left->name,filename,4))
        inode_num = now->left->inum-1;
    else{
        next = now->left;
        while(next->right!=NULL){
            if(!strncmp(next->right->name,filename,4)){
                inode_num = next->right->inum-1;
                break;
            }
            next= next->right;
        }
    }
    db_num = in[inode_num].datablock_info[0];
    
    for(int i = argv_1; i <= argv_2; i++)
        printf("%c", db[db_num-1].data[i - 1]);
    putchar('\n');
}


void set_datablock_info(int *n, int tmp) // 데이터블록 정보(direct, single, double) 저장
{
    *n = tmp + 1;
}

void myshowblock(char *tmp2) // myshowblock
{
    int index = 0;
    index = atoi(tmp2);
    for(int i = 0; i < 128; i++)
        printf("%c", db[index - 1].data[i]);
    putchar('\n');
}

void myshowinode(char *tmp2) // myshowinode
{
    int index = 0;
    index = atoi(tmp2);
    show_inode_info(in[index - 1]);
}

void show_inode_info(inode in) // 아이노드 정보 보여주는 함수
{
    if(in.file_type == 'd')
        printf("file type : directory file\n");
    else if(in.file_type == '-')
        printf("file type : regular file\n");
    printf("file size : %d\n", in.file_size);
    printf("modified time : %d/%d/%d %d:%d:%d\n", in.file_creation_time[0], in.file_creation_time[1], in.file_creation_time[2], in.file_creation_time[3], in.file_creation_time[4], in.file_creation_time[5]);
    printf("data block list : %d, %d, %d\n", in.datablock_info[0], in.datablock_info[1], in.datablock_info[2]);
}

void mymkdir(char *tmp2) // mymkdir
{
    int num;
    tree * dir = NULL;
    dir = (tree *)malloc(sizeof(tree));
    num = set_inode_info_d(tmp2);
    strncpy(dir -> name,tmp2,4);
    if(now -> left == NULL){
        now -> left = dir;
        now -> left -> up = now;
        now -> left -> right=NULL;
        now -> left -> left=NULL;
        now -> left -> inum = num+1;
    }
    else{
        next = now -> left;
        while(!(next -> right == NULL)){
            next = next -> right;}
        next -> right = dir;
        next -> right -> up = now;
        next -> right -> left =NULL;
        next -> right -> right = NULL;
        next -> right -> inum = num+1;
    }
}

void get_directory(char * tmp2) // 디렉터리 할당
{
    set_inode_info_d(tmp2);
}


void mystate(void) // free inode, free data block 수 출력
{
    int free_inodes = 0, free_datablocks = 0;
    for(int i = 0; i < 16; i++){
        free_inodes += free_print(sb -> freeinode[i]);
    }
    for(int i = 0; i < 32; i++){
        free_datablocks += free_print(sb -> freedatablock[i]);
    }
    printf("free inode : %d\n", free_inodes);
    printf("free data block : %d\n", free_datablocks);
    
}

void classify(char * tmp,char *tmp1,char *tmp2,char *tmp3,char *tmp4)
{         //스캔한 문자열 인자들로 나누어 구분
    int i=0,j=0,num=0;
    while(tmp[i]!='\0'){
        if(tmp[i]==' '){
            num++;
            j=0;
            i++;
        }
        if(num==0){
            tmp1[j]=tmp[i];
            i++;
            j++;
        }
        if(num==1){
            tmp2[j]=tmp[i];
            i++;
            j++;
        }
        if(num==2){
            tmp3[j]=tmp[i];
            i++;
            j++;
        }
        if(num==3){
            tmp4[j]=tmp[i];
            i++;
            j++;
        }
    }
}

int set_inode_info_d(char * dirname) // 디렉터리의 아이노드 정보 저장
{
    int a, b, len;
    len = strlen(dirname);
    a = find_free_i();
    b = find_free_d();
    set_datablock_info(&(in[a].datablock_info[0]), b);
    in[a].file_type = 'd';
    set_time(&in[a]);
    in[a].file_size = 0;
    strncpy(db[b].data, dirname, len);
    return a;
}

void mytouch(char * tmp2){   //mytouch
    int num;
    num = set_inode_info_d("\0");
    in[num].file_type = '-';
    tree * file = NULL;
    file = (tree *)malloc(sizeof(tree));
    strncpy(file -> name,tmp2,4);
    if(now -> left == NULL){
        now -> left = file;
        now -> left -> up = now;
        now -> left -> right=NULL;
        now -> left -> left=NULL;
        now -> left -> inum = num+1;
    }
    else{
        next = now -> left;
        while(!(next -> right == NULL)){
            next = next -> right;}
        next -> right = file;
        next -> right -> up = NULL;
        next -> right -> left = NULL;
        next -> right -> right = NULL;
        next -> right -> inum = num+1;
    }
    
}

int set_inode_info_f(char * tmp2) // 파일의 아이노드 정보 저장
{
    int a;
    a = find_free_i();
    in[a].file_type = '-';
    set_time(&in[a]);
    set_size(&in[a], tmp2);
    return a;
}

void set_size(inode *in, char * sourcefile) // inode의 파일 사이즈 정보 저장
{
    FILE * ifp;
    int size;
    ifp = fopen(sourcefile, "rb");
    fseek(ifp, 0, SEEK_END);
    size = ftell(ifp);
    in -> file_size = size;
    return;
}


int getblock(datablock * db, FILE * ifp) // datablock 할당
{
    int a, b;
    b = find_free_d();
    db = db + b;
    getdata(db, ifp);
    return b;
}

void getdata(datablock *db, FILE * ifp) // datablock에 데이터 저장
{
    int c;
    if((c = fread(&db -> data, 128, 1, ifp)) == EOF){
        rewind(ifp);
        return;
    }
}

void freeinode(inode * in) // 아이노드 정보 제거
{
    in -> file_size = 0;
    for(int i = 0; i < 6; i++){
        in -> file_creation_time[i] = 0;
    }
    for(int i = 0; i < 3; i++){
        in -> datablock_info[i] = 0;
    }
    in -> file_type = '\0';
}

void freedatablock(datablock * db) // 데이터블록 정보 제거
{
    int len;
    len = strlen(db -> data);
    for(int i = 0; i < len; i++){
        db -> data[i] = '\0';
    }
}
void set_time(inode * in) // 아이노드에 시간 저장
{
    struct tm *t;
    time_t now;
    now = time(NULL);
    t = localtime(&now);
    
    in -> file_creation_time[0] = t -> tm_year + 1900;
    in -> file_creation_time[1] = t -> tm_mon + 1;
    in -> file_creation_time[2] = t -> tm_mday;
    in -> file_creation_time[3] = t -> tm_hour;
    in -> file_creation_time[4] = t -> tm_min;
    in -> file_creation_time[5] = t -> tm_sec;
}

void freesuperblock_i(int n, superblock *sb) // 슈퍼블록에 저장된 사용 정보 제거
{
    int div = 0, mod = 0;
    div = n / 32;
    mod = n % 32;
    freebit(&(sb -> freeinode[div]), mod);
}
void freesuperblock_d(int n, superblock *sb) // 슈퍼블록에 저장된 사용 정보 제거
{
    int div = 0, mod = 0;
    div = n / 32;
    mod = n % 32;
    freebit(&(sb -> freedatablock[div]), mod);
}

void set_superblock_d(unsigned *a){ // 슈퍼블록에 사용 정보 저장
    int tmp, sum = 0;
    static int n = 31;
    tmp = 1 << n;
    *a |= tmp;
    --n;
    if(n < 0)
        n = 31;
}
void set_superblock_i(unsigned *a){ // 슈퍼블록에 사용 정보 저장
    int tmp, sum = 0;
    static int n = 31;
    tmp = 1 << n;
    *a |= tmp;
    --n;
    if(n < 0)
        n = 31;
}

int find_free_superblock(unsigned a) // 가용 아이노드, 데이터블록 검색용 함수
{
    int i, flag = 0;
    int n = sizeof(unsigned) * 8;
    int mask = 1 << (n-1);
    
    for(i = 1; i <= n; ++i){
        if((a & mask) == 0){
            return flag;
        }
        else{
            a <<= 1;
            flag++;
        }
    }
}

void freebit(unsigned *a, int mod){ // 비트 1에서 0으로 바꾸는 함수(파일 제거할 시 사용)
    int i, tmp;
    int n = sizeof(unsigned) * 8;
    int del = 32 - mod;
    tmp = 1 << del;
    *a ^= tmp;
}

int free_print(unsigned a){ // mystate 정보 계산용 함수
    int i, free = 0;
    bool check = false;
    int n = sizeof(unsigned) * 8;
    int mask = 1 << (n-1);
    for(i = 1; i <= n; ++i){
        if((a & mask) == 0){
            free++;
        }
        a <<= 1;
    }
    return free;
}
void get_single(char * a, int n, int num){
    // single indirect block에 datablock의 number를 저장
    if(n%5 == 0){
        for(int i = 0; i < 8; i++)
            if(num&1<<(i+2))
                a[n]|=1<<i;
        for(int i = 6; i < 8; i++)
            if(num&1<<(7-i))
                a[n+1] |= 1<<(13-i);
        
    }
    else if(n%5 == 1){
        for(int i=0; i<6; i++)
            if(num&1<<(i+3))
                a[n]|= 1<<i;
        for(int i= 4; i<8; i++)
            if(num&1<<(i-4))
                a[n+1]|= 1<<i;
    }
    else if(n%5 == 2){
        for(int i= 0; i<4; i++)
            if(num&1<<(i+3))
                a[n]|= 1<<i;
        for(int i=2; i<8; i++)
            if(num&1<<(i-2))
                a[n+1]|= 1<<i;
    }
    else if(n%5 == 3){
        for(int i=0;i<2;i++)
            if(num&1<<(i+8))
                a[n]|= 1<<i;
        for(int i = 0; i <8; i++)
            if(num&1<<(i))
                a[n+1]|= 1<<i;
    }
}

void get_double(char * a, int n, int num){
    // double indirect에 single indirect block number들 저장
    if(n%5 == 0){
        for(int i = 0; i < 8; i++)
            if(num&1<<(i+2))
                a[n]|=1<<i;
        for(int i = 6; i < 8; i++)
            if(num&1<<(7-i))
                a[n+1] |= 1<<(13-i);
        
    }
    else if(n%5 == 1){
        for(int i=0; i<6; i++)
            if(num&1<<(i+3))
                a[n]|= 1<<i;
        for(int i= 4; i<8; i++)
            if(num&1<<(i-4))
                a[n+1]|= 1<<i;
    }
    else if(n%5 == 2){
        for(int i= 0; i<4; i++)
            if(num&1<<(i+3))
                a[n]|= 1<<i;
        for(int i=2; i<8; i++)
            if(num&1<<(i-2))
                a[n+1]|= 1<<i;
    }
    else if(n%5 == 3){
        for(int i=0;i<2;i++)
            if(num&1<<(i+8))
                a[n]|= 1<<i;
        for(int i = 0; i <8; i++)
            if(num&1<<(i))
                a[n+1]|= 1<<i;
    }
}

void mycat(char * filename) // mycat
{
    int inode_num, tmp, tmp2, tmp3, len;
    if(now->left==NULL)
        return;
    if(!strncmp(now->left->name,filename,4))
        inode_num = now->left->inum-1;
    else{
        next = now->left;
        while(next->right!=NULL){
            if(!strncmp(next->right->name,filename,4)){
                inode_num = next->right->inum-1;
                break;
            }
            next= next->right;
        }
    }
    
    tmp = in[inode_num].datablock_info[0];
    tmp2 = in[inode_num].datablock_info[1];
    tmp3 = in[inode_num].datablock_info[2];
    
    if(tmp2 == 0 && tmp3 == 0){
        len = strlen(db[tmp-1].data);
        for(int i = 0; i < len; i++){
            printf("%c", db[tmp-1].data[i]);
        }
    }
    else if(tmp2 != 0 && tmp3 == 0){
        for(int j = tmp; j < tmp2-1; j++){
            len = strlen(db[tmp-1+j].data);
            for(int i = 0; i < len; i++){
                printf("%c", db[tmp-1+j].data[i]);
            }
        }
    }
    
    else{
        for(int j = tmp; j < tmp3 - 1; j++){
            if(j == tmp2)
                ++j;
            len = strlen(db[tmp-1+j].data);
            for(int i = 0; i < len; i++){
                printf("%c", db[tmp-1+j].data[i]);
            }
        }
    }
}
int bit_print(char * a, int n){
    char * c = (char *) a;
    int bit = c[n/8]&1<<(7-(n%8));
    return bit;
}

int num_bit(char * a, int n){
    int num=0,count=0;
    
    for(int i=(n-1)*10;i<(n-1)*10+10;i++,count++)
        if(bit_print(a,i))
            num |= 1 << (9-count);
    
    return num;
}
void init(){  //트리 시작
    tree * start = NULL;
    start = (tree *)malloc(sizeof(tree));
    root = start;
    strcpy(root->name,"/");
    root -> left = NULL;
    root -> inum = 1;
    now = root;
    root -> up = root;
}
void mypwd(){ // mypwd
    if(now==root){
        putchar('/');
    }
    else{
        printf("/%s\n",now->name);
    }
}
void mycd(char * a, int num){ // mycd
    int i=1;
    
    if(!num){
        now = root;
        return;
    }
    
    else if(num){
        if(!strncmp(a,".",1))
            return;
        
        else if(!strncmp(a,"..",2))
            now = now->up;
        
        else{
            if(now->left==NULL){
                printf("비어있습니다.\n");
                return;
            }
            if(!strncmp(now->left->name,a,4)){
                if(in[now->left->inum-1].file_type=='-'){
                    printf("Not directory\n");
                    return;
                }
                now = now->left;
            }
            else{
                next = now->left;
                while(i){
                    if(next->right==NULL){
                        printf("없는 디렉터리입니다.\n");
                        return;
                    }
                    if(!strncmp(next->right->name,a,4)){
                        if(in[next->right->inum-1].file_type=='-'){
                            printf("Not directory\n");
                            return;
                        }
                        i--;
                        now = next -> right;
                    }
                    next = next -> right;
                }
            }
        }
    }
}
void myls(char *a,int op){ // myls
    char name[100][5],tmp[5];
    int num,i=0,inm[100]={0},itmp;
    work = now;
    
    if(now->left!=NULL){
        strncpy(name[i],now->left->name, 4);
        inm[i]=now->left->inum;
        i++;
        next = now->left;
        
        while(!(next->right==NULL)){
            strncpy(name[i],next->right->name, 4);
            inm[i]=next->right->inum;
            i++;
            next = next->right;
        }
    }
    
    num = i;
    for(int j=0;j<num;j++){
        for(int k = j+1; k<num;k++){
            if(strncmp(name[j],name[k], 4)>0)
            {
                strncpy(tmp,name[j], 4);
                itmp = inm[j];
                strncpy(name[j],name[k], 4);
                inm[j] = inm[k];
                strncpy(name[k],tmp, 4);
                inm[k] = itmp;
            }
        }
    }
    if(!op){
        printf(".\n");
        printf("..\n");
        for(int a=0;a<num;a++)
            printf("%s\n",name[a]);
    }
    if(op==1){
        printf("%d .\n",now->inum);
        printf("%d ..\n",now->up->inum);
        for(int a=0;a<num;a++)
            printf("%d %s\n",inm[a],name[a]);
    }
    if(op==2){
        printf("%c %4d %d/%d/%d %d:%d:%d .\n",in[((now->inum)-1)].file_type,in[((now->inum)-1)].file_size,in[((now->inum)-1)].file_creation_time[0],in[((now->inum)-1)].file_creation_time[1],in[((now->inum)-1)].file_creation_time[2],in[((now->inum)-1)].file_creation_time[3],in[((now->inum)-1)].file_creation_time[4],in[((now->inum)-1)].file_creation_time[5]);
        printf("%c %4d %d/%d/%d %d:%d:%d ..\n",in[((now->up->inum)-1)].file_type,in[((now->up->inum)-1)].file_size,in[((now->up->inum)-1)].file_creation_time[0],in[((now->up->inum)-1)].file_creation_time[1],in[((now->up->inum)-1)].file_creation_time[2],in[((now->up->inum)-1)].file_creation_time[3],in[((now->up->inum)-1)].file_creation_time[4],in[((now->up->inum)-1)].file_creation_time[5]);
        for(int a=0;a<num;a++)
            printf("%c %4d %d/%d/%d %d:%d:%d %s\n",in[(inm[a]-1)].file_type,in[(inm[a]-1)].file_size,in[(inm[a]-1)].file_creation_time[0],in[(inm[a]-1)].file_creation_time[1],in[(inm[a]-1)].file_creation_time[2],in[(inm[a]-1)].file_creation_time[3],in[(inm[a]-1)].file_creation_time[4],in[(inm[a]-1)].file_creation_time[5],name[a]);
    }
    if(op==3){
        printf("%d %c %4d %d/%d/%d %d:%d:%d .\n",now->inum,in[((now->inum)-1)].file_type,in[((now->inum)-1)].file_size,in[((now->inum)-1)].file_creation_time[0],in[((now->inum)-1)].file_creation_time[1],in[((now->inum)-1)].file_creation_time[2],in[((now->inum)-1)].file_creation_time[3],in[((now->inum)-1)].file_creation_time[4],in[((now->inum)-1)].file_creation_time[5]);
        printf("%d %c %4d %d/%d/%d %d:%d:%d ..\n",now->up->inum,in[((now->up->inum)-1)].file_type,in[((now->up->inum)-1)].file_size,in[((now->up->inum)-1)].file_creation_time[0],in[((now->up->inum)-1)].file_creation_time[1],in[((now->up->inum)-1)].file_creation_time[2],in[((now->up->inum)-1)].file_creation_time[3],in[((now->up->inum)-1)].file_creation_time[4],in[((now->up->inum)-1)].file_creation_time[5]);
        for(int a=0;a<num;a++)
            printf("%d %c %4d %d/%d/%d %d:%d:%d %s\n",inm[a],in[(inm[a]-1)].file_type,in[(inm[a]-1)].file_size,in[(inm[a]-1)].file_creation_time[0],in[(inm[a]-1)].file_creation_time[1],in[(inm[a]-1)].file_creation_time[2],in[(inm[a]-1)].file_creation_time[3],in[(inm[a]-1)].file_creation_time[4],in[(inm[a]-1)].file_creation_time[5],name[a]);
    }
    now = work;
}
void myrmdir(char * a){ // myrmdir
    tree * pre;
    tree * con;
    int inode_num = 0, datablock_num = 0, tmp = 0, tmp2 = 0;
    
    if(!strncmp(now->left->name,a, 4)){
        inode_num = (now->left->inum)-1;
        if(in[inode_num].file_type=='-')
            return;
        if(now->left->left!=NULL){
            return;
        }
        if(now->left->right==NULL){
            free(now->left);
            now->left=NULL;
        }
        else{
            pre = now;
            con = now->left->right;
            free(now->left);
            pre->left=con;
        }
    }
    else{
        next = now->left;
        pre = next;
        while(!(next->right==NULL)){
            if(!strncmp(next->right->name,a, 4)){
                inode_num = (next -> right -> inum)-1;
                if(in[inode_num].file_type=='-')
                    return;
                if(next->right->left!=NULL)
                    return;
                if(next->right->right==NULL){
                    free(next->right);
                    next->right=NULL;
                }
                else{
                    con = next->right->right;
                    free(next->right);
                    pre->right =con;
                }
                break;
            }
            pre = next;
            next = next->right;
        }
    }
    datablock_num = in[inode_num].datablock_info[0];
    freesuperblock_i(inode_num + 1, sb);
    freesuperblock_d(datablock_num, sb);
    freedatablock(&(db[datablock_num - 1]));
    freeinode(&(in[inode_num]));
}
void myrm(char * a){ // myrm
    tree * pre;
    tree * con;
    int inode_num = 0, datablock_num = 0, single_db_num = 0, double_db_num = 0, last_db_num = 0;
    
    if(!strncmp(now->left->name,a, 4)){
        inode_num = (now->left->inum)-1;
        if(in[inode_num].file_type=='d')
            return;
        if(now->left->left!=NULL){
            return;
        }
        if(now->left->right==NULL){
            free(now->left);
            now->left=NULL;
        }
        else{
            pre = now;
            con = now->left->right;
            free(now->left);
            pre->left=con;
        }
    }
    else{
        next = now->left;
        pre = next;
        while(!(next->right==NULL)){
            if(!strncmp(next->right->name,a, 4)){
                inode_num = (next -> right -> inum)-1;
                if(in[inode_num].file_type=='d')
                    return;
                if(next->right->left!=NULL)
                    return;
                if(next->right->right==NULL){
                    free(next->right);
                    next->right=NULL;
                }
                else{
                    con = next->right->right;
                    free(next->right);
                    pre->right =con;
                }
                break;
            }
            pre = next;
            next = next->right;
        }
    }
    datablock_num = in[inode_num].datablock_info[0];
    single_db_num = in[inode_num].datablock_info[1];
    double_db_num = in[inode_num].datablock_info[2];
    if(single_db_num != 0 && double_db_num == 0)
        last_db_num = single_db_num;
    else if(double_db_num != 0)
        last_db_num = double_db_num;
    else
        last_db_num = datablock_num;
    freesuperblock_i(inode_num, sb);
    for(int i = 0; i < last_db_num; i++)
        freesuperblock_d(datablock_num + i, sb);
    for(int i = 0; i < last_db_num; i++)
        freedatablock(&(db[(datablock_num - 1) + i]));
    freeinode(&(in[inode_num]));
}


void mytree(char * a){ // mytree
    int level[512]={0},j=1,count=0,ncheck[512]={0},go,check2=0;
    tree *check[512];
    tree * next = NULL;
    tree * save = NULL;
    tree * turn = NULL;
    for(int i=0;i<512;i++)
        level[i]=i;
    
    if(a[0]=='\0'){
        printf("%s\n",now->name);
        if(now->left!=NULL){
            
            for(int i=0;i<3*level[j-1]+2;i++)
                printf("-");
            printf("* ");
            printf("%s\n",now->left->name);
            next = now -> left;
            
            if(next->right!=NULL){
                save = next;
                check[count] = save;
                ncheck[count]=j;
                count++;
            } // 첫번째 이동
            
            while(next->left!=NULL){
                j++;
                for(int i=0;i<3*level[j-1]+2;i++)
                    printf("-");
                printf("* ");
                printf("%s\n",next->left->name);
                next = next->left;
                
                if(next->right!=NULL){
                    save = next;
                    check[count] = save;
                    ncheck[count]=j;
                    count++;
                }
            } //맨 왼쪽 끝
            
            turn = next;
            while(turn->right!=NULL){
                for(int i=0;i<3*level[j-1]+2;i++)
                    printf("-");
                printf("* ");
                printf("%s\n",next->right->name);
                next = next-> right;
                count--;
                check[count]=NULL;
                ncheck[count]=0;
                
                if(next->left!=NULL){
                    if(next->right!=NULL){
                        save = next;
                        check[count] = save;
                        ncheck[count]=j;
                        count++;
                    }
                    while(next->left!=NULL){
                        j++;
                        for(int i=0;i<3*level[j-1]+2;i++)
                            printf("-");
                        printf("* ");
                        printf("%s\n",next->left->name);
                        next = next->left;
                        
                        if(next->right!=NULL){
                            save = next;
                            check[count] = save;
                            ncheck[count]=j;
                            count++;
                        }
                    }
                }
                turn = next;
            }
            while(ncheck[0]){
				if(check2)
					break;
                for(int a = 511;a>=0;--a){
                    if(ncheck[a]){
						if(a==0)
							check2++;
                        j = ncheck[a];
                        turn = check[a];
                        count--;
                        check[a]=NULL;
                        ncheck[a]=0;
                        break;
                    }
                }
                while(turn->right!=NULL){
                    next = turn;
                    for(int i=0;i<3*level[j-1]+2;i++)
                        printf("-");
                    printf("* ");
                    printf("%s\n",next->right->name);
                    next = next-> right;
                    
                    if(next->left!=NULL){
                        if(next->right!=NULL){
                            save = next;
                            check[count] = save;
                            ncheck[count]=j;
                            count++;
                        }
                        
                        while(next->left!=NULL){
                            j++;
                            for(int i=0;i<3*level[j-1]+2;i++)
                                printf("-");
                            printf("* ");
                            printf("%s\n",next->left->name);
                            next = next->left;
                            
                            if(next->right!=NULL){
                                save = next;
                                check[count] = save;
                                ncheck[count]=j;
                                count++;
                            }
                        }
                    }
                    turn = next;
                }
            }
        }
    }
}
void mymv(char * a, char * b){ // mymv
    tree * pre=NULL;
    tree * con=NULL;
    tree * sou=NULL;
    tree * dir=NULL;
    int check = 0;
    
    if(now->left != NULL){
        next = now->left;
        
        if(!strncmp(a,next->name,4)&&in[(next->inum)-1].file_type=='-'){
            pre = now;
            if(next->right!=NULL)
                con = next->right;
            sou = next;
        }
        
        if(!strncmp(b,next->name,4)&&in[(next->inum)-1].file_type=='d')
            dir = next;
        
        while(next->right!=NULL){
            if(!strncmp(b,next->right->name,4)&&in[(next->right->inum)-1].file_type=='d')
                dir = next->right;
            if(!strncmp(a,next->right->name,4)&&in[(next->right->inum)-1].file_type=='-'){
                check++;
                pre = next;
                if(next->right->right!=NULL)
                    con = next ->right->right;
                sou = next->right;
            }
            next = next->right;
        }
    }
    
    if(dir!=NULL&&sou!=NULL){
        if(dir->left==NULL){
            dir->left = sou;
            if(!check){
                if(con!=NULL)
                    pre->left = con;
                else
                    pre->left=NULL;
            }
            else{
                if(con!=NULL)
                    pre->right = con;
                else
                    pre->right =NULL;
            }
        }
        else{
            next = dir->left;
            while(next->right!=NULL)
                next = next->right;
            next -> right = sou;
            if(!check){
                if(con!=NULL)
                    pre->left = con;
            }
            else{
                if(con!=NULL)
                    pre->right = con;
            }
        }
    }
    
    if(dir==NULL){
        if(!strncmp(a,now->left->name,4)&&in[(now->left->inum)-1].file_type=='-')
            strncpy(now->left->name,b,4);
        else{
            next = now->left;
            while(next->right!=NULL){
                if(!strncmp(next->right->name,a,4)&&in[(next->right->inum)-1].file_type=='-'){
                    strncpy(next->right->name,b,4);
                    break;
                }
                next = next -> right;
            }
        }
    }
}
void myfs_shell(){ //쉘 함수
    char *dir;
    char *tmp, *tmp1, *tmp2, *tmp3, *tmp4;
    int len, my=0,check=0, len2 = 0, len3;
    bool check_fs = false, check2 = true;
    FILE * point;
    
    while(1){
        tmp = (char *)calloc(25, sizeof(char));
        tmp1 = (char *)calloc(25, sizeof(char));
        tmp2 = (char *)calloc(25, sizeof(char));
        tmp3 = (char *)calloc(25, sizeof(char));
        tmp4 = (char *)calloc(25, sizeof(char));
        dir = (char *)calloc(10, sizeof(char));
        if(now==root){
            strncpy(dir,"/",1);
        }
        else{
            dir[0] = '/';
            len3 = strlen(now -> name);
            for(int i = 1; i < len3+1; i++){
                dir[i] = (now -> name[i-1]);
            }
            dir[len3+1] = '/';
        }
        printf("[%s ]$ : ",dir);
        if(check2){
            printf("파일 시스템 생성 : ");
            check2 = false;
        }
        scanf("%[^\n]",tmp);
        getchar();
        classify(tmp,tmp1,tmp2,tmp3,tmp4);
        if(!strncmp(tmp,"my",2))
            my++;
        if(!strcmp(tmp,"byebye")){
            filesave();
            free(tmp);
            free(tmp1);
            free(tmp2);
            free(tmp3);
            free(tmp4);
            free(dir);
            exit(1);
        }
        if(!my)
            system(tmp);
        if(my){
            if(!strncmp(tmp1, "mymkfs", 6)){
                if(check_fs){
                    printf("error : myfs exists\n");
                    continue;
                }
                if((point = fopen("myfs","rb+"))){
                    fopen("myfs", "rb+");
                    fileload();
                }
                init();
                getroot();
                check_fs = 1;
            }
            if(!strncmp(tmp1,"myls",4)){
                
                if((!strncmp(tmp2, "-li", 3))||(!strncmp(tmp2, "-il", 3))){
                    myls(tmp3,3);
                }
                
                else if(!strncmp(tmp2,"-i",2)){
                    myls(tmp3,1);
                }
                
                else if(!strncmp(tmp2,"-l",2)){
                    myls(tmp3,2);
                }
                else
                    myls(tmp2,0);
            }
            
            else if(!strncmp(tmp1,"mycat",5))
                mycat(tmp2);
            
            else if(!strncmp(tmp1,"myshowfile",10)){
                myshowfile(tmp2, tmp3, tmp4);
            }
            
            else if(!strncmp(tmp1,"mypwd",5)){
                mypwd();
            }
            
            else if(!strncmp(tmp1,"mycd",4)){
                if(!(tmp[4]=='\0'))
                    check++;
                mycd(tmp2,check);
                check=0;
            }
            
            else if(!strncmp(tmp1,"mycp",4) && tmp[4] == ' ')
                mycp(tmp2, tmp3);
            
            else if(!strncmp(tmp1,"mycpto",6))
                mycpto(tmp2, tmp3);
            
            else if(!strncmp(tmp1,"mycpfrom",8)){
                mycpfrom(tmp2, tmp3);
            }
            else if(!strncmp(tmp1,"mymkdir",7)){
                mymkdir(tmp2);
            }
            else if(!strncmp(tmp1, "myrm", 4)){
                len = strlen(tmp1);
                if(len == 4)
                    myrm(tmp2);
                else if(len == 7)
                    myrmdir(tmp2);
            }
            
            
            else if(!strncmp(tmp1,"mymv",4))
                mymv(tmp2,tmp3);
            
            else if(!strncmp(tmp1,"mytouch",7))
                mytouch(tmp2);
            else if(!strncmp(tmp1,"myshowinode",11)){
                myshowinode(tmp2);
            }
            
            else if(!strncmp(tmp1,"myshowblock",11)){
                myshowblock(tmp2);
            }
            
            else if(!strncmp(tmp1,"mystate",7)){
                mystate();
            }
            else if(!strncmp(tmp1,"mytree",6))
                mytree(tmp2);
            
            my--;
        }
        
    }
}
void filesave(){ // 파일 저장
    FILE * ifp;
    ifp = fopen("myfs","wb");
    fseek(ifp,2,SEEK_SET);
    fwrite(sb,sizeof(superblock),1,ifp);
    fwrite(in,sizeof(inode),512,ifp);
    fwrite(db,sizeof(datablock),1024,ifp);
    fclose(ifp);
}
void fileload(){ // 파일 불러오기
    FILE * ifp;
    ifp = fopen("myfs","rb");
    fseek(ifp,2,SEEK_SET);
    fread(sb,sizeof(superblock),1,ifp);
    fread(in,sizeof(inode),512,ifp);
    fread(db,sizeof(datablock),1024,ifp);
    fclose(ifp);
}

int find_free_d() // 가용 데이터블록 검색
{
    int tmp, val = 0;
    static int j;
    while(1){
        if(j == 32){
            j = 0;
            break;
        }
        tmp = find_free_superblock(sb -> freedatablock[j]);
        set_superblock_d(&(sb -> freedatablock[j]));
        val = tmp + (32 * j);
        if(tmp == 31)
            j++;
        return val;
    }
}

int find_free_i() // 가용 아이노드 검색
{
    int tmp, val = 0;
    static int k;
    while(1){
        if(k == 32){
            k = 0;
            break;
        }
        tmp = find_free_superblock(sb -> freeinode[k]);
        set_superblock_i(&(sb -> freeinode[k]));
        val = tmp + (32 * k);
        if(tmp == 31)
            k++;
        return val;
    }
}
