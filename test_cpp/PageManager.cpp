#include "../basic_component/PageManager.h"
#include <iostream>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <random>
#include <unordered_map>
#include <fstream>

using namespace std;
using std::ios;
const char str[]="!@#$%^&*()-+=ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz 0123456789!@#$%^&*()-+=ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_`abcdefghijklmnopqrstuvwxyz 0123456789";
constexpr int M=(sizeof(str)-1)/2;
/*
struct Data{
    int32_t size;
    char* dat;
    Data(int c,const char* origin):size(c){
        dat=new char[c];
        if(origin){
            if(strlen(origin)+1!=c)
                throw std::logic_error("Fault");
            strcpy(dat,origin);
        } else{
            int offset=rand()%M;
            memcpy(dat,str+offset,c-1);
            dat[c]='\0';
        }
    }
    ~Data(){delete [] dat;}
};
 */
const int BUF_SIZE=1048576*20;
char buf1[BUF_SIZE],buf2[BUF_SIZE];
void random_fill(char* buf,int n){
    for(int i=0;i<n;++i){
        buf[i]=str[rand()%M];
    }
}
void read_test(int s,int e,t_sys::PageManager& pm,fstream& cmper){
    if(s>e)swap(s,e);
    pm.read(buf1,s,e-s+1);
    cmper.seekg(s,ios::beg);
    cmper.read(buf2,e-s+1);

    for(int i=0;i<=e-s;++i){
        if(buf1[i]!=buf2[i]){
            int r=3;
            r--;
        }
    }
}
void write_test(int s,int e,t_sys::PageManager& pm,fstream& cmper){
    if(s>e)swap(s,e);
    random_fill(buf1,e-s+1);
    cmper.seekp(s,ios::beg);
    cmper.clear();
    cmper.write(buf1,e-s+1).flush();
    pm.write(buf1,s,e-s+1);
    if(!cmper.good()){
        int r=1;
        r--;
    }
}
int main(){
    srand(1926081712);
    system("cp ../test/num.dat ../test/data.cmp");
    system("cp ../test/num.dat ../test/data.ope");

    t_sys::PageManager pm("../test/data.ope");
    fstream cmper("../test/data.cmp",ios::binary|ios::in|ios::out);
    cmper.seekg(0,ios::end);
    const int file_size=cmper.tellg();
    cmper.seekg(0,ios::beg);
    //begin test
    for(int i=0;i<100;++i){
        if(i%100==0)cout<<i<<endl;
        int s=rand()%file_size,e=rand()%file_size;
        read_test(s,e,pm,cmper);
    }
//    *
    for(int i=0;i<100;++i) {
        if (i%100 == 0)cout << i << endl;
        int s = rand()%file_size, e = rand()%file_size;
        write_test(s, e, pm, cmper);
    }

//    int max_size=0;
//    for(int i=0;i<100;++i){
//        if(i%100==0)cout<<i<<endl;
//        int s=BUF_SIZE+rand()%BUF_SIZE,e=BUF_SIZE+rand()%BUF_SIZE;
//        if(s>e)swap(s,e);
//        max_size=max(max_size,e+1);
//        write_test(s,e,pm,cmper);
//    }
//     *
    write_test(file_size,BUF_SIZE,pm,cmper);
    for(int i=0;i<100;++i){
        if(i%100==0)cout<<i<<endl;
        int s=rand()%BUF_SIZE,e=rand()%BUF_SIZE;
        read_test(s,e,pm,cmper);
    }
    for(int i=0;i<100;++i){
        if(i%100==0)cout<<i<<endl;
        int s=rand()%BUF_SIZE,e=rand()%BUF_SIZE;
        write_test(s,e,pm,cmper);
    }
//    *
    for(int i=0;i<100;++i){
        if(i%100==0)cout<<i<<endl;
        int s=rand()%BUF_SIZE,e=rand()%BUF_SIZE;
        read_test(s,e,pm,cmper);
    }

    cmper.flush();
    cmper.close();
//     */
}