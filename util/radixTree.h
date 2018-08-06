/*
 * radixTree.h
 *
 *  Created on: 2018年6月11日
 *      Author: liwei
 */

#ifndef UTIL_RADIXTREE_H_
#define UTIL_RADIXTREE_H_
#include <mutex>
#include <queue>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <type_traits>
#include "perThread.h"
#include "db_chain.h"
using namespace std;
/*
 * only support x86 platform
 * */
#define RADIX_BASE 4
#define RADIX_BASE_MASK 0x0f
#define RADIDX_NODE_SLOTS (1u<<RADIX_BASE)
#define GET_VALUE(data,idx)   (((idx)&0x01)==0?(data)[(idx)>>1]&RADIX_BASE_MASK:(data)[(idx)>>1]>>RADIX_BASE)
#if __x86_64__
#define KERNEL_MEM 0xffff800000000000lu
#define IS_NORMAL_PTR(ptr) (((uint64_t)(void*)(ptr))<KERNEL_MEM)
#define CREATE_SPECIFIC_PTR(ptr,v) (ptr)=(void*)(((uint64_t)v)|KERNEL_MEM)
#define GET_SPECIFIC_PTR_REAL_VALUE(ptr) (ptr)&(~KERNEL_MEM)
#elif __i386__
#define KERNEL_MEM 0xc0000000u
#define IS_NORMAL_PTR(ptr) (((uint32_t)(void*)(ptr))<KERNEL_MEM)
#define CREATE_SPECIFIC_PTR(ptr,v) (ptr)=(void*)(((uint32_t)v)|KERNEL_MEM)
#define GET_SPECIFIC_PTR_REAL_VALUE(ptr) (ptr)&(~KERNEL_MEM)
#endif
#define VERSION_OUT_MASK 0x80000000u
#define SET_VERSION_IN(dest,version) (dest) = ((version)|VERSION_OUT_MASK)
#define SET_VERSION_OUT(dest,version) (dest) = (version)
#define IS_VERSION_OUT(v) (!((v)&VERSION_OUT_MASK))
#define GET_REAL_VERSION(v) ((v)&(~VERSION_OUT_MASK))

#define GET_RADIX_NEXT_INDEX(v) ((uint8_t*)&(v))[0]
#define SET_RADIX_NEXT_INDEX(v,idx) assert(idx<RADIDX_NODE_SLOTS);((uint8_t*)&(v))[0] = (idx)
#define GET_RADIX_PREV_INDEX(v) ((uint8_t*)&(v))[1]
#define SET_RADIX_PREV_INDEX(v,idx) assert(idx<RADIDX_NODE_SLOTS);((uint8_t*)&(v))[1] = (idx)


template<typename T>
class radixTree
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
				uint8_t first_;
				uint8_t end_;
			};
		};
		bool isLeaf;
		_node():parent(NULL),prev(NULL),next(NULL),childNum(0),index(0),fisrtAndEnd(0),isLeaf(false)
		{
			memset(child, 0, sizeof(child));
		}
	};
	mutable PER_THREAD_T(m_versionLocal,uint64_t);
	mutable PER_THREAD_T(m_iterListLocal,chain_head);
	std::mutex m_lock;
	std::queue<_node*> m_expiryNodeQueue;
	_node m_root;
	_node * m_firstLeafNode;
	_node * m_lastLeafNode;
	int m_maxNodes;
	int m_nodes;
	int m_version;
	uint32_t m_flag;
	int (*m_free_value)(void*, int); //格式要求：清除成功必须返回0
	const uint m_typeSize;
public:
	class iterator
	{
		friend class radixTree;
	private:
		const radixTree * m_tree;
		const _node * m_node;
		uint8_t m_indexInNode;
		T m_key;
		void * m_value;
		uint64_t m_version;
		mutable chain_node m_cn;
	public:
		iterator():m_tree(nullptr),m_node(nullptr),m_indexInNode(0),m_key(0),m_value(nullptr),m_version(0){m_cn.next=m_cn.prev=nullptr;}
		iterator(const iterator & iter)
		{
			m_version = iter.m_version;
			c_insert_aft(&PER_THREAD_V(m_tree->m_iterListLocal),&iter.m_cn,&m_cn);
			if(IS_VERSION_OUT(PER_THREAD_V(m_tree->m_versionLocal)))
				SET_VERSION_IN(PER_THREAD_V(m_tree->m_versionLocal),iter.m_tree->m_version);
			m_tree = iter.m_tree;
			m_node = iter.m_node;
			m_indexInNode = iter.m_indexInNode;
			m_key = iter.m_key;
			m_value = iter.m_value;
		}
		iterator & operator =(const iterator &iter)
		{
			m_version = iter.m_version;
			c_insert_aft(&PER_THREAD_V(m_tree->m_iterListLocal),&iter.m_cn,&m_cn);
			if(IS_VERSION_OUT(PER_THREAD_V(m_tree->m_versionLocal)))
				SET_VERSION_IN(PER_THREAD_V(m_tree->m_versionLocal),iter.m_tree->m_version);
			m_tree = iter.m_tree;
			m_node = iter.m_node;
			m_indexInNode = iter.m_indexInNode;
			m_key = iter.m_key;
			m_value = iter.m_value;
			return *this;
		}
		~iterator()
		{
			if(m_tree)
			{
				if(c_is_head(&PER_THREAD_V(m_tree->m_iterListLocal),&m_cn))
				{
					if(PER_THREAD_V(m_tree->m_iterListLocal).count>1)
					{
						iterator * next = get_next_dt(this,iterator,m_cn);
						SET_VERSION_IN(PER_THREAD_V(m_tree->m_versionLocal),next->m_version);
					}
					else
					{
						SET_VERSION_OUT(PER_THREAD_V(m_tree->m_versionLocal),m_tree->m_version);
					}
				}
				c_delete_node(&PER_THREAD_V(m_tree->m_iterListLocal),&m_cn);
			}
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
			if(m_node->end_ == m_indexInNode)
			{
				m_node = m_node->next;
				if(m_node)
					m_indexInNode = m_node->first_;
			}
			else
			{
				void * tmp = nullptr;
				m_tree->getNextValueInNode(m_node,m_indexInNode,tmp,m_indexInNode);
				if(tmp == nullptr)
				{
					m_node = m_node->next;
					if(m_node)
						m_indexInNode = m_node->first_;
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
			if(m_node->first_ == m_indexInNode)
			{
				m_node = m_node->prev;
				if(m_node)
					m_indexInNode = m_node->end_;
			}
			else
			{
				void * tmp = nullptr;
				getPrevValueInNode(m_node,m_indexInNode,tmp,m_indexInNode);
				if(tmp == nullptr)
				{
					m_node = m_node->prev;
					if(m_node)
						m_indexInNode = m_node->end_;
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
	radixTree(int (*free_value)(void*, int), uint32_t maxNodes = 1024 * 1024) :
		m_firstLeafNode(nullptr),m_lastLeafNode(nullptr),m_maxNodes(maxNodes),m_nodes(0),m_version(0),m_flag(0),m_free_value(free_value),m_typeSize(sizeof(T)*2)
	{
		memset(m_versionLocal,0,sizeof(m_versionLocal));
		for(int idx=0;idx<sizeof(m_iterListLocal)/sizeof(chain_head);idx++)
		{
			c_init_chain(&m_iterListLocal[idx]);
		}
	}
	~radixTree()
	{
		clear();
	}
	void clear()
	{
		m_lock.lock();
		destroyNode(&m_root);
		m_firstLeafNode = m_lastLeafNode = nullptr;
		m_lock.unlock();
	}
	int insert(const T &key, void* value)
	{
		if(value == nullptr)
			return 0;
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root,*topUpdatedNode = nullptr;
		m_lock.lock();
		tryCleanExpiryNode();
		for(int idx =m_typeSize-1;;idx--)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == 0)
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
				if(idx == 0)//leaf node
				{
					break;
				}
				else
				{
					if(m_nodes+idx>=m_maxNodes)
					{
						if(topUpdatedNode)
							destroyNode(topUpdatedNode);
						m_lock.unlock();
						return -2;//mem over limit
					}
					_node * tmp = new _node;
					tmp->parent = n;
					tmp->index = v;
					m_nodes ++;
					if(topUpdatedNode==nullptr)
					{
						topUpdatedNode = tmp;
					}
					else
					{
						n->child[v] = tmp;
						n->childNum ++;
						n->first_ = n->end_ = v;
					}
					n = tmp;
					continue;
				}
			}
		}
		if(topUpdatedNode != nullptr)
			insertIntoNodeFirstStep(topUpdatedNode->parent,topUpdatedNode->index);
		if(n->childNum==0)//update leaf node first
		{
			n->isLeaf = true;
			n->prev = findPrevLeafNode(n);
			n->next = findNextLeafNode(n);
			assert(n->next!=n);
			assert(n->prev!=n);
			n->first_ = n->end_ = v;
			n->valueInLeaf[v] = value;
		}
		else
		{
			insertIntoNodeFirstStep(n,v);
		}
		/*
		 * new we have finished build all info of new node
		 * all reader can not see new nodes before now
		 * then we insert new nodes into tree
		 * */
		__asm__ __volatile__("sfence" ::: "memory");
		if(n->childNum==0)//update neighborhood of leaf node
		{
			n->childNum++;
			if(n->prev)
				n->prev->next = n;
			if(n->next)
				n->next->prev = n;
			if(m_firstLeafNode==nullptr||m_firstLeafNode==n->next)
			{
				assert(n->prev == nullptr);
				if(m_firstLeafNode)
					assert(getKey(m_firstLeafNode,m_firstLeafNode->first_)>key);
				m_firstLeafNode = n;
			}
			else
				assert(getKey(m_firstLeafNode,m_firstLeafNode->first_)<key);
			if(m_lastLeafNode==nullptr||m_lastLeafNode==n->prev)
			{
				assert(n->next == nullptr);
				if(m_lastLeafNode)
					assert(getKey(m_lastLeafNode,m_firstLeafNode->end_)<key);
				m_lastLeafNode = n;
			}
			else
				assert(getKey(m_lastLeafNode,m_firstLeafNode->end_)>key);
		}
		else
		{
			insertIntoNodeSecondStep(n,value,v);
		}

		if(topUpdatedNode != nullptr)
		{
			insertIntoNodeSecondStep(topUpdatedNode->parent,topUpdatedNode,topUpdatedNode->index);
			m_version ++;
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
		for(int idx =m_typeSize-1;idx>=0;idx--)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == 0)
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
		if(n->childNum == 1)
		{
			if(m_firstLeafNode == n)
				m_firstLeafNode = n->next;
			if(m_lastLeafNode == n)
				m_lastLeafNode = n->prev;
		}
		void * value = n->valueInLeaf[v];
		__asm__ __volatile__("sfence" ::: "memory");
		if(n->childNum == 1)
		{
			if(n->prev)
				n->prev->next = n->next;
			if(n->next)
				n->next->prev = n->prev;
			if(m_firstLeafNode==n)
				m_firstLeafNode = n->next;
			if(m_lastLeafNode==n)
				m_lastLeafNode = n->prev;
		}
		__asm__ __volatile__("sfence" ::: "memory");
		n->valueInLeaf[v] = nullptr;
		n->childNum --;
		m_lock.unlock();
		return value;
	}
	void eraseNodeFromParent(_node * n)
	{
		uint16_t fisrtAndEnd = n->parent->fisrtAndEnd;

		uint8_t nextIndex = 0;
		void * nextNode = nullptr;
		getNextValueInNode(n->parent,n->index,nextNode,nextIndex);

		uint8_t prevIndex = 0;
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

		if(n->parent->first_==n->index)
		{
			((uint8_t*)&fisrtAndEnd)[0]=prevIndex;
		}
		if(n->parent->end_ == n->index)
		{
			((uint8_t*)&fisrtAndEnd)[1]=nextIndex;
		}
		n->parent->fisrtAndEnd = fisrtAndEnd;

		n->expiryVersion = m_version++;
		m_expiryNodeQueue.push(n);
	}
	void * findValue(const T key)
	{
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root;
		bool versionOut = false;
		if(IS_VERSION_OUT(PER_THREAD_V(m_versionLocal)))//maybe some iterator of this thread owner m_versionLocal
		{
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),m_version);
			versionOut = true;
		}
		__asm__ __volatile__("mfence" ::: "memory");
		for(uint idx =m_typeSize-1;idx>=0;idx--)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == 0)
				{
					void * value = n->child[v];
					__asm__ __volatile__("mfence" ::: "memory");
					if(versionOut)
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
				if(versionOut)
					SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
				return nullptr;//key not exits
			}
		}
		abort();//invalid
	}
	iterator begin()
	{
		iterator iter;
		iter.m_tree = this;
		iter.m_version = PER_THREAD_V(m_versionLocal);
		bool versionOut = false;
		if(IS_VERSION_OUT(PER_THREAD_V(m_versionLocal)))
		{
			versionOut = true;
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),m_version);
		}
		__asm__ __volatile__("mfence" ::: "memory");
		if((iter.m_node=m_firstLeafNode) !=nullptr)
		{
			iter.m_indexInNode = iter.m_node->first_;
			iter.m_key = getKey(iter.m_node,iter.m_indexInNode);
			iter.m_value = iter.m_node->valueInLeaf[iter.m_indexInNode];
			c_insert_in_end(&PER_THREAD_V(m_iterListLocal),&iter.m_cn);
			return iter;
		}
		if(versionOut)
			SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
		iter.m_tree = nullptr;
		return iter;//key not exits
	}
	iterator rbegin()
	{
		iterator iter;
		iter.m_tree = this;
		iter.m_version = PER_THREAD_V(m_versionLocal);
		bool versionOut = false;
		if(IS_VERSION_OUT(PER_THREAD_V(m_versionLocal)))
		{
			versionOut = true;
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),m_version);
		}
		__asm__ __volatile__("mfence" ::: "memory");
		if((iter.m_node=m_lastLeafNode) !=nullptr)
		{
			iter.m_indexInNode = iter.m_node->end_;
			iter.m_key = getKey(iter.m_node,iter.m_indexInNode);
			iter.m_value = iter.m_node->valueInLeaf[iter.m_indexInNode];
			c_insert_in_end(&PER_THREAD_V(m_iterListLocal),&iter.m_cn);
			return iter;
		}
		if(versionOut)
			SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
		iter.m_tree = nullptr;
		return iter;//key not exits
	}

	iterator find(const T key)
	{
		iterator iter;
		iter.m_tree = this;
		iter.m_version = PER_THREAD_V(m_versionLocal);
		const uint8_t * _key = (const uint8_t *)&key;
		uint8_t v;
		_node *n = &m_root;
		bool versionOut = false;
		if(IS_VERSION_OUT(PER_THREAD_V(m_versionLocal)))
		{
			versionOut = true;
			SET_VERSION_IN(PER_THREAD_V(m_versionLocal),m_version);
		}
		__asm__ __volatile__("mfence" ::: "memory");
		for(uint idx =m_typeSize-1;idx>=0;idx--)
		{
			v = GET_VALUE(_key,idx);
			if(n->child[v] != nullptr&&IS_NORMAL_PTR(n->child[v]))
			{
				if(idx == 0)
				{
					iter.m_key = getKey(n,v);
					iter.m_value = n->valueInLeaf[v];
					iter.m_indexInNode = v;
					iter.m_node = n;
					c_insert_in_end(&PER_THREAD_V(m_iterListLocal),&iter.m_cn);
					return iter;
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
				if(versionOut)
					SET_VERSION_OUT(PER_THREAD_V(m_versionLocal),m_version);
				iter.m_tree = nullptr;
				return iter;//key not exits
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
			nextIndex = GET_RADIX_NEXT_INDEX(parent->valueInLeaf[index]);
			prevIndex = GET_RADIX_PREV_INDEX(parent->valueInLeaf[index]);
		}
		if(index!=RADIDX_NODE_SLOTS-1&&nextIndex==0)
		{
			for(int16_t idx = index+1;idx<RADIDX_NODE_SLOTS;idx++)
			{
				if(parent->valueInLeaf[idx]!=nullptr)
				{
					if(IS_NORMAL_PTR(parent->valueInLeaf[idx]))
					{
						nextIndex = idx;
						break;
					}
					else
					{
						if(GET_RADIX_NEXT_INDEX(parent->valueInLeaf[idx])!=0)
						{
							nextIndex = GET_RADIX_NEXT_INDEX(parent->valueInLeaf[idx]);
							break;
						}
					}
				}
			}
		}
		if(index!=0&&prevIndex==0)
		{
			for(int16_t idx = index-1;idx>=0;idx--)
			{
				if(parent->valueInLeaf[idx]!=nullptr)
				{
					if(IS_NORMAL_PTR(parent->valueInLeaf[idx]))
					{
						prevIndex = idx;
						break;
					}
					else
					{
						if(GET_RADIX_PREV_INDEX(parent->valueInLeaf[idx])!=0)
						{
							prevIndex = GET_RADIX_PREV_INDEX(parent->valueInLeaf[idx]);
							break;
						}
					}
				}
			}
		}
		if(nextIndex!=0)
			setNextValueInNode(parent,index,nextIndex);
		if(prevIndex!=0||parent->valueInLeaf[0]!=nullptr)
			setPrevValueInNode(parent,index,prevIndex);
	}
	/*then update neighborhood and parent
	 * must have rw barrier before it*/
	void insertIntoNodeSecondStep(_node * parent,void * node,uint8_t index)
	{
		parent->valueInLeaf[index] = node;
		parent->childNum ++;
		__asm__ __volatile__("sfence" ::: "memory");
		void * n;
		uint8_t nextIndex = 0,prevIndex = 0;
		uint16_t fisrtAndEnd = parent->fisrtAndEnd;
		if(parent->first_ > index)
		{
			((uint8_t*)&fisrtAndEnd)[0]=index;
		}
		if(parent->end_ < index)
		{
			((uint8_t*)&fisrtAndEnd)[1]=index;
		}
		parent->fisrtAndEnd = fisrtAndEnd;
		getNextValueInNode(parent,index,n,nextIndex);
		if(n!=nullptr)
			setPrevValueInNode(parent,nextIndex,index);
		getPrevValueInNode(parent,index,n,prevIndex);
		if(n!=nullptr)
			setNextValueInNode(parent,prevIndex,index);
	}

	inline void getNextValueInNode(const _node * n,uint8_t index,void *& nextValue,uint8_t &nextIndex) const
	{

		for(int i=index+1;i<RADIDX_NODE_SLOTS;i++)
			if(n->valueInLeaf[i]!=nullptr&&IS_NORMAL_PTR(n->valueInLeaf[i]))
			{
				nextIndex = i;
				break;
			}

		if(index == RADIDX_NODE_SLOTS-1)
		{
			nextValue = nullptr;
			return;
		}
		if(!IS_NORMAL_PTR(n->valueInLeaf[index+1]))
		{
			assert(nextIndex == GET_RADIX_NEXT_INDEX(n->valueInLeaf[index+1]));
			nextIndex = GET_RADIX_NEXT_INDEX(n->valueInLeaf[index+1]);
			assert(nextIndex!=0);
			nextValue = n->valueInLeaf[nextIndex];
			assert( nextValue!=nullptr );
			return;
		}
		else if(n->valueInLeaf[index+1] != nullptr)
		{
			assert(nextIndex==index+1);
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
	inline void getPrevValueInNode(const _node * n,uint8_t index,void *& prevValue,uint8_t &prevIndex) const
	{
		for(int i=index-1;i>=0;i--)
			if(n->valueInLeaf[i]!=nullptr&&IS_NORMAL_PTR(n->valueInLeaf[i]))
			{
				prevIndex = i;
				break;
			}
		if(index == 0)
		{
			prevValue = nullptr;
			return;
		}
		if(!IS_NORMAL_PTR(n->valueInLeaf[index-1]))
		{
			assert(prevIndex == GET_RADIX_PREV_INDEX(n->valueInLeaf[index-1]));
			prevIndex = GET_RADIX_PREV_INDEX(n->valueInLeaf[index-1]);
			assert(prevIndex<RADIDX_NODE_SLOTS);
			prevValue = n->valueInLeaf[prevIndex];
			assert( prevValue!=nullptr );
			return;
		}
		else if(n->valueInLeaf[index-1] != nullptr)
		{
			assert(prevIndex==index-1);
			prevIndex = index-1;
			prevValue = n->valueInLeaf[prevIndex];
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
		if(index == RADIDX_NODE_SLOTS-1||nextIndex == index+1||(IS_NORMAL_PTR(n->valueInLeaf[index+1])&&n->valueInLeaf[index+1]!=nullptr))
			return;
		void * next = n->valueInLeaf[index+1];
		SET_RADIX_NEXT_INDEX(next,nextIndex);
		CREATE_SPECIFIC_PTR(n->valueInLeaf[index+1],next);
	}
	inline void setPrevValueInNode(_node * n,uint8_t index,uint8_t prevIndex)
	{
		if(index == 0||prevIndex == index-1||(IS_NORMAL_PTR(n->valueInLeaf[index-1])&&n->valueInLeaf[index-1]!=nullptr))
			return;
		void * next = n->valueInLeaf[index-1];
		SET_RADIX_PREV_INDEX(next,prevIndex);
		CREATE_SPECIFIC_PTR(n->valueInLeaf[index-1],next);
	}
	void destroyNode(_node *n)
	{
		if(n->childNum>0)
		{
			uint8_t idx = n->first_;
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
				if(idx!=n->end_)
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
		if(n!=&m_root)
		{
			m_nodes--;
			delete n;
		}
		else
		{
			memset(n->child,0,sizeof(n->child));
			n->fisrtAndEnd = 0;
			n->childNum = 0;
			n->index = 0;
		}
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
	_node* findNextLeafNode(const _node * node) const
	{
		_node* next = nullptr;
		for(_node * parent = node->parent;;node = parent,parent = parent->parent)
		{
			if(parent->childNum>0)
			{
				for(int16_t idx = node->index+1;idx<RADIDX_NODE_SLOTS;idx++)
				{
					if(parent->valueInLeaf[idx]!=nullptr)
					{
						if(IS_NORMAL_PTR(parent->valueInLeaf[idx]))
						{
							next = parent->child[idx];
							break;
						}
						else
						{
							if(GET_RADIX_NEXT_INDEX(parent->valueInLeaf[idx])!=0)
							{
								next = parent->child[GET_RADIX_NEXT_INDEX(parent->valueInLeaf[idx])];
								break;
							}
						}
					}
				}
				while(next != nullptr)
				{
					if(next->isLeaf)
						return next;
					assert(next->child[next->first_]!=nullptr);
					next = next->child[next->first_];
				}
			}
			if(parent == &m_root)
				break;
		}
		return nullptr;
	}
	_node* findPrevLeafNode(const _node * node) const
	{
		_node* prev = nullptr;
		for(_node * parent = node->parent;;node = parent,parent = parent->parent)
		{
			if(parent->childNum>0)
			{
				for(int16_t idx = node->index-1;idx>=0;idx--)
				{
					if(parent->valueInLeaf[idx]!=nullptr)
					{
						if(IS_NORMAL_PTR(parent->valueInLeaf[idx]))
						{
							prev = parent->child[idx];
							break;
						}
						else
						{
							if(GET_RADIX_PREV_INDEX(parent->valueInLeaf[idx])!=0||parent->valueInLeaf[0]!=nullptr)
							{
								prev = parent->child[GET_RADIX_PREV_INDEX(parent->valueInLeaf[idx])];
								break;
							}
						}
					}
				}
				while(prev != nullptr)
				{
					if(prev->isLeaf)
						return prev;
					assert(prev->child[prev->end_]!=nullptr);
					prev = prev->child[prev->end_];
				}
			}
			if(parent == &m_root)
				break;
		}
		return nullptr;
	}
	T getKey(const _node * node,uint8_t index) const
	{
		uint8_t key[sizeof(T)] = {0};
		for(uint8_t idx=0;idx<m_typeSize;idx++)
		{
			if(idx&0x01)
				key[idx>>1] |= (index<<RADIX_BASE);
			else
				key[idx>>1] = index;
			index = node->index;
			node = node->parent;
		}
		return *(T*)&key[0];
	}
};
#endif /* UTIL_RADIXTREE_H_ */
