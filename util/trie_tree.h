/*
 * trie_tree.h
 *
 *  Created on: 2014年12月17日
 *      Author: liwei
 */
/*
 * 字典树 ，为每条字符序列放一个指针v
 */
#ifndef TRIE_TREE_H_
#define TRIE_TREE_H_
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "db_chain.h"
//#define __i3861__ 1
#if __i386__
#include "bitmap.h"
#elif __x86_64__
/*当一个字符序列结束，并且字典树中没有包含这个序列的序列时（即序列的末尾也是这条分支的结尾），
 * 将不会为序列的最后一个字符单独创建一个节点，而是用node->child[pos]这个指针来保存这个序列的value。
 * 当一个字符序列结束，并且字典树中有包含这个序列的序列时，序列末尾节点必然有子树，
 * 它的node->child[pos]需要用来保存子树的指针，这时用node->child[0]来保存这个序列的value。
 * 例如序列“abc”,序列对应的值为v,并且没有以“abc”开头的其他序列时，将创建节点a，b，a->child['b']=b. b->child['c']=v
 * 当有序列“abc”，v1；”abcd“，v2，时，将创建节点a,b,c,a->child['b']=b. b->child['c']=c.c->child[0]=v1.c->child['d']=v2
 * 在node->child[pos]！=NULL时，无法判断node->child[pos]到底是子树的指针还是v。64位与32位下用了不同的方式来设置标识位
*/
//x86 64模式下 0xffffffff80000000以上的线性地址空间为内核态，用户态不可能得到这样的线性地址，故可使用高位作为标识位
#define TRIE_MASK 0x8000000000000000
#define TRIE_C_MASK 0x8000000000000000
#endif
#ifndef __CHARMAP
#define __CHARMAP
static unsigned char charmap[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xe1, 0xe2, 0xe3, 0xe4, 0xc5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};
#endif
#define MAX_TRIE_CACHE 32
#define TRIE_LRU 0x00000001
typedef struct _trie_node
{
	struct _trie_node**child;
	struct _trie_node* pnode;
	chain_node lru_node;
	unsigned char childpos[256];
	unsigned char character;
	unsigned char count;
	unsigned char child_node_count;
	unsigned char mem_count;
#if __i386__
//x86 32模式下，0xc00000000以上的线性地址空间为内核态，无法用高位做做标识位，故使用bitmap来判断
	d_bitmap(mp,256);
#endif
}trie_node;
typedef struct _trie_tree
{
	trie_node root;
	chain_head lru;
	chain_head pre_lru;
	int max_node;
	int node;
#ifdef TRIE_USE_CACHE
	trie_node *cache[MAX_TRIE_CACHE];
	int cache_count;
#endif
    uint32_t flag;
	int (*free_value)(void*,int);//格式要求：清除成功必须返回0
}trie_tree;
#define init_trie_node(node) do{memset((node),0,sizeof(trie_node));(node)->child=(trie_node**)malloc(((node)->mem_count=20)*sizeof(trie_node*));memset((node)->child,0,sizeof(trie_node*)*20);}while(0);
#if __i386__
#define is_trie_end(node,pos) is_setted_bitmap((node)->mp,(pos))
#define set_trie_end(node,pos,v) (((node)->child[(node)->childpos[(pos)]]=(trie_node*)(unsigned long)(v)),set_bitmap((node)->mp,pos))
#define get_trie_end_value(node,pos) ((node)->child[(node)->childpos[(pos)]])
#elif __x86_64__
#define is_trie_end(node,pos) (((unsigned long)(node)->child[(node)->childpos[(pos)]])&TRIE_C_MASK)
#define set_trie_end(node,pos,v) (((node)->child[(node)->childpos[(pos)]])=(trie_node*)(((unsigned long)v)|TRIE_C_MASK))
#define get_trie_end_value(node,pos) (void*)(((unsigned long)(node)->child[(node)->childpos[(pos)]])&(~TRIE_MASK))
#endif

static inline  trie_node *GET_NEW_TRIE_NODE(trie_tree *tree)
{
#ifdef TRIE_USE_CACHE
	if(tree->cache_count<=0)
#endif
	{
		trie_node *__tmp=(trie_node*)malloc(sizeof(trie_node));
		init_trie_node(__tmp);
		return __tmp;
	}
#ifdef TRIE_USE_CACHE
	else
		return tree->cache[--(tree)->cache_count];
#endif
}
static inline void init_trie_tree(trie_tree *tree,int max_node_count,int (*_free_value)(void*,int),int flag=TRIE_LRU)
{
	c_init_chain(&tree->lru);
	c_init_chain(&tree->pre_lru);
	init_trie_node(&tree->root);
	tree->root.child[0]=(trie_node*)TRIE_C_MASK;
	tree->root.child_node_count=1;
	c_insert_in_head(&tree->pre_lru,&tree->root.lru_node);
	tree->max_node=max_node_count;
	tree->free_value=_free_value;
#ifdef TRIE_USE_CACHE
	tree->cache_count=0;
#endif
	tree->node=0;
	tree->flag=flag;
}
static inline int free_trie_node(trie_tree *tree,trie_node* node,int force)
{
	trie_node* p;
	void * ptr;
	if(node==&tree->root)
		return 0;
	if(node->child[0]!=NULL)
	{
		if(0!=tree->free_value(node->child[0],force)&&!force)
			return -1;
		node->child[0]=NULL;
	}
	if(node->count>0)
	{
#ifdef TRIE_USE_CACHE
		//如果cache可以回收node，则清理掉node的child于childpos数组，下次再利用时将不需要再初始化
		if(tree->cache_count<MAX_TRIE_CACHE-2)
		{
			for(int i='0';i<256&&node->count>0;i++)
			{
				if(node->childpos[i]!=0)
				{
					if((ptr=get_trie_end_value(node,i))!=NULL)
					{
						if(0!=tree->free_value(ptr,force)&&!force)
							return -1;
						node->count--;
						node->child[node->childpos[i]]=NULL;
						node->childpos[i]=0;
					}
				}
			}
			for(int i=1;i<'0'&&node->count>0;i++)
			{
				if(node->childpos[i]!=0)
				{
					if((ptr=get_trie_end_value(node,i))!=NULL)
					{
						if(0!=tree->free_value(ptr,force)&&!force)
							return -1;
						node->count--;
						node->child[node->childpos[i]]=NULL;
						node->childpos[i]=0;
					}
				}
			}
		}
		//没有空间来回收，则只free掉value
		else
#endif
		{
			for(int i='0';i<256&&node->count>0;i++)
			{
				if(node->childpos[i]!=0)
				{
					if((ptr=get_trie_end_value(node,i))!=NULL)
					{
						if(0!=tree->free_value(ptr,force)&&!force)
							return -1;
						node->count--;
					}
				}
			}
			for(int i=1;i<'0'&&node->count>0;i++)
			{
				if(node->childpos[i]!=0)
				{
					if((ptr=get_trie_end_value(node,i))!=NULL)
					{
						if(0!=tree->free_value(ptr,force)&&!force)
							return -1;
						node->count--;
					}
				}
			}
		}
	}
	if(node->child_node_count>0)
		return 0;
	//向上递归释放路径上的节点直到节点还有其他路径
	do
	{
		p=node->pnode;
		unsigned char c=node->character;
#ifdef TRIE_USE_CACHE
		if(tree->cache_count<MAX_TRIE_CACHE-2)
		{
			c_delete_node(&tree->lru,&node->lru_node);
			node->lru_node.next=node->lru_node.prev=NULL;
			tree->cache[tree->cache_count++]=node;
			tree->node--;
		}
		else
#endif
		{
			c_delete_node(&tree->lru,&node->lru_node);
			free(node->child);
			free(node);
			tree->node--;
		}
		//父节点必须为非root节点，只能有一个子节点（即node），不能是一个word的终点才可以释放
		//即父节点为root节点，或者父节点还有其他子接点，或者父节点是一个word的终点时，停止
		p->child[p->childpos[c]]=NULL;
		p->childpos[c]=0;
		p->child_node_count--;
		if((p->child_node_count==0&&p!=&tree->root&&p->child[0]==NULL&&p->count==0))
		{
			node=p;
		}
		else
		{
			if(p->child_node_count==0&&p!=&tree->root)
			{
				c_delete_node(&tree->pre_lru,&p->lru_node);
				c_insert_in_head(&tree->lru,&p->lru_node);
			}
			break;
		}
	}while(1);
	return 0;
}
static void clear_trie_cache(trie_tree *tree)
{
#ifdef TRIE_USE_CACHE
	for(int i=0;i<tree->cache_count;i++)
	{
		free(tree->cache[i]->child);
		free(tree->cache[i]);
	}
	tree->cache_count=0;
#endif
}
static void destroy_trie_node_value(trie_tree *tree,trie_node* node)
{
	void *ptr;
	if(node->child[0]!=NULL&&node!=&tree->root)
		tree->free_value(node->child[0],1);
	for(int i='0';i<256&&node->count>0;i++)
	{
		if(node->childpos[i]!=0&&is_trie_end(node,i))
		{
			if((ptr=get_trie_end_value(node,i))!=NULL)
			{
				tree->free_value(ptr,1);
				node->count--;
			}
		}
	}
	for(int i=1;i<'0'&&node->count>0;i++)
	{
		if(node->childpos[i]!=0&&is_trie_end(node,i))
		{
			if((ptr=get_trie_end_value(node,i))!=NULL)
			{
				tree->free_value(ptr,1);
				node->count--;
			}
		}
	}
}
#define DESTORY_TRIE_NODE(node) destroy_trie_node_value(tree,(node));free((node)->child);free(node);
//todo
static inline void destroy_trie_tree(trie_tree *tree,int force)
{
	/*
	trie_node* tmp;
	void* ptr;
	if(tree->lru.count!=0)
	{
		c_get_list_of_chain_b(node,&tree->lru,trie_node,lru_node)
		{
			tmp=get_next_dt(node,trie_node,lru_node);
			c_delete_node(&tree->lru,&node->lru_node);
			free_trie_node(tree,node,force);
			node=tmp;
		}
	}
	for(int i=1;i<256&&tree->root.count>0;i++)
	{
		if(tree->root.childpos[i]!=0&&is_trie_end(&tree->root,i))
		{
			if((ptr=get_trie_end_value(&tree->root,i))!=NULL)
			{
				if(0!=tree->free_value(ptr,force)&&!force)
					return ;
			}
		}
	}

	 * */
	c_delete_node(&tree->pre_lru,&tree->root.lru_node);
	destroy_trie_node_value(tree,&tree->root);
	free(tree->root.child);
	realase_chain(&tree->lru,trie_node,lru_node,DESTORY_TRIE_NODE);
	realase_chain(&tree->pre_lru,trie_node,lru_node,DESTORY_TRIE_NODE);
	clear_trie_cache(tree);
}
static void clear_trie(trie_tree *tree)
{
	c_delete_node(&tree->pre_lru,&tree->root.lru_node);
	destroy_trie_node_value(tree,&tree->root);
	realase_chain(&tree->lru,trie_node,lru_node,DESTORY_TRIE_NODE);
	realase_chain(&tree->pre_lru,trie_node,lru_node,DESTORY_TRIE_NODE);
	c_insert_in_head(&tree->pre_lru,&tree->root.lru_node);
	memset(&tree->root.childpos,0,sizeof(tree->root.childpos));
}
static inline int destroy_trie(void * v,int force)
{
	destroy_trie_tree((trie_tree *)v,force);
	free(v);
	return 0;
}
static inline int trie_tree_lru(trie_tree *tree)
{
	trie_node* tmp;
	if(tree->lru.count==0)
		return -1;
	c_get_list_of_chain_b(node,&tree->lru,trie_node,lru_node)
	{
		tmp=get_next_dt(node,trie_node,lru_node);
		//c_delete_node(&tree->lru,&node->lru_node);
		if(0==free_trie_node(tree,node,0))
		{
			node=tmp;
			if(tree->node<tree->max_node)
				return 0;
			else
				continue;
		}
		//c_insert_in_end(&tree->lru,&node->lru_node);
	}
	return -1;
}
static inline void get_child_ptr_pos(trie_node *n,unsigned char pos)
{
	if(n->mem_count>n->child_node_count+n->count+1)
	{
		if(n->child[n->count+n->child_node_count+1]==NULL)
			n->childpos[pos]=n->count+n->child_node_count+1;
		else
		{
			for(int i=1;i<n->mem_count;i++)
			{
				if(n->child[i]==NULL)
				{
					n->childpos[pos]=i;
					break;
				}
			}
		}
	}
	else
	{
		struct _trie_node** _tmp=(struct _trie_node**)malloc(sizeof(n->child[0])*(n->mem_count+10));
		memcpy(_tmp,n->child,sizeof(n->child[0])*n->mem_count);
		free(n->child);
		n->child=_tmp;
		//n->child=(struct _trie_node**)realloc(n->child,sizeof(n->child[0])*(n->mem_count+=10));
		n->childpos[pos]=n->count+n->child_node_count+1;
		memset(&n->child[n->mem_count],0,sizeof(trie_node*)*10);
		n->mem_count+=10;
	}
}
static inline int trie_erase(trie_tree *tree,unsigned char* word)
{
    const unsigned char *pos=word;
    unsigned char p;
    trie_node *n=&tree->root;
begin:
    p=charmap[*pos];
    if(n->childpos[p]==0)
        return -1;
    if(*(pos+1)==0)
    {
        if(is_trie_end(n,p))
        {
            if(n->count==1&&n->child[0]==NULL&&n->child_node_count==0)
            {
                free_trie_node(tree,n,1);
                return 0;
            }
            else
            {
                n->count--;
                tree->free_value(get_trie_end_value(n,p),1);
                n->child[n->childpos[p]]=NULL;
                n->childpos[p]=0;
                return 0;
            }
        }
        else if(n->child[n->childpos[p]]->child[0]!=NULL)
        {
            n=n->child[n->childpos[p]];
            if(n->count==0&&n->child_node_count==0)
            {
                free_trie_node(tree,n,1);
                return 0;
            }
            else
            {
                tree->free_value(n->child[0],1);
                n->child[0]=NULL;
                return 0;
            }
        }
        else
            return -1;
    }
    else if(is_trie_end(n,p))
        return -1;
    n=n->child[n->childpos[p]];
    pos++;
    goto begin;
}
static inline int insert_trie_tree(trie_tree *tree,unsigned char* word,void *ptr)
{
	unsigned char *pos=word,p;
	trie_node *n=&tree->root,*tmp;
	if(*pos==0)
		return -1;
	//循环从n中查找下一层
begin:
	p=charmap[*pos];
	//如果pos对应的节点为NULL,即之前没有任何分支经过pos
	if(n->childpos[p]==0)
	{
		//到pos时结束，则直接在n->child[pos]上保存ptr
		if(*(pos+1)==0)
		{
			n->count++;
			get_child_ptr_pos(n,p);
			set_trie_end(n,p,ptr);
			return 0;
		}
		else //没有结束，为pos创建一个节点
		{
			if(n->child_node_count==0)
			{
				c_delete_node(&tree->lru,&n->lru_node);
				c_insert_in_head(&tree->pre_lru,&n->lru_node);
			}
			if(tree->node>=tree->max_node&&(!(tree->flag&TRIE_LRU)||trie_tree_lru(tree)!=0))
			{
			        if(n->child_node_count==0)
                        	{
                                	c_delete_node(&tree->pre_lru,&n->lru_node);
                                	c_insert_in_head(&tree->lru,&n->lru_node);
                        	}
				return -2;
			}
			n->child_node_count++;
			get_child_ptr_pos(n,p);
			tmp=GET_NEW_TRIE_NODE(tree);
			tmp->pnode=n;
			tmp->count=tmp->child_node_count=0;
			tmp->character=p;
			c_insert_in_head(&tree->lru,&tmp->lru_node);
			n->child[n->childpos[p]]=tmp;
			tree->node++;
		}
	}
	//有一个序列在此终止，并且没有包含这个序列的序列
	else if(is_trie_end(n,p))
	{
		//序列结束，两个序列相同，插入失败
		if(*(pos+1)==0)
			return -1;
		//本序列包含之前的序列，为pos创建节点,并用新节点的child[0]保存原先序列的指针
		else
		{
			if(tree->node>=tree->max_node&&trie_tree_lru(tree)!=0)
				return -2;
			if(n->child_node_count==0)
			{
				c_delete_node(&tree->lru,&n->lru_node);
				c_insert_in_head(&tree->pre_lru,&n->lru_node);
			}
			n->count--;
			n->child_node_count++;
			tmp=GET_NEW_TRIE_NODE(tree);
			tmp->pnode=n;
			tmp->count=tmp->child_node_count=0;
			tmp->child[0]=(trie_node*)get_trie_end_value(n,p);
			tmp->character=p;
			c_insert_in_head(&tree->lru,&tmp->lru_node);
			n->child[n->childpos[p]]=tmp;
			tree->node++;
		}
	}
	//目前尚在一个已经存在的序列中
	else
	{
		//本序列终止
		if(*(pos+1)==0)
		{
			n=n->child[n->childpos[p]];
			//有一个序列在此终止，并且有包含这个序列的序列，两个序列相同，插入失败
			if(n->child[0]!=NULL)
				return -1;
			//序列在此终止，并且有包含本序列的序列
			else
			{
				n->child[0]=(struct _trie_node*)ptr;
				return 0;
			}
		}
	}
	n=n->child[n->childpos[p]];
	pos++;
	goto begin;
}
static inline void* get_trie_tree_value(trie_tree *tree,const unsigned char* word)
{
	const unsigned char *pos=word;
	unsigned char p;
	trie_node *n=&tree->root;
begin:
	p=charmap[*pos];
	if(n->childpos[p]==0)
		return NULL;
	if(*(pos+1)==0)
	{
		if(is_trie_end(n,p))
		{
			if((tree->flag&TRIE_LRU)&&n->child_node_count==0)
			{
				c_delete_node(&tree->lru,&n->lru_node);
				c_insert_in_head(&tree->lru,&n->lru_node);
			}
			return get_trie_end_value(n,p);
		}
		else if(n->child[n->childpos[p]]->child[0]!=NULL)
		{
			if((tree->flag&TRIE_LRU)&&n->child[n->childpos[p]]->child_node_count==0)
			{
				c_delete_node(&tree->lru,&n->child[n->childpos[p]]->lru_node);
				c_insert_in_head(&tree->lru,&n->child[n->childpos[p]]->lru_node);
			}
			return n->child[n->childpos[p]]->child[0];
		}
		else
			return NULL;
	}
	else if(is_trie_end(n,p))
		return NULL;
	n=n->child[n->childpos[p]];
	pos++;
	goto begin;
}
#if 0
/*
static inline int get_size(trie_tree *tree,int (*v_get_size)(void *v))
{
	int stack_size=128;
	trie_node ** stack=(trie_node **)malloc(sizeof(trie_node *)*stack_size);
	unsigned char *stacki=(char*)malloc(stack_size*sizeof(char));
	int top=0;
	unsigned char i=1;
	trie_node * tmp=&tree->root;
	stacki[top]=0;
	stack[top]=tmp;
b:
	i=stacki[top]+1;
	for(;i<255;i++)
	{
		if(stack[top]->childpos[i]==0)
			continue;
		if(is_trie_end(stack[top],i))
		{
			for(int j=1;j<=top;j++)
				printf("%c",stack[j]->character);
			printf("%c		",i);
			printf("%s\n",(char*)get_trie_end_value(stack[top],i));
			//top--;
			//i=stacki[top]+1;
		}
		else
		{
			stacki[top]=i;
			stacki[top+1]=1;
			stack[top+1]=stack[top]->child[stack[top]->childpos[i]];
			if(++top>=stack_size)
			{
				 stack=(trie_node **)realloc(stack,sizeof(trie_node *)*(stack_size+=128));
				 stacki=(char*)realloc(stacki,stack_size*sizeof(char));
			}
			i=1;
			if(stack[top]->child[0]!=NULL)
			{
				for(int j=1;j<=top;j++)
					printf("%c",stack[j]->character);
				printf("		%s",(char*)stack[top]->child[0]);
				printf("\n");
			}
		}
	}
	if(top>0)
	{
		top--;
		goto b;
	}
}
*/
static inline void prt(trie_tree *tree)
{
	trie_node * stack[128];
	unsigned char stacki[128];
	int top=0;
	unsigned char i=1;
	trie_node * tmp=&tree->root;
	stacki[top]=0;
	stack[top]=tmp;
b:
	i=stacki[top]+1;
	for(;i<255;i++)
	{
		if(stack[top]->childpos[i]==0)
			continue;
		if(is_trie_end(stack[top],i))
		{
			for(int j=1;j<=top;j++)
				printf("%c",stack[j]->character);
			printf("%c		",i);
			printf("%s\n",(char*)get_trie_end_value(stack[top],i));
			//top--;
			//i=stacki[top]+1;
		}
		else
		{
			stacki[top]=i;
			stacki[top+1]=1;
			stack[top+1]=stack[top]->child[stack[top]->childpos[i]];
			top++;
			i=1;
			if(stack[top]->child[0]!=NULL)
			{
				for(int j=1;j<=top;j++)
					printf("%c",stack[j]->character);
				printf("		%s",(char*)stack[top]->child[0]);
				printf("\n");
			}
		}
	}
	if(top>0)
	{
		top--;
		goto b;
	}
}
static inline void prt1(trie_node *node)
{
	for(unsigned char i=0;i<255;i++)
	{
		if(node->childpos[i]==0)
			continue;
		if(is_trie_end(node,i))
		{
			printf("%c\n",i);
		}
		else
		{
			prt1(node->child[i]);
		}
	}
	return;
}
#endif
#endif /* TRIE_TREE_H_ */
