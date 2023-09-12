//------------------------------------------------------------------------------
//  memory.cc
//  (C) 2020 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------

#include "memory.h"
#include "math/scalar.h"
namespace CoreGraphics
{

Util::Array<MemoryPool> Pools;
Threading::CriticalSection AllocationLock;

//------------------------------------------------------------------------------
/**
*/
Alloc
MemoryPool::AllocateMemory(uint alignment, uint size)
{
    return this->Allocate(alignment, size);
}

//------------------------------------------------------------------------------
/**
    Deallocates memory, if a GPU block was freed this function returns true
*/
bool 
MemoryPool::DeallocateMemory(const Alloc& alloc)
{
    Memory::SCAllocator& allocator = this->allocators[alloc.blockIndex];
    allocator.Dealloc(Memory::SCAlloc{ (uint)alloc.offset, alloc.nodeIndex });
    if (allocator.Empty())
    {
        this->DestroyBlock(this->blocks[alloc.blockIndex]);
        this->blockPool.Dealloc(alloc.blockIndex);
        this->blocks[alloc.blockIndex] = DeviceMemory(0);
        this->blockMappedPointers[alloc.blockIndex] = nullptr;
    }

    return true;
}

//------------------------------------------------------------------------------
/**
*/
void 
MemoryPool::Clear()
{
    for (IndexT i = 0; i < this->blocks.Size(); i++)
        if (this->blocks[i] != DeviceMemory(0))
            this->DestroyBlock(this->blocks[i]);

    this->blocks.Clear();
    this->allocators.Clear();
}

//------------------------------------------------------------------------------
/**
*/
void* 
MemoryPool::GetMappedMemory(const Alloc& alloc)
{
    return (char*)this->blockMappedPointers[alloc.blockIndex] + alloc.offset;
}

//------------------------------------------------------------------------------
/**
*/
Alloc
MemoryPool::AllocateExclusiveBlock(DeviceSize alignment, DeviceSize size)
{
     // store old block size and reset it after block allocation
    uint oldSize = this->blockSize;
    this->blockSize = size;

    uint id = this->blockPool.Alloc();
    if (id >= (uint)this->blockMappedPointers.Size())
    {
        this->blockMappedPointers.Append(nullptr);
        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);

        this->blocks.Append(mem);
        this->allocators.Append(Memory::SCAllocator{(SizeT)size, 1});
    }
    else
    {
        this->blockMappedPointers[id] = nullptr;
        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);

        this->blocks[id] = mem;
        this->allocators[id] = Memory::SCAllocator{ (SizeT)size, 1 };
    }
    this->blockSize = oldSize;
    Memory::SCAlloc alloc = this->allocators[id].Alloc(size);

    n_warning("Allocation of size %d is bigger than block size %d will receive a dedicated memory block\n", size, this->blockSize);
    Alloc ret{ this->blocks[id], alloc.offset, size, alloc.node, this->memoryType, id };
    return ret;
}

//------------------------------------------------------------------------------
/**
*/
Alloc
MemoryPool::Allocate(DeviceSize alignment, DeviceSize size)
{
    // if size is too big, allocate a unique block for it
    if (size > this->blockSize)
        return this->AllocateExclusiveBlock(alignment, size);

    for (IndexT blockIndex = 0; blockIndex < this->blocks.Size(); blockIndex++)
    {
        // if the block has been dealloced, use it
        if (this->blocks[blockIndex] == DeviceMemory(0))
            break;

        Memory::SCAlloc alloc = this->allocators[blockIndex].Alloc(size, alignment);

        // If block can't fit our memory, go to next block
        if (alloc.offset == Memory::SCAlloc::OOM)
            continue;
     
        Alloc ret{ this->blocks[blockIndex], alloc.offset, size, alloc.node, this->memoryType, (uint)blockIndex };
        return ret;
    }

    n_assert(this->size + this->blockSize <= this->maxSize);
    this->size += this->blockSize;

    // No existing block could fit our allocation, so create new block
    uint id = this->blockPool.Alloc();
    if (id >= (uint)this->blockMappedPointers.Size())
    {
        // Block is completely new, make space in our arrays for the data
        this->blockMappedPointers.Append(nullptr);
        this->allocators.Append(Memory::SCAllocator{(SizeT)this->blockSize, (SizeT)(this->blockSize / 16)});

        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);
        this->blocks.Append(mem);
    }
    else
    {
        DeviceMemory mem = this->CreateBlock(&this->blockMappedPointers[id]);

        this->allocators[id].Clear();
        this->blocks[id] = mem;
    }
    Memory::SCAlloc alloc = this->allocators[id].Alloc(size);
    Alloc ret{ this->blocks[id], alloc.offset, size, alloc.node, this->memoryType, id };
    return ret;
}

} // namespace CoreGraphics
