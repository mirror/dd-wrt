 /* Complicated way to move "len" data from begining of "src" to end of "dst" */

 if (!vstr_iter_fwd_beg(src, 1, len, iter)) abort();

 movlen = 0;
 do
 {
   movlen += iter->len;
   addlen  = iter->len;
 } while (vstr_iter_fwd_nxt(iter));
 movlen -= addlen;

 /* move all complete nodes -- note that this still has to copy the first
 * node if it is a _BUF node and has some of the deleted "used" */
 vstr_mov(dst, dst->len, src, 1, movlen);

 /* add/del the data in the last node */
 vstr_add_vstr(dst, dst->len, src, 1, addlen, VSTR_TYPE_ADD_DEF);
 vstr_del(src, 1, addlen);
