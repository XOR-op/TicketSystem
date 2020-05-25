# Documentation of Ticket System

## 整体架构

## 缓存设计

### cache::LRUCache

设计时参照了mmap的思路，将文件的对应位置的块直接映射至内存中，依赖于人工的dirty_bit_set()实现写入一致性

##### bool remove(DiskLoc_T offset)

从cache中移除对应offset的块，并依照dirty_bit将修改后的数据写回磁盘

##### DataPtr get(DiskLoc_T offset)

若在cache里则返回对应地址，否则从磁盘中读取数据至cache中并返回

##### void dirty_bit_set(DiskLoc_T offset)

手动设置块对应的dirty_bit



#### t_sys::PageManager

以页为单位的缓存，从磁盘读入时需内存复制两次，因此效率不及LRUCache，但优势在于容易使用

##### void read(char* dst, DiskLoc_T offset, size_t size)

从offset中读取size大小数据至dst指向的内存空间中

##### void write(const char* src, DiskLoc_T offset, size_t size)

将src指向的size大小数据写入offset对应的磁盘地址中



## 文件结构

### LRUBPTree

##### metadata

| 字段          | 描述                 |
| ------------- | -------------------- |
| file_size     | 文件大小             |
| freelist_head | 第一个空闲块的offset |
| root          | 根结点offset         |

##### records

| 字段           | 描述                           |
| -------------- | ------------------------------ |
| type           | non leaf or leaf nodes         |
| offset         | -                              |
| next           | 后继结点（仅对type==LEAF生效） |
| prev           | 前驱结点（仅对type==LEAF生效） |
| size           | K的大小                        |
| K              | key数组                        |
| V or sub_nodes | v或子结点数组，size=size(K)+1  |

### UserManager

| 字段      | 描述              |
| --------- | ----------------- |
| file_size | -                 |
| is_null   | 是否size(用户)==0 |

### TrainManager

todo

### OrderManager

块状链表

| 字段 | 描述                |
| ---- | ------------------- |
| next | 下一个block的offset |
| size | data中order个数     |
| data | size个order         |

