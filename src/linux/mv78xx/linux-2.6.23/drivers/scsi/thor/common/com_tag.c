#include "mv_include.h"

#include "com_tag.h"

MV_VOID Tag_Init( PTag_Stack pTagStack, MV_U8 size )
{
	MV_U8 i;
	
	MV_DASSERT( size <= MAX_TAG_NUMBER );
#ifdef _OS_LINUX
	if ( pTagStack->Top && pTagStack->Top < size )
		MV_DBG(DMSG_CORE, "__MV__ Init an in-use tag pool "
		       "curr_size:init_size - %d:%d.\n", pTagStack->Top, size);
#endif /* _OS_LINUX */

	pTagStack->Top = size;
	for ( i=0; i<size; i++ )
	{
		pTagStack->Stack[i] = size-1-i;
	}
}

MV_U8 Tag_GetOne(PTag_Stack pTagStack)
{
	MV_DASSERT( pTagStack->Top>0 );
	return pTagStack->Stack[--pTagStack->Top];
}

MV_VOID Tag_ReleaseOne(PTag_Stack pTagStack, MV_U8 tag)
{
	MV_DASSERT( pTagStack->Top<MAX_TAG_NUMBER );
	pTagStack->Stack[pTagStack->Top++] = tag;
}

MV_BOOLEAN Tag_IsEmpty(PTag_Stack pTagStack)
{
	if ( pTagStack->Top==0 )
	{
	    return MV_TRUE;
	}
	return MV_FALSE;
}

