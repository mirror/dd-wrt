#include "tracker.h"

#ifndef WINDOWS
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <netdb.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "peerlist.h"
#include "httpencode.h"
#include "bencode.h"
#include "setnonblock.h"
#include "connect_nonb.h"
#include "btcontent.h"
#include "iplist.h"

#include "btconfig.h"

btTracker Tracker;

btTracker::btTracker()
{
  memset(m_host,0,MAXHOSTNAMELEN);
  memset(m_path,0,MAXPATHLEN);

  m_sock = INVALID_SOCKET;
  m_port = 80;
  m_status = T_FREE;
  m_f_started = m_f_stoped = m_f_pause = 0;
  m_interval = 15;

  m_connect_refuse_click = 0;
  m_last_timestamp = (time_t) 0;
}

btTracker::~btTracker()
{
  if( m_sock != INVALID_SOCKET) CLOSE_SOCKET(m_sock);
}

void btTracker::Reset(time_t new_interval)
{
  if(new_interval) m_interval = new_interval;
  
  if( INVALID_SOCKET != m_sock ){
    CLOSE_SOCKET(m_sock);
    m_sock = INVALID_SOCKET;
  }
  
  m_reponse_buffer.Reset();
  time(&m_last_timestamp);
  m_status = T_FREE;
}

int btTracker:: _IPsin(char *h, int p, struct sockaddr_in *psin)
{
  psin->sin_family = AF_INET;
  psin->sin_port = htons(p);
  psin->sin_addr.s_addr = inet_addr(h);
  return ( psin->sin_addr.s_addr == INADDR_NONE ) ? -1 : 0;
}

int btTracker:: _s2sin(char *h,int p,struct sockaddr_in *psin)
{
  psin->sin_family = AF_INET;
  psin->sin_port = htons(p);
  if( h ){
    psin->sin_addr.s_addr = inet_addr(h);
    if(psin->sin_addr.s_addr == INADDR_NONE){
      struct hostent *ph = gethostbyname(h);
      if( !ph  || ph->h_addrtype != AF_INET){
	memset(psin,0,sizeof(struct sockaddr_in));
	return -1;
      }
      memcpy(&psin->sin_addr,ph->h_addr_list[0],sizeof(struct in_addr));
    }
  }else 
    psin->sin_addr.s_addr = htonl(INADDR_ANY);
  return 0;
}

int btTracker::_UpdatePeerList(char *buf,size_t bufsiz)
{
  char tmphost[MAXHOSTNAMELEN];
  const char *ps;
  size_t i,pos,tmpport;
  size_t cnt = 0;

  struct sockaddr_in addr;

  if( decode_query(buf,bufsiz,"failure reason",&ps,&i,QUERY_STR) ){
    char failreason[1024];
    if( i < 1024 ){
      memcpy(failreason, ps, i);
      failreason[i] = '\0';
    }else{
      memcpy(failreason, ps, 1000);
      failreason[1000] = '\0';
      strcat(failreason,"...");
    }
    fprintf(stderr,"TRACKER FAILURE REASON: %s\n",failreason);
    return -1;
  }

  if(!decode_query(buf,bufsiz,"interval",(const char**) 0,&i,QUERY_INT)){return -1;}

  if(m_interval != (time_t)i) m_interval = (time_t)i;

  pos = decode_query(buf,bufsiz,"peers",(const char**) 0,(size_t *) 0,QUERY_POS);

  if( !pos ){
    return -1;
  }

  if(4 > bufsiz - pos){return -1; } // peers list ̫С

  buf += (pos + 1); bufsiz -= (pos + 1);

  ps = buf-1;
  if (*ps != 'l') {		// binary peers section if not 'l'
	addr.sin_family = AF_INET;
	i = 0;
	while (*ps != ':' ) i = i * 10 + (*ps++ - '0');
	i /= 6;
	ps++;
	while (i-- > 0) {
		// if peer is not us
		if(memcmp(&Self.m_sin.sin_addr,ps,sizeof(struct in_addr))) {
			memcpy(&addr.sin_addr,ps,sizeof(struct in_addr));
			memcpy(&addr.sin_port,ps+sizeof(struct in_addr),sizeof(unsigned short));
			cnt++;
			IPQUEUE.Add(&addr);
		}
		ps += 6;
	}
  }
  else
  for( ;bufsiz && *buf!='e'; buf += pos, bufsiz -= pos ){
    pos = decode_dict(buf,bufsiz,(char*) 0);
    if(!pos) break;
    if(!decode_query(buf,pos,"ip",&ps,&i,QUERY_STR) || MAXHOSTNAMELEN < i) continue;
    memcpy(tmphost,ps,i); tmphost[i] = '\0';

    if(!decode_query(buf,pos,"port",(const char**) 0,&tmpport,QUERY_INT)) continue;

    if(!decode_query(buf,pos,"peer id",&ps,&i,QUERY_STR) && i != 20 ) continue;

    if(_IPsin(tmphost,tmpport,&addr) < 0){
      fprintf(stderr,"warn, detected invalid ip address %s.\n",tmphost);
      continue;
    }

    if( !Self.IpEquiv(addr) ){
      cnt++;
      IPQUEUE.Add(&addr);
    }
  }
  
  if( !cnt ) fprintf(stderr,"warn, peers list received from tracker is empty.\n");
  return 0;
}

int btTracker::CheckReponse()
{
#define MAX_LINE_SIZ 32
  char *pdata;
  ssize_t r;
  size_t q, hlen, dlen;

  r = m_reponse_buffer.FeedIn(m_sock);

  if( r > 0 ) return 0;
  
  q = m_reponse_buffer.Count();

  Reset( (-1 == r) ? 15 : 0 );

  if( !q ){
    int error = 0;
    socklen_t n = sizeof(error);
    if(getsockopt(m_sock, SOL_SOCKET,SO_ERROR,&error,&n) < 0 ||
       error != 0 ){
      fprintf(stderr,"warn, received nothing from tracker! %s\n",strerror(error));
    }
    return -1;
  }

  hlen = Http_split(m_reponse_buffer.BasePointer(), q, &pdata,&dlen);

  if( !hlen ){
    fprintf(stderr,"warn, tracker reponse invalid. No html header found.\n");
    return -1;
  }

  r = Http_reponse_code(m_reponse_buffer.BasePointer(),hlen);
  if ( r != 200 ){
    if( r == 301 || r == 302 ){
      char redirect[MAXPATHLEN],ih_buf[20 * 3 + 1],pi_buf[20 * 3 + 1],tmppath[MAXPATHLEN];
      if( Http_get_header(m_reponse_buffer.BasePointer(), hlen, "Location", redirect) < 0 )
	return -1;

      if( Http_url_analyse(redirect,m_host,&m_port,m_path) < 0){
	fprintf(stderr,"warn, tracker redirect to an invalid url %s!\n", redirect);
	return -1;
      }

      strcpy(tmppath,m_path);
      
      if(MAXPATHLEN < snprintf(m_path,MAXPATHLEN,REQ_URL_P1_FMT,
			       tmppath,
			       Http_url_encode(ih_buf, (char*)BTCONTENT.GetInfoHash(), 20),
			       Http_url_encode(pi_buf, (char*)BTCONTENT.GetPeerId(), 20),
			       cfg_listen_port)){
	return -1;
      }

      return Connect();
    }else if( r >= 400 ){
      fprintf(stderr,"\nTracker reponse code >= 400 !!! The file is not registered on this tracker, or it may have been removed. IF YOU SEE THIS MESSAGE FOR A LONG TIME AND DOWNLOAD DOESN'T BEGIN, RECOMMEND YOU STOP NOW!!!\n");
      fprintf(stderr,"\nTracker reponse data DUMP:\n");
      if( pdata && dlen ) write(STDERR_FILENO, pdata, dlen);
      fprintf(stderr,"\n== DUMP OVER==\n");
      return -1;
    }else
      return 0;
  }

  if ( !pdata ) return 0;

  if( !m_f_started ) m_f_started = 1;
  m_connect_refuse_click = 0;

  return _UpdatePeerList(pdata,dlen);
}

int btTracker::Initial()
{
  char ih_buf[20 * 3 + 1],pi_buf[20 * 3 + 1],tmppath[MAXPATHLEN];

  if(Http_url_analyse(BTCONTENT.GetAnnounce(),m_host,&m_port,m_path) < 0){
    fprintf(stderr,"error, invalid tracker url format!\n");
    return -1;
  }

  strcpy(tmppath,m_path);

  if(MAXPATHLEN < snprintf((char*)m_path,MAXPATHLEN,REQ_URL_P1_FMT,
			   tmppath,
			   Http_url_encode(ih_buf,(char*)BTCONTENT.GetInfoHash(),20),
			   Http_url_encode(pi_buf,(char*)BTCONTENT.GetPeerId(),20),
			   cfg_listen_port)){
    return -1;
  }
	
  /* get local ip address */
  // 1st: if behind firewall, this only gets local side
  {
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  if(getsockname(m_sock,(struct sockaddr*)&addr,&addrlen) == 0)
	Self.SetIp(addr);
  }
  // 2nd: better to use addr of our domain
  {
	  struct hostent *h;
	  char hostname[128];
	  char *hostdots[2]={0,0}, *hdptr=hostname;

	  if (gethostname(hostname, 128) == -1) return -1;
//	  printf("%s\n", hostname);
	  while(*hdptr) if(*hdptr++ == '.') {
		  hostdots[0] = hostdots[1];
		  hostdots[1] = hdptr;
	  }
	  if (hostdots[0] == 0) return -1;
//	  printf("%s\n", hostdots[0]);
	  if ((h = gethostbyname(hostdots[0])) == NULL) return -1;
	  //printf("Host domain  : %s\n", h->h_name);
	  //printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr)));
	  memcpy(&Self.m_sin.sin_addr,h->h_addr,sizeof(struct in_addr));
  }
  return 0;
}

int btTracker::Connect()
{
  ssize_t r;
  time(&m_last_timestamp);

  if(_s2sin(m_host,m_port,&m_sin) < 0) {
    fprintf(stderr,"warn, get tracker's ip address failed.");
    return -1;
  }

  m_sock = socket(AF_INET,SOCK_STREAM,0);
  if(INVALID_SOCKET == m_sock) return -1;

  if(setfd_nonblock(m_sock) < 0) {CLOSE_SOCKET(m_sock); return -1; }

  r = connect_nonb(m_sock,(struct sockaddr*)&m_sin);

  if( r == -1 ){ CLOSE_SOCKET(m_sock); return -1;}
  else if( r == -2 ) m_status = T_CONNECTING;
  else{
    if( 0 == SendRequest()) m_status = T_READY;
    else{ CLOSE_SOCKET(m_sock); return -1;}
  }
  return 0;
}

int btTracker::SendRequest()
{
  char *event,*str_event[] = {"started","stopped","completed" };
  char REQ_BUFFER[MAXPATHLEN];
  socklen_t addrlen;

  struct sockaddr_in addr;
  addrlen = sizeof(struct sockaddr_in);

  /* get local ip address */
  if(getsockname(m_sock,(struct sockaddr*)&addr,&addrlen) < 0){ return -1;}

//jc  Self.SetIp(addr);
//  fprintf(stdout,"Old Set Self:");
//  fprintf(stdout,"%s\n", inet_ntoa(Self.m_sin.sin_addr));

  if( m_f_stoped )	/* stopped */
    event = str_event[1];
  else if( BTCONTENT.pBF->IsFull())	/* download complete */
    event = str_event[2];
  else if( m_f_started ) 	/* interval */
    event = (char*) 0;
  else
    event = str_event[0];	/* started */

  if(event){
    if(MAXPATHLEN < snprintf(REQ_BUFFER,MAXPATHLEN,REQ_URL_P2_FMT,
			     m_path,
			     (size_t)Self.TotalUL(),
			     (size_t)Self.TotalDL(),
			     (size_t)BTCONTENT.GetLeftBytes(),
			     event)){
      return -1;
    }
  }else{
    if(MAXPATHLEN < snprintf(REQ_BUFFER,MAXPATHLEN,REQ_URL_P3_FMT,
			     m_path,
			     (size_t)Self.TotalUL(),
			     (size_t)Self.TotalDL(),
			     (size_t)BTCONTENT.GetLeftBytes()
			     )){
      return -1;
    }
  }

  if(_IPsin(m_host, m_port, &addr) < 0){
    char REQ_HOST[MAXHOSTNAMELEN];
    if(MAXHOSTNAMELEN < snprintf(REQ_HOST,MAXHOSTNAMELEN,"\r\nHost: %s",m_host)) return -1;
    strcat(REQ_BUFFER, REQ_HOST);
  }
  
  strcat(REQ_BUFFER,"\r\n\r\n");
  // hc
  //fprintf(stderr,"SendRequest: %s\n", REQ_BUFFER);

  if( 0 != m_reponse_buffer.PutFlush(m_sock,REQ_BUFFER,strlen((char*)REQ_BUFFER))){
    fprintf(stderr,"warn, send request to tracker failed. %s\n",strerror(errno));
    return -1;
  }

  return 0;
}

int btTracker::IntervalCheck(const time_t *pnow, fd_set *rfdp, fd_set *wfdp)
{
  /* tracker communication */
  if( T_FREE == m_status ){
    if((*pnow - m_last_timestamp >= m_interval) &&
       (cfg_min_peers > WORLD.TotalPeers())){
   
      if(Connect() < 0){ Reset(15); return -1; }
    
      if( m_status == T_CONNECTING ){
	FD_SET(m_sock, rfdp);
	FD_SET(m_sock, wfdp);
      }else{
	FD_SET(m_sock, rfdp);
      }
    }
  }else{
    if( m_status == T_CONNECTING ){
      FD_SET(m_sock, rfdp);
      FD_SET(m_sock, wfdp);
    }else{
      FD_SET(m_sock, rfdp);
    }
  }
  return m_sock;
}

int btTracker::SocketReady(fd_set *rfdp, fd_set *wfdp, int *nfds)
{
  if( T_FREE == m_status ) return 0;

  if( T_CONNECTING == m_status && 
      (FD_ISSET(m_sock, wfdp) || FD_ISSET(m_sock,wfdp)) ){
    int error = 0;
    socklen_t n = sizeof(error);
    (*nfds)--;
    FD_CLR(m_sock, wfdp); 
    if(getsockopt(m_sock, SOL_SOCKET,SO_ERROR,&error,&n) < 0 ||
       error != 0 ){
      if( ECONNREFUSED != error )
	fprintf(stderr,"warn, connect to tracker failed. %s\n",strerror(error));
      else
	m_connect_refuse_click++;
      Reset(15);
      return -1;
    }else{
      if( SendRequest() == 0 ) m_status = T_READY; 
      else { Reset(15); return -1; }
    }
  }else if(FD_ISSET(m_sock, rfdp) ){
    (*nfds)--;
    FD_CLR(m_sock,rfdp);
    CheckReponse();
  }
  return 0;
}
