#include "rbt.h"

/******************************* private_info *********************************************/
/*比较复杂的对外操作*/
#define _RBT_COLOR_RED   0
#define _RBT_COLOR_BLACK 1

/*叶子的操作*/
/*operation of pstLeafRight*/
#define _RBT_LEAF_RightGet(__rbt_leaf) \
    ((__rbt_leaf)->pstLeafRight)

#define _RBT_LEAF_RightSet(__rbt_leaf,__i) \
    ((__rbt_leaf)->pstLeafRight = (__i))

/*operation of pstLeafLeft*/
#define _RBT_LEAF_LeftGet(__rbt_leaf) \
    ((__rbt_leaf)->pstLeafLeft)

#define _RBT_LEAF_LeftSet(__rbt_leaf,__i) \
    ((__rbt_leaf)->pstLeafLeft = (__i))

/*operation of ucLeafColor*/
#define _RBT_LEAF_ColorGet(__rbt_leaf) \
    ((__rbt_leaf)->ucLeafColor)

#define _RBT_LEAF_ColorSet(__rbt_leaf,__i) \
    ((__rbt_leaf)->ucLeafColor = (__i))

/*树的相关操作*/
/*operation of pstRbtRoot*/
#define _RBTREE_RootGet(__rbtree) \
    ((__rbtree)->pstRbtRoot)

#define _RBTREE_RootSet(__rbtree,__i) \
    ((__rbtree)->pstRbtRoot = (__i))

/*operation of ulLeaves*/
#define _RBTREE_LeavesSelfAdd(__rbtree) \
    ((__rbtree)->ulLeaves++)
#define _RBTREE_LeavesSelfSub(__rbtree) \
    ((__rbtree)->ulLeaves--)

#define _RBT_STACK_DEPTH(__rbtree) (sizeof(RBTREE_LeavesGet(__rbtree))<<4)

#define _RBT_LEAF_INIT(__pstLeaf) \
    _RBT_LEAF_ColorSet(__pstLeaf, _RBT_COLOR_RED); \
    _RBT_LEAF_LeftSet(__pstLeaf,  0); \
    _RBT_LEAF_RightSet(__pstLeaf, 0)

extern void _RBT_Rotate_Right(RBTREE* pstRBTree, RBT_LEAF* apstLeafStack[], int i);
extern void _RBT_Rotate_Left(RBTREE* pstRBTree, RBT_LEAF* apstLeafStack[], int i);
extern void _RBT_LeafDel_Rebalance(RBTREE* pstRBTree 
                                            , RBT_LEAF* x 
                                            , RBT_LEAF* apstLeafStack[] 
                                            , int i);
                                            
/*********************************************************************************
  全局变量定义 
*********************************************************************************/

/*********************************************************************************
  Function Name: RBT_LeafFind 
  Description: 根据key值查找结点
  Return: NULL或找到的结点
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 28
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
RBT_LEAF* RBT_LeafFind(RBTREE* pstRBTree, 
                                RBT_LEAF* pstKey, 
                                RBT_KeyCompare pfKeyCompare)
{
    RBT_LEAF* pstLeaf = _RBTREE_RootGet(pstRBTree);
    char cResult = 0;

    if((RBT_NULL == pstKey) 
            || (RBT_NULL == pstRBTree) 
            || RBT_NULL == pfKeyCompare) {
        return RBT_NULL;
    }
    
    while(RBT_NULL != pstLeaf) {        
        cResult = pfKeyCompare(pstKey, pstLeaf);
        if(0 ==cResult) { 
            return pstLeaf; 
        }        
        else if(0 < cResult){ 
            pstLeaf = _RBT_LEAF_RightGet(pstLeaf); 
        } 
        else {             
            pstLeaf = _RBT_LEAF_LeftGet(pstLeaf); 
        }
    }

    return RBT_NULL;
}

/*********************************************************************************
  Function Name: RBT_LeafInsert_Update 
  Description: 插入结点，如果有重复键值的叶子，则进行更新
  Return: RBT_LEAF/ NULL；返回值为重复的叶子结点
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 19
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
RBT_LEAF* RBT_LeafInsert_Update(RBTREE* pstRBTree
                                            , RBT_LEAF* pstRBLeaf
                                            , RBT_KeyCompare pfKeyCompare)
{
    RBT_LEAF* pstLeaf = _RBTREE_RootGet(pstRBTree);
    RBT_LEAF* pstLeafParent = (RBT_LEAF*)pstRBTree;
    RBT_LEAF* apstLeafStack[_RBT_STACK_DEPTH(pstRBTree)];
    char cResult = 0;
    int i = 0;

    if((RBT_NULL == pstRBLeaf) 
            || (RBT_NULL == pstRBTree) 
            || RBT_NULL == pfKeyCompare) {
        return RBT_NULL;
    }
    
    /*插入结点*/
    _RBT_LEAF_INIT(pstRBLeaf);

    /*查找RBLeaf的位置,因为要记录路径栈，所以不能直接使用Find的函数*/
    if(RBT_NULL == pstLeaf) {
        _RBTREE_RootSet(pstRBTree, pstRBLeaf);
        _RBT_LEAF_ColorSet(pstRBLeaf, _RBT_COLOR_BLACK);        
        _RBTREE_LeavesSelfAdd(pstRBTree);
        return RBT_NULL;
    }

    while(RBT_NULL != pstLeaf) {        
        apstLeafStack[i++] = pstLeaf;
        
        cResult = pfKeyCompare(pstRBLeaf, pstLeaf);
        if(0 ==cResult) { 
            if(pstLeaf == _RBT_LEAF_RightGet(pstLeafParent)) { 
                _RBT_LEAF_RightSet(pstLeafParent, pstRBLeaf); 
            } 
            else { 
                _RBT_LEAF_LeftSet(pstLeafParent, pstRBLeaf); 
            } 
            _RBT_LEAF_ColorSet(pstRBLeaf, _RBT_LEAF_ColorGet(pstLeaf));
            return pstLeaf; 
        }        
        else if(0 < cResult){ 
            pstLeafParent = pstLeaf;
            pstLeaf = _RBT_LEAF_RightGet(pstLeaf); 
        } 
        else {             
            pstLeafParent = pstLeaf;
            pstLeaf = _RBT_LEAF_LeftGet(pstLeaf); 
        }
    }
    i--;
    
    _RBTREE_LeavesSelfAdd(pstRBTree);

    if(0 > cResult) {
        _RBT_LEAF_LeftSet(pstLeafParent, pstRBLeaf);
    }
    else {
        _RBT_LEAF_RightSet(pstLeafParent, pstRBLeaf);
    }

    /*开始平衡旋转*/
    while((pstRBLeaf != _RBTREE_RootGet(pstRBTree))
        &&(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(apstLeafStack[i]))) {
        RBT_LEAF* pstTmp;
        if(apstLeafStack[i] == _RBT_LEAF_LeftGet(apstLeafStack[i-1])) {
            pstTmp = _RBT_LEAF_RightGet(apstLeafStack[i-1]);
            if((RBT_NULL != pstTmp)
                &&(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(pstTmp))) {
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(pstTmp, _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);

                pstRBLeaf = apstLeafStack[--i];
                i--;
            }
            else {
                if(pstRBLeaf == _RBT_LEAF_RightGet(apstLeafStack[i])) {
                    pstTmp = pstRBLeaf;
                    _RBT_Rotate_Left(pstRBTree, apstLeafStack, i);                    
                    pstRBLeaf = apstLeafStack[i];
                    apstLeafStack[i] = pstTmp;
                }
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);
                _RBT_Rotate_Right(pstRBTree, apstLeafStack, i -1);
            }
        }
        else {
            pstTmp = _RBT_LEAF_LeftGet(apstLeafStack[i-1]);
            if((RBT_NULL != pstTmp)
                &&(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(pstTmp))) {
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(pstTmp, _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);

                pstRBLeaf = apstLeafStack[--i];
                i--;
            }            
            else {
                if(pstRBLeaf == _RBT_LEAF_LeftGet(apstLeafStack[i])) {
                    pstTmp = pstRBLeaf;
                    _RBT_Rotate_Right(pstRBTree, apstLeafStack, i);                    
                    pstRBLeaf = apstLeafStack[i];
                    apstLeafStack[i] = pstTmp;
                }
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);
                _RBT_Rotate_Left(pstRBTree, apstLeafStack, i -1);
            }
        }
    }

    _RBT_LEAF_ColorSet(_RBTREE_RootGet(pstRBTree), _RBT_COLOR_BLACK);
    
    return RBT_NULL;
}

/*********************************************************************************
  Function Name: RBT_LeafInsert
  Description: 插入结点
  Return: void
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 19
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
void RBT_LeafInsert(RBTREE* pstRBTree
                                            , RBT_LEAF* pstRBLeaf
                                            , RBT_KeyCompare pfKeyCompare)
{
    RBT_LEAF* pstLeaf = _RBTREE_RootGet(pstRBTree);
    RBT_LEAF* pstLeafParent = (RBT_LEAF*)pstRBTree;
    RBT_LEAF* apstLeafStack[_RBT_STACK_DEPTH(pstRBTree)];
    char cResult = 0;
    int i = 0;

    if((RBT_NULL == pstRBLeaf) 
            || (RBT_NULL == pstRBTree) 
            || RBT_NULL == pfKeyCompare) {
        return;
    }

    /*插入结点*/
    _RBT_LEAF_INIT(pstRBLeaf);
    _RBTREE_LeavesSelfAdd(pstRBTree);

    /*查找RBLeaf的位置,因为要记录路径栈，所以不能直接使用Find的函数*/
    if(RBT_NULL == pstLeaf) {
        _RBTREE_RootSet(pstRBTree, pstRBLeaf);
        _RBT_LEAF_ColorSet(pstRBLeaf, _RBT_COLOR_BLACK);        
        return;
    }

    while(RBT_NULL != pstLeaf) {        
        apstLeafStack[i++] = pstLeaf;
        
        cResult = pfKeyCompare(pstRBLeaf, pstLeaf);
        if(0 < cResult) { 
            pstLeafParent = pstLeaf;
            pstLeaf = _RBT_LEAF_RightGet(pstLeaf); 
        } 
        else {             
            pstLeafParent = pstLeaf;
            pstLeaf = _RBT_LEAF_LeftGet(pstLeaf); 
        }
    }
    i--;

    if(0 > cResult) {
        _RBT_LEAF_LeftSet(pstLeafParent, pstRBLeaf);
    }
    else {
        _RBT_LEAF_RightSet(pstLeafParent, pstRBLeaf);
    }

    /*开始平衡旋转*/
    while((pstRBLeaf != _RBTREE_RootGet(pstRBTree))
        &&(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(apstLeafStack[i]))) {
        RBT_LEAF* pstTmp;
        if(apstLeafStack[i] == _RBT_LEAF_LeftGet(apstLeafStack[i-1])) {
            pstTmp = _RBT_LEAF_RightGet(apstLeafStack[i-1]);
            if((RBT_NULL != pstTmp)
                &&(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(pstTmp))) {
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(pstTmp, _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);

                pstRBLeaf = apstLeafStack[--i];
                i--;
            }
            else {
                if(pstRBLeaf == _RBT_LEAF_RightGet(apstLeafStack[i])) {
                    pstTmp = pstRBLeaf;
                    _RBT_Rotate_Left(pstRBTree, apstLeafStack, i);                    
                    pstRBLeaf = apstLeafStack[i];
                    apstLeafStack[i] = pstTmp;
                }
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);
                _RBT_Rotate_Right(pstRBTree, apstLeafStack, i -1);
            }
        }
        else {
            pstTmp = _RBT_LEAF_LeftGet(apstLeafStack[i-1]);
            if((RBT_NULL != pstTmp)
                &&(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(pstTmp))) {
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(pstTmp, _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);

                pstRBLeaf = apstLeafStack[--i];
                i--;
            }            
            else {
                if(pstRBLeaf == _RBT_LEAF_LeftGet(apstLeafStack[i])) {
                    pstTmp = pstRBLeaf;
                    _RBT_Rotate_Right(pstRBTree, apstLeafStack, i);                    
                    pstRBLeaf = apstLeafStack[i];
                    apstLeafStack[i] = pstTmp;
                }
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i - 1], _RBT_COLOR_RED);
                _RBT_Rotate_Left(pstRBTree, apstLeafStack, i -1);
            }
        }
    }

    _RBT_LEAF_ColorSet(_RBTREE_RootGet(pstRBTree), _RBT_COLOR_BLACK);
    
    return;
}

/*********************************************************************************
  Function Name: RBT_LeafDelete 
  Description: 根据输入的key值进行叶子结点删除
  Return: 叶子节点；如果是空，则没有该结点
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 20
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
RBT_LEAF* RBT_LeafDelete(RBTREE* pstRBTree
                                , RBT_LEAF* pstKey
                                , RBT_KeyCompare pfKeyCompare)
{
    
    RBT_LEAF* pstLeaf = _RBTREE_RootGet(pstRBTree);
    RBT_LEAF* pstLeafParent = (RBT_LEAF*)pstRBTree;
    RBT_LEAF* apstLeafStack[_RBT_STACK_DEPTH(pstRBTree)];
    char cResult = 0;
    int i = 0;

    RBT_LEAF* x;
    char color = _RBT_COLOR_RED;

    
    if((RBT_NULL == pstKey) 
            || (RBT_NULL == pstRBTree) 
            || RBT_NULL == pfKeyCompare) {
        return RBT_NULL;
    }
    
    /*查找key值所在的结点，并记录路径栈*/
    while(RBT_NULL != pstLeaf) {        
        apstLeafStack[i++] = pstLeaf;
        
        cResult = pfKeyCompare(pstKey, pstLeaf);
        if(0 ==cResult) { 
            break;
        }        
        else if(0 < cResult) { 
            pstLeafParent = pstLeaf;
            pstLeaf = _RBT_LEAF_RightGet(pstLeaf); 
        } 
        else {             
            pstLeafParent = pstLeaf;
            pstLeaf = _RBT_LEAF_LeftGet(pstLeaf); 
        }
    }

    /*如果没有找到相应的结点，返回空指针*/
    if(RBT_NULL == pstLeaf) {
        return RBT_NULL;
    }
        
    _RBTREE_LeavesSelfSub(pstRBTree);

    color = _RBT_LEAF_ColorGet(pstLeaf);

    if((RBT_NULL != _RBT_LEAF_LeftGet(pstLeaf))
            && (RBT_NULL != _RBT_LEAF_RightGet(pstLeaf))) {
        RBT_LEAF* z = _RBT_LEAF_RightGet(pstLeaf);
        int j = i - 1;
        char zColor;

        pstLeafParent = apstLeafStack[i - 2];

        while(RBT_NULL != z) {            
            apstLeafStack[i++] = z;
            z = _RBT_LEAF_LeftGet(z);
        }

        z = apstLeafStack[--i];
        x = _RBT_LEAF_RightGet(z);       
        apstLeafStack[j] = z;     

        zColor = _RBT_LEAF_ColorGet(z);
            
        _RBT_LEAF_LeftSet(z, _RBT_LEAF_LeftGet(pstLeaf));
        _RBT_LEAF_ColorSet(z, color);
        
        i --;
        if(z != _RBT_LEAF_RightGet(pstLeaf)) {            
            _RBT_LEAF_RightSet(z, _RBT_LEAF_RightGet(pstLeaf));       
            _RBT_LEAF_LeftSet(apstLeafStack[i], x);
        }

        if(pstLeaf == _RBTREE_RootGet(pstRBTree)) {
            _RBTREE_RootSet(pstRBTree, z);
        }
        else {
            if(pstLeaf == _RBT_LEAF_LeftGet(pstLeafParent)) {
                _RBT_LEAF_LeftSet(pstLeafParent, z);
            }
            else {
                _RBT_LEAF_RightSet(pstLeafParent, z);
            }
        }        
        
        if(_RBT_COLOR_BLACK == zColor) {
            _RBT_LeafDel_Rebalance(pstRBTree, x, apstLeafStack, i);
        }
        return pstLeaf;
    }

    if(RBT_NULL == _RBT_LEAF_LeftGet(pstLeaf)) {
        x = _RBT_LEAF_RightGet(pstLeaf);
    }
    else {
        x = _RBT_LEAF_LeftGet(pstLeaf);
    }

    if(pstLeaf == _RBTREE_RootGet(pstRBTree)) {
        _RBTREE_RootSet(pstRBTree, x);
    }
    else {  
        i -= 2;
        if(pstLeaf == _RBT_LEAF_LeftGet(apstLeafStack[i])) {
            _RBT_LEAF_LeftSet(apstLeafStack[i], x);
        } 
        else {
            _RBT_LEAF_RightSet(apstLeafStack[i], x);
        }
    }
    if(_RBT_COLOR_BLACK == color) {
        _RBT_LeafDel_Rebalance(pstRBTree, x, apstLeafStack, i);
    }
    
    return pstLeaf;
}

/*********************************************************************************
  Function Name: _RBT_LeafDel_Rebalance 
  Description: 删除结点后进行平衡旋转
  Return: void
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 20
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
void _RBT_LeafDel_Rebalance(RBTREE* pstRBTree 
                                            , RBT_LEAF* x 
                                            , RBT_LEAF* apstLeafStack[] 
                                            , int i)
{
    RBT_LEAF* w;    
    
    while(((RBT_NULL == x) || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(x)))
            && (x != _RBTREE_RootGet(pstRBTree))) {
        if(x == _RBT_LEAF_LeftGet(apstLeafStack[i])) {
            w = _RBT_LEAF_RightGet(apstLeafStack[i]);
            if(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(w)) {
                _RBT_LEAF_ColorSet(w, _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_RED);
                _RBT_Rotate_Left(pstRBTree, apstLeafStack, i);

                apstLeafStack[i+1] = apstLeafStack[i];
                apstLeafStack[i++] = w;
                
                w = _RBT_LEAF_RightGet(apstLeafStack[i]);
            }

            if(((RBT_NULL == _RBT_LEAF_RightGet(w)) || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(_RBT_LEAF_RightGet(w))))
                    &&((RBT_NULL == _RBT_LEAF_LeftGet(w)) || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(_RBT_LEAF_LeftGet(w))))) {
                _RBT_LEAF_ColorSet(w, _RBT_COLOR_RED);
                x = apstLeafStack[i--];
            }
            else {
                if((RBT_NULL == _RBT_LEAF_RightGet(w)) 
                        || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(_RBT_LEAF_RightGet(w)))) {
                    if(RBT_NULL != _RBT_LEAF_LeftGet(w)) {
                        _RBT_LEAF_ColorSet(_RBT_LEAF_LeftGet(w), _RBT_COLOR_BLACK);
                    }

                    _RBT_LEAF_ColorSet(w, _RBT_COLOR_RED);
                    apstLeafStack[++i] = w;
                    _RBT_Rotate_Right(pstRBTree, apstLeafStack, i);

                    w = _RBT_LEAF_RightGet(apstLeafStack[--i]);
                }

                _RBT_LEAF_ColorSet(w, _RBT_LEAF_ColorGet(apstLeafStack[i]));
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);

                if(RBT_NULL != _RBT_LEAF_RightGet(w)) {
                    _RBT_LEAF_ColorSet(_RBT_LEAF_RightGet(w), _RBT_COLOR_BLACK);
                }

                _RBT_Rotate_Left(pstRBTree, apstLeafStack, i);
                x = _RBTREE_RootGet(pstRBTree);
                break;
            }
        }
        else {
            w = _RBT_LEAF_LeftGet(apstLeafStack[i]);
            if(_RBT_COLOR_RED == _RBT_LEAF_ColorGet(w)) {                               
                _RBT_LEAF_ColorSet(w, _RBT_COLOR_BLACK);
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_RED);
                
                _RBT_Rotate_Right(pstRBTree, apstLeafStack, i);

                apstLeafStack[i+1] = apstLeafStack[i];
                apstLeafStack[i++] = w;
                
                w = _RBT_LEAF_LeftGet(apstLeafStack[i]);
            }
            
            if(((RBT_NULL == _RBT_LEAF_LeftGet(w)) || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(_RBT_LEAF_LeftGet(w))))
                    &&((RBT_NULL == _RBT_LEAF_RightGet(w)) || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(_RBT_LEAF_RightGet(w))))) {
                _RBT_LEAF_ColorSet(w, _RBT_COLOR_RED);
                x = apstLeafStack[i--];
            }
            else {
                if((RBT_NULL == _RBT_LEAF_LeftGet(w)) 
                        || (_RBT_COLOR_BLACK == _RBT_LEAF_ColorGet(_RBT_LEAF_LeftGet(w)))) {
                    if(RBT_NULL != _RBT_LEAF_RightGet(w)) {
                        _RBT_LEAF_ColorSet(_RBT_LEAF_RightGet(w), _RBT_COLOR_BLACK);
                    }

                    _RBT_LEAF_ColorSet(w, _RBT_COLOR_RED);
                    apstLeafStack[++i] = w;
                    _RBT_Rotate_Left(pstRBTree, apstLeafStack, i);

                    w = _RBT_LEAF_LeftGet(apstLeafStack[--i]);
                }

                _RBT_LEAF_ColorSet(w, _RBT_LEAF_ColorGet(apstLeafStack[i]));
                _RBT_LEAF_ColorSet(apstLeafStack[i], _RBT_COLOR_BLACK);

                if(RBT_NULL != _RBT_LEAF_LeftGet(w)) {
                    _RBT_LEAF_ColorSet(_RBT_LEAF_LeftGet(w), _RBT_COLOR_BLACK);
                }

                _RBT_Rotate_Right(pstRBTree, apstLeafStack, i);
                x = _RBTREE_RootGet(pstRBTree);
                break;
            }
        }            
    }

    if(RBT_NULL != x) {
        _RBT_LEAF_ColorSet(x, _RBT_COLOR_BLACK);
    }

    return;
}

/*********************************************************************************
  Function Name: _RBT_Rotate_Right 
  Description: 向右旋转
  Return: void
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 19
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
void _RBT_Rotate_Right(RBTREE* pstRBTree, RBT_LEAF* apstLeafStack[], int i)
{
    RBT_LEAF* pstLeaf = _RBT_LEAF_LeftGet(apstLeafStack[i]);

    _RBT_LEAF_LeftSet(apstLeafStack[i], _RBT_LEAF_RightGet(pstLeaf));
    if(apstLeafStack[i] == _RBTREE_RootGet(pstRBTree)) {
        _RBTREE_RootSet(pstRBTree, pstLeaf);
    }
    else if(apstLeafStack[i] == _RBT_LEAF_RightGet(apstLeafStack[i -1])) {
        _RBT_LEAF_RightSet(apstLeafStack[i -1], pstLeaf);
    }
    else {
        _RBT_LEAF_LeftSet(apstLeafStack[i -1], pstLeaf);
    }

    _RBT_LEAF_RightSet(pstLeaf, apstLeafStack[i]);
}

/*********************************************************************************
  Function Name: _RBT_Rotate_Left 
  Description: 向左旋转
  Return: void
  History:
 --------------------------------------------
    1. created: Eagle 
        date: 2008- 8 - 19
    2. modified: Eagle
        date: 
        reason: 
*********************************************************************************/
void _RBT_Rotate_Left(RBTREE* pstRBTree, RBT_LEAF* apstLeafStack[], int i)
{
    RBT_LEAF* pstLeaf = _RBT_LEAF_RightGet(apstLeafStack[i]);

    _RBT_LEAF_RightSet(apstLeafStack[i], _RBT_LEAF_LeftGet(pstLeaf));
    if(apstLeafStack[i] == _RBTREE_RootGet(pstRBTree)) {
        _RBTREE_RootSet(pstRBTree, pstLeaf);
    }
    else if(apstLeafStack[i] == _RBT_LEAF_LeftGet(apstLeafStack[i -1])) {
        _RBT_LEAF_LeftSet(apstLeafStack[i -1], pstLeaf);
    }
    else {
        _RBT_LEAF_RightSet(apstLeafStack[i -1], pstLeaf);
    }

    _RBT_LEAF_LeftSet(pstLeaf, apstLeafStack[i]);
}

