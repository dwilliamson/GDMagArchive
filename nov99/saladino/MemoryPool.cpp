#include <stdlib.h>

// Classes
class MemoryPool {
public:
	MemoryPool *next;

	int max_num_elements;
	int size_of_elements_in_bytes;

	int num_elements;
	unsigned char *data;

private:
	void BuildDataArray();

public:
	void *AddElement();

	void Empty();

	void SetNumberOfElements(int num_elements);
	void SetSizeOfElementInBytes(int num_bytes);

	MemoryPool();
	~MemoryPool();
};

class MemoryPoolSentry {
private:
	MemoryPool *pools;

	int max_num_elements;
	int size_of_elements_in_bytes;

	int auto_expansion;

	#ifdef _DEBUG
	int num_allocated_elements;
	int num_used_elements;

	int peak_allocated_elements;
	int peak_used_elements;
	#endif

public:
	void *AddElement();

	void EmptyAllPools();

	void BuildNewPools(int num_new_pools);
	void DeleteUnusedPools();
	void DeleteAllPools();

	void SetNumberOfElements(int num_elements);
	void SetSizeOfElementInBytes(int num_bytes);

	void AutoExpansion(int new_auto_expansion);

	MemoryPoolSentry();
	~MemoryPoolSentry();
};

// Functions
void MemoryPool::BuildDataArray()
{
	if(max_num_elements<=0) {
		return;
	}

	if(size_of_elements_in_bytes<=0) {
		return;
	}

	if(data) {
		delete []data;
	}

	data=new unsigned char[max_num_elements*size_of_elements_in_bytes];
}

void *MemoryPool::AddElement()
{
	if(num_elements==max_num_elements) {
		// No soup for you
		return NULL;
	}

	num_elements++;

	return (void *)(data+(num_elements-1)*size_of_elements_in_bytes);
}

void MemoryPool::Empty()
{
	num_elements=0;
}

void MemoryPool::SetNumberOfElements(int num_elements)
{
	max_num_elements=num_elements;

	BuildDataArray();
}

void MemoryPool::SetSizeOfElementInBytes(int num_bytes)
{
	size_of_elements_in_bytes=num_bytes;

	BuildDataArray();
}

MemoryPool::MemoryPool()
{
	next=NULL;

	max_num_elements=0;
	size_of_elements_in_bytes=0;

	data=NULL;
	num_elements=0;
}

MemoryPool::~MemoryPool()
{
	if(data) {
		delete []data;
	}
}

// Sentry
void *MemoryPoolSentry::AddElement()
{
	MemoryPool *prev_pool=NULL;
	MemoryPool *cur_pool=pools;
	void *new_element=NULL;
	while(cur_pool&&(new_element==NULL)) {
		new_element=cur_pool->AddElement();

		prev_pool=cur_pool;
		cur_pool=cur_pool->next;
	}

	if((new_element==NULL)&&(auto_expansion)) {
		// Create a new pool and add there
		MemoryPool *new_pool=new MemoryPool;
		new_pool->SetNumberOfElements(max_num_elements);
		new_pool->SetSizeOfElementInBytes(size_of_elements_in_bytes);
		if(prev_pool) {
			prev_pool->next=new_pool;
		} else {
			pools=new_pool;
		}

		#ifdef _DEBUG
		num_allocated_elements+=max_num_elements;
		if(num_allocated_elements>peak_allocated_elements) {
			peak_allocated_elements=num_allocated_elements;
		}
		#endif

		new_element=new_pool->AddElement();
	}

	#ifdef _DEBUG
	if(new_element) {
		num_used_elements++;
		if(num_used_elements>peak_used_elements) {
			peak_used_elements=num_used_elements;
		}
	}
	#endif

	return new_element;
}

void MemoryPoolSentry::EmptyAllPools()
{
	MemoryPool *cur_pool=pools;
	while(cur_pool) {
		cur_pool->Empty();

		cur_pool=cur_pool->next;
	}

	#ifdef _DEBUG
	num_used_elements=0;
	#endif
}

void MemoryPoolSentry::BuildNewPools(int num_new_pools)
{
	#ifdef _DEBUG
	if(num_new_pools<=0) {
		return;
	}
	#endif

	MemoryPool *prev_pool=NULL;
	MemoryPool *cur_pool=pools;
	while(cur_pool) {
		prev_pool=cur_pool;
		cur_pool=cur_pool->next;
	}

	cur_pool=prev_pool;
	for(; num_new_pools>0; --num_new_pools) {
		MemoryPool *new_pool=new MemoryPool;
		new_pool->SetNumberOfElements(max_num_elements);
		new_pool->SetSizeOfElementInBytes(size_of_elements_in_bytes);
		cur_pool->next=new_pool;

		#ifdef _DEBUG
		num_allocated_elements+=max_num_elements;
		if(num_allocated_elements>peak_allocated_elements) {
			peak_allocated_elements=num_allocated_elements;
		}
		#endif

		cur_pool=new_pool;
	}
}

void MemoryPoolSentry::DeleteUnusedPools()
{
	MemoryPool *cur_pool=pools;
	while(cur_pool) {
		MemoryPool *next=cur_pool->next;
		if(cur_pool->num_elements==0) {
			#ifdef _DEBUG
			num_allocated_elements-=max_num_elements;
			#endif

			delete cur_pool;
		}

		cur_pool=next;
	}
}

void MemoryPoolSentry::DeleteAllPools()
{
	while(pools) {
		MemoryPool *next=pools->next;
		delete pools;

		pools=next;
	}

	#ifdef _DEBUG
	num_allocated_elements=0;
	num_used_elements=0;
	#endif
}

void MemoryPoolSentry::SetNumberOfElements(int num_elements)
{
	max_num_elements=num_elements;
}

void MemoryPoolSentry::SetSizeOfElementInBytes(int num_bytes)
{
	size_of_elements_in_bytes=num_bytes;
}

void MemoryPoolSentry::AutoExpansion(int new_auto_expansion)
{
	auto_expansion=new_auto_expansion;
}

MemoryPoolSentry::MemoryPoolSentry()
{
	pools=NULL;

	max_num_elements=0;
	size_of_elements_in_bytes=0;

	auto_expansion=0;

	#ifdef _DEBUG
	num_allocated_elements=0;
	num_used_elements=0;

	peak_allocated_elements=0;
	peak_used_elements=0;
	#endif
}

MemoryPoolSentry::~MemoryPoolSentry()
{
	DeleteAllPools();
}
