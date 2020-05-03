#include <bits/stdc++.h>
#include "../src/structure.h"
#include "../src/TrainManager.h"
using namespace std;
using namespace t_sys;
int main()
{
    TrainManager a("233","2333",true);
    char** s;
    s=new char* [3];
    s[0]=new char [2];
    s[1]=new char [2];
    s[2]=new char [2];
    s[0][0]='A';s[0][1]='\0';
    s[1][0]='B';s[1][1]='\0';
    s[2][0]='Z';s[2][1]='\0';
    int *b,*c,*d;
    b=new int [2];
    b[0]=100;b[1]=200;
    c=new int [2];
    c[0]=600;c[1]=8000;
    d=new int [1];
    d[0]=20;
    a.Add_train(trainID_t("G1515"),3,10,s,b,1200,c,d,6010801,'G');
    //a.Release_train(trainID_t("G1515"));
    a.Delete_train(trainID_t("G1515"));
    a.Add_train(trainID_t("G1515"),3,10,s,b,1200,c,d,6010801,'G');
   // a.Add_train(trainID_t("G1515"),3,10,s,b,1200,c,d,6010801,'G');
    a.Release_train(trainID_t("G1515"));
    a.Query_train(trainID_t("G1515"),731);
}