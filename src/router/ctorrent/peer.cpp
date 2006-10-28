#include "peer.h"

#include <stdlib.h>
#include <string.h>

#include "./btcontent.h"
#include "./msgencode.h"
#include "./peerlist.h"
#include "./btconfig.h"

btBasic Self;

void btBasic::SetIp(struct sockaddr_in addr)
{
  memcpy(&m_sin.sin_addr,&addr.sin_addr,sizeof(struct in_addr));
}

void btBasic::SetAddress(struct sockaddr_in addr)
{
  memcpy(&m_sin,&addr,sizeof(struct sockaddr_in));
}

int btBasic::IpEquiv(struct sockaddr_in addr)
{
//	fprintf(stdout,"IpEquiv: %s <=> ", inet_ntoa(m_sin.sin_addr));
//	fprintf(stdout,"%s\n", inet_ntoa(addr.sin_addr));
	return (memcmp(&m_sin.sin_addr,&addr.sin_addr,sizeof(struct in_addr)) == 0) ? 
    1 : 0;
}

int btPeer::Need_Local_Data()
{
  if( m_state.remote_interested && !bitfield.IsFull()){

    if( BTCONTENT.pBF->IsFull() ) return 1; // i am seed

    BitField tmpBitfield = *BTCONTENT.pBF;
    tmpBitfield.Except(bitfield);
    return tmpBitfield.IsEmpty() ? 0 : 1;

  }
  return 0;
}

int btPeer::Need_Remote_Data()
{
  if( BTCONTENT.pBF->IsFull()) return 0;
  else if( bitfield.IsFull() ) return 1;
  else{
    BitField tmpBitfield = bitfield;
    tmpBitfield.Except(*BTCONTENT.pBF);
    return tmpBitfield.IsEmpty() ? 0 : 1;
  }
  return 0;
}

btPeer::btPeer()
{
  m_f_keepalive = 0;
  m_status = P_CONNECTING;
  m_unchoke_timestamp = (time_t) 0;
  time(&m_last_timestamp);
  m_state.remote_choked = m_state.local_choked = 1;
  m_state.remote_interested = m_state.local_interested = 0;

  m_err_count = 0;
  m_cached_idx = BTCONTENT.GetNPieces();
}

int btPeer::SetLocal(unsigned char s)
{
  switch(s){
  case M_CHOKE:
    if( m_state.local_choked ) return 0;
    m_state.local_choked = 1; 
    break;
  case M_UNCHOKE: 
    if( !reponse_q.IsEmpty() ) StartULTimer();
    if( !m_state.local_choked ) return 0;
    time(&m_unchoke_timestamp);
    m_state.local_choked = 0;
    break;
  case M_INTERESTED: 
    if( m_state.local_interested ) return 0;
    m_state.local_interested = 1;
    break;
  case M_NOT_INTERESTED:
    if( !m_state.local_interested ) return 0;
    m_state.local_interested = 0; 
    break;
  default:
    return -1;			// BUG ???
  }
  return stream.Send_State(s);
}

int btPeer::RequestPiece()
{
  size_t idx;

  PENDINGQUEUE.ReAssign(&request_q,bitfield);

  if( !request_q.IsEmpty() ) return SendRequest();

  if( m_cached_idx < BTCONTENT.GetNPieces() ){
    idx = m_cached_idx;
    m_cached_idx = BTCONTENT.GetNPieces();
    if( !BTCONTENT.pBF->IsSet(idx) &&
	!PENDINGQUEUE.Exist(idx) &&
	!WORLD.AlreadyRequested(idx) ){
      return (request_q.CreateWithIdx(idx) < 0) ? -1 : SendRequest();
    }
  }else{
    BitField tmpBitField;
    if( bitfield.IsFull() ){
      tmpBitField = *BTCONTENT.pBF;
      tmpBitField.Invert();
    }else{
      tmpBitField = bitfield;
      tmpBitField.Except(*BTCONTENT.pBF);
    }

    if( !tmpBitField.IsEmpty() ){
      WORLD.CheckBitField(tmpBitField);
      if(tmpBitField.IsEmpty()){
	
	btPeer *peer = WORLD.Who_Can_Abandon(this);
	if(peer){
	  peer->StopDLTimer();
	  request_q = peer->request_q;

	  if(peer->CancelRequest(request_q.GetHead()) < 0 ||
	     peer->RequestCheck() < 0){
	    peer->CloseConnection();
	  }
	  
	  return SendRequest();
	}
	
      }else{
	idx = tmpBitField.Random();
	return (request_q.CreateWithIdx(idx) < 0) ? -1 : SendRequest();
      }
    }
  }
  return 0;
}

int btPeer::MsgDeliver()
{
  size_t r,idx,off,len;

  char *msgbuf = stream.in_buffer.BasePointer();

  r = ntohl(*(size_t*) msgbuf);

  if( 0 == r ){
    time(&m_last_timestamp);
    if( !m_f_keepalive ) if( stream.Send_Keepalive() < 0 ) return -1;
    m_f_keepalive = 0;
    return (!m_state.remote_choked && request_q.IsEmpty()) ? RequestCheck() : 0;
  }else{
    switch(msgbuf[4]){
    case M_CHOKE:
      if(H_BASE_LEN != r){ return -1;}
      m_state.remote_choked = 1;
      StopDLTimer();
      if( !request_q.IsEmpty()){
	PSLICE ps = request_q.GetHead();
	PENDINGQUEUE.Pending(&request_q);
	if( CancelRequest(ps) < 0) return -1;
      }
      return 0;
    case M_UNCHOKE:
      if(H_BASE_LEN != r){return -1;}
      m_state.remote_choked = 0;
      return RequestCheck();

    case M_INTERESTED:
      if(H_BASE_LEN != r){return -1;}
      m_state.remote_interested = 1;
      break;

    case M_NOT_INTERESTED:
      if(r != H_BASE_LEN){return -1;}

      m_state.remote_interested = 0;
      StopULTimer();

      /* remove peer's reponse queue */
      if( !reponse_q.IsEmpty()) reponse_q.Empty();
      return 0;
    case M_HAVE:
      if(H_HAVE_LEN != r){return -1;}

      idx = ntohl(*(size_t*) (msgbuf + 5));

      if( idx >= BTCONTENT.GetNPieces() || bitfield.IsSet(idx)) return -1;

      bitfield.Set(idx);

      if( bitfield.IsFull() && BTCONTENT.pBF->IsFull() ){ return -2; }

      if( !BTCONTENT.pBF->IsSet(idx) ) m_cached_idx = idx;
      
      return ( !m_state.remote_choked && request_q.IsEmpty() ) ? RequestCheck() : 0;

    case M_REQUEST:
      if(H_REQUEST_LEN != r || !m_state.remote_interested){ return -1; }

      idx = ntohl(*(size_t*)(msgbuf + 5));
      
      if( !BTCONTENT.pBF->IsSet(idx) ) return -1;
      
      off = ntohl(*(size_t*)(msgbuf + 9));
      len = ntohl(*(size_t*)(msgbuf + 13));

      if( !reponse_q.IsValidRequest(idx, off, len) ) return -1;
      
      return reponse_q.Add(idx, off, len);

    case M_PIECE:
      if( request_q.IsEmpty() || !m_state.local_interested){
	m_err_count++;
	return 0;
      }
      return PieceDeliver(r);

    case M_BITFIELD:
      if( (r - 1) != bitfield.NBytes() || !bitfield.IsEmpty()) return -1;
      bitfield.SetReferBuffer(msgbuf + 5);
      if(bitfield.IsFull() && BTCONTENT.pBF->IsFull()) return -2;
      return 0;

    case M_CANCEL:
      if(r != H_CANCEL_LEN || !m_state.remote_interested) return -1;

      idx = ntohl(*(size_t*)(msgbuf + 5));
      off = ntohl(*(size_t*)(msgbuf + 9));
      len = ntohl(*(size_t*)(msgbuf + 13));
      if( reponse_q.Remove(idx,off,len) < 0 ){
	m_err_count++;
	return 0;
      }
      if( reponse_q.IsEmpty() ) StopULTimer();
      return 0;
    default:
      return -1;		// unknow message type
    }
  }
  return 0;
}

int btPeer::ReponseSlice()
{
  size_t len = 0;

  reponse_q.Peek((size_t*) 0,(size_t*) 0, &len);

  if(len && stream.out_buffer.LeftSize() <= (len + 13 + 3 * 1024))
    stream.Flush();

  if(len && stream.out_buffer.LeftSize() > (len + 13 + 3 * 1024)){
    size_t idx,off;
    reponse_q.Pop(&idx,&off,(size_t *) 0);

    if(BTCONTENT.ReadSlice(BTCONTENT.global_piece_buffer,idx,off,len) != 0 ){
      return -1;
    }

    Self.DataSended(len);
    DataSended(len);
    return stream.Send_Piece(idx,off,BTCONTENT.global_piece_buffer,len);
  }

  return 0;
}

int btPeer::SendRequest()
{
  PSLICE ps = request_q.GetHead();
  for( ; ps ; ps = ps->next )
    if(stream.Send_Request(ps->index,ps->offset,ps->length) < 0){ return -1; }

  return stream.Flush();
}

int btPeer::CancelRequest(PSLICE ps)
{
  for( ; ps; ps = ps->next){
    if(stream.Send_Cancel(ps->index,ps->offset,ps->length) < 0)
      return -1;
  }
  return stream.Flush();
}

int btPeer::ReportComplete(size_t idx)
{
  if( BTCONTENT.APieceComplete(idx) ){
    WORLD.Tell_World_I_Have(idx);
    if( BTCONTENT.pBF->IsFull() ){
      ResetDLTimer();
      WORLD.CloseAllConnectionToSeed();
    }
  }else
    m_err_count++;
  return (P_FAILED == m_status) ? -1 : RequestCheck();
}

int btPeer::PieceDeliver(size_t mlen)
{
  size_t idx,off,len;
  char *msgbuf = stream.in_buffer.BasePointer();

  idx = ntohl(*(size_t*) (msgbuf + 5));
  off = ntohl(*(size_t*) (msgbuf + 9));
  len = mlen - 9;

  if( request_q.Remove(idx,off,len) < 0 ){
    m_err_count++;
    return 0;
  }

  if(BTCONTENT.WriteSlice((char*)(msgbuf + 13),idx,off,len) < 0){
    return 0;
  }

  Self.StartDLTimer();
  Self.DataRecved(len);
  DataRecved(len);

  /* if piece download complete. */
  return request_q.IsEmpty() ? ReportComplete(idx) : 0;
}

int btPeer::RequestCheck()
{
  if( BandWidthLimit() ) return 0;
  
  if( BTCONTENT.pBF->IsFull() ){
    if( bitfield.IsFull() ){ return -1; }
    return SetLocal(M_NOT_INTERESTED);
  }

  if( Need_Remote_Data() ){
    if(!m_state.local_interested && SetLocal(M_INTERESTED) < 0) return -1;
    if(request_q.IsEmpty() && !m_state.remote_choked){
      if( RequestPiece() < 0 ) return -1;
    }
  }
  
  if(!request_q.IsEmpty()) StartDLTimer();
  return 0;
}

void btPeer::CloseConnection()
{
  if( P_FAILED != m_status ){
    m_status = P_FAILED;
    stream.Close();
  }
}

int btPeer::HandShake()
{
  ssize_t r = stream.Feed();
  if( r < 0 ) return -1;
  else if( r < 68 ){
    if(r && memcmp(stream.in_buffer.BasePointer(),BTCONTENT.GetShakeBuffer(),r) != 0) return -1;
    return 0;
  }

  if( memcmp(stream.in_buffer.BasePointer(),BTCONTENT.GetShakeBuffer(),48) != 0 ) return -1;

  // ignore peer id verify
  if( !BTCONTENT.pBF->IsEmpty()){
    char *bf = new char[BTCONTENT.pBF->NBytes()];
#ifndef WINDOWS
    if(!bf) return -1;
#endif
    BTCONTENT.pBF->WriteToBuffer(bf);
    r = stream.Send_Bitfield(bf,BTCONTENT.pBF->NBytes());
    delete []bf;
  }

  if( r >= 0){
    if( stream.in_buffer.PickUp(68) < 0 ) return -1;
    m_status = P_SUCCESS;
  }
  return r;
}

int btPeer::Send_ShakeInfo()
{
  return stream.Send_Buffer((char*)BTCONTENT.GetShakeBuffer(),68);
}

int btPeer::BandWidthLimit()
{
  if( cfg_max_bandwidth <= 0 ) return 0;
  return ((Self.RateDL() + Self.RateUL()*2) / 1024 >= cfg_max_bandwidth) ?
    1:0;
}

int btPeer::NeedWrite()
{
  int yn = 0;
  if( stream.out_buffer.Count() || // data need send in buffer.
      (!reponse_q.IsEmpty() && CouldReponseSlice() && !BandWidthLimit()) ||
      P_CONNECTING == m_status ) // peer is connecting
    yn = 1;
  return yn;
}

int btPeer::CouldReponseSlice()
{
  if(!m_state.local_choked &&
     (stream.out_buffer.LeftSize() > reponse_q.GetRequestLen() + 4 * 1024 ))
    return 1;
  return 0;
}

int btPeer::AreYouOK()
{
  m_f_keepalive = 1;
  return stream.Send_Keepalive();
}

int btPeer::RecvModule()
{
  int f_peer_closed = 0;
  ssize_t r;
  
  if ( 64 < m_err_count ) return -1;
  
  r = stream.Feed();

  if( r < 0 && r != -2 )
    return -1;
  else if ( r == -2 )
    f_peer_closed = 1;
  
  r = stream.HaveMessage();
  for( ; r;){
    if( r < 0 ) return -1;
    if(MsgDeliver() < 0 || stream.PickMessage() < 0) return -1;
    r = stream.HaveMessage();
  }
  return f_peer_closed ? -1 : 0;
}

int btPeer::SendModule()
{
  if( stream.out_buffer.Count() && stream.Flush() < 0) return -1;

  if(! reponse_q.IsEmpty() &&  CouldReponseSlice() ) {
    StartULTimer();
    Self.StartULTimer();
  }

  for(; !reponse_q.IsEmpty() && CouldReponseSlice(); )
    if( ReponseSlice() < 0) return -1;

  return 0;
}

void btPeer::dump()
{
  struct sockaddr_in sin;

  GetAddress(&sin);
  printf("%s: %d -> %d:%d   %lud:%lud\n", inet_ntoa(sin.sin_addr), 
		 bitfield.Count(),
		 Is_Remote_UnChoked() ? 1 : 0,
		 request_q.IsEmpty() ? 0 : 1,
		 (unsigned long)TotalDL(),
		 (unsigned long)TotalUL());
}

