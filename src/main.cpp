#include <iostream>
using namespace std;
class Connector;
const int LINE_BUF_SIZE=2048;
#define _OnlineJudge_
#ifdef _OnlineJudge_
class Connector{
private:
    char buf[LINE_BUF_SIZE];
public:
    Connector()=default;
    const char* getLine(int& id){
        cin.getline(buf,LINE_BUF_SIZE);
        return buf;
    }
    void writeUp(const char* str,int id){
        cout<<str<<endl;
    }
};
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
class Connector{
private:
    static const int LOCAL_PORT=7817;
    static const int QUEUE_LEN=30;
    int listen_fd;
    char buf[LINE_BUF_SIZE];
public:
    /*
     * use IPv4 and TCP for compatibility
     */
    Connector(){
        int err;
        listen_fd=socket(AF_INET,SOCK_STREAM,0);
        sockaddr_in local_addr;
        local_addr.sin_family=AF_INET;
        local_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        local_addr.sin_port=htons(LOCAL_PORT);
        if((err=bind(listen_fd,(sockaddr*)&local_addr,sizeof(local_addr)))!=0)throw std::runtime_error("bind error "+to_string(err));
        if((err=listen(listen_fd,QUEUE_LEN))!=0)throw std::runtime_error("listen error "+to_string(err));
    }
    ~Connector(){
        close(listen_fd);
    }

    const char* getLine(int& id){
        int size;
        sockaddr_in client_addr;
        socklen_t client_len=sizeof(client_addr);
        id=accept(listen_fd,(sockaddr*)&client_addr,&client_len);
        if((size=read(id, buf, LINE_BUF_SIZE)) < 0)throw std::runtime_error("read error "+to_string(size));
        buf[size]='\0';
        return buf;
    }

    void writeUp(const char* str,int fd){
        write(fd,str,sizeof(str));
        close(fd);
    }
};
#endif
int main(){
    Connector client;
    // todo use Connector to interact with OJ or python server
    return 0;
}
