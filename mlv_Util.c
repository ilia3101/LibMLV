#include "libmlv.h"

typedef struct
{
    union {
        struct {
            mlv_Alloc allocator;
            void * ud;
            uint64_t size;
        };
        uint8_t enforce_size[32];
    };
}
mlv_AllocationInfo;

void * mlv_Malloc(mlv_Alloc Allocator, void * AllocatorUD, uint64_t Size)
{
    mlv_AllocationInfo * info = Allocator(AllocatorUD, NULL, 0, Size + sizeof(mlv_AllocationInfo));
    info->allocator = Allocator;
    info->ud = AllocatorUD;
    return info + 1;
}

void * mlv_Malloc2(void * UseAllocatorFrom, uint64_t Size)
{
    mlv_AllocationInfo * info = ((mlv_AllocationInfo *)UseAllocatorFrom) - 1;
    return mlv_Malloc(info->allocator, info->ud, Size);
}

void mlv_Free(void * Pointer)
{
    mlv_AllocationInfo * info = ((mlv_AllocationInfo *)Pointer) - 1;
    info->allocator(info->ud, info, info->size, 0);
}

void * mlv_Realloc(void * Pointer, uint64_t NewSize)
{
    mlv_AllocationInfo * info = ((mlv_AllocationInfo *)Pointer) - 1;
    info = info->allocator(info->ud, info, info->size, NewSize+sizeof(mlv_AllocationInfo));
    info->size = NewSize;
    return info + 1;
}