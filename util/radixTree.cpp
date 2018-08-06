/*
 * trieTree.cpp
 *
 *  Created on: 2018年6月7日
 *      Author: liwei
 */
#define __cplusplus 201103L
#include <mutex>
#include <queue>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "perThread.h"
#include "db_chain.h"
using namespace std;
/*
 * only support x86 platform
 * */
#define RADIX_BASE 4
#define RADIDX_NODE_SLOTS (1u<<RADIX_BASE)
#define GET_VALUE(data,idx) ((data)[(idx)>>1]>>(((idx)&0x01)>>2))
#if __x86_64__
#define IS_NORMAL_PTR(ptr) (((uint64_t)(void*)(ptr))<0xffff800000000000lu)
#define CREATE_SPECIFIC_PTR(ptr,v) (ptr)=((v)|0xffff800000000000lu)
#define GET_SPECIFIC_PTR_REAL_VALUE(ptr) (ptr)&(~0xffff800000000000lu)
#elif __i386__
#define IS_NORMAL_PTR(ptr) (((uint32_t)(void*)(ptr))<0xc0000000u)
#define CREATE_SPECIFIC_PTR(ptr,v) (ptr)=((v)|0xc0000000u)
#define GET_SPECIFIC_PTR_REAL_VALUE (ptr)&(~0xc0000000u)
#endif
#define VERSION_OUT_MASK 0x80000000u
#define SET_VERSION_IN(dest,version) (dest) = ((version)|VERSION_OUT_MASK)
#define SET_VERSION_OUT(dest,version) (dest) = (version)
#define IS_VERSION_OUT(v) (!((v)&VERSION_OUT_MASK))
#define GET_REAL_VERSION(v) ((v)&(~VERSION_OUT_MASK))
template<typename T>
class redixTree
{
private:
	struct _node
	{
		union
		{
			struct _node* child[RADIDX_NODE_SLOTS];
			void * valueInLeaf[RADIDX_NODE_SLOTS];
		};
		union
		{
			struct _node* parent;
			uint32_t expiryVersion;
		};
		struct _node* prev;
		struct _node* next;
		uint8_t childNum;
		uint8_t index;
		union
		{
			uint16_t fisrtAndEnd;
			struct
			{
				uint8_t first;
				uint8_t end;
			};
		};
		bool isLeaf;
		_node():parent(NULL),prev(NULL),next(NULL),childNum(0),index(0),fisrtAndEnd(0),isLeaf(false)
		{
			memset(child, 0, sizeof(child));
		}
	};
	PER_THREAD_T(m_versionLocal,uint32_t);
	std::mutex m_lock;
	std::queue<_node*> m_expiryNodeQueue;
	_node m_root;
	chain_head m_lru;
	chain_head m_pre_lru;
	int m_max_node;
	int m_node;
	int m_version;
	uint32_t m_flag;
	int (*m_free_value)(void*, int); //格式要求：清除成功必须返回0
	const uint m_typeSize;
public:
	class iterator
	{
	private:
		const redixTree * m_tree;
		const _node * m_node;
		uint32_t m_indexInNode;
		T m_key;
		void * m_value;
	private:
		iterator(redixTree * tree,const _node * node,uint32_t indexInNode):m_tree(tree),m_node(node),m_indexInNode(indexInNode)
		{
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),tree->m_version);
			if(valid())
			{
				m_key = tree->getKey(node,indexInNode);
				m_value = node->valueInLeaf[indexInNode]==nullptr?nullptr:(IS_NORMAL_PTR(node->valueInLeaf[indexInNode])?node->valueInLeaf[indexInNode]:nullptr);
			}
		}
	public:
		iterator(const iterator & iter)
		{
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),iter.m_tree->m_version);
			m_tree = iter.m_tree;
			m_node = iter.m_node;
			m_indexInNode = iter.m_indexInNode;
			m_key = iter.m_key;
			m_value = iter.m_value;
		}
		iterator & operator =(const iterator &iter)
		{
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),iter.m_tree->m_version);
			m_tree = iter.m_tree;
			m_node = iter.m_node;
			m_indexInNode = iter.m_indexInNode;
			m_key = iter.m_key;
			m_value = iter.m_value;
			return *this;
		}
		~iterator()
		{
			SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_tree->m_version);
		}
		bool valid()
		{
			if(m_node == nullptr||m_indexInNode>=RADIDX_NODE_SLOTS||m_node->valueInLeaf[m_indexInNode]==nullptr||!IS_NORMAL_PTR(m_node->valueInLeaf[m_indexInNode]))
				return false;
			else
				return true;
		}
		bool next()
		{
			if(m_node->end == m_indexInNode)
			{
				m_node = m_node->next;
				m_indexInNode = m_node->first;
			}
			else
			{
				void * tmp = nullptr;
				getNextValueInNode(m_node,m_indexInNode,tmp,m_indexInNode);
				if(tmp == nullptr)
				{
					m_node = m_node->next;
					m_indexInNode = m_node->first;
				}
			}
			if(valid())
			{
				m_key = m_tree->getKey(m_node,m_indexInNode);
				m_value = m_node->valueInLeaf[m_indexInNode]==nullptr?nullptr:(IS_NORMAL_PTR(m_node->valueInLeaf[m_indexInNode])?m_node->valueInLeaf[m_indexInNode]:nullptr);
				return true;
			}
			else
				return false;
		}
		bool prev()
		{
			if(m_node->first == m_indexInNode)
			{
				m_node = m_node->prev;
				m_indexInNode = m_node->end;
			}
			else
			{
				void * tmp = nullptr;
				getPrevValueInNode(m_node,m_indexInNode,tmp,m_indexInNode);
				if(tmp == nullptr)
				{
					m_node = m_node->prev;
					m_indexInNode = m_node->end;
				}
			}
			if(valid())
			{
				m_key = m_tree->getKey(m_node,m_indexInNode);
				m_value = m_node->valueInLeaf[m_indexInNode]==nullptr?nullptr:(IS_NORMAL_PTR(m_node->valueInLeaf[m_indexInNode])?m_node->valueInLeaf[m_indexInNode]:nullptr);
				return true;
			}
			else
				return false;
		}
		void * value()
		{
			return m_value;
		}
		T & key()
		{
			return m_key;
		}
	};
public:
	redixTree(int (*free_value)(void*, int), uint32_t max_node = 1024 * 1024) :
			m_free_value(free_value), m_max_node(max_node),m_typeSize(sizeof(T)*2)
	{
		c_init_chain(&m_lru);
		c_init_chain(&m_pre_lru);
		m_flag = 0;
		m_node = 0;
		m_version = 0;
	}
	int insert(const T &key, void* value)
	{
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root,*topUpdatedNode = nullptr;
		m_lock.lock();
		tryCleanExpiryNode();
		for(uint idx =0;idx<m_typeSize;idx++)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == m_typeSize-1)
				{
					m_lock.unlock();
					return -1;//key exist
				}
				else
				{
					n = n->child[v];
					continue;
				}
			}
			else
			{
				if(idx == m_typeSize-1)//leaf node
				{
					n->valueInLeaf[v] = value;
				}
				else
				{
					_node * tmp = new _node;
					tmp->parent = n;
					tmp->index = v;
					m_node ++;
					if(topUpdatedNode==nullptr)
					{
						topUpdatedNode = tmp;
					}
					else
					{
						n->child[v] = tmp;
						n->childNum ++;
						n->first = n->end = v;
					}
					n = tmp;
					continue;
				}
			}
		}
		if(topUpdatedNode != nullptr)
			insertIntoNodeFirstStep(topUpdatedNode->parent,topUpdatedNode->index);
		if(n->childNum==1)//update leaf node first
		{
			n->prev = findPrevLeafNode(n);
			n->next = findNextLeafNode(n);
			n->first = n->end = v;
		}
		else
		{
			insertIntoNodeFirstStep(n->parent,v);
		}
		/*
		 * new we have finished build all info of new node
		 * all reader can not see new nodes before now
		 * then we insert new nodes into tree
		 * */
		__asm__ __volatile__("sfence" ::: "memory");
		if(topUpdatedNode != nullptr)
		{
			insertIntoNodeSecondStep(topUpdatedNode->parent,topUpdatedNode,topUpdatedNode->index);
			m_version ++;
		}
		if(n->childNum==1)//update neighborhood of leaf node
		{
			if(n->prev)
				n->prev->next = n;
			if(n->next)
				n->next->prev = n;
		}
		else
		{
			insertIntoNodeSecondStep(n->parent,n,v);
		}
		m_lock.unlock();
		return 0;
	}
	void* erase(const T &key)
	{
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root,*topUpdatedNode = nullptr;
		m_lock.lock();
		tryCleanExpiryNode();
		for(uint idx =0;idx<m_typeSize;idx++)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == m_typeSize-1)
				{
					break;
				}
				else
				{
					assert(n->childNum>0);
					if(n->childNum == 1)
					{
						if(topUpdatedNode == nullptr&&n!=&m_root)
						{
							topUpdatedNode = n;
						}
					}
					else
					{
						if(topUpdatedNode != nullptr)
							topUpdatedNode = nullptr;
					}
					n = n->child[v];
					continue;
				}
			}
			else
			{
				m_lock.unlock();
				return nullptr;//key not exits
			}
		}
		__asm__ __volatile__("sfence" ::: "memory");
		if(topUpdatedNode!=nullptr)
		{
			eraseNodeFromParent(topUpdatedNode);
		}
		void * value = n->valueInLeaf[v];
		__asm__ __volatile__("sfence" ::: "memory");
		if(n->childNum == 1)
		{
			if(n->prev)
				n->prev->next = n->next;
			if(n->next)
				n->next->prev = n->prev;
		}
		__asm__ __volatile__("sfence" ::: "memory");
		n->valueInLeaf[v] = nullptr;
		n->childNum --;
		m_lock.unlock();
		return value;
	}
	void eraseNodeFromParent(_node * n)
	{
		uint16_t firstAdnEnd = 0;

		uint16_t nextIndex = 0;
		void * nextNode = nullptr;
		getNextValueInNode(n->parent,n->index,nextNode,nextIndex);

		uint16_t prevIndex = 0;
		void * prevNode = nullptr;
		getPrevValueInNode(n->parent,n->index,prevNode,prevIndex);

		if(nextIndex!=n->index+1&&prevIndex!=n->index-1)
		{
			n->parent->child[n->index] = nullptr;
			n->parent->childNum--;
			if(nextIndex!=0)
			{
				if(prevIndex!=0||n->parent->child[prevIndex]!=nullptr)
				{
					setNextValueInNode(n->parent,prevIndex,nextIndex);
					setPrevValueInNode(n->parent,nextIndex,prevIndex);
				}
				else
				{
					setPrevValueInNode(n->parent,nextIndex,0);
				}
			}
			else
			{
				if(prevIndex!=0||n->parent->child[prevIndex]!=nullptr)
				{
					setNextValueInNode(n->parent,prevIndex,0);
				}
			}
		}
		else
		{
			uint64_t tmp = 0;
			if(nextIndex==n->index+1&&prevIndex==n->index-1)
			{
				setNextValueInNode(n->parent,prevIndex,nextIndex);
				setPrevValueInNode(n->parent,nextIndex,prevIndex);
			}
			else if(nextIndex==n->index+1)
			{
				if(prevIndex!=0||n->parent->child[prevIndex]!=nullptr)
				{
					tmp = prevIndex<<8;
					setNextValueInNode(n->parent,prevIndex,nextIndex);
					CREATE_SPECIFIC_PTR(n->parent->valueInLeaf[n->index],tmp);
				}
				else
				{
					n->parent->valueInLeaf[n->index] = nullptr;
				}
			}
			else
			{
				if(prevIndex!=0||n->parent->child[prevIndex]!=nullptr)
				{
					if(nextIndex!=0)
					{
						tmp = nextIndex;
						setPrevValueInNode(n->parent,nextIndex,prevIndex);
						CREATE_SPECIFIC_PTR(n->parent->valueInLeaf[n->index],tmp);
					}
					else
					{
						n->parent->valueInLeaf[n->index] = nullptr;
					}
				}
				else if(nextIndex!=0)
				{
					setPrevValueInNode(n->parent,nextIndex,prevIndex);
				}
				else
				{
					n->parent->valueInLeaf[n->index] = nullptr;
				}
			}
		}

		if(n->parent->first==n->index)
		{
			firstAdnEnd = nextIndex;
			firstAdnEnd<<=8;
		}
		if(n->parent->end == n->index)
		{
			firstAdnEnd |= prevIndex;
		}
		n->parent->fisrtAndEnd = firstAdnEnd;

		n->expiryVersion = m_version++;
		m_expiryNodeQueue.push(n);
	}
	void * find(const T &key)
	{
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root;
		SET_VERSION_IN(PER_THREAD_V(m_versionLocal),m_version);
		__asm__ __volatile__("mfence" ::: "memory");
		for(uint idx =0;idx<m_typeSize;idx++)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == m_typeSize-1)
				{
					void * value = n->child[v];
					__asm__ __volatile__("mfence" ::: "memory");
					SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
					return value;
				}
				else
				{
					n = n->child[v];
					continue;
				}
			}
			else
			{
				__asm__ __volatile__("mfence" ::: "memory");
				SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
				return nullptr;//key not exits
			}
		}
		abort();//invalid
	}
	iterator find_(const T &key)
	{
		iterator iter;
		iter.m_tree = this;
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root;
		SET_VERSION_IN(PER_THREAD_V(m_versionLocal),m_version);
		__asm__ __volatile__("mfence" ::: "memory");
		for(uint idx =0;idx<m_typeSize;idx++)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == m_typeSize-1)
				{
					void * value = n->child[v];
					__asm__ __volatile__("mfence" ::: "memory");
					SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
					return value;
				}
				else
				{
					n = n->child[v];
					continue;
				}
			}
			else
			{
				__asm__ __volatile__("mfence" ::: "memory");
				SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
				return nullptr;//key not exits
			}
		}
		abort();//invalid
	}
private:

	/*first update self*/
	void insertIntoNodeFirstStep(_node* parent,uint8_t index)
	{
		uint8_t nextIndex = 0,prevIndex = 0;
		if(parent->valueInLeaf[index]!=nullptr)
		{
			assert(!IS_NORMAL_PTR(parent->valueInLeaf[index]));
			nextIndex = (uint8_t)(GET_SPECIFIC_PTR_REAL_VALUE(parent->valueInLeaf[index+1])&0xff);
			prevIndex = (uint8_t)((GET_SPECIFIC_PTR_REAL_VALUE(parent->valueInLeaf[index-1])&0xff00)>>8);
		}
		if(index!=RADIDX_NODE_SLOTS&&nextIndex==0)
		{
			for(int16_t idx = index+1;idx<RADIDX_NODE_SLOTS;idx++)
			{
				if(parent->valueInLeaf[idx]!=nullptr)
				{
					if(IS_NORMAL_PTR(parent->valueInLeaf[idx]))
					{
						nextIndex = idx;
					}
					else
					{
						prevIndex = (uint8_t)((GET_SPECIFIC_PTR_REAL_VALUE(parent->valueInLeaf[idx])&0xff00)>>8);
						assert(idx<RADIDX_NODE_SLOTS-1);
						nextIndex = idx+1;
					}
					break;
				}
			}
		}
		if(index!=0&&prevIndex==0&&parent->valueInLeaf[0]==nullptr)
		{
			for(int16_t idx = index-1;idx>=0;idx--)
			{
				if(parent->valueInLeaf[idx]!=nullptr)
				{
					if(IS_NORMAL_PTR(parent->valueInLeaf[idx]))
					{
						prevIndex = idx;
					}
					else
					{
						assert(idx>0);
						prevIndex = idx-1;
					}
					break;
				}
			}
		}
		if(nextIndex!=0)
			setNextValueInNode(parent,index,nextIndex);
		if(prevIndex!=0||parent->valueInLeaf[0]!=nullptr)
			setPrevValueInNode(parent,index,prevIndex);
		__asm__ __volatile__("mfence" ::: "memory");
		if(nextIndex!=0)
			setPrevValueInNode(parent,nextIndex,index);
		if(prevIndex!=0||parent->valueInLeaf[0]!=nullptr)
			setNextValueInNode(parent,prevIndex,index);

	}
	/*then update neighborhood and parent
	 * must have rw barrier before it*/
	void insertIntoNodeSecondStep(_node* parent,void * node,uint8_t index)
	{
		_node * n;
		uint8_t nextIndex = 0,prevIndex = 0;
		uint16_t fisrtAndEnd = 0;
		if(parent->first>index)
		{
			fisrtAndEnd = index;
			fisrtAndEnd<<=8;
		}
		if(parent->end<index)
		{
			fisrtAndEnd|=index;
		}
		parent->fisrtAndEnd = fisrtAndEnd;
		getNextValueInNode(parent,index,n,nextIndex);
		if(n!=nullptr)
			setPrevValueInNode(parent,nextIndex,index);
		getPrevValueInNode(parent,index,n,prevIndex);
		if(n!=nullptr)
			setNextValueInNode(parent,prevIndex,index);
		parent->childNum ++;
		parent->valueInLeaf[index] = node;
	}

	inline void getNextValueInNode(const _node * n,uint8_t index,void *& nextValue,uint8_t &nextIndex)
	{
		if(index == RADIDX_NODE_SLOTS-1)
		{
			nextValue = nullptr;
			return;
		}
		if(!IS_NORMAL_PTR(n->valueInLeaf[index+1]))
		{
			nextIndex = (uint8_t)(GET_SPECIFIC_PTR_REAL_VALUE(n->valueInLeaf[index+1])&0xff);
			nextValue = n->valueInLeaf[nextIndex];
			assert( nextValue!=nullptr );
			return;
		}
		else if(n->valueInLeaf[index+1] != nullptr)
		{
			nextIndex = index+1;
			nextValue = n->valueInLeaf[nextIndex];
			return;
		}
		else
		{
			nextValue = nullptr;
			return;
		}
	}
	inline void getPrevValueInNode(const _node * n,uint8_t index,void *& prevValue,uint8_t &prevIndex)
	{
		if(index == 0)
		{
			prevValue = nullptr;
			return;
		}
		if(!IS_NORMAL_PTR(n->valueInLeaf[index-1]))
		{
			prevIndex = (uint8_t)((GET_SPECIFIC_PTR_REAL_VALUE(n->valueInLeaf[index-1])&0xff00)>>8);
			assert(prevIndex<RADIDX_NODE_SLOTS);
			prevValue = n->valueInLeaf[prevIndex];
			assert( prevValue!=nullptr );
			return;
		}
		else if(n->valueInLeaf[index-1] != nullptr)
		{
			prevIndex = index-1;
			prevIndex = n->valueInLeaf[prevIndex];
			return;
		}
		else
		{
			prevValue = nullptr;
			return;
		}
	}
	inline void setNextValueInNode(_node * n,uint8_t index,uint8_t nextIndex)
	{
		if(index == RADIDX_NODE_SLOTS-1||nextIndex == index+1)
			return;
		void * next = n->valueInLeaf[index+1];
		next&= ~(void*)(0xff);
		next|=0xff&nextIndex;
		CREATE_SPECIFIC_PTR(n->valueInLeaf[index+1],next);
	}
	inline void setPrevValueInNode(_node * n,uint8_t index,uint8_t prevIndex)
	{
		if(index == 0||prevIndex == index-1)
			return;
		void * next = n->valueInLeaf[index-1];
		next&= ~(void*)(0xff00);
		next|=((0x00ff&prevIndex)<<8);
		CREATE_SPECIFIC_PTR(n->valueInLeaf[index-1],next);
	}
	void destroyNode(_node *n)
	{
		if(n->childNum>0)
		{
			uint8_t idx = n->first;
			while(true)
			{
				if(n->valueInLeaf[idx])
				{
					if(n->isLeaf)
					{
						if(m_free_value)
							m_free_value(n->valueInLeaf[idx],1);
					}
					else
					{
						destroyNode(n->child[idx]);
					}
					n->valueInLeaf[idx] = nullptr;
					n->childNum--;
				}
				if(idx!=n->end)
				{
					void * nextValue = nullptr;
					uint8_t nextIndex = 0;
					getNextValueInNode(n,idx,nextValue,nextIndex);
					idx = nextIndex;
				}
				else
				{
					break;
				}
			}
		}
		delete n;
	}
	void tryCleanExpiryNode()
	{
		for(;!m_expiryNodeQueue.empty();)
		{
			_node *n = m_expiryNodeQueue.front();
			FOR_EACH_PER_THREAD_V(iter,m_versionLocal)
			{
				if(GET_REAL_VERSION(iter)>n->expiryVersion)
				{
					continue;
				}
				else
				{
					__asm__ __volatile__("lfence" ::: "memory");
					if(IS_VERSION_OUT(iter))
					{
						__asm__ __volatile__("lfence" ::: "memory");
						if(GET_REAL_VERSION(iter)>n->expiryVersion)
							continue;
						__asm__ __volatile__("lfence" ::: "memory");
						if(IS_VERSION_OUT(iter))
							continue;
						else
							return;
					}
					else
					{
						return;
					}
				}
			}
			m_expiryNodeQueue.pop();
			destroyNode(n);
		}
	}
	_node* findNextLeafNode(_node * node)
	{
		for(_node * _tmp = node->parent;;node = _tmp,_tmp = _tmp->parent)
		{
			if(_tmp->childNum>1)
			{
				_node * next = nullptr;
				uint8_t idxTmp = 0;
				getNextValueInNode(_tmp,node->index,next,idxTmp);
				while(next != nullptr)
				{
					if(next->isLeaf)
						return next;
					assert(next->child[next->first]!=nullptr);
					next = next->child[next->first];
				}
			}
			if(_tmp == &m_root)
				break;
		}
		return nullptr;
	}
	_node* findPrevLeafNode(_node * node)
	{
		for(_node * _tmp = node->parent;;node = _tmp,_tmp = _tmp->parent)
		{
			if(_tmp->childNum>1)
			{
				_node * prev = nullptr;
				uint8_t idxTmp = 0;
				getPrevValueInNode(_tmp,node->index,prev,idxTmp);
				while(prev != nullptr)
				{
					if(prev->isLeaf)
						return prev;
					assert(prev->child[prev->end]!=nullptr);
					prev = prev->child[prev->end];
				}
			}
			if(_tmp == &m_root)
				break;
		}
		return nullptr;
	}
	T getKey(_node * node,uint8_t index)
	{
		T key = 0;
		do{
			key<<=4;
			key|=index;
			index = node->index;
		}while(node!=&m_root?(node = node->parent,true):false);
		return key;
	}
};

