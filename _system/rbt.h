#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus


/*************************** 用户可设置信息 *****************************/ 
/*define basic data structure*/
#define _RBT_ULONG      unsigned int
#define _RBT_USHORT     unsigned short
#define _RBT_UCHAR      unsigned char

/*定义错误码基址，因为系统可能需要对错误码进行统一编号*/
#define _RBT_ERRCODE_OFFSET 0

/**** 对外结构及接口*************/
/***************************************************************/
typedef struct stRBT_LEAF
{
    struct stRBT_LEAF* pstLeafRight;
    struct stRBT_LEAF* pstLeafLeft;
    _RBT_UCHAR ucLeafColor;
}RBT_LEAF;

/***************************************************************/
typedef struct stRBTREE
{
    RBT_LEAF* pstRbtRoot;
    _RBT_ULONG ulLeaves;
}RBTREE;

#define RBTREE_LeavesGet(__rbtree) \
    ((__rbtree)->ulLeaves)

typedef int (*RBT_KeyCompare)(RBT_LEAF* pstKey, RBT_LEAF* pstLeaf);
/*插入的叶子如果重复则对原来的叶子进行覆盖*/
extern RBT_LEAF* RBT_LeafInsert_Update(RBTREE* pstRBTree
                                                 , RBT_LEAF* pstRBLeaf
                                                 , RBT_KeyCompare pfKeyCompare);

/*插入的叶子如果重复继续插入*/
extern void RBT_LeafInsert(RBTREE* pstRBTree
                                 , RBT_LEAF* pstRBLeaf
                                 , RBT_KeyCompare pfKeyCompare);

/*叶子删除，如果有重复叶子，则删除第一个叶子*/
extern RBT_LEAF* RBT_LeafDelete(RBTREE* pstRBTree
                                        , RBT_LEAF* pstKey
                                        , RBT_KeyCompare pfKeyCompare);

/*叶子查找，如果有重复叶子，返回找到的第一个叶子*/
extern RBT_LEAF* RBT_LeafFind(RBTREE* pstRBTree,
  RBT_LEAF* pstKey,
  RBT_KeyCompare pfKeyCompare);

#define RBT_Walk_Begin(__rbtree, __iterator) {\
        RBT_LEAF* __apstLeaves[sizeof((__rbtree)->ulLeaves)<<4] = \
            { (__rbtree)->pstRbtRoot }; \
        int __i = 0; \
        __iterator = nullptr;\
        while(0 != __apstLeaves[__i]) { \
            __i++;\
            __apstLeaves[__i] =  __apstLeaves[__i-1]->pstLeafLeft; \
        } \
        __i--; \
        while(__i >= 0) { \
            __iterator = (void*)__apstLeaves[__i];\
            if(0 != __apstLeaves[__i]->pstLeafRight) { \
               __apstLeaves[__i] =  __apstLeaves[__i]->pstLeafRight; \
                while(0 != __apstLeaves[__i]) { \
                    __i++; \
                    __apstLeaves[__i] = __apstLeaves[__i - 1]->pstLeafLeft;\
                } \
            }

#define RBT_Walk_End(__iterator) \
            __i--; \
        } \
    }


/****对外的常量定义等*************/
/*define basic constant*/
#define RBT_OK          0
#define RBT_TRUE        1
#define RBT_FALSE       0
#define RBT_NULL        0 /*only refrence to null pointer*/

/*define ERROR_CODE*/
enum en_RBT_ERRCODE
{
   RBT_ERR = _RBT_ERRCODE_OFFSET + 1,
};

#ifdef __cplusplus
}
#endif //__cplusplus

