//
// Created by vortox on 2/6/20.
//

#ifndef TICKETSYSTEM_SERIALIZATIONHELPER_H
#define TICKETSYSTEM_SERIALIZATIONHELPER_H
#include "../include/vector.hpp"
#include "../include/unordered_map.h"
#include "../src/global.h"
#include <cstring>
#include <fstream>
using std::ios;
using t_sys::DiskLoc_T;
using t_sys::DISKLOC_MAX;
namespace ds{
    template<typename T>
    class SerialVector {
    public:
        static void serialize(const ds::vector<T>& vec, const std::string& path) {
            t_sys::DiskLoc_T size = vec.size()*sizeof(T)+sizeof(DiskLoc_T);
            char* buf = (char*) malloc(size);
            memcpy(buf, &size, sizeof(DiskLoc_T));
            char* ptr = buf+sizeof(DiskLoc_T);
            for (int i=0;i<vec.size();++i,ptr+=sizeof(T))
                memcpy(ptr, (void*) &vec[i], sizeof(T));
            std::ofstream ofs(path, ios::binary);
            ofs.seekp(0);
            ofs.write(buf, size);
            ofs.close();
            free(buf);
        }
        static void deserialize(ds::vector<T>& vec, const std::string& path) {
            std::ifstream ifs(path, ios::binary);
            DiskLoc_T size;
            ifs.seekg(0);
            ifs.read((char*) &size, sizeof(size));
            size -= sizeof(DiskLoc_T);
            if (size == 0) {
                ifs.close();
                return;
            }
            char* buf = (char*) malloc(size);
            char* ptr = buf;
            ifs.read(buf, size);
            ifs.close();
            for (int i = 0, E = size/sizeof(T); i < E; ++i, ptr += sizeof(T)) {
                vec.push_back(*((T*) ptr));
            }
            free(buf);
        }
        static void Init(const std::string& path){t_sys::init_subprocess(path);}
    };

    template<typename K,typename V>
    class SerialMap{
    public:
        static void serialize(const ds::unordered_map<K,V>& map,const std::string& path){
            DiskLoc_T size = map.size()*(sizeof(K)+sizeof(V))+sizeof(DiskLoc_T);
            char* buf = (char*) malloc(size);
            memcpy(buf, &size, sizeof(DiskLoc_T));
            char* ptr = buf+sizeof(DiskLoc_T);
            for(int i=0;i<map.__bucket_count;++i){
                for(auto* node_ptr=map.buckets[i].head;node_ptr;node_ptr=node_ptr->next){
                    memcpy(ptr,&(node_ptr->val.first),sizeof(K));ptr+=sizeof(K);
                    memcpy(ptr,&(node_ptr->val.second),sizeof(V));ptr+=sizeof(V);
                    assert(node_ptr->val.second);
                }
            }
            std::ofstream ofs(path, ios::binary);
            ofs.seekp(0);
            ofs.write(buf, size);
            assert(ofs.good());
            ofs.close();
            free(buf);
        }
        static void deserialize(ds::unordered_map<K,V>& map, const std::string& path) {
            std::ifstream ifs(path, ios::binary);
            DiskLoc_T size;
            ifs.seekg(0);
            ifs.read((char*) &size, sizeof(size));
            size -= sizeof(DiskLoc_T);
            if (size == 0) {
                ifs.close();
                return;
            }
            char* buf = (char*) malloc(size);
            char* ptr = buf;
            ifs.read(buf, size);
            ifs.close();
            for(int i=0,E=size/(sizeof(K)+sizeof(V));i<E;++i,ptr+=sizeof(K)+sizeof(V)){
                map[*((K*)ptr)]=*((V*)(ptr+sizeof(K)));
            }
            free(buf);
        }
        static void Init(const std::string& path){t_sys::init_subprocess(path);}
    };


}
#endif //TICKETSYSTEM_SERIALIZATIONHELPER_H
