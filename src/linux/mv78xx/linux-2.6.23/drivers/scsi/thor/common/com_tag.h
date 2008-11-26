#if !defined(COM_TAG_H)
#define COM_TAG_H

typedef struct _Tag_Stack Tag_Stack, *PTag_Stack;

#ifdef _OS_BIOS
#define MAX_TAG_NUMBER	1
#else
#define MAX_TAG_NUMBER	32
#endif

//TBD: I suppose the stack size is always the same. If not, improve.
struct _Tag_Stack
{
    MV_U8   Stack[MAX_TAG_NUMBER];
    MV_U8   Top;
};

MV_U8 Tag_GetOne(PTag_Stack pTagStack);
MV_VOID Tag_ReleaseOne(PTag_Stack pTagStack, MV_U8 tag);
MV_VOID Tag_Init(PTag_Stack pTagStack, MV_U8 size);
MV_BOOLEAN Tag_IsEmpty(PTag_Stack pTagStack);

#endif

