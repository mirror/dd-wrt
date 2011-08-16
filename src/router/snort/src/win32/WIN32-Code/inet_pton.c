#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

int inet_pton(int af, const char *src, void *dst) {
    u_int16_t ipbuf[8];
    u_int32_t val;
    u_int16_t short_val;
    char *end_ptr;
    int i,j;
    int index = 0;
    int skip_idx = -1;
    u_int16_t *dstip = (u_int16_t*)dst;

    if(!src || !dst) return -1;

    if(af == AF_INET) {
        return inet_aton(src, dst);
    }

    while(*src) {
        val = strtoul(src, &end_ptr, 16);
        if (val > USHRT_MAX)
        {
            return -1;
        }
        short_val = (u_int16_t)val;

        if(*src == ':') {
            src++;
        
            if(*src == ':') {
                if(skip_idx != -1)
                    return -1;

                skip_idx = index;

                src++;

                if(*src && *src == ':') 
                    return -1;
            }
            else if(!*src) 
                return -1;

            continue;
        }
        else if(*end_ptr == '.') {
            if(!inet_aton(src, (struct in_addr *)&ipbuf[index]))
                return -1;

            index += 2;
            
            break;
        }
        else {
            if(end_ptr == src) {
                return -1;
            }

            ipbuf[index++] = htons(short_val);

            src = end_ptr;

            /* Check for trailing garbage after the IP */
            if(index == 8 && *src) 
                return -1;
        }
    }

    if(index < 8 && skip_idx == -1)
        return -1;

    for(i = 0; i < skip_idx; i++) {
        dstip[i] = ipbuf[i];
    }

    if(skip_idx == -1) skip_idx = 0;

    for(; i < 8 - (index - skip_idx); i++) {
        dstip[i] = 0;
    }
    
    for(j = skip_idx; i < 8; i++, j++) {
        dstip[i] = ipbuf[j];
    }

    return 1;
}

