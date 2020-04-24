//
// Created by vortox on 10/4/20.
//

#ifndef BPTREE_FILE_ALTERNATIVE_H
#define BPTREE_FILE_ALTERNATIVE_H

#include <cstdio>
namespace ds{
    class File{
        /*
         *  OOP iostream alternative
         *  in order to use *write* instead of *writev* on linux platform
         */
    private:
        FILE* file;
    public:
        void seekg(size_t n){fseek(file,n,SEEK_SET);}
        void seekp(size_t n){fseek(file,n,SEEK_SET);}
        void read(char* ptr,size_t n){fread(ptr, sizeof(char),n,file);}
        void write(const char* ptr,size_t n){fwrite(ptr, sizeof(char),n,file);}
        void flush(){fflush(file);}
        void close(){fclose(file);}
        bool fail()const { return false;}
        void open(const char* str){
            file=fopen(str,"r+b");
            setbuf(file, nullptr);
        }
    };
}
#endif //BPTREE_FILE_ALTERNATIVE_H
