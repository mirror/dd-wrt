#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <broadcom.h>

/*
 * Example data = $NAME:111$STAT:222 find_pattern(data, strlen(data),
 * "$NAME:", sizeof("$NAME:")-1, '$', &offset, &len); produces : ret = 1,
 * offset=6, len=3, buf = 111 find_pattern(data, strlen(data), "$IP:",
 * sizeof("$IP:")-1, '$', &offset, &len); produces : ret = 0 
 */
/*
 * Return 1 for match, 0 for accept, -1 for partial. 
 */
int
find_pattern( const char *data, size_t dlen,
	      const char *pattern, size_t plen,
	      char term, unsigned int *numoff, unsigned int *numlen )
{
    size_t i, j, k;

    // DEBUGP("find_pattern `%s': dlen = %u\n", pattern, dlen);
    if( dlen == 0 )
	return 0;

    if( dlen <= plen )
    {
	/*
	 * Short packet: try for partial? 
	 */
	if( strncmp( data, pattern, dlen ) == 0 )
	    return -1;
	else
	    return 0;
    }

    for( i = 0; i <= ( dlen - plen ); i++ )
    {
	if( memcmp( data + i, pattern, plen ) != 0 )
	    continue;

	/*
	 * patten match !! 
	 */
	*numoff = i + plen;
	for( j = *numoff, k = 0; data[j] != term; j++, k++ )
	    if( j > dlen )
		return -1;	/* no terminal char */

	*numlen = k;
	return 1;
    }

    return 0;
}

/*
 * Example data = $NAME:111$STAT:222 find_match_pattern(name, strlen(name),
 * data, "$NAME", ""); produces : ret = 1, name = 111 find_match_pattern(ip, 
 * strlen(ip), data, "$IP", "0.0.0.0"); produces : ret = 0, ip = 0.0.0.0 
 */
int
find_match_pattern( char *name, size_t mlen,
		    const char *data, const char *pattern, char *def )
{
    int ret = 0;
    unsigned int offset, len, length;

    ret =
	find_pattern( data, strlen( data ), pattern, strlen( pattern ), '$',
		      &offset, &len );
    // printf("ret=[%d] offset=[%d] len=[%d]\n", ret, offset,len);

    if( ret == 1 && len > 0 )
    {
	length = len > mlen ? mlen : len;
	strncpy( name, data + offset, length );
	name[length] = '\0';
    }
    else
	snprintf( name, mlen + 1, "%s", def );	// not found and set to
						// default value

    return ret;
}

/*
 * Example data = www.kimo.com.tw<&nbsp;>www.google<&nbsp;> find_each(name,
 * sizeof(name), data, "<&nbsp;>", 0, ""); produces : ret = 1, name =
 * "www.kimo.com.tw" find_each(name, sizeof(name), data, "<&nbsp;>", 1, "");
 * produces : ret = 1, name = "google.com" find_each(name, sizeof(name),
 * data, "<&nbsp;>", 2, "No find!"); produces : ret = 0, name = "No Find!" 
 */
int
find_each( char *name, int len, char *data, char *token, int which,
	   char *def )
{
    int i;
    int maxlen;
    char *str;

    memset( name, 0, len );

    for( i = 0;; i++ )
    {
	str = strstr( data, token );
	if( !str )
	    break;
	maxlen = ( str - data ) > len ? len : str - data;
	strncpy( name, data, maxlen );
	name[maxlen] = '\0';
	if( i == which )
	{
	    return 1;
	}
	data = str + strlen( token );
    }

    strncpy( name, def, len );	// No find

    return 0;
}
