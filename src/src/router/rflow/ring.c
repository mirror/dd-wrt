#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <net/if.h>

#include "pcap-bpf.h"

#define PAGE_SIZE         4096
#include "ring.h"

/* Copy the bucket in order to avoid kernel
   crash if the application faults
*/
#define SAFE_RING_MODE
#undef RING_DEBUG

static struct ring_stat ringStats;
u_int  numPollCalls = 0;
#ifdef SAFE_RING_MODE
static char staticBucket[2048];
#endif

ring_t* 
ring_open (const char* device, int snaplen, int promisc, 
           char* ebuf)
{
  ring_t* handle;
  int err;
  int open_ok = 0;

  handle = malloc(sizeof(*handle));
  if (handle == NULL) {
    snprintf(ebuf, RING_ERRBUF_SIZE, "malloc: %s",
        strerror(errno));
    return 0;
  }

  memset(handle, 0, sizeof(*handle));
  handle->snapshot  = snaplen;
  handle->timeout  = 500; // to_ms;

  handle->ring_fd = socket(PF_RING, SOCK_RAW, htons(ETH_P_ALL));
  printf("Open RING [fd=%d]\n", handle->ring_fd);
  if(handle->ring_fd > 0) {
    struct sockaddr sa;
    int             rc;
    u_int memSlotsLen;

    err = 0;
    sa.sa_family   = PF_RING;
    snprintf(sa.sa_data, sizeof(sa.sa_data), "%s", device);
    rc = bind(handle->ring_fd, (struct sockaddr *)&sa, sizeof(sa));

    if(rc == 0) {
      handle->device = strdup(device);
      handle->ring_buffer = (char *)mmap(NULL, PAGE_SIZE,
          PROT_READ|PROT_WRITE,
          MAP_SHARED,
          handle->ring_fd, 0);

      if(handle->ring_buffer == MAP_FAILED) {
        sprintf(ebuf, "mmap() failed");
        return (NULL);
      }

      handle->slots_info = (FlowSlotInfo *)handle->ring_buffer;
      if(handle->slots_info->version != RING_FLOWSLOT_VERSION) {
        snprintf(ebuf, RING_ERRBUF_SIZE, "Wrong RING version: "
            "kernel is %i, program was compiled with %i\n",
            handle->slots_info->version, RING_FLOWSLOT_VERSION);
        return (NULL);
      }

      memSlotsLen = handle->slots_info->tot_mem;
      munmap(handle->ring_buffer, PAGE_SIZE);

      handle->ring_buffer = (char *)mmap(NULL, memSlotsLen,
          PROT_READ|PROT_WRITE,
          MAP_SHARED, handle->ring_fd, 0);

      if(handle->ring_buffer == MAP_FAILED) {
        sprintf(ebuf, "mmap() failed");
        return (NULL);
      }

      handle->slots_info   = (FlowSlotInfo *)handle->ring_buffer;
      handle->ring_slots = (char *)(handle->ring_buffer+sizeof(FlowSlotInfo));

      /* Safety check */
      if(handle->slots_info->remove_idx >= handle->slots_info->tot_slots)
        handle->slots_info->remove_idx = 0;

      handle->page_id = PAGE_SIZE, handle->slot_id = 0,
      handle->pkts_per_page = 0;

      /* Set defaults */
      handle->linktype = DLT_EN10MB;
      handle->offset = 2;

      printf("RING (%s): tot_slots=%d/slot_len=%d/"
          "insertIdx=%d/remove_idx=%d/dropped=%d\n",
          device,
          handle->slots_info->tot_slots,
          handle->slots_info->slot_len,
          handle->slots_info->insert_idx,
          handle->slots_info->remove_idx,
          handle->slots_info->tot_lost);

      ringStats.ps_recv = handle->slots_info->tot_read;
      ringStats.ps_drop = handle->slots_info->tot_lost;
      
      if(promisc) {
        struct ifreq ifr;

        err = 0;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, device, sizeof(ifr.ifr_name));
        if (ioctl(handle->fd, SIOCGIFFLAGS, &ifr) == -1) {
          snprintf(ebuf, RING_ERRBUF_SIZE,
              "ioctl: %s", ring_strerror(errno));
          err = 1;
        }

        if(err == 0) {
          if ((ifr.ifr_flags & IFF_PROMISC) == 0) {
            ifr.ifr_flags |= IFF_PROMISC;
            if (ioctl(handle->fd, SIOCSIFFLAGS, &ifr) == -1) {
              snprintf(ebuf, RING_ERRBUF_SIZE,
                  "ioctl: %s", ring_strerror(errno));
              err = 1;
            }
          }

          if(err == 0)
            handle->clear_promisc = 1;
        }
      }

      if(err == 0)
        return handle;
    }

    close(handle->ring_fd);
    // XXX unmap mem, free handle?
    return 0;
  }

  return handle;
}

int
ring_read_packet(ring_t *handle, ring_handler callback, void *userdata)
{
  u_int idx, numRuns = 0, ptrAddr;
  FlowSlot *slot;

  if (handle->ring_buffer == NULL)
    return -1;

  slot = (FlowSlot*)&handle->ring_slots[
                handle->slots_info->remove_idx * handle->slots_info->slot_len
                                       ];
  while(1) {
    u_int32_t queuedPkts;

    if(handle->slots_info->tot_insert >= handle->slots_info->tot_read)
      queuedPkts = 
        handle->slots_info->tot_insert - handle->slots_info->tot_read;
    else
      queuedPkts = 
        handle->slots_info->tot_slots + 
        handle->slots_info->tot_insert - 
        handle->slots_info->tot_read;

    if(queuedPkts && (slot->slot_state == 1)) {
      char *bucket = &slot->bucket;
#ifdef RING_MAGIC
      if(slot->magic != RING_MAGIC_VALUE) {
        printf("==>> Bad Magic [remove_idx=%u][insert_idx=%u][ptrAddr=%u]\n",
            handle->slots_info->remove_idx,
            handle->slots_info->insert_idx,
            ptrAddr);
        slot->magic = RING_MAGIC_VALUE;
      }
#endif

      handle->stat.ps_recv++;
      
#ifdef SAFE_RING_MODE
      {
        struct ring_pkthdr *hdr = (struct ring_pkthdr*)bucket;
        int bktLen = hdr->caplen;
        if(bktLen > sizeof(staticBucket))
          bktLen = sizeof(staticBucket);
        memcpy(staticBucket, &bucket[sizeof(struct ring_pkthdr)], bktLen);
#ifdef RING_DEBUG
        printf("==>> [remove_idx=%u][insert_idx=%u][ptrAddr=%u]\n",
            handle->slots_info->remove_idx,
            handle->slots_info->insert_idx,
            ptrAddr);
#endif
        (*callback)(userdata, hdr, staticBucket);
      }
#else
        (*callback)(userdata,
            (const struct ring_pkthdr*)bucket,
            (const u_char*)&bucket[sizeof(struct ring_pkthdr)]);
#endif
        if(handle->slots_info->remove_idx >= (handle->slots_info->tot_slots-1))
        {
          handle->slots_info->remove_idx = 0;
          
          handle->page_id = PAGE_SIZE, 
          handle->slot_id = 0, 
          handle->pkts_per_page = 0;
        } else {
          handle->slots_info->remove_idx++;
          handle->pkts_per_page++, 
          handle->slot_id += handle->slots_info->slot_len;
        }

        handle->slots_info->tot_read++;
        slot->slot_state = 0;
        return(1);
      } else {
        struct pollfd pfd;
        int rc;
        /* Sleep when nothing is happening */
        pfd.fd      = handle->ring_fd;
        pfd.events  = POLLIN|POLLERR;
        pfd.revents = 0;
#ifdef RING_DEBUG
        printf("==>> poll [remove_idx=%u][insert_idx=%u][loss=%d][queuedPkts=%u]"
            "[slot_state=%d][tot_insert=%u][tot_read=%u]\n",
            handle->slots_info->remove_idx,
            handle->slots_info->insert_idx,
            handle->slots_info->tot_lost,
            queuedPkts, slot->slot_state,
            handle->slots_info->tot_insert,
            handle->slots_info->tot_read);
#endif
#ifdef RING_DEBUG
        printf("==>> poll @ [remove_idx=%u][slot_id=%u]\n", 
            handle->slots_info->remove_idx, handle->slot_id);
#endif
        errno = 0;
        rc = poll(&pfd, 1, -1);
#ifdef RING_DEBUG
        printf("==>> poll returned %d [%s][errno=%d][break_loop=%d]\n",
            rc, strerror(errno), errno, handle->break_loop);
#endif
        numPollCalls++;

        if(rc == -1) {
          if(errno == EINTR) {
            if(handle->break_loop) {
              handle->break_loop = 0;
              return(-2);
            } else
              return(0);
          } else
            return(-1);
        }
      }
  } /* while() */
}

int
ring_stats(ring_t *handle, struct ring_stat *stats)
{
  if(handle->ring_fd > 0) {
    stats->ps_recv = handle->slots_info->tot_read-ringStats.ps_recv;
    stats->ps_drop = handle->slots_info->tot_lost-ringStats.ps_drop;
    printf("RING: numPollCalls=%d [%.1f packets/call]\n",
        numPollCalls, (float)stats->ps_recv/(float)numPollCalls);
    printf("RING: [tot_pkts=%u][tot_read=%u][tot_lost=%u]\n",
        handle->slots_info->tot_pkts,
        handle->slots_info->tot_read,
        handle->slots_info->tot_lost);
    return(0);
  }

  return -1;
}

void ring_close (ring_t *handle)
{
  ring_t *p, *prevp;
  struct ifreq  ifr;

  if(handle->ring_buffer != NULL) {
    munmap(handle->ring_buffer, handle->slots_info->tot_mem);
    handle->ring_buffer = NULL;
  }
}

int ring_fileno (ring_t *handle)
{
	return handle->ring_fd;
}

unsigned int ring_datalink (ring_t *handle)
{
	return handle->linktype;
}

char* ring_strerror (int errnum)
{
	return strerror (errnum);
}

