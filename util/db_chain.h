/*
 * db_chain.h
 *
 *  Created on: 2014-9-3
 *      Author: liwei
 */

#ifndef DB_CHAIN_H_
#define DB_CHAIN_H_
//用于c语言则需要使用c99编译
#include <stddef.h>
#include <stdlib.h>
//链表节点
typedef struct chaint_node
{
    struct chaint_node *prev;
    struct chaint_node *next;
}chain_node;
//链表表头
typedef struct chain_cst
{
    struct chaint_node head;
#ifndef CHAIN_NO_COUNT
    unsigned int count;
#endif
}chain_head;

#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})


#define container_of_chain_node(ptr, type, member) (type *)( (char *)ptr - offsetof(type,member) )

//将pins_node链表节点指针指向的节点插入到表头pchain的pnode节点的后面
#ifndef CHAIN_NO_COUNT
#define  INC_CHIAN_COUNT(pchain) (pchain)->count++;
#define  DEC_CHIAN_COUNT(pchain) (pchain)->count--;
#define  ADD_CHINA_COUNT(pchain,cnt) (pchain)->count+=(cnt)
#define  GET_CHINA_COUNT(pchain) (pchain)->count
#else
#define  INC_CHIAN_COUNT(pchain)
#define  DEC_CHIAN_COUNT(pchain)
#define  ADD_CHINA_COUNT(pchain,count)
#define  GET_CHINA_COUNT(pchain)
#endif

#define c_insert_aft(pchain,pnode,pins_node) {  \
    struct chaint_node *__tmp=(pnode)->next;\
    (pins_node)->prev=(pnode); \
    (pins_node)->next=__tmp; \
    (pnode)->next=(pins_node); \
    __tmp->prev=(pins_node); \
    INC_CHIAN_COUNT(pchain);\
    };

//初始化表头

#ifndef CHAIN_NO_COUNT
#define c_init_chain(pchain) { \
    (pchain)->count=0;\
    (pchain)->head.next=(pchain)->head.prev=&(pchain)->head;\
    };
#else
#define c_init_chain(pchain) { \
    (pchain)->head.next=(pchain)->head.prev=&(pchain)->head;\
    };
#endif
//将节点插入到链表头部
#define c_insert_in_head(pchain,pins_node) c_insert_aft((pchain),&(pchain)->head,(pins_node));
//将节点插入到链表尾部
#define c_insert_in_end(pchain,pins_node) c_insert_aft((pchain),(pchain)->head.prev,(pins_node));
//pnode是否是链表的末尾节点
#define c_is_end(pchain,pnode) ((pnode)->next==&(pchain)->head)
#define c_is_head(pchain,pnode) ((pnode)->prev==&(pchain)->head)
//从链表中移除节点，这个宏并不会释放掉节点的内存
#define c_delete_node(pchain,pdel_node) {(pdel_node)->prev->next=(pdel_node)->next;\
    (pdel_node)->next->prev=(pdel_node)->prev;\
    (pdel_node)->prev=(pdel_node)->next=(struct chaint_node *)0;\
    DEC_CHIAN_COUNT(pchain);};

#define c_lru(pchain,pnode) { \
(pnode)->prev->next=(pnode)->next;\
(pnode)->next->prev=(pnode)->prev;\
(pnode)->next=(pchain)->head.next;\
(pnode)->prev=&(pchain)->head;\
(pnode)->next->prev= (pnode);\
(pchain)->head.next=(pnode);\
}
//链表是否为空
//#define c_is_empty(pchain) ((pchain)->count==0)
#define c_is_empty(pchain) ((pchain)->head.next==&(pchain)->head)
//得到节点所对应的数据，p_chain_node是数据的链表节点的指针，type是数据的类型，member是链表节点结构体struct chaint_node在数据中的名称
#define get_dt_from_chain(p_chain_node,type,member) (container_of_chain_node(p_chain_node,type,member))
//得到数据在链表中的下一个数据
#define get_next_dt(pdt,type,member) get_dt_from_chain(((pdt)->member.next),type,member)
//得到数据在链表中的上一个数据
#define get_prev_dt(pdt,type,member) get_dt_from_chain(((pdt)->member.prev),type,member)
#define get_first_dt(pchain,type,member) (c_is_empty(pchain))?NULL:(get_dt_from_chain((pchain)->head.next,type,member))
#define get_last_dt(pchain,type,member) (c_is_empty(pchain))?NULL:(get_dt_from_chain((pchain)->head.prev,type,member))
#ifdef __cplusplus
#define cpp_del(p) delete p;
#endif
#define c_free(p) free(p);
//释放链表节点（不包括链表头）和链表节点所在的数据，deal_type是删除数据的方法，如果数据是使用malloc创建的，那么请将此参数设置为c_free，如果是使用c++的new创建的，请将此参数设置为cpp_del
#define realase_chain(pchain,type,member,deal_type) do{ \
    type *pdt; \
    while(!c_is_empty(pchain)) \
    {   \
        pdt=get_dt_from_chain((pchain)->head.next,type,member);\
        c_delete_node(pchain,&pdt->member);\
        deal_type(pdt);\
    }   \
}while(0);
//用于遍历链表中的数据的宏，实际上就是for循环的for部分，pdt_name是用于遍历的数据的指针，申明是放在for语句中的，不需要在外部申明
#define c_get_list_of_chain(pdt_name,pchain,type,member)  for(type *pdt_name=get_dt_from_chain((pchain)->head.next,type,member);\
    ((struct chaint_node *)((char*)pdt_name+offsetof(type,member)))!=&(pchain)->head;pdt_name=get_next_dt(pdt_name,type,member))

#define c_get_list_of_chain_b(pdt_name,pchain,type,member)  for(type *pdt_name=get_dt_from_chain((pchain)->head.prev,type,member);\
    ((struct chaint_node *)((char*)pdt_name+offsetof(type,member)))!=&(pchain)->head;pdt_name=get_prev_dt(pdt_name,type,member))
static inline void c_merge_list(chain_head *pchain_merge, chain_head *pchain)
{
    pchain_merge->head.prev->next = pchain->head.next;
    pchain->head.next->prev = pchain_merge->head.prev;
    pchain->head.prev->next = &pchain_merge->head;
    pchain_merge->head.prev = pchain->head.prev;
    ADD_CHINA_COUNT(pchain_merge, GET_CHINA_COUNT(pchain));
    c_init_chain(pchain);
}
static inline void check_chain(chain_head *head)
{
    int i=0;
    chain_node *cn=head->head.next;
    while(cn!=&head->head)
    {
        cn=cn->next;
        i++;
    }
#ifndef CHAIN_NO_COUNT
    if((unsigned int)i!=head->count)
        abort();
#endif
    i=0;
    cn = head->head.prev;
    while (cn != &head->head)
    {
        cn = cn->prev;
        i++;
    }
#ifndef CHAIN_NO_COUNT
    if((unsigned int)i!=head->count)
        abort();
#endif
}
//deamon：
#if 0
//////////////////////
struct test_st
{
    int i;
    chain_node node;
};
//////////////////////
chain_head head;
test_st * tmp;
c_init_chain(&head);
for(int i=0;i<100;i++)
{
    tmp=new test_st;
    tmp->i=i;
    c_insert_in_head(&head,&tmp->node);
}
c_get_list_of_chain(pnext,&head,test_st,node)
{
    cout<<pnext->i<<endl;
}
realase_chain(&head,test_st,node,cpp_del);
#endif
#endif /* DB_CHAIN_H_ */
