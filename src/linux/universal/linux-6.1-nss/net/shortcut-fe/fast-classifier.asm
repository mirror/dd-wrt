
fast-classifier.ko:     file format elf32-littlearm


Disassembly of section .text:

00000000 <sfe_ipv4_connection_match_compute_translations>:
/*
 * sfe_ipv4_connection_match_compute_translations()
 *	Compute port and address translations for a connection match entry.
 */
static void sfe_ipv4_connection_match_compute_translations(struct sfe_ipv4_connection_match *cm)
{
       0:	e1a0c00d 	mov	ip, sp
       4:	e92dd810 	push	{r4, fp, ip, lr, pc}
       8:	e24cb004 	sub	fp, ip, #4
	/*
	 * Before we insert the entry look to see if this is tagged as doing address
	 * translations.  If it is then work out the adjustment that we need to apply
	 * to the transport checksum.
	 */
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC) {
       c:	e5903030 	ldr	r3, [r0, #48]	@ 0x30
      10:	e213c001 	ands	ip, r3, #1
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_src_csum_adjustment = (u16)adj;

	}

	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
      14:	e2033002 	and	r3, r3, #2
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC) {
      18:	1a000028 	bne	c0 <sfe_ipv4_connection_match_compute_translations+0xc0>
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
      1c:	e3530000 	cmp	r3, #0
      20:	0a000024 	beq	b8 <sfe_ipv4_connection_match_compute_translations+0xb8>
		/*
		 * Precompute an incremental checksum adjustment so we can
		 * edit packets in this stream very quickly.  The algorithm is from RFC1624.
		 */
		u16 dest_ip_hi = cm->match_dest_ip >> 16;
      24:	e5903028 	ldr	r3, [r0, #40]	@ 0x28
		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_dest_csum_adjustment = (u16)adj;
	}

	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC) {
      28:	e35c0000 	cmp	ip, #0
		adj = dest_ip_hi + dest_ip_lo + cm->match_dest_port
      2c:	e1d042be 	ldrh	r4, [r0, #46]	@ 0x2e
		u16 xlate_dest_port = ~cm->xlate_dest_port;
      30:	e1d026b0 	ldrh	r2, [r0, #96]	@ 0x60
		u32 xlate_dest_ip = ~cm->xlate_dest_ip;
      34:	e590105c 	ldr	r1, [r0, #92]	@ 0x5c
		adj = dest_ip_hi + dest_ip_lo + cm->match_dest_port
      38:	e6ffe073 	uxth	lr, r3
      3c:	e08ee823 	add	lr, lr, r3, lsr #16
		u16 xlate_dest_port = ~cm->xlate_dest_port;
      40:	e1e02002 	mvn	r2, r2
		u32 xlate_dest_ip = ~cm->xlate_dest_ip;
      44:	e1e0c001 	mvn	ip, r1
		adj = dest_ip_hi + dest_ip_lo + cm->match_dest_port
      48:	e08ee004 	add	lr, lr, r4
		      + xlate_dest_ip_hi + xlate_dest_ip_lo + xlate_dest_port;
      4c:	e6fe2072 	uxtah	r2, lr, r2
      50:	e082282c 	add	r2, r2, ip, lsr #16
      54:	e6f2c07c 	uxtah	ip, r2, ip
		adj = (adj & 0xffff) + (adj >> 16);
      58:	e6ff207c 	uxth	r2, ip
      5c:	e082282c 	add	r2, r2, ip, lsr #16
		adj = (adj & 0xffff) + (adj >> 16);
      60:	e1a0c822 	lsr	ip, r2, #16
      64:	e6fc2072 	uxtah	r2, ip, r2
		cm->xlate_dest_csum_adjustment = (u16)adj;
      68:	e1c026b2 	strh	r2, [r0, #98]	@ 0x62
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC) {
      6c:	0a000009 	beq	98 <sfe_ipv4_connection_match_compute_translations+0x98>
		u32 adj = ~cm->match_src_ip + cm->xlate_src_ip;
      70:	e590c024 	ldr	ip, [r0, #36]	@ 0x24
      74:	e5902050 	ldr	r2, [r0, #80]	@ 0x50
		if (adj < cm->xlate_src_ip) {
      78:	e152000c 	cmp	r2, ip
		u32 adj = ~cm->match_src_ip + cm->xlate_src_ip;
      7c:	e042200c 	sub	r2, r2, ip
		if (adj < cm->xlate_src_ip) {
      80:	9a00002b 	bls	134 <sfe_ipv4_connection_match_compute_translations+0x134>
			adj++;
		}

		adj = (adj & 0xffff) + (adj >> 16);
      84:	e1a0c822 	lsr	ip, r2, #16
      88:	e6fc2072 	uxtah	r2, ip, r2
		adj = (adj & 0xffff) + (adj >> 16);
      8c:	e1a0c822 	lsr	ip, r2, #16
      90:	e6fc2072 	uxtah	r2, ip, r2
		cm->xlate_src_partial_csum_adjustment = (u16)adj;
      94:	e1c025b8 	strh	r2, [r0, #88]	@ 0x58
	}

	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
		u32 adj = ~cm->match_dest_ip + cm->xlate_dest_ip;
		if (adj < cm->xlate_dest_ip) {
      98:	e1530001 	cmp	r3, r1
		u32 adj = ~cm->match_dest_ip + cm->xlate_dest_ip;
      9c:	e0413003 	sub	r3, r1, r3
      a0:	22433001 	subcs	r3, r3, #1
			adj++;
		}

		adj = (adj & 0xffff) + (adj >> 16);
      a4:	e1a02823 	lsr	r2, r3, #16
      a8:	e6f23073 	uxtah	r3, r2, r3
		adj = (adj & 0xffff) + (adj >> 16);
      ac:	e1a02823 	lsr	r2, r3, #16
      b0:	e6f23073 	uxtah	r3, r2, r3
		cm->xlate_dest_partial_csum_adjustment = (u16)adj;
      b4:	e1c036b4 	strh	r3, [r0, #100]	@ 0x64
	}

}
      b8:	e24bd010 	sub	sp, fp, #16
      bc:	e89da810 	ldm	sp, {r4, fp, sp, pc}
		u16 src_ip_hi = cm->match_src_ip >> 16;
      c0:	e590e024 	ldr	lr, [r0, #36]	@ 0x24
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
      c4:	e3530000 	cmp	r3, #0
		adj = src_ip_hi + src_ip_lo + cm->match_src_port
      c8:	e1d032bc 	ldrh	r3, [r0, #44]	@ 0x2c
		u32 xlate_src_ip = ~cm->xlate_src_ip;
      cc:	e5902050 	ldr	r2, [r0, #80]	@ 0x50
		adj = src_ip_hi + src_ip_lo + cm->match_src_port
      d0:	e6ff407e 	uxth	r4, lr
      d4:	e084482e 	add	r4, r4, lr, lsr #16
		u32 xlate_src_ip = ~cm->xlate_src_ip;
      d8:	e1e01002 	mvn	r1, r2
		adj = src_ip_hi + src_ip_lo + cm->match_src_port
      dc:	e0844003 	add	r4, r4, r3
		u16 xlate_src_port = ~cm->xlate_src_port;
      e0:	e1d035b4 	ldrh	r3, [r0, #84]	@ 0x54
      e4:	e1e03003 	mvn	r3, r3
		      + xlate_src_ip_hi + xlate_src_ip_lo + xlate_src_port;
      e8:	e6f43073 	uxtah	r3, r4, r3
      ec:	e0833821 	add	r3, r3, r1, lsr #16
      f0:	e6f31071 	uxtah	r1, r3, r1
		adj = (adj & 0xffff) + (adj >> 16);
      f4:	e6ff3071 	uxth	r3, r1
      f8:	e0833821 	add	r3, r3, r1, lsr #16
		adj = (adj & 0xffff) + (adj >> 16);
      fc:	e1a01823 	lsr	r1, r3, #16
     100:	e6f13073 	uxtah	r3, r1, r3
		cm->xlate_src_csum_adjustment = (u16)adj;
     104:	e1c035b6 	strh	r3, [r0, #86]	@ 0x56
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
     108:	1affffc5 	bne	24 <sfe_ipv4_connection_match_compute_translations+0x24>
		if (adj < cm->xlate_src_ip) {
     10c:	e15e0002 	cmp	lr, r2
		u32 adj = ~cm->match_src_ip + cm->xlate_src_ip;
     110:	e042200e 	sub	r2, r2, lr
     114:	22422001 	subcs	r2, r2, #1
		adj = (adj & 0xffff) + (adj >> 16);
     118:	e1a03822 	lsr	r3, r2, #16
     11c:	e6f32072 	uxtah	r2, r3, r2
		adj = (adj & 0xffff) + (adj >> 16);
     120:	e1a03822 	lsr	r3, r2, #16
     124:	e6f32072 	uxtah	r2, r3, r2
		cm->xlate_src_partial_csum_adjustment = (u16)adj;
     128:	e1c025b8 	strh	r2, [r0, #88]	@ 0x58
}
     12c:	e24bd010 	sub	sp, fp, #16
     130:	e89da810 	ldm	sp, {r4, fp, sp, pc}
		u32 adj = ~cm->match_src_ip + cm->xlate_src_ip;
     134:	e2422001 	sub	r2, r2, #1
     138:	eaffffd1 	b	84 <sfe_ipv4_connection_match_compute_translations+0x84>

0000013c <sfe_ipv4_update_summary_stats>:
/*
 * sfe_ipv4_update_summary_stats()
 *	Update the summary stats.
 */
static void sfe_ipv4_update_summary_stats(struct sfe_ipv4 *si)
{
     13c:	e1a0c00d 	mov	ip, sp
     140:	e92dd8d0 	push	{r4, r6, r7, fp, ip, lr, pc}
     144:	e24cb004 	sub	fp, ip, #4
	int i;

	si->connection_create_requests64 += si->connection_create_requests;
     148:	e308e0f0 	movw	lr, #33008	@ 0x80f0
     14c:	e2802902 	add	r2, r0, #32768	@ 0x8000
     150:	e080100e 	add	r1, r0, lr
     154:	e592c030 	ldr	ip, [r2, #48]	@ 0x30
	si->connection_create_requests = 0;
     158:	e3a06000 	mov	r6, #0
	si->connection_create_requests64 += si->connection_create_requests;
     15c:	e5113008 	ldr	r3, [r1, #-8]
	si->connection_create_requests = 0;
     160:	e3a07000 	mov	r7, #0
	si->connection_create_requests64 += si->connection_create_requests;
     164:	e3a04000 	mov	r4, #0
     168:	e093300c 	adds	r3, r3, ip
     16c:	e5013008 	str	r3, [r1, #-8]
     170:	e5113004 	ldr	r3, [r1, #-4]
     174:	e2a33000 	adc	r3, r3, #0
     178:	e5013004 	str	r3, [r1, #-4]
	si->connection_create_collisions64 += si->connection_create_collisions;
     17c:	e790300e 	ldr	r3, [r0, lr]
     180:	e592c034 	ldr	ip, [r2, #52]	@ 0x34
     184:	e093300c 	adds	r3, r3, ip
     188:	e591c004 	ldr	ip, [r1, #4]
     18c:	e780300e 	str	r3, [r0, lr]
	si->connection_create_collisions = 0;
	si->connection_destroy_requests64 += si->connection_destroy_requests;
     190:	e2803c81 	add	r3, r0, #33024	@ 0x8100
	si->connection_create_collisions64 += si->connection_create_collisions;
     194:	e2acc000 	adc	ip, ip, #0
     198:	e581c004 	str	ip, [r1, #4]
	si->connection_destroy_requests64 += si->connection_destroy_requests;
     19c:	e9134002 	ldmdb	r3, {r1, lr}
     1a0:	e592c038 	ldr	ip, [r2, #56]	@ 0x38
     1a4:	e091100c 	adds	r1, r1, ip
	si->connection_destroy_requests = 0;
	si->connection_destroy_misses64 += si->connection_destroy_misses;
     1a8:	e593c004 	ldr	ip, [r3, #4]
	si->connection_destroy_requests64 += si->connection_destroy_requests;
     1ac:	e2aee000 	adc	lr, lr, #0
     1b0:	e9034002 	stmdb	r3, {r1, lr}
	si->connection_destroy_misses64 += si->connection_destroy_misses;
     1b4:	e5931000 	ldr	r1, [r3]
     1b8:	e592e03c 	ldr	lr, [r2, #60]	@ 0x3c
     1bc:	e091100e 	adds	r1, r1, lr
	si->connection_destroy_misses = 0;
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
     1c0:	e308e110 	movw	lr, #33040	@ 0x8110
	si->connection_destroy_misses64 += si->connection_destroy_misses;
     1c4:	e2acc000 	adc	ip, ip, #0
     1c8:	e8831002 	stm	r3, {r1, ip}
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
     1cc:	e080300e 	add	r3, r0, lr
     1d0:	e592c040 	ldr	ip, [r2, #64]	@ 0x40
     1d4:	e5131008 	ldr	r1, [r3, #-8]
     1d8:	e091100c 	adds	r1, r1, ip
     1dc:	e5031008 	str	r1, [r3, #-8]
     1e0:	e5131004 	ldr	r1, [r3, #-4]
     1e4:	e2a11000 	adc	r1, r1, #0
     1e8:	e5031004 	str	r1, [r3, #-4]
	si->connection_match_hash_hits = 0;
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
     1ec:	e790100e 	ldr	r1, [r0, lr]
     1f0:	e592c044 	ldr	ip, [r2, #68]	@ 0x44
     1f4:	e091100c 	adds	r1, r1, ip
     1f8:	e593c004 	ldr	ip, [r3, #4]
     1fc:	e780100e 	str	r1, [r0, lr]
	si->connection_match_hash_reorders = 0;
	si->connection_flushes64 += si->connection_flushes;
     200:	e308e120 	movw	lr, #33056	@ 0x8120
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
     204:	e2ac1000 	adc	r1, ip, #0
     208:	e5831004 	str	r1, [r3, #4]
	si->connection_flushes64 += si->connection_flushes;
     20c:	e080300e 	add	r3, r0, lr
     210:	e592c048 	ldr	ip, [r2, #72]	@ 0x48
     214:	e5131008 	ldr	r1, [r3, #-8]
     218:	e091100c 	adds	r1, r1, ip
     21c:	e513c004 	ldr	ip, [r3, #-4]
     220:	e5031008 	str	r1, [r3, #-8]
     224:	e2ac1000 	adc	r1, ip, #0
     228:	e5031004 	str	r1, [r3, #-4]
	si->connection_flushes = 0;
	si->packets_forwarded64 += si->packets_forwarded;
     22c:	e790100e 	ldr	r1, [r0, lr]
     230:	e592c04c 	ldr	ip, [r2, #76]	@ 0x4c
     234:	e091100c 	adds	r1, r1, ip
     238:	e593c004 	ldr	ip, [r3, #4]
     23c:	e780100e 	str	r1, [r0, lr]
	si->connection_create_requests = 0;
     240:	e308e030 	movw	lr, #32816	@ 0x8030
	si->packets_forwarded64 += si->packets_forwarded;
     244:	e2ac1000 	adc	r1, ip, #0
	si->connection_create_requests = 0;
     248:	e282c050 	add	ip, r2, #80	@ 0x50
	si->packets_forwarded64 += si->packets_forwarded;
     24c:	e5831004 	str	r1, [r3, #4]
	si->connection_create_requests = 0;
     250:	e3083040 	movw	r3, #32832	@ 0x8040
	si->packets_forwarded = 0;
	si->packets_not_forwarded64 += si->packets_not_forwarded;
     254:	e2801c81 	add	r1, r0, #33024	@ 0x8100
	si->connection_create_requests = 0;
     258:	e18060fe 	strd	r6, [r0, lr]
     25c:	e080e003 	add	lr, r0, r3
     260:	e14e60f8 	strd	r6, [lr, #-8]
     264:	e18060f3 	strd	r6, [r0, r3]
     268:	e2800c82 	add	r0, r0, #33280	@ 0x8200
     26c:	e14c60f8 	strd	r6, [ip, #-8]
	si->packets_not_forwarded64 += si->packets_not_forwarded;
     270:	e5913028 	ldr	r3, [r1, #40]	@ 0x28
     274:	e2800050 	add	r0, r0, #80	@ 0x50
     278:	e592e050 	ldr	lr, [r2, #80]	@ 0x50
     27c:	e093300e 	adds	r3, r3, lr
     280:	e591e02c 	ldr	lr, [r1, #44]	@ 0x2c
     284:	e5813028 	str	r3, [r1, #40]	@ 0x28
     288:	e2813028 	add	r3, r1, #40	@ 0x28
     28c:	e2aee000 	adc	lr, lr, #0
     290:	e581e02c 	str	lr, [r1, #44]	@ 0x2c
	si->packets_not_forwarded = 0;
     294:	e5824050 	str	r4, [r2, #80]	@ 0x50

	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
		si->exception_events64[i] += si->exception_events[i];
     298:	e5b32008 	ldr	r2, [r3, #8]!
     29c:	e5bce004 	ldr	lr, [ip, #4]!
     2a0:	e5931004 	ldr	r1, [r3, #4]
     2a4:	e092200e 	adds	r2, r2, lr
     2a8:	e5832000 	str	r2, [r3]
     2ac:	e2a12000 	adc	r2, r1, #0
	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
     2b0:	e1530000 	cmp	r3, r0
		si->exception_events64[i] += si->exception_events[i];
     2b4:	e5832004 	str	r2, [r3, #4]
		si->exception_events[i] = 0;
     2b8:	e58c4000 	str	r4, [ip]
	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
     2bc:	1afffff5 	bne	298 <sfe_ipv4_update_summary_stats+0x15c>
	}
}
     2c0:	e24bd018 	sub	sp, fp, #24
     2c4:	e89da8d0 	ldm	sp, {r4, r6, r7, fp, sp, pc}

000002c8 <sfe_ipv4_debug_dev_read>:
/*
 * sfe_ipv4_debug_dev_read()
 *	Send info to userspace upon read request from user
 */
static ssize_t sfe_ipv4_debug_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
     2c8:	e1a0c00d 	mov	ip, sp
     2cc:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
     2d0:	e24cb004 	sub	fp, ip, #4
     2d4:	e24ddfc7 	sub	sp, sp, #796	@ 0x31c
	char msg[CHAR_DEV_MSG_SIZE];
	int total_read = 0;
	struct sfe_ipv4_debug_xml_write_state *ws;
	struct sfe_ipv4 *si = &__si;

	ws = (struct sfe_ipv4_debug_xml_write_state *)filp->private_data;
     2d8:	e5904090 	ldr	r4, [r0, #144]	@ 0x90
	int total_read = 0;
     2dc:	e3a03000 	mov	r3, #0
{
     2e0:	e50b2330 	str	r2, [fp, #-816]	@ 0xfffffcd0
     2e4:	e1a06001 	mov	r6, r1
	int total_read = 0;
     2e8:	e50b3328 	str	r3, [fp, #-808]	@ 0xfffffcd8
	while ((ws->state != SFE_IPV4_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     2ec:	e594c000 	ldr	ip, [r4]
     2f0:	e35c0009 	cmp	ip, #9
     2f4:	0a000016 	beq	354 <sfe_ipv4_debug_dev_read+0x8c>
		if ((sfe_ipv4_debug_xml_write_methods[ws->state])(si, buffer, msg, &length, &total_read, ws)) {
     2f8:	e3008000 	movw	r8, #0
     2fc:	e3007000 	movw	r7, #0
     300:	e3408000 	movt	r8, #0
     304:	e3407000 	movt	r7, #0
     308:	ea000007 	b	32c <sfe_ipv4_debug_dev_read+0x64>
     30c:	e798510c 	ldr	r5, [r8, ip, lsl #2]
     310:	e24bcfca 	sub	ip, fp, #808	@ 0x328
     314:	e58dc000 	str	ip, [sp]
     318:	e58d4004 	str	r4, [sp, #4]
     31c:	e12fff35 	blx	r5
	while ((ws->state != SFE_IPV4_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     320:	e594c000 	ldr	ip, [r4]
     324:	e35c0009 	cmp	ip, #9
     328:	0a000006 	beq	348 <sfe_ipv4_debug_dev_read+0x80>
     32c:	e51be330 	ldr	lr, [fp, #-816]	@ 0xfffffcd0
		if ((sfe_ipv4_debug_xml_write_methods[ws->state])(si, buffer, msg, &length, &total_read, ws)) {
     330:	e24b3e33 	sub	r3, fp, #816	@ 0x330
     334:	e24b2fc9 	sub	r2, fp, #804	@ 0x324
     338:	e1a01006 	mov	r1, r6
     33c:	e1a00007 	mov	r0, r7
	while ((ws->state != SFE_IPV4_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     340:	e35e0c03 	cmp	lr, #768	@ 0x300
     344:	8afffff0 	bhi	30c <sfe_ipv4_debug_dev_read+0x44>
			continue;
		}
	}

	return total_read;
     348:	e51b0328 	ldr	r0, [fp, #-808]	@ 0xfffffcd8
}
     34c:	e24bd020 	sub	sp, fp, #32
     350:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
	while ((ws->state != SFE_IPV4_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     354:	e1a00003 	mov	r0, r3
     358:	eafffffb 	b	34c <sfe_ipv4_debug_dev_read+0x84>

0000035c <sfe_ipv6_connection_match_compute_translations>:
/*
 * sfe_ipv6_connection_match_compute_translations()
 *	Compute port and address translations for a connection match entry.
 */
static void sfe_ipv6_connection_match_compute_translations(struct sfe_ipv6_connection_match *cm)
{
     35c:	e1a0c00d 	mov	ip, sp
     360:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
     364:	e24cb004 	sub	fp, ip, #4
     368:	e24dd02c 	sub	sp, sp, #44	@ 0x2c
	/*
	 * Before we insert the entry look to see if this is tagged as doing address
	 * translations.  If it is then work out the adjustment that we need to apply
	 * to the transport checksum.
	 */
	if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC) {
     36c:	e590c048 	ldr	ip, [r0, #72]	@ 0x48
     370:	e31c0001 	tst	ip, #1
     374:	0a00002d 	beq	430 <sfe_ipv6_connection_match_compute_translations+0xd4>
		*(idx_32++) = cm->match_src_ip->addr[2];
		*(idx_32++) = cm->match_src_ip->addr[3];

		idx_16 = (u16 *)idx_32;
		*(idx_16++) = cm->match_src_port;
		*(idx_16++) = ~cm->xlate_src_port;
     378:	e1d0e7b8 	ldrh	lr, [r0, #120]	@ 0x78
		*(idx_32++) = cm->match_src_ip->addr[0];
     37c:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
		*(idx_16++) = cm->match_src_port;
     380:	e5902028 	ldr	r2, [r0, #40]	@ 0x28
     384:	e590702c 	ldr	r7, [r0, #44]	@ 0x2c
     388:	e5906030 	ldr	r6, [r0, #48]	@ 0x30
		*(idx_16++) = ~cm->xlate_src_port;
     38c:	e1e0e00e 	mvn	lr, lr
		*(idx_16++) = cm->match_src_port;
     390:	e1d014b4 	ldrh	r1, [r0, #68]	@ 0x44
		 */
		for (idx_32 = diff; idx_32 < diff + 9; idx_32++) {
			u32 w = *idx_32;
			adj += carry;
			adj += w;
			carry = (w > adj);
     394:	e0933002 	adds	r3, r3, r2
		*(idx_32++) = ~cm->xlate_src_ip->addr[1];
     398:	e590206c 	ldr	r2, [r0, #108]	@ 0x6c
		*(idx_16++) = cm->match_src_port;
     39c:	e14b14b4 	strh	r1, [fp, #-68]	@ 0xffffffbc
		*(idx_32++) = ~cm->xlate_src_ip->addr[0];
     3a0:	e5901068 	ldr	r1, [r0, #104]	@ 0x68
		*(idx_16++) = ~cm->xlate_src_port;
     3a4:	e14be4b2 	strh	lr, [fp, #-66]	@ 0xffffffbe
			adj += w;
     3a8:	e0a7e003 	adc	lr, r7, r3
		*(idx_32++) = ~cm->xlate_src_ip->addr[1];
     3ac:	e1e09002 	mvn	r9, r2
     3b0:	e086400e 	add	r4, r6, lr
			u32 w = *idx_32;
     3b4:	e51b5044 	ldr	r5, [fp, #-68]	@ 0xffffffbc
			adj += w;
     3b8:	e157000e 	cmp	r7, lr
     3bc:	82844001 	addhi	r4, r4, #1
		*(idx_32++) = ~cm->xlate_src_ip->addr[2];
     3c0:	e590e070 	ldr	lr, [r0, #112]	@ 0x70
		*(idx_32++) = ~cm->xlate_src_ip->addr[0];
     3c4:	e1e0a001 	mvn	sl, r1
		*(idx_32++) = ~cm->xlate_src_ip->addr[3];
     3c8:	e5903074 	ldr	r3, [r0, #116]	@ 0x74
		*(idx_32++) = ~cm->xlate_src_ip->addr[2];
     3cc:	e1e0800e 	mvn	r8, lr
     3d0:	e084e005 	add	lr, r4, r5
			adj += w;
     3d4:	e1560004 	cmp	r6, r4
     3d8:	828ee001 	addhi	lr, lr, #1
		*(idx_32++) = ~cm->xlate_src_ip->addr[3];
     3dc:	e1e07003 	mvn	r7, r3
     3e0:	e04e1001 	sub	r1, lr, r1
			adj += w;
     3e4:	e155000e 	cmp	r5, lr
     3e8:	92411001 	subls	r1, r1, #1
     3ec:	e0412002 	sub	r2, r1, r2
     3f0:	e15a0001 	cmp	sl, r1
     3f4:	92422001 	subls	r2, r2, #1
     3f8:	e5901070 	ldr	r1, [r0, #112]	@ 0x70
     3fc:	e0421001 	sub	r1, r2, r1
     400:	e1590002 	cmp	r9, r2
     404:	92411001 	subls	r1, r1, #1
     408:	e0413003 	sub	r3, r1, r3
     40c:	e1580001 	cmp	r8, r1
     410:	92433001 	subls	r3, r3, #1
		}
		adj += carry;
     414:	e1570003 	cmp	r7, r3
     418:	82833001 	addhi	r3, r3, #1
		adj = (adj & 0xffff) + (adj >> 16);
     41c:	e1a02823 	lsr	r2, r3, #16
     420:	e6f23073 	uxtah	r3, r2, r3
		adj = (adj & 0xffff) + (adj >> 16);
     424:	e1a02823 	lsr	r2, r3, #16
     428:	e6f23073 	uxtah	r3, r2, r3
		cm->xlate_src_csum_adjustment = (u16)adj;
     42c:	e1c037ba 	strh	r3, [r0, #122]	@ 0x7a
	}

	if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST) {
     430:	e31c0002 	tst	ip, #2
     434:	0a00002b 	beq	4e8 <sfe_ipv6_connection_match_compute_translations+0x18c>
		*(idx_32++) = cm->match_dest_ip->addr[2];
		*(idx_32++) = cm->match_dest_ip->addr[3];

		idx_16 = (u16 *)idx_32;
		*(idx_16++) = cm->match_dest_port;
		*(idx_16++) = ~cm->xlate_dest_port;
     438:	e1d028bc 	ldrh	r2, [r0, #140]	@ 0x8c
		*(idx_32++) = cm->match_dest_ip->addr[2];
     43c:	e2803034 	add	r3, r0, #52	@ 0x34
     440:	e8935008 	ldm	r3, {r3, ip, lr}
		*(idx_16++) = cm->match_dest_port;
     444:	e1d014b6 	ldrh	r1, [r0, #70]	@ 0x46
		*(idx_16++) = ~cm->xlate_dest_port;
     448:	e1e02002 	mvn	r2, r2
		*(idx_32++) = cm->match_dest_ip->addr[3];
     44c:	e5907040 	ldr	r7, [r0, #64]	@ 0x40
		*(idx_16++) = ~cm->xlate_dest_port;
     450:	e093300c 	adds	r3, r3, ip
		idx_32 = (u32 *)idx_16;

		*(idx_32++) = ~cm->xlate_dest_ip->addr[0];
		*(idx_32++) = ~cm->xlate_dest_ip->addr[1];
		*(idx_32++) = ~cm->xlate_dest_ip->addr[2];
     454:	e590c084 	ldr	ip, [r0, #132]	@ 0x84
		*(idx_16++) = cm->match_dest_port;
     458:	e14b14b4 	strh	r1, [fp, #-68]	@ 0xffffffbc
		 * wrap-around!
		 */
		for (idx_32 = diff; idx_32 < diff + 9; idx_32++) {
			u32 w = *idx_32;
			adj += carry;
			adj += w;
     45c:	e0ae3003 	adc	r3, lr, r3
		*(idx_32++) = ~cm->xlate_dest_ip->addr[0];
     460:	e590107c 	ldr	r1, [r0, #124]	@ 0x7c
		*(idx_16++) = ~cm->xlate_dest_port;
     464:	e14b24b2 	strh	r2, [fp, #-66]	@ 0xffffffbe
     468:	e0878003 	add	r8, r7, r3
		*(idx_32++) = ~cm->xlate_dest_ip->addr[1];
     46c:	e5902080 	ldr	r2, [r0, #128]	@ 0x80
			u32 w = *idx_32;
     470:	e51b5044 	ldr	r5, [fp, #-68]	@ 0xffffffbc
			adj += w;
     474:	e15e0003 	cmp	lr, r3
     478:	82888001 	addhi	r8, r8, #1
		*(idx_32++) = ~cm->xlate_dest_ip->addr[3];
     47c:	e5903088 	ldr	r3, [r0, #136]	@ 0x88
		*(idx_32++) = ~cm->xlate_dest_ip->addr[0];
     480:	e1e06001 	mvn	r6, r1
		*(idx_32++) = ~cm->xlate_dest_ip->addr[1];
     484:	e1e0e002 	mvn	lr, r2
     488:	e0884005 	add	r4, r8, r5
			adj += w;
     48c:	e1570008 	cmp	r7, r8
     490:	82844001 	addhi	r4, r4, #1
     494:	e0441001 	sub	r1, r4, r1
     498:	e1550004 	cmp	r5, r4
     49c:	92411001 	subls	r1, r1, #1
     4a0:	e0412002 	sub	r2, r1, r2
     4a4:	e1560001 	cmp	r6, r1
     4a8:	92422001 	subls	r2, r2, #1
     4ac:	e042100c 	sub	r1, r2, ip
		*(idx_32++) = ~cm->xlate_dest_ip->addr[2];
     4b0:	e1e0c00c 	mvn	ip, ip
			adj += w;
     4b4:	e15e0002 	cmp	lr, r2
     4b8:	92411001 	subls	r1, r1, #1
		*(idx_32++) = ~cm->xlate_dest_ip->addr[3];
     4bc:	e1e02003 	mvn	r2, r3
     4c0:	e0413003 	sub	r3, r1, r3
			adj += w;
     4c4:	e15c0001 	cmp	ip, r1
     4c8:	92433001 	subls	r3, r3, #1
			carry = (w > adj);
		}
		adj += carry;
     4cc:	e1520003 	cmp	r2, r3
     4d0:	82833001 	addhi	r3, r3, #1
		adj = (adj & 0xffff) + (adj >> 16);
     4d4:	e1a02823 	lsr	r2, r3, #16
     4d8:	e6f23073 	uxtah	r3, r2, r3
		adj = (adj & 0xffff) + (adj >> 16);
     4dc:	e1a02823 	lsr	r2, r3, #16
     4e0:	e6f23073 	uxtah	r3, r2, r3
		cm->xlate_dest_csum_adjustment = (u16)adj;
     4e4:	e1c038be 	strh	r3, [r0, #142]	@ 0x8e
	}
}
     4e8:	e24bd028 	sub	sp, fp, #40	@ 0x28
     4ec:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

000004f0 <sfe_ipv6_update_summary_stats>:
/*
 * sfe_ipv6_update_summary_stats()
 *	Update the summary stats.
 */
static void sfe_ipv6_update_summary_stats(struct sfe_ipv6 *si)
{
     4f0:	e1a0c00d 	mov	ip, sp
     4f4:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
     4f8:	e24cb004 	sub	fp, ip, #4
     4fc:	e24dd00c 	sub	sp, sp, #12
	int i;

	si->connection_create_requests64 += si->connection_create_requests;
     500:	e280e902 	add	lr, r0, #32768	@ 0x8000
     504:	e30810f0 	movw	r1, #33008	@ 0x80f0
     508:	e7903001 	ldr	r3, [r0, r1]
     50c:	e0804001 	add	r4, r0, r1
	si->connection_match_hash_hits = 0;
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
	si->connection_match_hash_reorders = 0;
	si->connection_flushes64 += si->connection_flushes;
	si->connection_flushes = 0;
	si->packets_forwarded64 += si->packets_forwarded;
     510:	e3086130 	movw	r6, #33072	@ 0x8130
	si->connection_create_requests64 += si->connection_create_requests;
     514:	e59e2030 	ldr	r2, [lr, #48]	@ 0x30
	si->connection_create_requests = 0;
     518:	e3087040 	movw	r7, #32832	@ 0x8040
     51c:	e3a08000 	mov	r8, #0
     520:	e3a09000 	mov	r9, #0
     524:	e28e5050 	add	r5, lr, #80	@ 0x50
     528:	e080a007 	add	sl, r0, r7
	si->connection_create_requests64 += si->connection_create_requests;
     52c:	e3a0c000 	mov	ip, #0
     530:	e0933002 	adds	r3, r3, r2
     534:	e5942004 	ldr	r2, [r4, #4]
     538:	e7803001 	str	r3, [r0, r1]
	si->connection_create_collisions64 += si->connection_create_collisions;
     53c:	e2803c81 	add	r3, r0, #33024	@ 0x8100
	si->connection_create_requests64 += si->connection_create_requests;
     540:	e2a22000 	adc	r2, r2, #0
     544:	e5842004 	str	r2, [r4, #4]
	si->connection_create_collisions64 += si->connection_create_collisions;
     548:	e9130014 	ldmdb	r3, {r2, r4}
     54c:	e59e1034 	ldr	r1, [lr, #52]	@ 0x34
     550:	e0922001 	adds	r2, r2, r1
	si->connection_destroy_requests64 += si->connection_destroy_requests;
     554:	e5931004 	ldr	r1, [r3, #4]
	si->connection_create_collisions64 += si->connection_create_collisions;
     558:	e2a44000 	adc	r4, r4, #0
     55c:	e9030014 	stmdb	r3, {r2, r4}
	si->connection_destroy_requests64 += si->connection_destroy_requests;
     560:	e5932000 	ldr	r2, [r3]
     564:	e59e4038 	ldr	r4, [lr, #56]	@ 0x38
     568:	e0922004 	adds	r2, r2, r4
	si->connection_destroy_misses64 += si->connection_destroy_misses;
     56c:	e3084110 	movw	r4, #33040	@ 0x8110
	si->connection_destroy_requests64 += si->connection_destroy_requests;
     570:	e2a11000 	adc	r1, r1, #0
     574:	e5832000 	str	r2, [r3]
     578:	e5831004 	str	r1, [r3, #4]
	si->connection_destroy_misses64 += si->connection_destroy_misses;
     57c:	e0803004 	add	r3, r0, r4
     580:	e59e103c 	ldr	r1, [lr, #60]	@ 0x3c
     584:	e5132008 	ldr	r2, [r3, #-8]
     588:	e0922001 	adds	r2, r2, r1
     58c:	e5032008 	str	r2, [r3, #-8]
     590:	e5132004 	ldr	r2, [r3, #-4]
     594:	e2a22000 	adc	r2, r2, #0
     598:	e5032004 	str	r2, [r3, #-4]
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
     59c:	e7902004 	ldr	r2, [r0, r4]
     5a0:	e59e1040 	ldr	r1, [lr, #64]	@ 0x40
     5a4:	e0922001 	adds	r2, r2, r1
     5a8:	e5931004 	ldr	r1, [r3, #4]
     5ac:	e7802004 	str	r2, [r0, r4]
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
     5b0:	e3084120 	movw	r4, #33056	@ 0x8120
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
     5b4:	e2a12000 	adc	r2, r1, #0
     5b8:	e5832004 	str	r2, [r3, #4]
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
     5bc:	e0803004 	add	r3, r0, r4
     5c0:	e59e1044 	ldr	r1, [lr, #68]	@ 0x44
     5c4:	e5132008 	ldr	r2, [r3, #-8]
     5c8:	e0922001 	adds	r2, r2, r1
     5cc:	e5032008 	str	r2, [r3, #-8]
     5d0:	e5132004 	ldr	r2, [r3, #-4]
     5d4:	e2a22000 	adc	r2, r2, #0
     5d8:	e5032004 	str	r2, [r3, #-4]
	si->connection_flushes64 += si->connection_flushes;
     5dc:	e7902004 	ldr	r2, [r0, r4]
     5e0:	e59e1048 	ldr	r1, [lr, #72]	@ 0x48
     5e4:	e0922001 	adds	r2, r2, r1
     5e8:	e5931004 	ldr	r1, [r3, #4]
     5ec:	e7802004 	str	r2, [r0, r4]
	si->packets_forwarded64 += si->packets_forwarded;
     5f0:	e0804006 	add	r4, r0, r6
	si->connection_flushes64 += si->connection_flushes;
     5f4:	e2a12000 	adc	r2, r1, #0
     5f8:	e2801c82 	add	r1, r0, #33280	@ 0x8200
     5fc:	e50b4030 	str	r4, [fp, #-48]	@ 0xffffffd0
     600:	e2811060 	add	r1, r1, #96	@ 0x60
     604:	e5832004 	str	r2, [r3, #4]
	si->packets_forwarded64 += si->packets_forwarded;
     608:	e5143008 	ldr	r3, [r4, #-8]
     60c:	e59e204c 	ldr	r2, [lr, #76]	@ 0x4c
     610:	e0933002 	adds	r3, r3, r2
     614:	e1a02005 	mov	r2, r5
     618:	e5043008 	str	r3, [r4, #-8]
     61c:	e5143004 	ldr	r3, [r4, #-4]
     620:	e2a33000 	adc	r3, r3, #0
     624:	e5043004 	str	r3, [r4, #-4]
	si->connection_create_requests = 0;
     628:	e3083030 	movw	r3, #32816	@ 0x8030
     62c:	e18080f3 	strd	r8, [r0, r3]
	si->packets_forwarded = 0;
	si->packets_not_forwarded64 += si->packets_not_forwarded;
	si->packets_not_forwarded = 0;
     630:	e1a03004 	mov	r3, r4
	si->connection_create_requests = 0;
     634:	e14a80f8 	strd	r8, [sl, #-8]
     638:	e18080f7 	strd	r8, [r0, r7]
     63c:	e14580f8 	strd	r8, [r5, #-8]
	si->packets_not_forwarded64 += si->packets_not_forwarded;
     640:	e7905006 	ldr	r5, [r0, r6]
     644:	e59e7050 	ldr	r7, [lr, #80]	@ 0x50
     648:	e0955007 	adds	r5, r5, r7
     64c:	e5947004 	ldr	r7, [r4, #4]
     650:	e7805006 	str	r5, [r0, r6]
     654:	e2a70000 	adc	r0, r7, #0
     658:	e5840004 	str	r0, [r4, #4]
	si->packets_not_forwarded = 0;
     65c:	e58ec050 	str	ip, [lr, #80]	@ 0x50

	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
		si->exception_events64[i] += si->exception_events[i];
     660:	e5b30008 	ldr	r0, [r3, #8]!
     664:	e5b24004 	ldr	r4, [r2, #4]!
     668:	e593e004 	ldr	lr, [r3, #4]
     66c:	e0900004 	adds	r0, r0, r4
     670:	e5830000 	str	r0, [r3]
     674:	e2ae0000 	adc	r0, lr, #0
	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
     678:	e1530001 	cmp	r3, r1
		si->exception_events64[i] += si->exception_events[i];
     67c:	e5830004 	str	r0, [r3, #4]
		si->exception_events[i] = 0;
     680:	e582c000 	str	ip, [r2]
	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
     684:	1afffff5 	bne	660 <sfe_ipv6_update_summary_stats+0x170>
	}
}
     688:	e24bd028 	sub	sp, fp, #40	@ 0x28
     68c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

00000690 <sfe_ipv6_debug_dev_read>:
/*
 * sfe_ipv6_debug_dev_read()
 *	Send info to userspace upon read request from user
 */
static ssize_t sfe_ipv6_debug_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
     690:	e1a0c00d 	mov	ip, sp
     694:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
     698:	e24cb004 	sub	fp, ip, #4
     69c:	e24ddfc7 	sub	sp, sp, #796	@ 0x31c
	char msg[CHAR_DEV_MSG_SIZE];
	int total_read = 0;
	struct sfe_ipv6_debug_xml_write_state *ws;
	struct sfe_ipv6 *si = &__si6;

	ws = (struct sfe_ipv6_debug_xml_write_state *)filp->private_data;
     6a0:	e5904090 	ldr	r4, [r0, #144]	@ 0x90
	int total_read = 0;
     6a4:	e3a0c000 	mov	ip, #0
{
     6a8:	e50b2330 	str	r2, [fp, #-816]	@ 0xfffffcd0
     6ac:	e1a05001 	mov	r5, r1
	int total_read = 0;
     6b0:	e50bc328 	str	ip, [fp, #-808]	@ 0xfffffcd8
	while ((ws->state != SFE_IPV6_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     6b4:	e5943000 	ldr	r3, [r4]
     6b8:	e3530009 	cmp	r3, #9
     6bc:	0a000017 	beq	720 <sfe_ipv6_debug_dev_read+0x90>
		if ((sfe_ipv6_debug_xml_write_methods[ws->state])(si, buffer, msg, &length, &total_read, ws)) {
     6c0:	e3007000 	movw	r7, #0
     6c4:	e3006000 	movw	r6, #0
     6c8:	e3407000 	movt	r7, #0
     6cc:	e3406000 	movt	r6, #0
     6d0:	ea000007 	b	6f4 <sfe_ipv6_debug_dev_read+0x64>
     6d4:	e59c8024 	ldr	r8, [ip, #36]	@ 0x24
     6d8:	e24bcfca 	sub	ip, fp, #808	@ 0x328
     6dc:	e58dc000 	str	ip, [sp]
     6e0:	e58d4004 	str	r4, [sp, #4]
     6e4:	e12fff38 	blx	r8
	while ((ws->state != SFE_IPV6_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     6e8:	e5943000 	ldr	r3, [r4]
     6ec:	e3530009 	cmp	r3, #9
     6f0:	0a000007 	beq	714 <sfe_ipv6_debug_dev_read+0x84>
     6f4:	e51be330 	ldr	lr, [fp, #-816]	@ 0xfffffcd0
		if ((sfe_ipv6_debug_xml_write_methods[ws->state])(si, buffer, msg, &length, &total_read, ws)) {
     6f8:	e087c103 	add	ip, r7, r3, lsl #2
     6fc:	e24b2fc9 	sub	r2, fp, #804	@ 0x324
     700:	e24b3e33 	sub	r3, fp, #816	@ 0x330
     704:	e1a01005 	mov	r1, r5
     708:	e1a00006 	mov	r0, r6
	while ((ws->state != SFE_IPV6_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     70c:	e35e0c03 	cmp	lr, #768	@ 0x300
     710:	8affffef 	bhi	6d4 <sfe_ipv6_debug_dev_read+0x44>
			continue;
		}
	}

	return total_read;
     714:	e51b0328 	ldr	r0, [fp, #-808]	@ 0xfffffcd8
}
     718:	e24bd020 	sub	sp, fp, #32
     71c:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
	while ((ws->state != SFE_IPV6_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
     720:	e1a0000c 	mov	r0, ip
     724:	eafffffb 	b	718 <sfe_ipv6_debug_dev_read+0x88>

00000728 <fast_classifier_nl_genl_msg_DUMP>:
 * fast_classifier_nl_genl_msg_DUMP()
 *	ignore fast_classifier_messages OFFLOADED and DONE
 */
static int fast_classifier_nl_genl_msg_DUMP(struct sk_buff *skb,
					    struct netlink_callback *cb)
{
     728:	e1a0c00d 	mov	ip, sp
     72c:	e92dd800 	push	{fp, ip, lr, pc}
     730:	e24cb004 	sub	fp, ip, #4
	return 0;
}
     734:	e3a00000 	mov	r0, #0
     738:	e24bd00c 	sub	sp, fp, #12
     73c:	e89da800 	ldm	sp, {fp, sp, pc}

00000740 <sfe_ipv4_process_tcp_option_sack>:
{
     740:	e1a0c00d 	mov	ip, sp
     744:	e92dd870 	push	{r4, r5, r6, fp, ip, lr, pc}
     748:	e24cb004 	sub	fp, ip, #4
	if (likely(data_offs == length + TCPOLEN_TIMESTAMP + 1 + 1)
     74c:	e3510020 	cmp	r1, #32
     750:	1a00000e 	bne	790 <sfe_ipv4_process_tcp_option_sack+0x50>
	    && likely(ptr[0] == TCPOPT_NOP)
     754:	e5d03014 	ldrb	r3, [r0, #20]
     758:	e3530001 	cmp	r3, #1
     75c:	1a00000d 	bne	798 <sfe_ipv4_process_tcp_option_sack+0x58>
	    && likely(ptr[1] == TCPOPT_NOP)
     760:	e5d03015 	ldrb	r3, [r0, #21]
     764:	e3530001 	cmp	r3, #1
     768:	1a00000a 	bne	798 <sfe_ipv4_process_tcp_option_sack+0x58>
	    && likely(ptr[2] == TCPOPT_TIMESTAMP)
     76c:	e5d03016 	ldrb	r3, [r0, #22]
     770:	e3530008 	cmp	r3, #8
     774:	1a000007 	bne	798 <sfe_ipv4_process_tcp_option_sack+0x58>
	    && likely(ptr[3] == TCPOLEN_TIMESTAMP)) {
     778:	e5d03017 	ldrb	r3, [r0, #23]
     77c:	e353000a 	cmp	r3, #10
     780:	1a000004 	bne	798 <sfe_ipv4_process_tcp_option_sack+0x58>
		return true;
     784:	e3a00001 	mov	r0, #1
}
     788:	e24bd018 	sub	sp, fp, #24
     78c:	e89da870 	ldm	sp, {r4, r5, r6, fp, sp, pc}
	while (length < data_offs) {
     790:	e3510014 	cmp	r1, #20
     794:	9afffffa 	bls	784 <sfe_ipv4_process_tcp_option_sack+0x44>
     798:	e3a03014 	mov	r3, #20
     79c:	ea000007 	b	7c0 <sfe_ipv4_process_tcp_option_sack+0x80>
		if (kind == TCPOPT_EOL) {
     7a0:	e35c0000 	cmp	ip, #0
     7a4:	0afffff6 	beq	784 <sfe_ipv4_process_tcp_option_sack+0x44>
		size = *(ptr + 1);
     7a8:	e5d5c001 	ldrb	ip, [r5, #1]
		if (size < 2) {
     7ac:	e35c0001 	cmp	ip, #1
     7b0:	9a000020 	bls	838 <sfe_ipv4_process_tcp_option_sack+0xf8>
		length += size;
     7b4:	e083300c 	add	r3, r3, ip
	while (length < data_offs) {
     7b8:	e1530001 	cmp	r3, r1
     7bc:	2afffff0 	bcs	784 <sfe_ipv4_process_tcp_option_sack+0x44>
		kind = *ptr;
     7c0:	e7d0c003 	ldrb	ip, [r0, r3]
		ptr = (u8 *)th + length;
     7c4:	e0805003 	add	r5, r0, r3
		if (kind == TCPOPT_NOP) {
     7c8:	e35c0001 	cmp	ip, #1
			length++;
     7cc:	02833001 	addeq	r3, r3, #1
		if (kind == TCPOPT_NOP) {
     7d0:	0afffff8 	beq	7b8 <sfe_ipv4_process_tcp_option_sack+0x78>
		if (kind == TCPOPT_SACK) {
     7d4:	e35c0005 	cmp	ip, #5
     7d8:	1afffff0 	bne	7a0 <sfe_ipv4_process_tcp_option_sack+0x60>
			size = *(ptr + 1);
     7dc:	e5d56001 	ldrb	r6, [r5, #1]
			if ((size < (1 + 1 + TCPOLEN_SACK_PERBLOCK))
     7e0:	e3560009 	cmp	r6, #9
     7e4:	9a000013 	bls	838 <sfe_ipv4_process_tcp_option_sack+0xf8>
			    || ((size - (1 + 1)) % (TCPOLEN_SACK_PERBLOCK))
     7e8:	e246e002 	sub	lr, r6, #2
     7ec:	e21ee007 	ands	lr, lr, #7
     7f0:	1a000010 	bne	838 <sfe_ipv4_process_tcp_option_sack+0xf8>
			    || (size > (data_offs - length))) {
     7f4:	e041c003 	sub	ip, r1, r3
     7f8:	e156000c 	cmp	r6, ip
     7fc:	8a00000d 	bhi	838 <sfe_ipv4_process_tcp_option_sack+0xf8>
			re += 4;
     800:	e3a0c006 	mov	ip, #6
				sack_re = (sptr[0] << 24) | (sptr[1] << 16) | (sptr[2] << 8) | sptr[3];
     804:	e795400c 	ldr	r4, [r5, ip]
     808:	e28cc008 	add	ip, ip, #8
     80c:	e6efc07c 	uxtb	ip, ip
     810:	e6bf4f34 	rev	r4, r4
				if (sack_re > sack) {
     814:	e15e0004 	cmp	lr, r4
     818:	31a0e004 	movcc	lr, r4
			while (re < size) {
     81c:	e156000c 	cmp	r6, ip
     820:	8afffff7 	bhi	804 <sfe_ipv4_process_tcp_option_sack+0xc4>
			if (sack > *ack) {
     824:	e592c000 	ldr	ip, [r2]
			length += size;
     828:	e0833006 	add	r3, r3, r6
			if (sack > *ack) {
     82c:	e15e000c 	cmp	lr, ip
				*ack = sack;
     830:	8582e000 	strhi	lr, [r2]
			continue;
     834:	eaffffdf 	b	7b8 <sfe_ipv4_process_tcp_option_sack+0x78>
				return false;
     838:	e3a00000 	mov	r0, #0
}
     83c:	e24bd018 	sub	sp, fp, #24
     840:	e89da870 	ldm	sp, {r4, r5, r6, fp, sp, pc}

00000844 <sfe_dev_get_master>:
/*
 * sfe_dev_get_master
 * 	get master of bridge port, and hold it
 */
static inline struct net_device *sfe_dev_get_master(struct net_device *dev)
{
     844:	e1a0c00d 	mov	ip, sp
     848:	e92dd800 	push	{fp, ip, lr, pc}
     84c:	e24cb004 	sub	fp, ip, #4
	struct net_device *master;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
	rcu_read_lock();
	master = netdev_master_upper_dev_get_rcu(dev);
     850:	ebfffffe 	bl	0 <netdev_master_upper_dev_get_rcu>
	if (master)
     854:	e3500000 	cmp	r0, #0
     858:	0a000007 	beq	87c <sfe_dev_get_master+0x38>
#define arch_local_irq_save arch_local_irq_save
static inline unsigned long arch_local_irq_save(void)
{
	unsigned long flags;

	asm volatile(
     85c:	e10fc000 	mrs	ip, CPSR
     860:	f10c0080 	cpsid	i

static inline void __dev_hold(struct net_device *dev)
{
	if (dev) {
#ifdef CONFIG_PCPU_DEV_REFCNT
		this_cpu_inc(*dev->pcpu_refcnt);
     864:	e5903310 	ldr	r3, [r0, #784]	@ 0x310
	/*
	 * Read TPIDRPRW.
	 * We want to allow caching the value, so avoid using volatile and
	 * instead use a fake stack read to hazard against barrier().
	 */
	asm("0:	mrc p15, 0, %0, c13, c0, 4			\n\t"
     868:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
     86c:	e7932001 	ldr	r2, [r3, r1]
     870:	e2822001 	add	r2, r2, #1
     874:	e7832001 	str	r2, [r3, r1]
 * restore saved IRQ & FIQ state
 */
#define arch_local_irq_restore arch_local_irq_restore
static inline void arch_local_irq_restore(unsigned long flags)
{
	asm volatile(
     878:	e121f00c 	msr	CPSR_c, ip
	master = dev->master;
	if (master)
		dev_hold(master);
#endif
	return master;
}
     87c:	e24bd00c 	sub	sp, fp, #12
     880:	e89da800 	ldm	sp, {fp, sp, pc}

00000884 <sfe_ipv4_debug_dev_release>:

/*
 * sfe_ipv4_debug_dev_release()
 */
static int sfe_ipv4_debug_dev_release(struct inode *inode, struct file *file)
{
     884:	e1a0c00d 	mov	ip, sp
     888:	e92dd800 	push	{fp, ip, lr, pc}
     88c:	e24cb004 	sub	fp, ip, #4
	struct sfe_ipv4_debug_xml_write_state *ws;

	ws = (struct sfe_ipv4_debug_xml_write_state *)file->private_data;
     890:	e5910090 	ldr	r0, [r1, #144]	@ 0x90
	if (ws) {
     894:	e3500000 	cmp	r0, #0
     898:	0a000000 	beq	8a0 <sfe_ipv4_debug_dev_release+0x1c>
		/*
		 * We've finished with our output so free the write state.
		 */
		kfree(ws);
     89c:	ebfffffe 	bl	0 <kfree>
	}

	return 0;
}
     8a0:	e3a00000 	mov	r0, #0
     8a4:	e24bd00c 	sub	sp, fp, #12
     8a8:	e89da800 	ldm	sp, {fp, sp, pc}

000008ac <fast_classifier_send_genl_msg>:
{
     8ac:	e1a0c00d 	mov	ip, sp
     8b0:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
     8b4:	e24cb004 	sub	fp, ip, #4
     8b8:	e24dd00c 	sub	sp, sp, #12
	buf_len = fast_classifier_gnl_family.hdrsize;
     8bc:	e3005000 	movw	r5, #0
 * This function is a convenient wrapper around __alloc_skb().
 */
static inline struct sk_buff *alloc_skb(unsigned int size,
					gfp_t priority)
{
	return __alloc_skb(size, priority, 0, NUMA_NO_NODE);
     8c0:	e3e03000 	mvn	r3, #0
     8c4:	e3405000 	movt	r5, #0
     8c8:	e3a02000 	mov	r2, #0
 * nlmsg_total_size - length of netlink message including padding
 * @payload: length of message payload
 */
static inline int nlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(nlmsg_msg_size(payload));
     8cc:	e595c004 	ldr	ip, [r5, #4]
{
     8d0:	e1a06000 	mov	r6, r0
     8d4:	e1a07001 	mov	r7, r1
     8d8:	e3a01ea2 	mov	r1, #2592	@ 0xa20
     8dc:	e28cc04b 	add	ip, ip, #75	@ 0x4b
 * genlmsg_total_size - length of genetlink message including padding
 * @payload: length of message payload
 */
static inline int genlmsg_total_size(int payload)
{
	return NLMSG_ALIGN(genlmsg_msg_size(payload));
     8e0:	e3ccc003 	bic	ip, ip, #3
     8e4:	e28c0014 	add	r0, ip, #20
     8e8:	ebfffffe 	bl	0 <__alloc_skb>
	if (!skb)
     8ec:	e2504000 	subs	r4, r0, #0
     8f0:	0a00002b 	beq	9a4 <fast_classifier_send_genl_msg+0xf8>
	msg_head = genlmsg_put(skb, 0, 0, &fast_classifier_gnl_family, 0, msg);
     8f4:	e6ef3076 	uxtb	r3, r6
     8f8:	e3a02000 	mov	r2, #0
     8fc:	e1a01002 	mov	r1, r2
     900:	e1cd20f0 	strd	r2, [sp]
     904:	e1a03005 	mov	r3, r5
     908:	ebfffffe 	bl	0 <genlmsg_put>
	if (!msg_head) {
     90c:	e2508000 	subs	r8, r0, #0
     910:	0a00002b 	beq	9c4 <fast_classifier_send_genl_msg+0x118>
	rc = nla_put(skb, FAST_CLASSIFIER_A_TUPLE, sizeof(struct fast_classifier_tuple), fc_msg);
     914:	e3a02034 	mov	r2, #52	@ 0x34
     918:	e3a01001 	mov	r1, #1
     91c:	e1a03007 	mov	r3, r7
     920:	e1a00004 	mov	r0, r4
     924:	ebfffffe 	bl	0 <nla_put>
	if (rc != 0) {
     928:	e2502000 	subs	r2, r0, #0
		nlmsg_cancel(skb, hdr - GENL_HDRLEN - NLMSG_HDRLEN);
     92c:	e2481014 	sub	r1, r8, #20
     930:	1a00001d 	bne	9ac <fast_classifier_send_genl_msg+0x100>
 * attributes. Only necessary if attributes have been added to
 * the message.
 */
static inline void nlmsg_end(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	nlh->nlmsg_len = skb_tail_pointer(skb) - (unsigned char *)nlh;
     934:	e59430a4 	ldr	r3, [r4, #164]	@ 0xa4
     938:	e0433001 	sub	r3, r3, r1
     93c:	e5083014 	str	r3, [r8, #-20]	@ 0xffffffec
	if (WARN_ON_ONCE(group >= family->n_mcgrps))
     940:	e5d53027 	ldrb	r3, [r5, #39]	@ 0x27
     944:	e3530000 	cmp	r3, #0
     948:	0a00002d 	beq	a04 <fast_classifier_send_genl_msg+0x158>
	group = family->mcgrp_offset + group;
     94c:	e5953020 	ldr	r3, [r5, #32]
{
	int err;

	NETLINK_CB(skb).dst_group = group;

	err = netlink_broadcast(sk, skb, portid, group, flags);
     950:	e3a0cea2 	mov	ip, #2592	@ 0xa20
	return nlmsg_multicast(net->genl_sock, skb, portid, group, flags);
     954:	e3001000 	movw	r1, #0
     958:	e3401000 	movt	r1, #0
     95c:	e59100a8 	ldr	r0, [r1, #168]	@ 0xa8
     960:	e1a01004 	mov	r1, r4
	NETLINK_CB(skb).dst_group = group;
     964:	e5843028 	str	r3, [r4, #40]	@ 0x28
	err = netlink_broadcast(sk, skb, portid, group, flags);
     968:	e58dc000 	str	ip, [sp]
     96c:	ebfffffe 	bl	0 <netlink_broadcast>
	switch (msg) {
     970:	e3560003 	cmp	r6, #3
     974:	1a000017 	bne	9d8 <fast_classifier_send_genl_msg+0x12c>
		if (rc == 0) {
     978:	e3500000 	cmp	r0, #0
     97c:	ba00002c 	blt	a34 <fast_classifier_send_genl_msg+0x188>

#if __LINUX_ARM_ARCH__ >= 7 && defined(CONFIG_SMP)
#define ARCH_HAS_PREFETCHW
static inline void prefetchw(const void *ptr)
{
	__asm__ __volatile__(
     980:	e3003000 	movw	r3, #0
     984:	e3403000 	movt	r3, #0
     988:	e2832008 	add	r2, r3, #8
     98c:	f592f000 	pldw	[r2]
#define ATOMIC_OPS(op, c_op, asm_op)					\
	ATOMIC_OP(op, c_op, asm_op)					\
	ATOMIC_OP_RETURN(op, c_op, asm_op)				\
	ATOMIC_FETCH_OP(op, c_op, asm_op)

ATOMIC_OPS(add, +=, add)
     990:	e1921f9f 	ldrex	r1, [r2]
     994:	e2811001 	add	r1, r1, #1
     998:	e1820f91 	strex	r0, r1, [r2]
     99c:	e3300000 	teq	r0, #0
     9a0:	1afffffa 	bne	990 <fast_classifier_send_genl_msg+0xe4>
}
     9a4:	e24bd020 	sub	sp, fp, #32
     9a8:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
	if (mark) {
     9ac:	e3510000 	cmp	r1, #0
     9b0:	0a000003 	beq	9c4 <fast_classifier_send_genl_msg+0x118>
		skb_trim(skb, (unsigned char *) mark - skb->data);
     9b4:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
     9b8:	e1a00004 	mov	r0, r4
     9bc:	e0411003 	sub	r1, r1, r3
     9c0:	ebfffffe 	bl	0 <skb_trim>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
     9c4:	e3a01001 	mov	r1, #1
     9c8:	e1a00004 	mov	r0, r4
     9cc:	ebfffffe 	bl	0 <kfree_skb_reason>
     9d0:	e24bd020 	sub	sp, fp, #32
     9d4:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
		if (rc == 0) {
     9d8:	e3500000 	cmp	r0, #0
     9dc:	ba00000a 	blt	a0c <fast_classifier_send_genl_msg+0x160>
     9e0:	e3003000 	movw	r3, #0
     9e4:	e3403000 	movt	r3, #0
     9e8:	f593f000 	pldw	[r3]
     9ec:	e1932f9f 	ldrex	r2, [r3]
     9f0:	e2822001 	add	r2, r2, #1
     9f4:	e1831f92 	strex	r1, r2, [r3]
     9f8:	e3310000 	teq	r1, #0
     9fc:	1afffffa 	bne	9ec <fast_classifier_send_genl_msg+0x140>
     a00:	eaffffe7 	b	9a4 <fast_classifier_send_genl_msg+0xf8>
	switch (msg) {
     a04:	e3560003 	cmp	r6, #3
     a08:	0a000009 	beq	a34 <fast_classifier_send_genl_msg+0x188>
     a0c:	e3003000 	movw	r3, #0
     a10:	e3403000 	movt	r3, #0
     a14:	e2832004 	add	r2, r3, #4
     a18:	f592f000 	pldw	[r2]
     a1c:	e1921f9f 	ldrex	r1, [r2]
     a20:	e2811001 	add	r1, r1, #1
     a24:	e1820f91 	strex	r0, r1, [r2]
     a28:	e3300000 	teq	r0, #0
     a2c:	1afffffa 	bne	a1c <fast_classifier_send_genl_msg+0x170>
     a30:	eaffffdb 	b	9a4 <fast_classifier_send_genl_msg+0xf8>
     a34:	e3003000 	movw	r3, #0
     a38:	e3403000 	movt	r3, #0
     a3c:	e283200c 	add	r2, r3, #12
     a40:	f592f000 	pldw	[r2]
     a44:	e1921f9f 	ldrex	r1, [r2]
     a48:	e2811001 	add	r1, r1, #1
     a4c:	e1820f91 	strex	r0, r1, [r2]
     a50:	e3300000 	teq	r0, #0
     a54:	1afffffa 	bne	a44 <fast_classifier_send_genl_msg+0x198>
     a58:	eaffffd1 	b	9a4 <fast_classifier_send_genl_msg+0xf8>

00000a5c <sfe_ipv4_debug_dev_open>:
{
     a5c:	e1a0c00d 	mov	ip, sp
     a60:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
     a64:	e24cb004 	sub	fp, ip, #4
	if (!ws) {
     a68:	e5915090 	ldr	r5, [r1, #144]	@ 0x90
{
     a6c:	e1a04001 	mov	r4, r1
	if (!ws) {
     a70:	e3550000 	cmp	r5, #0
     a74:	0a000002 	beq	a84 <sfe_ipv4_debug_dev_open+0x28>
	return 0;
     a78:	e3a00000 	mov	r0, #0
}
     a7c:	e24bd014 	sub	sp, fp, #20
     a80:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
		index = kmalloc_index(size);

		if (!index)
			return ZERO_SIZE_PTR;

		return kmalloc_trace(
     a84:	e3003000 	movw	r3, #0
     a88:	e3a02008 	mov	r2, #8
     a8c:	e3403000 	movt	r3, #0
     a90:	e3a01d37 	mov	r1, #3520	@ 0xdc0
     a94:	e5930018 	ldr	r0, [r3, #24]
     a98:	ebfffffe 	bl	0 <kmalloc_trace>
		if (!ws) {
     a9c:	e3500000 	cmp	r0, #0
     aa0:	0a000002 	beq	ab0 <sfe_ipv4_debug_dev_open+0x54>
		ws->state = SFE_IPV4_DEBUG_XML_STATE_START;
     aa4:	e5805000 	str	r5, [r0]
		file->private_data = ws;
     aa8:	e5840090 	str	r0, [r4, #144]	@ 0x90
     aac:	eafffff1 	b	a78 <sfe_ipv4_debug_dev_open+0x1c>
			return -ENOMEM;
     ab0:	e3e0000b 	mvn	r0, #11
     ab4:	eafffff0 	b	a7c <sfe_ipv4_debug_dev_open+0x20>

00000ab8 <sfe_ipv6_debug_dev_open>:

/*
 * sfe_ipv6_debug_dev_open()
 */
static int sfe_ipv6_debug_dev_open(struct inode *inode, struct file *file)
{
     ab8:	e1a0c00d 	mov	ip, sp
     abc:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
     ac0:	e24cb004 	sub	fp, ip, #4
	struct sfe_ipv6_debug_xml_write_state *ws;

	ws = (struct sfe_ipv6_debug_xml_write_state *)file->private_data;
	if (ws) {
     ac4:	e5915090 	ldr	r5, [r1, #144]	@ 0x90
{
     ac8:	e1a04001 	mov	r4, r1
	if (ws) {
     acc:	e3550000 	cmp	r5, #0
     ad0:	0a000002 	beq	ae0 <sfe_ipv6_debug_dev_open+0x28>
		return 0;
     ad4:	e3a00000 	mov	r0, #0

	ws->state = SFE_IPV6_DEBUG_XML_STATE_START;
	file->private_data = ws;

	return 0;
}
     ad8:	e24bd014 	sub	sp, fp, #20
     adc:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
     ae0:	e3003000 	movw	r3, #0
     ae4:	e3a02008 	mov	r2, #8
     ae8:	e3403000 	movt	r3, #0
     aec:	e3a01d37 	mov	r1, #3520	@ 0xdc0
     af0:	e5930018 	ldr	r0, [r3, #24]
     af4:	ebfffffe 	bl	0 <kmalloc_trace>
	if (!ws) {
     af8:	e3500000 	cmp	r0, #0
     afc:	0a000002 	beq	b0c <sfe_ipv6_debug_dev_open+0x54>
	ws->state = SFE_IPV6_DEBUG_XML_STATE_START;
     b00:	e5805000 	str	r5, [r0]
	file->private_data = ws;
     b04:	e5840090 	str	r0, [r4, #144]	@ 0x90
	return 0;
     b08:	eafffff1 	b	ad4 <sfe_ipv6_debug_dev_open+0x1c>
		return -ENOMEM;
     b0c:	e3e0000b 	mvn	r0, #11
     b10:	eafffff0 	b	ad8 <sfe_ipv6_debug_dev_open+0x20>

00000b14 <fast_classifier_get_defunct_all>:
 * 	dump state of SFE
 */
static ssize_t fast_classifier_get_defunct_all(struct device *dev,
                                      struct device_attribute *attr,
                                      char *buf)
{
     b14:	e1a0c00d 	mov	ip, sp
     b18:	e92dd800 	push	{fp, ip, lr, pc}
     b1c:	e24cb004 	sub	fp, ip, #4
     b20:	e1a00002 	mov	r0, r2
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", 0);
     b24:	e3002000 	movw	r2, #0
     b28:	e3a03000 	mov	r3, #0
     b2c:	e3402000 	movt	r2, #0
     b30:	e3a01a01 	mov	r1, #4096	@ 0x1000
     b34:	ebfffffe 	bl	0 <snprintf>
}
     b38:	e3a00002 	mov	r0, #2
     b3c:	e24bd00c 	sub	sp, fp, #12
     b40:	e89da800 	ldm	sp, {fp, sp, pc}

00000b44 <fast_classifier_get_stop>:
{
     b44:	e1a0c00d 	mov	ip, sp
     b48:	e92dd800 	push	{fp, ip, lr, pc}
     b4c:	e24cb004 	sub	fp, ip, #4
     b50:	e1a00002 	mov	r0, r2
	fast_recv = rcu_dereference(fast_nat_recv);
     b54:	e3003000 	movw	r3, #0
     b58:	e3403000 	movt	r3, #0
     b5c:	e5933000 	ldr	r3, [r3]
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", fast_recv ? 0 : 1);
     b60:	e3002000 	movw	r2, #0
     b64:	e16f3f13 	clz	r3, r3
     b68:	e1a032a3 	lsr	r3, r3, #5
     b6c:	e3a01a01 	mov	r1, #4096	@ 0x1000
     b70:	e3402000 	movt	r2, #0
     b74:	ebfffffe 	bl	0 <snprintf>
}
     b78:	e3a00002 	mov	r0, #2
     b7c:	e24bd00c 	sub	sp, fp, #12
     b80:	e89da800 	ldm	sp, {fp, sp, pc}

00000b84 <fast_classifier_get_skip_bridge_ingress>:
{
     b84:	e1a0c00d 	mov	ip, sp
     b88:	e92dd800 	push	{fp, ip, lr, pc}
     b8c:	e24cb004 	sub	fp, ip, #4
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", skip_to_bridge_ingress);
     b90:	e3003000 	movw	r3, #0
{
     b94:	e1a00002 	mov	r0, r2
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", skip_to_bridge_ingress);
     b98:	e3403000 	movt	r3, #0
     b9c:	e3002000 	movw	r2, #0
     ba0:	e5d33010 	ldrb	r3, [r3, #16]
     ba4:	e3402000 	movt	r2, #0
     ba8:	e3a01a01 	mov	r1, #4096	@ 0x1000
     bac:	ebfffffe 	bl	0 <snprintf>
}
     bb0:	e3a00002 	mov	r0, #2
     bb4:	e24bd00c 	sub	sp, fp, #12
     bb8:	e89da800 	ldm	sp, {fp, sp, pc}

00000bbc <fast_classifier_get_offload_at_pkts>:
{
     bbc:	e1a0c00d 	mov	ip, sp
     bc0:	e92dd800 	push	{fp, ip, lr, pc}
     bc4:	e24cb004 	sub	fp, ip, #4
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", offload_at_pkts);
     bc8:	e3003000 	movw	r3, #0
{
     bcc:	e1a00002 	mov	r0, r2
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", offload_at_pkts);
     bd0:	e3403000 	movt	r3, #0
     bd4:	e3002000 	movw	r2, #0
     bd8:	e5933048 	ldr	r3, [r3, #72]	@ 0x48
     bdc:	e3402000 	movt	r2, #0
     be0:	e3a01a01 	mov	r1, #4096	@ 0x1000
     be4:	ebfffffe 	bl	0 <snprintf>
}
     be8:	e24bd00c 	sub	sp, fp, #12
     bec:	e89da800 	ldm	sp, {fp, sp, pc}

00000bf0 <sfe_ipv4_get_debug_dev>:
{
     bf0:	e1a0c00d 	mov	ip, sp
     bf4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
     bf8:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock(&lock->rlock);
}

static __always_inline void spin_lock_bh(spinlock_t *lock)
{
	raw_spin_lock_bh(&lock->rlock);
     bfc:	e3004000 	movw	r4, #0
     c00:	e1a05002 	mov	r5, r2
     c04:	e3404000 	movt	r4, #0
     c08:	e1a00004 	mov	r0, r4
     c0c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	num = si->debug_dev;
     c10:	e2843902 	add	r3, r4, #32768	@ 0x8000
	raw_spin_unlock(&lock->rlock);
}

static __always_inline void spin_unlock_bh(spinlock_t *lock)
{
	raw_spin_unlock_bh(&lock->rlock);
     c14:	e1a00004 	mov	r0, r4
     c18:	e593425c 	ldr	r4, [r3, #604]	@ 0x25c
     c1c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", num);
     c20:	e3002000 	movw	r2, #0
     c24:	e3a01a01 	mov	r1, #4096	@ 0x1000
     c28:	e1a00005 	mov	r0, r5
     c2c:	e3402000 	movt	r2, #0
     c30:	e1a03004 	mov	r3, r4
     c34:	ebfffffe 	bl	0 <snprintf>
}
     c38:	e24bd014 	sub	sp, fp, #20
     c3c:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

00000c40 <sfe_ipv6_get_debug_dev>:
{
     c40:	e1a0c00d 	mov	ip, sp
     c44:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
     c48:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock_bh(&lock->rlock);
     c4c:	e3004000 	movw	r4, #0
     c50:	e1a05002 	mov	r5, r2
     c54:	e3404000 	movt	r4, #0
     c58:	e1a00004 	mov	r0, r4
     c5c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	num = si->debug_dev;
     c60:	e2843902 	add	r3, r4, #32768	@ 0x8000
	raw_spin_unlock_bh(&lock->rlock);
     c64:	e1a00004 	mov	r0, r4
     c68:	e593426c 	ldr	r4, [r3, #620]	@ 0x26c
     c6c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", num);
     c70:	e3002000 	movw	r2, #0
     c74:	e3a01a01 	mov	r1, #4096	@ 0x1000
     c78:	e1a00005 	mov	r0, r5
     c7c:	e3402000 	movt	r2, #0
     c80:	e1a03004 	mov	r3, r4
     c84:	ebfffffe 	bl	0 <snprintf>
}
     c88:	e24bd014 	sub	sp, fp, #20
     c8c:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

00000c90 <fast_classifier_set_skip_bridge_ingress>:
{
     c90:	e1a0c00d 	mov	ip, sp
     c94:	e92dd810 	push	{r4, fp, ip, lr, pc}
     c98:	e24cb004 	sub	fp, ip, #4
     c9c:	e24dd00c 	sub	sp, sp, #12
     ca0:	e1a00002 	mov	r0, r2
	 */
	if (sizeof(long) == sizeof(long long) &&
	    __alignof__(long) == __alignof__(long long))
		return kstrtoll(s, base, (long long *)res);
	else
		return _kstrtol(s, base, res);
     ca4:	e3a01000 	mov	r1, #0
     ca8:	e24b2018 	sub	r2, fp, #24
     cac:	e1a04003 	mov	r4, r3
     cb0:	ebfffffe 	bl	0 <_kstrtol>
	if (ret == -EINVAL || ((int)new != new))
     cb4:	e3700016 	cmn	r0, #22
     cb8:	0a000006 	beq	cd8 <fast_classifier_set_skip_bridge_ingress+0x48>
	skip_to_bridge_ingress = new ? 1 : 0;
     cbc:	e51b2018 	ldr	r2, [fp, #-24]	@ 0xffffffe8
     cc0:	e3003000 	movw	r3, #0
	return size;
     cc4:	e1a00004 	mov	r0, r4
	skip_to_bridge_ingress = new ? 1 : 0;
     cc8:	e3403000 	movt	r3, #0
     ccc:	e2522000 	subs	r2, r2, #0
     cd0:	13a02001 	movne	r2, #1
     cd4:	e5c32010 	strb	r2, [r3, #16]
}
     cd8:	e24bd010 	sub	sp, fp, #16
     cdc:	e89da810 	ldm	sp, {r4, fp, sp, pc}

00000ce0 <fast_classifier_set_offload_at_pkts>:
{
     ce0:	e1a0c00d 	mov	ip, sp
     ce4:	e92dd810 	push	{r4, fp, ip, lr, pc}
     ce8:	e24cb004 	sub	fp, ip, #4
     cec:	e24dd00c 	sub	sp, sp, #12
     cf0:	e1a00002 	mov	r0, r2
     cf4:	e3a01000 	mov	r1, #0
     cf8:	e24b2018 	sub	r2, fp, #24
     cfc:	e1a04003 	mov	r4, r3
     d00:	ebfffffe 	bl	0 <_kstrtol>
	if (ret == -EINVAL || ((int)new != new))
     d04:	e3700016 	cmn	r0, #22
     d08:	0a000004 	beq	d20 <fast_classifier_set_offload_at_pkts+0x40>
	offload_at_pkts = new;
     d0c:	e51b2018 	ldr	r2, [fp, #-24]	@ 0xffffffe8
     d10:	e3003000 	movw	r3, #0
	return size;
     d14:	e1a00004 	mov	r0, r4
	offload_at_pkts = new;
     d18:	e3403000 	movt	r3, #0
     d1c:	e5832048 	str	r2, [r3, #72]	@ 0x48
}
     d20:	e24bd010 	sub	sp, fp, #16
     d24:	e89da810 	ldm	sp, {r4, fp, sp, pc}

00000d28 <fast_classifier_get_debug_info>:
{
     d28:	e1a0c00d 	mov	ip, sp
     d2c:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
     d30:	e24cb004 	sub	fp, ip, #4
     d34:	e24dd02c 	sub	sp, sp, #44	@ 0x2c
	raw_spin_lock_bh(&lock->rlock);
     d38:	e3004000 	movw	r4, #0
     d3c:	e1a06002 	mov	r6, r2
     d40:	e59f9118 	ldr	r9, [pc, #280]	@ e60 <fast_classifier_get_debug_info+0x138>
     d44:	e3404000 	movt	r4, #0
		len += scnprintf(buf + len, PAGE_SIZE - len,
     d48:	e3008000 	movw	r8, #0
     d4c:	e2840014 	add	r0, r4, #20
     d50:	e3007000 	movw	r7, #0
     d54:	ebfffffe 	bl	0 <_raw_spin_lock_bh>

static __always_inline int
atomic_read(const atomic_t *v)
{
	instrument_atomic_read(v, sizeof(*v));
	return arch_atomic_read(v);
     d58:	e5943018 	ldr	r3, [r4, #24]
     d5c:	e3408000 	movt	r8, #0
     d60:	e3407000 	movt	r7, #0
     d64:	e289a902 	add	sl, r9, #32768	@ 0x8000
	len += scnprintf(buf, PAGE_SIZE - len, "size=%d offload=%d offload_no_match=%d"
     d68:	e58d3000 	str	r3, [sp]
     d6c:	e594301c 	ldr	r3, [r4, #28]
     d70:	e58d3004 	str	r3, [sp, #4]
     d74:	e5942000 	ldr	r2, [r4]
     d78:	e5940008 	ldr	r0, [r4, #8]
     d7c:	e5941004 	ldr	r1, [r4, #4]
     d80:	e594c00c 	ldr	ip, [r4, #12]
     d84:	e5943020 	ldr	r3, [r4, #32]
     d88:	e58d2008 	str	r2, [sp, #8]
     d8c:	e3002000 	movw	r2, #0
     d90:	e3402000 	movt	r2, #0
     d94:	e1cd00fc 	strd	r0, [sp, #12]
     d98:	e3a01a01 	mov	r1, #4096	@ 0x1000
     d9c:	e1a00006 	mov	r0, r6
     da0:	e58dc014 	str	ip, [sp, #20]
     da4:	ebfffffe 	bl	0 <scnprintf>
     da8:	e1a05000 	mov	r5, r0
	sfe_hash_for_each(fc_conn_ht, i, node, conn, hl) {
     dac:	e5b94004 	ldr	r4, [r9, #4]!
     db0:	e3540000 	cmp	r4, #0
     db4:	0a000022 	beq	e44 <fast_classifier_get_debug_info+0x11c>
				conn->sic->protocol,
     db8:	e5940008 	ldr	r0, [r4, #8]
		len += scnprintf(buf + len, PAGE_SIZE - len,
     dbc:	e594e010 	ldr	lr, [r4, #16]
     dc0:	e5943018 	ldr	r3, [r4, #24]
     dc4:	e5d4201c 	ldrb	r2, [r4, #28]
				ntohs(conn->sic->src_port),
     dc8:	e1d015b8 	ldrh	r1, [r0, #88]	@ 0x58
				ntohs(conn->sic->dest_port),
     dcc:	e1d0c5bc 	ldrh	ip, [r0, #92]	@ 0x5c
		len += scnprintf(buf + len, PAGE_SIZE - len,
     dd0:	e58de020 	str	lr, [sp, #32]
     dd4:	e590e098 	ldr	lr, [r0, #152]	@ 0x98
     dd8:	e3520000 	cmp	r2, #0
				ntohs(conn->sic->src_port),
     ddc:	e6bf1fb1 	rev16	r1, r1
		len += scnprintf(buf + len, PAGE_SIZE - len,
     de0:	01a02008 	moveq	r2, r8
				ntohs(conn->sic->dest_port),
     de4:	e6bfcfbc 	rev16	ip, ip
				ntohs(conn->sic->src_port),
     de8:	e6ff1071 	uxth	r1, r1
				ntohs(conn->sic->dest_port),
     dec:	e6ffc07c 	uxth	ip, ip
		len += scnprintf(buf + len, PAGE_SIZE - len,
     df0:	11a02007 	movne	r2, r7
     df4:	e58d100c 	str	r1, [sp, #12]
     df8:	e2801018 	add	r1, r0, #24
     dfc:	e58de01c 	str	lr, [sp, #28]
				conn->sic->dest_mac_xlate,
     e00:	e280e072 	add	lr, r0, #114	@ 0x72
		len += scnprintf(buf + len, PAGE_SIZE - len,
     e04:	e58d1008 	str	r1, [sp, #8]
     e08:	e2651a01 	rsb	r1, r5, #4096	@ 0x1000
     e0c:	e58dc014 	str	ip, [sp, #20]
     e10:	e280c038 	add	ip, r0, #56	@ 0x38
     e14:	e58de018 	str	lr, [sp, #24]
				conn->sic->src_mac,
     e18:	e280e060 	add	lr, r0, #96	@ 0x60
		len += scnprintf(buf + len, PAGE_SIZE - len,
     e1c:	e58de004 	str	lr, [sp, #4]
     e20:	e58dc010 	str	ip, [sp, #16]
     e24:	e590c000 	ldr	ip, [r0]
     e28:	e0860005 	add	r0, r6, r5
     e2c:	e58dc000 	str	ip, [sp]
     e30:	ebfffffe 	bl	0 <scnprintf>
	sfe_hash_for_each(fc_conn_ht, i, node, conn, hl) {
     e34:	e5944000 	ldr	r4, [r4]
		len += scnprintf(buf + len, PAGE_SIZE - len,
     e38:	e0855000 	add	r5, r5, r0
	sfe_hash_for_each(fc_conn_ht, i, node, conn, hl) {
     e3c:	e3540000 	cmp	r4, #0
     e40:	1affffdc 	bne	db8 <fast_classifier_get_debug_info+0x90>
     e44:	e15a0009 	cmp	sl, r9
     e48:	1affffd7 	bne	dac <fast_classifier_get_debug_info+0x84>
	raw_spin_unlock_bh(&lock->rlock);
     e4c:	e59f0010 	ldr	r0, [pc, #16]	@ e64 <fast_classifier_get_debug_info+0x13c>
     e50:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
     e54:	e1a00005 	mov	r0, r5
     e58:	e24bd028 	sub	sp, fp, #40	@ 0x28
     e5c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
     e60:	0000004c 	.word	0x0000004c
     e64:	00000014 	.word	0x00000014

00000e68 <sfe_addr_equal.part.0>:
 * sfe_addr_equal()
 *	compare ipv4 or ipv6 address
 *
 * return: 1, equal; 0, no equal
 */
static inline int sfe_addr_equal(sfe_ip_addr_t *a,
     e68:	e1a0c00d 	mov	ip, sp
     e6c:	e92dd800 	push	{fp, ip, lr, pc}
     e70:	e24cb004 	sub	fp, ip, #4
	       a->addr[2] == b->addr[2] &&
     e74:	e5902000 	ldr	r2, [r0]
     e78:	e5913000 	ldr	r3, [r1]
     e7c:	e1520003 	cmp	r2, r3
     e80:	0a000002 	beq	e90 <sfe_addr_equal.part.0+0x28>
     e84:	e3a00000 	mov	r0, #0
				 sfe_ip_addr_t *b, int is_v4)
{
	return is_v4 ? sfe_ipv4_addr_equal(a->ip, b->ip) : sfe_ipv6_addr_equal(a->ip6, b->ip6);
}
     e88:	e24bd00c 	sub	sp, fp, #12
     e8c:	e89da800 	ldm	sp, {fp, sp, pc}
	return a->addr[0] == b->addr[0] &&
     e90:	e5902004 	ldr	r2, [r0, #4]
     e94:	e5913004 	ldr	r3, [r1, #4]
     e98:	e1520003 	cmp	r2, r3
     e9c:	1afffff8 	bne	e84 <sfe_addr_equal.part.0+0x1c>
	       a->addr[1] == b->addr[1] &&
     ea0:	e5902008 	ldr	r2, [r0, #8]
     ea4:	e5913008 	ldr	r3, [r1, #8]
     ea8:	e1520003 	cmp	r2, r3
     eac:	1afffff4 	bne	e84 <sfe_addr_equal.part.0+0x1c>
	       a->addr[2] == b->addr[2] &&
     eb0:	e590000c 	ldr	r0, [r0, #12]
     eb4:	e591300c 	ldr	r3, [r1, #12]
     eb8:	e0400003 	sub	r0, r0, r3
     ebc:	e16f0f10 	clz	r0, r0
     ec0:	e1a002a0 	lsr	r0, r0, #5
     ec4:	eaffffef 	b	e88 <sfe_addr_equal.part.0+0x20>

00000ec8 <fast_classifier_set_stop>:
{
     ec8:	e1a0c00d 	mov	ip, sp
     ecc:	e92dd810 	push	{r4, fp, ip, lr, pc}
     ed0:	e24cb004 	sub	fp, ip, #4
     ed4:	e24dd00c 	sub	sp, sp, #12
     ed8:	e1a00002 	mov	r0, r2
	return kstrtoll(s, base, res);
}

static inline int __must_check kstrtou32(const char *s, unsigned int base, u32 *res)
{
	return kstrtouint(s, base, res);
     edc:	e3a01000 	mov	r1, #0
     ee0:	e24b2018 	sub	r2, fp, #24
     ee4:	e1a04003 	mov	r4, r3
     ee8:	ebfffffe 	bl	0 <kstrtouint>
	if (ret)
     eec:	e3500000 	cmp	r0, #0
     ef0:	1a000008 	bne	f18 <fast_classifier_set_stop+0x50>
	if (num) {
     ef4:	e51b3018 	ldr	r3, [fp, #-24]	@ 0xffffffe8
     ef8:	e3530000 	cmp	r3, #0
     efc:	1a000007 	bne	f20 <fast_classifier_set_stop+0x58>
		fast_recv = rcu_dereference(fast_nat_recv);
     f00:	e3003000 	movw	r3, #0
     f04:	e3403000 	movt	r3, #0
     f08:	e5932000 	ldr	r2, [r3]
		if (!fast_recv) {
     f0c:	e3520000 	cmp	r2, #0
     f10:	0a000006 	beq	f30 <fast_classifier_set_stop+0x68>
	return count;
     f14:	e1a00004 	mov	r0, r4
}
     f18:	e24bd010 	sub	sp, fp, #16
     f1c:	e89da810 	ldm	sp, {r4, fp, sp, pc}
		RCU_INIT_POINTER(fast_nat_recv, NULL);
     f20:	e3003000 	movw	r3, #0
     f24:	e3403000 	movt	r3, #0
     f28:	e5830000 	str	r0, [r3]
     f2c:	eafffff8 	b	f14 <fast_classifier_set_stop+0x4c>
			BUG_ON(fast_nat_recv);
     f30:	e5932000 	ldr	r2, [r3]
     f34:	e3520000 	cmp	r2, #0
			RCU_INIT_POINTER(fast_nat_recv, fast_classifier_recv);
     f38:	03002000 	movweq	r2, #0
     f3c:	03402000 	movteq	r2, #0
     f40:	05832000 	streq	r2, [r3]
			BUG_ON(fast_nat_recv);
     f44:	0afffff2 	beq	f14 <fast_classifier_set_stop+0x4c>
     f48:	e7f001f2 	.word	0xe7f001f2

00000f4c <sfe_ipv6_find_connection_match.constprop.0>:
sfe_ipv6_find_connection_match(struct sfe_ipv6 *si, struct net_device *dev, u8 protocol,
     f4c:	e1a0c00d 	mov	ip, sp
     f50:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
     f54:	e24cb004 	sub	fp, ip, #4
     f58:	e24dd01c 	sub	sp, sp, #28
     f5c:	e59bc004 	ldr	ip, [fp, #4]
     f60:	e50b0030 	str	r0, [fp, #-48]	@ 0xffffffd0
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
     f64:	e1c260d0 	ldrd	r6, [r2]
     f68:	e1c280d8 	ldrd	r8, [r2, #8]
sfe_ipv6_find_connection_match(struct sfe_ipv6 *si, struct net_device *dev, u8 protocol,
     f6c:	e1dbe0b8 	ldrh	lr, [fp, #8]
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
     f70:	e59ca000 	ldr	sl, [ip]
     f74:	e1cc40d4 	ldrd	r4, [ip, #4]
     f78:	e59cc00c 	ldr	ip, [ip, #12]
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
     f7c:	e023000e 	eor	r0, r3, lr
     f80:	e6bf0fb0 	rev16	r0, r0
     f84:	e024200a 	eor	r2, r4, sl
     f88:	e6ff0070 	uxth	r0, r0
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
     f8c:	e50b503c 	str	r5, [fp, #-60]	@ 0xffffffc4
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
     f90:	e0222001 	eor	r2, r2, r1
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
     f94:	e50b4038 	str	r4, [fp, #-56]	@ 0xffffffc8
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
     f98:	e0222005 	eor	r2, r2, r5
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
     f9c:	e50bc034 	str	ip, [fp, #-52]	@ 0xffffffcc
	cm = si->conn_match_hash[conn_match_idx];
     fa0:	e3005000 	movw	r5, #0
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
     fa4:	e022200c 	eor	r2, r2, ip
     fa8:	e51bc030 	ldr	ip, [fp, #-48]	@ 0xffffffd0
	cm = si->conn_match_hash[conn_match_idx];
     fac:	e3405000 	movt	r5, #0
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
     fb0:	e0222006 	eor	r2, r2, r6
     fb4:	e0222007 	eor	r2, r2, r7
     fb8:	e0222008 	eor	r2, r2, r8
     fbc:	e0222009 	eor	r2, r2, r9
     fc0:	e022200c 	eor	r2, r2, ip
     fc4:	e0222000 	eor	r2, r2, r0
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
     fc8:	e0222622 	eor	r2, r2, r2, lsr #12
     fcc:	e7ebc052 	ubfx	ip, r2, #0, #12
	cm = si->conn_match_hash[conn_match_idx];
     fd0:	e28cca01 	add	ip, ip, #4096	@ 0x1000
     fd4:	e28cc00c 	add	ip, ip, #12
     fd8:	e795410c 	ldr	r4, [r5, ip, lsl #2]
	if (unlikely(!cm)) {
     fdc:	e3540000 	cmp	r4, #0
     fe0:	0a000071 	beq	11ac <sfe_ipv6_find_connection_match.constprop.0+0x260>
	if ((cm->match_src_port == src_port)
     fe4:	e1d424b4 	ldrh	r2, [r4, #68]	@ 0x44
     fe8:	e1520003 	cmp	r2, r3
     fec:	0a000043 	beq	1100 <sfe_ipv6_find_connection_match.constprop.0+0x1b4>
		cm = cm->next;
     ff0:	e5940000 	ldr	r0, [r4]
	} while (cm && (cm->match_src_port != src_port
     ff4:	e3500000 	cmp	r0, #0
     ff8:	150b1040 	strne	r1, [fp, #-64]	@ 0xffffffc0
     ffc:	1a000003 	bne	1010 <sfe_ipv6_find_connection_match.constprop.0+0xc4>
    1000:	ea00003c 	b	10f8 <sfe_ipv6_find_connection_match.constprop.0+0x1ac>
		cm = cm->next;
    1004:	e5900000 	ldr	r0, [r0]
	} while (cm && (cm->match_src_port != src_port
    1008:	e3500000 	cmp	r0, #0
    100c:	0a000039 	beq	10f8 <sfe_ipv6_find_connection_match.constprop.0+0x1ac>
    1010:	e1d024b4 	ldrh	r2, [r0, #68]	@ 0x44
    1014:	e1520003 	cmp	r2, r3
    1018:	1afffff9 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
		 || cm->match_dest_port != dest_port
    101c:	e1d024b6 	ldrh	r2, [r0, #70]	@ 0x46
    1020:	e152000e 	cmp	r2, lr
    1024:	1afffff6 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
    1028:	e5902024 	ldr	r2, [r0, #36]	@ 0x24
    102c:	e1520006 	cmp	r2, r6
    1030:	1afffff3 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	return a->addr[0] == b->addr[0] &&
    1034:	e5902028 	ldr	r2, [r0, #40]	@ 0x28
    1038:	e1520007 	cmp	r2, r7
    103c:	1afffff0 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	       a->addr[1] == b->addr[1] &&
    1040:	e590202c 	ldr	r2, [r0, #44]	@ 0x2c
    1044:	e1520008 	cmp	r2, r8
    1048:	1affffed 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	       a->addr[2] == b->addr[2] &&
    104c:	e5902030 	ldr	r2, [r0, #48]	@ 0x30
    1050:	e1590002 	cmp	r9, r2
    1054:	1affffea 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
    1058:	e5902034 	ldr	r2, [r0, #52]	@ 0x34
    105c:	e152000a 	cmp	r2, sl
    1060:	1affffe7 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	return a->addr[0] == b->addr[0] &&
    1064:	e51b1038 	ldr	r1, [fp, #-56]	@ 0xffffffc8
    1068:	e5902038 	ldr	r2, [r0, #56]	@ 0x38
    106c:	e1520001 	cmp	r2, r1
    1070:	1affffe3 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	       a->addr[1] == b->addr[1] &&
    1074:	e51b103c 	ldr	r1, [fp, #-60]	@ 0xffffffc4
    1078:	e590203c 	ldr	r2, [r0, #60]	@ 0x3c
    107c:	e1520001 	cmp	r2, r1
    1080:	1affffdf 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	       a->addr[2] == b->addr[2] &&
    1084:	e51b1034 	ldr	r1, [fp, #-52]	@ 0xffffffcc
    1088:	e5902040 	ldr	r2, [r0, #64]	@ 0x40
    108c:	e1510002 	cmp	r1, r2
    1090:	1affffdb 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
		 || cm->match_protocol != protocol
    1094:	e51b1040 	ldr	r1, [fp, #-64]	@ 0xffffffc0
    1098:	e5d02020 	ldrb	r2, [r0, #32]
    109c:	e1520001 	cmp	r2, r1
    10a0:	1affffd7 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
		 || cm->match_dev != dev));
    10a4:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
    10a8:	e590201c 	ldr	r2, [r0, #28]
    10ac:	e1510002 	cmp	r1, r2
    10b0:	1affffd3 	bne	1004 <sfe_ipv6_find_connection_match.constprop.0+0xb8>
	if (cm->next) {
    10b4:	e5903000 	ldr	r3, [r0]
    10b8:	e3530000 	cmp	r3, #0
    10bc:	0a000002 	beq	10cc <sfe_ipv6_find_connection_match.constprop.0+0x180>
		cm->next->prev = cm->prev;
    10c0:	e5902004 	ldr	r2, [r0, #4]
    10c4:	e5832004 	str	r2, [r3, #4]
	cm->prev->next = cm->next;
    10c8:	e5903000 	ldr	r3, [r0]
    10cc:	e5902004 	ldr	r2, [r0, #4]
    10d0:	e5823000 	str	r3, [r2]
	si->connection_match_hash_reorders++;
    10d4:	e59f20dc 	ldr	r2, [pc, #220]	@ 11b8 <sfe_ipv6_find_connection_match.constprop.0+0x26c>
	cm->prev = NULL;
    10d8:	e3a03000 	mov	r3, #0
	cm->next = head;
    10dc:	e5804000 	str	r4, [r0]
	cm->prev = NULL;
    10e0:	e5803004 	str	r3, [r0, #4]
	head->prev = cm;
    10e4:	e5840004 	str	r0, [r4, #4]
	si->connection_match_hash_reorders++;
    10e8:	e5923044 	ldr	r3, [r2, #68]	@ 0x44
	si->conn_match_hash[conn_match_idx] = cm;
    10ec:	e785010c 	str	r0, [r5, ip, lsl #2]
	si->connection_match_hash_reorders++;
    10f0:	e2833001 	add	r3, r3, #1
    10f4:	e5823044 	str	r3, [r2, #68]	@ 0x44
}
    10f8:	e24bd028 	sub	sp, fp, #40	@ 0x28
    10fc:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
	    && (cm->match_dest_port == dest_port)
    1100:	e1d424b6 	ldrh	r2, [r4, #70]	@ 0x46
    1104:	e152000e 	cmp	r2, lr
    1108:	1affffb8 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
    110c:	e5942024 	ldr	r2, [r4, #36]	@ 0x24
    1110:	e1520006 	cmp	r2, r6
    1114:	1affffb5 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	return a->addr[0] == b->addr[0] &&
    1118:	e5942028 	ldr	r2, [r4, #40]	@ 0x28
    111c:	e1520007 	cmp	r2, r7
    1120:	1affffb2 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	       a->addr[1] == b->addr[1] &&
    1124:	e594202c 	ldr	r2, [r4, #44]	@ 0x2c
    1128:	e1520008 	cmp	r2, r8
    112c:	1affffaf 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	       a->addr[2] == b->addr[2] &&
    1130:	e5942030 	ldr	r2, [r4, #48]	@ 0x30
    1134:	e1590002 	cmp	r9, r2
    1138:	1affffac 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
    113c:	e5942034 	ldr	r2, [r4, #52]	@ 0x34
    1140:	e152000a 	cmp	r2, sl
    1144:	1affffa9 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	return a->addr[0] == b->addr[0] &&
    1148:	e51b0038 	ldr	r0, [fp, #-56]	@ 0xffffffc8
    114c:	e5942038 	ldr	r2, [r4, #56]	@ 0x38
    1150:	e1520000 	cmp	r2, r0
    1154:	1affffa5 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	       a->addr[1] == b->addr[1] &&
    1158:	e51b003c 	ldr	r0, [fp, #-60]	@ 0xffffffc4
    115c:	e594203c 	ldr	r2, [r4, #60]	@ 0x3c
    1160:	e1520000 	cmp	r2, r0
    1164:	1affffa1 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	       a->addr[2] == b->addr[2] &&
    1168:	e51b0034 	ldr	r0, [fp, #-52]	@ 0xffffffcc
    116c:	e5942040 	ldr	r2, [r4, #64]	@ 0x40
    1170:	e1500002 	cmp	r0, r2
    1174:	1affff9d 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	    && (cm->match_protocol == protocol)
    1178:	e5d42020 	ldrb	r2, [r4, #32]
    117c:	e1520001 	cmp	r2, r1
    1180:	1affff9a 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
	    && (cm->match_dev == dev)) {
    1184:	e51b0030 	ldr	r0, [fp, #-48]	@ 0xffffffd0
    1188:	e594201c 	ldr	r2, [r4, #28]
    118c:	e1500002 	cmp	r0, r2
    1190:	1affff96 	bne	ff0 <sfe_ipv6_find_connection_match.constprop.0+0xa4>
		si->connection_match_hash_hits++;
    1194:	e59f201c 	ldr	r2, [pc, #28]	@ 11b8 <sfe_ipv6_find_connection_match.constprop.0+0x26c>
		return cm;
    1198:	e1a00004 	mov	r0, r4
		si->connection_match_hash_hits++;
    119c:	e5923040 	ldr	r3, [r2, #64]	@ 0x40
    11a0:	e2833001 	add	r3, r3, #1
    11a4:	e5823040 	str	r3, [r2, #64]	@ 0x40
		return cm;
    11a8:	eaffffd2 	b	10f8 <sfe_ipv6_find_connection_match.constprop.0+0x1ac>
		return NULL;
    11ac:	e1a00004 	mov	r0, r4
}
    11b0:	e24bd028 	sub	sp, fp, #40	@ 0x28
    11b4:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
    11b8:	00010050 	.word	0x00010050

000011bc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0>:
sfe_ipv4_find_sfe_ipv4_connection_match(struct sfe_ipv4 *si, struct net_device *dev, u8 protocol,
    11bc:	e1a0c00d 	mov	ip, sp
    11c0:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
    11c4:	e24cb004 	sub	fp, ip, #4
    11c8:	e1db40b8 	ldrh	r4, [fp, #8]
    11cc:	e1a07000 	mov	r7, r0
	cm = si->conn_match_hash[conn_match_idx];
    11d0:	e3006000 	movw	r6, #0
sfe_ipv4_find_sfe_ipv4_connection_match(struct sfe_ipv4 *si, struct net_device *dev, u8 protocol,
    11d4:	e59b5004 	ldr	r5, [fp, #4]
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    11d8:	e027e001 	eor	lr, r7, r1
	cm = si->conn_match_hash[conn_match_idx];
    11dc:	e3406000 	movt	r6, #0
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    11e0:	e0230004 	eor	r0, r3, r4
    11e4:	e6bf0fb0 	rev16	r0, r0
    11e8:	e022c005 	eor	ip, r2, r5
    11ec:	e6ff0070 	uxth	r0, r0
    11f0:	e6bfcf3c 	rev	ip, ip
    11f4:	e02ee00c 	eor	lr, lr, ip
    11f8:	e02ee000 	eor	lr, lr, r0
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    11fc:	e02ee62e 	eor	lr, lr, lr, lsr #12
    1200:	e7ebe05e 	ubfx	lr, lr, #0, #12
	cm = si->conn_match_hash[conn_match_idx];
    1204:	e28eea01 	add	lr, lr, #4096	@ 0x1000
    1208:	e28ee00c 	add	lr, lr, #12
    120c:	e796810e 	ldr	r8, [r6, lr, lsl #2]
	if (unlikely(!cm)) {
    1210:	e3580000 	cmp	r8, #0
    1214:	0a000043 	beq	1328 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x16c>
	if ((cm->match_src_port == src_port)
    1218:	e1d802bc 	ldrh	r0, [r8, #44]	@ 0x2c
    121c:	e1500003 	cmp	r0, r3
    1220:	0a00002b 	beq	12d4 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x118>
		cm = cm->next;
    1224:	e5980000 	ldr	r0, [r8]
	} while (cm && (cm->match_src_port != src_port
    1228:	e3500000 	cmp	r0, #0
    122c:	1a000003 	bne	1240 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x84>
    1230:	ea000025 	b	12cc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x110>
		cm = cm->next;
    1234:	e5900000 	ldr	r0, [r0]
	} while (cm && (cm->match_src_port != src_port
    1238:	e3500000 	cmp	r0, #0
    123c:	0a000022 	beq	12cc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x110>
    1240:	e1d0c2bc 	ldrh	ip, [r0, #44]	@ 0x2c
    1244:	e15c0003 	cmp	ip, r3
    1248:	1afffff9 	bne	1234 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x78>
		 || cm->match_dest_port != dest_port
    124c:	e1d0c2be 	ldrh	ip, [r0, #46]	@ 0x2e
    1250:	e15c0004 	cmp	ip, r4
    1254:	1afffff6 	bne	1234 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x78>
		 || cm->match_src_ip != src_ip
    1258:	e590c024 	ldr	ip, [r0, #36]	@ 0x24
    125c:	e152000c 	cmp	r2, ip
    1260:	1afffff3 	bne	1234 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x78>
		 || cm->match_dest_ip != dest_ip
    1264:	e590c028 	ldr	ip, [r0, #40]	@ 0x28
    1268:	e155000c 	cmp	r5, ip
    126c:	1afffff0 	bne	1234 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x78>
		 || cm->match_protocol != protocol
    1270:	e5d0c020 	ldrb	ip, [r0, #32]
    1274:	e15c0001 	cmp	ip, r1
    1278:	1affffed 	bne	1234 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x78>
		 || cm->match_dev != dev));
    127c:	e590c01c 	ldr	ip, [r0, #28]
    1280:	e157000c 	cmp	r7, ip
    1284:	1affffea 	bne	1234 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x78>
	if (cm->next) {
    1288:	e5903000 	ldr	r3, [r0]
    128c:	e3530000 	cmp	r3, #0
    1290:	0a000002 	beq	12a0 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0xe4>
		cm->next->prev = cm->prev;
    1294:	e5902004 	ldr	r2, [r0, #4]
    1298:	e5832004 	str	r2, [r3, #4]
	cm->prev->next = cm->next;
    129c:	e5903000 	ldr	r3, [r0]
    12a0:	e5902004 	ldr	r2, [r0, #4]
    12a4:	e5823000 	str	r3, [r2]
	si->connection_match_hash_reorders++;
    12a8:	e59f2084 	ldr	r2, [pc, #132]	@ 1334 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x178>
	cm->prev = NULL;
    12ac:	e3a03000 	mov	r3, #0
	cm->next = head;
    12b0:	e5808000 	str	r8, [r0]
	cm->prev = NULL;
    12b4:	e5803004 	str	r3, [r0, #4]
	head->prev = cm;
    12b8:	e5880004 	str	r0, [r8, #4]
	si->connection_match_hash_reorders++;
    12bc:	e5923044 	ldr	r3, [r2, #68]	@ 0x44
	si->conn_match_hash[conn_match_idx] = cm;
    12c0:	e786010e 	str	r0, [r6, lr, lsl #2]
	si->connection_match_hash_reorders++;
    12c4:	e2833001 	add	r3, r3, #1
    12c8:	e5823044 	str	r3, [r2, #68]	@ 0x44
}
    12cc:	e24bd020 	sub	sp, fp, #32
    12d0:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
	    && (cm->match_dest_port == dest_port)
    12d4:	e1d802be 	ldrh	r0, [r8, #46]	@ 0x2e
    12d8:	e1500004 	cmp	r0, r4
    12dc:	1affffd0 	bne	1224 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x68>
	    && (cm->match_src_ip == src_ip)
    12e0:	e5980024 	ldr	r0, [r8, #36]	@ 0x24
    12e4:	e1520000 	cmp	r2, r0
    12e8:	1affffcd 	bne	1224 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x68>
	    && (cm->match_dest_ip == dest_ip)
    12ec:	e5980028 	ldr	r0, [r8, #40]	@ 0x28
    12f0:	e1550000 	cmp	r5, r0
    12f4:	1affffca 	bne	1224 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x68>
	    && (cm->match_protocol == protocol)
    12f8:	e5d80020 	ldrb	r0, [r8, #32]
    12fc:	e1500001 	cmp	r0, r1
    1300:	1affffc7 	bne	1224 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x68>
	    && (cm->match_dev == dev)) {
    1304:	e598001c 	ldr	r0, [r8, #28]
    1308:	e1570000 	cmp	r7, r0
    130c:	1affffc4 	bne	1224 <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x68>
		si->connection_match_hash_hits++;
    1310:	e2866902 	add	r6, r6, #32768	@ 0x8000
		return cm;
    1314:	e1a00008 	mov	r0, r8
		si->connection_match_hash_hits++;
    1318:	e5963040 	ldr	r3, [r6, #64]	@ 0x40
    131c:	e2833001 	add	r3, r3, #1
    1320:	e5863040 	str	r3, [r6, #64]	@ 0x40
		return cm;
    1324:	eaffffe8 	b	12cc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0+0x110>
		return NULL;
    1328:	e1a00008 	mov	r0, r8
}
    132c:	e24bd020 	sub	sp, fp, #32
    1330:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
    1334:	000182c8 	.word	0x000182c8

00001338 <sfe_ipv6_debug_dev_read_connections_connection>:
{
    1338:	e1a0c00d 	mov	ip, sp
    133c:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    1340:	e24cb004 	sub	fp, ip, #4
    1344:	e24dd0fc 	sub	sp, sp, #252	@ 0xfc
    1348:	e1a05000 	mov	r5, r0
    134c:	e59b6008 	ldr	r6, [fp, #8]
    1350:	e1a08003 	mov	r8, r3
    1354:	e50b1074 	str	r1, [fp, #-116]	@ 0xffffff8c
    1358:	e50b2070 	str	r2, [fp, #-112]	@ 0xffffff90
	raw_spin_lock_bh(&lock->rlock);
    135c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    1360:	e595400c 	ldr	r4, [r5, #12]
    1364:	e3540000 	cmp	r4, #0
    1368:	0a0000b5 	beq	1644 <sfe_ipv6_debug_dev_read_connections_connection+0x30c>
		if (c->debug_read_seq < si->debug_read_seq) {
    136c:	e2853902 	add	r3, r5, #32768	@ 0x8000
    1370:	e5932270 	ldr	r2, [r3, #624]	@ 0x270
    1374:	ea000002 	b	1384 <sfe_ipv6_debug_dev_read_connections_connection+0x4c>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    1378:	e5944070 	ldr	r4, [r4, #112]	@ 0x70
    137c:	e3540000 	cmp	r4, #0
    1380:	0a0000af 	beq	1644 <sfe_ipv6_debug_dev_read_connections_connection+0x30c>
		if (c->debug_read_seq < si->debug_read_seq) {
    1384:	e594307c 	ldr	r3, [r4, #124]	@ 0x7c
    1388:	e1530002 	cmp	r3, r2
    138c:	2afffff9 	bcs	1378 <sfe_ipv6_debug_dev_read_connections_connection+0x40>
			c->debug_read_seq = si->debug_read_seq;
    1390:	e584207c 	str	r2, [r4, #124]	@ 0x7c
	protocol = c->protocol;
    1394:	e5941008 	ldr	r1, [r4, #8]
	original_cm = c->original_match;
    1398:	e5943054 	ldr	r3, [r4, #84]	@ 0x54
	src_dev = c->original_dev;
    139c:	e5940058 	ldr	r0, [r4, #88]	@ 0x58
	src_port_xlate = c->src_port_xlate;
    13a0:	e1d4e4be 	ldrh	lr, [r4, #78]	@ 0x4e
	src_port = c->src_port;
    13a4:	e1d4c4bc 	ldrh	ip, [r4, #76]	@ 0x4c
	src_priority = original_cm->priority;
    13a8:	e5936090 	ldr	r6, [r3, #144]	@ 0x90
	reply_cm = c->reply_match;
    13ac:	e594205c 	ldr	r2, [r4, #92]	@ 0x5c
	protocol = c->protocol;
    13b0:	e14b07fc 	strd	r0, [fp, #-124]	@ 0xffffff84
	cm->rx_packet_count64 += cm->rx_packet_count;
    13b4:	e5930060 	ldr	r0, [r3, #96]	@ 0x60
    13b8:	e59310b0 	ldr	r1, [r3, #176]	@ 0xb0
	src_priority = original_cm->priority;
    13bc:	e50b6088 	str	r6, [fp, #-136]	@ 0xffffff78
	src_dscp = original_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
    13c0:	e5936094 	ldr	r6, [r3, #148]	@ 0x94
	src_port_xlate = c->src_port_xlate;
    13c4:	e50be084 	str	lr, [fp, #-132]	@ 0xffffff7c
	src_port = c->src_port;
    13c8:	e50bc080 	str	ip, [fp, #-128]	@ 0xffffff80
	cm->rx_packet_count64 += cm->rx_packet_count;
    13cc:	e091e000 	adds	lr, r1, r0
	cm->rx_byte_count64 += cm->rx_byte_count;
    13d0:	e5930064 	ldr	r0, [r3, #100]	@ 0x64
	src_dscp = original_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
    13d4:	e1a07126 	lsr	r7, r6, #2
	cm->rx_packet_count64 += cm->rx_packet_count;
    13d8:	e59310b4 	ldr	r1, [r3, #180]	@ 0xb4
	src_dscp = original_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
    13dc:	e50b708c 	str	r7, [fp, #-140]	@ 0xffffff74
	src_ip = c->src_ip[0];
    13e0:	e1c460dc 	ldrd	r6, [r4, #12]
	cm->rx_packet_count64 += cm->rx_packet_count;
    13e4:	e2a1c000 	adc	ip, r1, #0
	cm->rx_byte_count64 += cm->rx_byte_count;
    13e8:	e59310b8 	ldr	r1, [r3, #184]	@ 0xb8
	src_ip = c->src_ip[0];
    13ec:	e14b66fc 	strd	r6, [fp, #-108]	@ 0xffffff94
    13f0:	e1c461d4 	ldrd	r6, [r4, #20]
	cm->rx_byte_count64 += cm->rx_byte_count;
    13f4:	e0911000 	adds	r1, r1, r0
    13f8:	e59300bc 	ldr	r0, [r3, #188]	@ 0xbc
	src_ip = c->src_ip[0];
    13fc:	e14b66f4 	strd	r6, [fp, #-100]	@ 0xffffff9c
	src_ip_xlate = c->src_ip_xlate[0];
    1400:	e1c461dc 	ldrd	r6, [r4, #28]
	cm->rx_byte_count64 += cm->rx_byte_count;
    1404:	e2a00000 	adc	r0, r0, #0
	src_ip_xlate = c->src_ip_xlate[0];
    1408:	e14b65fc 	strd	r6, [fp, #-92]	@ 0xffffffa4
    140c:	e1c462d4 	ldrd	r6, [r4, #36]	@ 0x24
	cm->rx_packet_count64 += cm->rx_packet_count;
    1410:	e583e0b0 	str	lr, [r3, #176]	@ 0xb0
    1414:	e583c0b4 	str	ip, [r3, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    1418:	e58310b8 	str	r1, [r3, #184]	@ 0xb8
	cm->rx_packet_count = 0;
    141c:	e3a01000 	mov	r1, #0
	src_ip_xlate = c->src_ip_xlate[0];
    1420:	e14b65f4 	strd	r6, [fp, #-84]	@ 0xffffffac
	cm->rx_byte_count64 += cm->rx_byte_count;
    1424:	e58300bc 	str	r0, [r3, #188]	@ 0xbc
	cm->rx_packet_count = 0;
    1428:	e3a00000 	mov	r0, #0
    142c:	e1c306f0 	strd	r0, [r3, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    1430:	e5921060 	ldr	r1, [r2, #96]	@ 0x60
	cm->rx_byte_count64 += cm->rx_byte_count;
    1434:	e5926064 	ldr	r6, [r2, #100]	@ 0x64
	cm->rx_packet_count64 += cm->rx_packet_count;
    1438:	e59200b0 	ldr	r0, [r2, #176]	@ 0xb0
	cm->rx_byte_count64 += cm->rx_byte_count;
    143c:	e59270bc 	ldr	r7, [r2, #188]	@ 0xbc
	cm->rx_packet_count64 += cm->rx_packet_count;
    1440:	e091a000 	adds	sl, r1, r0
    1444:	e59210b4 	ldr	r1, [r2, #180]	@ 0xb4
	cm->rx_packet_count = 0;
    1448:	e3a00000 	mov	r0, #0
	cm->rx_packet_count64 += cm->rx_packet_count;
    144c:	e582a0b0 	str	sl, [r2, #176]	@ 0xb0
    1450:	e2a19000 	adc	r9, r1, #0
	cm->rx_packet_count = 0;
    1454:	e3a01000 	mov	r1, #0
    1458:	e1c206f0 	strd	r0, [r2, #96]	@ 0x60
	cm->rx_byte_count64 += cm->rx_byte_count;
    145c:	e59210b8 	ldr	r1, [r2, #184]	@ 0xb8
	cm->rx_packet_count64 += cm->rx_packet_count;
    1460:	e58290b4 	str	r9, [r2, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    1464:	e0966001 	adds	r6, r6, r1
	dest_dscp = reply_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
    1468:	e5921094 	ldr	r1, [r2, #148]	@ 0x94
	cm->rx_byte_count64 += cm->rx_byte_count;
    146c:	e2a77000 	adc	r7, r7, #0
    1470:	e1c26bf8 	strd	r6, [r2, #184]	@ 0xb8
	dest_priority = reply_cm->priority;
    1474:	e5922090 	ldr	r2, [r2, #144]	@ 0x90
	dest_dscp = reply_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
    1478:	e1a01121 	lsr	r1, r1, #2
	dest_port_xlate = c->dest_port_xlate;
    147c:	e1d4e5b2 	ldrh	lr, [r4, #82]	@ 0x52
	dest_port = c->dest_port;
    1480:	e1d4c5b0 	ldrh	ip, [r4, #80]	@ 0x50
	dest_priority = reply_cm->priority;
    1484:	e50b20b4 	str	r2, [fp, #-180]	@ 0xffffff4c
	dest_dscp = reply_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
    1488:	e50b10ac 	str	r1, [fp, #-172]	@ 0xffffff54
	dest_dev = c->reply_dev;
    148c:	e5941060 	ldr	r1, [r4, #96]	@ 0x60
	dest_port_xlate = c->dest_port_xlate;
    1490:	e50be0a8 	str	lr, [fp, #-168]	@ 0xffffff58
	dest_port = c->dest_port;
    1494:	e50bc0a4 	str	ip, [fp, #-164]	@ 0xffffff5c
	dest_dev = c->reply_dev;
    1498:	e50b10a0 	str	r1, [fp, #-160]	@ 0xffffff60
	dest_ip = c->dest_ip[0];
    149c:	e1c402dc 	ldrd	r0, [r4, #44]	@ 0x2c
    14a0:	e14b04fc 	strd	r0, [fp, #-76]	@ 0xffffffb4
    14a4:	e1c403d4 	ldrd	r0, [r4, #52]	@ 0x34
    14a8:	e14b04f4 	strd	r0, [fp, #-68]	@ 0xffffffbc
	dest_ip_xlate = c->dest_ip_xlate[0];
    14ac:	e1c403dc 	ldrd	r0, [r4, #60]	@ 0x3c
    14b0:	e14b03fc 	strd	r0, [fp, #-60]	@ 0xffffffc4
    14b4:	e1c404d4 	ldrd	r0, [r4, #68]	@ 0x44
    14b8:	e14b03f4 	strd	r0, [fp, #-52]	@ 0xffffffcc
	src_rx_packets = original_cm->rx_packet_count64;
    14bc:	e1c30bd0 	ldrd	r0, [r3, #176]	@ 0xb0
    14c0:	e14b09f4 	strd	r0, [fp, #-148]	@ 0xffffff6c
	src_rx_bytes = original_cm->rx_byte_count64;
    14c4:	e1c32bd8 	ldrd	r2, [r3, #184]	@ 0xb8
    14c8:	e14b29fc 	strd	r2, [fp, #-156]	@ 0xffffff64
	last_sync_jiffies = get_jiffies_64() - c->last_sync_jiffies;
    14cc:	ebfffffe 	bl	0 <get_jiffies_64>
    14d0:	e594c068 	ldr	ip, [r4, #104]	@ 0x68
    14d4:	e1a03000 	mov	r3, r0
	raw_spin_unlock_bh(&lock->rlock);
    14d8:	e1a00005 	mov	r0, r5
    14dc:	e053500c 	subs	r5, r3, ip
    14e0:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
	mark = c->mark;
    14e4:	e5944078 	ldr	r4, [r4, #120]	@ 0x78
	last_sync_jiffies = get_jiffies_64() - c->last_sync_jiffies;
    14e8:	e0c11003 	sbc	r1, r1, r3
    14ec:	e50b10b0 	str	r1, [fp, #-176]	@ 0xffffff50
    14f0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    14f4:	e14b09d4 	ldrd	r0, [fp, #-148]	@ 0xffffff6c
    14f8:	e1cd02f0 	strd	r0, [sp, #32]
    14fc:	e14b29dc 	ldrd	r2, [fp, #-156]	@ 0xffffff64
    1500:	e1cd22f8 	strd	r2, [sp, #40]	@ 0x28
    1504:	e24b306c 	sub	r3, fp, #108	@ 0x6c
    1508:	e51b007c 	ldr	r0, [fp, #-124]	@ 0xffffff84
    150c:	e88d0009 	stm	sp, {r0, r3}
    1510:	e24b305c 	sub	r3, fp, #92	@ 0x5c
    1514:	e58d3008 	str	r3, [sp, #8]
    1518:	e51b2088 	ldr	r2, [fp, #-136]	@ 0xffffff78
    151c:	e58d2014 	str	r2, [sp, #20]
    1520:	e51b208c 	ldr	r2, [fp, #-140]	@ 0xffffff74
    1524:	e58d2018 	str	r2, [sp, #24]
    1528:	e51b00a0 	ldr	r0, [fp, #-160]	@ 0xffffff60
    152c:	e58d0030 	str	r0, [sp, #48]	@ 0x30
    1530:	e24b004c 	sub	r0, fp, #76	@ 0x4c
    1534:	e58d0034 	str	r0, [sp, #52]	@ 0x34
    1538:	e24b003c 	sub	r0, fp, #60	@ 0x3c
    153c:	e58d0038 	str	r0, [sp, #56]	@ 0x38
    1540:	e51b20b4 	ldr	r2, [fp, #-180]	@ 0xffffff4c
    1544:	e58d2044 	str	r2, [sp, #68]	@ 0x44
    1548:	e51b20ac 	ldr	r2, [fp, #-172]	@ 0xffffff54
    154c:	e58d5060 	str	r5, [sp, #96]	@ 0x60
    1550:	e58d2048 	str	r2, [sp, #72]	@ 0x48
    1554:	e58da050 	str	sl, [sp, #80]	@ 0x50
    1558:	e58d9054 	str	r9, [sp, #84]	@ 0x54
    155c:	e1cd65f8 	strd	r6, [sp, #88]	@ 0x58
    1560:	e51b10b0 	ldr	r1, [fp, #-176]	@ 0xffffff50
    1564:	e58d1064 	str	r1, [sp, #100]	@ 0x64
    1568:	e3a01c03 	mov	r1, #768	@ 0x300
    156c:	e58d4068 	str	r4, [sp, #104]	@ 0x68
				ntohs(src_port), ntohs(src_port_xlate),
    1570:	e51be084 	ldr	lr, [fp, #-132]	@ 0xffffff7c
    1574:	e51bc080 	ldr	ip, [fp, #-128]	@ 0xffffff80
    1578:	e6bf3fbe 	rev16	r3, lr
    157c:	e6bf2fbc 	rev16	r2, ip
    1580:	e6ff3073 	uxth	r3, r3
    1584:	e6ff2072 	uxth	r2, r2
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    1588:	e1cd20fc 	strd	r2, [sp, #12]
				ntohs(dest_port), ntohs(dest_port_xlate),
    158c:	e51bc0a4 	ldr	ip, [fp, #-164]	@ 0xffffff5c
    1590:	e51be0a8 	ldr	lr, [fp, #-168]	@ 0xffffff58
    1594:	e6bf2fbc 	rev16	r2, ip
    1598:	e6bf3fbe 	rev16	r3, lr
    159c:	e6ff2072 	uxth	r2, r2
    15a0:	e6ff3073 	uxth	r3, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    15a4:	e1cd23fc 	strd	r2, [sp, #60]	@ 0x3c
    15a8:	e3002000 	movw	r2, #0
    15ac:	e51b3078 	ldr	r3, [fp, #-120]	@ 0xffffff88
    15b0:	e51b0070 	ldr	r0, [fp, #-112]	@ 0xffffff90
    15b4:	e3402000 	movt	r2, #0
    15b8:	ebfffffe 	bl	0 <snprintf>
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    15bc:	e51b2074 	ldr	r2, [fp, #-116]	@ 0xffffff8c
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    15c0:	e1a04000 	mov	r4, r0
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    15c4:	e59b3004 	ldr	r3, [fp, #4]
    15c8:	e5933000 	ldr	r3, [r3]
    15cc:	e0820003 	add	r0, r2, r3

	if (IS_ENABLED(CONFIG_ALTERNATE_USER_ADDRESS_SPACE) ||
	    !IS_ENABLED(CONFIG_MMU))
		return true;

	return (size <= limit) && (addr <= (limit - size));
    15d0:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    15d4:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    15d8:	e1500003 	cmp	r0, r3
    15dc:	9a000002 	bls	15ec <sfe_ipv6_debug_dev_read_connections_connection+0x2b4>
		return false;
    15e0:	e3a00000 	mov	r0, #0
}
    15e4:	e24bd028 	sub	sp, fp, #40	@ 0x28
    15e8:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
	 * load the TLS register only once in every function.
	 *
	 * Clang < 13.0.1 gets this wrong for Thumb2 builds:
	 * https://github.com/ClangBuiltLinux/linux/issues/1485
	 */
	cur = __builtin_thread_pointer();
    15ec:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
#ifdef CONFIG_CPU_CP15_MMU
static __always_inline unsigned int get_domain(void)
{
	unsigned int domain;

	asm(
    15f0:	ee135f10 	mrc	15, 0, r5, cr3, cr0, {0}
{
#ifdef CONFIG_CPU_SW_DOMAIN_PAN
	unsigned int old_domain = get_domain();

	/* Set the current domain access to permit user accesses */
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    15f4:	e3c5300c 	bic	r3, r5, #12
    15f8:	e3833004 	orr	r3, r3, #4
	return domain;
}

static __always_inline void set_domain(unsigned int val)
{
	asm volatile(
    15fc:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	"mcr	p15, 0, %0, c3, c0	@ set domain"
	  : : "r" (val) : "memory");
	isb();
    1600:	f57ff06f 	isb	sy
raw_copy_to_user(void __user *to, const void *from, unsigned long n)
{
#ifndef CONFIG_UACCESS_WITH_MEMCPY
	unsigned int __ua_flags;
	__ua_flags = uaccess_save_and_enable();
	n = arm_copy_to_user(to, from, n);
    1604:	e51b1070 	ldr	r1, [fp, #-112]	@ 0xffffff90
    1608:	e3a02c03 	mov	r2, #768	@ 0x300
    160c:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    1610:	ee035f10 	mcr	15, 0, r5, cr3, cr0, {0}
	isb();
    1614:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    1618:	e3500000 	cmp	r0, #0
    161c:	1affffef 	bne	15e0 <sfe_ipv6_debug_dev_read_connections_connection+0x2a8>
	*length -= bytes_read;
    1620:	e5983000 	ldr	r3, [r8]
	*total_read += bytes_read;
    1624:	e59b2004 	ldr	r2, [fp, #4]
	*length -= bytes_read;
    1628:	e0433004 	sub	r3, r3, r4
    162c:	e5883000 	str	r3, [r8]
	*total_read += bytes_read;
    1630:	e59b3004 	ldr	r3, [fp, #4]
    1634:	e5933000 	ldr	r3, [r3]
    1638:	e0833004 	add	r3, r3, r4
    163c:	e5823000 	str	r3, [r2]
	return true;
    1640:	ea000004 	b	1658 <sfe_ipv6_debug_dev_read_connections_connection+0x320>
    1644:	e1a00005 	mov	r0, r5
    1648:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		ws->state++;
    164c:	e5963000 	ldr	r3, [r6]
    1650:	e2833001 	add	r3, r3, #1
    1654:	e5863000 	str	r3, [r6]
		return true;
    1658:	e3a00001 	mov	r0, #1
}
    165c:	e24bd028 	sub	sp, fp, #40	@ 0x28
    1660:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

00001664 <sfe_ipv6_update_tcp_state.isra.0>:
sfe_ipv6_update_tcp_state(struct sfe_ipv6_connection *c,
    1664:	e1a0c00d 	mov	ip, sp
    1668:	e92dd800 	push	{fp, ip, lr, pc}
    166c:	e24cb004 	sub	fp, ip, #4
	if (orig_tcp->max_win < sic->src_td_max_window) {
    1670:	e590c054 	ldr	ip, [r0, #84]	@ 0x54
    1674:	e592307c 	ldr	r3, [r2, #124]	@ 0x7c
    1678:	e15c0003 	cmp	ip, r3
		orig_tcp->max_win = sic->src_td_max_window;
    167c:	35803054 	strcc	r3, [r0, #84]	@ 0x54
	if ((s32)(orig_tcp->end - sic->src_td_end) < 0) {
    1680:	e5903058 	ldr	r3, [r0, #88]	@ 0x58
    1684:	e592c080 	ldr	ip, [r2, #128]	@ 0x80
    1688:	e043300c 	sub	r3, r3, ip
    168c:	e3530000 	cmp	r3, #0
	if ((s32)(orig_tcp->max_end - sic->src_td_max_end) < 0) {
    1690:	e590305c 	ldr	r3, [r0, #92]	@ 0x5c
		orig_tcp->end = sic->src_td_end;
    1694:	b580c058 	strlt	ip, [r0, #88]	@ 0x58
	if ((s32)(orig_tcp->max_end - sic->src_td_max_end) < 0) {
    1698:	e592c084 	ldr	ip, [r2, #132]	@ 0x84
    169c:	e043300c 	sub	r3, r3, ip
    16a0:	e3530000 	cmp	r3, #0
		orig_tcp->max_end = sic->src_td_max_end;
    16a4:	b580c05c 	strlt	ip, [r0, #92]	@ 0x5c
	if (repl_tcp->max_win < sic->dest_td_max_window) {
    16a8:	e591c054 	ldr	ip, [r1, #84]	@ 0x54
    16ac:	e592308c 	ldr	r3, [r2, #140]	@ 0x8c
    16b0:	e15c0003 	cmp	ip, r3
		repl_tcp->max_win = sic->dest_td_max_window;
    16b4:	35813054 	strcc	r3, [r1, #84]	@ 0x54
	if ((s32)(repl_tcp->end - sic->dest_td_end) < 0) {
    16b8:	e5913058 	ldr	r3, [r1, #88]	@ 0x58
    16bc:	e592c090 	ldr	ip, [r2, #144]	@ 0x90
    16c0:	e043300c 	sub	r3, r3, ip
    16c4:	e3530000 	cmp	r3, #0
	if ((s32)(repl_tcp->max_end - sic->dest_td_max_end) < 0) {
    16c8:	e591305c 	ldr	r3, [r1, #92]	@ 0x5c
		repl_tcp->end = sic->dest_td_end;
    16cc:	b581c058 	strlt	ip, [r1, #88]	@ 0x58
	if ((s32)(repl_tcp->max_end - sic->dest_td_max_end) < 0) {
    16d0:	e592c094 	ldr	ip, [r2, #148]	@ 0x94
    16d4:	e043300c 	sub	r3, r3, ip
    16d8:	e3530000 	cmp	r3, #0
		repl_tcp->max_end = sic->dest_td_max_end;
    16dc:	b581c05c 	strlt	ip, [r1, #92]	@ 0x5c
	orig_cm->flags &= ~SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    16e0:	e5903048 	ldr	r3, [r0, #72]	@ 0x48
    16e4:	e3c33004 	bic	r3, r3, #4
    16e8:	e5803048 	str	r3, [r0, #72]	@ 0x48
	repl_cm->flags &= ~SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    16ec:	e5913048 	ldr	r3, [r1, #72]	@ 0x48
    16f0:	e3c33004 	bic	r3, r3, #4
    16f4:	e5813048 	str	r3, [r1, #72]	@ 0x48
	if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    16f8:	e592300c 	ldr	r3, [r2, #12]
    16fc:	e3130001 	tst	r3, #1
    1700:	0a000005 	beq	171c <sfe_ipv6_update_tcp_state.isra.0+0xb8>
		orig_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    1704:	e5903048 	ldr	r3, [r0, #72]	@ 0x48
    1708:	e3833004 	orr	r3, r3, #4
    170c:	e5803048 	str	r3, [r0, #72]	@ 0x48
		repl_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    1710:	e5913048 	ldr	r3, [r1, #72]	@ 0x48
    1714:	e3833004 	orr	r3, r3, #4
    1718:	e5813048 	str	r3, [r1, #72]	@ 0x48
}
    171c:	e24bd00c 	sub	sp, fp, #12
    1720:	e89da800 	ldm	sp, {fp, sp, pc}

00001724 <sfe_ipv6_flush_connection.constprop.1>:
static void sfe_ipv6_flush_connection(struct sfe_ipv6 *si,
    1724:	e1a0c00d 	mov	ip, sp
    1728:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    172c:	e24cb004 	sub	fp, ip, #4
    1730:	e24dd0cc 	sub	sp, sp, #204	@ 0xcc
    1734:	e1a04000 	mov	r4, r0
	raw_spin_lock_bh(&lock->rlock);
    1738:	e3005000 	movw	r5, #0
    173c:	e3405000 	movt	r5, #0
    1740:	e1a00005 	mov	r0, r5
    1744:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_flushes++;
    1748:	e2852902 	add	r2, r5, #32768	@ 0x8000
	raw_spin_unlock_bh(&lock->rlock);
    174c:	e1a00005 	mov	r0, r5
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    1750:	e595502c 	ldr	r5, [r5, #44]	@ 0x2c
	si->connection_flushes++;
    1754:	e5923048 	ldr	r3, [r2, #72]	@ 0x48
    1758:	e2833001 	add	r3, r3, #1
    175c:	e5823048 	str	r3, [r2, #72]	@ 0x48
    1760:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (sync_rule_callback) {
    1764:	e3550000 	cmp	r5, #0
    1768:	0a00006e 	beq	1928 <sfe_ipv6_flush_connection.constprop.1+0x204>
		now_jiffies = get_jiffies_64();
    176c:	ebfffffe 	bl	0 <get_jiffies_64>
	sis->protocol = c->protocol;
    1770:	e5943008 	ldr	r3, [r4, #8]
		now_jiffies = get_jiffies_64();
    1774:	e1a08001 	mov	r8, r1
    1778:	e1a0e000 	mov	lr, r0
	sis->dest_ip_xlate.ip6[0] = c->dest_ip_xlate[0];
    177c:	e1c464d4 	ldrd	r6, [r4, #68]	@ 0x44
	sis->is_v6 = 1;
    1780:	e3a0c001 	mov	ip, #1
	sis->src_port = c->src_port;
    1784:	e594104c 	ldr	r1, [r4, #76]	@ 0x4c
	reply_cm = c->reply_match;
    1788:	e594205c 	ldr	r2, [r4, #92]	@ 0x5c
	sis->is_v6 = 1;
    178c:	e50bc0dc 	str	ip, [fp, #-220]	@ 0xffffff24
	sis->protocol = c->protocol;
    1790:	e50b30d8 	str	r3, [fp, #-216]	@ 0xffffff28
	sis->dest_port = c->dest_port;
    1794:	e5943050 	ldr	r3, [r4, #80]	@ 0x50
	sis->dest_ip_xlate.ip6[0] = c->dest_ip_xlate[0];
    1798:	e14b69f8 	strd	r6, [fp, #-152]	@ 0xffffff68
	sis->src_port = c->src_port;
    179c:	e50b10b4 	str	r1, [fp, #-180]	@ 0xffffff4c
	sis->src_ip.ip6[0] = c->src_ip[0];
    17a0:	e1c400dc 	ldrd	r0, [r4, #12]
	sis->dest_port = c->dest_port;
    17a4:	e50b3090 	str	r3, [fp, #-144]	@ 0xffffff70
	original_cm = c->original_match;
    17a8:	e5943054 	ldr	r3, [r4, #84]	@ 0x54
	sis->src_ip.ip6[0] = c->src_ip[0];
    17ac:	e14b0df4 	strd	r0, [fp, #-212]	@ 0xffffff2c
	cm->rx_packet_count64 += cm->rx_packet_count;
    17b0:	e3a01000 	mov	r1, #0
		sync_rule_callback(&sis);
    17b4:	e24b00e4 	sub	r0, fp, #228	@ 0xe4
	sis->src_td_max_window = original_cm->protocol_state.tcp.max_win;
    17b8:	e5936054 	ldr	r6, [r3, #84]	@ 0x54
    17bc:	e50b608c 	str	r6, [fp, #-140]	@ 0xffffff74
	sis->src_td_end = original_cm->protocol_state.tcp.end;
    17c0:	e5936058 	ldr	r6, [r3, #88]	@ 0x58
    17c4:	e50b6088 	str	r6, [fp, #-136]	@ 0xffffff78
	sis->src_td_max_end = original_cm->protocol_state.tcp.max_end;
    17c8:	e593605c 	ldr	r6, [r3, #92]	@ 0x5c
    17cc:	e50b6084 	str	r6, [fp, #-132]	@ 0xffffff7c
	sis->src_ip.ip6[0] = c->src_ip[0];
    17d0:	e1c461d4 	ldrd	r6, [r4, #20]
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
    17d4:	e5929054 	ldr	r9, [r2, #84]	@ 0x54
	sis->src_ip.ip6[0] = c->src_ip[0];
    17d8:	e14b6cfc 	strd	r6, [fp, #-204]	@ 0xffffff34
	sis->src_ip_xlate.ip6[0] = c->src_ip_xlate[0];
    17dc:	e1c461dc 	ldrd	r6, [r4, #28]
    17e0:	e14b6cf4 	strd	r6, [fp, #-196]	@ 0xffffff3c
    17e4:	e1c462d4 	ldrd	r6, [r4, #36]	@ 0x24
    17e8:	e14b6bfc 	strd	r6, [fp, #-188]	@ 0xffffff44
	sis->dest_ip.ip6[0] = c->dest_ip[0];
    17ec:	e1c462dc 	ldrd	r6, [r4, #44]	@ 0x2c
    17f0:	e14b6bf0 	strd	r6, [fp, #-176]	@ 0xffffff50
    17f4:	e1c463d4 	ldrd	r6, [r4, #52]	@ 0x34
    17f8:	e14b6af8 	strd	r6, [fp, #-168]	@ 0xffffff58
	sis->dest_ip_xlate.ip6[0] = c->dest_ip_xlate[0];
    17fc:	e1c463dc 	ldrd	r6, [r4, #60]	@ 0x3c
    1800:	e14b6af0 	strd	r6, [fp, #-160]	@ 0xffffff60
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
    1804:	e50b9064 	str	r9, [fp, #-100]	@ 0xffffff9c
	sis->dest_td_end = reply_cm->protocol_state.tcp.end;
    1808:	e5926058 	ldr	r6, [r2, #88]	@ 0x58
    180c:	e50b6060 	str	r6, [fp, #-96]	@ 0xffffffa0
	sis->dest_td_max_end = reply_cm->protocol_state.tcp.max_end;
    1810:	e592605c 	ldr	r6, [r2, #92]	@ 0x5c
    1814:	e50b605c 	str	r6, [fp, #-92]	@ 0xffffffa4
	sis->src_new_packet_count = original_cm->rx_packet_count;
    1818:	e5936060 	ldr	r6, [r3, #96]	@ 0x60
    181c:	e50b606c 	str	r6, [fp, #-108]	@ 0xffffff94
	sis->src_new_byte_count = original_cm->rx_byte_count;
    1820:	e5936064 	ldr	r6, [r3, #100]	@ 0x64
    1824:	e50b6068 	str	r6, [fp, #-104]	@ 0xffffff98
	sis->dest_new_packet_count = reply_cm->rx_packet_count;
    1828:	e5926060 	ldr	r6, [r2, #96]	@ 0x60
    182c:	e50b6044 	str	r6, [fp, #-68]	@ 0xffffffbc
	sis->dest_new_byte_count = reply_cm->rx_byte_count;
    1830:	e5926064 	ldr	r6, [r2, #100]	@ 0x64
    1834:	e50b6040 	str	r6, [fp, #-64]	@ 0xffffffc0
	cm->rx_packet_count64 += cm->rx_packet_count;
    1838:	e5937060 	ldr	r7, [r3, #96]	@ 0x60
    183c:	e59360b0 	ldr	r6, [r3, #176]	@ 0xb0
	cm->rx_packet_count = 0;
    1840:	e5831060 	str	r1, [r3, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    1844:	e0966007 	adds	r6, r6, r7
	cm->rx_byte_count64 += cm->rx_byte_count;
    1848:	e5937064 	ldr	r7, [r3, #100]	@ 0x64
	cm->rx_byte_count = 0;
    184c:	e5831064 	str	r1, [r3, #100]	@ 0x64
	cm->rx_packet_count64 += cm->rx_packet_count;
    1850:	e58360b0 	str	r6, [r3, #176]	@ 0xb0
    1854:	e59360b4 	ldr	r6, [r3, #180]	@ 0xb4
    1858:	e2a66000 	adc	r6, r6, #0
    185c:	e58360b4 	str	r6, [r3, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    1860:	e59360b8 	ldr	r6, [r3, #184]	@ 0xb8
    1864:	e0966007 	adds	r6, r6, r7
    1868:	e58360b8 	str	r6, [r3, #184]	@ 0xb8
    186c:	e59360bc 	ldr	r6, [r3, #188]	@ 0xbc
    1870:	e2a66000 	adc	r6, r6, #0
    1874:	e58360bc 	str	r6, [r3, #188]	@ 0xbc
	cm->rx_packet_count64 += cm->rx_packet_count;
    1878:	e5927060 	ldr	r7, [r2, #96]	@ 0x60
    187c:	e59260b0 	ldr	r6, [r2, #176]	@ 0xb0
	cm->rx_packet_count = 0;
    1880:	e5821060 	str	r1, [r2, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    1884:	e59290b4 	ldr	r9, [r2, #180]	@ 0xb4
    1888:	e0976006 	adds	r6, r7, r6
    188c:	e2a99000 	adc	r9, r9, #0
    1890:	e50b60e8 	str	r6, [fp, #-232]	@ 0xffffff18
    1894:	e58260b0 	str	r6, [r2, #176]	@ 0xb0
	cm->rx_byte_count64 += cm->rx_byte_count;
    1898:	e5926064 	ldr	r6, [r2, #100]	@ 0x64
	cm->rx_packet_count64 += cm->rx_packet_count;
    189c:	e58290b4 	str	r9, [r2, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    18a0:	e592a0b8 	ldr	sl, [r2, #184]	@ 0xb8
	cm->rx_byte_count = 0;
    18a4:	e5821064 	str	r1, [r2, #100]	@ 0x64
	cm->rx_byte_count64 += cm->rx_byte_count;
    18a8:	e096600a 	adds	r6, r6, sl
    18ac:	e592a0bc 	ldr	sl, [r2, #188]	@ 0xbc
    18b0:	e58260b8 	str	r6, [r2, #184]	@ 0xb8
    18b4:	e50b60ec 	str	r6, [fp, #-236]	@ 0xffffff14
    18b8:	e2aa1000 	adc	r1, sl, #0
    18bc:	e58210bc 	str	r1, [r2, #188]	@ 0xbc
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    18c0:	e594a068 	ldr	sl, [r4, #104]	@ 0x68
	sis->src_dev = original_cm->match_dev;
    18c4:	e593701c 	ldr	r7, [r3, #28]
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    18c8:	e05e600a 	subs	r6, lr, sl
    18cc:	e594a06c 	ldr	sl, [r4, #108]	@ 0x6c
    18d0:	e50b60f0 	str	r6, [fp, #-240]	@ 0xffffff10
	sis->src_dev = original_cm->match_dev;
    18d4:	e50b70e4 	str	r7, [fp, #-228]	@ 0xffffff1c
	sis->src_packet_count = original_cm->rx_packet_count64;
    18d8:	e1c36bd0 	ldrd	r6, [r3, #176]	@ 0xb0
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    18dc:	e0c8a00a 	sbc	sl, r8, sl
	sis->src_packet_count = original_cm->rx_packet_count64;
    18e0:	e14b67fc 	strd	r6, [fp, #-124]	@ 0xffffff84
	sis->src_byte_count = original_cm->rx_byte_count64;
    18e4:	e1c36bd8 	ldrd	r6, [r3, #184]	@ 0xb8
    18e8:	e14b67f4 	strd	r6, [fp, #-116]	@ 0xffffff8c
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    18ec:	e51b60e8 	ldr	r6, [fp, #-232]	@ 0xffffff18
	sis->dest_dev = reply_cm->match_dev;
    18f0:	e592301c 	ldr	r3, [r2, #28]
	c->last_sync_jiffies = now_jiffies;
    18f4:	e584e068 	str	lr, [r4, #104]	@ 0x68
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    18f8:	e51b20ec 	ldr	r2, [fp, #-236]	@ 0xffffff14
	c->last_sync_jiffies = now_jiffies;
    18fc:	e584806c 	str	r8, [r4, #108]	@ 0x6c
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    1900:	e50b9050 	str	r9, [fp, #-80]	@ 0xffffffb0
    1904:	e50b6054 	str	r6, [fp, #-84]	@ 0xffffffac
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    1908:	e51b60f0 	ldr	r6, [fp, #-240]	@ 0xffffff10
	sis->dest_dev = reply_cm->match_dev;
    190c:	e50b30e0 	str	r3, [fp, #-224]	@ 0xffffff20
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    1910:	e50b204c 	str	r2, [fp, #-76]	@ 0xffffffb4
    1914:	e50b1048 	str	r1, [fp, #-72]	@ 0xffffffb8
	sis->reason = reason;
    1918:	e50bc03c 	str	ip, [fp, #-60]	@ 0xffffffc4
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    191c:	e50b6034 	str	r6, [fp, #-52]	@ 0xffffffcc
    1920:	e50ba030 	str	sl, [fp, #-48]	@ 0xffffffd0
		sync_rule_callback(&sis);
    1924:	e12fff35 	blx	r5
	dev_put(c->original_dev);
    1928:	e5943058 	ldr	r3, [r4, #88]	@ 0x58
}

static inline void netdev_put(struct net_device *dev,
			      netdevice_tracker *tracker)
{
	if (dev) {
    192c:	e3530000 	cmp	r3, #0
    1930:	0a000007 	beq	1954 <sfe_ipv6_flush_connection.constprop.1+0x230>
	asm volatile(
    1934:	e10f0000 	mrs	r0, CPSR
    1938:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    193c:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    1940:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    1944:	e7932001 	ldr	r2, [r3, r1]
    1948:	e2422001 	sub	r2, r2, #1
    194c:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    1950:	e121f000 	msr	CPSR_c, r0
	dev_put(c->reply_dev);
    1954:	e5943060 	ldr	r3, [r4, #96]	@ 0x60
	if (dev) {
    1958:	e3530000 	cmp	r3, #0
    195c:	0a000007 	beq	1980 <sfe_ipv6_flush_connection.constprop.1+0x25c>
	asm volatile(
    1960:	e10f0000 	mrs	r0, CPSR
    1964:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    1968:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    196c:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    1970:	e7932001 	ldr	r2, [r3, r1]
    1974:	e2422001 	sub	r2, r2, #1
    1978:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    197c:	e121f000 	msr	CPSR_c, r0
	kfree(c->original_match);
    1980:	e5940054 	ldr	r0, [r4, #84]	@ 0x54
    1984:	ebfffffe 	bl	0 <kfree>
	kfree(c->reply_match);
    1988:	e594005c 	ldr	r0, [r4, #92]	@ 0x5c
    198c:	ebfffffe 	bl	0 <kfree>
	kfree(c);
    1990:	e1a00004 	mov	r0, r4
    1994:	ebfffffe 	bl	0 <kfree>
}
    1998:	e24bd028 	sub	sp, fp, #40	@ 0x28
    199c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

000019a0 <sfe_ipv4_update_tcp_state.isra.0>:
sfe_ipv4_update_tcp_state(struct sfe_ipv4_connection *c,
    19a0:	e1a0c00d 	mov	ip, sp
    19a4:	e92dd800 	push	{fp, ip, lr, pc}
    19a8:	e24cb004 	sub	fp, ip, #4
	if (orig_tcp->max_win < sic->src_td_max_window) {
    19ac:	e590c03c 	ldr	ip, [r0, #60]	@ 0x3c
    19b0:	e592307c 	ldr	r3, [r2, #124]	@ 0x7c
    19b4:	e15c0003 	cmp	ip, r3
		orig_tcp->max_win = sic->src_td_max_window;
    19b8:	3580303c 	strcc	r3, [r0, #60]	@ 0x3c
	if ((s32)(orig_tcp->end - sic->src_td_end) < 0) {
    19bc:	e5903040 	ldr	r3, [r0, #64]	@ 0x40
    19c0:	e592c080 	ldr	ip, [r2, #128]	@ 0x80
    19c4:	e043300c 	sub	r3, r3, ip
    19c8:	e3530000 	cmp	r3, #0
	if ((s32)(orig_tcp->max_end - sic->src_td_max_end) < 0) {
    19cc:	e5903044 	ldr	r3, [r0, #68]	@ 0x44
		orig_tcp->end = sic->src_td_end;
    19d0:	b580c040 	strlt	ip, [r0, #64]	@ 0x40
	if ((s32)(orig_tcp->max_end - sic->src_td_max_end) < 0) {
    19d4:	e592c084 	ldr	ip, [r2, #132]	@ 0x84
    19d8:	e043300c 	sub	r3, r3, ip
    19dc:	e3530000 	cmp	r3, #0
		orig_tcp->max_end = sic->src_td_max_end;
    19e0:	b580c044 	strlt	ip, [r0, #68]	@ 0x44
	if (repl_tcp->max_win < sic->dest_td_max_window) {
    19e4:	e591c03c 	ldr	ip, [r1, #60]	@ 0x3c
    19e8:	e592308c 	ldr	r3, [r2, #140]	@ 0x8c
    19ec:	e15c0003 	cmp	ip, r3
		repl_tcp->max_win = sic->dest_td_max_window;
    19f0:	3581303c 	strcc	r3, [r1, #60]	@ 0x3c
	if ((s32)(repl_tcp->end - sic->dest_td_end) < 0) {
    19f4:	e5913040 	ldr	r3, [r1, #64]	@ 0x40
    19f8:	e592c090 	ldr	ip, [r2, #144]	@ 0x90
    19fc:	e043300c 	sub	r3, r3, ip
    1a00:	e3530000 	cmp	r3, #0
	if ((s32)(repl_tcp->max_end - sic->dest_td_max_end) < 0) {
    1a04:	e5913044 	ldr	r3, [r1, #68]	@ 0x44
		repl_tcp->end = sic->dest_td_end;
    1a08:	b581c040 	strlt	ip, [r1, #64]	@ 0x40
	if ((s32)(repl_tcp->max_end - sic->dest_td_max_end) < 0) {
    1a0c:	e592c094 	ldr	ip, [r2, #148]	@ 0x94
    1a10:	e043300c 	sub	r3, r3, ip
    1a14:	e3530000 	cmp	r3, #0
		repl_tcp->max_end = sic->dest_td_max_end;
    1a18:	b581c044 	strlt	ip, [r1, #68]	@ 0x44
	orig_cm->flags &= ~SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    1a1c:	e5903030 	ldr	r3, [r0, #48]	@ 0x30
    1a20:	e3c33004 	bic	r3, r3, #4
    1a24:	e5803030 	str	r3, [r0, #48]	@ 0x30
	repl_cm->flags &= ~SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    1a28:	e5913030 	ldr	r3, [r1, #48]	@ 0x30
    1a2c:	e3c33004 	bic	r3, r3, #4
    1a30:	e5813030 	str	r3, [r1, #48]	@ 0x30
	if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    1a34:	e592300c 	ldr	r3, [r2, #12]
    1a38:	e3130001 	tst	r3, #1
    1a3c:	0a000005 	beq	1a58 <sfe_ipv4_update_tcp_state.isra.0+0xb8>
		orig_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    1a40:	e5903030 	ldr	r3, [r0, #48]	@ 0x30
    1a44:	e3833004 	orr	r3, r3, #4
    1a48:	e5803030 	str	r3, [r0, #48]	@ 0x30
		repl_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    1a4c:	e5913030 	ldr	r3, [r1, #48]	@ 0x30
    1a50:	e3833004 	orr	r3, r3, #4
    1a54:	e5813030 	str	r3, [r1, #48]	@ 0x30
}
    1a58:	e24bd00c 	sub	sp, fp, #12
    1a5c:	e89da800 	ldm	sp, {fp, sp, pc}

00001a60 <sfe_ipv4_gen_sync_sfe_ipv4_connection.isra.0>:
static void sfe_ipv4_gen_sync_sfe_ipv4_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c,
    1a60:	e1a0c00d 	mov	ip, sp
    1a64:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
    1a68:	e24cb004 	sub	fp, ip, #4
	sis->is_v6 = 0;
    1a6c:	e3a03000 	mov	r3, #0
static void sfe_ipv4_gen_sync_sfe_ipv4_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c,
    1a70:	e99b5000 	ldmib	fp, {ip, lr}
	sis->is_v6 = 0;
    1a74:	e5813008 	str	r3, [r1, #8]
	sis->protocol = c->protocol;
    1a78:	e5903008 	ldr	r3, [r0, #8]
    1a7c:	e581300c 	str	r3, [r1, #12]
	sis->src_ip.ip = c->src_ip;
    1a80:	e590300c 	ldr	r3, [r0, #12]
    1a84:	e5813010 	str	r3, [r1, #16]
	sis->src_ip_xlate.ip = c->src_ip_xlate;
    1a88:	e5903010 	ldr	r3, [r0, #16]
    1a8c:	e5813020 	str	r3, [r1, #32]
	sis->dest_ip.ip = c->dest_ip;
    1a90:	e5903014 	ldr	r3, [r0, #20]
    1a94:	e5813034 	str	r3, [r1, #52]	@ 0x34
	sis->dest_ip_xlate.ip = c->dest_ip_xlate;
    1a98:	e5903018 	ldr	r3, [r0, #24]
    1a9c:	e5813044 	str	r3, [r1, #68]	@ 0x44
	sis->src_port = c->src_port;
    1aa0:	e1d031bc 	ldrh	r3, [r0, #28]
    1aa4:	e1c133b0 	strh	r3, [r1, #48]	@ 0x30
	sis->src_port_xlate = c->src_port_xlate;
    1aa8:	e1d031be 	ldrh	r3, [r0, #30]
    1aac:	e1c133b2 	strh	r3, [r1, #50]	@ 0x32
	sis->dest_port = c->dest_port;
    1ab0:	e1d032b0 	ldrh	r3, [r0, #32]
    1ab4:	e1c135b4 	strh	r3, [r1, #84]	@ 0x54
	sis->dest_port_xlate = c->dest_port_xlate;
    1ab8:	e1d032b2 	ldrh	r3, [r0, #34]	@ 0x22
    1abc:	e1c135b6 	strh	r3, [r1, #86]	@ 0x56
	original_cm = c->original_match;
    1ac0:	e5906024 	ldr	r6, [r0, #36]	@ 0x24
	reply_cm = c->reply_match;
    1ac4:	e590302c 	ldr	r3, [r0, #44]	@ 0x2c
	sis->src_td_max_window = original_cm->protocol_state.tcp.max_win;
    1ac8:	e596403c 	ldr	r4, [r6, #60]	@ 0x3c
    1acc:	e5814058 	str	r4, [r1, #88]	@ 0x58
	sis->src_td_end = original_cm->protocol_state.tcp.end;
    1ad0:	e5964040 	ldr	r4, [r6, #64]	@ 0x40
    1ad4:	e581405c 	str	r4, [r1, #92]	@ 0x5c
	sis->src_td_max_end = original_cm->protocol_state.tcp.max_end;
    1ad8:	e5964044 	ldr	r4, [r6, #68]	@ 0x44
    1adc:	e5814060 	str	r4, [r1, #96]	@ 0x60
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
    1ae0:	e593403c 	ldr	r4, [r3, #60]	@ 0x3c
    1ae4:	e5814080 	str	r4, [r1, #128]	@ 0x80
	sis->dest_td_end = reply_cm->protocol_state.tcp.end;
    1ae8:	e5934040 	ldr	r4, [r3, #64]	@ 0x40
    1aec:	e5814084 	str	r4, [r1, #132]	@ 0x84
	sis->dest_td_max_end = reply_cm->protocol_state.tcp.max_end;
    1af0:	e5934044 	ldr	r4, [r3, #68]	@ 0x44
    1af4:	e5814088 	str	r4, [r1, #136]	@ 0x88
	sis->src_new_packet_count = original_cm->rx_packet_count;
    1af8:	e5964048 	ldr	r4, [r6, #72]	@ 0x48
    1afc:	e5814078 	str	r4, [r1, #120]	@ 0x78
	sis->src_new_byte_count = original_cm->rx_byte_count;
    1b00:	e596404c 	ldr	r4, [r6, #76]	@ 0x4c
    1b04:	e581407c 	str	r4, [r1, #124]	@ 0x7c
	sis->dest_new_packet_count = reply_cm->rx_packet_count;
    1b08:	e5934048 	ldr	r4, [r3, #72]	@ 0x48
    1b0c:	e58140a0 	str	r4, [r1, #160]	@ 0xa0
	sis->dest_new_byte_count = reply_cm->rx_byte_count;
    1b10:	e593404c 	ldr	r4, [r3, #76]	@ 0x4c
    1b14:	e58140a4 	str	r4, [r1, #164]	@ 0xa4
	cm->rx_packet_count64 += cm->rx_packet_count;
    1b18:	e5965048 	ldr	r5, [r6, #72]	@ 0x48
    1b1c:	e5964088 	ldr	r4, [r6, #136]	@ 0x88
    1b20:	e0944005 	adds	r4, r4, r5
	cm->rx_byte_count64 += cm->rx_byte_count;
    1b24:	e596504c 	ldr	r5, [r6, #76]	@ 0x4c
	cm->rx_packet_count64 += cm->rx_packet_count;
    1b28:	e5864088 	str	r4, [r6, #136]	@ 0x88
    1b2c:	e596408c 	ldr	r4, [r6, #140]	@ 0x8c
    1b30:	e2a44000 	adc	r4, r4, #0
    1b34:	e586408c 	str	r4, [r6, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    1b38:	e5964090 	ldr	r4, [r6, #144]	@ 0x90
    1b3c:	e0944005 	adds	r4, r4, r5
	cm->rx_packet_count = 0;
    1b40:	e3a05000 	mov	r5, #0
	cm->rx_byte_count64 += cm->rx_byte_count;
    1b44:	e5864090 	str	r4, [r6, #144]	@ 0x90
    1b48:	e5964094 	ldr	r4, [r6, #148]	@ 0x94
    1b4c:	e2a44000 	adc	r4, r4, #0
    1b50:	e5864094 	str	r4, [r6, #148]	@ 0x94
	cm->rx_packet_count = 0;
    1b54:	e3a04000 	mov	r4, #0
    1b58:	e1c644f8 	strd	r4, [r6, #72]	@ 0x48
	cm->rx_packet_count64 += cm->rx_packet_count;
    1b5c:	e5937048 	ldr	r7, [r3, #72]	@ 0x48
    1b60:	e5938088 	ldr	r8, [r3, #136]	@ 0x88
    1b64:	e0987007 	adds	r7, r8, r7
    1b68:	e5837088 	str	r7, [r3, #136]	@ 0x88
	cm->rx_byte_count64 += cm->rx_byte_count;
    1b6c:	e593704c 	ldr	r7, [r3, #76]	@ 0x4c
	cm->rx_packet_count = 0;
    1b70:	e1c344f8 	strd	r4, [r3, #72]	@ 0x48
	cm->rx_packet_count64 += cm->rx_packet_count;
    1b74:	e593408c 	ldr	r4, [r3, #140]	@ 0x8c
    1b78:	e2a44000 	adc	r4, r4, #0
    1b7c:	e583408c 	str	r4, [r3, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    1b80:	e5934090 	ldr	r4, [r3, #144]	@ 0x90
    1b84:	e0944007 	adds	r4, r4, r7
    1b88:	e5834090 	str	r4, [r3, #144]	@ 0x90
    1b8c:	e5934094 	ldr	r4, [r3, #148]	@ 0x94
    1b90:	e2a44000 	adc	r4, r4, #0
    1b94:	e5834094 	str	r4, [r3, #148]	@ 0x94
	sis->src_dev = original_cm->match_dev;
    1b98:	e596401c 	ldr	r4, [r6, #28]
    1b9c:	e5814000 	str	r4, [r1]
	sis->src_packet_count = original_cm->rx_packet_count64;
    1ba0:	e1c648d8 	ldrd	r4, [r6, #136]	@ 0x88
    1ba4:	e1c146f8 	strd	r4, [r1, #104]	@ 0x68
	sis->src_byte_count = original_cm->rx_byte_count64;
    1ba8:	e1c649d0 	ldrd	r4, [r6, #144]	@ 0x90
    1bac:	e1c147f0 	strd	r4, [r1, #112]	@ 0x70
	sis->dest_dev = reply_cm->match_dev;
    1bb0:	e593401c 	ldr	r4, [r3, #28]
    1bb4:	e5814004 	str	r4, [r1, #4]
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    1bb8:	e1c348d8 	ldrd	r4, [r3, #136]	@ 0x88
    1bbc:	e1c149f0 	strd	r4, [r1, #144]	@ 0x90
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    1bc0:	e1c349d0 	ldrd	r4, [r3, #144]	@ 0x90
    1bc4:	e1c149f8 	strd	r4, [r1, #152]	@ 0x98
	sis->reason = reason;
    1bc8:	e58120a8 	str	r2, [r1, #168]	@ 0xa8
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    1bcc:	e5903038 	ldr	r3, [r0, #56]	@ 0x38
    1bd0:	e590203c 	ldr	r2, [r0, #60]	@ 0x3c
    1bd4:	e05c3003 	subs	r3, ip, r3
    1bd8:	e0ce2002 	sbc	r2, lr, r2
    1bdc:	e58130b0 	str	r3, [r1, #176]	@ 0xb0
    1be0:	e58120b4 	str	r2, [r1, #180]	@ 0xb4
	c->last_sync_jiffies = now_jiffies;
    1be4:	e580c038 	str	ip, [r0, #56]	@ 0x38
    1be8:	e580e03c 	str	lr, [r0, #60]	@ 0x3c
}
    1bec:	e24bd020 	sub	sp, fp, #32
    1bf0:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}

00001bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>:
static void sfe_ipv4_flush_sfe_ipv4_connection(struct sfe_ipv4 *si,
    1bf4:	e1a0c00d 	mov	ip, sp
    1bf8:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
    1bfc:	e24cb004 	sub	fp, ip, #4
    1c00:	e24dd0c0 	sub	sp, sp, #192	@ 0xc0
    1c04:	e1a04000 	mov	r4, r0
	raw_spin_lock_bh(&lock->rlock);
    1c08:	e3005000 	movw	r5, #0
    1c0c:	e3405000 	movt	r5, #0
    1c10:	e1a00005 	mov	r0, r5
    1c14:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_flushes++;
    1c18:	e2852902 	add	r2, r5, #32768	@ 0x8000
	raw_spin_unlock_bh(&lock->rlock);
    1c1c:	e1a00005 	mov	r0, r5
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    1c20:	e595502c 	ldr	r5, [r5, #44]	@ 0x2c
	si->connection_flushes++;
    1c24:	e5923048 	ldr	r3, [r2, #72]	@ 0x48
    1c28:	e2833001 	add	r3, r3, #1
    1c2c:	e5823048 	str	r3, [r2, #72]	@ 0x48
    1c30:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (sync_rule_callback) {
    1c34:	e3550000 	cmp	r5, #0
    1c38:	0a000007 	beq	1c5c <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1+0x68>
		now_jiffies = get_jiffies_64();
    1c3c:	ebfffffe 	bl	0 <get_jiffies_64>
		sfe_ipv4_gen_sync_sfe_ipv4_connection(si, c, &sis, reason, now_jiffies);
    1c40:	e3a02001 	mov	r2, #1
    1c44:	e1cd00f0 	strd	r0, [sp]
    1c48:	e24b10cc 	sub	r1, fp, #204	@ 0xcc
    1c4c:	e1a00004 	mov	r0, r4
    1c50:	ebffff82 	bl	1a60 <sfe_ipv4_gen_sync_sfe_ipv4_connection.isra.0>
		sync_rule_callback(&sis);
    1c54:	e1a00001 	mov	r0, r1
    1c58:	e12fff35 	blx	r5
	dev_put(c->original_dev);
    1c5c:	e5943028 	ldr	r3, [r4, #40]	@ 0x28
	if (dev) {
    1c60:	e3530000 	cmp	r3, #0
    1c64:	0a000007 	beq	1c88 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1+0x94>
	asm volatile(
    1c68:	e10f0000 	mrs	r0, CPSR
    1c6c:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    1c70:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    1c74:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    1c78:	e7932001 	ldr	r2, [r3, r1]
    1c7c:	e2422001 	sub	r2, r2, #1
    1c80:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    1c84:	e121f000 	msr	CPSR_c, r0
	dev_put(c->reply_dev);
    1c88:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
	if (dev) {
    1c8c:	e3530000 	cmp	r3, #0
    1c90:	0a000007 	beq	1cb4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1+0xc0>
	asm volatile(
    1c94:	e10f0000 	mrs	r0, CPSR
    1c98:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    1c9c:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    1ca0:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    1ca4:	e7932001 	ldr	r2, [r3, r1]
    1ca8:	e2422001 	sub	r2, r2, #1
    1cac:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    1cb0:	e121f000 	msr	CPSR_c, r0
	kfree(c->original_match);
    1cb4:	e5940024 	ldr	r0, [r4, #36]	@ 0x24
    1cb8:	ebfffffe 	bl	0 <kfree>
	kfree(c->reply_match);
    1cbc:	e594002c 	ldr	r0, [r4, #44]	@ 0x2c
    1cc0:	ebfffffe 	bl	0 <kfree>
	kfree(c);
    1cc4:	e1a00004 	mov	r0, r4
    1cc8:	ebfffffe 	bl	0 <kfree>
}
    1ccc:	e24bd014 	sub	sp, fp, #20
    1cd0:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

00001cd4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0>:
static void sfe_ipv4_flush_sfe_ipv4_connection(struct sfe_ipv4 *si,
    1cd4:	e1a0c00d 	mov	ip, sp
    1cd8:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
    1cdc:	e24cb004 	sub	fp, ip, #4
    1ce0:	e24dd0c0 	sub	sp, sp, #192	@ 0xc0
    1ce4:	e1a04000 	mov	r4, r0
	raw_spin_lock_bh(&lock->rlock);
    1ce8:	e3005000 	movw	r5, #0
    1cec:	e3405000 	movt	r5, #0
    1cf0:	e1a00005 	mov	r0, r5
    1cf4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_flushes++;
    1cf8:	e2852902 	add	r2, r5, #32768	@ 0x8000
	raw_spin_unlock_bh(&lock->rlock);
    1cfc:	e1a00005 	mov	r0, r5
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    1d00:	e595502c 	ldr	r5, [r5, #44]	@ 0x2c
	si->connection_flushes++;
    1d04:	e5923048 	ldr	r3, [r2, #72]	@ 0x48
    1d08:	e2833001 	add	r3, r3, #1
    1d0c:	e5823048 	str	r3, [r2, #72]	@ 0x48
    1d10:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (sync_rule_callback) {
    1d14:	e3550000 	cmp	r5, #0
    1d18:	0a000007 	beq	1d3c <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0+0x68>
		now_jiffies = get_jiffies_64();
    1d1c:	ebfffffe 	bl	0 <get_jiffies_64>
		sfe_ipv4_gen_sync_sfe_ipv4_connection(si, c, &sis, reason, now_jiffies);
    1d20:	e3a02002 	mov	r2, #2
    1d24:	e1cd00f0 	strd	r0, [sp]
    1d28:	e24b10cc 	sub	r1, fp, #204	@ 0xcc
    1d2c:	e1a00004 	mov	r0, r4
    1d30:	ebffff4a 	bl	1a60 <sfe_ipv4_gen_sync_sfe_ipv4_connection.isra.0>
		sync_rule_callback(&sis);
    1d34:	e1a00001 	mov	r0, r1
    1d38:	e12fff35 	blx	r5
	dev_put(c->original_dev);
    1d3c:	e5943028 	ldr	r3, [r4, #40]	@ 0x28
	if (dev) {
    1d40:	e3530000 	cmp	r3, #0
    1d44:	0a000007 	beq	1d68 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0+0x94>
	asm volatile(
    1d48:	e10f0000 	mrs	r0, CPSR
    1d4c:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    1d50:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    1d54:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    1d58:	e7932001 	ldr	r2, [r3, r1]
    1d5c:	e2422001 	sub	r2, r2, #1
    1d60:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    1d64:	e121f000 	msr	CPSR_c, r0
	dev_put(c->reply_dev);
    1d68:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
	if (dev) {
    1d6c:	e3530000 	cmp	r3, #0
    1d70:	0a000007 	beq	1d94 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0+0xc0>
	asm volatile(
    1d74:	e10f0000 	mrs	r0, CPSR
    1d78:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    1d7c:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    1d80:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    1d84:	e7932001 	ldr	r2, [r3, r1]
    1d88:	e2422001 	sub	r2, r2, #1
    1d8c:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    1d90:	e121f000 	msr	CPSR_c, r0
	kfree(c->original_match);
    1d94:	e5940024 	ldr	r0, [r4, #36]	@ 0x24
    1d98:	ebfffffe 	bl	0 <kfree>
	kfree(c->reply_match);
    1d9c:	e594002c 	ldr	r0, [r4, #44]	@ 0x2c
    1da0:	ebfffffe 	bl	0 <kfree>
	kfree(c);
    1da4:	e1a00004 	mov	r0, r4
    1da8:	ebfffffe 	bl	0 <kfree>
}
    1dac:	e24bd014 	sub	sp, fp, #20
    1db0:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}

00001db4 <sfe_ipv6_debug_dev_release>:

/*
 * sfe_ipv6_debug_dev_release()
 */
static int sfe_ipv6_debug_dev_release(struct inode *inode, struct file *file)
    1db4:	e1a0c00d 	mov	ip, sp
    1db8:	e92dd800 	push	{fp, ip, lr, pc}
    1dbc:	e24cb004 	sub	fp, ip, #4
    1dc0:	e5910090 	ldr	r0, [r1, #144]	@ 0x90
    1dc4:	e3500000 	cmp	r0, #0
    1dc8:	0a000000 	beq	1dd0 <sfe_ipv6_debug_dev_release+0x1c>
    1dcc:	ebfffffe 	bl	0 <kfree>
    1dd0:	e3a00000 	mov	r0, #0
    1dd4:	e24bd00c 	sub	sp, fp, #12
    1dd8:	e89da800 	ldm	sp, {fp, sp, pc}

00001ddc <fast_classifier_sync_rule>:
{
    1ddc:	e1a0c00d 	mov	ip, sp
    1de0:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
    1de4:	e24cb004 	sub	fp, ip, #4
    1de8:	e24dd0f4 	sub	sp, sp, #244	@ 0xf4
	if (sis->is_v6) {
    1dec:	e590c008 	ldr	ip, [r0, #8]
{
    1df0:	e1a05000 	mov	r5, r0
	memset(&tuple, 0, sizeof(tuple));
    1df4:	e3a02000 	mov	r2, #0
	tuple.src.u.all = (__be16)sis->src_port;
    1df8:	e1d013b0 	ldrh	r1, [r0, #48]	@ 0x30
	memset(&tuple, 0, sizeof(tuple));
    1dfc:	e3a03000 	mov	r3, #0
	tuple.dst.protonum = (u8)sis->protocol;
    1e00:	e590000c 	ldr	r0, [r0, #12]
	if (sis->is_v6) {
    1e04:	e35c0000 	cmp	ip, #0
	tuple.src.u.all = (__be16)sis->src_port;
    1e08:	e24bcf41 	sub	ip, fp, #260	@ 0x104
    1e0c:	e1cc10b0 	strh	r1, [ip]
	memset(&tuple, 0, sizeof(tuple));
    1e10:	e24bcf45 	sub	ip, fp, #276	@ 0x114
	tuple.dst.u.all = (__be16)sis->dest_port;
    1e14:	e1d515b4 	ldrh	r1, [r5, #84]	@ 0x54
	memset(&tuple, 0, sizeof(tuple));
    1e18:	e1cc20f0 	strd	r2, [ip]
    1e1c:	e24bcf43 	sub	ip, fp, #268	@ 0x10c
    1e20:	e1cc20f0 	strd	r2, [ip]
    1e24:	e14b2ffc 	strd	r2, [fp, #-252]	@ 0xffffff04
    1e28:	e14b2ff4 	strd	r2, [fp, #-244]	@ 0xffffff0c
	tuple.dst.u.all = (__be16)sis->dest_port;
    1e2c:	e14b1fb0 	strh	r1, [fp, #-240]	@ 0xffffff10
	tuple.dst.protonum = (u8)sis->protocol;
    1e30:	e54b00ee 	strb	r0, [fp, #-238]	@ 0xffffff12
	if (sis->is_v6) {
    1e34:	0a0000ac 	beq	20ec <fast_classifier_sync_rule+0x310>
		tuple.src.u3.in6 = *((struct in6_addr *)sis->src_ip.ip6);
    1e38:	e1c501d0 	ldrd	r0, [r5, #16]
    1e3c:	e24bcf45 	sub	ip, fp, #276	@ 0x114
    1e40:	e1c521d8 	ldrd	r2, [r5, #24]
    1e44:	e1cc00f0 	strd	r0, [ip]
    1e48:	e24b1f43 	sub	r1, fp, #268	@ 0x10c
		tuple.dst.u3.in6 = *((struct in6_addr *)sis->dest_ip.ip6);
    1e4c:	e24bcc01 	sub	ip, fp, #256	@ 0x100
		tuple.src.u3.in6 = *((struct in6_addr *)sis->src_ip.ip6);
    1e50:	e1c120f0 	strd	r2, [r1]
		tuple.dst.u3.in6 = *((struct in6_addr *)sis->dest_ip.ip6);
    1e54:	e1c523dc 	ldrd	r2, [r5, #60]	@ 0x3c
    1e58:	e1c503d4 	ldrd	r0, [r5, #52]	@ 0x34
    1e5c:	e14b2ff8 	strd	r2, [fp, #-248]	@ 0xffffff08
		tuple.src.l3num = AF_INET6;
    1e60:	e3a0300a 	mov	r3, #10
		tuple.dst.u3.in6 = *((struct in6_addr *)sis->dest_ip.ip6);
    1e64:	e1cc00f0 	strd	r0, [ip]
    1e68:	e24b2c01 	sub	r2, fp, #256	@ 0x100
	if (skip_to_bridge_ingress) {
    1e6c:	e3004000 	movw	r4, #0
    1e70:	e3404000 	movt	r4, #0
    1e74:	e2422002 	sub	r2, r2, #2
    1e78:	e1c230b0 	strh	r3, [r2]
    1e7c:	e5d43010 	ldrb	r3, [r4, #16]
    1e80:	e3530000 	cmp	r3, #0
    1e84:	0a00001f 	beq	1f08 <fast_classifier_sync_rule+0x12c>
		if (sis->src_dev && sis->src_dev->priv_flags & IFF_EBRIDGE &&
    1e88:	e5951000 	ldr	r1, [r5]
		nlstats.tx_packets = 0;
    1e8c:	e3a02000 	mov	r2, #0
    1e90:	e3a03000 	mov	r3, #0
    1e94:	e14b2ef4 	strd	r2, [fp, #-228]	@ 0xffffff1c
		nlstats.tx_bytes = 0;
    1e98:	e14b2df4 	strd	r2, [fp, #-212]	@ 0xffffff2c
		if (sis->src_dev && sis->src_dev->priv_flags & IFF_EBRIDGE &&
    1e9c:	e3510000 	cmp	r1, #0
    1ea0:	0a000011 	beq	1eec <fast_classifier_sync_rule+0x110>
    1ea4:	e5913070 	ldr	r3, [r1, #112]	@ 0x70
    1ea8:	e3130002 	tst	r3, #2
    1eac:	0a00000e 	beq	1eec <fast_classifier_sync_rule+0x110>
		    (sis->src_new_packet_count || sis->src_new_byte_count)) {
    1eb0:	e5951078 	ldr	r1, [r5, #120]	@ 0x78
    1eb4:	e595307c 	ldr	r3, [r5, #124]	@ 0x7c
    1eb8:	e1910003 	orrs	r0, r1, r3
    1ebc:	0a00000a 	beq	1eec <fast_classifier_sync_rule+0x110>
	raw_spin_lock_bh(&lock->rlock);
    1ec0:	e2840014 	add	r0, r4, #20
			nlstats.rx_packets = sis->src_new_packet_count;
    1ec4:	e50b10ec 	str	r1, [fp, #-236]	@ 0xffffff14
    1ec8:	e50b20e8 	str	r2, [fp, #-232]	@ 0xffffff18
			nlstats.rx_bytes = sis->src_new_byte_count;
    1ecc:	e50b30dc 	str	r3, [fp, #-220]	@ 0xffffff24
    1ed0:	e50b20d8 	str	r2, [fp, #-216]	@ 0xffffff28
    1ed4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
			br_dev_update_stats(sis->src_dev, &nlstats);
    1ed8:	e5950000 	ldr	r0, [r5]
    1edc:	e24b10ec 	sub	r1, fp, #236	@ 0xec
    1ee0:	ebfffffe 	bl	0 <br_dev_update_stats>
	raw_spin_unlock_bh(&lock->rlock);
    1ee4:	e2840014 	add	r0, r4, #20
    1ee8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		if (sis->dest_dev && sis->dest_dev->priv_flags & IFF_EBRIDGE &&
    1eec:	e5953004 	ldr	r3, [r5, #4]
    1ef0:	e3530000 	cmp	r3, #0
    1ef4:	0a000003 	beq	1f08 <fast_classifier_sync_rule+0x12c>
    1ef8:	e5933070 	ldr	r3, [r3, #112]	@ 0x70
    1efc:	e3a02000 	mov	r2, #0
    1f00:	e3130002 	tst	r3, #2
    1f04:	1a0000ac 	bne	21bc <fast_classifier_sync_rule+0x3e0>
	h = nf_conntrack_find_get(&init_net, SFE_NF_CT_DEFAULT_ZONE, &tuple);
    1f08:	e3001000 	movw	r1, #0
    1f0c:	e3000000 	movw	r0, #0
    1f10:	e24b2f45 	sub	r2, fp, #276	@ 0x114
    1f14:	e3401000 	movt	r1, #0
    1f18:	e3400000 	movt	r0, #0
    1f1c:	ebfffffe 	bl	0 <nf_conntrack_find_get>
	if (unlikely(!h)) {
    1f20:	e3500000 	cmp	r0, #0
    1f24:	0a000050 	beq	206c <fast_classifier_sync_rule+0x290>
}

static inline struct nf_conn *
nf_ct_tuplehash_to_ctrack(const struct nf_conntrack_tuple_hash *hash)
{
	return container_of(hash, struct nf_conn,
    1f28:	e5d0302f 	ldrb	r3, [r0, #47]	@ 0x2f
    1f2c:	e3e0200b 	mvn	r2, #11
    1f30:	e0833083 	add	r3, r3, r3, lsl #1
    1f34:	e0423203 	sub	r3, r2, r3, lsl #4
    1f38:	e0804003 	add	r4, r0, r3
	/*
	 * Unlike the bitops with the '__' prefix above, this one *is* atomic,
	 * so `volatile` must always stay here with no cast-aways. See
	 * `Documentation/atomic_bitops.txt` for the details.
	 */
	return 1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1)));
    1f3c:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
	if (!test_bit(IPS_FIXED_TIMEOUT_BIT, &ct->status)) {
    1f40:	e3130b01 	tst	r3, #1024	@ 0x400
    1f44:	0a000092 	beq	2194 <fast_classifier_sync_rule+0x3b8>
};

static inline
struct nf_conn_acct *nf_conn_acct_find(const struct nf_conn *ct)
{
	return nf_ct_ext_find(ct, NF_CT_EXT_ACCT);
    1f48:	e5946080 	ldr	r6, [r4, #128]	@ 0x80

static inline void *nf_ct_ext_find(const struct nf_conn *ct, u8 id)
{
	struct nf_ct_ext *ext = ct->ext;

	if (!ext || !__nf_ct_ext_exist(ext, id))
    1f4c:	e3560000 	cmp	r6, #0
    1f50:	0a000033 	beq	2024 <fast_classifier_sync_rule+0x248>
	return !!ext->offset[id];
    1f54:	e5d63003 	ldrb	r3, [r6, #3]
	if (!ext || !__nf_ct_ext_exist(ext, id))
    1f58:	e3530000 	cmp	r3, #0
		return NULL;
    1f5c:	01a06003 	moveq	r6, r3
	if (!ext || !__nf_ct_ext_exist(ext, id))
    1f60:	0a00002f 	beq	2024 <fast_classifier_sync_rule+0x248>

	if (unlikely(ext->gen_id))
    1f64:	e5962008 	ldr	r2, [r6, #8]
    1f68:	e3520000 	cmp	r2, #0
		return __nf_ct_ext_find(ext, id);

	return (void *)ct->ext + ct->ext->offset[id];
    1f6c:	00866003 	addeq	r6, r6, r3
	if (unlikely(ext->gen_id))
    1f70:	1a0000aa 	bne	2220 <fast_classifier_sync_rule+0x444>
	if (acct) {
    1f74:	e3560000 	cmp	r6, #0
    1f78:	0a000029 	beq	2024 <fast_classifier_sync_rule+0x248>
	raw_spin_lock_bh(&lock->rlock);
    1f7c:	e2847004 	add	r7, r4, #4
    1f80:	e1a00007 	mov	r0, r7
    1f84:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
    1f88:	f596f000 	pldw	[r6]
		atomic64_add(sis->src_new_packet_count, &SFE_ACCT_COUNTER(acct)[IP_CT_DIR_ORIGINAL].packets);
    1f8c:	e3a0c000 	mov	ip, #0
    1f90:	e5952078 	ldr	r2, [r5, #120]	@ 0x78
    1f94:	e1a0300c 	mov	r3, ip
#define ATOMIC64_OPS(op, op1, op2)					\
	ATOMIC64_OP(op, op1, op2)					\
	ATOMIC64_OP_RETURN(op, op1, op2)				\
	ATOMIC64_FETCH_OP(op, op1, op2)

ATOMIC64_OPS(add, adds, adc)
    1f98:	e1b60f9f 	ldrexd	r0, [r6]
    1f9c:	e0900002 	adds	r0, r0, r2
    1fa0:	e0a11003 	adc	r1, r1, r3
    1fa4:	e1a6ef90 	strexd	lr, r0, [r6]
    1fa8:	e33e0000 	teq	lr, #0
    1fac:	1afffff9 	bne	1f98 <fast_classifier_sync_rule+0x1bc>
		atomic64_add(sis->src_new_byte_count, &SFE_ACCT_COUNTER(acct)[IP_CT_DIR_ORIGINAL].bytes);
    1fb0:	e286e008 	add	lr, r6, #8
    1fb4:	f59ef000 	pldw	[lr]
    1fb8:	e595207c 	ldr	r2, [r5, #124]	@ 0x7c
    1fbc:	e1be0f9f 	ldrexd	r0, [lr]
    1fc0:	e0900002 	adds	r0, r0, r2
    1fc4:	e0a11003 	adc	r1, r1, r3
    1fc8:	e1ae8f90 	strexd	r8, r0, [lr]
    1fcc:	e3380000 	teq	r8, #0
    1fd0:	1afffff9 	bne	1fbc <fast_classifier_sync_rule+0x1e0>
		atomic64_add(sis->dest_new_packet_count, &SFE_ACCT_COUNTER(acct)[IP_CT_DIR_REPLY].packets);
    1fd4:	e286e010 	add	lr, r6, #16
    1fd8:	f59ef000 	pldw	[lr]
    1fdc:	e59520a0 	ldr	r2, [r5, #160]	@ 0xa0
    1fe0:	e1be0f9f 	ldrexd	r0, [lr]
    1fe4:	e0900002 	adds	r0, r0, r2
    1fe8:	e0a11003 	adc	r1, r1, r3
    1fec:	e1ae8f90 	strexd	r8, r0, [lr]
    1ff0:	e3380000 	teq	r8, #0
    1ff4:	1afffff9 	bne	1fe0 <fast_classifier_sync_rule+0x204>
		atomic64_add(sis->dest_new_byte_count, &SFE_ACCT_COUNTER(acct)[IP_CT_DIR_REPLY].bytes);
    1ff8:	e286e018 	add	lr, r6, #24
    1ffc:	f59ef000 	pldw	[lr]
    2000:	e59520a4 	ldr	r2, [r5, #164]	@ 0xa4
    2004:	e1be0f9f 	ldrexd	r0, [lr]
    2008:	e0900002 	adds	r0, r0, r2
    200c:	e0a11003 	adc	r1, r1, r3
    2010:	e1aecf90 	strexd	ip, r0, [lr]
    2014:	e33c0000 	teq	ip, #0
    2018:	1afffff9 	bne	2004 <fast_classifier_sync_rule+0x228>
	raw_spin_unlock_bh(&lock->rlock);
    201c:	e1a00007 	mov	r0, r7
    2020:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	switch (sis->protocol) {
    2024:	e595300c 	ldr	r3, [r5, #12]
    2028:	e3530006 	cmp	r3, #6
    202c:	0a000034 	beq	2104 <fast_classifier_sync_rule+0x328>
    2030:	e3530011 	cmp	r3, #17
    2034:	0a00000e 	beq	2074 <fast_classifier_sync_rule+0x298>
void nf_conntrack_tcp_set_closing(struct nf_conn *ct);

/* decrement reference count on a conntrack */
static inline void nf_ct_put(struct nf_conn *ct)
{
	if (ct && refcount_dec_and_test(&ct->ct_general.use))
    2038:	e3540000 	cmp	r4, #0
    203c:	0a00000a 	beq	206c <fast_classifier_sync_rule+0x290>

#ifndef arch_atomic_fetch_sub_release
static __always_inline int
arch_atomic_fetch_sub_release(int i, atomic_t *v)
{
	__atomic_release_fence();
    2040:	f57ff05b 	dmb	ish
    2044:	f594f000 	pldw	[r4]
ATOMIC_OPS(sub, -=, sub)
    2048:	e1943f9f 	ldrex	r3, [r4]
    204c:	e2432001 	sub	r2, r3, #1
    2050:	e1841f92 	strex	r1, r2, [r4]
    2054:	e3310000 	teq	r1, #0
    2058:	1afffffa 	bne	2048 <fast_classifier_sync_rule+0x26c>
	int old = atomic_fetch_sub_release(i, &r->refs);

	if (oldp)
		*oldp = old;

	if (old == i) {
    205c:	e3530001 	cmp	r3, #1
    2060:	0a000069 	beq	220c <fast_classifier_sync_rule+0x430>
		smp_acquire__after_ctrl_dep();
		return true;
	}

	if (unlikely(old < 0 || old - i < 0))
    2064:	e3530000 	cmp	r3, #0
    2068:	da000063 	ble	21fc <fast_classifier_sync_rule+0x420>
}
    206c:	e24bd020 	sub	sp, fp, #32
    2070:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
    2074:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
		if (!test_bit(IPS_ASSURED_BIT, &ct->status) && acct) {
    2078:	e3130004 	tst	r3, #4
    207c:	1a000005 	bne	2098 <fast_classifier_sync_rule+0x2bc>
    2080:	e3560000 	cmp	r6, #0
    2084:	0a000003 	beq	2098 <fast_classifier_sync_rule+0x2bc>
			if (atomic64_read(&SFE_ACCT_COUNTER(acct)[IP_CT_DIR_REPLY].packets)) {
    2088:	e2866010 	add	r6, r6, #16
	__asm__ __volatile__("@ atomic64_read\n"
    208c:	e1b62f9f 	ldrexd	r2, [r6]
    2090:	e1932002 	orrs	r2, r3, r2
    2094:	1a000066 	bne	2234 <fast_classifier_sync_rule+0x458>
			timeouts = udp_get_timeouts(nf_ct_net(ct));
    2098:	e3000000 	movw	r0, #0
	raw_spin_lock_bh(&lock->rlock);
    209c:	e2846004 	add	r6, r4, #4
    20a0:	e3400000 	movt	r0, #0
    20a4:	ebfffffe 	bl	0 <udp_get_timeouts>
    20a8:	e1a05000 	mov	r5, r0
    20ac:	e1a00006 	mov	r0, r6
    20b0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
    20b4:	e594306c 	ldr	r3, [r4, #108]	@ 0x6c
	raw_spin_unlock_bh(&lock->rlock);
    20b8:	e1a00006 	mov	r0, r6
		if (test_bit(IPS_SEEN_REPLY_BIT, &ct->status)) {
    20bc:	e3130002 	tst	r3, #2
			ct->timeout = nfct_time_stamp + timeouts[UDP_CT_REPLIED];
    20c0:	e3003000 	movw	r3, #0
    20c4:	e3403000 	movt	r3, #0
    20c8:	15952004 	ldrne	r2, [r5, #4]
    20cc:	e5933000 	ldr	r3, [r3]
			ct->timeout = nfct_time_stamp + timeouts[UDP_CT_UNREPLIED];
    20d0:	05952000 	ldreq	r2, [r5]
    20d4:	e0833002 	add	r3, r3, r2
    20d8:	e5843008 	str	r3, [r4, #8]
    20dc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    20e0:	e3540000 	cmp	r4, #0
    20e4:	1affffd5 	bne	2040 <fast_classifier_sync_rule+0x264>
    20e8:	eaffffdf 	b	206c <fast_classifier_sync_rule+0x290>
		tuple.src.u3.ip = sis->src_ip.ip;
    20ec:	e5951010 	ldr	r1, [r5, #16]
		tuple.src.l3num = AF_INET;
    20f0:	e3a03002 	mov	r3, #2
		tuple.dst.u3.ip = sis->dest_ip.ip;
    20f4:	e5952034 	ldr	r2, [r5, #52]	@ 0x34
		tuple.src.u3.ip = sis->src_ip.ip;
    20f8:	e50b1114 	str	r1, [fp, #-276]	@ 0xfffffeec
		tuple.dst.u3.ip = sis->dest_ip.ip;
    20fc:	e50b2100 	str	r2, [fp, #-256]	@ 0xffffff00
		tuple.src.l3num = AF_INET;
    2100:	eaffff58 	b	1e68 <fast_classifier_sync_rule+0x8c>
	raw_spin_lock_bh(&lock->rlock);
    2104:	e2846004 	add	r6, r4, #4
    2108:	e1a00006 	mov	r0, r6
    210c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		if (ct->proto.tcp.seen[0].td_maxwin < sis->src_td_max_window) {
    2110:	e5953058 	ldr	r3, [r5, #88]	@ 0x58
	raw_spin_unlock_bh(&lock->rlock);
    2114:	e1a00006 	mov	r0, r6
    2118:	e5942098 	ldr	r2, [r4, #152]	@ 0x98
    211c:	e1520003 	cmp	r2, r3
			ct->proto.tcp.seen[0].td_maxwin = sis->src_td_max_window;
    2120:	35843098 	strcc	r3, [r4, #152]	@ 0x98
		if ((s32)(ct->proto.tcp.seen[0].td_end - sis->src_td_end) < 0) {
    2124:	e595205c 	ldr	r2, [r5, #92]	@ 0x5c
    2128:	e5943090 	ldr	r3, [r4, #144]	@ 0x90
    212c:	e0433002 	sub	r3, r3, r2
    2130:	e3530000 	cmp	r3, #0
		if ((s32)(ct->proto.tcp.seen[0].td_maxend - sis->src_td_max_end) < 0) {
    2134:	e5943094 	ldr	r3, [r4, #148]	@ 0x94
			ct->proto.tcp.seen[0].td_end = sis->src_td_end;
    2138:	b5842090 	strlt	r2, [r4, #144]	@ 0x90
		if ((s32)(ct->proto.tcp.seen[0].td_maxend - sis->src_td_max_end) < 0) {
    213c:	e5952060 	ldr	r2, [r5, #96]	@ 0x60
    2140:	e0433002 	sub	r3, r3, r2
    2144:	e3530000 	cmp	r3, #0
			ct->proto.tcp.seen[0].td_maxend = sis->src_td_max_end;
    2148:	b5842094 	strlt	r2, [r4, #148]	@ 0x94
		if (ct->proto.tcp.seen[1].td_maxwin < sis->dest_td_max_window) {
    214c:	e5953080 	ldr	r3, [r5, #128]	@ 0x80
    2150:	e59420ac 	ldr	r2, [r4, #172]	@ 0xac
    2154:	e1520003 	cmp	r2, r3
			ct->proto.tcp.seen[1].td_maxwin = sis->dest_td_max_window;
    2158:	358430ac 	strcc	r3, [r4, #172]	@ 0xac
		if ((s32)(ct->proto.tcp.seen[1].td_end - sis->dest_td_end) < 0) {
    215c:	e5952084 	ldr	r2, [r5, #132]	@ 0x84
    2160:	e59430a4 	ldr	r3, [r4, #164]	@ 0xa4
    2164:	e0433002 	sub	r3, r3, r2
    2168:	e3530000 	cmp	r3, #0
		if ((s32)(ct->proto.tcp.seen[1].td_maxend - sis->dest_td_max_end) < 0) {
    216c:	e59430a8 	ldr	r3, [r4, #168]	@ 0xa8
			ct->proto.tcp.seen[1].td_end = sis->dest_td_end;
    2170:	b58420a4 	strlt	r2, [r4, #164]	@ 0xa4
		if ((s32)(ct->proto.tcp.seen[1].td_maxend - sis->dest_td_max_end) < 0) {
    2174:	e5952088 	ldr	r2, [r5, #136]	@ 0x88
    2178:	e0433002 	sub	r3, r3, r2
    217c:	e3530000 	cmp	r3, #0
			ct->proto.tcp.seen[1].td_maxend = sis->dest_td_max_end;
    2180:	b58420a8 	strlt	r2, [r4, #168]	@ 0xa8
    2184:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    2188:	e3540000 	cmp	r4, #0
    218c:	1affffab 	bne	2040 <fast_classifier_sync_rule+0x264>
    2190:	eaffffb5 	b	206c <fast_classifier_sync_rule+0x290>
	raw_spin_lock_bh(&lock->rlock);
    2194:	e2846004 	add	r6, r4, #4
    2198:	e1a00006 	mov	r0, r6
    219c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		ct->timeout += sis->delta_jiffies;
    21a0:	e5943008 	ldr	r3, [r4, #8]
	raw_spin_unlock_bh(&lock->rlock);
    21a4:	e1a00006 	mov	r0, r6
    21a8:	e59520b0 	ldr	r2, [r5, #176]	@ 0xb0
    21ac:	e0833002 	add	r3, r3, r2
    21b0:	e5843008 	str	r3, [r4, #8]
    21b4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
    21b8:	eaffff62 	b	1f48 <fast_classifier_sync_rule+0x16c>
		    (sis->dest_new_packet_count || sis->dest_new_byte_count)) {
    21bc:	e59510a0 	ldr	r1, [r5, #160]	@ 0xa0
    21c0:	e59530a4 	ldr	r3, [r5, #164]	@ 0xa4
    21c4:	e1910003 	orrs	r0, r1, r3
    21c8:	0affff4e 	beq	1f08 <fast_classifier_sync_rule+0x12c>
	raw_spin_lock_bh(&lock->rlock);
    21cc:	e59f0080 	ldr	r0, [pc, #128]	@ 2254 <fast_classifier_sync_rule+0x478>
			nlstats.rx_packets = sis->dest_new_packet_count;
    21d0:	e50b10ec 	str	r1, [fp, #-236]	@ 0xffffff14
    21d4:	e50b20e8 	str	r2, [fp, #-232]	@ 0xffffff18
			nlstats.rx_bytes = sis->dest_new_byte_count;
    21d8:	e50b30dc 	str	r3, [fp, #-220]	@ 0xffffff24
    21dc:	e50b20d8 	str	r2, [fp, #-216]	@ 0xffffff28
    21e0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
			br_dev_update_stats(sis->dest_dev, &nlstats);
    21e4:	e5950004 	ldr	r0, [r5, #4]
    21e8:	e24b10ec 	sub	r1, fp, #236	@ 0xec
    21ec:	ebfffffe 	bl	0 <br_dev_update_stats>
	raw_spin_unlock_bh(&lock->rlock);
    21f0:	e59f005c 	ldr	r0, [pc, #92]	@ 2254 <fast_classifier_sync_rule+0x478>
    21f4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
    21f8:	eaffff42 	b	1f08 <fast_classifier_sync_rule+0x12c>
		refcount_warn_saturate(r, REFCOUNT_SUB_UAF);
    21fc:	e3a01003 	mov	r1, #3
    2200:	e1a00004 	mov	r0, r4
    2204:	ebfffffe 	bl	0 <refcount_warn_saturate>
    2208:	eaffff97 	b	206c <fast_classifier_sync_rule+0x290>
		smp_acquire__after_ctrl_dep();
    220c:	f57ff05b 	dmb	ish
		nf_ct_destroy(&ct->ct_general);
    2210:	e1a00004 	mov	r0, r4
    2214:	ebfffffe 	bl	0 <nf_ct_destroy>
}
    2218:	e24bd020 	sub	sp, fp, #32
    221c:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
		return __nf_ct_ext_find(ext, id);
    2220:	e1a00006 	mov	r0, r6
    2224:	e3a01003 	mov	r1, #3
    2228:	ebfffffe 	bl	0 <__nf_ct_ext_find>
    222c:	e1a06000 	mov	r6, r0
    2230:	eaffff4f 	b	1f74 <fast_classifier_sync_rule+0x198>
	if (!test_bit(IPS_FIXED_TIMEOUT_BIT, &ct->status)) {
    2234:	e284506c 	add	r5, r4, #108	@ 0x6c
				set_bit(IPS_SEEN_REPLY_BIT, &ct->status);
    2238:	e3a00001 	mov	r0, #1
    223c:	e1a01005 	mov	r1, r5
    2240:	ebfffffe 	bl	0 <_set_bit>
				set_bit(IPS_ASSURED_BIT, &ct->status);
    2244:	e1a01005 	mov	r1, r5
    2248:	e3a00002 	mov	r0, #2
    224c:	ebfffffe 	bl	0 <_set_bit>
static inline struct net *read_pnet(const possible_net_t *pnet)
{
#ifdef CONFIG_NET_NS
	return pnet->net;
#else
	return &init_net;
    2250:	eaffff90 	b	2098 <fast_classifier_sync_rule+0x2bc>
    2254:	00000014 	.word	0x00000014

00002258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>:
static void sfe_ipv4_remove_sfe_ipv4_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c)
    2258:	e1a0c00d 	mov	ip, sp
    225c:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
    2260:	e24cb004 	sub	fp, ip, #4
	sfe_ipv4_remove_sfe_ipv4_connection_match(si, c->reply_match);
    2264:	e590302c 	ldr	r3, [r0, #44]	@ 0x2c
		cm->prev->next = cm->next;
    2268:	e593e000 	ldr	lr, [r3]
	if (cm->prev) {
    226c:	e5931004 	ldr	r1, [r3, #4]
    2270:	e3510000 	cmp	r1, #0
    2274:	0a00003f 	beq	2378 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0x120>
		cm->prev->next = cm->next;
    2278:	e3002000 	movw	r2, #0
    227c:	e581e000 	str	lr, [r1]
    2280:	e3402000 	movt	r2, #0
	if (cm->next) {
    2284:	e5931000 	ldr	r1, [r3]
    2288:	e3510000 	cmp	r1, #0
		cm->next->prev = cm->prev;
    228c:	1593c004 	ldrne	ip, [r3, #4]
    2290:	1581c004 	strne	ip, [r1, #4]
	if (cm->active) {
    2294:	e5d31018 	ldrb	r1, [r3, #24]
    2298:	e3510000 	cmp	r1, #0
    229c:	0a000008 	beq	22c4 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0x6c>
			cm->active_prev->active_next = cm->active_next;
    22a0:	e593c010 	ldr	ip, [r3, #16]
		if (likely(cm->active_prev)) {
    22a4:	e5931014 	ldr	r1, [r3, #20]
    22a8:	e3510000 	cmp	r1, #0
			cm->active_prev->active_next = cm->active_next;
    22ac:	1581c010 	strne	ip, [r1, #16]
			cm->active_next->active_prev = cm->active_prev;
    22b0:	15931014 	ldrne	r1, [r3, #20]
			si->active_head = cm->active_next;
    22b4:	0582c004 	streq	ip, [r2, #4]
		if (likely(cm->active_next)) {
    22b8:	e35c0000 	cmp	ip, #0
			si->active_tail = cm->active_prev;
    22bc:	05821008 	streq	r1, [r2, #8]
			cm->active_next->active_prev = cm->active_prev;
    22c0:	158c1014 	strne	r1, [ip, #20]
	sfe_ipv4_remove_sfe_ipv4_connection_match(si, c->original_match);
    22c4:	e5903024 	ldr	r3, [r0, #36]	@ 0x24
		cm->prev->next = cm->next;
    22c8:	e593e000 	ldr	lr, [r3]
	if (cm->prev) {
    22cc:	e5931004 	ldr	r1, [r3, #4]
    22d0:	e3510000 	cmp	r1, #0
		cm->prev->next = cm->next;
    22d4:	1581e000 	strne	lr, [r1]
	if (cm->prev) {
    22d8:	0a00004d 	beq	2414 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0x1bc>
	if (cm->next) {
    22dc:	e5931000 	ldr	r1, [r3]
    22e0:	e3510000 	cmp	r1, #0
		cm->next->prev = cm->prev;
    22e4:	1593c004 	ldrne	ip, [r3, #4]
    22e8:	1581c004 	strne	ip, [r1, #4]
	if (cm->active) {
    22ec:	e5d31018 	ldrb	r1, [r3, #24]
    22f0:	e3510000 	cmp	r1, #0
    22f4:	0a000008 	beq	231c <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0xc4>
			cm->active_prev->active_next = cm->active_next;
    22f8:	e593c010 	ldr	ip, [r3, #16]
		if (likely(cm->active_prev)) {
    22fc:	e5931014 	ldr	r1, [r3, #20]
    2300:	e3510000 	cmp	r1, #0
			cm->active_prev->active_next = cm->active_next;
    2304:	1581c010 	strne	ip, [r1, #16]
			cm->active_next->active_prev = cm->active_prev;
    2308:	15931014 	ldrne	r1, [r3, #20]
			si->active_head = cm->active_next;
    230c:	0582c004 	streq	ip, [r2, #4]
		if (likely(cm->active_next)) {
    2310:	e35c0000 	cmp	ip, #0
			si->active_tail = cm->active_prev;
    2314:	05821008 	streq	r1, [r2, #8]
			cm->active_next->active_prev = cm->active_prev;
    2318:	158c1014 	strne	r1, [ip, #20]
		c->prev->next = c->next;
    231c:	e590c000 	ldr	ip, [r0]
	if (c->prev) {
    2320:	e5903004 	ldr	r3, [r0, #4]
    2324:	e3530000 	cmp	r3, #0
		c->prev->next = c->next;
    2328:	1583c000 	strne	ip, [r3]
	if (c->prev) {
    232c:	0a000027 	beq	23d0 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0x178>
	if (c->next) {
    2330:	e5903000 	ldr	r3, [r0]
    2334:	e3530000 	cmp	r3, #0
		c->next->prev = c->prev;
    2338:	15901004 	ldrne	r1, [r0, #4]
    233c:	15831004 	strne	r1, [r3, #4]
		c->all_connections_prev->all_connections_next = c->all_connections_next;
    2340:	e5901040 	ldr	r1, [r0, #64]	@ 0x40
	if (c->all_connections_prev) {
    2344:	e5903044 	ldr	r3, [r0, #68]	@ 0x44
    2348:	e3530000 	cmp	r3, #0
		c->all_connections_prev->all_connections_next = c->all_connections_next;
    234c:	15831040 	strne	r1, [r3, #64]	@ 0x40
		c->all_connections_next->all_connections_prev = c->all_connections_prev;
    2350:	15903044 	ldrne	r3, [r0, #68]	@ 0x44
		si->all_connections_head = c->all_connections_next;
    2354:	0582100c 	streq	r1, [r2, #12]
	if (c->all_connections_next) {
    2358:	e3510000 	cmp	r1, #0
		si->all_connections_tail = c->all_connections_prev;
    235c:	05823010 	streq	r3, [r2, #16]
		c->all_connections_next->all_connections_prev = c->all_connections_prev;
    2360:	15813044 	strne	r3, [r1, #68]	@ 0x44
	si->num_connections--;
    2364:	e5923014 	ldr	r3, [r2, #20]
    2368:	e2433001 	sub	r3, r3, #1
    236c:	e5823014 	str	r3, [r2, #20]
}
    2370:	e24bd014 	sub	sp, fp, #20
    2374:	e89da830 	ldm	sp, {r4, r5, fp, sp, pc}
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    2378:	e1d312be 	ldrh	r1, [r3, #46]	@ 0x2e
    237c:	e1d3c2bc 	ldrh	ip, [r3, #44]	@ 0x2c
    2380:	e5932024 	ldr	r2, [r3, #36]	@ 0x24
    2384:	e5d34020 	ldrb	r4, [r3, #32]
    2388:	e02cc001 	eor	ip, ip, r1
    238c:	e5931028 	ldr	r1, [r3, #40]	@ 0x28
    2390:	e6bfcfbc 	rev16	ip, ip
    2394:	e6ffc07c 	uxth	ip, ip
    2398:	e0222001 	eor	r2, r2, r1
    239c:	e593101c 	ldr	r1, [r3, #28]
    23a0:	e6bf2f32 	rev	r2, r2
    23a4:	e0211004 	eor	r1, r1, r4
    23a8:	e0211002 	eor	r1, r1, r2
		si->conn_match_hash[conn_match_idx] = cm->next;
    23ac:	e3002000 	movw	r2, #0
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    23b0:	e021100c 	eor	r1, r1, ip
		si->conn_match_hash[conn_match_idx] = cm->next;
    23b4:	e3402000 	movt	r2, #0
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    23b8:	e0211621 	eor	r1, r1, r1, lsr #12
    23bc:	e7eb1051 	ubfx	r1, r1, #0, #12
		si->conn_match_hash[conn_match_idx] = cm->next;
    23c0:	e2811a01 	add	r1, r1, #4096	@ 0x1000
    23c4:	e281100c 	add	r1, r1, #12
    23c8:	e782e101 	str	lr, [r2, r1, lsl #2]
    23cc:	eaffffac 	b	2284 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0x2c>
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    23d0:	e1d032b0 	ldrh	r3, [r0, #32]
    23d4:	e1d011bc 	ldrh	r1, [r0, #28]
    23d8:	e590e00c 	ldr	lr, [r0, #12]
    23dc:	e5904014 	ldr	r4, [r0, #20]
    23e0:	e0211003 	eor	r1, r1, r3
    23e4:	e5d03008 	ldrb	r3, [r0, #8]
    23e8:	e6bf1fb1 	rev16	r1, r1
    23ec:	e02ee004 	eor	lr, lr, r4
    23f0:	e6ff1071 	uxth	r1, r1
    23f4:	e6bfef3e 	rev	lr, lr
    23f8:	e023300e 	eor	r3, r3, lr
    23fc:	e0233001 	eor	r3, r3, r1
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    2400:	e0233623 	eor	r3, r3, r3, lsr #12
    2404:	e7eb3053 	ubfx	r3, r3, #0, #12
		si->conn_hash[conn_idx] = c->next;
    2408:	e283300c 	add	r3, r3, #12
    240c:	e782c103 	str	ip, [r2, r3, lsl #2]
    2410:	eaffffc6 	b	2330 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0xd8>
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    2414:	e5934024 	ldr	r4, [r3, #36]	@ 0x24
    2418:	e5931028 	ldr	r1, [r3, #40]	@ 0x28
    241c:	e1d3c2bc 	ldrh	ip, [r3, #44]	@ 0x2c
    2420:	e5d35020 	ldrb	r5, [r3, #32]
    2424:	e0244001 	eor	r4, r4, r1
    2428:	e1d312be 	ldrh	r1, [r3, #46]	@ 0x2e
    242c:	e6bf4f34 	rev	r4, r4
    2430:	e02cc001 	eor	ip, ip, r1
    2434:	e593101c 	ldr	r1, [r3, #28]
    2438:	e6bfcfbc 	rev16	ip, ip
    243c:	e6ffc07c 	uxth	ip, ip
    2440:	e0211005 	eor	r1, r1, r5
    2444:	e0211004 	eor	r1, r1, r4
    2448:	e021100c 	eor	r1, r1, ip
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    244c:	e0211621 	eor	r1, r1, r1, lsr #12
    2450:	e7eb1051 	ubfx	r1, r1, #0, #12
		si->conn_match_hash[conn_match_idx] = cm->next;
    2454:	e2811a01 	add	r1, r1, #4096	@ 0x1000
    2458:	e281100c 	add	r1, r1, #12
    245c:	e782e101 	str	lr, [r2, r1, lsl #2]
    2460:	eaffff9d 	b	22dc <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0+0x84>

00002464 <sfe_ipv6_remove_connection.constprop.0>:
static void sfe_ipv6_remove_connection(struct sfe_ipv6 *si, struct sfe_ipv6_connection *c)
    2464:	e1a0c00d 	mov	ip, sp
    2468:	e92dd810 	push	{r4, fp, ip, lr, pc}
    246c:	e24cb004 	sub	fp, ip, #4
	sfe_ipv6_remove_connection_match(si, c->reply_match);
    2470:	e590305c 	ldr	r3, [r0, #92]	@ 0x5c
		cm->prev->next = cm->next;
    2474:	e593c000 	ldr	ip, [r3]
	if (cm->prev) {
    2478:	e5931004 	ldr	r1, [r3, #4]
    247c:	e3510000 	cmp	r1, #0
    2480:	0a00003e 	beq	2580 <sfe_ipv6_remove_connection.constprop.0+0x11c>
		cm->prev->next = cm->next;
    2484:	e3002000 	movw	r2, #0
    2488:	e581c000 	str	ip, [r1]
    248c:	e3402000 	movt	r2, #0
	if (cm->next) {
    2490:	e5931000 	ldr	r1, [r3]
    2494:	e3510000 	cmp	r1, #0
		cm->next->prev = cm->prev;
    2498:	1593c004 	ldrne	ip, [r3, #4]
    249c:	1581c004 	strne	ip, [r1, #4]
	if (cm->active) {
    24a0:	e5d31018 	ldrb	r1, [r3, #24]
    24a4:	e3510000 	cmp	r1, #0
    24a8:	0a000008 	beq	24d0 <sfe_ipv6_remove_connection.constprop.0+0x6c>
			cm->active_prev->active_next = cm->active_next;
    24ac:	e593c010 	ldr	ip, [r3, #16]
		if (likely(cm->active_prev)) {
    24b0:	e5931014 	ldr	r1, [r3, #20]
    24b4:	e3510000 	cmp	r1, #0
			cm->active_prev->active_next = cm->active_next;
    24b8:	1581c010 	strne	ip, [r1, #16]
			cm->active_next->active_prev = cm->active_prev;
    24bc:	15931014 	ldrne	r1, [r3, #20]
			si->active_head = cm->active_next;
    24c0:	0582c004 	streq	ip, [r2, #4]
		if (likely(cm->active_next)) {
    24c4:	e35c0000 	cmp	ip, #0
			si->active_tail = cm->active_prev;
    24c8:	05821008 	streq	r1, [r2, #8]
			cm->active_next->active_prev = cm->active_prev;
    24cc:	158c1014 	strne	r1, [ip, #20]
	sfe_ipv6_remove_connection_match(si, c->original_match);
    24d0:	e5903054 	ldr	r3, [r0, #84]	@ 0x54
		cm->prev->next = cm->next;
    24d4:	e593e000 	ldr	lr, [r3]
	if (cm->prev) {
    24d8:	e5931004 	ldr	r1, [r3, #4]
    24dc:	e3510000 	cmp	r1, #0
		cm->prev->next = cm->next;
    24e0:	1581e000 	strne	lr, [r1]
	if (cm->prev) {
    24e4:	0a000062 	beq	2674 <sfe_ipv6_remove_connection.constprop.0+0x210>
	if (cm->next) {
    24e8:	e5931000 	ldr	r1, [r3]
    24ec:	e3510000 	cmp	r1, #0
		cm->next->prev = cm->prev;
    24f0:	1593c004 	ldrne	ip, [r3, #4]
    24f4:	1581c004 	strne	ip, [r1, #4]
	if (cm->active) {
    24f8:	e5d31018 	ldrb	r1, [r3, #24]
    24fc:	e3510000 	cmp	r1, #0
    2500:	0a000008 	beq	2528 <sfe_ipv6_remove_connection.constprop.0+0xc4>
			cm->active_prev->active_next = cm->active_next;
    2504:	e593c010 	ldr	ip, [r3, #16]
		if (likely(cm->active_prev)) {
    2508:	e5931014 	ldr	r1, [r3, #20]
    250c:	e3510000 	cmp	r1, #0
			cm->active_prev->active_next = cm->active_next;
    2510:	1581c010 	strne	ip, [r1, #16]
			cm->active_next->active_prev = cm->active_prev;
    2514:	15931014 	ldrne	r1, [r3, #20]
			si->active_head = cm->active_next;
    2518:	0582c004 	streq	ip, [r2, #4]
		if (likely(cm->active_next)) {
    251c:	e35c0000 	cmp	ip, #0
			si->active_tail = cm->active_prev;
    2520:	05821008 	streq	r1, [r2, #8]
			cm->active_next->active_prev = cm->active_prev;
    2524:	158c1014 	strne	r1, [ip, #20]
	if (c->prev) {
    2528:	e890000a 	ldm	r0, {r1, r3}
    252c:	e3530000 	cmp	r3, #0
		c->prev->next = c->next;
    2530:	15831000 	strne	r1, [r3]
	if (c->prev) {
    2534:	0a000032 	beq	2604 <sfe_ipv6_remove_connection.constprop.0+0x1a0>
	if (c->next) {
    2538:	e5903000 	ldr	r3, [r0]
    253c:	e3530000 	cmp	r3, #0
		c->next->prev = c->prev;
    2540:	15901004 	ldrne	r1, [r0, #4]
    2544:	15831004 	strne	r1, [r3, #4]
		c->all_connections_prev->all_connections_next = c->all_connections_next;
    2548:	e5901070 	ldr	r1, [r0, #112]	@ 0x70
	if (c->all_connections_prev) {
    254c:	e5903074 	ldr	r3, [r0, #116]	@ 0x74
    2550:	e3530000 	cmp	r3, #0
		c->all_connections_prev->all_connections_next = c->all_connections_next;
    2554:	15831070 	strne	r1, [r3, #112]	@ 0x70
		c->all_connections_next->all_connections_prev = c->all_connections_prev;
    2558:	15903074 	ldrne	r3, [r0, #116]	@ 0x74
		si->all_connections_head = c->all_connections_next;
    255c:	0582100c 	streq	r1, [r2, #12]
	if (c->all_connections_next) {
    2560:	e3510000 	cmp	r1, #0
		si->all_connections_tail = c->all_connections_prev;
    2564:	05823010 	streq	r3, [r2, #16]
		c->all_connections_next->all_connections_prev = c->all_connections_prev;
    2568:	15813074 	strne	r3, [r1, #116]	@ 0x74
	si->num_connections--;
    256c:	e5923014 	ldr	r3, [r2, #20]
    2570:	e2433001 	sub	r3, r3, #1
    2574:	e5823014 	str	r3, [r2, #20]
}
    2578:	e24bd010 	sub	sp, fp, #16
    257c:	e89da810 	ldm	sp, {r4, fp, sp, pc}
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
    2580:	e5934024 	ldr	r4, [r3, #36]	@ 0x24
    2584:	e5931028 	ldr	r1, [r3, #40]	@ 0x28
    2588:	e5932034 	ldr	r2, [r3, #52]	@ 0x34
    258c:	e1d3e4b4 	ldrh	lr, [r3, #68]	@ 0x44
    2590:	e0222004 	eor	r2, r2, r4
    2594:	e1d344b6 	ldrh	r4, [r3, #70]	@ 0x46
    2598:	e0222001 	eor	r2, r2, r1
    259c:	e5931038 	ldr	r1, [r3, #56]	@ 0x38
    25a0:	e02ee004 	eor	lr, lr, r4
    25a4:	e0222001 	eor	r2, r2, r1
    25a8:	e593102c 	ldr	r1, [r3, #44]	@ 0x2c
    25ac:	e6bfefbe 	rev16	lr, lr
    25b0:	e6ffe07e 	uxth	lr, lr
    25b4:	e0222001 	eor	r2, r2, r1
    25b8:	e593103c 	ldr	r1, [r3, #60]	@ 0x3c
    25bc:	e0222001 	eor	r2, r2, r1
    25c0:	e5931030 	ldr	r1, [r3, #48]	@ 0x30
    25c4:	e0222001 	eor	r2, r2, r1
    25c8:	e5931040 	ldr	r1, [r3, #64]	@ 0x40
    25cc:	e0222001 	eor	r2, r2, r1
    25d0:	e593101c 	ldr	r1, [r3, #28]
    25d4:	e0211002 	eor	r1, r1, r2
    25d8:	e5d32020 	ldrb	r2, [r3, #32]
    25dc:	e0211002 	eor	r1, r1, r2
		si->conn_match_hash[conn_match_idx] = cm->next;
    25e0:	e3002000 	movw	r2, #0
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
    25e4:	e021100e 	eor	r1, r1, lr
		si->conn_match_hash[conn_match_idx] = cm->next;
    25e8:	e3402000 	movt	r2, #0
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    25ec:	e0211621 	eor	r1, r1, r1, lsr #12
    25f0:	e7eb1051 	ubfx	r1, r1, #0, #12
		si->conn_match_hash[conn_match_idx] = cm->next;
    25f4:	e2811a01 	add	r1, r1, #4096	@ 0x1000
    25f8:	e281100c 	add	r1, r1, #12
    25fc:	e782c101 	str	ip, [r2, r1, lsl #2]
    2600:	eaffffa2 	b	2490 <sfe_ipv6_remove_connection.constprop.0+0x2c>
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    2604:	e590e00c 	ldr	lr, [r0, #12]
    2608:	e590c010 	ldr	ip, [r0, #16]
    260c:	e590302c 	ldr	r3, [r0, #44]	@ 0x2c
    2610:	e5904034 	ldr	r4, [r0, #52]	@ 0x34
    2614:	e023300e 	eor	r3, r3, lr
    2618:	e590e030 	ldr	lr, [r0, #48]	@ 0x30
    261c:	e023300c 	eor	r3, r3, ip
    2620:	e590c014 	ldr	ip, [r0, #20]
    2624:	e023300e 	eor	r3, r3, lr
    2628:	e590e018 	ldr	lr, [r0, #24]
    262c:	e023300c 	eor	r3, r3, ip
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    2630:	e1d0c4bc 	ldrh	ip, [r0, #76]	@ 0x4c
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    2634:	e0233004 	eor	r3, r3, r4
    2638:	e023300e 	eor	r3, r3, lr
    263c:	e590e038 	ldr	lr, [r0, #56]	@ 0x38
    2640:	e023300e 	eor	r3, r3, lr
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    2644:	e5d0e008 	ldrb	lr, [r0, #8]
    2648:	e023300e 	eor	r3, r3, lr
    264c:	e1d0e5b0 	ldrh	lr, [r0, #80]	@ 0x50
    2650:	e02cc00e 	eor	ip, ip, lr
    2654:	e6bfcfbc 	rev16	ip, ip
    2658:	e6ffc07c 	uxth	ip, ip
    265c:	e023300c 	eor	r3, r3, ip
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    2660:	e0233623 	eor	r3, r3, r3, lsr #12
    2664:	e7eb3053 	ubfx	r3, r3, #0, #12
		si->conn_hash[conn_idx] = c->next;
    2668:	e283300c 	add	r3, r3, #12
    266c:	e7821103 	str	r1, [r2, r3, lsl #2]
    2670:	eaffffb0 	b	2538 <sfe_ipv6_remove_connection.constprop.0+0xd4>
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
    2674:	e1d314b6 	ldrh	r1, [r3, #70]	@ 0x46
    2678:	e1d344b4 	ldrh	r4, [r3, #68]	@ 0x44
    267c:	e593c034 	ldr	ip, [r3, #52]	@ 0x34
    2680:	e0244001 	eor	r4, r4, r1
    2684:	e5931024 	ldr	r1, [r3, #36]	@ 0x24
    2688:	e6bf4fb4 	rev16	r4, r4
    268c:	e6ff4074 	uxth	r4, r4
    2690:	e02cc001 	eor	ip, ip, r1
    2694:	e5931028 	ldr	r1, [r3, #40]	@ 0x28
    2698:	e02cc001 	eor	ip, ip, r1
    269c:	e5931038 	ldr	r1, [r3, #56]	@ 0x38
    26a0:	e02cc001 	eor	ip, ip, r1
    26a4:	e593102c 	ldr	r1, [r3, #44]	@ 0x2c
    26a8:	e02cc001 	eor	ip, ip, r1
    26ac:	e593103c 	ldr	r1, [r3, #60]	@ 0x3c
    26b0:	e02cc001 	eor	ip, ip, r1
    26b4:	e5931030 	ldr	r1, [r3, #48]	@ 0x30
    26b8:	e02cc001 	eor	ip, ip, r1
    26bc:	e5931040 	ldr	r1, [r3, #64]	@ 0x40
    26c0:	e02cc001 	eor	ip, ip, r1
    26c4:	e593101c 	ldr	r1, [r3, #28]
    26c8:	e021100c 	eor	r1, r1, ip
    26cc:	e5d3c020 	ldrb	ip, [r3, #32]
    26d0:	e021100c 	eor	r1, r1, ip
    26d4:	e0211004 	eor	r1, r1, r4
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    26d8:	e0211621 	eor	r1, r1, r1, lsr #12
    26dc:	e7eb1051 	ubfx	r1, r1, #0, #12
		si->conn_match_hash[conn_match_idx] = cm->next;
    26e0:	e2811a01 	add	r1, r1, #4096	@ 0x1000
    26e4:	e281100c 	add	r1, r1, #12
    26e8:	e782e101 	str	lr, [r2, r1, lsl #2]
    26ec:	eaffff7d 	b	24e8 <sfe_ipv6_remove_connection.constprop.0+0x84>

000026f0 <sfe_ipv6_debug_dev_read_end>:
{
    26f0:	e1a0c00d 	mov	ip, sp
    26f4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    26f8:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "</sfe_ipv6>\n");
    26fc:	e300c000 	movw	ip, #0
{
    2700:	e1a04003 	mov	r4, r3
    2704:	e99b0060 	ldmib	fp, {r5, r6}
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "</sfe_ipv6>\n");
    2708:	e340c000 	movt	ip, #0
    270c:	e59c3008 	ldr	r3, [ip, #8]
    2710:	e1cc80d0 	ldrd	r8, [ip]
    2714:	e5823008 	str	r3, [r2, #8]
    2718:	e5dc300c 	ldrb	r3, [ip, #12]
    271c:	e5828000 	str	r8, [r2]
    2720:	e5829004 	str	r9, [r2, #4]
    2724:	e5c2300c 	strb	r3, [r2, #12]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2728:	e5953000 	ldr	r3, [r5]
    272c:	e0810003 	add	r0, r1, r3
    2730:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2734:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2738:	e1500003 	cmp	r0, r3
    273c:	9a000002 	bls	274c <sfe_ipv6_debug_dev_read_end+0x5c>
		return false;
    2740:	e3a00000 	mov	r0, #0
}
    2744:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2748:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    274c:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2750:	ee137f10 	mrc	15, 0, r7, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2754:	e3c7300c 	bic	r3, r7, #12
    2758:	e3833004 	orr	r3, r3, #4
	asm volatile(
    275c:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2760:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2764:	e1a01002 	mov	r1, r2
    2768:	e3a02c03 	mov	r2, #768	@ 0x300
    276c:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2770:	ee037f10 	mcr	15, 0, r7, cr3, cr0, {0}
	isb();
    2774:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2778:	e3500000 	cmp	r0, #0
    277c:	1affffef 	bne	2740 <sfe_ipv6_debug_dev_read_end+0x50>
	*length -= bytes_read;
    2780:	e5943000 	ldr	r3, [r4]
	return true;
    2784:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2788:	e243300c 	sub	r3, r3, #12
    278c:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2790:	e5953000 	ldr	r3, [r5]
    2794:	e283300c 	add	r3, r3, #12
    2798:	e5853000 	str	r3, [r5]
	ws->state++;
    279c:	e5963000 	ldr	r3, [r6]
    27a0:	e0833000 	add	r3, r3, r0
    27a4:	e5863000 	str	r3, [r6]
}
    27a8:	e24bd024 	sub	sp, fp, #36	@ 0x24
    27ac:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

000027b0 <sfe_ipv4_debug_dev_read_connections_end>:
{
    27b0:	e1a0c00d 	mov	ip, sp
    27b4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    27b8:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</connections>\n");
    27bc:	e300c000 	movw	ip, #0
{
    27c0:	e99b0060 	ldmib	fp, {r5, r6}
    27c4:	e1a04003 	mov	r4, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</connections>\n");
    27c8:	e340c000 	movt	ip, #0
    27cc:	e1cc80d0 	ldrd	r8, [ip]
    27d0:	e5dc3010 	ldrb	r3, [ip, #16]
    27d4:	e5828000 	str	r8, [r2]
    27d8:	e5829004 	str	r9, [r2, #4]
    27dc:	e1cc80d8 	ldrd	r8, [ip, #8]
    27e0:	e5828008 	str	r8, [r2, #8]
    27e4:	e582900c 	str	r9, [r2, #12]
    27e8:	e5c23010 	strb	r3, [r2, #16]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    27ec:	e5950000 	ldr	r0, [r5]
    27f0:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    27f4:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    27f8:	e0810000 	add	r0, r1, r0
    27fc:	e1500003 	cmp	r0, r3
    2800:	9a000002 	bls	2810 <sfe_ipv4_debug_dev_read_connections_end+0x60>
		return false;
    2804:	e3a00000 	mov	r0, #0
}
    2808:	e24bd024 	sub	sp, fp, #36	@ 0x24
    280c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2810:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2814:	ee138f10 	mrc	15, 0, r8, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2818:	e3c8300c 	bic	r3, r8, #12
    281c:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2820:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2824:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2828:	e1a01002 	mov	r1, r2
    282c:	e3a02c03 	mov	r2, #768	@ 0x300
    2830:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2834:	ee038f10 	mcr	15, 0, r8, cr3, cr0, {0}
	isb();
    2838:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    283c:	e3500000 	cmp	r0, #0
    2840:	1affffef 	bne	2804 <sfe_ipv4_debug_dev_read_connections_end+0x54>
	*length -= bytes_read;
    2844:	e5943000 	ldr	r3, [r4]
	return true;
    2848:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    284c:	e2433010 	sub	r3, r3, #16
    2850:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2854:	e5953000 	ldr	r3, [r5]
    2858:	e2833010 	add	r3, r3, #16
    285c:	e5853000 	str	r3, [r5]
	ws->state++;
    2860:	e5963000 	ldr	r3, [r6]
    2864:	e0833000 	add	r3, r3, r0
    2868:	e5863000 	str	r3, [r6]
}
    286c:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2870:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002874 <sfe_ipv6_debug_dev_read_exceptions_end>:
{
    2874:	e1a0c00d 	mov	ip, sp
    2878:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    287c:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</exceptions>\n");
    2880:	e300c000 	movw	ip, #0
{
    2884:	e99b0060 	ldmib	fp, {r5, r6}
    2888:	e1a04003 	mov	r4, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</exceptions>\n");
    288c:	e340c000 	movt	ip, #0
    2890:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2894:	e1cc80d0 	ldrd	r8, [ip]
    2898:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    289c:	e5828000 	str	r8, [r2]
    28a0:	e5829004 	str	r9, [r2, #4]
    28a4:	e1cc80d8 	ldrd	r8, [ip, #8]
    28a8:	e5828008 	str	r8, [r2, #8]
    28ac:	e582900c 	str	r9, [r2, #12]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    28b0:	e5950000 	ldr	r0, [r5]
    28b4:	e0810000 	add	r0, r1, r0
    28b8:	e1500003 	cmp	r0, r3
    28bc:	9a000002 	bls	28cc <sfe_ipv6_debug_dev_read_exceptions_end+0x58>
		return false;
    28c0:	e3a00000 	mov	r0, #0
}
    28c4:	e24bd024 	sub	sp, fp, #36	@ 0x24
    28c8:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    28cc:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    28d0:	ee138f10 	mrc	15, 0, r8, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    28d4:	e3c8300c 	bic	r3, r8, #12
    28d8:	e3833004 	orr	r3, r3, #4
	asm volatile(
    28dc:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    28e0:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    28e4:	e1a01002 	mov	r1, r2
    28e8:	e3a02c03 	mov	r2, #768	@ 0x300
    28ec:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    28f0:	ee038f10 	mcr	15, 0, r8, cr3, cr0, {0}
	isb();
    28f4:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    28f8:	e3500000 	cmp	r0, #0
    28fc:	1affffef 	bne	28c0 <sfe_ipv6_debug_dev_read_exceptions_end+0x4c>
	*length -= bytes_read;
    2900:	e5943000 	ldr	r3, [r4]
	return true;
    2904:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2908:	e243300f 	sub	r3, r3, #15
    290c:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2910:	e5953000 	ldr	r3, [r5]
    2914:	e283300f 	add	r3, r3, #15
    2918:	e5853000 	str	r3, [r5]
	ws->state++;
    291c:	e5963000 	ldr	r3, [r6]
    2920:	e0833000 	add	r3, r3, r0
    2924:	e5863000 	str	r3, [r6]
}
    2928:	e24bd024 	sub	sp, fp, #36	@ 0x24
    292c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002930 <sfe_ipv6_debug_dev_read_connections_end>:
{
    2930:	e1a0c00d 	mov	ip, sp
    2934:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2938:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</connections>\n");
    293c:	e300c000 	movw	ip, #0
{
    2940:	e99b0060 	ldmib	fp, {r5, r6}
    2944:	e1a04003 	mov	r4, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</connections>\n");
    2948:	e340c000 	movt	ip, #0
    294c:	e1cc80d0 	ldrd	r8, [ip]
    2950:	e5dc3010 	ldrb	r3, [ip, #16]
    2954:	e5828000 	str	r8, [r2]
    2958:	e5829004 	str	r9, [r2, #4]
    295c:	e1cc80d8 	ldrd	r8, [ip, #8]
    2960:	e5828008 	str	r8, [r2, #8]
    2964:	e582900c 	str	r9, [r2, #12]
    2968:	e5c23010 	strb	r3, [r2, #16]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    296c:	e5950000 	ldr	r0, [r5]
    2970:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2974:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2978:	e0810000 	add	r0, r1, r0
    297c:	e1500003 	cmp	r0, r3
    2980:	9a000002 	bls	2990 <sfe_ipv6_debug_dev_read_connections_end+0x60>
		return false;
    2984:	e3a00000 	mov	r0, #0
}
    2988:	e24bd024 	sub	sp, fp, #36	@ 0x24
    298c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2990:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2994:	ee138f10 	mrc	15, 0, r8, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2998:	e3c8300c 	bic	r3, r8, #12
    299c:	e3833004 	orr	r3, r3, #4
	asm volatile(
    29a0:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    29a4:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    29a8:	e1a01002 	mov	r1, r2
    29ac:	e3a02c03 	mov	r2, #768	@ 0x300
    29b0:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    29b4:	ee038f10 	mcr	15, 0, r8, cr3, cr0, {0}
	isb();
    29b8:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    29bc:	e3500000 	cmp	r0, #0
    29c0:	1affffef 	bne	2984 <sfe_ipv6_debug_dev_read_connections_end+0x54>
	*length -= bytes_read;
    29c4:	e5943000 	ldr	r3, [r4]
	return true;
    29c8:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    29cc:	e2433010 	sub	r3, r3, #16
    29d0:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    29d4:	e5953000 	ldr	r3, [r5]
    29d8:	e2833010 	add	r3, r3, #16
    29dc:	e5853000 	str	r3, [r5]
	ws->state++;
    29e0:	e5963000 	ldr	r3, [r6]
    29e4:	e0833000 	add	r3, r3, r0
    29e8:	e5863000 	str	r3, [r6]
}
    29ec:	e24bd024 	sub	sp, fp, #36	@ 0x24
    29f0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

000029f4 <sfe_ipv4_debug_dev_read_end>:
{
    29f4:	e1a0c00d 	mov	ip, sp
    29f8:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    29fc:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "</sfe_ipv4>\n");
    2a00:	e300c000 	movw	ip, #0
{
    2a04:	e1a04003 	mov	r4, r3
    2a08:	e99b0060 	ldmib	fp, {r5, r6}
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "</sfe_ipv4>\n");
    2a0c:	e340c000 	movt	ip, #0
    2a10:	e59c3008 	ldr	r3, [ip, #8]
    2a14:	e1cc80d0 	ldrd	r8, [ip]
    2a18:	e5823008 	str	r3, [r2, #8]
    2a1c:	e5dc300c 	ldrb	r3, [ip, #12]
    2a20:	e5828000 	str	r8, [r2]
    2a24:	e5829004 	str	r9, [r2, #4]
    2a28:	e5c2300c 	strb	r3, [r2, #12]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2a2c:	e5953000 	ldr	r3, [r5]
    2a30:	e0810003 	add	r0, r1, r3
    2a34:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2a38:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2a3c:	e1500003 	cmp	r0, r3
    2a40:	9a000002 	bls	2a50 <sfe_ipv4_debug_dev_read_end+0x5c>
		return false;
    2a44:	e3a00000 	mov	r0, #0
}
    2a48:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2a4c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2a50:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2a54:	ee137f10 	mrc	15, 0, r7, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2a58:	e3c7300c 	bic	r3, r7, #12
    2a5c:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2a60:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2a64:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2a68:	e1a01002 	mov	r1, r2
    2a6c:	e3a02c03 	mov	r2, #768	@ 0x300
    2a70:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2a74:	ee037f10 	mcr	15, 0, r7, cr3, cr0, {0}
	isb();
    2a78:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2a7c:	e3500000 	cmp	r0, #0
    2a80:	1affffef 	bne	2a44 <sfe_ipv4_debug_dev_read_end+0x50>
	*length -= bytes_read;
    2a84:	e5943000 	ldr	r3, [r4]
	return true;
    2a88:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2a8c:	e243300c 	sub	r3, r3, #12
    2a90:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2a94:	e5953000 	ldr	r3, [r5]
    2a98:	e283300c 	add	r3, r3, #12
    2a9c:	e5853000 	str	r3, [r5]
	ws->state++;
    2aa0:	e5963000 	ldr	r3, [r6]
    2aa4:	e0833000 	add	r3, r3, r0
    2aa8:	e5863000 	str	r3, [r6]
}
    2aac:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2ab0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002ab4 <sfe_ipv4_debug_dev_read_exceptions_start>:
{
    2ab4:	e1a0c00d 	mov	ip, sp
    2ab8:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2abc:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<exceptions>\n");
    2ac0:	e300c000 	movw	ip, #0
{
    2ac4:	e1a04003 	mov	r4, r3
    2ac8:	e99b0060 	ldmib	fp, {r5, r6}
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<exceptions>\n");
    2acc:	e340c000 	movt	ip, #0
    2ad0:	e59c3008 	ldr	r3, [ip, #8]
    2ad4:	e1cc80d0 	ldrd	r8, [ip]
    2ad8:	e5823008 	str	r3, [r2, #8]
    2adc:	e1dc30bc 	ldrh	r3, [ip, #12]
    2ae0:	e5828000 	str	r8, [r2]
    2ae4:	e5829004 	str	r9, [r2, #4]
    2ae8:	e1c230bc 	strh	r3, [r2, #12]
    2aec:	e5dc300e 	ldrb	r3, [ip, #14]
    2af0:	e5c2300e 	strb	r3, [r2, #14]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2af4:	e5953000 	ldr	r3, [r5]
    2af8:	e0810003 	add	r0, r1, r3
    2afc:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2b00:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2b04:	e1500003 	cmp	r0, r3
    2b08:	9a000002 	bls	2b18 <sfe_ipv4_debug_dev_read_exceptions_start+0x64>
		return false;
    2b0c:	e3a00000 	mov	r0, #0
}
    2b10:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2b14:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2b18:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2b1c:	ee137f10 	mrc	15, 0, r7, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2b20:	e3c7300c 	bic	r3, r7, #12
    2b24:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2b28:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2b2c:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2b30:	e1a01002 	mov	r1, r2
    2b34:	e3a02c03 	mov	r2, #768	@ 0x300
    2b38:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2b3c:	ee037f10 	mcr	15, 0, r7, cr3, cr0, {0}
	isb();
    2b40:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2b44:	e3500000 	cmp	r0, #0
    2b48:	1affffef 	bne	2b0c <sfe_ipv4_debug_dev_read_exceptions_start+0x58>
	*length -= bytes_read;
    2b4c:	e5943000 	ldr	r3, [r4]
	return true;
    2b50:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2b54:	e243300e 	sub	r3, r3, #14
    2b58:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2b5c:	e5953000 	ldr	r3, [r5]
    2b60:	e283300e 	add	r3, r3, #14
    2b64:	e5853000 	str	r3, [r5]
	ws->state++;
    2b68:	e5963000 	ldr	r3, [r6]
    2b6c:	e0833000 	add	r3, r3, r0
    2b70:	e5863000 	str	r3, [r6]
}
    2b74:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2b78:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002b7c <sfe_ipv4_debug_dev_read_connections_start>:
{
    2b7c:	e1a0c00d 	mov	ip, sp
    2b80:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2b84:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<connections>\n");
    2b88:	e300c000 	movw	ip, #0
{
    2b8c:	e99b0060 	ldmib	fp, {r5, r6}
    2b90:	e1a04003 	mov	r4, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<connections>\n");
    2b94:	e340c000 	movt	ip, #0
    2b98:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2b9c:	e1cc80d0 	ldrd	r8, [ip]
    2ba0:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2ba4:	e5828000 	str	r8, [r2]
    2ba8:	e5829004 	str	r9, [r2, #4]
    2bac:	e1cc80d8 	ldrd	r8, [ip, #8]
    2bb0:	e5828008 	str	r8, [r2, #8]
    2bb4:	e582900c 	str	r9, [r2, #12]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2bb8:	e5950000 	ldr	r0, [r5]
    2bbc:	e0810000 	add	r0, r1, r0
    2bc0:	e1500003 	cmp	r0, r3
    2bc4:	9a000002 	bls	2bd4 <sfe_ipv4_debug_dev_read_connections_start+0x58>
		return false;
    2bc8:	e3a00000 	mov	r0, #0
}
    2bcc:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2bd0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2bd4:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2bd8:	ee138f10 	mrc	15, 0, r8, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2bdc:	e3c8300c 	bic	r3, r8, #12
    2be0:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2be4:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2be8:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2bec:	e1a01002 	mov	r1, r2
    2bf0:	e3a02c03 	mov	r2, #768	@ 0x300
    2bf4:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2bf8:	ee038f10 	mcr	15, 0, r8, cr3, cr0, {0}
	isb();
    2bfc:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2c00:	e3500000 	cmp	r0, #0
    2c04:	1affffef 	bne	2bc8 <sfe_ipv4_debug_dev_read_connections_start+0x4c>
	*length -= bytes_read;
    2c08:	e5943000 	ldr	r3, [r4]
	return true;
    2c0c:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2c10:	e243300f 	sub	r3, r3, #15
    2c14:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2c18:	e5953000 	ldr	r3, [r5]
    2c1c:	e283300f 	add	r3, r3, #15
    2c20:	e5853000 	str	r3, [r5]
	ws->state++;
    2c24:	e5963000 	ldr	r3, [r6]
    2c28:	e0833000 	add	r3, r3, r0
    2c2c:	e5863000 	str	r3, [r6]
}
    2c30:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2c34:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002c38 <sfe_ipv6_debug_dev_read_exceptions_start>:
{
    2c38:	e1a0c00d 	mov	ip, sp
    2c3c:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2c40:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<exceptions>\n");
    2c44:	e300c000 	movw	ip, #0
{
    2c48:	e1a04003 	mov	r4, r3
    2c4c:	e99b0060 	ldmib	fp, {r5, r6}
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<exceptions>\n");
    2c50:	e340c000 	movt	ip, #0
    2c54:	e59c3008 	ldr	r3, [ip, #8]
    2c58:	e1cc80d0 	ldrd	r8, [ip]
    2c5c:	e5823008 	str	r3, [r2, #8]
    2c60:	e1dc30bc 	ldrh	r3, [ip, #12]
    2c64:	e5828000 	str	r8, [r2]
    2c68:	e5829004 	str	r9, [r2, #4]
    2c6c:	e1c230bc 	strh	r3, [r2, #12]
    2c70:	e5dc300e 	ldrb	r3, [ip, #14]
    2c74:	e5c2300e 	strb	r3, [r2, #14]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2c78:	e5953000 	ldr	r3, [r5]
    2c7c:	e0810003 	add	r0, r1, r3
    2c80:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2c84:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2c88:	e1500003 	cmp	r0, r3
    2c8c:	9a000002 	bls	2c9c <sfe_ipv6_debug_dev_read_exceptions_start+0x64>
		return false;
    2c90:	e3a00000 	mov	r0, #0
}
    2c94:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2c98:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2c9c:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2ca0:	ee137f10 	mrc	15, 0, r7, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2ca4:	e3c7300c 	bic	r3, r7, #12
    2ca8:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2cac:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2cb0:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2cb4:	e1a01002 	mov	r1, r2
    2cb8:	e3a02c03 	mov	r2, #768	@ 0x300
    2cbc:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2cc0:	ee037f10 	mcr	15, 0, r7, cr3, cr0, {0}
	isb();
    2cc4:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2cc8:	e3500000 	cmp	r0, #0
    2ccc:	1affffef 	bne	2c90 <sfe_ipv6_debug_dev_read_exceptions_start+0x58>
	*length -= bytes_read;
    2cd0:	e5943000 	ldr	r3, [r4]
	return true;
    2cd4:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2cd8:	e243300e 	sub	r3, r3, #14
    2cdc:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2ce0:	e5953000 	ldr	r3, [r5]
    2ce4:	e283300e 	add	r3, r3, #14
    2ce8:	e5853000 	str	r3, [r5]
	ws->state++;
    2cec:	e5963000 	ldr	r3, [r6]
    2cf0:	e0833000 	add	r3, r3, r0
    2cf4:	e5863000 	str	r3, [r6]
}
    2cf8:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2cfc:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002d00 <sfe_ipv4_debug_dev_read_exceptions_end>:
{
    2d00:	e1a0c00d 	mov	ip, sp
    2d04:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2d08:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</exceptions>\n");
    2d0c:	e300c000 	movw	ip, #0
{
    2d10:	e99b0060 	ldmib	fp, {r5, r6}
    2d14:	e1a04003 	mov	r4, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</exceptions>\n");
    2d18:	e340c000 	movt	ip, #0
    2d1c:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2d20:	e1cc80d0 	ldrd	r8, [ip]
    2d24:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2d28:	e5828000 	str	r8, [r2]
    2d2c:	e5829004 	str	r9, [r2, #4]
    2d30:	e1cc80d8 	ldrd	r8, [ip, #8]
    2d34:	e5828008 	str	r8, [r2, #8]
    2d38:	e582900c 	str	r9, [r2, #12]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2d3c:	e5950000 	ldr	r0, [r5]
    2d40:	e0810000 	add	r0, r1, r0
    2d44:	e1500003 	cmp	r0, r3
    2d48:	9a000002 	bls	2d58 <sfe_ipv4_debug_dev_read_exceptions_end+0x58>
		return false;
    2d4c:	e3a00000 	mov	r0, #0
}
    2d50:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2d54:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2d58:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2d5c:	ee138f10 	mrc	15, 0, r8, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2d60:	e3c8300c 	bic	r3, r8, #12
    2d64:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2d68:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2d6c:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2d70:	e1a01002 	mov	r1, r2
    2d74:	e3a02c03 	mov	r2, #768	@ 0x300
    2d78:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2d7c:	ee038f10 	mcr	15, 0, r8, cr3, cr0, {0}
	isb();
    2d80:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2d84:	e3500000 	cmp	r0, #0
    2d88:	1affffef 	bne	2d4c <sfe_ipv4_debug_dev_read_exceptions_end+0x4c>
	*length -= bytes_read;
    2d8c:	e5943000 	ldr	r3, [r4]
	return true;
    2d90:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2d94:	e243300f 	sub	r3, r3, #15
    2d98:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2d9c:	e5953000 	ldr	r3, [r5]
    2da0:	e283300f 	add	r3, r3, #15
    2da4:	e5853000 	str	r3, [r5]
	ws->state++;
    2da8:	e5963000 	ldr	r3, [r6]
    2dac:	e0833000 	add	r3, r3, r0
    2db0:	e5863000 	str	r3, [r6]
}
    2db4:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2db8:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002dbc <sfe_ipv6_debug_dev_read_connections_start>:
{
    2dbc:	e1a0c00d 	mov	ip, sp
    2dc0:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2dc4:	e24cb004 	sub	fp, ip, #4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<connections>\n");
    2dc8:	e300c000 	movw	ip, #0
{
    2dcc:	e99b0060 	ldmib	fp, {r5, r6}
    2dd0:	e1a04003 	mov	r4, r3
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<connections>\n");
    2dd4:	e340c000 	movt	ip, #0
    2dd8:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2ddc:	e1cc80d0 	ldrd	r8, [ip]
    2de0:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2de4:	e5828000 	str	r8, [r2]
    2de8:	e5829004 	str	r9, [r2, #4]
    2dec:	e1cc80d8 	ldrd	r8, [ip, #8]
    2df0:	e5828008 	str	r8, [r2, #8]
    2df4:	e582900c 	str	r9, [r2, #12]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2df8:	e5950000 	ldr	r0, [r5]
    2dfc:	e0810000 	add	r0, r1, r0
    2e00:	e1500003 	cmp	r0, r3
    2e04:	9a000002 	bls	2e14 <sfe_ipv6_debug_dev_read_connections_start+0x58>
		return false;
    2e08:	e3a00000 	mov	r0, #0
}
    2e0c:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2e10:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2e14:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2e18:	ee138f10 	mrc	15, 0, r8, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2e1c:	e3c8300c 	bic	r3, r8, #12
    2e20:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2e24:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2e28:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2e2c:	e1a01002 	mov	r1, r2
    2e30:	e3a02c03 	mov	r2, #768	@ 0x300
    2e34:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2e38:	ee038f10 	mcr	15, 0, r8, cr3, cr0, {0}
	isb();
    2e3c:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2e40:	e3500000 	cmp	r0, #0
    2e44:	1affffef 	bne	2e08 <sfe_ipv6_debug_dev_read_connections_start+0x4c>
	*length -= bytes_read;
    2e48:	e5943000 	ldr	r3, [r4]
	return true;
    2e4c:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2e50:	e243300f 	sub	r3, r3, #15
    2e54:	e5843000 	str	r3, [r4]
	*total_read += bytes_read;
    2e58:	e5953000 	ldr	r3, [r5]
    2e5c:	e283300f 	add	r3, r3, #15
    2e60:	e5853000 	str	r3, [r5]
	ws->state++;
    2e64:	e5963000 	ldr	r3, [r6]
    2e68:	e0833000 	add	r3, r3, r0
    2e6c:	e5863000 	str	r3, [r6]
}
    2e70:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2e74:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002e78 <sfe_ipv4_debug_dev_read_start>:
{
    2e78:	e1a0c00d 	mov	ip, sp
    2e7c:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2e80:	e24cb004 	sub	fp, ip, #4
	si->debug_read_seq++;
    2e84:	e2800902 	add	r0, r0, #32768	@ 0x8000
{
    2e88:	e1a06003 	mov	r6, r3
    2e8c:	e1cb40d4 	ldrd	r4, [fp, #4]
	si->debug_read_seq++;
    2e90:	e5903260 	ldr	r3, [r0, #608]	@ 0x260
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "<sfe_ipv4>\n");
    2e94:	e300c000 	movw	ip, #0
    2e98:	e340c000 	movt	ip, #0
    2e9c:	e1cc80d0 	ldrd	r8, [ip]
	si->debug_read_seq++;
    2ea0:	e2833001 	add	r3, r3, #1
    2ea4:	e5803260 	str	r3, [r0, #608]	@ 0x260
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "<sfe_ipv4>\n");
    2ea8:	e59c0008 	ldr	r0, [ip, #8]
    2eac:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2eb0:	e5828000 	str	r8, [r2]
    2eb4:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2eb8:	e5829004 	str	r9, [r2, #4]
    2ebc:	e5820008 	str	r0, [r2, #8]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2ec0:	e5940000 	ldr	r0, [r4]
    2ec4:	e0810000 	add	r0, r1, r0
    2ec8:	e1500003 	cmp	r0, r3
    2ecc:	9a000002 	bls	2edc <sfe_ipv4_debug_dev_read_start+0x64>
		return false;
    2ed0:	e3a00000 	mov	r0, #0
}
    2ed4:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2ed8:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2edc:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2ee0:	ee137f10 	mrc	15, 0, r7, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2ee4:	e3c7300c 	bic	r3, r7, #12
    2ee8:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2eec:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2ef0:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2ef4:	e1a01002 	mov	r1, r2
    2ef8:	e3a02c03 	mov	r2, #768	@ 0x300
    2efc:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2f00:	ee037f10 	mcr	15, 0, r7, cr3, cr0, {0}
	isb();
    2f04:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2f08:	e3500000 	cmp	r0, #0
    2f0c:	1affffef 	bne	2ed0 <sfe_ipv4_debug_dev_read_start+0x58>
	*length -= bytes_read;
    2f10:	e5963000 	ldr	r3, [r6]
	return true;
    2f14:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2f18:	e243300b 	sub	r3, r3, #11
    2f1c:	e5863000 	str	r3, [r6]
	*total_read += bytes_read;
    2f20:	e5943000 	ldr	r3, [r4]
    2f24:	e283300b 	add	r3, r3, #11
    2f28:	e5843000 	str	r3, [r4]
	ws->state++;
    2f2c:	e5953000 	ldr	r3, [r5]
    2f30:	e0833000 	add	r3, r3, r0
    2f34:	e5853000 	str	r3, [r5]
}
    2f38:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2f3c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00002f40 <sfe_ipv6_debug_dev_read_start>:
{
    2f40:	e1a0c00d 	mov	ip, sp
    2f44:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    2f48:	e24cb004 	sub	fp, ip, #4
	si->debug_read_seq++;
    2f4c:	e2800902 	add	r0, r0, #32768	@ 0x8000
{
    2f50:	e1a06003 	mov	r6, r3
    2f54:	e1cb40d4 	ldrd	r4, [fp, #4]
	si->debug_read_seq++;
    2f58:	e5903270 	ldr	r3, [r0, #624]	@ 0x270
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "<sfe_ipv6>\n");
    2f5c:	e300c000 	movw	ip, #0
    2f60:	e340c000 	movt	ip, #0
    2f64:	e1cc80d0 	ldrd	r8, [ip]
	si->debug_read_seq++;
    2f68:	e2833001 	add	r3, r3, #1
    2f6c:	e5803270 	str	r3, [r0, #624]	@ 0x270
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "<sfe_ipv6>\n");
    2f70:	e59c0008 	ldr	r0, [ip, #8]
    2f74:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    2f78:	e5828000 	str	r8, [r2]
    2f7c:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    2f80:	e5829004 	str	r9, [r2, #4]
    2f84:	e5820008 	str	r0, [r2, #8]
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2f88:	e5940000 	ldr	r0, [r4]
    2f8c:	e0810000 	add	r0, r1, r0
    2f90:	e1500003 	cmp	r0, r3
    2f94:	9a000002 	bls	2fa4 <sfe_ipv6_debug_dev_read_start+0x64>
		return false;
    2f98:	e3a00000 	mov	r0, #0
}
    2f9c:	e24bd024 	sub	sp, fp, #36	@ 0x24
    2fa0:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    2fa4:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    2fa8:	ee137f10 	mrc	15, 0, r7, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    2fac:	e3c7300c 	bic	r3, r7, #12
    2fb0:	e3833004 	orr	r3, r3, #4
	asm volatile(
    2fb4:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    2fb8:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    2fbc:	e1a01002 	mov	r1, r2
    2fc0:	e3a02c03 	mov	r2, #768	@ 0x300
    2fc4:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    2fc8:	ee037f10 	mcr	15, 0, r7, cr3, cr0, {0}
	isb();
    2fcc:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    2fd0:	e3500000 	cmp	r0, #0
    2fd4:	1affffef 	bne	2f98 <sfe_ipv6_debug_dev_read_start+0x58>
	*length -= bytes_read;
    2fd8:	e5963000 	ldr	r3, [r6]
	return true;
    2fdc:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    2fe0:	e243300b 	sub	r3, r3, #11
    2fe4:	e5863000 	str	r3, [r6]
	*total_read += bytes_read;
    2fe8:	e5943000 	ldr	r3, [r4]
    2fec:	e283300b 	add	r3, r3, #11
    2ff0:	e5843000 	str	r3, [r4]
	ws->state++;
    2ff4:	e5953000 	ldr	r3, [r5]
    2ff8:	e0833000 	add	r3, r3, r0
    2ffc:	e5853000 	str	r3, [r5]
}
    3000:	e24bd024 	sub	sp, fp, #36	@ 0x24
    3004:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00003008 <fast_classifier_find_dev_and_mac_addr>:
{
    3008:	e1a0c00d 	mov	ip, sp
    300c:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    3010:	e24cb004 	sub	fp, ip, #4
    3014:	e24dd040 	sub	sp, sp, #64	@ 0x40
	if (unlikely(skb)) {
    3018:	e2505000 	subs	r5, r0, #0
{
    301c:	e1a04001 	mov	r4, r1
    3020:	e5db0004 	ldrb	r0, [fp, #4]
    3024:	e1a07002 	mov	r7, r2
    3028:	e1a08003 	mov	r8, r3
	if (unlikely(skb)) {
    302c:	1a00005e 	bne	31ac <fast_classifier_find_dev_and_mac_addr+0x1a4>
	if (likely(is_v4)) {
    3030:	e3500000 	cmp	r0, #0
    3034:	0a000060 	beq	31bc <fast_classifier_find_dev_and_mac_addr+0x1b4>
		rt = ip_route_output(&init_net, addr->ip, 0, 0, 0);
    3038:	e5946000 	ldr	r6, [r4]
}

static inline struct rtable *ip_route_output(struct net *net, __be32 daddr,
					     __be32 saddr, u8 tos, int oif)
{
	struct flowi4 fl4 = {
    303c:	e3a02034 	mov	r2, #52	@ 0x34
    3040:	e1a01005 	mov	r1, r5
    3044:	e24b0058 	sub	r0, fp, #88	@ 0x58
    3048:	ebfffffe 	bl	0 <memset>
	return ip_route_output_flow(net, flp, NULL);
    304c:	e3000000 	movw	r0, #0
    3050:	e1a02005 	mov	r2, r5
	struct flowi4 fl4 = {
    3054:	e50b505c 	str	r5, [fp, #-92]	@ 0xffffffa4
	return ip_route_output_flow(net, flp, NULL);
    3058:	e3400000 	movt	r0, #0
    305c:	e24b105c 	sub	r1, fp, #92	@ 0x5c
	struct flowi4 fl4 = {
    3060:	e50b6030 	str	r6, [fp, #-48]	@ 0xffffffd0
	return ip_route_output_flow(net, flp, NULL);
    3064:	ebfffffe 	bl	0 <ip_route_output_flow>
		if (unlikely(IS_ERR(rt))) {
    3068:	e3700a01 	cmn	r0, #4096	@ 0x1000
    306c:	e1a06000 	mov	r6, r0
    3070:	8a00004b 	bhi	31a4 <fast_classifier_find_dev_and_mac_addr+0x19c>
{
}

static inline struct neighbour *dst_neigh_lookup(const struct dst_entry *dst, const void *daddr)
{
	struct neighbour *n = dst->ops->neigh_lookup(dst, NULL, daddr);
    3074:	e5963004 	ldr	r3, [r6, #4]
    3078:	e1a02004 	mov	r2, r4
    307c:	e3a01000 	mov	r1, #0
    3080:	e1a00006 	mov	r0, r6
    3084:	e5933038 	ldr	r3, [r3, #56]	@ 0x38
    3088:	e12fff33 	blx	r3
	return IS_ERR(n) ? NULL : n;
    308c:	e3700a01 	cmn	r0, #4096	@ 0x1000
	if (unlikely(!neigh)) {
    3090:	e1a04000 	mov	r4, r0
    3094:	93a03000 	movls	r3, #0
    3098:	83a03001 	movhi	r3, #1
    309c:	e3500000 	cmp	r0, #0
    30a0:	03833001 	orreq	r3, r3, #1
    30a4:	e3530000 	cmp	r3, #0
    30a8:	1a000042 	bne	31b8 <fast_classifier_find_dev_and_mac_addr+0x1b0>
	if (unlikely(!(neigh->nud_state & NUD_VALID))) {
    30ac:	e5d0304c 	ldrb	r3, [r0, #76]	@ 0x4c
    30b0:	e31300de 	tst	r3, #222	@ 0xde
    30b4:	0a00002a 	beq	3164 <fast_classifier_find_dev_and_mac_addr+0x15c>
	mac_dev = neigh->dev;
    30b8:	e590912c 	ldr	r9, [r0, #300]	@ 0x12c
	if (!mac_dev) {
    30bc:	e3590000 	cmp	r9, #0
    30c0:	0a000027 	beq	3164 <fast_classifier_find_dev_and_mac_addr+0x15c>
	memcpy(mac_addr, neigh->ha, (size_t)mac_dev->addr_len);
    30c4:	e5d92181 	ldrb	r2, [r9, #385]	@ 0x181
    30c8:	e1a00008 	mov	r0, r8
    30cc:	e2841060 	add	r1, r4, #96	@ 0x60
    30d0:	ebfffffe 	bl	0 <memcpy>
	asm volatile(
    30d4:	e10f0000 	mrs	r0, CPSR
    30d8:	f10c0080 	cpsid	i
		this_cpu_inc(*dev->pcpu_refcnt);
    30dc:	e5993310 	ldr	r3, [r9, #784]	@ 0x310
    30e0:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    30e4:	e7932001 	ldr	r2, [r3, r1]
    30e8:	e2822001 	add	r2, r2, #1
    30ec:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    30f0:	e121f000 	msr	CPSR_c, r0
	*dev = mac_dev;
    30f4:	e5879000 	str	r9, [r7]
 *	Neighbour references
 */

static inline void neigh_release(struct neighbour *neigh)
{
	if (refcount_dec_and_test(&neigh->refcnt))
    30f8:	e2843018 	add	r3, r4, #24
    30fc:	f57ff05b 	dmb	ish
    3100:	f593f000 	pldw	[r3]
ATOMIC_OPS(sub, -=, sub)
    3104:	e1932f9f 	ldrex	r2, [r3]
    3108:	e2421001 	sub	r1, r2, #1
    310c:	e1830f91 	strex	r0, r1, [r3]
    3110:	e3300000 	teq	r0, #0
    3114:	1afffffa 	bne	3104 <fast_classifier_find_dev_and_mac_addr+0xfc>
	if (old == i) {
    3118:	e3520001 	cmp	r2, #1
    311c:	0a000008 	beq	3144 <fast_classifier_find_dev_and_mac_addr+0x13c>
	if (unlikely(old < 0 || old - i < 0))
    3120:	e3520000 	cmp	r2, #0
    3124:	da00000a 	ble	3154 <fast_classifier_find_dev_and_mac_addr+0x14c>
	if (likely(!skb)) {
    3128:	e3550000 	cmp	r5, #0
    312c:	1a000001 	bne	3138 <fast_classifier_find_dev_and_mac_addr+0x130>
		dst_release(dst);
    3130:	e1a00006 	mov	r0, r6
    3134:	ebfffffe 	bl	0 <dst_release>
	return true;
    3138:	e3a00001 	mov	r0, #1
}
    313c:	e24bd024 	sub	sp, fp, #36	@ 0x24
    3140:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		smp_acquire__after_ctrl_dep();
    3144:	f57ff05b 	dmb	ish
		neigh_destroy(neigh);
    3148:	e1a00004 	mov	r0, r4
    314c:	ebfffffe 	bl	0 <neigh_destroy>
    3150:	eafffff4 	b	3128 <fast_classifier_find_dev_and_mac_addr+0x120>
		refcount_warn_saturate(r, REFCOUNT_SUB_UAF);
    3154:	e3a01003 	mov	r1, #3
    3158:	e1a00003 	mov	r0, r3
    315c:	ebfffffe 	bl	0 <refcount_warn_saturate>
    3160:	eafffff0 	b	3128 <fast_classifier_find_dev_and_mac_addr+0x120>
	if (refcount_dec_and_test(&neigh->refcnt))
    3164:	e2844018 	add	r4, r4, #24
    3168:	f57ff05b 	dmb	ish
    316c:	f594f000 	pldw	[r4]
    3170:	e1943f9f 	ldrex	r3, [r4]
    3174:	e2432001 	sub	r2, r3, #1
    3178:	e1841f92 	strex	r1, r2, [r4]
    317c:	e3310000 	teq	r1, #0
    3180:	1afffffa 	bne	3170 <fast_classifier_find_dev_and_mac_addr+0x168>
	if (old == i) {
    3184:	e3530001 	cmp	r3, #1
    3188:	0a000015 	beq	31e4 <fast_classifier_find_dev_and_mac_addr+0x1dc>
	if (unlikely(old < 0 || old - i < 0))
    318c:	e3530000 	cmp	r3, #0
    3190:	da000016 	ble	31f0 <fast_classifier_find_dev_and_mac_addr+0x1e8>
		if (likely(!skb)) {
    3194:	e3550000 	cmp	r5, #0
    3198:	1a000001 	bne	31a4 <fast_classifier_find_dev_and_mac_addr+0x19c>
			dst_release(dst);
    319c:	e1a00006 	mov	r0, r6
    31a0:	ebfffffe 	bl	0 <dst_release>
	return false;
    31a4:	e3a00000 	mov	r0, #0
    31a8:	eaffffe3 	b	313c <fast_classifier_find_dev_and_mac_addr+0x134>
	return (struct dst_entry *)(skb->_skb_refdst & SKB_DST_PTRMASK);
    31ac:	e595604c 	ldr	r6, [r5, #76]	@ 0x4c
    31b0:	e3c66001 	bic	r6, r6, #1
		goto skip_dst_lookup;
    31b4:	eaffffae 	b	3074 <fast_classifier_find_dev_and_mac_addr+0x6c>
	preempt_disable();
}

static inline void __rcu_read_unlock(void)
{
	preempt_enable();
    31b8:	eafffff5 	b	3194 <fast_classifier_find_dev_and_mac_addr+0x18c>
		rt6 = rt6_lookup(&init_net, (struct in6_addr *)addr->ip6, NULL, 0,NULL, 0);
    31bc:	e3000000 	movw	r0, #0
    31c0:	e1a03005 	mov	r3, r5
    31c4:	e58d5000 	str	r5, [sp]
    31c8:	e3400000 	movt	r0, #0
    31cc:	e1a02005 	mov	r2, r5
    31d0:	e58d5004 	str	r5, [sp, #4]
    31d4:	ebfffffe 	bl	0 <rt6_lookup>
		if (!rt6) {
    31d8:	e2506000 	subs	r6, r0, #0
    31dc:	1affffa4 	bne	3074 <fast_classifier_find_dev_and_mac_addr+0x6c>
    31e0:	eaffffef 	b	31a4 <fast_classifier_find_dev_and_mac_addr+0x19c>
		smp_acquire__after_ctrl_dep();
    31e4:	f57ff05b 	dmb	ish
		neigh_destroy(neigh);
    31e8:	ebfffffe 	bl	0 <neigh_destroy>
    31ec:	eaffffe8 	b	3194 <fast_classifier_find_dev_and_mac_addr+0x18c>
		refcount_warn_saturate(r, REFCOUNT_SUB_UAF);
    31f0:	e3a01003 	mov	r1, #3
    31f4:	e1a00004 	mov	r0, r4
    31f8:	ebfffffe 	bl	0 <refcount_warn_saturate>
		if (likely(!skb)) {
    31fc:	eaffffe4 	b	3194 <fast_classifier_find_dev_and_mac_addr+0x18c>

00003200 <sfe_ipv6_debug_dev_write>:
{
    3200:	e1a0c00d 	mov	ip, sp
    3204:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
    3208:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock_bh(&lock->rlock);
    320c:	e3005000 	movw	r5, #0
    3210:	e1a04002 	mov	r4, r2
    3214:	e3405000 	movt	r5, #0
    3218:	e1a00005 	mov	r0, r5
    321c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_create_requests = 0;
    3220:	e59f10b8 	ldr	r1, [pc, #184]	@ 32e0 <sfe_ipv6_debug_dev_write+0xe0>
    3224:	e3a03000 	mov	r3, #0
    3228:	e3a02000 	mov	r2, #0
    322c:	e3080030 	movw	r0, #32816	@ 0x8030
    3230:	e18520f0 	strd	r2, [r5, r0]
    3234:	e3080040 	movw	r0, #32832	@ 0x8040
    3238:	e281e010 	add	lr, r1, #16
    323c:	e14120f8 	strd	r2, [r1, #-8]
	si->packets_not_forwarded = 0;
    3240:	e2851902 	add	r1, r5, #32768	@ 0x8000
    3244:	e2817e13 	add	r7, r1, #304	@ 0x130
    3248:	e2816e26 	add	r6, r1, #608	@ 0x260
	si->connection_create_requests = 0;
    324c:	e14e20f8 	strd	r2, [lr, #-8]
    3250:	e18520f0 	strd	r2, [r5, r0]
	si->packets_not_forwarded = 0;
    3254:	e3a00000 	mov	r0, #0
    3258:	e1a03007 	mov	r3, r7
    325c:	e5810050 	str	r0, [r1, #80]	@ 0x50
		si->exception_events64[i] += si->exception_events[i];
    3260:	e5b31008 	ldr	r1, [r3, #8]!
    3264:	e5be2004 	ldr	r2, [lr, #4]!
    3268:	e593c004 	ldr	ip, [r3, #4]
    326c:	e0911002 	adds	r1, r1, r2
    3270:	e5831000 	str	r1, [r3]
    3274:	e2ac1000 	adc	r1, ip, #0
	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
    3278:	e1560003 	cmp	r6, r3
		si->exception_events64[i] += si->exception_events[i];
    327c:	e5831004 	str	r1, [r3, #4]
		si->exception_events[i] = 0;
    3280:	e58e0000 	str	r0, [lr]
	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
    3284:	1afffff5 	bne	3260 <sfe_ipv6_debug_dev_write+0x60>
	si->packets_forwarded64 = 0;
    3288:	e3a02000 	mov	r2, #0
    328c:	e3a03000 	mov	r3, #0
	si->connection_create_collisions64 = 0;
    3290:	e59f004c 	ldr	r0, [pc, #76]	@ 32e4 <sfe_ipv6_debug_dev_write+0xe4>
	si->packets_forwarded64 = 0;
    3294:	e3081130 	movw	r1, #33072	@ 0x8130
    3298:	e14720f8 	strd	r2, [r7, #-8]
	si->packets_not_forwarded64 = 0;
    329c:	e18520f1 	strd	r2, [r5, r1]
	si->connection_create_requests64 = 0;
    32a0:	e30810f0 	movw	r1, #33008	@ 0x80f0
	si->connection_create_collisions64 = 0;
    32a4:	e14020f8 	strd	r2, [r0, #-8]
	si->connection_create_requests64 = 0;
    32a8:	e18520f1 	strd	r2, [r5, r1]
	si->connection_destroy_requests64 = 0;
    32ac:	e1a01000 	mov	r1, r0
	si->connection_match_hash_reorders64 = 0;
    32b0:	e1c021f8 	strd	r2, [r0, #24]
	raw_spin_unlock_bh(&lock->rlock);
    32b4:	e2400c81 	sub	r0, r0, #33024	@ 0x8100
	si->connection_destroy_requests64 = 0;
    32b8:	e0c121f0 	strd	r2, [r1], #16
	si->connection_destroy_misses64 = 0;
    32bc:	e14120f8 	strd	r2, [r1, #-8]
	si->connection_flushes64 = 0;
    32c0:	e3081120 	movw	r1, #33056	@ 0x8120
    32c4:	e18520f1 	strd	r2, [r5, r1]
	si->connection_destroy_misses64 = 0;
    32c8:	e3081110 	movw	r1, #33040	@ 0x8110
	si->connection_match_hash_hits64 = 0;
    32cc:	e18520f1 	strd	r2, [r5, r1]
    32d0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
    32d4:	e1a00004 	mov	r0, r4
    32d8:	e24bd01c 	sub	sp, fp, #28
    32dc:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
    32e0:	00010090 	.word	0x00010090
    32e4:	00010150 	.word	0x00010150

000032e8 <sfe_ipv4_debug_dev_write>:
{
    32e8:	e1a0c00d 	mov	ip, sp
    32ec:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    32f0:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock_bh(&lock->rlock);
    32f4:	e3006000 	movw	r6, #0
    32f8:	e1a04002 	mov	r4, r2
    32fc:	e3406000 	movt	r6, #0
	si->connection_create_requests = 0;
    3300:	e3a08000 	mov	r8, #0
    3304:	e1a00006 	mov	r0, r6
    3308:	e3a09000 	mov	r9, #0
    330c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
    3310:	e59f30a8 	ldr	r3, [pc, #168]	@ 33c0 <sfe_ipv4_debug_dev_write+0xd8>
	si->packets_not_forwarded = 0;
    3314:	e2862902 	add	r2, r6, #32768	@ 0x8000
	si->connection_create_requests = 0;
    3318:	e3081040 	movw	r1, #32832	@ 0x8040
	si->packets_not_forwarded = 0;
    331c:	e3a05000 	mov	r5, #0
    3320:	e2820e25 	add	r0, r2, #592	@ 0x250
	si->connection_create_requests = 0;
    3324:	e18680f1 	strd	r8, [r6, r1]
	si->packets_not_forwarded = 0;
    3328:	e5825050 	str	r5, [r2, #80]	@ 0x50
	si->connection_create_requests = 0;
    332c:	e283e010 	add	lr, r3, #16
    3330:	e14380f8 	strd	r8, [r3, #-8]
    3334:	e3083030 	movw	r3, #32816	@ 0x8030
    3338:	e14e80f8 	strd	r8, [lr, #-8]
    333c:	e18680f3 	strd	r8, [r6, r3]
    3340:	e2823f4a 	add	r3, r2, #296	@ 0x128
		si->exception_events64[i] += si->exception_events[i];
    3344:	e5b31008 	ldr	r1, [r3, #8]!
    3348:	e5be2004 	ldr	r2, [lr, #4]!
    334c:	e593c004 	ldr	ip, [r3, #4]
    3350:	e0911002 	adds	r1, r1, r2
    3354:	e5831000 	str	r1, [r3]
    3358:	e2ac1000 	adc	r1, ip, #0
	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
    335c:	e1500003 	cmp	r0, r3
		si->exception_events64[i] += si->exception_events[i];
    3360:	e5831004 	str	r1, [r3, #4]
		si->exception_events[i] = 0;
    3364:	e58e5000 	str	r5, [lr]
	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
    3368:	1afffff5 	bne	3344 <sfe_ipv4_debug_dev_write+0x5c>
	si->packets_forwarded64 = 0;
    336c:	e59f3050 	ldr	r3, [pc, #80]	@ 33c4 <sfe_ipv4_debug_dev_write+0xdc>
    3370:	e3a01000 	mov	r1, #0
    3374:	e3a00000 	mov	r0, #0
    3378:	e3082120 	movw	r2, #33056	@ 0x8120
    337c:	e18600f2 	strd	r0, [r6, r2]
	si->connection_create_requests64 = 0;
    3380:	e14303f8 	strd	r0, [r3, #-56]	@ 0xffffffc8
	si->connection_destroy_requests64 = 0;
    3384:	e14302f8 	strd	r0, [r3, #-40]	@ 0xffffffd8
	si->connection_destroy_misses64 = 0;
    3388:	e14302f0 	strd	r0, [r3, #-32]	@ 0xffffffe0
	si->connection_match_hash_hits64 = 0;
    338c:	e14301f8 	strd	r0, [r3, #-24]	@ 0xffffffe8
	si->connection_flushes64 = 0;
    3390:	e14300f8 	strd	r0, [r3, #-8]
	si->packets_not_forwarded64 = 0;
    3394:	e1c300f8 	strd	r0, [r3, #8]
	si->connection_create_requests64 = 0;
    3398:	e30830f0 	movw	r3, #33008	@ 0x80f0
	si->connection_create_collisions64 = 0;
    339c:	e18600f3 	strd	r0, [r6, r3]
	si->connection_match_hash_hits64 = 0;
    33a0:	e3083110 	movw	r3, #33040	@ 0x8110
	si->connection_match_hash_reorders64 = 0;
    33a4:	e18600f3 	strd	r0, [r6, r3]
	raw_spin_unlock_bh(&lock->rlock);
    33a8:	e3000000 	movw	r0, #0
    33ac:	e3400000 	movt	r0, #0
    33b0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
    33b4:	e1a00004 	mov	r0, r4
    33b8:	e24bd024 	sub	sp, fp, #36	@ 0x24
    33bc:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    33c0:	00018308 	.word	0x00018308
    33c4:	000183e8 	.word	0x000183e8

000033c8 <sfe_ipv4_debug_dev_read_connections_connection>:
{
    33c8:	e1a0c00d 	mov	ip, sp
    33cc:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    33d0:	e24cb004 	sub	fp, ip, #4
    33d4:	e24dd0cc 	sub	sp, sp, #204	@ 0xcc
    33d8:	e1a05000 	mov	r5, r0
    33dc:	e59b6008 	ldr	r6, [fp, #8]
    33e0:	e50b3084 	str	r3, [fp, #-132]	@ 0xffffff7c
    33e4:	e50b1044 	str	r1, [fp, #-68]	@ 0xffffffbc
    33e8:	e50b2040 	str	r2, [fp, #-64]	@ 0xffffffc0
	raw_spin_lock_bh(&lock->rlock);
    33ec:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    33f0:	e595400c 	ldr	r4, [r5, #12]
    33f4:	e3540000 	cmp	r4, #0
    33f8:	0a0000ac 	beq	36b0 <sfe_ipv4_debug_dev_read_connections_connection+0x2e8>
		if (c->debug_read_seq < si->debug_read_seq) {
    33fc:	e2853902 	add	r3, r5, #32768	@ 0x8000
    3400:	e5932260 	ldr	r2, [r3, #608]	@ 0x260
    3404:	ea000002 	b	3414 <sfe_ipv4_debug_dev_read_connections_connection+0x4c>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    3408:	e5944040 	ldr	r4, [r4, #64]	@ 0x40
    340c:	e3540000 	cmp	r4, #0
    3410:	0a0000a6 	beq	36b0 <sfe_ipv4_debug_dev_read_connections_connection+0x2e8>
		if (c->debug_read_seq < si->debug_read_seq) {
    3414:	e594304c 	ldr	r3, [r4, #76]	@ 0x4c
    3418:	e1530002 	cmp	r3, r2
    341c:	2afffff9 	bcs	3408 <sfe_ipv4_debug_dev_read_connections_connection+0x40>
	protocol = c->protocol;
    3420:	e5943008 	ldr	r3, [r4, #8]
			c->debug_read_seq = si->debug_read_seq;
    3424:	e584204c 	str	r2, [r4, #76]	@ 0x4c
	src_ip_xlate = c->src_ip_xlate;
    3428:	e5946010 	ldr	r6, [r4, #16]
	original_cm = c->original_match;
    342c:	e5941024 	ldr	r1, [r4, #36]	@ 0x24
	reply_cm = c->reply_match;
    3430:	e594002c 	ldr	r0, [r4, #44]	@ 0x2c
	protocol = c->protocol;
    3434:	e50b3048 	str	r3, [fp, #-72]	@ 0xffffffb8
	src_ip = c->src_ip;
    3438:	e594300c 	ldr	r3, [r4, #12]
	src_port = c->src_port;
    343c:	e1d471bc 	ldrh	r7, [r4, #28]
	cm->rx_byte_count64 += cm->rx_byte_count;
    3440:	e591c04c 	ldr	ip, [r1, #76]	@ 0x4c
	cm->rx_packet_count64 += cm->rx_packet_count;
    3444:	e5912088 	ldr	r2, [r1, #136]	@ 0x88
    3448:	e591e08c 	ldr	lr, [r1, #140]	@ 0x8c
	src_ip = c->src_ip;
    344c:	e50b303c 	str	r3, [fp, #-60]	@ 0xffffffc4
	cm->rx_packet_count64 += cm->rx_packet_count;
    3450:	e5913048 	ldr	r3, [r1, #72]	@ 0x48
	src_port_xlate = c->src_port_xlate;
    3454:	e1d481be 	ldrh	r8, [r4, #30]
	src_ip_xlate = c->src_ip_xlate;
    3458:	e50b6038 	str	r6, [fp, #-56]	@ 0xffffffc8
	src_dev = c->original_dev;
    345c:	e5946028 	ldr	r6, [r4, #40]	@ 0x28
	cm->rx_packet_count64 += cm->rx_packet_count;
    3460:	e0922003 	adds	r2, r2, r3
	cm->rx_byte_count64 += cm->rx_byte_count;
    3464:	e5913090 	ldr	r3, [r1, #144]	@ 0x90
	cm->rx_packet_count64 += cm->rx_packet_count;
    3468:	e2aee000 	adc	lr, lr, #0
	src_port = c->src_port;
    346c:	e50b7050 	str	r7, [fp, #-80]	@ 0xffffffb0
	cm->rx_packet_count64 += cm->rx_packet_count;
    3470:	e5812088 	str	r2, [r1, #136]	@ 0x88
	cm->rx_packet_count = 0;
    3474:	e3a02000 	mov	r2, #0
	cm->rx_packet_count64 += cm->rx_packet_count;
    3478:	e581e08c 	str	lr, [r1, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    347c:	e093300c 	adds	r3, r3, ip
    3480:	e591c094 	ldr	ip, [r1, #148]	@ 0x94
	src_port_xlate = c->src_port_xlate;
    3484:	e50b8054 	str	r8, [fp, #-84]	@ 0xffffffac
	src_dev = c->original_dev;
    3488:	e50b604c 	str	r6, [fp, #-76]	@ 0xffffffb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    348c:	e5813090 	str	r3, [r1, #144]	@ 0x90
	src_dscp = original_cm->dscp >> SFE_IPV4_DSCP_SHIFT;
    3490:	e591306c 	ldr	r3, [r1, #108]	@ 0x6c
	cm->rx_byte_count64 += cm->rx_byte_count;
    3494:	e2acc000 	adc	ip, ip, #0
    3498:	e581c094 	str	ip, [r1, #148]	@ 0x94
	src_priority = original_cm->priority;
    349c:	e591c068 	ldr	ip, [r1, #104]	@ 0x68
	src_dscp = original_cm->dscp >> SFE_IPV4_DSCP_SHIFT;
    34a0:	e1a0e123 	lsr	lr, r3, #2
	cm->rx_packet_count = 0;
    34a4:	e3a03000 	mov	r3, #0
	src_dscp = original_cm->dscp >> SFE_IPV4_DSCP_SHIFT;
    34a8:	e50be05c 	str	lr, [fp, #-92]	@ 0xffffffa4
	src_priority = original_cm->priority;
    34ac:	e50bc058 	str	ip, [fp, #-88]	@ 0xffffffa8
	cm->rx_packet_count = 0;
    34b0:	e1c124f8 	strd	r2, [r1, #72]	@ 0x48
	cm->rx_packet_count64 += cm->rx_packet_count;
    34b4:	e590c048 	ldr	ip, [r0, #72]	@ 0x48
    34b8:	e590e088 	ldr	lr, [r0, #136]	@ 0x88
    34bc:	e09c700e 	adds	r7, ip, lr
    34c0:	e5807088 	str	r7, [r0, #136]	@ 0x88
	cm->rx_byte_count64 += cm->rx_byte_count;
    34c4:	e590e04c 	ldr	lr, [r0, #76]	@ 0x4c
	cm->rx_packet_count = 0;
    34c8:	e1c024f8 	strd	r2, [r0, #72]	@ 0x48
	dest_dscp = reply_cm->dscp >> SFE_IPV4_DSCP_SHIFT;
    34cc:	e590a06c 	ldr	sl, [r0, #108]	@ 0x6c
	cm->rx_packet_count64 += cm->rx_packet_count;
    34d0:	e590c08c 	ldr	ip, [r0, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    34d4:	e1c089d0 	ldrd	r8, [r0, #144]	@ 0x90
	dest_dscp = reply_cm->dscp >> SFE_IPV4_DSCP_SHIFT;
    34d8:	e1a0a12a 	lsr	sl, sl, #2
	cm->rx_packet_count64 += cm->rx_packet_count;
    34dc:	e2ac6000 	adc	r6, ip, #0
	cm->rx_byte_count64 += cm->rx_byte_count;
    34e0:	e09e8008 	adds	r8, lr, r8
    34e4:	e2a99000 	adc	r9, r9, #0
	cm->rx_packet_count64 += cm->rx_packet_count;
    34e8:	e580608c 	str	r6, [r0, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    34ec:	e1c089f0 	strd	r8, [r0, #144]	@ 0x90
	dest_ip = c->dest_ip;
    34f0:	e5943014 	ldr	r3, [r4, #20]
	dest_dev = c->reply_dev;
    34f4:	e5942030 	ldr	r2, [r4, #48]	@ 0x30
	dest_ip = c->dest_ip;
    34f8:	e50b3034 	str	r3, [fp, #-52]	@ 0xffffffcc
	dest_ip_xlate = c->dest_ip_xlate;
    34fc:	e5943018 	ldr	r3, [r4, #24]
	dest_dev = c->reply_dev;
    3500:	e50b207c 	str	r2, [fp, #-124]	@ 0xffffff84
	dest_port_xlate = c->dest_port_xlate;
    3504:	e1d422b2 	ldrh	r2, [r4, #34]	@ 0x22
	dest_ip_xlate = c->dest_ip_xlate;
    3508:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
	dest_priority = reply_cm->priority;
    350c:	e5903068 	ldr	r3, [r0, #104]	@ 0x68
	dest_port = c->dest_port;
    3510:	e1d402b0 	ldrh	r0, [r4, #32]
	dest_priority = reply_cm->priority;
    3514:	e50b3080 	str	r3, [fp, #-128]	@ 0xffffff80
	dest_port_xlate = c->dest_port_xlate;
    3518:	e50b2074 	str	r2, [fp, #-116]	@ 0xffffff8c
	src_rx_packets = original_cm->rx_packet_count64;
    351c:	e1c128d8 	ldrd	r2, [r1, #136]	@ 0x88
	dest_port = c->dest_port;
    3520:	e50b0070 	str	r0, [fp, #-112]	@ 0xffffff90
	src_rx_bytes = original_cm->rx_byte_count64;
    3524:	e1c109d0 	ldrd	r0, [r1, #144]	@ 0x90
    3528:	e14b06fc 	strd	r0, [fp, #-108]	@ 0xffffff94
	src_rx_packets = original_cm->rx_packet_count64;
    352c:	e14b26f4 	strd	r2, [fp, #-100]	@ 0xffffff9c
	last_sync_jiffies = get_jiffies_64() - c->last_sync_jiffies;
    3530:	ebfffffe 	bl	0 <get_jiffies_64>
    3534:	e594e038 	ldr	lr, [r4, #56]	@ 0x38
    3538:	e1a0c000 	mov	ip, r0
	raw_spin_unlock_bh(&lock->rlock);
    353c:	e1a00005 	mov	r0, r5
    3540:	e05c500e 	subs	r5, ip, lr
    3544:	e594c03c 	ldr	ip, [r4, #60]	@ 0x3c
	mark = c->mark;
    3548:	e5944048 	ldr	r4, [r4, #72]	@ 0x48
	last_sync_jiffies = get_jiffies_64() - c->last_sync_jiffies;
    354c:	e0c1100c 	sbc	r1, r1, ip
    3550:	e50b1078 	str	r1, [fp, #-120]	@ 0xffffff88
    3554:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    3558:	e14b26d4 	ldrd	r2, [fp, #-100]	@ 0xffffff9c
    355c:	e1cd22f0 	strd	r2, [sp, #32]
    3560:	e14b06dc 	ldrd	r0, [fp, #-108]	@ 0xffffff94
    3564:	e1cd02f8 	strd	r0, [sp, #40]	@ 0x28
    3568:	e51b204c 	ldr	r2, [fp, #-76]	@ 0xffffffb4
    356c:	e58d2000 	str	r2, [sp]
    3570:	e51bc058 	ldr	ip, [fp, #-88]	@ 0xffffffa8
    3574:	e58dc014 	str	ip, [sp, #20]
    3578:	e51be05c 	ldr	lr, [fp, #-92]	@ 0xffffffa4
    357c:	e58de018 	str	lr, [sp, #24]
    3580:	e51b207c 	ldr	r2, [fp, #-124]	@ 0xffffff84
    3584:	e58d2030 	str	r2, [sp, #48]	@ 0x30
    3588:	e51b3080 	ldr	r3, [fp, #-128]	@ 0xffffff80
    358c:	e58d5060 	str	r5, [sp, #96]	@ 0x60
    3590:	e58d3044 	str	r3, [sp, #68]	@ 0x44
    3594:	e58da048 	str	sl, [sp, #72]	@ 0x48
    3598:	e58d7050 	str	r7, [sp, #80]	@ 0x50
    359c:	e58d6054 	str	r6, [sp, #84]	@ 0x54
    35a0:	e1cd85f8 	strd	r8, [sp, #88]	@ 0x58
    35a4:	e51b1078 	ldr	r1, [fp, #-120]	@ 0xffffff88
    35a8:	e58d1064 	str	r1, [sp, #100]	@ 0x64
    35ac:	e3a01c03 	mov	r1, #768	@ 0x300
    35b0:	e58d4068 	str	r4, [sp, #104]	@ 0x68
				ntohs(dest_port), ntohs(dest_port_xlate),
    35b4:	e51b0070 	ldr	r0, [fp, #-112]	@ 0xffffff90
				ntohs(src_port), ntohs(src_port_xlate),
    35b8:	e51b8054 	ldr	r8, [fp, #-84]	@ 0xffffffac
    35bc:	e51b7050 	ldr	r7, [fp, #-80]	@ 0xffffffb0
    35c0:	e6bf3fb8 	rev16	r3, r8
    35c4:	e6bf2fb7 	rev16	r2, r7
    35c8:	e6ff3073 	uxth	r3, r3
    35cc:	e6ff2072 	uxth	r2, r2
				ntohs(dest_port), ntohs(dest_port_xlate),
    35d0:	e6bf7fb0 	rev16	r7, r0
    35d4:	e6ff7077 	uxth	r7, r7
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    35d8:	e1cd20fc 	strd	r2, [sp, #12]
				ntohs(dest_port), ntohs(dest_port_xlate),
    35dc:	e51b2074 	ldr	r2, [fp, #-116]	@ 0xffffff8c
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    35e0:	e58d703c 	str	r7, [sp, #60]	@ 0x3c
				ntohs(dest_port), ntohs(dest_port_xlate),
    35e4:	e6bf6fb2 	rev16	r6, r2
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    35e8:	e24b2030 	sub	r2, fp, #48	@ 0x30
				ntohs(dest_port), ntohs(dest_port_xlate),
    35ec:	e6ff6076 	uxth	r6, r6
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    35f0:	e58d6040 	str	r6, [sp, #64]	@ 0x40
    35f4:	e51b3048 	ldr	r3, [fp, #-72]	@ 0xffffffb8
    35f8:	e58d2038 	str	r2, [sp, #56]	@ 0x38
    35fc:	e24b2034 	sub	r2, fp, #52	@ 0x34
    3600:	e58d2034 	str	r2, [sp, #52]	@ 0x34
    3604:	e24b2038 	sub	r2, fp, #56	@ 0x38
    3608:	e58d2008 	str	r2, [sp, #8]
    360c:	e24b203c 	sub	r2, fp, #60	@ 0x3c
    3610:	e58d2004 	str	r2, [sp, #4]
    3614:	e3002000 	movw	r2, #0
    3618:	e51b0040 	ldr	r0, [fp, #-64]	@ 0xffffffc0
    361c:	e3402000 	movt	r2, #0
    3620:	ebfffffe 	bl	0 <snprintf>
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3624:	e51b2044 	ldr	r2, [fp, #-68]	@ 0xffffffbc
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
    3628:	e1a04000 	mov	r4, r0
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    362c:	e59b3004 	ldr	r3, [fp, #4]
    3630:	e5933000 	ldr	r3, [r3]
    3634:	e0820003 	add	r0, r2, r3
    3638:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    363c:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    3640:	e1500003 	cmp	r0, r3
    3644:	9a000002 	bls	3654 <sfe_ipv4_debug_dev_read_connections_connection+0x28c>
		return false;
    3648:	e3a00000 	mov	r0, #0
}
    364c:	e24bd028 	sub	sp, fp, #40	@ 0x28
    3650:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
    3654:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    3658:	ee135f10 	mrc	15, 0, r5, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    365c:	e3c5300c 	bic	r3, r5, #12
    3660:	e3833004 	orr	r3, r3, #4
	asm volatile(
    3664:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    3668:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    366c:	e51b1040 	ldr	r1, [fp, #-64]	@ 0xffffffc0
    3670:	e3a02c03 	mov	r2, #768	@ 0x300
    3674:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    3678:	ee035f10 	mcr	15, 0, r5, cr3, cr0, {0}
	isb();
    367c:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3680:	e3500000 	cmp	r0, #0
    3684:	1affffef 	bne	3648 <sfe_ipv4_debug_dev_read_connections_connection+0x280>
	*length -= bytes_read;
    3688:	e51b2084 	ldr	r2, [fp, #-132]	@ 0xffffff7c
    368c:	e5923000 	ldr	r3, [r2]
    3690:	e0433004 	sub	r3, r3, r4
    3694:	e5823000 	str	r3, [r2]
	*total_read += bytes_read;
    3698:	e59b3004 	ldr	r3, [fp, #4]
    369c:	e59b2004 	ldr	r2, [fp, #4]
    36a0:	e5933000 	ldr	r3, [r3]
    36a4:	e0833004 	add	r3, r3, r4
    36a8:	e5823000 	str	r3, [r2]
	return true;
    36ac:	ea000004 	b	36c4 <sfe_ipv4_debug_dev_read_connections_connection+0x2fc>
    36b0:	e1a00005 	mov	r0, r5
    36b4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		ws->state++;
    36b8:	e5963000 	ldr	r3, [r6]
    36bc:	e2833001 	add	r3, r3, #1
    36c0:	e5863000 	str	r3, [r6]
		return true;
    36c4:	e3a00001 	mov	r0, #1
}
    36c8:	e24bd028 	sub	sp, fp, #40	@ 0x28
    36cc:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

000036d0 <sfe_ipv6_debug_dev_read_exceptions_exception>:
{
    36d0:	e1a0c00d 	mov	ip, sp
    36d4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    36d8:	e24cb004 	sub	fp, ip, #4
    36dc:	e24dd008 	sub	sp, sp, #8
    36e0:	e59b5008 	ldr	r5, [fp, #8]
    36e4:	e1a04000 	mov	r4, r0
    36e8:	e1a06001 	mov	r6, r1
    36ec:	e1a07002 	mov	r7, r2
    36f0:	e1a08003 	mov	r8, r3
	raw_spin_lock_bh(&lock->rlock);
    36f4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	raw_spin_unlock_bh(&lock->rlock);
    36f8:	e1a00004 	mov	r0, r4
	ct = si->exception_events64[ws->iter_exception];
    36fc:	e595c004 	ldr	ip, [r5, #4]
    3700:	e28cca01 	add	ip, ip, #4096	@ 0x1000
    3704:	e28cc027 	add	ip, ip, #39	@ 0x27
    3708:	e084318c 	add	r3, r4, ip, lsl #3
    370c:	e794918c 	ldr	r9, [r4, ip, lsl #3]
    3710:	e5934004 	ldr	r4, [r3, #4]
    3714:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (ct) {
    3718:	e1993004 	orrs	r3, r9, r4
    371c:	1a00000d 	bne	3758 <sfe_ipv6_debug_dev_read_exceptions_exception+0x88>
	ws->iter_exception++;
    3720:	e5953004 	ldr	r3, [r5, #4]
    3724:	e2833001 	add	r3, r3, #1
	if (ws->iter_exception >= SFE_IPV6_EXCEPTION_EVENT_LAST) {
    3728:	e3530025 	cmp	r3, #37	@ 0x25
    372c:	ca000003 	bgt	3740 <sfe_ipv6_debug_dev_read_exceptions_exception+0x70>
	ws->iter_exception++;
    3730:	e5853004 	str	r3, [r5, #4]
	return true;
    3734:	e3a00001 	mov	r0, #1
}
    3738:	e24bd024 	sub	sp, fp, #36	@ 0x24
    373c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		ws->state++;
    3740:	e5953000 	ldr	r3, [r5]
		ws->iter_exception = 0;
    3744:	e3a02000 	mov	r2, #0
    3748:	e5852004 	str	r2, [r5, #4]
		ws->state++;
    374c:	e2833001 	add	r3, r3, #1
    3750:	e5853000 	str	r3, [r5]
    3754:	eafffff6 	b	3734 <sfe_ipv6_debug_dev_read_exceptions_exception+0x64>
		bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE,
    3758:	e58d9000 	str	r9, [sp]
    375c:	e3003000 	movw	r3, #0
    3760:	e3002000 	movw	r2, #0
    3764:	e58d4004 	str	r4, [sp, #4]
    3768:	e5951004 	ldr	r1, [r5, #4]
    376c:	e3403000 	movt	r3, #0
    3770:	e3402000 	movt	r2, #0
    3774:	e1a00007 	mov	r0, r7
    3778:	e0833101 	add	r3, r3, r1, lsl #2
    377c:	e3a01c03 	mov	r1, #768	@ 0x300
    3780:	e59330b8 	ldr	r3, [r3, #184]	@ 0xb8
    3784:	ebfffffe 	bl	0 <snprintf>
		if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3788:	e59b3004 	ldr	r3, [fp, #4]
		bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE,
    378c:	e1a04000 	mov	r4, r0
		if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3790:	e5933000 	ldr	r3, [r3]
    3794:	e0860003 	add	r0, r6, r3
    3798:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    379c:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    37a0:	e1500003 	cmp	r0, r3
    37a4:	8a000015 	bhi	3800 <sfe_ipv6_debug_dev_read_exceptions_exception+0x130>
    37a8:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    37ac:	ee136f10 	mrc	15, 0, r6, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    37b0:	e3c6300c 	bic	r3, r6, #12
    37b4:	e3833004 	orr	r3, r3, #4
	asm volatile(
    37b8:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    37bc:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    37c0:	e3a02c03 	mov	r2, #768	@ 0x300
    37c4:	e1a01007 	mov	r1, r7
    37c8:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    37cc:	ee036f10 	mcr	15, 0, r6, cr3, cr0, {0}
	isb();
    37d0:	f57ff06f 	isb	sy
    37d4:	e3500000 	cmp	r0, #0
    37d8:	1a000008 	bne	3800 <sfe_ipv6_debug_dev_read_exceptions_exception+0x130>
		*length -= bytes_read;
    37dc:	e5983000 	ldr	r3, [r8]
		*total_read += bytes_read;
    37e0:	e59b2004 	ldr	r2, [fp, #4]
		*length -= bytes_read;
    37e4:	e0433004 	sub	r3, r3, r4
    37e8:	e5883000 	str	r3, [r8]
		*total_read += bytes_read;
    37ec:	e59b3004 	ldr	r3, [fp, #4]
    37f0:	e5933000 	ldr	r3, [r3]
    37f4:	e0833004 	add	r3, r3, r4
    37f8:	e5823000 	str	r3, [r2]
    37fc:	eaffffc7 	b	3720 <sfe_ipv6_debug_dev_read_exceptions_exception+0x50>
			return false;
    3800:	e3a00000 	mov	r0, #0
}
    3804:	e24bd024 	sub	sp, fp, #36	@ 0x24
    3808:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

0000380c <sfe_ipv4_debug_dev_read_exceptions_exception>:
{
    380c:	e1a0c00d 	mov	ip, sp
    3810:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    3814:	e24cb004 	sub	fp, ip, #4
    3818:	e24dd008 	sub	sp, sp, #8
    381c:	e59b5008 	ldr	r5, [fp, #8]
    3820:	e1a04000 	mov	r4, r0
    3824:	e1a06001 	mov	r6, r1
    3828:	e1a07002 	mov	r7, r2
    382c:	e1a08003 	mov	r8, r3
	raw_spin_lock_bh(&lock->rlock);
    3830:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	raw_spin_unlock_bh(&lock->rlock);
    3834:	e1a00004 	mov	r0, r4
	ct = si->exception_events64[ws->iter_exception];
    3838:	e595c004 	ldr	ip, [r5, #4]
    383c:	e28cca01 	add	ip, ip, #4096	@ 0x1000
    3840:	e28cc026 	add	ip, ip, #38	@ 0x26
    3844:	e084318c 	add	r3, r4, ip, lsl #3
    3848:	e794918c 	ldr	r9, [r4, ip, lsl #3]
    384c:	e5934004 	ldr	r4, [r3, #4]
    3850:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (ct) {
    3854:	e1993004 	orrs	r3, r9, r4
    3858:	1a00000d 	bne	3894 <sfe_ipv4_debug_dev_read_exceptions_exception+0x88>
	ws->iter_exception++;
    385c:	e5953004 	ldr	r3, [r5, #4]
    3860:	e2833001 	add	r3, r3, #1
	if (ws->iter_exception >= SFE_IPV4_EXCEPTION_EVENT_LAST) {
    3864:	e3530024 	cmp	r3, #36	@ 0x24
    3868:	ca000003 	bgt	387c <sfe_ipv4_debug_dev_read_exceptions_exception+0x70>
	ws->iter_exception++;
    386c:	e5853004 	str	r3, [r5, #4]
	return true;
    3870:	e3a00001 	mov	r0, #1
}
    3874:	e24bd024 	sub	sp, fp, #36	@ 0x24
    3878:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
		ws->state++;
    387c:	e5953000 	ldr	r3, [r5]
		ws->iter_exception = 0;
    3880:	e3a02000 	mov	r2, #0
    3884:	e5852004 	str	r2, [r5, #4]
		ws->state++;
    3888:	e2833001 	add	r3, r3, #1
    388c:	e5853000 	str	r3, [r5]
    3890:	eafffff6 	b	3870 <sfe_ipv4_debug_dev_read_exceptions_exception+0x64>
		bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE,
    3894:	e58d9000 	str	r9, [sp]
    3898:	e3003000 	movw	r3, #0
    389c:	e3002000 	movw	r2, #0
    38a0:	e58d4004 	str	r4, [sp, #4]
    38a4:	e5951004 	ldr	r1, [r5, #4]
    38a8:	e3403000 	movt	r3, #0
    38ac:	e3402000 	movt	r2, #0
    38b0:	e1a00007 	mov	r0, r7
    38b4:	e0833101 	add	r3, r3, r1, lsl #2
    38b8:	e3a01c03 	mov	r1, #768	@ 0x300
    38bc:	e5933150 	ldr	r3, [r3, #336]	@ 0x150
    38c0:	ebfffffe 	bl	0 <snprintf>
		if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    38c4:	e59b3004 	ldr	r3, [fp, #4]
		bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE,
    38c8:	e1a04000 	mov	r4, r0
		if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    38cc:	e5933000 	ldr	r3, [r3]
    38d0:	e0860003 	add	r0, r6, r3
    38d4:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    38d8:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    38dc:	e1500003 	cmp	r0, r3
    38e0:	8a000015 	bhi	393c <sfe_ipv4_debug_dev_read_exceptions_exception+0x130>
    38e4:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    38e8:	ee136f10 	mrc	15, 0, r6, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    38ec:	e3c6300c 	bic	r3, r6, #12
    38f0:	e3833004 	orr	r3, r3, #4
	asm volatile(
    38f4:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    38f8:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    38fc:	e3a02c03 	mov	r2, #768	@ 0x300
    3900:	e1a01007 	mov	r1, r7
    3904:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    3908:	ee036f10 	mcr	15, 0, r6, cr3, cr0, {0}
	isb();
    390c:	f57ff06f 	isb	sy
    3910:	e3500000 	cmp	r0, #0
    3914:	1a000008 	bne	393c <sfe_ipv4_debug_dev_read_exceptions_exception+0x130>
		*length -= bytes_read;
    3918:	e5983000 	ldr	r3, [r8]
		*total_read += bytes_read;
    391c:	e59b2004 	ldr	r2, [fp, #4]
		*length -= bytes_read;
    3920:	e0433004 	sub	r3, r3, r4
    3924:	e5883000 	str	r3, [r8]
		*total_read += bytes_read;
    3928:	e59b3004 	ldr	r3, [fp, #4]
    392c:	e5933000 	ldr	r3, [r3]
    3930:	e0833004 	add	r3, r3, r4
    3934:	e5823000 	str	r3, [r2]
    3938:	eaffffc7 	b	385c <sfe_ipv4_debug_dev_read_exceptions_exception+0x50>
			return false;
    393c:	e3a00000 	mov	r0, #0
}
    3940:	e24bd024 	sub	sp, fp, #36	@ 0x24
    3944:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}

00003948 <fast_classifier_find_conn>:
{
    3948:	e1a0c00d 	mov	ip, sp
    394c:	e92dd9f0 	push	{r4, r5, r6, r7, r8, fp, ip, lr, pc}
    3950:	e24cb004 	sub	fp, ip, #4
    3954:	e5dbc008 	ldrb	ip, [fp, #8]
    3958:	e1a06000 	mov	r6, r0
	return hash ^ (sport | (dport << 16));
    395c:	e1820803 	orr	r0, r2, r3, lsl #16
	u32 hash = saddr->ip ^ daddr->ip;
    3960:	e5915000 	ldr	r5, [r1]
    3964:	e5964000 	ldr	r4, [r6]
{
    3968:	e5dbe004 	ldrb	lr, [fp, #4]
	if (is_v4)
    396c:	e35c0000 	cmp	ip, #0
    3970:	0a000024 	beq	3a08 <fast_classifier_find_conn+0xc0>
	return hash ^ (sport | (dport << 16));
    3974:	e0241000 	eor	r1, r4, r0
#ifndef HAVE_ARCH__HASH_32
#define __hash_32 __hash_32_generic
#endif
static inline u32 __hash_32_generic(u32 val)
{
	return val * GOLDEN_RATIO_32;
    3978:	e3080647 	movw	r0, #34375	@ 0x8647
    397c:	e34601c8 	movt	r0, #25032	@ 0x61c8
    3980:	e0211005 	eor	r1, r1, r5
    3984:	e0010190 	mul	r1, r0, r1
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    3988:	e3000000 	movw	r0, #0
    398c:	e3400000 	movt	r0, #0
}

static inline u32 hash_32(u32 val, unsigned int bits)
{
	/* High bits are more random, so use them. */
	return __hash_32(val) >> (32 - bits);
    3990:	e1a019a1 	lsr	r1, r1, #19
    3994:	e7900101 	ldr	r0, [r0, r1, lsl #2]
    3998:	e3500000 	cmp	r0, #0
    399c:	1a000007 	bne	39c0 <fast_classifier_find_conn+0x78>
}
    39a0:	e24bd020 	sub	sp, fp, #32
    39a4:	e89da9f0 	ldm	sp, {r4, r5, r6, r7, r8, fp, sp, pc}
		if (p_sic->protocol == proto &&
    39a8:	e1d1c5b8 	ldrh	ip, [r1, #88]	@ 0x58
    39ac:	e15c0002 	cmp	ip, r2
    39b0:	0a00000a 	beq	39e0 <fast_classifier_find_conn+0x98>
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    39b4:	e5900000 	ldr	r0, [r0]
    39b8:	e3500000 	cmp	r0, #0
    39bc:	0afffff7 	beq	39a0 <fast_classifier_find_conn+0x58>
		if (conn->is_v4 != is_v4) {
    39c0:	e5d0101c 	ldrb	r1, [r0, #28]
    39c4:	e3510000 	cmp	r1, #0
    39c8:	0afffff9 	beq	39b4 <fast_classifier_find_conn+0x6c>
		p_sic = conn->sic;
    39cc:	e5901008 	ldr	r1, [r0, #8]
		if (p_sic->protocol == proto &&
    39d0:	e591c000 	ldr	ip, [r1]
    39d4:	e15c000e 	cmp	ip, lr
    39d8:	1afffff5 	bne	39b4 <fast_classifier_find_conn+0x6c>
    39dc:	eafffff1 	b	39a8 <fast_classifier_find_conn+0x60>
		    p_sic->src_port == sport &&
    39e0:	e1d1c5bc 	ldrh	ip, [r1, #92]	@ 0x5c
    39e4:	e15c0003 	cmp	ip, r3
    39e8:	1afffff1 	bne	39b4 <fast_classifier_find_conn+0x6c>
		    p_sic->dest_port == dport &&
    39ec:	e591c018 	ldr	ip, [r1, #24]
    39f0:	e154000c 	cmp	r4, ip
    39f4:	1affffee 	bne	39b4 <fast_classifier_find_conn+0x6c>
		    sfe_addr_equal(&p_sic->src_ip, saddr, is_v4) &&
    39f8:	e5911038 	ldr	r1, [r1, #56]	@ 0x38
    39fc:	e1550001 	cmp	r5, r1
    3a00:	1affffeb 	bne	39b4 <fast_classifier_find_conn+0x6c>
    3a04:	eaffffe5 	b	39a0 <fast_classifier_find_conn+0x58>
	return hash ^ (sport | (dport << 16));
    3a08:	e591c004 	ldr	ip, [r1, #4]
    3a0c:	e5917008 	ldr	r7, [r1, #8]
    3a10:	e02cc005 	eor	ip, ip, r5
    3a14:	e02cc007 	eor	ip, ip, r7
    3a18:	e591700c 	ldr	r7, [r1, #12]
    3a1c:	e02cc007 	eor	ip, ip, r7
    3a20:	e02cc004 	eor	ip, ip, r4
    3a24:	e02cc000 	eor	ip, ip, r0
    3a28:	e5960004 	ldr	r0, [r6, #4]
    3a2c:	e02cc000 	eor	ip, ip, r0
    3a30:	e5960008 	ldr	r0, [r6, #8]
    3a34:	e02cc000 	eor	ip, ip, r0
    3a38:	e596000c 	ldr	r0, [r6, #12]
    3a3c:	e02cc000 	eor	ip, ip, r0
	return val * GOLDEN_RATIO_32;
    3a40:	e3080647 	movw	r0, #34375	@ 0x8647
    3a44:	e34601c8 	movt	r0, #25032	@ 0x61c8
    3a48:	e00c0c90 	mul	ip, r0, ip
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    3a4c:	e3000000 	movw	r0, #0
    3a50:	e3400000 	movt	r0, #0
	return __hash_32(val) >> (32 - bits);
    3a54:	e1a0c9ac 	lsr	ip, ip, #19
    3a58:	e790010c 	ldr	r0, [r0, ip, lsl #2]
    3a5c:	e3500000 	cmp	r0, #0
    3a60:	0affffce 	beq	39a0 <fast_classifier_find_conn+0x58>
		if (conn->is_v4 != is_v4) {
    3a64:	e5d0c01c 	ldrb	ip, [r0, #28]
    3a68:	e35c0000 	cmp	ip, #0
    3a6c:	0a000005 	beq	3a88 <fast_classifier_find_conn+0x140>
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    3a70:	e5900000 	ldr	r0, [r0]
    3a74:	e3500000 	cmp	r0, #0
    3a78:	0affffc8 	beq	39a0 <fast_classifier_find_conn+0x58>
		if (conn->is_v4 != is_v4) {
    3a7c:	e5d0c01c 	ldrb	ip, [r0, #28]
    3a80:	e35c0000 	cmp	ip, #0
    3a84:	1afffff9 	bne	3a70 <fast_classifier_find_conn+0x128>
		p_sic = conn->sic;
    3a88:	e590c008 	ldr	ip, [r0, #8]
		if (p_sic->protocol == proto &&
    3a8c:	e59c7000 	ldr	r7, [ip]
    3a90:	e15e0007 	cmp	lr, r7
    3a94:	1afffff5 	bne	3a70 <fast_classifier_find_conn+0x128>
    3a98:	e1dc75b8 	ldrh	r7, [ip, #88]	@ 0x58
    3a9c:	e1570002 	cmp	r7, r2
    3aa0:	1afffff2 	bne	3a70 <fast_classifier_find_conn+0x128>
		    p_sic->src_port == sport &&
    3aa4:	e1dc75bc 	ldrh	r7, [ip, #92]	@ 0x5c
    3aa8:	e1570003 	cmp	r7, r3
    3aac:	1affffef 	bne	3a70 <fast_classifier_find_conn+0x128>
    3ab0:	e59c7018 	ldr	r7, [ip, #24]
    3ab4:	e1540007 	cmp	r4, r7
    3ab8:	1affffec 	bne	3a70 <fast_classifier_find_conn+0x128>
	return a->addr[0] == b->addr[0] &&
    3abc:	e5967004 	ldr	r7, [r6, #4]
    3ac0:	e59c801c 	ldr	r8, [ip, #28]
    3ac4:	e1580007 	cmp	r8, r7
    3ac8:	1affffe8 	bne	3a70 <fast_classifier_find_conn+0x128>
	       a->addr[1] == b->addr[1] &&
    3acc:	e5967008 	ldr	r7, [r6, #8]
    3ad0:	e59c8020 	ldr	r8, [ip, #32]
    3ad4:	e1580007 	cmp	r8, r7
    3ad8:	1affffe4 	bne	3a70 <fast_classifier_find_conn+0x128>
	       a->addr[2] == b->addr[2] &&
    3adc:	e596700c 	ldr	r7, [r6, #12]
    3ae0:	e59c8024 	ldr	r8, [ip, #36]	@ 0x24
    3ae4:	e1580007 	cmp	r8, r7
    3ae8:	1affffe0 	bne	3a70 <fast_classifier_find_conn+0x128>
    3aec:	e59c7038 	ldr	r7, [ip, #56]	@ 0x38
    3af0:	e1550007 	cmp	r5, r7
    3af4:	1affffdd 	bne	3a70 <fast_classifier_find_conn+0x128>
	return a->addr[0] == b->addr[0] &&
    3af8:	e5918004 	ldr	r8, [r1, #4]
    3afc:	e59c703c 	ldr	r7, [ip, #60]	@ 0x3c
    3b00:	e1580007 	cmp	r8, r7
    3b04:	1affffd9 	bne	3a70 <fast_classifier_find_conn+0x128>
	       a->addr[1] == b->addr[1] &&
    3b08:	e5917008 	ldr	r7, [r1, #8]
    3b0c:	e59c8040 	ldr	r8, [ip, #64]	@ 0x40
    3b10:	e1580007 	cmp	r8, r7
    3b14:	1affffd5 	bne	3a70 <fast_classifier_find_conn+0x128>
	       a->addr[2] == b->addr[2] &&
    3b18:	e59c7044 	ldr	r7, [ip, #68]	@ 0x44
    3b1c:	e591c00c 	ldr	ip, [r1, #12]
    3b20:	e157000c 	cmp	r7, ip
    3b24:	1affffd1 	bne	3a70 <fast_classifier_find_conn+0x128>
    3b28:	eaffff9c 	b	39a0 <fast_classifier_find_conn+0x58>

00003b2c <fast_classifier_inet_event>:
{
    3b2c:	e1a0c00d 	mov	ip, sp
    3b30:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
    3b34:	e24cb004 	sub	fp, ip, #4
	struct net_device *dev = ((struct in_ifaddr *)ptr)->ifa_dev->dev;
    3b38:	e592300c 	ldr	r3, [r2, #12]
    3b3c:	e5935000 	ldr	r5, [r3]
	if (dev && (event == NETDEV_DOWN)) {
    3b40:	e2553000 	subs	r3, r5, #0
    3b44:	13a03001 	movne	r3, #1
    3b48:	e3510002 	cmp	r1, #2
    3b4c:	13a03000 	movne	r3, #0
    3b50:	e3530000 	cmp	r3, #0
    3b54:	1a000002 	bne	3b64 <fast_classifier_inet_event+0x38>
}
    3b58:	e3a00000 	mov	r0, #0
    3b5c:	e24bd01c 	sub	sp, fp, #28
    3b60:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
    3b64:	e3006000 	movw	r6, #0
    3b68:	e3406000 	movt	r6, #0
	raw_spin_lock_bh(&lock->rlock);
    3b6c:	e1a00006 	mov	r0, r6
    3b70:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    3b74:	e596400c 	ldr	r4, [r6, #12]
    3b78:	e3540000 	cmp	r4, #0
    3b7c:	1a000006 	bne	3b9c <fast_classifier_inet_event+0x70>
    3b80:	ea00000f 	b	3bc4 <fast_classifier_inet_event+0x98>
		    || (dev == c->reply_dev)) {
    3b84:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    3b88:	e1550003 	cmp	r5, r3
    3b8c:	0a000005 	beq	3ba8 <fast_classifier_inet_event+0x7c>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    3b90:	e5944040 	ldr	r4, [r4, #64]	@ 0x40
    3b94:	e3540000 	cmp	r4, #0
    3b98:	0a000009 	beq	3bc4 <fast_classifier_inet_event+0x98>
		    || (dev == c->original_dev)
    3b9c:	e5943028 	ldr	r3, [r4, #40]	@ 0x28
    3ba0:	e1550003 	cmp	r5, r3
    3ba4:	1afffff6 	bne	3b84 <fast_classifier_inet_event+0x58>
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    3ba8:	e1a00004 	mov	r0, r4
    3bac:	ebfff9a9 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
    3bb0:	e1a00006 	mov	r0, r6
    3bb4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
    3bb8:	e1a00004 	mov	r0, r4
    3bbc:	ebfff844 	bl	1cd4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0>
		goto another_round;
    3bc0:	eaffffe9 	b	3b6c <fast_classifier_inet_event+0x40>
    3bc4:	e3000000 	movw	r0, #0
    3bc8:	e3400000 	movt	r0, #0
    3bcc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    3bd0:	e3a00000 	mov	r0, #0
    3bd4:	e24bd01c 	sub	sp, fp, #28
    3bd8:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

00003bdc <sfe_ipv6_debug_dev_read_stats>:
{
    3bdc:	e1a0c00d 	mov	ip, sp
    3be0:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    3be4:	e24cb004 	sub	fp, ip, #4
    3be8:	e24dd09c 	sub	sp, sp, #156	@ 0x9c
    3bec:	e1a04000 	mov	r4, r0
    3bf0:	e50b3078 	str	r3, [fp, #-120]	@ 0xffffff88
	si->packets_forwarded64 += si->packets_forwarded;
    3bf4:	e3088130 	movw	r8, #33072	@ 0x8130
{
    3bf8:	e50b103c 	str	r1, [fp, #-60]	@ 0xffffffc4
	si->connection_create_requests = 0;
    3bfc:	e3a06000 	mov	r6, #0
    3c00:	e3a07000 	mov	r7, #0
{
    3c04:	e50b2030 	str	r2, [fp, #-48]	@ 0xffffffd0
	raw_spin_lock_bh(&lock->rlock);
    3c08:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_create_requests64 += si->connection_create_requests;
    3c0c:	e2840902 	add	r0, r4, #32768	@ 0x8000
    3c10:	e30830f0 	movw	r3, #33008	@ 0x80f0
    3c14:	e794c003 	ldr	ip, [r4, r3]
    3c18:	e0841003 	add	r1, r4, r3
	si->connection_create_requests = 0;
    3c1c:	e2805050 	add	r5, r0, #80	@ 0x50
	si->connection_create_requests64 += si->connection_create_requests;
    3c20:	e5902030 	ldr	r2, [r0, #48]	@ 0x30
    3c24:	e092200c 	adds	r2, r2, ip
    3c28:	e1a0c002 	mov	ip, r2
    3c2c:	e5912004 	ldr	r2, [r1, #4]
    3c30:	e784c003 	str	ip, [r4, r3]
    3c34:	e50bc040 	str	ip, [fp, #-64]	@ 0xffffffc0
    3c38:	e2a23000 	adc	r3, r2, #0
    3c3c:	e50b3044 	str	r3, [fp, #-68]	@ 0xffffffbc
    3c40:	e5813004 	str	r3, [r1, #4]
	si->connection_create_collisions64 += si->connection_create_collisions;
    3c44:	e2843c81 	add	r3, r4, #33024	@ 0x8100
    3c48:	e5902034 	ldr	r2, [r0, #52]	@ 0x34
    3c4c:	e5131008 	ldr	r1, [r3, #-8]
    3c50:	e0922001 	adds	r2, r2, r1
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3c54:	e5931000 	ldr	r1, [r3]
	si->connection_create_collisions64 += si->connection_create_collisions;
    3c58:	e50b2048 	str	r2, [fp, #-72]	@ 0xffffffb8
    3c5c:	e5032008 	str	r2, [r3, #-8]
    3c60:	e5132004 	ldr	r2, [r3, #-4]
    3c64:	e2a22000 	adc	r2, r2, #0
    3c68:	e50b204c 	str	r2, [fp, #-76]	@ 0xffffffb4
    3c6c:	e5032004 	str	r2, [r3, #-4]
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3c70:	e5902038 	ldr	r2, [r0, #56]	@ 0x38
    3c74:	e0922001 	adds	r2, r2, r1
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    3c78:	e3081110 	movw	r1, #33040	@ 0x8110
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3c7c:	e50b2050 	str	r2, [fp, #-80]	@ 0xffffffb0
    3c80:	e5832000 	str	r2, [r3]
    3c84:	e5932004 	ldr	r2, [r3, #4]
    3c88:	e2a22000 	adc	r2, r2, #0
    3c8c:	e5832004 	str	r2, [r3, #4]
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    3c90:	e0843001 	add	r3, r4, r1
    3c94:	e590c03c 	ldr	ip, [r0, #60]	@ 0x3c
    3c98:	e513e008 	ldr	lr, [r3, #-8]
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3c9c:	e50b2054 	str	r2, [fp, #-84]	@ 0xffffffac
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
    3ca0:	e3082120 	movw	r2, #33056	@ 0x8120
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    3ca4:	e09cc00e 	adds	ip, ip, lr
    3ca8:	e50bc058 	str	ip, [fp, #-88]	@ 0xffffffa8
    3cac:	e503c008 	str	ip, [r3, #-8]
    3cb0:	e513c004 	ldr	ip, [r3, #-4]
    3cb4:	e2acc000 	adc	ip, ip, #0
    3cb8:	e50bc05c 	str	ip, [fp, #-92]	@ 0xffffffa4
    3cbc:	e503c004 	str	ip, [r3, #-4]
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
    3cc0:	e794e001 	ldr	lr, [r4, r1]
    3cc4:	e590c040 	ldr	ip, [r0, #64]	@ 0x40
    3cc8:	e09cc00e 	adds	ip, ip, lr
    3ccc:	e1a0e00c 	mov	lr, ip
    3cd0:	e593c004 	ldr	ip, [r3, #4]
    3cd4:	e50be060 	str	lr, [fp, #-96]	@ 0xffffffa0
    3cd8:	e784e001 	str	lr, [r4, r1]
	si->packets_forwarded64 += si->packets_forwarded;
    3cdc:	e084e008 	add	lr, r4, r8
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
    3ce0:	e2ac1000 	adc	r1, ip, #0
    3ce4:	e5831004 	str	r1, [r3, #4]
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
    3ce8:	e0843002 	add	r3, r4, r2
    3cec:	e513c008 	ldr	ip, [r3, #-8]
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
    3cf0:	e50b1064 	str	r1, [fp, #-100]	@ 0xffffff9c
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
    3cf4:	e5901044 	ldr	r1, [r0, #68]	@ 0x44
    3cf8:	e50b5034 	str	r5, [fp, #-52]	@ 0xffffffcc
    3cfc:	e50be038 	str	lr, [fp, #-56]	@ 0xffffffc8
    3d00:	e091a00c 	adds	sl, r1, ip
    3d04:	e5131004 	ldr	r1, [r3, #-4]
    3d08:	e503a008 	str	sl, [r3, #-8]
    3d0c:	e2a19000 	adc	r9, r1, #0
    3d10:	e5039004 	str	r9, [r3, #-4]
	si->connection_flushes64 += si->connection_flushes;
    3d14:	e794c002 	ldr	ip, [r4, r2]
    3d18:	e5901048 	ldr	r1, [r0, #72]	@ 0x48
    3d1c:	e091100c 	adds	r1, r1, ip
    3d20:	e1a0c001 	mov	ip, r1
    3d24:	e5931004 	ldr	r1, [r3, #4]
    3d28:	e784c002 	str	ip, [r4, r2]
    3d2c:	e50bc068 	str	ip, [fp, #-104]	@ 0xffffff98
	si->connection_create_requests64 += si->connection_create_requests;
    3d30:	e3a0c000 	mov	ip, #0
	si->connection_flushes64 += si->connection_flushes;
    3d34:	e2a12000 	adc	r2, r1, #0
    3d38:	e2841c82 	add	r1, r4, #33280	@ 0x8200
    3d3c:	e2811060 	add	r1, r1, #96	@ 0x60
    3d40:	e50b206c 	str	r2, [fp, #-108]	@ 0xffffff94
    3d44:	e5832004 	str	r2, [r3, #4]
	si->packets_forwarded64 += si->packets_forwarded;
    3d48:	e51e2008 	ldr	r2, [lr, #-8]
    3d4c:	e590304c 	ldr	r3, [r0, #76]	@ 0x4c
    3d50:	e0933002 	adds	r3, r3, r2
	si->connection_create_requests = 0;
    3d54:	e3082030 	movw	r2, #32816	@ 0x8030
	si->packets_forwarded64 += si->packets_forwarded;
    3d58:	e50b3070 	str	r3, [fp, #-112]	@ 0xffffff90
    3d5c:	e50e3008 	str	r3, [lr, #-8]
    3d60:	e51e3004 	ldr	r3, [lr, #-4]
    3d64:	e2a33000 	adc	r3, r3, #0
    3d68:	e50b3074 	str	r3, [fp, #-116]	@ 0xffffff8c
    3d6c:	e50e3004 	str	r3, [lr, #-4]
	si->connection_create_requests = 0;
    3d70:	e2843902 	add	r3, r4, #32768	@ 0x8000
    3d74:	e2833040 	add	r3, r3, #64	@ 0x40
    3d78:	e18460f2 	strd	r6, [r4, r2]
    3d7c:	e14360f8 	strd	r6, [r3, #-8]
    3d80:	e3083040 	movw	r3, #32832	@ 0x8040
    3d84:	e18460f3 	strd	r6, [r4, r3]
	si->packets_not_forwarded = 0;
    3d88:	e1a0300e 	mov	r3, lr
	si->connection_create_requests = 0;
    3d8c:	e14560f8 	strd	r6, [r5, #-8]
	si->packets_not_forwarded64 += si->packets_not_forwarded;
    3d90:	e7946008 	ldr	r6, [r4, r8]
    3d94:	e5905050 	ldr	r5, [r0, #80]	@ 0x50
    3d98:	e0955006 	adds	r5, r5, r6
    3d9c:	e59e6004 	ldr	r6, [lr, #4]
    3da0:	e7845008 	str	r5, [r4, r8]
    3da4:	e2a66000 	adc	r6, r6, #0
    3da8:	e58e6004 	str	r6, [lr, #4]
	si->packets_not_forwarded = 0;
    3dac:	e580c050 	str	ip, [r0, #80]	@ 0x50
    3db0:	e51b2034 	ldr	r2, [fp, #-52]	@ 0xffffffcc
		si->exception_events64[i] += si->exception_events[i];
    3db4:	e5b2e004 	ldr	lr, [r2, #4]!
    3db8:	e5b30008 	ldr	r0, [r3, #8]!
    3dbc:	e090000e 	adds	r0, r0, lr
    3dc0:	e5830000 	str	r0, [r3]
    3dc4:	e5930004 	ldr	r0, [r3, #4]
    3dc8:	e2a00000 	adc	r0, r0, #0
	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
    3dcc:	e1510003 	cmp	r1, r3
		si->exception_events64[i] += si->exception_events[i];
    3dd0:	e5830004 	str	r0, [r3, #4]
		si->exception_events[i] = 0;
    3dd4:	e582c000 	str	ip, [r2]
	for (i = 0; i < SFE_IPV6_EXCEPTION_EVENT_LAST; i++) {
    3dd8:	1afffff5 	bne	3db4 <sfe_ipv6_debug_dev_read_stats+0x1d8>
	raw_spin_unlock_bh(&lock->rlock);
    3ddc:	e1a00004 	mov	r0, r4
	num_connections = si->num_connections;
    3de0:	e5944014 	ldr	r4, [r4, #20]
    3de4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<stats "
    3de8:	e51b3070 	ldr	r3, [fp, #-112]	@ 0xffffff90
    3dec:	e3002000 	movw	r2, #0
    3df0:	e3a01c03 	mov	r1, #768	@ 0x300
    3df4:	e3402000 	movt	r2, #0
    3df8:	e58d3000 	str	r3, [sp]
    3dfc:	e51b3074 	ldr	r3, [fp, #-116]	@ 0xffffff8c
    3e00:	e98d0068 	stmib	sp, {r3, r5, r6}
    3e04:	e51b3040 	ldr	r3, [fp, #-64]	@ 0xffffffc0
    3e08:	e58d3010 	str	r3, [sp, #16]
    3e0c:	e51b3044 	ldr	r3, [fp, #-68]	@ 0xffffffbc
    3e10:	e58d3014 	str	r3, [sp, #20]
    3e14:	e51b3048 	ldr	r3, [fp, #-72]	@ 0xffffffb8
    3e18:	e58d3018 	str	r3, [sp, #24]
    3e1c:	e51b304c 	ldr	r3, [fp, #-76]	@ 0xffffffb4
    3e20:	e58d301c 	str	r3, [sp, #28]
    3e24:	e51b3050 	ldr	r3, [fp, #-80]	@ 0xffffffb0
    3e28:	e58d3020 	str	r3, [sp, #32]
    3e2c:	e51b3054 	ldr	r3, [fp, #-84]	@ 0xffffffac
    3e30:	e58d3024 	str	r3, [sp, #36]	@ 0x24
    3e34:	e51b3058 	ldr	r3, [fp, #-88]	@ 0xffffffa8
    3e38:	e58d3028 	str	r3, [sp, #40]	@ 0x28
    3e3c:	e51b305c 	ldr	r3, [fp, #-92]	@ 0xffffffa4
    3e40:	e58d302c 	str	r3, [sp, #44]	@ 0x2c
    3e44:	e51b3068 	ldr	r3, [fp, #-104]	@ 0xffffff98
    3e48:	e58d3030 	str	r3, [sp, #48]	@ 0x30
    3e4c:	e51b306c 	ldr	r3, [fp, #-108]	@ 0xffffff94
    3e50:	e58d3034 	str	r3, [sp, #52]	@ 0x34
    3e54:	e51b3060 	ldr	r3, [fp, #-96]	@ 0xffffffa0
    3e58:	e58d3038 	str	r3, [sp, #56]	@ 0x38
    3e5c:	e51b3064 	ldr	r3, [fp, #-100]	@ 0xffffff9c
    3e60:	e58d303c 	str	r3, [sp, #60]	@ 0x3c
    3e64:	e1a03004 	mov	r3, r4
    3e68:	e58da040 	str	sl, [sp, #64]	@ 0x40
    3e6c:	e58d9044 	str	r9, [sp, #68]	@ 0x44
    3e70:	e51b0030 	ldr	r0, [fp, #-48]	@ 0xffffffd0
    3e74:	ebfffffe 	bl	0 <snprintf>
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3e78:	e51b203c 	ldr	r2, [fp, #-60]	@ 0xffffffc4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<stats "
    3e7c:	e1a04000 	mov	r4, r0
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3e80:	e59b3004 	ldr	r3, [fp, #4]
    3e84:	e5933000 	ldr	r3, [r3]
    3e88:	e0820003 	add	r0, r2, r3
    3e8c:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    3e90:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    3e94:	e1500003 	cmp	r0, r3
    3e98:	9a000002 	bls	3ea8 <sfe_ipv6_debug_dev_read_stats+0x2cc>
		return false;
    3e9c:	e3a00000 	mov	r0, #0
}
    3ea0:	e24bd028 	sub	sp, fp, #40	@ 0x28
    3ea4:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
    3ea8:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    3eac:	ee135f10 	mrc	15, 0, r5, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    3eb0:	e3c5300c 	bic	r3, r5, #12
    3eb4:	e3833004 	orr	r3, r3, #4
	asm volatile(
    3eb8:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    3ebc:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    3ec0:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
    3ec4:	e3a02c03 	mov	r2, #768	@ 0x300
    3ec8:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    3ecc:	ee035f10 	mcr	15, 0, r5, cr3, cr0, {0}
	isb();
    3ed0:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    3ed4:	e3500000 	cmp	r0, #0
    3ed8:	1affffef 	bne	3e9c <sfe_ipv6_debug_dev_read_stats+0x2c0>
	*length -= bytes_read;
    3edc:	e51b2078 	ldr	r2, [fp, #-120]	@ 0xffffff88
	return true;
    3ee0:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    3ee4:	e5923000 	ldr	r3, [r2]
    3ee8:	e0433004 	sub	r3, r3, r4
    3eec:	e5823000 	str	r3, [r2]
	*total_read += bytes_read;
    3ef0:	e59b3004 	ldr	r3, [fp, #4]
    3ef4:	e59b2004 	ldr	r2, [fp, #4]
    3ef8:	e5933000 	ldr	r3, [r3]
    3efc:	e0833004 	add	r3, r3, r4
    3f00:	e5823000 	str	r3, [r2]
	ws->state++;
    3f04:	e59b3008 	ldr	r3, [fp, #8]
    3f08:	e59b2008 	ldr	r2, [fp, #8]
    3f0c:	e5933000 	ldr	r3, [r3]
    3f10:	e0833000 	add	r3, r3, r0
    3f14:	e5823000 	str	r3, [r2]
}
    3f18:	e24bd028 	sub	sp, fp, #40	@ 0x28
    3f1c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

00003f20 <sfe_ipv4_debug_dev_read_stats>:
{
    3f20:	e1a0c00d 	mov	ip, sp
    3f24:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    3f28:	e24cb004 	sub	fp, ip, #4
    3f2c:	e24dd094 	sub	sp, sp, #148	@ 0x94
    3f30:	e1a04000 	mov	r4, r0
    3f34:	e50b3074 	str	r3, [fp, #-116]	@ 0xffffff8c
	si->connection_create_requests = 0;
    3f38:	e3a06000 	mov	r6, #0
{
    3f3c:	e50b103c 	str	r1, [fp, #-60]	@ 0xffffffc4
	si->connection_create_requests = 0;
    3f40:	e3a07000 	mov	r7, #0
{
    3f44:	e50b2030 	str	r2, [fp, #-48]	@ 0xffffffd0
	raw_spin_lock_bh(&lock->rlock);
    3f48:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_create_requests64 += si->connection_create_requests;
    3f4c:	e30810f0 	movw	r1, #33008	@ 0x80f0
    3f50:	e2840902 	add	r0, r4, #32768	@ 0x8000
    3f54:	e0842001 	add	r2, r4, r1
    3f58:	e590c030 	ldr	ip, [r0, #48]	@ 0x30
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3f5c:	e2843c81 	add	r3, r4, #33024	@ 0x8100
	si->connection_create_requests64 += si->connection_create_requests;
    3f60:	e512e008 	ldr	lr, [r2, #-8]
	si->connection_create_requests = 0;
    3f64:	e2808050 	add	r8, r0, #80	@ 0x50
	si->connection_create_requests64 += si->connection_create_requests;
    3f68:	e09cc00e 	adds	ip, ip, lr
    3f6c:	e50bc040 	str	ip, [fp, #-64]	@ 0xffffffc0
    3f70:	e502c008 	str	ip, [r2, #-8]
    3f74:	e512c004 	ldr	ip, [r2, #-4]
    3f78:	e2acc000 	adc	ip, ip, #0
    3f7c:	e50bc044 	str	ip, [fp, #-68]	@ 0xffffffbc
    3f80:	e502c004 	str	ip, [r2, #-4]
	si->connection_create_collisions64 += si->connection_create_collisions;
    3f84:	e794e001 	ldr	lr, [r4, r1]
    3f88:	e590c034 	ldr	ip, [r0, #52]	@ 0x34
    3f8c:	e09cc00e 	adds	ip, ip, lr
    3f90:	e1a0e00c 	mov	lr, ip
    3f94:	e592c004 	ldr	ip, [r2, #4]
    3f98:	e784e001 	str	lr, [r4, r1]
    3f9c:	e50be048 	str	lr, [fp, #-72]	@ 0xffffffb8
    3fa0:	e2ac1000 	adc	r1, ip, #0
    3fa4:	e50b104c 	str	r1, [fp, #-76]	@ 0xffffffb4
    3fa8:	e5821004 	str	r1, [r2, #4]
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3fac:	e5131008 	ldr	r1, [r3, #-8]
    3fb0:	e5902038 	ldr	r2, [r0, #56]	@ 0x38
    3fb4:	e0922001 	adds	r2, r2, r1
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    3fb8:	e5931000 	ldr	r1, [r3]
	si->connection_destroy_requests64 += si->connection_destroy_requests;
    3fbc:	e50b2050 	str	r2, [fp, #-80]	@ 0xffffffb0
    3fc0:	e5032008 	str	r2, [r3, #-8]
    3fc4:	e5132004 	ldr	r2, [r3, #-4]
    3fc8:	e2a22000 	adc	r2, r2, #0
    3fcc:	e50b2054 	str	r2, [fp, #-84]	@ 0xffffffac
    3fd0:	e5032004 	str	r2, [r3, #-4]
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    3fd4:	e590203c 	ldr	r2, [r0, #60]	@ 0x3c
    3fd8:	e0922001 	adds	r2, r2, r1
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
    3fdc:	e3081110 	movw	r1, #33040	@ 0x8110
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    3fe0:	e50b2058 	str	r2, [fp, #-88]	@ 0xffffffa8
    3fe4:	e5832000 	str	r2, [r3]
    3fe8:	e5932004 	ldr	r2, [r3, #4]
    3fec:	e2a22000 	adc	r2, r2, #0
    3ff0:	e5832004 	str	r2, [r3, #4]
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
    3ff4:	e0843001 	add	r3, r4, r1
    3ff8:	e590c040 	ldr	ip, [r0, #64]	@ 0x40
    3ffc:	e513e008 	ldr	lr, [r3, #-8]
	si->connection_destroy_misses64 += si->connection_destroy_misses;
    4000:	e50b205c 	str	r2, [fp, #-92]	@ 0xffffffa4
	si->connection_flushes64 += si->connection_flushes;
    4004:	e3082120 	movw	r2, #33056	@ 0x8120
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
    4008:	e09cc00e 	adds	ip, ip, lr
    400c:	e50bc060 	str	ip, [fp, #-96]	@ 0xffffffa0
    4010:	e503c008 	str	ip, [r3, #-8]
    4014:	e513c004 	ldr	ip, [r3, #-4]
    4018:	e50b8034 	str	r8, [fp, #-52]	@ 0xffffffcc
    401c:	e2acc000 	adc	ip, ip, #0
    4020:	e50bc064 	str	ip, [fp, #-100]	@ 0xffffff9c
    4024:	e503c004 	str	ip, [r3, #-4]
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
    4028:	e794e001 	ldr	lr, [r4, r1]
    402c:	e590c044 	ldr	ip, [r0, #68]	@ 0x44
    4030:	e09ca00e 	adds	sl, ip, lr
    4034:	e593c004 	ldr	ip, [r3, #4]
	si->packets_not_forwarded64 += si->packets_not_forwarded;
    4038:	e284ec81 	add	lr, r4, #33024	@ 0x8100
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
    403c:	e784a001 	str	sl, [r4, r1]
    4040:	e2ac9000 	adc	r9, ip, #0
    4044:	e5839004 	str	r9, [r3, #4]
	si->connection_flushes64 += si->connection_flushes;
    4048:	e0843002 	add	r3, r4, r2
    404c:	e5901048 	ldr	r1, [r0, #72]	@ 0x48
    4050:	e513c008 	ldr	ip, [r3, #-8]
	si->packets_forwarded64 += si->packets_forwarded;
    4054:	e5935004 	ldr	r5, [r3, #4]
	si->connection_flushes64 += si->connection_flushes;
    4058:	e091100c 	adds	r1, r1, ip
    405c:	e1a0c001 	mov	ip, r1
    4060:	e5131004 	ldr	r1, [r3, #-4]
    4064:	e50bc068 	str	ip, [fp, #-104]	@ 0xffffff98
    4068:	e503c008 	str	ip, [r3, #-8]
    406c:	e2a11000 	adc	r1, r1, #0
    4070:	e50b106c 	str	r1, [fp, #-108]	@ 0xffffff94
    4074:	e5031004 	str	r1, [r3, #-4]
	si->packets_forwarded64 += si->packets_forwarded;
    4078:	e794c002 	ldr	ip, [r4, r2]
    407c:	e590104c 	ldr	r1, [r0, #76]	@ 0x4c
    4080:	e091100c 	adds	r1, r1, ip
	si->connection_create_requests64 += si->connection_create_requests;
    4084:	e3a0c000 	mov	ip, #0
	si->packets_forwarded64 += si->packets_forwarded;
    4088:	e2a55000 	adc	r5, r5, #0
    408c:	e7841002 	str	r1, [r4, r2]
    4090:	e28e2028 	add	r2, lr, #40	@ 0x28
    4094:	e5835004 	str	r5, [r3, #4]
	si->connection_create_requests = 0;
    4098:	e2843902 	add	r3, r4, #32768	@ 0x8000
    409c:	e2833040 	add	r3, r3, #64	@ 0x40
	si->packets_forwarded64 += si->packets_forwarded;
    40a0:	e50b1070 	str	r1, [fp, #-112]	@ 0xffffff90
    40a4:	e2841c82 	add	r1, r4, #33280	@ 0x8200
    40a8:	e50b2038 	str	r2, [fp, #-56]	@ 0xffffffc8
	si->connection_create_requests = 0;
    40ac:	e3082030 	movw	r2, #32816	@ 0x8030
    40b0:	e2811050 	add	r1, r1, #80	@ 0x50
    40b4:	e18460f2 	strd	r6, [r4, r2]
	si->packets_not_forwarded = 0;
    40b8:	e1a02008 	mov	r2, r8
	si->connection_create_requests = 0;
    40bc:	e14360f8 	strd	r6, [r3, #-8]
    40c0:	e3083040 	movw	r3, #32832	@ 0x8040
    40c4:	e18460f3 	strd	r6, [r4, r3]
    40c8:	e14860f8 	strd	r6, [r8, #-8]
	si->packets_not_forwarded64 += si->packets_not_forwarded;
    40cc:	e59e7028 	ldr	r7, [lr, #40]	@ 0x28
    40d0:	e5906050 	ldr	r6, [r0, #80]	@ 0x50
    40d4:	e0966007 	adds	r6, r6, r7
    40d8:	e59e702c 	ldr	r7, [lr, #44]	@ 0x2c
    40dc:	e2a77000 	adc	r7, r7, #0
    40e0:	e1ce62f8 	strd	r6, [lr, #40]	@ 0x28
	si->packets_not_forwarded = 0;
    40e4:	e580c050 	str	ip, [r0, #80]	@ 0x50
    40e8:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
		si->exception_events64[i] += si->exception_events[i];
    40ec:	e5b2e004 	ldr	lr, [r2, #4]!
    40f0:	e5b30008 	ldr	r0, [r3, #8]!
    40f4:	e090000e 	adds	r0, r0, lr
    40f8:	e5830000 	str	r0, [r3]
    40fc:	e5930004 	ldr	r0, [r3, #4]
    4100:	e2a00000 	adc	r0, r0, #0
	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
    4104:	e1510003 	cmp	r1, r3
		si->exception_events64[i] += si->exception_events[i];
    4108:	e5830004 	str	r0, [r3, #4]
		si->exception_events[i] = 0;
    410c:	e582c000 	str	ip, [r2]
	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
    4110:	1afffff5 	bne	40ec <sfe_ipv4_debug_dev_read_stats+0x1cc>
	raw_spin_unlock_bh(&lock->rlock);
    4114:	e1a00004 	mov	r0, r4
	num_connections = si->num_connections;
    4118:	e5944014 	ldr	r4, [r4, #20]
    411c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<stats "
    4120:	e51b3070 	ldr	r3, [fp, #-112]	@ 0xffffff90
    4124:	e3002000 	movw	r2, #0
    4128:	e3a01c03 	mov	r1, #768	@ 0x300
    412c:	e3402000 	movt	r2, #0
    4130:	e88d00e8 	stm	sp, {r3, r5, r6, r7}
    4134:	e51b3040 	ldr	r3, [fp, #-64]	@ 0xffffffc0
    4138:	e58d3010 	str	r3, [sp, #16]
    413c:	e51b3044 	ldr	r3, [fp, #-68]	@ 0xffffffbc
    4140:	e58d3014 	str	r3, [sp, #20]
    4144:	e51b3048 	ldr	r3, [fp, #-72]	@ 0xffffffb8
    4148:	e58d3018 	str	r3, [sp, #24]
    414c:	e51b304c 	ldr	r3, [fp, #-76]	@ 0xffffffb4
    4150:	e58d301c 	str	r3, [sp, #28]
    4154:	e51b3050 	ldr	r3, [fp, #-80]	@ 0xffffffb0
    4158:	e58d3020 	str	r3, [sp, #32]
    415c:	e51b3054 	ldr	r3, [fp, #-84]	@ 0xffffffac
    4160:	e58d3024 	str	r3, [sp, #36]	@ 0x24
    4164:	e51b3058 	ldr	r3, [fp, #-88]	@ 0xffffffa8
    4168:	e58d3028 	str	r3, [sp, #40]	@ 0x28
    416c:	e51b305c 	ldr	r3, [fp, #-92]	@ 0xffffffa4
    4170:	e58d302c 	str	r3, [sp, #44]	@ 0x2c
    4174:	e51b3068 	ldr	r3, [fp, #-104]	@ 0xffffff98
    4178:	e58d3030 	str	r3, [sp, #48]	@ 0x30
    417c:	e51b306c 	ldr	r3, [fp, #-108]	@ 0xffffff94
    4180:	e58d3034 	str	r3, [sp, #52]	@ 0x34
    4184:	e51b3060 	ldr	r3, [fp, #-96]	@ 0xffffffa0
    4188:	e58d3038 	str	r3, [sp, #56]	@ 0x38
    418c:	e51b3064 	ldr	r3, [fp, #-100]	@ 0xffffff9c
    4190:	e58d303c 	str	r3, [sp, #60]	@ 0x3c
    4194:	e1a03004 	mov	r3, r4
    4198:	e58da040 	str	sl, [sp, #64]	@ 0x40
    419c:	e58d9044 	str	r9, [sp, #68]	@ 0x44
    41a0:	e51b0030 	ldr	r0, [fp, #-48]	@ 0xffffffd0
    41a4:	ebfffffe 	bl	0 <snprintf>
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    41a8:	e51b203c 	ldr	r2, [fp, #-60]	@ 0xffffffc4
	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<stats "
    41ac:	e1a04000 	mov	r4, r0
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    41b0:	e59b3004 	ldr	r3, [fp, #4]
    41b4:	e5933000 	ldr	r3, [r3]
    41b8:	e0820003 	add	r0, r2, r3
    41bc:	e3a03cfd 	mov	r3, #64768	@ 0xfd00
    41c0:	e34b3eff 	movt	r3, #48895	@ 0xbeff
    41c4:	e1500003 	cmp	r0, r3
    41c8:	9a000002 	bls	41d8 <sfe_ipv4_debug_dev_read_stats+0x2b8>
		return false;
    41cc:	e3a00000 	mov	r0, #0
}
    41d0:	e24bd028 	sub	sp, fp, #40	@ 0x28
    41d4:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
    41d8:	ee1d3f70 	mrc	15, 0, r3, cr13, cr0, {3}
	asm(
    41dc:	ee135f10 	mrc	15, 0, r5, cr3, cr0, {0}
	set_domain((old_domain & ~domain_mask(DOMAIN_USER)) |
    41e0:	e3c5300c 	bic	r3, r5, #12
    41e4:	e3833004 	orr	r3, r3, #4
	asm volatile(
    41e8:	ee033f10 	mcr	15, 0, r3, cr3, cr0, {0}
	isb();
    41ec:	f57ff06f 	isb	sy
	n = arm_copy_to_user(to, from, n);
    41f0:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
    41f4:	e3a02c03 	mov	r2, #768	@ 0x300
    41f8:	ebfffffe 	bl	0 <arm_copy_to_user>
	asm volatile(
    41fc:	ee035f10 	mcr	15, 0, r5, cr3, cr0, {0}
	isb();
    4200:	f57ff06f 	isb	sy
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
    4204:	e3500000 	cmp	r0, #0
    4208:	1affffef 	bne	41cc <sfe_ipv4_debug_dev_read_stats+0x2ac>
	*length -= bytes_read;
    420c:	e51b2074 	ldr	r2, [fp, #-116]	@ 0xffffff8c
	return true;
    4210:	e3a00001 	mov	r0, #1
	*length -= bytes_read;
    4214:	e5923000 	ldr	r3, [r2]
    4218:	e0433004 	sub	r3, r3, r4
    421c:	e5823000 	str	r3, [r2]
	*total_read += bytes_read;
    4220:	e59b3004 	ldr	r3, [fp, #4]
    4224:	e59b2004 	ldr	r2, [fp, #4]
    4228:	e5933000 	ldr	r3, [r3]
    422c:	e0833004 	add	r3, r3, r4
    4230:	e5823000 	str	r3, [r2]
	ws->state++;
    4234:	e59b3008 	ldr	r3, [fp, #8]
    4238:	e59b2008 	ldr	r2, [fp, #8]
    423c:	e5933000 	ldr	r3, [r3]
    4240:	e0833000 	add	r3, r3, r0
    4244:	e5823000 	str	r3, [r2]
}
    4248:	e24bd028 	sub	sp, fp, #40	@ 0x28
    424c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

00004250 <fast_classifier_sb_find_conn>:
{
    4250:	e1a0c00d 	mov	ip, sp
    4254:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    4258:	e24cb004 	sub	fp, ip, #4
    425c:	e24dd01c 	sub	sp, sp, #28
    4260:	e5dbc008 	ldrb	ip, [fp, #8]
    4264:	e1a05000 	mov	r5, r0
	return hash ^ (sport | (dport << 16));
    4268:	e1820803 	orr	r0, r2, r3, lsl #16
	u32 hash = saddr->ip ^ daddr->ip;
    426c:	e5918000 	ldr	r8, [r1]
    4270:	e5957000 	ldr	r7, [r5]
	if (is_v4)
    4274:	e35c0000 	cmp	ip, #0
{
    4278:	e5dbc004 	ldrb	ip, [fp, #4]
	if (is_v4)
    427c:	0a000045 	beq	4398 <fast_classifier_sb_find_conn+0x148>
	return hash ^ (sport | (dport << 16));
    4280:	e0271000 	eor	r1, r7, r0
	return val * GOLDEN_RATIO_32;
    4284:	e3080647 	movw	r0, #34375	@ 0x8647
    4288:	e34601c8 	movt	r0, #25032	@ 0x61c8
    428c:	e0211008 	eor	r1, r1, r8
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    4290:	e3006000 	movw	r6, #0
    4294:	e0010190 	mul	r1, r0, r1
    4298:	e3406000 	movt	r6, #0
	return __hash_32(val) >> (32 - bits);
    429c:	e1a019a1 	lsr	r1, r1, #19
    42a0:	e7960101 	ldr	r0, [r6, r1, lsl #2]
    42a4:	e3500000 	cmp	r0, #0
    42a8:	1a000028 	bne	4350 <fast_classifier_sb_find_conn+0x100>
	return hash ^ (sport | (dport << 16));
    42ac:	e1831802 	orr	r1, r3, r2, lsl #16
	u32 hash = saddr->ip ^ daddr->ip;
    42b0:	e0270008 	eor	r0, r7, r8
	return hash ^ (sport | (dport << 16));
    42b4:	e0211000 	eor	r1, r1, r0
	return val * GOLDEN_RATIO_32;
    42b8:	e3080647 	movw	r0, #34375	@ 0x8647
    42bc:	e34601c8 	movt	r0, #25032	@ 0x61c8
    42c0:	e0010190 	mul	r1, r0, r1
	return __hash_32(val) >> (32 - bits);
    42c4:	e1a019a1 	lsr	r1, r1, #19
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    42c8:	e7960101 	ldr	r0, [r6, r1, lsl #2]
    42cc:	e3500000 	cmp	r0, #0
    42d0:	1a000003 	bne	42e4 <fast_classifier_sb_find_conn+0x94>
    42d4:	ea000015 	b	4330 <fast_classifier_sb_find_conn+0xe0>
    42d8:	e5900000 	ldr	r0, [r0]
    42dc:	e3500000 	cmp	r0, #0
    42e0:	0a000012 	beq	4330 <fast_classifier_sb_find_conn+0xe0>
		if (conn->is_v4 != is_v4) {
    42e4:	e5d0101c 	ldrb	r1, [r0, #28]
    42e8:	e3510000 	cmp	r1, #0
    42ec:	0afffff9 	beq	42d8 <fast_classifier_sb_find_conn+0x88>
		p_sic = conn->sic;
    42f0:	e5901008 	ldr	r1, [r0, #8]
		if (p_sic->protocol == proto &&
    42f4:	e591e000 	ldr	lr, [r1]
    42f8:	e15e000c 	cmp	lr, ip
    42fc:	1afffff5 	bne	42d8 <fast_classifier_sb_find_conn+0x88>
    4300:	e1d1e5b8 	ldrh	lr, [r1, #88]	@ 0x58
    4304:	e15e0003 	cmp	lr, r3
    4308:	1afffff2 	bne	42d8 <fast_classifier_sb_find_conn+0x88>
		    p_sic->src_port == dport &&
    430c:	e1d1e5be 	ldrh	lr, [r1, #94]	@ 0x5e
    4310:	e15e0002 	cmp	lr, r2
    4314:	1affffef 	bne	42d8 <fast_classifier_sb_find_conn+0x88>
		    p_sic->dest_port_xlate == sport &&
    4318:	e591e018 	ldr	lr, [r1, #24]
    431c:	e15e0008 	cmp	lr, r8
    4320:	1affffec 	bne	42d8 <fast_classifier_sb_find_conn+0x88>
		    sfe_addr_equal(&p_sic->src_ip, daddr, is_v4) &&
    4324:	e5911048 	ldr	r1, [r1, #72]	@ 0x48
    4328:	e1510007 	cmp	r1, r7
    432c:	1affffe9 	bne	42d8 <fast_classifier_sb_find_conn+0x88>
}
    4330:	e24bd028 	sub	sp, fp, #40	@ 0x28
    4334:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		if (p_sic->protocol == proto &&
    4338:	e1d1e5b8 	ldrh	lr, [r1, #88]	@ 0x58
    433c:	e15e0002 	cmp	lr, r2
    4340:	0a00000a 	beq	4370 <fast_classifier_sb_find_conn+0x120>
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    4344:	e5900000 	ldr	r0, [r0]
    4348:	e3500000 	cmp	r0, #0
    434c:	0affffd6 	beq	42ac <fast_classifier_sb_find_conn+0x5c>
		if (conn->is_v4 != is_v4) {
    4350:	e5d0101c 	ldrb	r1, [r0, #28]
    4354:	e3510000 	cmp	r1, #0
    4358:	0afffff9 	beq	4344 <fast_classifier_sb_find_conn+0xf4>
		p_sic = conn->sic;
    435c:	e5901008 	ldr	r1, [r0, #8]
		if (p_sic->protocol == proto &&
    4360:	e591e000 	ldr	lr, [r1]
    4364:	e15e000c 	cmp	lr, ip
    4368:	1afffff5 	bne	4344 <fast_classifier_sb_find_conn+0xf4>
    436c:	eafffff1 	b	4338 <fast_classifier_sb_find_conn+0xe8>
		    p_sic->src_port == sport &&
    4370:	e1d1e5be 	ldrh	lr, [r1, #94]	@ 0x5e
    4374:	e15e0003 	cmp	lr, r3
    4378:	1afffff1 	bne	4344 <fast_classifier_sb_find_conn+0xf4>
		    p_sic->dest_port_xlate == dport &&
    437c:	e591e018 	ldr	lr, [r1, #24]
    4380:	e15e0007 	cmp	lr, r7
    4384:	1affffee 	bne	4344 <fast_classifier_sb_find_conn+0xf4>
		    sfe_addr_equal(&p_sic->src_ip, saddr, is_v4) &&
    4388:	e5911048 	ldr	r1, [r1, #72]	@ 0x48
    438c:	e1510008 	cmp	r1, r8
    4390:	1affffeb 	bne	4344 <fast_classifier_sb_find_conn+0xf4>
    4394:	eaffffe5 	b	4330 <fast_classifier_sb_find_conn+0xe0>
	hash ^= saddr->ip6[0].addr[3] ^ daddr->ip6[0].addr[3];
    4398:	e9914210 	ldmib	r1, {r4, r9, lr}
    439c:	e5956004 	ldr	r6, [r5, #4]
    43a0:	e1a0a00e 	mov	sl, lr
    43a4:	e595e008 	ldr	lr, [r5, #8]
    43a8:	e50b6030 	str	r6, [fp, #-48]	@ 0xffffffd0
    43ac:	e595600c 	ldr	r6, [r5, #12]
    43b0:	e50ba040 	str	sl, [fp, #-64]	@ 0xffffffc0
    43b4:	e50be034 	str	lr, [fp, #-52]	@ 0xffffffcc
	return val * GOLDEN_RATIO_32;
    43b8:	e308e647 	movw	lr, #34375	@ 0x8647
    43bc:	e346e1c8 	movt	lr, #25032	@ 0x61c8
    43c0:	e50be03c 	str	lr, [fp, #-60]	@ 0xffffffc4
    43c4:	e028e004 	eor	lr, r8, r4
    43c8:	e02ee009 	eor	lr, lr, r9
    43cc:	e50b6038 	str	r6, [fp, #-56]	@ 0xffffffc8
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    43d0:	e3006000 	movw	r6, #0
	hash ^= saddr->ip6[0].addr[3] ^ daddr->ip6[0].addr[3];
    43d4:	e02ee00a 	eor	lr, lr, sl
    43d8:	e51ba030 	ldr	sl, [fp, #-48]	@ 0xffffffd0
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    43dc:	e3406000 	movt	r6, #0
	hash ^= saddr->ip6[0].addr[3] ^ daddr->ip6[0].addr[3];
    43e0:	e02ee007 	eor	lr, lr, r7
    43e4:	e02ee00a 	eor	lr, lr, sl
    43e8:	e51ba034 	ldr	sl, [fp, #-52]	@ 0xffffffcc
    43ec:	e02ee00a 	eor	lr, lr, sl
    43f0:	e51ba038 	ldr	sl, [fp, #-56]	@ 0xffffffc8
    43f4:	e02ee00a 	eor	lr, lr, sl
    43f8:	e51ba03c 	ldr	sl, [fp, #-60]	@ 0xffffffc4
	return hash ^ (sport | (dport << 16));
    43fc:	e020000e 	eor	r0, r0, lr
    4400:	e000009a 	mul	r0, sl, r0
	return __hash_32(val) >> (32 - bits);
    4404:	e1a009a0 	lsr	r0, r0, #19
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    4408:	e7960100 	ldr	r0, [r6, r0, lsl #2]
    440c:	e3500000 	cmp	r0, #0
    4410:	0a00003e 	beq	4510 <fast_classifier_sb_find_conn+0x2c0>
    4414:	e1a0a004 	mov	sl, r4
    4418:	e50b603c 	str	r6, [fp, #-60]	@ 0xffffffc4
    441c:	ea000002 	b	442c <fast_classifier_sb_find_conn+0x1dc>
    4420:	e5900000 	ldr	r0, [r0]
    4424:	e3500000 	cmp	r0, #0
    4428:	0a00002b 	beq	44dc <fast_classifier_sb_find_conn+0x28c>
		if (conn->is_v4 != is_v4) {
    442c:	e5d0e01c 	ldrb	lr, [r0, #28]
    4430:	e35e0000 	cmp	lr, #0
    4434:	1afffff9 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
		p_sic = conn->sic;
    4438:	e590e008 	ldr	lr, [r0, #8]
		if (p_sic->protocol == proto &&
    443c:	e59e4000 	ldr	r4, [lr]
    4440:	e15c0004 	cmp	ip, r4
    4444:	1afffff5 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
    4448:	e1de45b8 	ldrh	r4, [lr, #88]	@ 0x58
    444c:	e1540002 	cmp	r4, r2
    4450:	1afffff2 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
		    p_sic->src_port == sport &&
    4454:	e1de45be 	ldrh	r4, [lr, #94]	@ 0x5e
    4458:	e1540003 	cmp	r4, r3
    445c:	1affffef 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
    4460:	e59e4018 	ldr	r4, [lr, #24]
    4464:	e1540007 	cmp	r4, r7
    4468:	1affffec 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
	return a->addr[0] == b->addr[0] &&
    446c:	e5956004 	ldr	r6, [r5, #4]
    4470:	e59e401c 	ldr	r4, [lr, #28]
    4474:	e1560004 	cmp	r6, r4
    4478:	1affffe8 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
	       a->addr[1] == b->addr[1] &&
    447c:	e5956008 	ldr	r6, [r5, #8]
    4480:	e59e4020 	ldr	r4, [lr, #32]
    4484:	e1560004 	cmp	r6, r4
    4488:	1affffe4 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
	       a->addr[2] == b->addr[2] &&
    448c:	e595600c 	ldr	r6, [r5, #12]
    4490:	e59e4024 	ldr	r4, [lr, #36]	@ 0x24
    4494:	e1560004 	cmp	r6, r4
    4498:	1affffe0 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
    449c:	e59e4048 	ldr	r4, [lr, #72]	@ 0x48
    44a0:	e1540008 	cmp	r4, r8
    44a4:	1affffdd 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
	return a->addr[0] == b->addr[0] &&
    44a8:	e5916004 	ldr	r6, [r1, #4]
    44ac:	e59e404c 	ldr	r4, [lr, #76]	@ 0x4c
    44b0:	e1560004 	cmp	r6, r4
    44b4:	1affffd9 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
	       a->addr[1] == b->addr[1] &&
    44b8:	e5916008 	ldr	r6, [r1, #8]
    44bc:	e59e4050 	ldr	r4, [lr, #80]	@ 0x50
    44c0:	e1560004 	cmp	r6, r4
    44c4:	1affffd5 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
	       a->addr[2] == b->addr[2] &&
    44c8:	e591400c 	ldr	r4, [r1, #12]
    44cc:	e59ee054 	ldr	lr, [lr, #84]	@ 0x54
    44d0:	e154000e 	cmp	r4, lr
    44d4:	1affffd1 	bne	4420 <fast_classifier_sb_find_conn+0x1d0>
    44d8:	eaffff94 	b	4330 <fast_classifier_sb_find_conn+0xe0>
	hash ^= saddr->ip6[0].addr[3] ^ daddr->ip6[0].addr[3];
    44dc:	e51be040 	ldr	lr, [fp, #-64]	@ 0xffffffc0
    44e0:	e1a0400a 	mov	r4, sl
    44e4:	e0244008 	eor	r4, r4, r8
    44e8:	e51b603c 	ldr	r6, [fp, #-60]	@ 0xffffffc4
    44ec:	e0240009 	eor	r0, r4, r9
    44f0:	e020000e 	eor	r0, r0, lr
    44f4:	e51be030 	ldr	lr, [fp, #-48]	@ 0xffffffd0
    44f8:	e0200007 	eor	r0, r0, r7
    44fc:	e020000e 	eor	r0, r0, lr
    4500:	e51be034 	ldr	lr, [fp, #-52]	@ 0xffffffcc
    4504:	e020000e 	eor	r0, r0, lr
    4508:	e51be038 	ldr	lr, [fp, #-56]	@ 0xffffffc8
    450c:	e02ee000 	eor	lr, lr, r0
	return hash ^ (sport | (dport << 16));
    4510:	e1830802 	orr	r0, r3, r2, lsl #16
    4514:	e020000e 	eor	r0, r0, lr
	return val * GOLDEN_RATIO_32;
    4518:	e308e647 	movw	lr, #34375	@ 0x8647
    451c:	e346e1c8 	movt	lr, #25032	@ 0x61c8
    4520:	e000009e 	mul	r0, lr, r0
	return __hash_32(val) >> (32 - bits);
    4524:	e1a009a0 	lsr	r0, r0, #19
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    4528:	e7960100 	ldr	r0, [r6, r0, lsl #2]
    452c:	e3500000 	cmp	r0, #0
    4530:	1a000003 	bne	4544 <fast_classifier_sb_find_conn+0x2f4>
    4534:	eaffff7d 	b	4330 <fast_classifier_sb_find_conn+0xe0>
    4538:	e5900000 	ldr	r0, [r0]
    453c:	e3500000 	cmp	r0, #0
    4540:	0affff7a 	beq	4330 <fast_classifier_sb_find_conn+0xe0>
		if (conn->is_v4 != is_v4) {
    4544:	e5d0e01c 	ldrb	lr, [r0, #28]
    4548:	e35e0000 	cmp	lr, #0
    454c:	1afffff9 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
		p_sic = conn->sic;
    4550:	e590e008 	ldr	lr, [r0, #8]
		if (p_sic->protocol == proto &&
    4554:	e59e4000 	ldr	r4, [lr]
    4558:	e154000c 	cmp	r4, ip
    455c:	1afffff5 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
    4560:	e1de45b8 	ldrh	r4, [lr, #88]	@ 0x58
    4564:	e1540003 	cmp	r4, r3
    4568:	1afffff2 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
		    p_sic->src_port == dport &&
    456c:	e1de45be 	ldrh	r4, [lr, #94]	@ 0x5e
    4570:	e1540002 	cmp	r4, r2
    4574:	1affffef 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
    4578:	e59e4018 	ldr	r4, [lr, #24]
    457c:	e1540008 	cmp	r4, r8
    4580:	1affffec 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
	return a->addr[0] == b->addr[0] &&
    4584:	e5914004 	ldr	r4, [r1, #4]
    4588:	e59e601c 	ldr	r6, [lr, #28]
    458c:	e1560004 	cmp	r6, r4
    4590:	1affffe8 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
	       a->addr[1] == b->addr[1] &&
    4594:	e5916008 	ldr	r6, [r1, #8]
    4598:	e59e4020 	ldr	r4, [lr, #32]
    459c:	e1560004 	cmp	r6, r4
    45a0:	1affffe4 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
	       a->addr[2] == b->addr[2] &&
    45a4:	e591600c 	ldr	r6, [r1, #12]
    45a8:	e59e4024 	ldr	r4, [lr, #36]	@ 0x24
    45ac:	e1560004 	cmp	r6, r4
    45b0:	1affffe0 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
    45b4:	e59e4048 	ldr	r4, [lr, #72]	@ 0x48
    45b8:	e1540007 	cmp	r4, r7
    45bc:	1affffdd 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
	return a->addr[0] == b->addr[0] &&
    45c0:	e5954004 	ldr	r4, [r5, #4]
    45c4:	e59e604c 	ldr	r6, [lr, #76]	@ 0x4c
    45c8:	e1560004 	cmp	r6, r4
    45cc:	1affffd9 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
	       a->addr[1] == b->addr[1] &&
    45d0:	e5956008 	ldr	r6, [r5, #8]
    45d4:	e59e4050 	ldr	r4, [lr, #80]	@ 0x50
    45d8:	e1560004 	cmp	r6, r4
    45dc:	1affffd5 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
	       a->addr[2] == b->addr[2] &&
    45e0:	e59e4054 	ldr	r4, [lr, #84]	@ 0x54
    45e4:	e595e00c 	ldr	lr, [r5, #12]
    45e8:	e154000e 	cmp	r4, lr
    45ec:	1affffd1 	bne	4538 <fast_classifier_sb_find_conn+0x2e8>
    45f0:	eaffff4e 	b	4330 <fast_classifier_sb_find_conn+0xe0>

000045f4 <fast_classifier_offload_genl_msg>:
{
    45f4:	e1a0c00d 	mov	ip, sp
    45f8:	e92dd810 	push	{r4, fp, ip, lr, pc}
    45fc:	e24cb004 	sub	fp, ip, #4
    4600:	e24dd00c 	sub	sp, sp, #12
	na = info->attrs[FAST_CLASSIFIER_A_TUPLE];
    4604:	e5913014 	ldr	r3, [r1, #20]
	raw_spin_lock_bh(&lock->rlock);
    4608:	e59f00b8 	ldr	r0, [pc, #184]	@ 46c8 <fast_classifier_offload_genl_msg+0xd4>
    460c:	e5934004 	ldr	r4, [r3, #4]
    4610:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	if (fc_msg->ethertype == AF_INET)
    4614:	e1d430b4 	ldrh	r3, [r4, #4]
	conn = fast_classifier_sb_find_conn((sfe_ip_addr_t *)&fc_msg->src_saddr,
    4618:	e2841018 	add	r1, r4, #24
    461c:	e2840008 	add	r0, r4, #8
    4620:	e1d422b8 	ldrh	r2, [r4, #40]	@ 0x28
	if (fc_msg->ethertype == AF_INET)
    4624:	e3530002 	cmp	r3, #2
	conn = fast_classifier_sb_find_conn((sfe_ip_addr_t *)&fc_msg->src_saddr,
    4628:	e1d432ba 	ldrh	r3, [r4, #42]	@ 0x2a
	conn = fast_classifier_sb_find_conn((sfe_ip_addr_t *)&fc_msg->src_saddr,
    462c:	03a0c001 	moveq	ip, #1
	conn = fast_classifier_sb_find_conn((sfe_ip_addr_t *)&fc_msg->src_saddr,
    4630:	13a0c000 	movne	ip, #0
    4634:	e58dc004 	str	ip, [sp, #4]
    4638:	e5d4c006 	ldrb	ip, [r4, #6]
    463c:	e58dc000 	str	ip, [sp]
    4640:	ebffff02 	bl	4250 <fast_classifier_sb_find_conn>
	if (!conn) {
    4644:	e3500000 	cmp	r0, #0
	conn = fast_classifier_sb_find_conn((sfe_ip_addr_t *)&fc_msg->src_saddr,
    4648:	e1a03000 	mov	r3, r0
	if (!conn) {
    464c:	0a00000f 	beq	4690 <fast_classifier_offload_genl_msg+0x9c>
	conn->offload_permit = 1;
    4650:	e3a02001 	mov	r2, #1
	raw_spin_unlock_bh(&lock->rlock);
    4654:	e3004000 	movw	r4, #0
    4658:	e3404000 	movt	r4, #0
    465c:	e2840014 	add	r0, r4, #20
    4660:	e5832014 	str	r2, [r3, #20]
    4664:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    4668:	e2843018 	add	r3, r4, #24
    466c:	f593f000 	pldw	[r3]
ATOMIC_OPS(add, +=, add)
    4670:	e1932f9f 	ldrex	r2, [r3]
    4674:	e2822001 	add	r2, r2, #1
    4678:	e1831f92 	strex	r1, r2, [r3]
    467c:	e3310000 	teq	r1, #0
    4680:	1afffffa 	bne	4670 <fast_classifier_offload_genl_msg+0x7c>
}
    4684:	e3a00000 	mov	r0, #0
    4688:	e24bd010 	sub	sp, fp, #16
    468c:	e89da810 	ldm	sp, {r4, fp, sp, pc}
    4690:	e3004000 	movw	r4, #0
    4694:	e3404000 	movt	r4, #0
    4698:	e2840014 	add	r0, r4, #20
    469c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    46a0:	e284301c 	add	r3, r4, #28
    46a4:	f593f000 	pldw	[r3]
    46a8:	e1932f9f 	ldrex	r2, [r3]
    46ac:	e2822001 	add	r2, r2, #1
    46b0:	e1831f92 	strex	r1, r2, [r3]
    46b4:	e3310000 	teq	r1, #0
    46b8:	1afffffa 	bne	46a8 <fast_classifier_offload_genl_msg+0xb4>
    46bc:	e3a00000 	mov	r0, #0
    46c0:	e24bd010 	sub	sp, fp, #16
    46c4:	e89da810 	ldm	sp, {r4, fp, sp, pc}
    46c8:	00000014 	.word	0x00000014

000046cc <sfe_ipv6_recv_udp.constprop.0>:
static int sfe_ipv6_recv_udp(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
    46cc:	e1a0c00d 	mov	ip, sp
    46d0:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    46d4:	e24cb004 	sub	fp, ip, #4
    46d8:	e24dd01c 	sub	sp, sp, #28
    46dc:	e59b8004 	ldr	r8, [fp, #4]
    46e0:	e1a09002 	mov	r9, r2
    46e4:	e1a06003 	mov	r6, r3
	return skb->data_len;
}

static inline unsigned int skb_headlen(const struct sk_buff *skb)
{
	return skb->len - skb->data_len;
    46e8:	e590205c 	ldr	r2, [r0, #92]	@ 0x5c
    46ec:	e50b1030 	str	r1, [fp, #-48]	@ 0xffffffd0
    46f0:	e1a05000 	mov	r5, r0
    46f4:	e5901060 	ldr	r1, [r0, #96]	@ 0x60
    46f8:	e5db3008 	ldrb	r3, [fp, #8]
    46fc:	e042c001 	sub	ip, r2, r1
    4700:	e50b3038 	str	r3, [fp, #-56]	@ 0xffffffc8
	if (!pskb_may_pull(skb, (sizeof(struct sfe_ipv6_udp_hdr) + ihl))) {
    4704:	e2883008 	add	r3, r8, #8

void *__pskb_pull_tail(struct sk_buff *skb, int delta);

static inline bool pskb_may_pull(struct sk_buff *skb, unsigned int len)
{
	if (likely(len <= skb_headlen(skb)))
    4708:	e153000c 	cmp	r3, ip
    470c:	8a000085 	bhi	4928 <sfe_ipv6_recv_udp.constprop.0+0x25c>
	udph = (struct sfe_ipv6_udp_hdr *)(skb->data + ihl);
    4710:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
	raw_spin_lock_bh(&lock->rlock);
    4714:	e3007000 	movw	r7, #0
    4718:	e3407000 	movt	r7, #0
    471c:	e1a00007 	mov	r0, r7
    4720:	e083a008 	add	sl, r3, r8
	src_port = udph->source;
    4724:	e19330b8 	ldrh	r3, [r3, r8]
    4728:	e50b3034 	str	r3, [fp, #-52]	@ 0xffffffcc
	dest_port = udph->dest;
    472c:	e1da40b2 	ldrh	r4, [sl, #2]
    4730:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	dest_ip = &iph->daddr;
    4734:	e2862018 	add	r2, r6, #24
	cm = sfe_ipv6_find_connection_match(si, dev, IPPROTO_UDP, src_ip, src_port, dest_ip, dest_port);
    4738:	e3a01011 	mov	r1, #17
    473c:	e58d4004 	str	r4, [sp, #4]
    4740:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    4744:	e51b0030 	ldr	r0, [fp, #-48]	@ 0xffffffd0
    4748:	e58d2000 	str	r2, [sp]
    474c:	e2862008 	add	r2, r6, #8
    4750:	ebfff1fd 	bl	f4c <sfe_ipv6_find_connection_match.constprop.0>
	if (unlikely(!cm)) {
    4754:	e2504000 	subs	r4, r0, #0
    4758:	0a000086 	beq	4978 <sfe_ipv6_recv_udp.constprop.0+0x2ac>
	if (unlikely(flush_on_find)) {
    475c:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
    4760:	e3530000 	cmp	r3, #0
    4764:	1a00008d 	bne	49a0 <sfe_ipv6_recv_udp.constprop.0+0x2d4>
	if (unlikely(!cm->flow_accel)) {
    4768:	e594304c 	ldr	r3, [r4, #76]	@ 0x4c
    476c:	e3530000 	cmp	r3, #0
    4770:	0a000064 	beq	4908 <sfe_ipv6_recv_udp.constprop.0+0x23c>
	if (unlikely(iph->hop_limit < 2)) {
    4774:	e5d63007 	ldrb	r3, [r6, #7]
    4778:	e3530001 	cmp	r3, #1
    477c:	9a0000a2 	bls	4a0c <sfe_ipv6_recv_udp.constprop.0+0x340>
	if (unlikely(len > cm->xmit_dev_mtu)) {
    4780:	e1d429bc 	ldrh	r2, [r4, #156]	@ 0x9c
    4784:	e1520009 	cmp	r2, r9
    4788:	3a0000ae 	bcc	4a48 <sfe_ipv6_recv_udp.constprop.0+0x37c>
	return skb->cloned &&
    478c:	e5d5206a 	ldrb	r2, [r5, #106]	@ 0x6a
    4790:	e3120001 	tst	r2, #1
    4794:	1a000045 	bne	48b0 <sfe_ipv6_recv_udp.constprop.0+0x1e4>
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
    4798:	e5942048 	ldr	r2, [r4, #72]	@ 0x48
    479c:	e3120040 	tst	r2, #64	@ 0x40
    47a0:	1a0000c0 	bne	4aa8 <sfe_ipv6_recv_udp.constprop.0+0x3dc>
	iph->hop_limit -= 1;
    47a4:	e2433001 	sub	r3, r3, #1
    47a8:	e5c63007 	strb	r3, [r6, #7]
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
    47ac:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    47b0:	e3130001 	tst	r3, #1
    47b4:	1a0000c4 	bne	4acc <sfe_ipv6_recv_udp.constprop.0+0x400>
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    47b8:	e3130002 	tst	r3, #2
    47bc:	1a0000d4 	bne	4b14 <sfe_ipv6_recv_udp.constprop.0+0x448>
	cm->rx_byte_count += len;
    47c0:	e1c426d0 	ldrd	r2, [r4, #96]	@ 0x60
    47c4:	e0833009 	add	r3, r3, r9
	cm->rx_packet_count++;
    47c8:	e2822001 	add	r2, r2, #1
	cm->rx_byte_count += len;
    47cc:	e1c426f0 	strd	r2, [r4, #96]	@ 0x60
	if (unlikely(!cm->active)) {
    47d0:	e5d43018 	ldrb	r3, [r4, #24]
    47d4:	e3530000 	cmp	r3, #0
    47d8:	0a0000a9 	beq	4a84 <sfe_ipv6_recv_udp.constprop.0+0x3b8>
	xmit_dev = cm->xmit_dev;
    47dc:	e5941098 	ldr	r1, [r4, #152]	@ 0x98
	skb->dev = xmit_dev;
    47e0:	e5851008 	str	r1, [r5, #8]
	if (likely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
    47e4:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    47e8:	e3130010 	tst	r3, #16
    47ec:	0a000018 	beq	4854 <sfe_ipv6_recv_udp.constprop.0+0x188>
		if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
    47f0:	e3130008 	tst	r3, #8
    47f4:	0a0000d7 	beq	4b58 <sfe_ipv6_recv_udp.constprop.0+0x48c>
	skb->len  += len;
    47f8:	e595205c 	ldr	r2, [r5, #92]	@ 0x5c
			eth->h_proto = htons(ETH_P_IPV6);
    47fc:	e3e00079 	mvn	r0, #121	@ 0x79
    4800:	e3e01022 	mvn	r1, #34	@ 0x22
	skb->data -= len;
    4804:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
	skb->len  += len;
    4808:	e282200e 	add	r2, r2, #14
	skb->data -= len;
    480c:	e243c00e 	sub	ip, r3, #14
	skb->len  += len;
    4810:	e585205c 	str	r2, [r5, #92]	@ 0x5c
	skb->data -= len;
    4814:	e585c0b0 	str	ip, [r5, #176]	@ 0xb0
    4818:	e5430002 	strb	r0, [r3, #-2]
    481c:	e5431001 	strb	r1, [r3, #-1]
			eth->h_dest[0] = cm->xmit_dest_mac[0];
    4820:	e1d429be 	ldrh	r2, [r4, #158]	@ 0x9e
    4824:	e14320be 	strh	r2, [r3, #-14]
			eth->h_dest[1] = cm->xmit_dest_mac[1];
    4828:	e1d42ab0 	ldrh	r2, [r4, #160]	@ 0xa0
    482c:	e14320bc 	strh	r2, [r3, #-12]
			eth->h_dest[2] = cm->xmit_dest_mac[2];
    4830:	e1d42ab2 	ldrh	r2, [r4, #162]	@ 0xa2
    4834:	e14320ba 	strh	r2, [r3, #-10]
			eth->h_source[0] = cm->xmit_src_mac[0];
    4838:	e1d42ab4 	ldrh	r2, [r4, #164]	@ 0xa4
    483c:	e14320b8 	strh	r2, [r3, #-8]
			eth->h_source[1] = cm->xmit_src_mac[1];
    4840:	e1d42ab6 	ldrh	r2, [r4, #166]	@ 0xa6
    4844:	e14320b6 	strh	r2, [r3, #-6]
			eth->h_source[2] = cm->xmit_src_mac[2];
    4848:	e1d42ab8 	ldrh	r2, [r4, #168]	@ 0xa8
    484c:	e14320b4 	strh	r2, [r3, #-4]
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    4850:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    4854:	e3130020 	tst	r3, #32
		skb->priority = cm->priority;
    4858:	15943090 	ldrne	r3, [r4, #144]	@ 0x90
    485c:	15853078 	strne	r3, [r5, #120]	@ 0x78
	skb->mark = cm->connection->mark;
    4860:	e5942008 	ldr	r2, [r4, #8]
	si->packets_forwarded++;
    4864:	e59f3334 	ldr	r3, [pc, #820]	@ 4ba0 <sfe_ipv6_recv_udp.constprop.0+0x4d4>
	skb->mark = cm->connection->mark;
    4868:	e5922078 	ldr	r2, [r2, #120]	@ 0x78
	raw_spin_unlock_bh(&lock->rlock);
    486c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
    4870:	e5852090 	str	r2, [r5, #144]	@ 0x90
	si->packets_forwarded++;
    4874:	e593204c 	ldr	r2, [r3, #76]	@ 0x4c
    4878:	e2822001 	add	r2, r2, #1
    487c:	e583204c 	str	r2, [r3, #76]	@ 0x4c
    4880:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	__asm__ __volatile__(
    4884:	e59530a8 	ldr	r3, [r5, #168]	@ 0xa8
    4888:	f5d3f000 	pld	[r3]
	skb->fast_forwarded = 1;
    488c:	e5d53071 	ldrb	r3, [r5, #113]	@ 0x71
	return __dev_queue_xmit(skb, NULL);
    4890:	e3a01000 	mov	r1, #0
    4894:	e1a00005 	mov	r0, r5
    4898:	e3833008 	orr	r3, r3, #8
    489c:	e5c53071 	strb	r3, [r5, #113]	@ 0x71
    48a0:	ebfffffe 	bl	0 <__dev_queue_xmit>
	return 1;
    48a4:	e3a00001 	mov	r0, #1
}
    48a8:	e24bd028 	sub	sp, fp, #40	@ 0x28
    48ac:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
	return skb->end;
    48b0:	e59510a8 	ldr	r1, [r5, #168]	@ 0xa8
    48b4:	e5912020 	ldr	r2, [r1, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    48b8:	e6ff2072 	uxth	r2, r2
	return skb->cloned &&
    48bc:	e3520001 	cmp	r2, #1
    48c0:	0affffb4 	beq	4798 <sfe_ipv6_recv_udp.constprop.0+0xcc>
    48c4:	e5913020 	ldr	r3, [r1, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    48c8:	e6ff3073 	uxth	r3, r3
	return skb->cloned &&
    48cc:	e3530001 	cmp	r3, #1
    48d0:	0a000041 	beq	49dc <sfe_ipv6_recv_udp.constprop.0+0x310>
		struct sk_buff *nskb = skb_copy(skb, pri);
    48d4:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    48d8:	e1a00005 	mov	r0, r5
    48dc:	ebfffffe 	bl	0 <skb_copy>
		if (likely(nskb))
    48e0:	e2506000 	subs	r6, r0, #0
    48e4:	0a0000a9 	beq	4b90 <sfe_ipv6_recv_udp.constprop.0+0x4c4>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    48e8:	e1a00005 	mov	r0, r5
    48ec:	e3a01001 	mov	r1, #1
    48f0:	e1a05006 	mov	r5, r6
    48f4:	ebfffffe 	bl	0 <kfree_skb_reason>
		iph = (struct sfe_ipv6_ip_hdr *)skb->data;
    48f8:	e59560b0 	ldr	r6, [r5, #176]	@ 0xb0
	iph->hop_limit -= 1;
    48fc:	e5d63007 	ldrb	r3, [r6, #7]
		udph = (struct sfe_ipv6_udp_hdr *)(skb->data + ihl);
    4900:	e086a008 	add	sl, r6, r8
    4904:	eaffffa3 	b	4798 <sfe_ipv6_recv_udp.constprop.0+0xcc>
		si->packets_not_forwarded++;
    4908:	e2872902 	add	r2, r7, #32768	@ 0x8000
    490c:	e1a00007 	mov	r0, r7
    4910:	e5923050 	ldr	r3, [r2, #80]	@ 0x50
    4914:	e2833001 	add	r3, r3, #1
    4918:	e5823050 	str	r3, [r2, #80]	@ 0x50
    491c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    4920:	e3a00000 	mov	r0, #0
    4924:	eaffffdf 	b	48a8 <sfe_ipv6_recv_udp.constprop.0+0x1dc>
		return true;
	if (unlikely(len > skb->len))
    4928:	e1530002 	cmp	r3, r2
    492c:	8a000004 	bhi	4944 <sfe_ipv6_recv_udp.constprop.0+0x278>
		return false;
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    4930:	e0433002 	sub	r3, r3, r2
    4934:	e0831001 	add	r1, r3, r1
    4938:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, (sizeof(struct sfe_ipv6_udp_hdr) + ihl))) {
    493c:	e3500000 	cmp	r0, #0
    4940:	1affff72 	bne	4710 <sfe_ipv6_recv_udp.constprop.0+0x44>
	raw_spin_lock_bh(&lock->rlock);
    4944:	e3000000 	movw	r0, #0
    4948:	e3400000 	movt	r0, #0
    494c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    4950:	e59f3248 	ldr	r3, [pc, #584]	@ 4ba0 <sfe_ipv6_recv_udp.constprop.0+0x4d4>
		si->packets_not_forwarded++;
    4954:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    4958:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    495c:	e5932054 	ldr	r2, [r3, #84]	@ 0x54
		si->packets_not_forwarded++;
    4960:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    4964:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4968:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    496c:	e5832054 	str	r2, [r3, #84]	@ 0x54
    4970:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    4974:	eaffffe9 	b	4920 <sfe_ipv6_recv_udp.constprop.0+0x254>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4978:	e2873902 	add	r3, r7, #32768	@ 0x8000
    497c:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    4980:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4984:	e5932058 	ldr	r2, [r3, #88]	@ 0x58
		si->packets_not_forwarded++;
    4988:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    498c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4990:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4994:	e5832058 	str	r2, [r3, #88]	@ 0x58
    4998:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    499c:	eaffffdf 	b	4920 <sfe_ipv6_recv_udp.constprop.0+0x254>
		struct sfe_ipv6_connection *c = cm->connection;
    49a0:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    49a4:	e1a00004 	mov	r0, r4
    49a8:	ebfff6ad 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    49ac:	e2873902 	add	r3, r7, #32768	@ 0x8000
    49b0:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    49b4:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    49b8:	e593205c 	ldr	r2, [r3, #92]	@ 0x5c
		si->packets_not_forwarded++;
    49bc:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    49c0:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    49c4:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    49c8:	e583205c 	str	r2, [r3, #92]	@ 0x5c
    49cc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    49d0:	e1a00004 	mov	r0, r4
    49d4:	ebfff352 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    49d8:	eaffffd0 	b	4920 <sfe_ipv6_recv_udp.constprop.0+0x254>
                if (!skb) {
    49dc:	e3550000 	cmp	r5, #0
    49e0:	1affffc4 	bne	48f8 <sfe_ipv6_recv_udp.constprop.0+0x22c>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    49e4:	e59f31b4 	ldr	r3, [pc, #436]	@ 4ba0 <sfe_ipv6_recv_udp.constprop.0+0x4d4>
			si->packets_not_forwarded++;
    49e8:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    49ec:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    49f0:	e59320e8 	ldr	r2, [r3, #232]	@ 0xe8
			si->packets_not_forwarded++;
    49f4:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    49f8:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    49fc:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    4a00:	e58320e8 	str	r2, [r3, #232]	@ 0xe8
    4a04:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    4a08:	eaffffc4 	b	4920 <sfe_ipv6_recv_udp.constprop.0+0x254>
		struct sfe_ipv6_connection *c = cm->connection;
    4a0c:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    4a10:	e1a00004 	mov	r0, r4
    4a14:	ebfff692 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    4a18:	e2873902 	add	r3, r7, #32768	@ 0x8000
    4a1c:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    4a20:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    4a24:	e5932060 	ldr	r2, [r3, #96]	@ 0x60
		si->packets_not_forwarded++;
    4a28:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    4a2c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4a30:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    4a34:	e5832060 	str	r2, [r3, #96]	@ 0x60
    4a38:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    4a3c:	e1a00004 	mov	r0, r4
    4a40:	ebfff337 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    4a44:	eaffffb5 	b	4920 <sfe_ipv6_recv_udp.constprop.0+0x254>
		struct sfe_ipv6_connection *c = cm->connection;
    4a48:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    4a4c:	e1a00004 	mov	r0, r4
    4a50:	ebfff683 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    4a54:	e2873902 	add	r3, r7, #32768	@ 0x8000
    4a58:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    4a5c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    4a60:	e5932064 	ldr	r2, [r3, #100]	@ 0x64
		si->packets_not_forwarded++;
    4a64:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    4a68:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4a6c:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    4a70:	e5832064 	str	r2, [r3, #100]	@ 0x64
    4a74:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    4a78:	e1a00004 	mov	r0, r4
    4a7c:	ebfff328 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    4a80:	eaffffa6 	b	4920 <sfe_ipv6_recv_udp.constprop.0+0x254>
		cm->active_prev = si->active_tail;
    4a84:	e5973008 	ldr	r3, [r7, #8]
		cm->active = true;
    4a88:	e3a02001 	mov	r2, #1
		si->active_tail = cm;
    4a8c:	e5874008 	str	r4, [r7, #8]
		if (likely(si->active_tail)) {
    4a90:	e3530000 	cmp	r3, #0
		cm->active_prev = si->active_tail;
    4a94:	e5843014 	str	r3, [r4, #20]
		cm->active = true;
    4a98:	e5c42018 	strb	r2, [r4, #24]
			si->active_head = cm;
    4a9c:	05874004 	streq	r4, [r7, #4]
			si->active_tail->active_next = cm;
    4aa0:	15834010 	strne	r4, [r3, #16]
		si->active_tail = cm;
    4aa4:	eaffff4c 	b	47dc <sfe_ipv6_recv_udp.constprop.0+0x110>
	*p = ((*p & htons(SFE_IPV6_DSCP_MASK)) | htons((u16)dscp << 4));
    4aa8:	e5d41094 	ldrb	r1, [r4, #148]	@ 0x94
    4aac:	e3032ff0 	movw	r2, #16368	@ 0x3ff0
    4ab0:	e1d600b0 	ldrh	r0, [r6]
    4ab4:	e1a01201 	lsl	r1, r1, #4
    4ab8:	e0022000 	and	r2, r2, r0
    4abc:	e6bf1fb1 	rev16	r1, r1
    4ac0:	e1822001 	orr	r2, r2, r1
    4ac4:	e1c620b0 	strh	r2, [r6]
}
    4ac8:	eaffff35 	b	47a4 <sfe_ipv6_recv_udp.constprop.0+0xd8>
		iph->saddr = cm->xlate_src_ip[0];
    4acc:	e1c426d8 	ldrd	r2, [r4, #104]	@ 0x68
    4ad0:	e5862008 	str	r2, [r6, #8]
    4ad4:	e586300c 	str	r3, [r6, #12]
    4ad8:	e1c427d0 	ldrd	r2, [r4, #112]	@ 0x70
    4adc:	e5862010 	str	r2, [r6, #16]
    4ae0:	e5863014 	str	r3, [r6, #20]
		udp_csum = udph->check;
    4ae4:	e1da30b6 	ldrh	r3, [sl, #6]
		udph->source = cm->xlate_src_port;
    4ae8:	e1d427b8 	ldrh	r2, [r4, #120]	@ 0x78
		if (likely(udp_csum)) {
    4aec:	e3530000 	cmp	r3, #0
		udph->source = cm->xlate_src_port;
    4af0:	e1ca20b0 	strh	r2, [sl]
		if (likely(udp_csum)) {
    4af4:	0a000004 	beq	4b0c <sfe_ipv6_recv_udp.constprop.0+0x440>
			u32 sum = udp_csum + cm->xlate_src_csum_adjustment;
    4af8:	e1d427ba 	ldrh	r2, [r4, #122]	@ 0x7a
    4afc:	e0833002 	add	r3, r3, r2
			sum = (sum & 0xffff) + (sum >> 16);
    4b00:	e6ff2073 	uxth	r2, r3
    4b04:	e0823823 	add	r3, r2, r3, lsr #16
			udph->check = (u16)sum;
    4b08:	e1ca30b6 	strh	r3, [sl, #6]
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    4b0c:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    4b10:	eaffff28 	b	47b8 <sfe_ipv6_recv_udp.constprop.0+0xec>
		iph->daddr = cm->xlate_dest_ip[0];
    4b14:	e1c427dc 	ldrd	r2, [r4, #124]	@ 0x7c
    4b18:	e5862018 	str	r2, [r6, #24]
    4b1c:	e586301c 	str	r3, [r6, #28]
    4b20:	e1c428d4 	ldrd	r2, [r4, #132]	@ 0x84
    4b24:	e5862020 	str	r2, [r6, #32]
    4b28:	e5863024 	str	r3, [r6, #36]	@ 0x24
		udph->dest = cm->xlate_dest_port;
    4b2c:	e1d438bc 	ldrh	r3, [r4, #140]	@ 0x8c
    4b30:	e1ca30b2 	strh	r3, [sl, #2]
		udp_csum = udph->check;
    4b34:	e1da30b6 	ldrh	r3, [sl, #6]
		if (likely(udp_csum)) {
    4b38:	e3530000 	cmp	r3, #0
    4b3c:	0affff1f 	beq	47c0 <sfe_ipv6_recv_udp.constprop.0+0xf4>
			u32 sum = udp_csum + cm->xlate_dest_csum_adjustment;
    4b40:	e1d428be 	ldrh	r2, [r4, #142]	@ 0x8e
    4b44:	e0833002 	add	r3, r3, r2
			sum = (sum & 0xffff) + (sum >> 16);
    4b48:	e6ff2073 	uxth	r2, r3
    4b4c:	e0823823 	add	r3, r2, r3, lsr #16
			udph->check = (u16)sum;
    4b50:	e1ca30b6 	strh	r3, [sl, #6]
    4b54:	eaffff19 	b	47c0 <sfe_ipv6_recv_udp.constprop.0+0xf4>
	if (!dev->header_ops || !dev->header_ops->create)
    4b58:	e5912158 	ldr	r2, [r1, #344]	@ 0x158
    4b5c:	e3520000 	cmp	r2, #0
    4b60:	0affff3b 	beq	4854 <sfe_ipv6_recv_udp.constprop.0+0x188>
    4b64:	e5926000 	ldr	r6, [r2]
    4b68:	e3560000 	cmp	r6, #0
    4b6c:	0affff38 	beq	4854 <sfe_ipv6_recv_udp.constprop.0+0x188>
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
    4b70:	e28430a4 	add	r3, r4, #164	@ 0xa4
	return dev->header_ops->create(skb, dev, type, daddr, saddr, len);
    4b74:	e30826dd 	movw	r2, #34525	@ 0x86dd
    4b78:	e1a00005 	mov	r0, r5
    4b7c:	e88d0208 	stm	sp, {r3, r9}
    4b80:	e284309e 	add	r3, r4, #158	@ 0x9e
    4b84:	e12fff36 	blx	r6
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    4b88:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    4b8c:	eaffff30 	b	4854 <sfe_ipv6_recv_udp.constprop.0+0x188>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    4b90:	e3a01001 	mov	r1, #1
    4b94:	e1a00005 	mov	r0, r5
    4b98:	ebfffffe 	bl	0 <kfree_skb_reason>
                if (!skb) {
    4b9c:	eaffff90 	b	49e4 <sfe_ipv6_recv_udp.constprop.0+0x318>
    4ba0:	00010050 	.word	0x00010050

00004ba4 <sfe_ipv4_recv_udp.constprop.0>:
static int sfe_ipv4_recv_udp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
    4ba4:	e1a0c00d 	mov	ip, sp
    4ba8:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    4bac:	e24cb004 	sub	fp, ip, #4
    4bb0:	e24dd024 	sub	sp, sp, #36	@ 0x24
    4bb4:	e59b9004 	ldr	r9, [fp, #4]
    4bb8:	e1a08002 	mov	r8, r2
    4bbc:	e1a06003 	mov	r6, r3
	return skb->len - skb->data_len;
    4bc0:	e590205c 	ldr	r2, [r0, #92]	@ 0x5c
    4bc4:	e50b1030 	str	r1, [fp, #-48]	@ 0xffffffd0
    4bc8:	e1a05000 	mov	r5, r0
    4bcc:	e5901060 	ldr	r1, [r0, #96]	@ 0x60
    4bd0:	e5db3008 	ldrb	r3, [fp, #8]
    4bd4:	e042c001 	sub	ip, r2, r1
    4bd8:	e50b3040 	str	r3, [fp, #-64]	@ 0xffffffc0
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_udp_hdr) + ihl)))) {
    4bdc:	e2893008 	add	r3, r9, #8
	if (likely(len <= skb_headlen(skb)))
    4be0:	e153000c 	cmp	r3, ip
    4be4:	8a0000c9 	bhi	4f10 <sfe_ipv4_recv_udp.constprop.0+0x36c>
	udph = (struct sfe_ipv4_udp_hdr *)(skb->data + ihl);
    4be8:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
	raw_spin_lock_bh(&lock->rlock);
    4bec:	e3007000 	movw	r7, #0
	src_ip = iph->saddr;
    4bf0:	e596200c 	ldr	r2, [r6, #12]
    4bf4:	e3407000 	movt	r7, #0
    4bf8:	e1a00007 	mov	r0, r7
	udph = (struct sfe_ipv4_udp_hdr *)(skb->data + ihl);
    4bfc:	e083a009 	add	sl, r3, r9
	src_port = udph->source;
    4c00:	e19330b9 	ldrh	r3, [r3, r9]
	src_ip = iph->saddr;
    4c04:	e50b203c 	str	r2, [fp, #-60]	@ 0xffffffc4
	dest_ip = iph->daddr;
    4c08:	e5961010 	ldr	r1, [r6, #16]
    4c0c:	e50b1038 	str	r1, [fp, #-56]	@ 0xffffffc8
	src_port = udph->source;
    4c10:	e50b3034 	str	r3, [fp, #-52]	@ 0xffffffcc
	dest_port = udph->dest;
    4c14:	e1da40b2 	ldrh	r4, [sl, #2]
    4c18:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, IPPROTO_UDP, src_ip, src_port, dest_ip, dest_port);
    4c1c:	e51b1038 	ldr	r1, [fp, #-56]	@ 0xffffffc8
    4c20:	e88d0012 	stm	sp, {r1, r4}
    4c24:	e51b203c 	ldr	r2, [fp, #-60]	@ 0xffffffc4
    4c28:	e3a01011 	mov	r1, #17
    4c2c:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    4c30:	e51b0030 	ldr	r0, [fp, #-48]	@ 0xffffffd0
    4c34:	ebfff160 	bl	11bc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0>
	if (unlikely(!cm)) {
    4c38:	e2504000 	subs	r4, r0, #0
    4c3c:	0a0000c8 	beq	4f64 <sfe_ipv4_recv_udp.constprop.0+0x3c0>
	if (unlikely(flush_on_find)) {
    4c40:	e51b3040 	ldr	r3, [fp, #-64]	@ 0xffffffc0
    4c44:	e3530000 	cmp	r3, #0
    4c48:	1a0000cf 	bne	4f8c <sfe_ipv4_recv_udp.constprop.0+0x3e8>
	if (unlikely(!cm->flow_accel)) {
    4c4c:	e5943034 	ldr	r3, [r4, #52]	@ 0x34
    4c50:	e3530000 	cmp	r3, #0
    4c54:	0a0000db 	beq	4fc8 <sfe_ipv4_recv_udp.constprop.0+0x424>
	ttl = iph->ttl;
    4c58:	e5d63008 	ldrb	r3, [r6, #8]
	if (unlikely(ttl < 2)) {
    4c5c:	e3530001 	cmp	r3, #1
    4c60:	9a0000df 	bls	4fe4 <sfe_ipv4_recv_udp.constprop.0+0x440>
	if (unlikely(len > cm->xmit_dev_mtu)) {
    4c64:	e1d427b4 	ldrh	r2, [r4, #116]	@ 0x74
    4c68:	e1520008 	cmp	r2, r8
    4c6c:	3a0000eb 	bcc	5020 <sfe_ipv4_recv_udp.constprop.0+0x47c>
	return skb->cloned &&
    4c70:	e5d5206a 	ldrb	r2, [r5, #106]	@ 0x6a
    4c74:	e3120001 	tst	r2, #1
    4c78:	0a000015 	beq	4cd4 <sfe_ipv4_recv_udp.constprop.0+0x130>
	return skb->end;
    4c7c:	e59510a8 	ldr	r1, [r5, #168]	@ 0xa8
    4c80:	e5912020 	ldr	r2, [r1, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    4c84:	e6ff2072 	uxth	r2, r2
	return skb->cloned &&
    4c88:	e3520001 	cmp	r2, #1
    4c8c:	0a000010 	beq	4cd4 <sfe_ipv4_recv_udp.constprop.0+0x130>
    4c90:	e5912020 	ldr	r2, [r1, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    4c94:	e6ff2072 	uxth	r2, r2
	return skb->cloned &&
    4c98:	e3520001 	cmp	r2, #1
    4c9c:	0a0000fc 	beq	5094 <sfe_ipv4_recv_udp.constprop.0+0x4f0>
		struct sk_buff *nskb = skb_copy(skb, pri);
    4ca0:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    4ca4:	e1a00005 	mov	r0, r5
    4ca8:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
    4cac:	ebfffffe 	bl	0 <skb_copy>
		if (likely(nskb))
    4cb0:	e2506000 	subs	r6, r0, #0
    4cb4:	0a000102 	beq	50c4 <sfe_ipv4_recv_udp.constprop.0+0x520>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    4cb8:	e1a00005 	mov	r0, r5
    4cbc:	e3a01001 	mov	r1, #1
    4cc0:	ebfffffe 	bl	0 <kfree_skb_reason>
                if (!skb) {
    4cc4:	e51b3030 	ldr	r3, [fp, #-48]	@ 0xffffffd0
    4cc8:	e1a05006 	mov	r5, r6
		iph = (struct sfe_ipv4_ip_hdr *)skb->data;
    4ccc:	e59560b0 	ldr	r6, [r5, #176]	@ 0xb0
		udph = (struct sfe_ipv4_udp_hdr *)(skb->data + ihl);
    4cd0:	e086a009 	add	sl, r6, r9
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
    4cd4:	e5942030 	ldr	r2, [r4, #48]	@ 0x30
    4cd8:	e3120040 	tst	r2, #64	@ 0x40
    4cdc:	1a000059 	bne	4e48 <sfe_ipv4_recv_udp.constprop.0+0x2a4>
	iph->ttl = ttl - 1;
    4ce0:	e2433001 	sub	r3, r3, #1
    4ce4:	e5c63008 	strb	r3, [r6, #8]
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
    4ce8:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    4cec:	e3130001 	tst	r3, #1
    4cf0:	1a000063 	bne	4e84 <sfe_ipv4_recv_udp.constprop.0+0x2e0>
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    4cf4:	e3130002 	tst	r3, #2
    4cf8:	1a000073 	bne	4ecc <sfe_ipv4_recv_udp.constprop.0+0x328>
	sum = i[0] + i[1] + i[2] + i[3] + i[4] + i[5] + i[6] + i[7] + i[8] + i[9];
    4cfc:	e1d620b2 	ldrh	r2, [r6, #2]
    4d00:	e1d630b0 	ldrh	r3, [r6]
    4d04:	e0833002 	add	r3, r3, r2
    4d08:	e1d620b4 	ldrh	r2, [r6, #4]
    4d0c:	e0833002 	add	r3, r3, r2
    4d10:	e1d620b6 	ldrh	r2, [r6, #6]
    4d14:	e0833002 	add	r3, r3, r2
    4d18:	e1d620b8 	ldrh	r2, [r6, #8]
    4d1c:	e0833002 	add	r3, r3, r2
    4d20:	e1d620bc 	ldrh	r2, [r6, #12]
    4d24:	e0833002 	add	r3, r3, r2
    4d28:	e1d620be 	ldrh	r2, [r6, #14]
    4d2c:	e0833002 	add	r3, r3, r2
    4d30:	e1d621b0 	ldrh	r2, [r6, #16]
    4d34:	e0833002 	add	r3, r3, r2
    4d38:	e1d621b2 	ldrh	r2, [r6, #18]
    4d3c:	e0833002 	add	r3, r3, r2
	sum = (sum & 0xffff) + (sum >> 16);
    4d40:	e6ff2073 	uxth	r2, r3
    4d44:	e0823823 	add	r3, r2, r3, lsr #16
	sum = (sum & 0xffff) + (sum >> 16);
    4d48:	e1a02823 	lsr	r2, r3, #16
    4d4c:	e6f23073 	uxtah	r3, r2, r3
	return (u16)sum ^ 0xffff;
    4d50:	e1e03003 	mvn	r3, r3
	iph->check = sfe_ipv4_gen_ip_csum(iph);
    4d54:	e1c630ba 	strh	r3, [r6, #10]
	cm->rx_byte_count += len;
    4d58:	e1c424d8 	ldrd	r2, [r4, #72]	@ 0x48
    4d5c:	e0833008 	add	r3, r3, r8
	cm->rx_packet_count++;
    4d60:	e2822001 	add	r2, r2, #1
	cm->rx_byte_count += len;
    4d64:	e1c424f8 	strd	r2, [r4, #72]	@ 0x48
	if (unlikely(!cm->active)) {
    4d68:	e5d43018 	ldrb	r3, [r4, #24]
    4d6c:	e3530000 	cmp	r3, #0
    4d70:	0a00003a 	beq	4e60 <sfe_ipv4_recv_udp.constprop.0+0x2bc>
	xmit_dev = cm->xmit_dev;
    4d74:	e5941070 	ldr	r1, [r4, #112]	@ 0x70
	skb->dev = xmit_dev;
    4d78:	e5851008 	str	r1, [r5, #8]
	if (likely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
    4d7c:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    4d80:	e3130010 	tst	r3, #16
    4d84:	0a000018 	beq	4dec <sfe_ipv4_recv_udp.constprop.0+0x248>
		if (unlikely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
    4d88:	e3130008 	tst	r3, #8
    4d8c:	0a0000b2 	beq	505c <sfe_ipv4_recv_udp.constprop.0+0x4b8>
	skb->len  += len;
    4d90:	e595205c 	ldr	r2, [r5, #92]	@ 0x5c
			eth->h_proto = htons(ETH_P_IP);
    4d94:	e3a01000 	mov	r1, #0
    4d98:	e3a00008 	mov	r0, #8
	skb->data -= len;
    4d9c:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
	skb->len  += len;
    4da0:	e282200e 	add	r2, r2, #14
	skb->data -= len;
    4da4:	e243c00e 	sub	ip, r3, #14
	skb->len  += len;
    4da8:	e585205c 	str	r2, [r5, #92]	@ 0x5c
	skb->data -= len;
    4dac:	e585c0b0 	str	ip, [r5, #176]	@ 0xb0
    4db0:	e5430002 	strb	r0, [r3, #-2]
    4db4:	e5431001 	strb	r1, [r3, #-1]
			eth->h_dest[0] = cm->xmit_dest_mac[0];
    4db8:	e1d427b6 	ldrh	r2, [r4, #118]	@ 0x76
    4dbc:	e14320be 	strh	r2, [r3, #-14]
			eth->h_dest[1] = cm->xmit_dest_mac[1];
    4dc0:	e1d427b8 	ldrh	r2, [r4, #120]	@ 0x78
    4dc4:	e14320bc 	strh	r2, [r3, #-12]
			eth->h_dest[2] = cm->xmit_dest_mac[2];
    4dc8:	e1d427ba 	ldrh	r2, [r4, #122]	@ 0x7a
    4dcc:	e14320ba 	strh	r2, [r3, #-10]
			eth->h_source[0] = cm->xmit_src_mac[0];
    4dd0:	e1d427bc 	ldrh	r2, [r4, #124]	@ 0x7c
    4dd4:	e14320b8 	strh	r2, [r3, #-8]
			eth->h_source[1] = cm->xmit_src_mac[1];
    4dd8:	e1d427be 	ldrh	r2, [r4, #126]	@ 0x7e
    4ddc:	e14320b6 	strh	r2, [r3, #-6]
			eth->h_source[2] = cm->xmit_src_mac[2];
    4de0:	e1d428b0 	ldrh	r2, [r4, #128]	@ 0x80
    4de4:	e14320b4 	strh	r2, [r3, #-4]
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    4de8:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    4dec:	e3130020 	tst	r3, #32
		skb->priority = cm->priority;
    4df0:	15943068 	ldrne	r3, [r4, #104]	@ 0x68
    4df4:	15853078 	strne	r3, [r5, #120]	@ 0x78
	skb->mark = cm->connection->mark;
    4df8:	e5942008 	ldr	r2, [r4, #8]
	si->packets_forwarded++;
    4dfc:	e59f32d0 	ldr	r3, [pc, #720]	@ 50d4 <sfe_ipv4_recv_udp.constprop.0+0x530>
	skb->mark = cm->connection->mark;
    4e00:	e5922048 	ldr	r2, [r2, #72]	@ 0x48
	raw_spin_unlock_bh(&lock->rlock);
    4e04:	e2430902 	sub	r0, r3, #32768	@ 0x8000
    4e08:	e5852090 	str	r2, [r5, #144]	@ 0x90
	si->packets_forwarded++;
    4e0c:	e593204c 	ldr	r2, [r3, #76]	@ 0x4c
    4e10:	e2822001 	add	r2, r2, #1
    4e14:	e583204c 	str	r2, [r3, #76]	@ 0x4c
    4e18:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    4e1c:	e59530a8 	ldr	r3, [r5, #168]	@ 0xa8
    4e20:	f5d3f000 	pld	[r3]
	skb->fast_forwarded = 1;
    4e24:	e5d53071 	ldrb	r3, [r5, #113]	@ 0x71
	return __dev_queue_xmit(skb, NULL);
    4e28:	e3a01000 	mov	r1, #0
    4e2c:	e1a00005 	mov	r0, r5
    4e30:	e3833008 	orr	r3, r3, #8
    4e34:	e5c53071 	strb	r3, [r5, #113]	@ 0x71
    4e38:	ebfffffe 	bl	0 <__dev_queue_xmit>
	return 1;
    4e3c:	e3a00001 	mov	r0, #1
}
    4e40:	e24bd028 	sub	sp, fp, #40	@ 0x28
    4e44:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		iph->tos = (iph->tos & SFE_IPV4_DSCP_MASK) | cm->dscp;
    4e48:	e5d62001 	ldrb	r2, [r6, #1]
    4e4c:	e594106c 	ldr	r1, [r4, #108]	@ 0x6c
    4e50:	e2022003 	and	r2, r2, #3
    4e54:	e1822001 	orr	r2, r2, r1
    4e58:	e5c62001 	strb	r2, [r6, #1]
    4e5c:	eaffff9f 	b	4ce0 <sfe_ipv4_recv_udp.constprop.0+0x13c>
		cm->active_prev = si->active_tail;
    4e60:	e5973008 	ldr	r3, [r7, #8]
		cm->active = true;
    4e64:	e3a02001 	mov	r2, #1
		si->active_tail = cm;
    4e68:	e5874008 	str	r4, [r7, #8]
		if (likely(si->active_tail)) {
    4e6c:	e3530000 	cmp	r3, #0
		cm->active_prev = si->active_tail;
    4e70:	e5843014 	str	r3, [r4, #20]
		cm->active = true;
    4e74:	e5c42018 	strb	r2, [r4, #24]
			si->active_head = cm;
    4e78:	05874004 	streq	r4, [r7, #4]
			si->active_tail->active_next = cm;
    4e7c:	15834010 	strne	r4, [r3, #16]
		si->active_tail = cm;
    4e80:	eaffffbb 	b	4d74 <sfe_ipv4_recv_udp.constprop.0+0x1d0>
		iph->saddr = cm->xlate_src_ip;
    4e84:	e5943050 	ldr	r3, [r4, #80]	@ 0x50
    4e88:	e586300c 	str	r3, [r6, #12]
		udp_csum = udph->check;
    4e8c:	e1da30b6 	ldrh	r3, [sl, #6]
		udph->source = cm->xlate_src_port;
    4e90:	e1d425b4 	ldrh	r2, [r4, #84]	@ 0x54
		if (likely(udp_csum)) {
    4e94:	e3530000 	cmp	r3, #0
		udph->source = cm->xlate_src_port;
    4e98:	e1ca20b0 	strh	r2, [sl]
		if (likely(udp_csum)) {
    4e9c:	0a000008 	beq	4ec4 <sfe_ipv4_recv_udp.constprop.0+0x320>
			if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
    4ea0:	e5d5206c 	ldrb	r2, [r5, #108]	@ 0x6c
    4ea4:	e2022060 	and	r2, r2, #96	@ 0x60
    4ea8:	e3520060 	cmp	r2, #96	@ 0x60
				sum = udp_csum + cm->xlate_src_partial_csum_adjustment;
    4eac:	01d425b8 	ldrheq	r2, [r4, #88]	@ 0x58
				sum = udp_csum + cm->xlate_src_csum_adjustment;
    4eb0:	11d425b6 	ldrhne	r2, [r4, #86]	@ 0x56
    4eb4:	e0823003 	add	r3, r2, r3
			sum = (sum & 0xffff) + (sum >> 16);
    4eb8:	e1a02823 	lsr	r2, r3, #16
    4ebc:	e6f23073 	uxtah	r3, r2, r3
			udph->check = (u16)sum;
    4ec0:	e1ca30b6 	strh	r3, [sl, #6]
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    4ec4:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    4ec8:	eaffff89 	b	4cf4 <sfe_ipv4_recv_udp.constprop.0+0x150>
		iph->daddr = cm->xlate_dest_ip;
    4ecc:	e594305c 	ldr	r3, [r4, #92]	@ 0x5c
    4ed0:	e5863010 	str	r3, [r6, #16]
		udp_csum = udph->check;
    4ed4:	e1da30b6 	ldrh	r3, [sl, #6]
		udph->dest = cm->xlate_dest_port;
    4ed8:	e1d426b0 	ldrh	r2, [r4, #96]	@ 0x60
		if (likely(udp_csum)) {
    4edc:	e3530000 	cmp	r3, #0
		udph->dest = cm->xlate_dest_port;
    4ee0:	e1ca20b2 	strh	r2, [sl, #2]
		if (likely(udp_csum)) {
    4ee4:	0affff84 	beq	4cfc <sfe_ipv4_recv_udp.constprop.0+0x158>
			if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
    4ee8:	e5d5206c 	ldrb	r2, [r5, #108]	@ 0x6c
    4eec:	e2022060 	and	r2, r2, #96	@ 0x60
    4ef0:	e3520060 	cmp	r2, #96	@ 0x60
				sum = udp_csum + cm->xlate_dest_partial_csum_adjustment;
    4ef4:	01d426b4 	ldrheq	r2, [r4, #100]	@ 0x64
				sum = udp_csum + cm->xlate_dest_csum_adjustment;
    4ef8:	11d426b2 	ldrhne	r2, [r4, #98]	@ 0x62
    4efc:	e0823003 	add	r3, r2, r3
			sum = (sum & 0xffff) + (sum >> 16);
    4f00:	e1a02823 	lsr	r2, r3, #16
    4f04:	e6f23073 	uxtah	r3, r2, r3
			udph->check = (u16)sum;
    4f08:	e1ca30b6 	strh	r3, [sl, #6]
    4f0c:	eaffff7a 	b	4cfc <sfe_ipv4_recv_udp.constprop.0+0x158>
	if (unlikely(len > skb->len))
    4f10:	e1530002 	cmp	r3, r2
    4f14:	8a000004 	bhi	4f2c <sfe_ipv4_recv_udp.constprop.0+0x388>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    4f18:	e0433002 	sub	r3, r3, r2
    4f1c:	e0831001 	add	r1, r3, r1
    4f20:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_udp_hdr) + ihl)))) {
    4f24:	e3500000 	cmp	r0, #0
    4f28:	1affff2e 	bne	4be8 <sfe_ipv4_recv_udp.constprop.0+0x44>
	raw_spin_lock_bh(&lock->rlock);
    4f2c:	e3000000 	movw	r0, #0
    4f30:	e3400000 	movt	r0, #0
    4f34:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    4f38:	e59f3194 	ldr	r3, [pc, #404]	@ 50d4 <sfe_ipv4_recv_udp.constprop.0+0x530>
		si->packets_not_forwarded++;
    4f3c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    4f40:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    4f44:	e5932054 	ldr	r2, [r3, #84]	@ 0x54
		si->packets_not_forwarded++;
    4f48:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    4f4c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4f50:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
    4f54:	e5832054 	str	r2, [r3, #84]	@ 0x54
    4f58:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    4f5c:	e3a00000 	mov	r0, #0
    4f60:	eaffffb6 	b	4e40 <sfe_ipv4_recv_udp.constprop.0+0x29c>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4f64:	e2873902 	add	r3, r7, #32768	@ 0x8000
    4f68:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    4f6c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4f70:	e5932058 	ldr	r2, [r3, #88]	@ 0x58
		si->packets_not_forwarded++;
    4f74:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4f78:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4f7c:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
    4f80:	e5832058 	str	r2, [r3, #88]	@ 0x58
    4f84:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    4f88:	eafffff3 	b	4f5c <sfe_ipv4_recv_udp.constprop.0+0x3b8>
		struct sfe_ipv4_connection *c = cm->connection;
    4f8c:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    4f90:	e1a00004 	mov	r0, r4
    4f94:	ebfff4af 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    4f98:	e2873902 	add	r3, r7, #32768	@ 0x8000
    4f9c:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    4fa0:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    4fa4:	e593205c 	ldr	r2, [r3, #92]	@ 0x5c
		si->packets_not_forwarded++;
    4fa8:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    4fac:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    4fb0:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    4fb4:	e583205c 	str	r2, [r3, #92]	@ 0x5c
    4fb8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    4fbc:	e1a00004 	mov	r0, r4
    4fc0:	ebfff30b 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    4fc4:	eaffffe4 	b	4f5c <sfe_ipv4_recv_udp.constprop.0+0x3b8>
		si->packets_not_forwarded++;
    4fc8:	e2872902 	add	r2, r7, #32768	@ 0x8000
    4fcc:	e1a00007 	mov	r0, r7
    4fd0:	e5923050 	ldr	r3, [r2, #80]	@ 0x50
    4fd4:	e2833001 	add	r3, r3, #1
    4fd8:	e5823050 	str	r3, [r2, #80]	@ 0x50
    4fdc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    4fe0:	eaffffdd 	b	4f5c <sfe_ipv4_recv_udp.constprop.0+0x3b8>
		struct sfe_ipv4_connection *c = cm->connection;
    4fe4:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    4fe8:	e1a00004 	mov	r0, r4
    4fec:	ebfff499 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    4ff0:	e2873902 	add	r3, r7, #32768	@ 0x8000
    4ff4:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    4ff8:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    4ffc:	e5932060 	ldr	r2, [r3, #96]	@ 0x60
		si->packets_not_forwarded++;
    5000:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    5004:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5008:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
    500c:	e5832060 	str	r2, [r3, #96]	@ 0x60
    5010:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5014:	e1a00004 	mov	r0, r4
    5018:	ebfff2f5 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    501c:	eaffffce 	b	4f5c <sfe_ipv4_recv_udp.constprop.0+0x3b8>
		struct sfe_ipv4_connection *c = cm->connection;
    5020:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    5024:	e1a00004 	mov	r0, r4
    5028:	ebfff48a 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    502c:	e2873902 	add	r3, r7, #32768	@ 0x8000
    5030:	e1a00007 	mov	r0, r7
		si->packets_not_forwarded++;
    5034:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    5038:	e5932064 	ldr	r2, [r3, #100]	@ 0x64
		si->packets_not_forwarded++;
    503c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    5040:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5044:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
    5048:	e5832064 	str	r2, [r3, #100]	@ 0x64
    504c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5050:	e1a00004 	mov	r0, r4
    5054:	ebfff2e6 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    5058:	eaffffbf 	b	4f5c <sfe_ipv4_recv_udp.constprop.0+0x3b8>
	if (!dev->header_ops || !dev->header_ops->create)
    505c:	e5912158 	ldr	r2, [r1, #344]	@ 0x158
    5060:	e3520000 	cmp	r2, #0
    5064:	0affff60 	beq	4dec <sfe_ipv4_recv_udp.constprop.0+0x248>
    5068:	e5926000 	ldr	r6, [r2]
    506c:	e3560000 	cmp	r6, #0
    5070:	0affff5d 	beq	4dec <sfe_ipv4_recv_udp.constprop.0+0x248>
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
    5074:	e284307c 	add	r3, r4, #124	@ 0x7c
	return dev->header_ops->create(skb, dev, type, daddr, saddr, len);
    5078:	e3a02b02 	mov	r2, #2048	@ 0x800
    507c:	e1a00005 	mov	r0, r5
    5080:	e88d0108 	stm	sp, {r3, r8}
    5084:	e2843076 	add	r3, r4, #118	@ 0x76
    5088:	e12fff36 	blx	r6
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    508c:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    5090:	eaffff55 	b	4dec <sfe_ipv4_recv_udp.constprop.0+0x248>
                if (!skb) {
    5094:	e3550000 	cmp	r5, #0
    5098:	1affff0b 	bne	4ccc <sfe_ipv4_recv_udp.constprop.0+0x128>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    509c:	e59f3030 	ldr	r3, [pc, #48]	@ 50d4 <sfe_ipv4_recv_udp.constprop.0+0x530>
			si->packets_not_forwarded++;
    50a0:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    50a4:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    50a8:	e59320e4 	ldr	r2, [r3, #228]	@ 0xe4
			si->packets_not_forwarded++;
    50ac:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    50b0:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    50b4:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    50b8:	e58320e4 	str	r2, [r3, #228]	@ 0xe4
    50bc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    50c0:	eaffffa5 	b	4f5c <sfe_ipv4_recv_udp.constprop.0+0x3b8>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    50c4:	e3a01001 	mov	r1, #1
    50c8:	e1a00005 	mov	r0, r5
    50cc:	ebfffffe 	bl	0 <kfree_skb_reason>
                if (!skb) {
    50d0:	eafffff1 	b	509c <sfe_ipv4_recv_udp.constprop.0+0x4f8>
    50d4:	000182c8 	.word	0x000182c8

000050d8 <sfe_ipv4_periodic_sync>:
{
    50d8:	e1a0c00d 	mov	ip, sp
    50dc:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    50e0:	e24cb004 	sub	fp, ip, #4
    50e4:	e24dd0cc 	sub	sp, sp, #204	@ 0xcc
    50e8:	e1a07000 	mov	r7, r0
	now_jiffies = get_jiffies_64();
    50ec:	ebfffffe 	bl	0 <get_jiffies_64>
    50f0:	e1a04000 	mov	r4, r0
    50f4:	e1a05001 	mov	r5, r1
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    50f8:	e5973014 	ldr	r3, [r7, #20]
	if (!sync_rule_callback) {
    50fc:	e3530000 	cmp	r3, #0
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    5100:	e50b30ec 	str	r3, [fp, #-236]	@ 0xffffff14
	if (!sync_rule_callback) {
    5104:	0a000094 	beq	535c <sfe_ipv4_periodic_sync+0x284>
	struct sfe_ipv4 *si= from_timer(si, t, timer);
    5108:	e247a018 	sub	sl, r7, #24
	raw_spin_lock_bh(&lock->rlock);
    510c:	e1a0000a 	mov	r0, sl
    5110:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	sfe_ipv4_update_summary_stats(si);
    5114:	e1a0000a 	mov	r0, sl
    5118:	ebffec07 	bl	13c <sfe_ipv4_update_summary_stats>
	quota = (si->num_connections + 63) / 64;
    511c:	e5178004 	ldr	r8, [r7, #-4]
    5120:	e288803f 	add	r8, r8, #63	@ 0x3f
	while (quota--) {
    5124:	e1b08328 	lsrs	r8, r8, #6
    5128:	0a000089 	beq	5354 <sfe_ipv4_periodic_sync+0x27c>
			counter_cm->active = false;
    512c:	e3a06000 	mov	r6, #0
			counter_cm->active_next = NULL;
    5130:	e50b50f4 	str	r5, [fp, #-244]	@ 0xffffff0c
    5134:	e50b40f0 	str	r4, [fp, #-240]	@ 0xffffff10
    5138:	e50ba0e8 	str	sl, [fp, #-232]	@ 0xffffff18
    513c:	ea000080 	b	5344 <sfe_ipv4_periodic_sync+0x26c>
		counter_cm = cm->counter_match;
    5140:	e593200c 	ldr	r2, [r3, #12]
		if (counter_cm->active) {
    5144:	e5d21018 	ldrb	r1, [r2, #24]
    5148:	e3510000 	cmp	r1, #0
    514c:	0a00000b 	beq	5180 <sfe_ipv4_periodic_sync+0xa8>
			counter_cm->active_prev->active_next = counter_cm->active_next;
    5150:	e5921010 	ldr	r1, [r2, #16]
			counter_cm->active = false;
    5154:	e5c26018 	strb	r6, [r2, #24]
			counter_cm->active_prev->active_next = counter_cm->active_next;
    5158:	e5920014 	ldr	r0, [r2, #20]
			if (likely(counter_cm->active_next)) {
    515c:	e3510000 	cmp	r1, #0
			counter_cm->active_prev->active_next = counter_cm->active_next;
    5160:	e5801010 	str	r1, [r0, #16]
				counter_cm->active_next->active_prev = counter_cm->active_prev;
    5164:	15920014 	ldrne	r0, [r2, #20]
				si->active_tail = counter_cm->active_prev;
    5168:	05921014 	ldreq	r1, [r2, #20]
				counter_cm->active_next->active_prev = counter_cm->active_prev;
    516c:	15810014 	strne	r0, [r1, #20]
			counter_cm->active_next = NULL;
    5170:	e3a00000 	mov	r0, #0
				si->active_tail = counter_cm->active_prev;
    5174:	05071010 	streq	r1, [r7, #-16]
			counter_cm->active_next = NULL;
    5178:	e3a01000 	mov	r1, #0
    517c:	e1c201f0 	strd	r0, [r2, #16]
		si->active_head = cm->active_next;
    5180:	e5932010 	ldr	r2, [r3, #16]
		cm->active = false;
    5184:	e5c36018 	strb	r6, [r3, #24]
	cm->rx_packet_count = 0;
    5188:	e3a04000 	mov	r4, #0
	raw_spin_unlock_bh(&lock->rlock);
    518c:	e51b00e8 	ldr	r0, [fp, #-232]	@ 0xffffff18
    5190:	e3a05000 	mov	r5, #0
		if (likely(cm->active_next)) {
    5194:	e3520000 	cmp	r2, #0
		si->active_head = cm->active_next;
    5198:	e5072014 	str	r2, [r7, #-20]	@ 0xffffffec
			si->active_tail = NULL;
    519c:	05072010 	streq	r2, [r7, #-16]
			cm->active_next->active_prev = NULL;
    51a0:	15826014 	strne	r6, [r2, #20]
		c = cm->connection;
    51a4:	e5931008 	ldr	r1, [r3, #8]
	sis->is_v6 = 0;
    51a8:	e50b60dc 	str	r6, [fp, #-220]	@ 0xffffff24
		cm->active_next = NULL;
    51ac:	e5836010 	str	r6, [r3, #16]
	sis->protocol = c->protocol;
    51b0:	e5913008 	ldr	r3, [r1, #8]
    51b4:	e50b30d8 	str	r3, [fp, #-216]	@ 0xffffff28
	sis->src_ip.ip = c->src_ip;
    51b8:	e591300c 	ldr	r3, [r1, #12]
    51bc:	e50b30d4 	str	r3, [fp, #-212]	@ 0xffffff2c
	sis->src_ip_xlate.ip = c->src_ip_xlate;
    51c0:	e5913010 	ldr	r3, [r1, #16]
    51c4:	e50b30c4 	str	r3, [fp, #-196]	@ 0xffffff3c
	sis->dest_ip.ip = c->dest_ip;
    51c8:	e5913014 	ldr	r3, [r1, #20]
    51cc:	e50b30b0 	str	r3, [fp, #-176]	@ 0xffffff50
	sis->dest_ip_xlate.ip = c->dest_ip_xlate;
    51d0:	e5913018 	ldr	r3, [r1, #24]
    51d4:	e50b30a0 	str	r3, [fp, #-160]	@ 0xffffff60
	sis->src_port = c->src_port;
    51d8:	e1d131bc 	ldrh	r3, [r1, #28]
    51dc:	e14b3bb4 	strh	r3, [fp, #-180]	@ 0xffffff4c
	sis->src_port_xlate = c->src_port_xlate;
    51e0:	e1d131be 	ldrh	r3, [r1, #30]
    51e4:	e14b3bb2 	strh	r3, [fp, #-178]	@ 0xffffff4e
	sis->dest_port = c->dest_port;
    51e8:	e1d132b0 	ldrh	r3, [r1, #32]
    51ec:	e14b39b0 	strh	r3, [fp, #-144]	@ 0xffffff70
	sis->dest_port_xlate = c->dest_port_xlate;
    51f0:	e1d132b2 	ldrh	r3, [r1, #34]	@ 0x22
    51f4:	e14b38be 	strh	r3, [fp, #-142]	@ 0xffffff72
	original_cm = c->original_match;
    51f8:	e5913024 	ldr	r3, [r1, #36]	@ 0x24
	reply_cm = c->reply_match;
    51fc:	e591202c 	ldr	r2, [r1, #44]	@ 0x2c
	sis->src_td_max_window = original_cm->protocol_state.tcp.max_win;
    5200:	e593c03c 	ldr	ip, [r3, #60]	@ 0x3c
    5204:	e50bc08c 	str	ip, [fp, #-140]	@ 0xffffff74
	sis->src_td_end = original_cm->protocol_state.tcp.end;
    5208:	e593c040 	ldr	ip, [r3, #64]	@ 0x40
    520c:	e50bc088 	str	ip, [fp, #-136]	@ 0xffffff78
	sis->src_td_max_end = original_cm->protocol_state.tcp.max_end;
    5210:	e593c044 	ldr	ip, [r3, #68]	@ 0x44
    5214:	e50bc084 	str	ip, [fp, #-132]	@ 0xffffff7c
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
    5218:	e592c03c 	ldr	ip, [r2, #60]	@ 0x3c
    521c:	e50bc064 	str	ip, [fp, #-100]	@ 0xffffff9c
	sis->dest_td_end = reply_cm->protocol_state.tcp.end;
    5220:	e592c040 	ldr	ip, [r2, #64]	@ 0x40
    5224:	e50bc060 	str	ip, [fp, #-96]	@ 0xffffffa0
	sis->dest_td_max_end = reply_cm->protocol_state.tcp.max_end;
    5228:	e592c044 	ldr	ip, [r2, #68]	@ 0x44
    522c:	e50bc05c 	str	ip, [fp, #-92]	@ 0xffffffa4
	sis->src_new_packet_count = original_cm->rx_packet_count;
    5230:	e593c048 	ldr	ip, [r3, #72]	@ 0x48
    5234:	e50bc06c 	str	ip, [fp, #-108]	@ 0xffffff94
	sis->src_new_byte_count = original_cm->rx_byte_count;
    5238:	e593c04c 	ldr	ip, [r3, #76]	@ 0x4c
    523c:	e50bc068 	str	ip, [fp, #-104]	@ 0xffffff98
	sis->dest_new_packet_count = reply_cm->rx_packet_count;
    5240:	e592c048 	ldr	ip, [r2, #72]	@ 0x48
    5244:	e50bc044 	str	ip, [fp, #-68]	@ 0xffffffbc
	sis->dest_new_byte_count = reply_cm->rx_byte_count;
    5248:	e592c04c 	ldr	ip, [r2, #76]	@ 0x4c
    524c:	e50bc040 	str	ip, [fp, #-64]	@ 0xffffffc0
	cm->rx_packet_count64 += cm->rx_packet_count;
    5250:	e593e048 	ldr	lr, [r3, #72]	@ 0x48
    5254:	e593c088 	ldr	ip, [r3, #136]	@ 0x88
    5258:	e09cc00e 	adds	ip, ip, lr
	cm->rx_byte_count64 += cm->rx_byte_count;
    525c:	e593e04c 	ldr	lr, [r3, #76]	@ 0x4c
	cm->rx_packet_count = 0;
    5260:	e1c344f8 	strd	r4, [r3, #72]	@ 0x48
	cm->rx_packet_count64 += cm->rx_packet_count;
    5264:	e583c088 	str	ip, [r3, #136]	@ 0x88
    5268:	e593c08c 	ldr	ip, [r3, #140]	@ 0x8c
    526c:	e2acc000 	adc	ip, ip, #0
    5270:	e583c08c 	str	ip, [r3, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    5274:	e593c090 	ldr	ip, [r3, #144]	@ 0x90
    5278:	e09cc00e 	adds	ip, ip, lr
    527c:	e583c090 	str	ip, [r3, #144]	@ 0x90
    5280:	e593c094 	ldr	ip, [r3, #148]	@ 0x94
    5284:	e2acc000 	adc	ip, ip, #0
    5288:	e583c094 	str	ip, [r3, #148]	@ 0x94
	cm->rx_packet_count64 += cm->rx_packet_count;
    528c:	e592c048 	ldr	ip, [r2, #72]	@ 0x48
	cm->rx_byte_count64 += cm->rx_byte_count;
    5290:	e592a04c 	ldr	sl, [r2, #76]	@ 0x4c
	cm->rx_packet_count64 += cm->rx_packet_count;
    5294:	e592e088 	ldr	lr, [r2, #136]	@ 0x88
    5298:	e592908c 	ldr	r9, [r2, #140]	@ 0x8c
    529c:	e09cc00e 	adds	ip, ip, lr
	cm->rx_byte_count64 += cm->rx_byte_count;
    52a0:	e592e090 	ldr	lr, [r2, #144]	@ 0x90
	cm->rx_packet_count = 0;
    52a4:	e1c244f8 	strd	r4, [r2, #72]	@ 0x48
	cm->rx_packet_count64 += cm->rx_packet_count;
    52a8:	e2a99000 	adc	r9, r9, #0
    52ac:	e582c088 	str	ip, [r2, #136]	@ 0x88
    52b0:	e582908c 	str	r9, [r2, #140]	@ 0x8c
	cm->rx_byte_count64 += cm->rx_byte_count;
    52b4:	e09ee00a 	adds	lr, lr, sl
    52b8:	e582e090 	str	lr, [r2, #144]	@ 0x90
    52bc:	e592e094 	ldr	lr, [r2, #148]	@ 0x94
    52c0:	e2aee000 	adc	lr, lr, #0
    52c4:	e582e094 	str	lr, [r2, #148]	@ 0x94
	sis->src_dev = original_cm->match_dev;
    52c8:	e593e01c 	ldr	lr, [r3, #28]
	sis->src_packet_count = original_cm->rx_packet_count64;
    52cc:	e1c348d8 	ldrd	r4, [r3, #136]	@ 0x88
	sis->src_dev = original_cm->match_dev;
    52d0:	e50be0e4 	str	lr, [fp, #-228]	@ 0xffffff1c
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    52d4:	e51be0f0 	ldr	lr, [fp, #-240]	@ 0xffffff10
	sis->src_packet_count = original_cm->rx_packet_count64;
    52d8:	e14b47fc 	strd	r4, [fp, #-124]	@ 0xffffff84
	sis->src_byte_count = original_cm->rx_byte_count64;
    52dc:	e1c349d0 	ldrd	r4, [r3, #144]	@ 0x90
    52e0:	e14b47f4 	strd	r4, [fp, #-116]	@ 0xffffff8c
	sis->dest_dev = reply_cm->match_dev;
    52e4:	e592301c 	ldr	r3, [r2, #28]
    52e8:	e50b30e0 	str	r3, [fp, #-224]	@ 0xffffff20
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    52ec:	e50bc054 	str	ip, [fp, #-84]	@ 0xffffffac
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    52f0:	e51bc0f4 	ldr	ip, [fp, #-244]	@ 0xffffff0c
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    52f4:	e50b9050 	str	r9, [fp, #-80]	@ 0xffffffb0
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    52f8:	e1c229d0 	ldrd	r2, [r2, #144]	@ 0x90
    52fc:	e14b24fc 	strd	r2, [fp, #-76]	@ 0xffffffb4
	sis->reason = reason;
    5300:	e50b603c 	str	r6, [fp, #-60]	@ 0xffffffc4
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    5304:	e5913038 	ldr	r3, [r1, #56]	@ 0x38
    5308:	e591203c 	ldr	r2, [r1, #60]	@ 0x3c
    530c:	e05e3003 	subs	r3, lr, r3
    5310:	e50b3034 	str	r3, [fp, #-52]	@ 0xffffffcc
    5314:	e0cc3002 	sbc	r3, ip, r2
    5318:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
	c->last_sync_jiffies = now_jiffies;
    531c:	e581e038 	str	lr, [r1, #56]	@ 0x38
    5320:	e581c03c 	str	ip, [r1, #60]	@ 0x3c
    5324:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sync_rule_callback(&sis);
    5328:	e51b30ec 	ldr	r3, [fp, #-236]	@ 0xffffff14
    532c:	e24b00e4 	sub	r0, fp, #228	@ 0xe4
    5330:	e12fff33 	blx	r3
	raw_spin_lock_bh(&lock->rlock);
    5334:	e51b00e8 	ldr	r0, [fp, #-232]	@ 0xffffff18
    5338:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	while (quota--) {
    533c:	e2588001 	subs	r8, r8, #1
    5340:	0a000002 	beq	5350 <sfe_ipv4_periodic_sync+0x278>
		cm = si->active_head;
    5344:	e5173014 	ldr	r3, [r7, #-20]	@ 0xffffffec
		if (!cm) {
    5348:	e3530000 	cmp	r3, #0
    534c:	1affff7b 	bne	5140 <sfe_ipv4_periodic_sync+0x68>
    5350:	e51ba0e8 	ldr	sl, [fp, #-232]	@ 0xffffff18
	raw_spin_unlock_bh(&lock->rlock);
    5354:	e1a0000a 	mov	r0, sl
    5358:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	mod_timer(&si->timer, jiffies + ((HZ + 99) / 100));
    535c:	e3003000 	movw	r3, #0
    5360:	e1a00007 	mov	r0, r7
    5364:	e3403000 	movt	r3, #0
    5368:	e5931000 	ldr	r1, [r3]
    536c:	e2811001 	add	r1, r1, #1
    5370:	ebfffffe 	bl	0 <mod_timer>
}
    5374:	e24bd028 	sub	sp, fp, #40	@ 0x28
    5378:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

0000537c <sfe_ipv6_periodic_sync>:
{
    537c:	e1a0c00d 	mov	ip, sp
    5380:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    5384:	e24cb004 	sub	fp, ip, #4
    5388:	e24dd0cc 	sub	sp, sp, #204	@ 0xcc
    538c:	e1a05000 	mov	r5, r0
	now_jiffies = get_jiffies_64();
    5390:	ebfffffe 	bl	0 <get_jiffies_64>
    5394:	e1a07000 	mov	r7, r0
    5398:	e1a06001 	mov	r6, r1
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    539c:	e5953014 	ldr	r3, [r5, #20]
	if (!sync_rule_callback) {
    53a0:	e3530000 	cmp	r3, #0
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    53a4:	e50b30f4 	str	r3, [fp, #-244]	@ 0xffffff0c
	if (!sync_rule_callback) {
    53a8:	0a00009b 	beq	561c <sfe_ipv6_periodic_sync+0x2a0>
	struct sfe_ipv6 *si= from_timer(si, t, timer);
    53ac:	e2453018 	sub	r3, r5, #24
    53b0:	e1a04003 	mov	r4, r3
	raw_spin_lock_bh(&lock->rlock);
    53b4:	e1a00003 	mov	r0, r3
    53b8:	e50b30e8 	str	r3, [fp, #-232]	@ 0xffffff18
    53bc:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	sfe_ipv6_update_summary_stats(si);
    53c0:	e1a00004 	mov	r0, r4
    53c4:	ebffec49 	bl	4f0 <sfe_ipv6_update_summary_stats>
	quota = (si->num_connections + 63) / 64;
    53c8:	e5153004 	ldr	r3, [r5, #-4]
    53cc:	e283303f 	add	r3, r3, #63	@ 0x3f
	while (quota--) {
    53d0:	e1b08323 	lsrs	r8, r3, #6
    53d4:	0a00008e 	beq	5614 <sfe_ipv6_periodic_sync+0x298>
			counter_cm->active = false;
    53d8:	e3a04000 	mov	r4, #0
	sis->is_v6 = 1;
    53dc:	e50b80ec 	str	r8, [fp, #-236]	@ 0xffffff14
    53e0:	ea000088 	b	5608 <sfe_ipv6_periodic_sync+0x28c>
		counter_cm = cm->counter_match;
    53e4:	e592300c 	ldr	r3, [r2, #12]
		if (counter_cm->active) {
    53e8:	e5d31018 	ldrb	r1, [r3, #24]
    53ec:	e3510000 	cmp	r1, #0
    53f0:	0a00000a 	beq	5420 <sfe_ipv6_periodic_sync+0xa4>
			counter_cm->active_prev->active_next = counter_cm->active_next;
    53f4:	e5931010 	ldr	r1, [r3, #16]
			counter_cm->active = false;
    53f8:	e5c34018 	strb	r4, [r3, #24]
			counter_cm->active_prev->active_next = counter_cm->active_next;
    53fc:	e5930014 	ldr	r0, [r3, #20]
			if (likely(counter_cm->active_next)) {
    5400:	e3510000 	cmp	r1, #0
			counter_cm->active_prev->active_next = counter_cm->active_next;
    5404:	e5801010 	str	r1, [r0, #16]
				counter_cm->active_next->active_prev = counter_cm->active_prev;
    5408:	15930014 	ldrne	r0, [r3, #20]
				si->active_tail = counter_cm->active_prev;
    540c:	05931014 	ldreq	r1, [r3, #20]
				counter_cm->active_next->active_prev = counter_cm->active_prev;
    5410:	15810014 	strne	r0, [r1, #20]
				si->active_tail = counter_cm->active_prev;
    5414:	05051010 	streq	r1, [r5, #-16]
			counter_cm->active_next = NULL;
    5418:	e5834010 	str	r4, [r3, #16]
			counter_cm->active_prev = NULL;
    541c:	e5834014 	str	r4, [r3, #20]
		si->active_head = cm->active_next;
    5420:	e5923010 	ldr	r3, [r2, #16]
		cm->active = false;
    5424:	e5c24018 	strb	r4, [r2, #24]
	sis->is_v6 = 1;
    5428:	e3a01001 	mov	r1, #1
		if (likely(cm->active_next)) {
    542c:	e3530000 	cmp	r3, #0
		si->active_head = cm->active_next;
    5430:	e5053014 	str	r3, [r5, #-20]	@ 0xffffffec
			si->active_tail = NULL;
    5434:	05053010 	streq	r3, [r5, #-16]
			cm->active_next->active_prev = NULL;
    5438:	15834014 	strne	r4, [r3, #20]
		c = cm->connection;
    543c:	e5923008 	ldr	r3, [r2, #8]
	sis->is_v6 = 1;
    5440:	e50b10dc 	str	r1, [fp, #-220]	@ 0xffffff24
		cm->active_next = NULL;
    5444:	e5824010 	str	r4, [r2, #16]
	sis->protocol = c->protocol;
    5448:	e5931008 	ldr	r1, [r3, #8]
	reply_cm = c->reply_match;
    544c:	e593205c 	ldr	r2, [r3, #92]	@ 0x5c
	sis->protocol = c->protocol;
    5450:	e50b10d8 	str	r1, [fp, #-216]	@ 0xffffff28
	sis->src_ip.ip6[0] = c->src_ip[0];
    5454:	e1c300dc 	ldrd	r0, [r3, #12]
    5458:	e14b0df4 	strd	r0, [fp, #-212]	@ 0xffffff2c
    545c:	e1c301d4 	ldrd	r0, [r3, #20]
    5460:	e14b0cfc 	strd	r0, [fp, #-204]	@ 0xffffff34
	sis->src_ip_xlate.ip6[0] = c->src_ip_xlate[0];
    5464:	e1c301dc 	ldrd	r0, [r3, #28]
    5468:	e14b0cf4 	strd	r0, [fp, #-196]	@ 0xffffff3c
    546c:	e1c302d4 	ldrd	r0, [r3, #36]	@ 0x24
    5470:	e14b0bfc 	strd	r0, [fp, #-188]	@ 0xffffff44
	sis->dest_ip.ip6[0] = c->dest_ip[0];
    5474:	e1c302dc 	ldrd	r0, [r3, #44]	@ 0x2c
    5478:	e14b0bf0 	strd	r0, [fp, #-176]	@ 0xffffff50
    547c:	e1c303d4 	ldrd	r0, [r3, #52]	@ 0x34
    5480:	e14b0af8 	strd	r0, [fp, #-168]	@ 0xffffff58
	sis->dest_ip_xlate.ip6[0] = c->dest_ip_xlate[0];
    5484:	e1c303dc 	ldrd	r0, [r3, #60]	@ 0x3c
    5488:	e14b0af0 	strd	r0, [fp, #-160]	@ 0xffffff60
    548c:	e1c304d4 	ldrd	r0, [r3, #68]	@ 0x44
    5490:	e14b09f8 	strd	r0, [fp, #-152]	@ 0xffffff68
	sis->dest_port = c->dest_port;
    5494:	e1d315b0 	ldrh	r1, [r3, #80]	@ 0x50
	sis->src_port = c->src_port;
    5498:	e593004c 	ldr	r0, [r3, #76]	@ 0x4c
	sis->dest_port = c->dest_port;
    549c:	e14b19b0 	strh	r1, [fp, #-144]	@ 0xffffff70
	sis->dest_port_xlate = c->dest_port_xlate;
    54a0:	e1d315b2 	ldrh	r1, [r3, #82]	@ 0x52
	sis->src_port = c->src_port;
    54a4:	e50b00b4 	str	r0, [fp, #-180]	@ 0xffffff4c
	sis->dest_port_xlate = c->dest_port_xlate;
    54a8:	e14b18be 	strh	r1, [fp, #-142]	@ 0xffffff72
	original_cm = c->original_match;
    54ac:	e5931054 	ldr	r1, [r3, #84]	@ 0x54
	sis->src_td_max_window = original_cm->protocol_state.tcp.max_win;
    54b0:	e5910054 	ldr	r0, [r1, #84]	@ 0x54
    54b4:	e50b008c 	str	r0, [fp, #-140]	@ 0xffffff74
	sis->src_td_end = original_cm->protocol_state.tcp.end;
    54b8:	e5910058 	ldr	r0, [r1, #88]	@ 0x58
    54bc:	e50b0088 	str	r0, [fp, #-136]	@ 0xffffff78
	sis->src_td_max_end = original_cm->protocol_state.tcp.max_end;
    54c0:	e591005c 	ldr	r0, [r1, #92]	@ 0x5c
    54c4:	e50b0084 	str	r0, [fp, #-132]	@ 0xffffff7c
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
    54c8:	e5920054 	ldr	r0, [r2, #84]	@ 0x54
    54cc:	e50b0064 	str	r0, [fp, #-100]	@ 0xffffff9c
	sis->dest_td_end = reply_cm->protocol_state.tcp.end;
    54d0:	e5920058 	ldr	r0, [r2, #88]	@ 0x58
    54d4:	e50b0060 	str	r0, [fp, #-96]	@ 0xffffffa0
	sis->dest_td_max_end = reply_cm->protocol_state.tcp.max_end;
    54d8:	e592005c 	ldr	r0, [r2, #92]	@ 0x5c
    54dc:	e50b005c 	str	r0, [fp, #-92]	@ 0xffffffa4
	sis->src_new_packet_count = original_cm->rx_packet_count;
    54e0:	e5910060 	ldr	r0, [r1, #96]	@ 0x60
    54e4:	e50b006c 	str	r0, [fp, #-108]	@ 0xffffff94
	sis->src_new_byte_count = original_cm->rx_byte_count;
    54e8:	e5910064 	ldr	r0, [r1, #100]	@ 0x64
    54ec:	e50b0068 	str	r0, [fp, #-104]	@ 0xffffff98
	sis->dest_new_packet_count = reply_cm->rx_packet_count;
    54f0:	e5920060 	ldr	r0, [r2, #96]	@ 0x60
    54f4:	e50b0044 	str	r0, [fp, #-68]	@ 0xffffffbc
	sis->dest_new_byte_count = reply_cm->rx_byte_count;
    54f8:	e5920064 	ldr	r0, [r2, #100]	@ 0x64
    54fc:	e50b0040 	str	r0, [fp, #-64]	@ 0xffffffc0
	cm->rx_packet_count64 += cm->rx_packet_count;
    5500:	e591c060 	ldr	ip, [r1, #96]	@ 0x60
    5504:	e59100b0 	ldr	r0, [r1, #176]	@ 0xb0
	cm->rx_packet_count = 0;
    5508:	e5814060 	str	r4, [r1, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    550c:	e090000c 	adds	r0, r0, ip
	cm->rx_byte_count64 += cm->rx_byte_count;
    5510:	e591c064 	ldr	ip, [r1, #100]	@ 0x64
	cm->rx_byte_count = 0;
    5514:	e5814064 	str	r4, [r1, #100]	@ 0x64
	cm->rx_packet_count64 += cm->rx_packet_count;
    5518:	e58100b0 	str	r0, [r1, #176]	@ 0xb0
    551c:	e59100b4 	ldr	r0, [r1, #180]	@ 0xb4
    5520:	e2a00000 	adc	r0, r0, #0
    5524:	e58100b4 	str	r0, [r1, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    5528:	e59100b8 	ldr	r0, [r1, #184]	@ 0xb8
    552c:	e090000c 	adds	r0, r0, ip
    5530:	e58100b8 	str	r0, [r1, #184]	@ 0xb8
    5534:	e59100bc 	ldr	r0, [r1, #188]	@ 0xbc
    5538:	e2a00000 	adc	r0, r0, #0
    553c:	e58100bc 	str	r0, [r1, #188]	@ 0xbc
	cm->rx_packet_count64 += cm->rx_packet_count;
    5540:	e592e060 	ldr	lr, [r2, #96]	@ 0x60
	cm->rx_byte_count64 += cm->rx_byte_count;
    5544:	e592c064 	ldr	ip, [r2, #100]	@ 0x64
	cm->rx_packet_count = 0;
    5548:	e5824060 	str	r4, [r2, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    554c:	e59200b0 	ldr	r0, [r2, #176]	@ 0xb0
	cm->rx_byte_count = 0;
    5550:	e5824064 	str	r4, [r2, #100]	@ 0x64
	cm->rx_packet_count64 += cm->rx_packet_count;
    5554:	e592a0b4 	ldr	sl, [r2, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    5558:	e59290bc 	ldr	r9, [r2, #188]	@ 0xbc
	cm->rx_packet_count64 += cm->rx_packet_count;
    555c:	e09ee000 	adds	lr, lr, r0
	cm->rx_byte_count64 += cm->rx_byte_count;
    5560:	e59200b8 	ldr	r0, [r2, #184]	@ 0xb8
	cm->rx_packet_count64 += cm->rx_packet_count;
    5564:	e2aaa000 	adc	sl, sl, #0
    5568:	e582e0b0 	str	lr, [r2, #176]	@ 0xb0
    556c:	e582a0b4 	str	sl, [r2, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    5570:	e09cc000 	adds	ip, ip, r0
    5574:	e2a90000 	adc	r0, r9, #0
    5578:	e582c0b8 	str	ip, [r2, #184]	@ 0xb8
    557c:	e58200bc 	str	r0, [r2, #188]	@ 0xbc
	sis->src_packet_count = original_cm->rx_packet_count64;
    5580:	e1c18bd0 	ldrd	r8, [r1, #176]	@ 0xb0
	cm->rx_byte_count64 += cm->rx_byte_count;
    5584:	e50b00f0 	str	r0, [fp, #-240]	@ 0xffffff10
	sis->src_dev = original_cm->match_dev;
    5588:	e591001c 	ldr	r0, [r1, #28]
	sis->src_packet_count = original_cm->rx_packet_count64;
    558c:	e14b87fc 	strd	r8, [fp, #-124]	@ 0xffffff84
	sis->src_byte_count = original_cm->rx_byte_count64;
    5590:	e1c18bd8 	ldrd	r8, [r1, #184]	@ 0xb8
	sis->src_dev = original_cm->match_dev;
    5594:	e50b00e4 	str	r0, [fp, #-228]	@ 0xffffff1c
	raw_spin_unlock_bh(&lock->rlock);
    5598:	e51b00e8 	ldr	r0, [fp, #-232]	@ 0xffffff18
	sis->src_byte_count = original_cm->rx_byte_count64;
    559c:	e14b87f4 	strd	r8, [fp, #-116]	@ 0xffffff8c
	sis->dest_dev = reply_cm->match_dev;
    55a0:	e592201c 	ldr	r2, [r2, #28]
    55a4:	e50b20e0 	str	r2, [fp, #-224]	@ 0xffffff20
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    55a8:	e51b20f0 	ldr	r2, [fp, #-240]	@ 0xffffff10
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    55ac:	e50be054 	str	lr, [fp, #-84]	@ 0xffffffac
    55b0:	e50ba050 	str	sl, [fp, #-80]	@ 0xffffffb0
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    55b4:	e50bc04c 	str	ip, [fp, #-76]	@ 0xffffffb4
    55b8:	e50b2048 	str	r2, [fp, #-72]	@ 0xffffffb8
	sis->reason = reason;
    55bc:	e50b403c 	str	r4, [fp, #-60]	@ 0xffffffc4
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    55c0:	e5932068 	ldr	r2, [r3, #104]	@ 0x68
    55c4:	e593106c 	ldr	r1, [r3, #108]	@ 0x6c
    55c8:	e0572002 	subs	r2, r7, r2
    55cc:	e0c61001 	sbc	r1, r6, r1
    55d0:	e50b2034 	str	r2, [fp, #-52]	@ 0xffffffcc
    55d4:	e50b1030 	str	r1, [fp, #-48]	@ 0xffffffd0
	c->last_sync_jiffies = now_jiffies;
    55d8:	e583606c 	str	r6, [r3, #108]	@ 0x6c
    55dc:	e5837068 	str	r7, [r3, #104]	@ 0x68
    55e0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sync_rule_callback(&sis);
    55e4:	e51b30f4 	ldr	r3, [fp, #-244]	@ 0xffffff0c
    55e8:	e24b00e4 	sub	r0, fp, #228	@ 0xe4
    55ec:	e12fff33 	blx	r3
	raw_spin_lock_bh(&lock->rlock);
    55f0:	e51b00e8 	ldr	r0, [fp, #-232]	@ 0xffffff18
    55f4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	while (quota--) {
    55f8:	e51b30ec 	ldr	r3, [fp, #-236]	@ 0xffffff14
    55fc:	e2533001 	subs	r3, r3, #1
    5600:	e50b30ec 	str	r3, [fp, #-236]	@ 0xffffff14
    5604:	0a000002 	beq	5614 <sfe_ipv6_periodic_sync+0x298>
		cm = si->active_head;
    5608:	e5152014 	ldr	r2, [r5, #-20]	@ 0xffffffec
		if (!cm) {
    560c:	e3520000 	cmp	r2, #0
    5610:	1affff73 	bne	53e4 <sfe_ipv6_periodic_sync+0x68>
	raw_spin_unlock_bh(&lock->rlock);
    5614:	e51b00e8 	ldr	r0, [fp, #-232]	@ 0xffffff18
    5618:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	mod_timer(&si->timer, jiffies + ((HZ + 99) / 100));
    561c:	e3003000 	movw	r3, #0
    5620:	e1a00005 	mov	r0, r5
    5624:	e3403000 	movt	r3, #0
    5628:	e5931000 	ldr	r1, [r3]
    562c:	e2811001 	add	r1, r1, #1
    5630:	ebfffffe 	bl	0 <mod_timer>
}
    5634:	e24bd028 	sub	sp, fp, #40	@ 0x28
    5638:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

0000563c <sfe_ipv6_recv_tcp.constprop.0>:
static int sfe_ipv6_recv_tcp(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
    563c:	e1a0c00d 	mov	ip, sp
    5640:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    5644:	e24cb004 	sub	fp, ip, #4
    5648:	e24dd024 	sub	sp, sp, #36	@ 0x24
    564c:	e1a06003 	mov	r6, r3
    5650:	e5db3008 	ldrb	r3, [fp, #8]
    5654:	e1a04001 	mov	r4, r1
	return skb->len - skb->data_len;
    5658:	e5901060 	ldr	r1, [r0, #96]	@ 0x60
    565c:	e1a05000 	mov	r5, r0
    5660:	e50b303c 	str	r3, [fp, #-60]	@ 0xffffffc4
	if (!pskb_may_pull(skb, (sizeof(struct sfe_ipv6_tcp_hdr) + ihl))) {
    5664:	e59b3004 	ldr	r3, [fp, #4]
static int sfe_ipv6_recv_tcp(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
    5668:	e50b2038 	str	r2, [fp, #-56]	@ 0xffffffc8
    566c:	e590205c 	ldr	r2, [r0, #92]	@ 0x5c
	if (!pskb_may_pull(skb, (sizeof(struct sfe_ipv6_tcp_hdr) + ihl))) {
    5670:	e2833014 	add	r3, r3, #20
    5674:	e042c001 	sub	ip, r2, r1
	if (likely(len <= skb_headlen(skb)))
    5678:	e153000c 	cmp	r3, ip
    567c:	8a0000cf 	bhi	59c0 <sfe_ipv6_recv_tcp.constprop.0+0x384>
	tcph = (struct sfe_ipv6_tcp_hdr *)(skb->data + ihl);
    5680:	e59b2004 	ldr	r2, [fp, #4]
	raw_spin_lock_bh(&lock->rlock);
    5684:	e3008000 	movw	r8, #0
    5688:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
    568c:	e3408000 	movt	r8, #0
    5690:	e1a00008 	mov	r0, r8
    5694:	e0837002 	add	r7, r3, r2
	src_port = tcph->source;
    5698:	e19390b2 	ldrh	r9, [r3, r2]
	flags = tcp_flag_word(tcph);
    569c:	e597300c 	ldr	r3, [r7, #12]
    56a0:	e50b3040 	str	r3, [fp, #-64]	@ 0xffffffc0
	dest_port = tcph->dest;
    56a4:	e1d7a0b2 	ldrh	sl, [r7, #2]
    56a8:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	dest_ip = &iph->daddr;
    56ac:	e2862018 	add	r2, r6, #24
	cm = sfe_ipv6_find_connection_match(si, dev, IPPROTO_TCP, src_ip, src_port, dest_ip, dest_port);
    56b0:	e1a00004 	mov	r0, r4
    56b4:	e1a03009 	mov	r3, r9
    56b8:	e3a01006 	mov	r1, #6
    56bc:	e88d0404 	stm	sp, {r2, sl}
    56c0:	e2862008 	add	r2, r6, #8
    56c4:	ebffee20 	bl	f4c <sfe_ipv6_find_connection_match.constprop.0>
	if (unlikely(!cm)) {
    56c8:	e2504000 	subs	r4, r0, #0
    56cc:	0a0000cf 	beq	5a10 <sfe_ipv6_recv_tcp.constprop.0+0x3d4>
	if (unlikely(flush_on_find)) {
    56d0:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
    56d4:	e3530000 	cmp	r3, #0
    56d8:	1a0000da 	bne	5a48 <sfe_ipv6_recv_tcp.constprop.0+0x40c>
	if (unlikely(!cm->flow_accel)) {
    56dc:	e594304c 	ldr	r3, [r4, #76]	@ 0x4c
    56e0:	e3530000 	cmp	r3, #0
    56e4:	0a0000ad 	beq	59a0 <sfe_ipv6_recv_tcp.constprop.0+0x364>
	if (unlikely(iph->hop_limit < 2)) {
    56e8:	e5d63007 	ldrb	r3, [r6, #7]
    56ec:	e3530001 	cmp	r3, #1
    56f0:	9a00012f 	bls	5bb4 <sfe_ipv6_recv_tcp.constprop.0+0x578>
	if (unlikely((len > cm->xmit_dev_mtu) && !skb_is_gso(skb))) {
    56f4:	e51b2038 	ldr	r2, [fp, #-56]	@ 0xffffffc8
    56f8:	e1d439bc 	ldrh	r3, [r4, #156]	@ 0x9c
    56fc:	e1530002 	cmp	r3, r2
    5700:	3a00013a 	bcc	5bf0 <sfe_ipv6_recv_tcp.constprop.0+0x5b4>
	if (unlikely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) != TCP_FLAG_ACK)) {
    5704:	e51b3040 	ldr	r3, [fp, #-64]	@ 0xffffffc0
    5708:	e2039c17 	and	r9, r3, #5888	@ 0x1700
    570c:	e3590a01 	cmp	r9, #4096	@ 0x1000
    5710:	1a000149 	bne	5c3c <sfe_ipv6_recv_tcp.constprop.0+0x600>
	counter_cm = cm->counter_match;
    5714:	e594a00c 	ldr	sl, [r4, #12]
	if (likely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK))) {
    5718:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    571c:	e3130004 	tst	r3, #4
    5720:	1a00003e 	bne	5820 <sfe_ipv6_recv_tcp.constprop.0+0x1e4>
		seq = ntohl(tcph->seq);
    5724:	e5979004 	ldr	r9, [r7, #4]
		if (unlikely((s32)(seq - (cm->protocol_state.tcp.max_end + 1)) > 0)) {
    5728:	e594305c 	ldr	r3, [r4, #92]	@ 0x5c
		seq = ntohl(tcph->seq);
    572c:	e6bf9f39 	rev	r9, r9
		if (unlikely((s32)(seq - (cm->protocol_state.tcp.max_end + 1)) > 0)) {
    5730:	e0493003 	sub	r3, r9, r3
    5734:	e2433001 	sub	r3, r3, #1
    5738:	e3530000 	cmp	r3, #0
    573c:	ca00014d 	bgt	5c78 <sfe_ipv6_recv_tcp.constprop.0+0x63c>
		data_offs = tcph->doff << 2;
    5740:	e5d7100c 	ldrb	r1, [r7, #12]
    5744:	e1a01221 	lsr	r1, r1, #4
    5748:	e1a01101 	lsl	r1, r1, #2
		if (unlikely(data_offs < sizeof(struct sfe_ipv6_tcp_hdr))) {
    574c:	e3510013 	cmp	r1, #19
    5750:	9a000157 	bls	5cb4 <sfe_ipv6_recv_tcp.constprop.0+0x678>
		ack = ntohl(tcph->ack_seq);
    5754:	e5973008 	ldr	r3, [r7, #8]
		if (unlikely(!sfe_ipv6_process_tcp_option_sack(tcph, data_offs, &sack))) {
    5758:	e24b2030 	sub	r2, fp, #48	@ 0x30
    575c:	e1a00007 	mov	r0, r7
		ack = ntohl(tcph->ack_seq);
    5760:	e6bf3f33 	rev	r3, r3
		sack = ack;
    5764:	e50b303c 	str	r3, [fp, #-60]	@ 0xffffffc4
    5768:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
		if (unlikely(!sfe_ipv6_process_tcp_option_sack(tcph, data_offs, &sack))) {
    576c:	ebffebf3 	bl	740 <sfe_ipv4_process_tcp_option_sack>
    5770:	e3500000 	cmp	r0, #0
    5774:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
    5778:	0a00015c 	beq	5cf0 <sfe_ipv6_recv_tcp.constprop.0+0x6b4>
		if (unlikely(len < data_offs)) {
    577c:	e51b2038 	ldr	r2, [fp, #-56]	@ 0xffffffc8
		data_offs += sizeof(struct sfe_ipv6_ip_hdr);
    5780:	e2811028 	add	r1, r1, #40	@ 0x28
		if (unlikely(len < data_offs)) {
    5784:	e1520001 	cmp	r2, r1
    5788:	3a000167 	bcc	5d2c <sfe_ipv6_recv_tcp.constprop.0+0x6f0>
		end = seq + len - data_offs;
    578c:	e0421001 	sub	r1, r2, r1
    5790:	e0812009 	add	r2, r1, r9
		if (unlikely((s32)(end - (cm->protocol_state.tcp.end
    5794:	e59a1054 	ldr	r1, [sl, #84]	@ 0x54
    5798:	e5949058 	ldr	r9, [r4, #88]	@ 0x58
    579c:	e0411009 	sub	r1, r1, r9
    57a0:	e2811001 	add	r1, r1, #1
    57a4:	e1710002 	cmn	r1, r2
    57a8:	4a00016e 	bmi	5d68 <sfe_ipv6_recv_tcp.constprop.0+0x72c>
		if (unlikely((s32)(sack - (counter_cm->protocol_state.tcp.end + 1)) > 0)) {
    57ac:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
    57b0:	e59ac058 	ldr	ip, [sl, #88]	@ 0x58
    57b4:	e2410001 	sub	r0, r1, #1
    57b8:	e040000c 	sub	r0, r0, ip
    57bc:	e3500000 	cmp	r0, #0
    57c0:	ca0000af 	bgt	5a84 <sfe_ipv6_recv_tcp.constprop.0+0x448>
			    - cm->protocol_state.tcp.max_win
    57c4:	e594e054 	ldr	lr, [r4, #84]	@ 0x54
		if (unlikely((s32)(sack - left_edge) < 0)) {
    57c8:	e2810cff 	add	r0, r1, #65280	@ 0xff00
    57cc:	e28000f1 	add	r0, r0, #241	@ 0xf1
    57d0:	e080000e 	add	r0, r0, lr
    57d4:	e040000c 	sub	r0, r0, ip
    57d8:	e3500000 	cmp	r0, #0
    57dc:	ba000170 	blt	5da4 <sfe_ipv6_recv_tcp.constprop.0+0x768>
		scaled_win = ntohs(tcph->window) << cm->protocol_state.tcp.win_scale;
    57e0:	e1d700be 	ldrh	r0, [r7, #14]
		scaled_win += (sack - ack);
    57e4:	e0413003 	sub	r3, r1, r3
		if (likely((s32)(end - cm->protocol_state.tcp.end) >= 0)) {
    57e8:	e0429009 	sub	r9, r2, r9
		scaled_win = ntohs(tcph->window) << cm->protocol_state.tcp.win_scale;
    57ec:	e5d4c050 	ldrb	ip, [r4, #80]	@ 0x50
    57f0:	e6bf0fb0 	rev16	r0, r0
    57f4:	e6ff0070 	uxth	r0, r0
		scaled_win += (sack - ack);
    57f8:	e083cc10 	add	ip, r3, r0, lsl ip
		if (unlikely(cm->protocol_state.tcp.max_win < scaled_win)) {
    57fc:	e15e000c 	cmp	lr, ip
		max_end = sack + scaled_win;
    5800:	e081100c 	add	r1, r1, ip
			cm->protocol_state.tcp.max_win = scaled_win;
    5804:	3584c054 	strcc	ip, [r4, #84]	@ 0x54
		if (likely((s32)(end - cm->protocol_state.tcp.end) >= 0)) {
    5808:	e3590000 	cmp	r9, #0
			cm->protocol_state.tcp.end = end;
    580c:	a5842058 	strge	r2, [r4, #88]	@ 0x58
		if (likely((s32)(max_end - counter_cm->protocol_state.tcp.max_end) >= 0)) {
    5810:	e59a305c 	ldr	r3, [sl, #92]	@ 0x5c
    5814:	e0413003 	sub	r3, r1, r3
    5818:	e3530000 	cmp	r3, #0
			counter_cm->protocol_state.tcp.max_end = max_end;
    581c:	a58a105c 	strge	r1, [sl, #92]	@ 0x5c
	return skb->cloned &&
    5820:	e5d5306a 	ldrb	r3, [r5, #106]	@ 0x6a
    5824:	e3130001 	tst	r3, #1
    5828:	0a000014 	beq	5880 <sfe_ipv6_recv_tcp.constprop.0+0x244>
	return skb->end;
    582c:	e59520a8 	ldr	r2, [r5, #168]	@ 0xa8
    5830:	e5923020 	ldr	r3, [r2, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    5834:	e6ff3073 	uxth	r3, r3
	return skb->cloned &&
    5838:	e3530001 	cmp	r3, #1
    583c:	0a00000f 	beq	5880 <sfe_ipv6_recv_tcp.constprop.0+0x244>
    5840:	e5923020 	ldr	r3, [r2, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    5844:	e6ff3073 	uxth	r3, r3
	return skb->cloned &&
    5848:	e3530001 	cmp	r3, #1
    584c:	0a0000cc 	beq	5b84 <sfe_ipv6_recv_tcp.constprop.0+0x548>
		struct sk_buff *nskb = skb_copy(skb, pri);
    5850:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    5854:	e1a00005 	mov	r0, r5
    5858:	ebfffffe 	bl	0 <skb_copy>
		if (likely(nskb))
    585c:	e2506000 	subs	r6, r0, #0
    5860:	0a000175 	beq	5e3c <sfe_ipv6_recv_tcp.constprop.0+0x800>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    5864:	e1a00005 	mov	r0, r5
    5868:	e3a01001 	mov	r1, #1
    586c:	e1a05006 	mov	r5, r6
    5870:	ebfffffe 	bl	0 <kfree_skb_reason>
		tcph = (struct sfe_ipv6_tcp_hdr *)(skb->data + ihl);
    5874:	e59b3004 	ldr	r3, [fp, #4]
		iph = (struct sfe_ipv6_ip_hdr *)skb->data;
    5878:	e59560b0 	ldr	r6, [r5, #176]	@ 0xb0
		tcph = (struct sfe_ipv6_tcp_hdr *)(skb->data + ihl);
    587c:	e0867003 	add	r7, r6, r3
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
    5880:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    5884:	e3130040 	tst	r3, #64	@ 0x40
    5888:	1a00008c 	bne	5ac0 <sfe_ipv6_recv_tcp.constprop.0+0x484>
	iph->hop_limit -= 1;
    588c:	e5d63007 	ldrb	r3, [r6, #7]
    5890:	e2433001 	sub	r3, r3, #1
    5894:	e5c63007 	strb	r3, [r6, #7]
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
    5898:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    589c:	e3130001 	tst	r3, #1
    58a0:	1a00008f 	bne	5ae4 <sfe_ipv6_recv_tcp.constprop.0+0x4a8>
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    58a4:	e3130002 	tst	r3, #2
    58a8:	1a00009d 	bne	5b24 <sfe_ipv6_recv_tcp.constprop.0+0x4e8>
	cm->rx_byte_count += len;
    58ac:	e51b1038 	ldr	r1, [fp, #-56]	@ 0xffffffc8
    58b0:	e1c426d0 	ldrd	r2, [r4, #96]	@ 0x60
    58b4:	e0833001 	add	r3, r3, r1
	cm->rx_packet_count++;
    58b8:	e2822001 	add	r2, r2, #1
	cm->rx_byte_count += len;
    58bc:	e1c426f0 	strd	r2, [r4, #96]	@ 0x60
	if (unlikely(!cm->active)) {
    58c0:	e5d43018 	ldrb	r3, [r4, #24]
    58c4:	e3530000 	cmp	r3, #0
    58c8:	0a0000a4 	beq	5b60 <sfe_ipv6_recv_tcp.constprop.0+0x524>
	xmit_dev = cm->xmit_dev;
    58cc:	e5941098 	ldr	r1, [r4, #152]	@ 0x98
	skb->dev = xmit_dev;
    58d0:	e5851008 	str	r1, [r5, #8]
	if (likely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
    58d4:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    58d8:	e3130010 	tst	r3, #16
    58dc:	0a000018 	beq	5944 <sfe_ipv6_recv_tcp.constprop.0+0x308>
		if (unlikely(!(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
    58e0:	e3130008 	tst	r3, #8
    58e4:	0a00013d 	beq	5de0 <sfe_ipv6_recv_tcp.constprop.0+0x7a4>
	skb->len  += len;
    58e8:	e595205c 	ldr	r2, [r5, #92]	@ 0x5c
			eth->h_proto = htons(ETH_P_IPV6);
    58ec:	e3e00079 	mvn	r0, #121	@ 0x79
    58f0:	e3e01022 	mvn	r1, #34	@ 0x22
	skb->data -= len;
    58f4:	e59530b0 	ldr	r3, [r5, #176]	@ 0xb0
	skb->len  += len;
    58f8:	e282200e 	add	r2, r2, #14
	skb->data -= len;
    58fc:	e243c00e 	sub	ip, r3, #14
	skb->len  += len;
    5900:	e585205c 	str	r2, [r5, #92]	@ 0x5c
	skb->data -= len;
    5904:	e585c0b0 	str	ip, [r5, #176]	@ 0xb0
    5908:	e5430002 	strb	r0, [r3, #-2]
    590c:	e5431001 	strb	r1, [r3, #-1]
			eth->h_dest[0] = cm->xmit_dest_mac[0];
    5910:	e1d429be 	ldrh	r2, [r4, #158]	@ 0x9e
    5914:	e14320be 	strh	r2, [r3, #-14]
			eth->h_dest[1] = cm->xmit_dest_mac[1];
    5918:	e1d42ab0 	ldrh	r2, [r4, #160]	@ 0xa0
    591c:	e14320bc 	strh	r2, [r3, #-12]
			eth->h_dest[2] = cm->xmit_dest_mac[2];
    5920:	e1d42ab2 	ldrh	r2, [r4, #162]	@ 0xa2
    5924:	e14320ba 	strh	r2, [r3, #-10]
			eth->h_source[0] = cm->xmit_src_mac[0];
    5928:	e1d42ab4 	ldrh	r2, [r4, #164]	@ 0xa4
    592c:	e14320b8 	strh	r2, [r3, #-8]
			eth->h_source[1] = cm->xmit_src_mac[1];
    5930:	e1d42ab6 	ldrh	r2, [r4, #166]	@ 0xa6
    5934:	e14320b6 	strh	r2, [r3, #-6]
			eth->h_source[2] = cm->xmit_src_mac[2];
    5938:	e1d42ab8 	ldrh	r2, [r4, #168]	@ 0xa8
    593c:	e14320b4 	strh	r2, [r3, #-4]
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    5940:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    5944:	e3130020 	tst	r3, #32
		skb->priority = cm->priority;
    5948:	15943090 	ldrne	r3, [r4, #144]	@ 0x90
    594c:	15853078 	strne	r3, [r5, #120]	@ 0x78
	skb->mark = cm->connection->mark;
    5950:	e5942008 	ldr	r2, [r4, #8]
	si->packets_forwarded++;
    5954:	e59f34f0 	ldr	r3, [pc, #1264]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
	skb->mark = cm->connection->mark;
    5958:	e5922078 	ldr	r2, [r2, #120]	@ 0x78
	raw_spin_unlock_bh(&lock->rlock);
    595c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
    5960:	e5852090 	str	r2, [r5, #144]	@ 0x90
	si->packets_forwarded++;
    5964:	e593204c 	ldr	r2, [r3, #76]	@ 0x4c
    5968:	e2822001 	add	r2, r2, #1
    596c:	e583204c 	str	r2, [r3, #76]	@ 0x4c
    5970:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    5974:	e59530a8 	ldr	r3, [r5, #168]	@ 0xa8
    5978:	f5d3f000 	pld	[r3]
	skb->fast_forwarded = 1;
    597c:	e5d53071 	ldrb	r3, [r5, #113]	@ 0x71
	return __dev_queue_xmit(skb, NULL);
    5980:	e3a01000 	mov	r1, #0
    5984:	e1a00005 	mov	r0, r5
    5988:	e3833008 	orr	r3, r3, #8
    598c:	e5c53071 	strb	r3, [r5, #113]	@ 0x71
    5990:	ebfffffe 	bl	0 <__dev_queue_xmit>
	return 1;
    5994:	e3a00001 	mov	r0, #1
}
    5998:	e24bd028 	sub	sp, fp, #40	@ 0x28
    599c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		si->packets_not_forwarded++;
    59a0:	e2882902 	add	r2, r8, #32768	@ 0x8000
    59a4:	e1a00008 	mov	r0, r8
    59a8:	e5923050 	ldr	r3, [r2, #80]	@ 0x50
    59ac:	e2833001 	add	r3, r3, #1
    59b0:	e5823050 	str	r3, [r2, #80]	@ 0x50
    59b4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    59b8:	e3a00000 	mov	r0, #0
    59bc:	eafffff5 	b	5998 <sfe_ipv6_recv_tcp.constprop.0+0x35c>
	if (unlikely(len > skb->len))
    59c0:	e1530002 	cmp	r3, r2
    59c4:	8a000004 	bhi	59dc <sfe_ipv6_recv_tcp.constprop.0+0x3a0>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    59c8:	e0433002 	sub	r3, r3, r2
    59cc:	e0831001 	add	r1, r3, r1
    59d0:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, (sizeof(struct sfe_ipv6_tcp_hdr) + ihl))) {
    59d4:	e3500000 	cmp	r0, #0
    59d8:	1affff28 	bne	5680 <sfe_ipv6_recv_tcp.constprop.0+0x44>
	raw_spin_lock_bh(&lock->rlock);
    59dc:	e3000000 	movw	r0, #0
    59e0:	e3400000 	movt	r0, #0
    59e4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    59e8:	e59f345c 	ldr	r3, [pc, #1116]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
		si->packets_not_forwarded++;
    59ec:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    59f0:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    59f4:	e5932068 	ldr	r2, [r3, #104]	@ 0x68
		si->packets_not_forwarded++;
    59f8:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    59fc:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5a00:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    5a04:	e5832068 	str	r2, [r3, #104]	@ 0x68
    5a08:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    5a0c:	eaffffe9 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    5a10:	e51b2040 	ldr	r2, [fp, #-64]	@ 0xffffffc0
			si->packets_not_forwarded++;
    5a14:	e2883902 	add	r3, r8, #32768	@ 0x8000
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    5a18:	e2029c17 	and	r9, r2, #5888	@ 0x1700
			si->packets_not_forwarded++;
    5a1c:	e5932050 	ldr	r2, [r3, #80]	@ 0x50
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    5a20:	e3590a01 	cmp	r9, #4096	@ 0x1000
			si->packets_not_forwarded++;
    5a24:	e2822001 	add	r2, r2, #1
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    5a28:	1a0000fc 	bne	5e20 <sfe_ipv6_recv_tcp.constprop.0+0x7e4>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS]++;
    5a2c:	e5931070 	ldr	r1, [r3, #112]	@ 0x70
			si->packets_not_forwarded++;
    5a30:	e5832050 	str	r2, [r3, #80]	@ 0x50
    5a34:	e1a00008 	mov	r0, r8
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS]++;
    5a38:	e2812001 	add	r2, r1, #1
    5a3c:	e5832070 	str	r2, [r3, #112]	@ 0x70
    5a40:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    5a44:	eaffffdb 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
		struct sfe_ipv6_connection *c = cm->connection;
    5a48:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    5a4c:	e1a00004 	mov	r0, r4
    5a50:	ebfff283 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    5a54:	e2883902 	add	r3, r8, #32768	@ 0x8000
    5a58:	e1a00008 	mov	r0, r8
		si->packets_not_forwarded++;
    5a5c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    5a60:	e5932074 	ldr	r2, [r3, #116]	@ 0x74
		si->packets_not_forwarded++;
    5a64:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    5a68:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5a6c:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    5a70:	e5832074 	str	r2, [r3, #116]	@ 0x74
    5a74:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5a78:	e1a00004 	mov	r0, r4
    5a7c:	ebffef28 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    5a80:	eaffffcc 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5a84:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5a88:	e1a00004 	mov	r0, r4
    5a8c:	ebfff274 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    5a90:	e59f33b4 	ldr	r3, [pc, #948]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5a94:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5a98:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    5a9c:	e5932098 	ldr	r2, [r3, #152]	@ 0x98
			si->packets_not_forwarded++;
    5aa0:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    5aa4:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5aa8:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    5aac:	e5832098 	str	r2, [r3, #152]	@ 0x98
    5ab0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5ab4:	e1a00004 	mov	r0, r4
    5ab8:	ebffef19 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5abc:	eaffffbd 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
	*p = ((*p & htons(SFE_IPV6_DSCP_MASK)) | htons((u16)dscp << 4));
    5ac0:	e5d42094 	ldrb	r2, [r4, #148]	@ 0x94
    5ac4:	e3033ff0 	movw	r3, #16368	@ 0x3ff0
    5ac8:	e1d610b0 	ldrh	r1, [r6]
    5acc:	e1a02202 	lsl	r2, r2, #4
    5ad0:	e0033001 	and	r3, r3, r1
    5ad4:	e6bf2fb2 	rev16	r2, r2
    5ad8:	e1833002 	orr	r3, r3, r2
    5adc:	e1c630b0 	strh	r3, [r6]
}
    5ae0:	eaffff69 	b	588c <sfe_ipv6_recv_tcp.constprop.0+0x250>
		iph->saddr = cm->xlate_src_ip[0];
    5ae4:	e1c426d8 	ldrd	r2, [r4, #104]	@ 0x68
    5ae8:	e5862008 	str	r2, [r6, #8]
    5aec:	e586300c 	str	r3, [r6, #12]
    5af0:	e1c427d0 	ldrd	r2, [r4, #112]	@ 0x70
    5af4:	e5862010 	str	r2, [r6, #16]
    5af8:	e5863014 	str	r3, [r6, #20]
		tcph->source = cm->xlate_src_port;
    5afc:	e1d437b8 	ldrh	r3, [r4, #120]	@ 0x78
    5b00:	e1c730b0 	strh	r3, [r7]
		tcp_csum = tcph->check;
    5b04:	e1d731b0 	ldrh	r3, [r7, #16]
		sum = tcp_csum + cm->xlate_src_csum_adjustment;
    5b08:	e1d427ba 	ldrh	r2, [r4, #122]	@ 0x7a
    5b0c:	e0833002 	add	r3, r3, r2
		sum = (sum & 0xffff) + (sum >> 16);
    5b10:	e6ff2073 	uxth	r2, r3
    5b14:	e0823823 	add	r3, r2, r3, lsr #16
		tcph->check = (u16)sum;
    5b18:	e1c731b0 	strh	r3, [r7, #16]
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    5b1c:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    5b20:	eaffff5f 	b	58a4 <sfe_ipv6_recv_tcp.constprop.0+0x268>
		iph->daddr = cm->xlate_dest_ip[0];
    5b24:	e1c427dc 	ldrd	r2, [r4, #124]	@ 0x7c
    5b28:	e5862018 	str	r2, [r6, #24]
    5b2c:	e586301c 	str	r3, [r6, #28]
    5b30:	e1c428d4 	ldrd	r2, [r4, #132]	@ 0x84
    5b34:	e5862020 	str	r2, [r6, #32]
    5b38:	e5863024 	str	r3, [r6, #36]	@ 0x24
		tcph->dest = cm->xlate_dest_port;
    5b3c:	e1d438bc 	ldrh	r3, [r4, #140]	@ 0x8c
    5b40:	e1c730b2 	strh	r3, [r7, #2]
		tcp_csum = tcph->check;
    5b44:	e1d731b0 	ldrh	r3, [r7, #16]
		sum = tcp_csum + cm->xlate_dest_csum_adjustment;
    5b48:	e1d428be 	ldrh	r2, [r4, #142]	@ 0x8e
    5b4c:	e0833002 	add	r3, r3, r2
		sum = (sum & 0xffff) + (sum >> 16);
    5b50:	e6ff2073 	uxth	r2, r3
    5b54:	e0823823 	add	r3, r2, r3, lsr #16
		tcph->check = (u16)sum;
    5b58:	e1c731b0 	strh	r3, [r7, #16]
    5b5c:	eaffff52 	b	58ac <sfe_ipv6_recv_tcp.constprop.0+0x270>
		cm->active_prev = si->active_tail;
    5b60:	e5983008 	ldr	r3, [r8, #8]
		cm->active = true;
    5b64:	e3a02001 	mov	r2, #1
		si->active_tail = cm;
    5b68:	e5884008 	str	r4, [r8, #8]
		if (likely(si->active_tail)) {
    5b6c:	e3530000 	cmp	r3, #0
		cm->active_prev = si->active_tail;
    5b70:	e5843014 	str	r3, [r4, #20]
		cm->active = true;
    5b74:	e5c42018 	strb	r2, [r4, #24]
			si->active_head = cm;
    5b78:	05884004 	streq	r4, [r8, #4]
			si->active_tail->active_next = cm;
    5b7c:	15834010 	strne	r4, [r3, #16]
		si->active_tail = cm;
    5b80:	eaffff51 	b	58cc <sfe_ipv6_recv_tcp.constprop.0+0x290>
                if (!skb) {
    5b84:	e3550000 	cmp	r5, #0
    5b88:	1affff39 	bne	5874 <sfe_ipv6_recv_tcp.constprop.0+0x238>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    5b8c:	e59f32b8 	ldr	r3, [pc, #696]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5b90:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5b94:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    5b98:	e59320e8 	ldr	r2, [r3, #232]	@ 0xe8
			si->packets_not_forwarded++;
    5b9c:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    5ba0:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5ba4:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    5ba8:	e58320e8 	str	r2, [r3, #232]	@ 0xe8
    5bac:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    5bb0:	eaffff80 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
		struct sfe_ipv6_connection *c = cm->connection;
    5bb4:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    5bb8:	e1a00004 	mov	r0, r4
    5bbc:	ebfff228 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    5bc0:	e2883902 	add	r3, r8, #32768	@ 0x8000
    5bc4:	e1a00008 	mov	r0, r8
		si->packets_not_forwarded++;
    5bc8:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    5bcc:	e5932078 	ldr	r2, [r3, #120]	@ 0x78
		si->packets_not_forwarded++;
    5bd0:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    5bd4:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5bd8:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    5bdc:	e5832078 	str	r2, [r3, #120]	@ 0x78
    5be0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5be4:	e1a00004 	mov	r0, r4
    5be8:	ebffeecd 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    5bec:	eaffff71 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
	return csum_fold(csum_partial(csum_start, plen, partial));
}

static inline bool skb_is_gso(const struct sk_buff *skb)
{
	return skb_shinfo(skb)->gso_size;
    5bf0:	e59530a8 	ldr	r3, [r5, #168]	@ 0xa8
	if (unlikely((len > cm->xmit_dev_mtu) && !skb_is_gso(skb))) {
    5bf4:	e1d330b4 	ldrh	r3, [r3, #4]
    5bf8:	e3530000 	cmp	r3, #0
    5bfc:	1afffec0 	bne	5704 <sfe_ipv6_recv_tcp.constprop.0+0xc8>
		struct sfe_ipv6_connection *c = cm->connection;
    5c00:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    5c04:	e1a00004 	mov	r0, r4
    5c08:	ebfff215 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    5c0c:	e2883902 	add	r3, r8, #32768	@ 0x8000
    5c10:	e1a00008 	mov	r0, r8
		si->packets_not_forwarded++;
    5c14:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    5c18:	e593207c 	ldr	r2, [r3, #124]	@ 0x7c
		si->packets_not_forwarded++;
    5c1c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    5c20:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5c24:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    5c28:	e583207c 	str	r2, [r3, #124]	@ 0x7c
    5c2c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5c30:	e1a00004 	mov	r0, r4
    5c34:	ebffeeba 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    5c38:	eaffff5e 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
		struct sfe_ipv6_connection *c = cm->connection;
    5c3c:	e5944008 	ldr	r4, [r4, #8]
		sfe_ipv6_remove_connection(si, c);
    5c40:	e1a00004 	mov	r0, r4
    5c44:	ebfff206 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_FLAGS]++;
    5c48:	e59f31fc 	ldr	r3, [pc, #508]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
		si->packets_not_forwarded++;
    5c4c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5c50:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_FLAGS]++;
    5c54:	e5932080 	ldr	r2, [r3, #128]	@ 0x80
		si->packets_not_forwarded++;
    5c58:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_FLAGS]++;
    5c5c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    5c60:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_FLAGS]++;
    5c64:	e5832080 	str	r2, [r3, #128]	@ 0x80
    5c68:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5c6c:	e1a00004 	mov	r0, r4
    5c70:	ebffeeab 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
		return 0;
    5c74:	eaffff4f 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5c78:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5c7c:	e1a00004 	mov	r0, r4
    5c80:	ebfff1f7 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    5c84:	e59f31c0 	ldr	r3, [pc, #448]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5c88:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5c8c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    5c90:	e5932084 	ldr	r2, [r3, #132]	@ 0x84
			si->packets_not_forwarded++;
    5c94:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    5c98:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5c9c:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    5ca0:	e5832084 	str	r2, [r3, #132]	@ 0x84
    5ca4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5ca8:	e1a00004 	mov	r0, r4
    5cac:	ebffee9c 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5cb0:	eaffff40 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5cb4:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5cb8:	e1a00004 	mov	r0, r4
    5cbc:	ebfff1e8 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    5cc0:	e59f3184 	ldr	r3, [pc, #388]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5cc4:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5cc8:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    5ccc:	e5932088 	ldr	r2, [r3, #136]	@ 0x88
			si->packets_not_forwarded++;
    5cd0:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    5cd4:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5cd8:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    5cdc:	e5832088 	str	r2, [r3, #136]	@ 0x88
    5ce0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5ce4:	e1a00004 	mov	r0, r4
    5ce8:	ebffee8d 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5cec:	eaffff31 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5cf0:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5cf4:	e1a00004 	mov	r0, r4
    5cf8:	ebfff1d9 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    5cfc:	e59f3148 	ldr	r3, [pc, #328]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5d00:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5d04:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    5d08:	e593208c 	ldr	r2, [r3, #140]	@ 0x8c
			si->packets_not_forwarded++;
    5d0c:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    5d10:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5d14:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    5d18:	e583208c 	str	r2, [r3, #140]	@ 0x8c
    5d1c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5d20:	e1a00004 	mov	r0, r4
    5d24:	ebffee7e 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5d28:	eaffff22 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5d2c:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5d30:	e1a00004 	mov	r0, r4
    5d34:	ebfff1ca 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    5d38:	e59f310c 	ldr	r3, [pc, #268]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5d3c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5d40:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    5d44:	e5932090 	ldr	r2, [r3, #144]	@ 0x90
			si->packets_not_forwarded++;
    5d48:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    5d4c:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5d50:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    5d54:	e5832090 	str	r2, [r3, #144]	@ 0x90
    5d58:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5d5c:	e1a00004 	mov	r0, r4
    5d60:	ebffee6f 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5d64:	eaffff13 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5d68:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5d6c:	e1a00004 	mov	r0, r4
    5d70:	ebfff1bb 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    5d74:	e59f30d0 	ldr	r3, [pc, #208]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5d78:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5d7c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    5d80:	e5932094 	ldr	r2, [r3, #148]	@ 0x94
			si->packets_not_forwarded++;
    5d84:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    5d88:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5d8c:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    5d90:	e5832094 	str	r2, [r3, #148]	@ 0x94
    5d94:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5d98:	e1a00004 	mov	r0, r4
    5d9c:	ebffee60 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5da0:	eaffff04 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
			struct sfe_ipv6_connection *c = cm->connection;
    5da4:	e5944008 	ldr	r4, [r4, #8]
			sfe_ipv6_remove_connection(si, c);
    5da8:	e1a00004 	mov	r0, r4
    5dac:	ebfff1ac 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    5db0:	e59f3094 	ldr	r3, [pc, #148]	@ 5e4c <sfe_ipv6_recv_tcp.constprop.0+0x810>
			si->packets_not_forwarded++;
    5db4:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    5db8:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    5dbc:	e593209c 	ldr	r2, [r3, #156]	@ 0x9c
			si->packets_not_forwarded++;
    5dc0:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    5dc4:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    5dc8:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    5dcc:	e583209c 	str	r2, [r3, #156]	@ 0x9c
    5dd0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    5dd4:	e1a00004 	mov	r0, r4
    5dd8:	ebffee51 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
			return 0;
    5ddc:	eafffef5 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
	if (!dev->header_ops || !dev->header_ops->create)
    5de0:	e5912158 	ldr	r2, [r1, #344]	@ 0x158
    5de4:	e3520000 	cmp	r2, #0
    5de8:	0afffed5 	beq	5944 <sfe_ipv6_recv_tcp.constprop.0+0x308>
    5dec:	e5926000 	ldr	r6, [r2]
    5df0:	e3560000 	cmp	r6, #0
    5df4:	0afffed2 	beq	5944 <sfe_ipv6_recv_tcp.constprop.0+0x308>
	return dev->header_ops->create(skb, dev, type, daddr, saddr, len);
    5df8:	e51b2038 	ldr	r2, [fp, #-56]	@ 0xffffffc8
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
    5dfc:	e28430a4 	add	r3, r4, #164	@ 0xa4
    5e00:	e1a00005 	mov	r0, r5
    5e04:	e58d3000 	str	r3, [sp]
    5e08:	e284309e 	add	r3, r4, #158	@ 0x9e
    5e0c:	e58d2004 	str	r2, [sp, #4]
    5e10:	e30826dd 	movw	r2, #34525	@ 0x86dd
    5e14:	e12fff36 	blx	r6
	if (unlikely(cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    5e18:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    5e1c:	eafffec8 	b	5944 <sfe_ipv6_recv_tcp.constprop.0+0x308>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS]++;
    5e20:	e593106c 	ldr	r1, [r3, #108]	@ 0x6c
		si->packets_not_forwarded++;
    5e24:	e5832050 	str	r2, [r3, #80]	@ 0x50
    5e28:	e1a00008 	mov	r0, r8
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS]++;
    5e2c:	e2812001 	add	r2, r1, #1
    5e30:	e583206c 	str	r2, [r3, #108]	@ 0x6c
    5e34:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    5e38:	eafffede 	b	59b8 <sfe_ipv6_recv_tcp.constprop.0+0x37c>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    5e3c:	e3a01001 	mov	r1, #1
    5e40:	e1a00005 	mov	r0, r5
    5e44:	ebfffffe 	bl	0 <kfree_skb_reason>
                if (!skb) {
    5e48:	eaffff4f 	b	5b8c <sfe_ipv6_recv_tcp.constprop.0+0x550>
    5e4c:	00010050 	.word	0x00010050

00005e50 <sfe_ipv4_recv_tcp.constprop.0>:
static int sfe_ipv4_recv_tcp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
    5e50:	e1a0c00d 	mov	ip, sp
    5e54:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    5e58:	e24cb004 	sub	fp, ip, #4
    5e5c:	e24dd02c 	sub	sp, sp, #44	@ 0x2c
    5e60:	e1a05003 	mov	r5, r3
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_tcp_hdr) + ihl)))) {
    5e64:	e59b3004 	ldr	r3, [fp, #4]
static int sfe_ipv4_recv_tcp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
    5e68:	e50b103c 	str	r1, [fp, #-60]	@ 0xffffffc4
	return skb->len - skb->data_len;
    5e6c:	e5901060 	ldr	r1, [r0, #96]	@ 0x60
    5e70:	e50b2038 	str	r2, [fp, #-56]	@ 0xffffffc8
    5e74:	e1a04000 	mov	r4, r0
    5e78:	e590205c 	ldr	r2, [r0, #92]	@ 0x5c
    5e7c:	e5dba008 	ldrb	sl, [fp, #8]
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_tcp_hdr) + ihl)))) {
    5e80:	e2833014 	add	r3, r3, #20
    5e84:	e042c001 	sub	ip, r2, r1
	if (likely(len <= skb_headlen(skb)))
    5e88:	e153000c 	cmp	r3, ip
    5e8c:	8a0000e1 	bhi	6218 <sfe_ipv4_recv_tcp.constprop.0+0x3c8>
	src_ip = iph->saddr;
    5e90:	e595200c 	ldr	r2, [r5, #12]
	raw_spin_lock_bh(&lock->rlock);
    5e94:	e3009000 	movw	r9, #0
	tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
    5e98:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
    5e9c:	e3409000 	movt	r9, #0
    5ea0:	e1a00009 	mov	r0, r9
	src_ip = iph->saddr;
    5ea4:	e50b2044 	str	r2, [fp, #-68]	@ 0xffffffbc
	tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
    5ea8:	e59b2004 	ldr	r2, [fp, #4]
	dest_ip = iph->daddr;
    5eac:	e5957010 	ldr	r7, [r5, #16]
	tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
    5eb0:	e0836002 	add	r6, r3, r2
	src_port = tcph->source;
    5eb4:	e19330b2 	ldrh	r3, [r3, r2]
    5eb8:	e50b3040 	str	r3, [fp, #-64]	@ 0xffffffc0
	flags = tcp_flag_word(tcph);
    5ebc:	e596300c 	ldr	r3, [r6, #12]
    5ec0:	e50b3048 	str	r3, [fp, #-72]	@ 0xffffffb8
	dest_port = tcph->dest;
    5ec4:	e1d680b2 	ldrh	r8, [r6, #2]
    5ec8:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, IPPROTO_TCP, src_ip, src_port, dest_ip, dest_port);
    5ecc:	e3a01006 	mov	r1, #6
    5ed0:	e88d0180 	stm	sp, {r7, r8}
    5ed4:	e14b24d4 	ldrd	r2, [fp, #-68]	@ 0xffffffbc
    5ed8:	e51b003c 	ldr	r0, [fp, #-60]	@ 0xffffffc4
    5edc:	ebffecb6 	bl	11bc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0>
	if (unlikely(!cm)) {
    5ee0:	e2507000 	subs	r7, r0, #0
    5ee4:	0a0000e0 	beq	626c <sfe_ipv4_recv_tcp.constprop.0+0x41c>
	if (unlikely(flush_on_find)) {
    5ee8:	e35a0000 	cmp	sl, #0
    5eec:	1a0000ec 	bne	62a4 <sfe_ipv4_recv_tcp.constprop.0+0x454>
	if (unlikely(!cm->flow_accel)) {
    5ef0:	e5973034 	ldr	r3, [r7, #52]	@ 0x34
    5ef4:	e3530000 	cmp	r3, #0
    5ef8:	0a0000f8 	beq	62e0 <sfe_ipv4_recv_tcp.constprop.0+0x490>
	ttl = iph->ttl;
    5efc:	e5d53008 	ldrb	r3, [r5, #8]
	if (unlikely(ttl < 2)) {
    5f00:	e3530001 	cmp	r3, #1
	ttl = iph->ttl;
    5f04:	e50b303c 	str	r3, [fp, #-60]	@ 0xffffffc4
	if (unlikely(ttl < 2)) {
    5f08:	9a0000fb 	bls	62fc <sfe_ipv4_recv_tcp.constprop.0+0x4ac>
	if (unlikely((len > cm->xmit_dev_mtu) && !skb_is_gso(skb))) {
    5f0c:	e51b2038 	ldr	r2, [fp, #-56]	@ 0xffffffc8
    5f10:	e1d737b4 	ldrh	r3, [r7, #116]	@ 0x74
    5f14:	e1530002 	cmp	r3, r2
    5f18:	3a000106 	bcc	6338 <sfe_ipv4_recv_tcp.constprop.0+0x4e8>
	if (unlikely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) != TCP_FLAG_ACK)) {
    5f1c:	e51b3048 	ldr	r3, [fp, #-72]	@ 0xffffffb8
    5f20:	e2038c17 	and	r8, r3, #5888	@ 0x1700
    5f24:	e3580a01 	cmp	r8, #4096	@ 0x1000
    5f28:	1a000115 	bne	6384 <sfe_ipv4_recv_tcp.constprop.0+0x534>
	counter_cm = cm->counter_match;
    5f2c:	e597a00c 	ldr	sl, [r7, #12]
	if (likely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK))) {
    5f30:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    5f34:	e3130004 	tst	r3, #4
    5f38:	1a00003e 	bne	6038 <sfe_ipv4_recv_tcp.constprop.0+0x1e8>
		seq = ntohl(tcph->seq);
    5f3c:	e5968004 	ldr	r8, [r6, #4]
		if (unlikely((s32)(seq - (cm->protocol_state.tcp.max_end + 1)) > 0)) {
    5f40:	e5973044 	ldr	r3, [r7, #68]	@ 0x44
		seq = ntohl(tcph->seq);
    5f44:	e6bf8f38 	rev	r8, r8
		if (unlikely((s32)(seq - (cm->protocol_state.tcp.max_end + 1)) > 0)) {
    5f48:	e0483003 	sub	r3, r8, r3
    5f4c:	e2433001 	sub	r3, r3, #1
    5f50:	e3530000 	cmp	r3, #0
    5f54:	ca000125 	bgt	63f0 <sfe_ipv4_recv_tcp.constprop.0+0x5a0>
		data_offs = tcph->doff << 2;
    5f58:	e5d6100c 	ldrb	r1, [r6, #12]
    5f5c:	e1a01221 	lsr	r1, r1, #4
    5f60:	e1a01101 	lsl	r1, r1, #2
		if (unlikely(data_offs < sizeof(struct sfe_ipv4_tcp_hdr))) {
    5f64:	e3510013 	cmp	r1, #19
    5f68:	9a00012f 	bls	642c <sfe_ipv4_recv_tcp.constprop.0+0x5dc>
		ack = ntohl(tcph->ack_seq);
    5f6c:	e5963008 	ldr	r3, [r6, #8]
		if (unlikely(!sfe_ipv4_process_tcp_option_sack(tcph, data_offs, &sack))) {
    5f70:	e24b2030 	sub	r2, fp, #48	@ 0x30
    5f74:	e1a00006 	mov	r0, r6
		ack = ntohl(tcph->ack_seq);
    5f78:	e6bf3f33 	rev	r3, r3
		sack = ack;
    5f7c:	e50b3040 	str	r3, [fp, #-64]	@ 0xffffffc0
    5f80:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
		if (unlikely(!sfe_ipv4_process_tcp_option_sack(tcph, data_offs, &sack))) {
    5f84:	ebffe9ed 	bl	740 <sfe_ipv4_process_tcp_option_sack>
    5f88:	e3500000 	cmp	r0, #0
    5f8c:	e51b3040 	ldr	r3, [fp, #-64]	@ 0xffffffc0
    5f90:	0a000134 	beq	6468 <sfe_ipv4_recv_tcp.constprop.0+0x618>
		if (unlikely(len < data_offs)) {
    5f94:	e51b2038 	ldr	r2, [fp, #-56]	@ 0xffffffc8
		data_offs += sizeof(struct sfe_ipv4_ip_hdr);
    5f98:	e2811014 	add	r1, r1, #20
		if (unlikely(len < data_offs)) {
    5f9c:	e1520001 	cmp	r2, r1
    5fa0:	3a00013f 	bcc	64a4 <sfe_ipv4_recv_tcp.constprop.0+0x654>
		end = seq + len - data_offs;
    5fa4:	e0421001 	sub	r1, r2, r1
    5fa8:	e0812008 	add	r2, r1, r8
		if (unlikely((s32)(end - (cm->protocol_state.tcp.end
    5fac:	e59a103c 	ldr	r1, [sl, #60]	@ 0x3c
    5fb0:	e5978040 	ldr	r8, [r7, #64]	@ 0x40
    5fb4:	e0411008 	sub	r1, r1, r8
    5fb8:	e2811001 	add	r1, r1, #1
    5fbc:	e1710002 	cmn	r1, r2
    5fc0:	4a000146 	bmi	64e0 <sfe_ipv4_recv_tcp.constprop.0+0x690>
		if (unlikely((s32)(sack - (counter_cm->protocol_state.tcp.end + 1)) > 0)) {
    5fc4:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
    5fc8:	e59ac040 	ldr	ip, [sl, #64]	@ 0x40
    5fcc:	e2410001 	sub	r0, r1, #1
    5fd0:	e040000c 	sub	r0, r0, ip
    5fd4:	e3500000 	cmp	r0, #0
    5fd8:	ca00014f 	bgt	651c <sfe_ipv4_recv_tcp.constprop.0+0x6cc>
			    - cm->protocol_state.tcp.max_win
    5fdc:	e597e03c 	ldr	lr, [r7, #60]	@ 0x3c
		if (unlikely((s32)(sack - left_edge) < 0)) {
    5fe0:	e2810cff 	add	r0, r1, #65280	@ 0xff00
    5fe4:	e28000f1 	add	r0, r0, #241	@ 0xf1
    5fe8:	e080000e 	add	r0, r0, lr
    5fec:	e040000c 	sub	r0, r0, ip
    5ff0:	e3500000 	cmp	r0, #0
    5ff4:	ba000185 	blt	6610 <sfe_ipv4_recv_tcp.constprop.0+0x7c0>
		scaled_win = ntohs(tcph->window) << cm->protocol_state.tcp.win_scale;
    5ff8:	e1d600be 	ldrh	r0, [r6, #14]
		scaled_win += (sack - ack);
    5ffc:	e0413003 	sub	r3, r1, r3
		if (likely((s32)(end - cm->protocol_state.tcp.end) >= 0)) {
    6000:	e0428008 	sub	r8, r2, r8
		scaled_win = ntohs(tcph->window) << cm->protocol_state.tcp.win_scale;
    6004:	e5d7c038 	ldrb	ip, [r7, #56]	@ 0x38
    6008:	e6bf0fb0 	rev16	r0, r0
    600c:	e6ff0070 	uxth	r0, r0
		scaled_win += (sack - ack);
    6010:	e083cc10 	add	ip, r3, r0, lsl ip
		if (unlikely(cm->protocol_state.tcp.max_win < scaled_win)) {
    6014:	e15e000c 	cmp	lr, ip
		max_end = sack + scaled_win;
    6018:	e081100c 	add	r1, r1, ip
			cm->protocol_state.tcp.max_win = scaled_win;
    601c:	3587c03c 	strcc	ip, [r7, #60]	@ 0x3c
		if (likely((s32)(end - cm->protocol_state.tcp.end) >= 0)) {
    6020:	e3580000 	cmp	r8, #0
			cm->protocol_state.tcp.end = end;
    6024:	a5872040 	strge	r2, [r7, #64]	@ 0x40
		if (likely((s32)(max_end - counter_cm->protocol_state.tcp.max_end) >= 0)) {
    6028:	e59a3044 	ldr	r3, [sl, #68]	@ 0x44
    602c:	e0413003 	sub	r3, r1, r3
    6030:	e3530000 	cmp	r3, #0
			counter_cm->protocol_state.tcp.max_end = max_end;
    6034:	a58a1044 	strge	r1, [sl, #68]	@ 0x44
	return skb->cloned &&
    6038:	e5d4306a 	ldrb	r3, [r4, #106]	@ 0x6a
    603c:	e3130001 	tst	r3, #1
    6040:	1a00005e 	bne	61c0 <sfe_ipv4_recv_tcp.constprop.0+0x370>
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
    6044:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    6048:	e3130040 	tst	r3, #64	@ 0x40
    604c:	1a000169 	bne	65f8 <sfe_ipv4_recv_tcp.constprop.0+0x7a8>
	iph->ttl = ttl - 1;
    6050:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
    6054:	e243a001 	sub	sl, r3, #1
    6058:	e5c5a008 	strb	sl, [r5, #8]
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
    605c:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    6060:	e3130001 	tst	r3, #1
    6064:	1a000144 	bne	657c <sfe_ipv4_recv_tcp.constprop.0+0x72c>
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    6068:	e3130002 	tst	r3, #2
    606c:	1a000152 	bne	65bc <sfe_ipv4_recv_tcp.constprop.0+0x76c>
	sum = i[0] + i[1] + i[2] + i[3] + i[4] + i[5] + i[6] + i[7] + i[8] + i[9];
    6070:	e1d520b2 	ldrh	r2, [r5, #2]
    6074:	e1d530b0 	ldrh	r3, [r5]
	cm->rx_byte_count += len;
    6078:	e51b1038 	ldr	r1, [fp, #-56]	@ 0xffffffc8
	sum = i[0] + i[1] + i[2] + i[3] + i[4] + i[5] + i[6] + i[7] + i[8] + i[9];
    607c:	e0833002 	add	r3, r3, r2
    6080:	e1d520b4 	ldrh	r2, [r5, #4]
    6084:	e0833002 	add	r3, r3, r2
    6088:	e1d520b6 	ldrh	r2, [r5, #6]
    608c:	e0833002 	add	r3, r3, r2
    6090:	e1d520b8 	ldrh	r2, [r5, #8]
    6094:	e0833002 	add	r3, r3, r2
    6098:	e1d520bc 	ldrh	r2, [r5, #12]
    609c:	e0833002 	add	r3, r3, r2
    60a0:	e1d520be 	ldrh	r2, [r5, #14]
    60a4:	e0833002 	add	r3, r3, r2
    60a8:	e1d521b0 	ldrh	r2, [r5, #16]
    60ac:	e0833002 	add	r3, r3, r2
    60b0:	e1d521b2 	ldrh	r2, [r5, #18]
    60b4:	e0833002 	add	r3, r3, r2
	sum = (sum & 0xffff) + (sum >> 16);
    60b8:	e6ff2073 	uxth	r2, r3
    60bc:	e0823823 	add	r3, r2, r3, lsr #16
	sum = (sum & 0xffff) + (sum >> 16);
    60c0:	e1a02823 	lsr	r2, r3, #16
    60c4:	e6f23073 	uxtah	r3, r2, r3
	return (u16)sum ^ 0xffff;
    60c8:	e1e03003 	mvn	r3, r3
	iph->check = sfe_ipv4_gen_ip_csum(iph);
    60cc:	e1c530ba 	strh	r3, [r5, #10]
	cm->rx_byte_count += len;
    60d0:	e1c724d8 	ldrd	r2, [r7, #72]	@ 0x48
    60d4:	e0833001 	add	r3, r3, r1
	cm->rx_packet_count++;
    60d8:	e2822001 	add	r2, r2, #1
	cm->rx_byte_count += len;
    60dc:	e1c724f8 	strd	r2, [r7, #72]	@ 0x48
	if (unlikely(!cm->active)) {
    60e0:	e5d73018 	ldrb	r3, [r7, #24]
    60e4:	e3530000 	cmp	r3, #0
    60e8:	0a00011a 	beq	6558 <sfe_ipv4_recv_tcp.constprop.0+0x708>
	xmit_dev = cm->xmit_dev;
    60ec:	e5971070 	ldr	r1, [r7, #112]	@ 0x70
	skb->dev = xmit_dev;
    60f0:	e5841008 	str	r1, [r4, #8]
	if (likely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
    60f4:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    60f8:	e3130010 	tst	r3, #16
    60fc:	0a000018 	beq	6164 <sfe_ipv4_recv_tcp.constprop.0+0x314>
		if (unlikely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
    6100:	e3130008 	tst	r3, #8
    6104:	0a000150 	beq	664c <sfe_ipv4_recv_tcp.constprop.0+0x7fc>
	skb->len  += len;
    6108:	e594205c 	ldr	r2, [r4, #92]	@ 0x5c
			eth->h_proto = htons(ETH_P_IP);
    610c:	e3a01000 	mov	r1, #0
    6110:	e3a00008 	mov	r0, #8
	skb->data -= len;
    6114:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
	skb->len  += len;
    6118:	e282200e 	add	r2, r2, #14
	skb->data -= len;
    611c:	e243c00e 	sub	ip, r3, #14
	skb->len  += len;
    6120:	e584205c 	str	r2, [r4, #92]	@ 0x5c
	skb->data -= len;
    6124:	e584c0b0 	str	ip, [r4, #176]	@ 0xb0
    6128:	e5430002 	strb	r0, [r3, #-2]
    612c:	e5431001 	strb	r1, [r3, #-1]
			eth->h_dest[0] = cm->xmit_dest_mac[0];
    6130:	e1d727b6 	ldrh	r2, [r7, #118]	@ 0x76
    6134:	e14320be 	strh	r2, [r3, #-14]
			eth->h_dest[1] = cm->xmit_dest_mac[1];
    6138:	e1d727b8 	ldrh	r2, [r7, #120]	@ 0x78
    613c:	e14320bc 	strh	r2, [r3, #-12]
			eth->h_dest[2] = cm->xmit_dest_mac[2];
    6140:	e1d727ba 	ldrh	r2, [r7, #122]	@ 0x7a
    6144:	e14320ba 	strh	r2, [r3, #-10]
			eth->h_source[0] = cm->xmit_src_mac[0];
    6148:	e1d727bc 	ldrh	r2, [r7, #124]	@ 0x7c
    614c:	e14320b8 	strh	r2, [r3, #-8]
			eth->h_source[1] = cm->xmit_src_mac[1];
    6150:	e1d727be 	ldrh	r2, [r7, #126]	@ 0x7e
    6154:	e14320b6 	strh	r2, [r3, #-6]
			eth->h_source[2] = cm->xmit_src_mac[2];
    6158:	e1d728b0 	ldrh	r2, [r7, #128]	@ 0x80
    615c:	e14320b4 	strh	r2, [r3, #-4]
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    6160:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    6164:	e3130020 	tst	r3, #32
		skb->priority = cm->priority;
    6168:	15973068 	ldrne	r3, [r7, #104]	@ 0x68
    616c:	15843078 	strne	r3, [r4, #120]	@ 0x78
	skb->mark = cm->connection->mark;
    6170:	e5972008 	ldr	r2, [r7, #8]
	si->packets_forwarded++;
    6174:	e59f353c 	ldr	r3, [pc, #1340]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
	skb->mark = cm->connection->mark;
    6178:	e5922048 	ldr	r2, [r2, #72]	@ 0x48
	raw_spin_unlock_bh(&lock->rlock);
    617c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
    6180:	e5842090 	str	r2, [r4, #144]	@ 0x90
	si->packets_forwarded++;
    6184:	e593204c 	ldr	r2, [r3, #76]	@ 0x4c
    6188:	e2822001 	add	r2, r2, #1
    618c:	e583204c 	str	r2, [r3, #76]	@ 0x4c
    6190:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    6194:	e59430a8 	ldr	r3, [r4, #168]	@ 0xa8
    6198:	f5d3f000 	pld	[r3]
	skb->fast_forwarded = 1;
    619c:	e5d43071 	ldrb	r3, [r4, #113]	@ 0x71
	return __dev_queue_xmit(skb, NULL);
    61a0:	e3a01000 	mov	r1, #0
    61a4:	e1a00004 	mov	r0, r4
    61a8:	e3833008 	orr	r3, r3, #8
    61ac:	e5c43071 	strb	r3, [r4, #113]	@ 0x71
    61b0:	ebfffffe 	bl	0 <__dev_queue_xmit>
	return 1;
    61b4:	e3a00001 	mov	r0, #1
}
    61b8:	e24bd028 	sub	sp, fp, #40	@ 0x28
    61bc:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
	return skb->end;
    61c0:	e59420a8 	ldr	r2, [r4, #168]	@ 0xa8
    61c4:	e5923020 	ldr	r3, [r2, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    61c8:	e6ff3073 	uxth	r3, r3
	return skb->cloned &&
    61cc:	e3530001 	cmp	r3, #1
    61d0:	0affff9b 	beq	6044 <sfe_ipv4_recv_tcp.constprop.0+0x1f4>
    61d4:	e5923020 	ldr	r3, [r2, #32]
	       (atomic_read(&skb_shinfo(skb)->dataref) & SKB_DATAREF_MASK) != 1;
    61d8:	e6ff3073 	uxth	r3, r3
	return skb->cloned &&
    61dc:	e3530001 	cmp	r3, #1
    61e0:	0a000076 	beq	63c0 <sfe_ipv4_recv_tcp.constprop.0+0x570>
		struct sk_buff *nskb = skb_copy(skb, pri);
    61e4:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    61e8:	e1a00004 	mov	r0, r4
    61ec:	ebfffffe 	bl	0 <skb_copy>
		if (likely(nskb))
    61f0:	e2505000 	subs	r5, r0, #0
    61f4:	0a00012b 	beq	66a8 <sfe_ipv4_recv_tcp.constprop.0+0x858>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    61f8:	e1a00004 	mov	r0, r4
    61fc:	e3a01001 	mov	r1, #1
    6200:	e1a04005 	mov	r4, r5
    6204:	ebfffffe 	bl	0 <kfree_skb_reason>
		tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
    6208:	e59b3004 	ldr	r3, [fp, #4]
		iph = (struct sfe_ipv4_ip_hdr *)skb->data;
    620c:	e59450b0 	ldr	r5, [r4, #176]	@ 0xb0
		tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
    6210:	e0856003 	add	r6, r5, r3
    6214:	eaffff8a 	b	6044 <sfe_ipv4_recv_tcp.constprop.0+0x1f4>
	if (unlikely(len > skb->len))
    6218:	e1530002 	cmp	r3, r2
    621c:	8a000004 	bhi	6234 <sfe_ipv4_recv_tcp.constprop.0+0x3e4>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    6220:	e0433002 	sub	r3, r3, r2
    6224:	e0831001 	add	r1, r3, r1
    6228:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_tcp_hdr) + ihl)))) {
    622c:	e3500000 	cmp	r0, #0
    6230:	1affff16 	bne	5e90 <sfe_ipv4_recv_tcp.constprop.0+0x40>
	raw_spin_lock_bh(&lock->rlock);
    6234:	e3000000 	movw	r0, #0
    6238:	e3400000 	movt	r0, #0
    623c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    6240:	e59f3470 	ldr	r3, [pc, #1136]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
		si->packets_not_forwarded++;
    6244:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    6248:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    624c:	e5932068 	ldr	r2, [r3, #104]	@ 0x68
		si->packets_not_forwarded++;
    6250:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    6254:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    6258:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
    625c:	e5832068 	str	r2, [r3, #104]	@ 0x68
    6260:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    6264:	e3a00000 	mov	r0, #0
    6268:	eaffffd2 	b	61b8 <sfe_ipv4_recv_tcp.constprop.0+0x368>
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    626c:	e51b2048 	ldr	r2, [fp, #-72]	@ 0xffffffb8
			si->packets_not_forwarded++;
    6270:	e2893902 	add	r3, r9, #32768	@ 0x8000
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    6274:	e2028c17 	and	r8, r2, #5888	@ 0x1700
			si->packets_not_forwarded++;
    6278:	e5932050 	ldr	r2, [r3, #80]	@ 0x50
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    627c:	e3580a01 	cmp	r8, #4096	@ 0x1000
			si->packets_not_forwarded++;
    6280:	e2822001 	add	r2, r2, #1
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
    6284:	1a000100 	bne	668c <sfe_ipv4_recv_tcp.constprop.0+0x83c>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS]++;
    6288:	e5931070 	ldr	r1, [r3, #112]	@ 0x70
			si->packets_not_forwarded++;
    628c:	e5832050 	str	r2, [r3, #80]	@ 0x50
    6290:	e1a00009 	mov	r0, r9
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS]++;
    6294:	e2812001 	add	r2, r1, #1
    6298:	e5832070 	str	r2, [r3, #112]	@ 0x70
    629c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    62a0:	eaffffef 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
		struct sfe_ipv4_connection *c = cm->connection;
    62a4:	e5974008 	ldr	r4, [r7, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    62a8:	e1a00004 	mov	r0, r4
    62ac:	ebffefe9 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    62b0:	e2893902 	add	r3, r9, #32768	@ 0x8000
    62b4:	e1a00009 	mov	r0, r9
		si->packets_not_forwarded++;
    62b8:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    62bc:	e5932074 	ldr	r2, [r3, #116]	@ 0x74
		si->packets_not_forwarded++;
    62c0:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    62c4:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    62c8:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
    62cc:	e5832074 	str	r2, [r3, #116]	@ 0x74
    62d0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    62d4:	e1a00004 	mov	r0, r4
    62d8:	ebffee45 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    62dc:	eaffffe0 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
		si->packets_not_forwarded++;
    62e0:	e2892902 	add	r2, r9, #32768	@ 0x8000
    62e4:	e1a00009 	mov	r0, r9
    62e8:	e5923050 	ldr	r3, [r2, #80]	@ 0x50
    62ec:	e2833001 	add	r3, r3, #1
    62f0:	e5823050 	str	r3, [r2, #80]	@ 0x50
    62f4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    62f8:	eaffffd9 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
		struct sfe_ipv4_connection *c = cm->connection;
    62fc:	e5974008 	ldr	r4, [r7, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    6300:	e1a00004 	mov	r0, r4
    6304:	ebffefd3 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    6308:	e2893902 	add	r3, r9, #32768	@ 0x8000
    630c:	e1a00009 	mov	r0, r9
		si->packets_not_forwarded++;
    6310:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    6314:	e5932078 	ldr	r2, [r3, #120]	@ 0x78
		si->packets_not_forwarded++;
    6318:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    631c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    6320:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
    6324:	e5832078 	str	r2, [r3, #120]	@ 0x78
    6328:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    632c:	e1a00004 	mov	r0, r4
    6330:	ebffee2f 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    6334:	eaffffca 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
	return skb_shinfo(skb)->gso_size;
    6338:	e59430a8 	ldr	r3, [r4, #168]	@ 0xa8
	if (unlikely((len > cm->xmit_dev_mtu) && !skb_is_gso(skb))) {
    633c:	e1d330b4 	ldrh	r3, [r3, #4]
    6340:	e3530000 	cmp	r3, #0
    6344:	1afffef4 	bne	5f1c <sfe_ipv4_recv_tcp.constprop.0+0xcc>
		struct sfe_ipv4_connection *c = cm->connection;
    6348:	e5974008 	ldr	r4, [r7, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    634c:	e1a00004 	mov	r0, r4
    6350:	ebffefc0 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    6354:	e2893902 	add	r3, r9, #32768	@ 0x8000
    6358:	e1a00009 	mov	r0, r9
		si->packets_not_forwarded++;
    635c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    6360:	e593207c 	ldr	r2, [r3, #124]	@ 0x7c
		si->packets_not_forwarded++;
    6364:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    6368:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    636c:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
    6370:	e583207c 	str	r2, [r3, #124]	@ 0x7c
    6374:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    6378:	e1a00004 	mov	r0, r4
    637c:	ebffee1c 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    6380:	eaffffb7 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
		struct sfe_ipv4_connection *c = cm->connection;
    6384:	e5974008 	ldr	r4, [r7, #8]
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    6388:	e1a00004 	mov	r0, r4
    638c:	ebffefb1 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS]++;
    6390:	e59f3320 	ldr	r3, [pc, #800]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
		si->packets_not_forwarded++;
    6394:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    6398:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS]++;
    639c:	e5932080 	ldr	r2, [r3, #128]	@ 0x80
		si->packets_not_forwarded++;
    63a0:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS]++;
    63a4:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    63a8:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS]++;
    63ac:	e5832080 	str	r2, [r3, #128]	@ 0x80
    63b0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    63b4:	e1a00004 	mov	r0, r4
    63b8:	ebffee0d 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
		return 0;
    63bc:	eaffffa8 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
                if (!skb) {
    63c0:	e3540000 	cmp	r4, #0
    63c4:	1affff8f 	bne	6208 <sfe_ipv4_recv_tcp.constprop.0+0x3b8>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    63c8:	e59f32e8 	ldr	r3, [pc, #744]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    63cc:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    63d0:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    63d4:	e59320e4 	ldr	r2, [r3, #228]	@ 0xe4
			si->packets_not_forwarded++;
    63d8:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    63dc:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    63e0:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
    63e4:	e58320e4 	str	r2, [r3, #228]	@ 0xe4
    63e8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    63ec:	eaffff9c 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
			struct sfe_ipv4_connection *c = cm->connection;
    63f0:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    63f4:	e1a00004 	mov	r0, r4
    63f8:	ebffef96 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    63fc:	e59f32b4 	ldr	r3, [pc, #692]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    6400:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    6404:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    6408:	e5932084 	ldr	r2, [r3, #132]	@ 0x84
			si->packets_not_forwarded++;
    640c:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    6410:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    6414:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
    6418:	e5832084 	str	r2, [r3, #132]	@ 0x84
    641c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    6420:	e1a00004 	mov	r0, r4
    6424:	ebffedf2 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    6428:	eaffff8d 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
			struct sfe_ipv4_connection *c = cm->connection;
    642c:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    6430:	e1a00004 	mov	r0, r4
    6434:	ebffef87 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    6438:	e59f3278 	ldr	r3, [pc, #632]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    643c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    6440:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    6444:	e5932088 	ldr	r2, [r3, #136]	@ 0x88
			si->packets_not_forwarded++;
    6448:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    644c:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    6450:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
    6454:	e5832088 	str	r2, [r3, #136]	@ 0x88
    6458:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    645c:	e1a00004 	mov	r0, r4
    6460:	ebffede3 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    6464:	eaffff7e 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
			struct sfe_ipv4_connection *c = cm->connection;
    6468:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    646c:	e1a00004 	mov	r0, r4
    6470:	ebffef78 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    6474:	e59f323c 	ldr	r3, [pc, #572]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    6478:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    647c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    6480:	e593208c 	ldr	r2, [r3, #140]	@ 0x8c
			si->packets_not_forwarded++;
    6484:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    6488:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    648c:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK]++;
    6490:	e583208c 	str	r2, [r3, #140]	@ 0x8c
    6494:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    6498:	e1a00004 	mov	r0, r4
    649c:	ebffedd4 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    64a0:	eaffff6f 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
			struct sfe_ipv4_connection *c = cm->connection;
    64a4:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    64a8:	e1a00004 	mov	r0, r4
    64ac:	ebffef69 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    64b0:	e59f3200 	ldr	r3, [pc, #512]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    64b4:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    64b8:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    64bc:	e5932090 	ldr	r2, [r3, #144]	@ 0x90
			si->packets_not_forwarded++;
    64c0:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    64c4:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    64c8:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
    64cc:	e5832090 	str	r2, [r3, #144]	@ 0x90
    64d0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    64d4:	e1a00004 	mov	r0, r4
    64d8:	ebffedc5 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    64dc:	eaffff60 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
			struct sfe_ipv4_connection *c = cm->connection;
    64e0:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    64e4:	e1a00004 	mov	r0, r4
    64e8:	ebffef5a 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    64ec:	e59f31c4 	ldr	r3, [pc, #452]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    64f0:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    64f4:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    64f8:	e5932094 	ldr	r2, [r3, #148]	@ 0x94
			si->packets_not_forwarded++;
    64fc:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    6500:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    6504:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
    6508:	e5832094 	str	r2, [r3, #148]	@ 0x94
    650c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    6510:	e1a00004 	mov	r0, r4
    6514:	ebffedb6 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    6518:	eaffff51 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
			struct sfe_ipv4_connection *c = cm->connection;
    651c:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    6520:	e1a00004 	mov	r0, r4
    6524:	ebffef4b 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    6528:	e59f3188 	ldr	r3, [pc, #392]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    652c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    6530:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    6534:	e5932098 	ldr	r2, [r3, #152]	@ 0x98
			si->packets_not_forwarded++;
    6538:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    653c:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    6540:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
    6544:	e5832098 	str	r2, [r3, #152]	@ 0x98
    6548:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    654c:	e1a00004 	mov	r0, r4
    6550:	ebffeda7 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    6554:	eaffff42 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
		cm->active_prev = si->active_tail;
    6558:	e5993008 	ldr	r3, [r9, #8]
		cm->active = true;
    655c:	e3a02001 	mov	r2, #1
		si->active_tail = cm;
    6560:	e5897008 	str	r7, [r9, #8]
		if (likely(si->active_tail)) {
    6564:	e3530000 	cmp	r3, #0
		cm->active_prev = si->active_tail;
    6568:	e5873014 	str	r3, [r7, #20]
		cm->active = true;
    656c:	e5c72018 	strb	r2, [r7, #24]
			si->active_head = cm;
    6570:	05897004 	streq	r7, [r9, #4]
			si->active_tail->active_next = cm;
    6574:	15837010 	strne	r7, [r3, #16]
		si->active_tail = cm;
    6578:	eafffedb 	b	60ec <sfe_ipv4_recv_tcp.constprop.0+0x29c>
		iph->saddr = cm->xlate_src_ip;
    657c:	e5973050 	ldr	r3, [r7, #80]	@ 0x50
    6580:	e585300c 	str	r3, [r5, #12]
		tcph->source = cm->xlate_src_port;
    6584:	e1d735b4 	ldrh	r3, [r7, #84]	@ 0x54
    6588:	e1c630b0 	strh	r3, [r6]
		if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
    658c:	e5d4306c 	ldrb	r3, [r4, #108]	@ 0x6c
    6590:	e2033060 	and	r3, r3, #96	@ 0x60
    6594:	e3530060 	cmp	r3, #96	@ 0x60
		tcp_csum = tcph->check;
    6598:	e1d631b0 	ldrh	r3, [r6, #16]
			sum = tcp_csum + cm->xlate_src_partial_csum_adjustment;
    659c:	01d725b8 	ldrheq	r2, [r7, #88]	@ 0x58
			sum = tcp_csum + cm->xlate_src_csum_adjustment;
    65a0:	11d725b6 	ldrhne	r2, [r7, #86]	@ 0x56
    65a4:	e0823003 	add	r3, r2, r3
		sum = (sum & 0xffff) + (sum >> 16);
    65a8:	e1a02823 	lsr	r2, r3, #16
    65ac:	e6f23073 	uxtah	r3, r2, r3
		tcph->check = (u16)sum;
    65b0:	e1c631b0 	strh	r3, [r6, #16]
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
    65b4:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    65b8:	eafffeaa 	b	6068 <sfe_ipv4_recv_tcp.constprop.0+0x218>
		iph->daddr = cm->xlate_dest_ip;
    65bc:	e597305c 	ldr	r3, [r7, #92]	@ 0x5c
    65c0:	e5853010 	str	r3, [r5, #16]
		tcph->dest = cm->xlate_dest_port;
    65c4:	e1d736b0 	ldrh	r3, [r7, #96]	@ 0x60
    65c8:	e1c630b2 	strh	r3, [r6, #2]
		if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
    65cc:	e5d4306c 	ldrb	r3, [r4, #108]	@ 0x6c
    65d0:	e2033060 	and	r3, r3, #96	@ 0x60
    65d4:	e3530060 	cmp	r3, #96	@ 0x60
		tcp_csum = tcph->check;
    65d8:	e1d631b0 	ldrh	r3, [r6, #16]
			sum = tcp_csum + cm->xlate_dest_partial_csum_adjustment;
    65dc:	01d726b4 	ldrheq	r2, [r7, #100]	@ 0x64
			sum = tcp_csum + cm->xlate_dest_csum_adjustment;
    65e0:	11d726b2 	ldrhne	r2, [r7, #98]	@ 0x62
    65e4:	e0823003 	add	r3, r2, r3
		sum = (sum & 0xffff) + (sum >> 16);
    65e8:	e1a02823 	lsr	r2, r3, #16
    65ec:	e6f23073 	uxtah	r3, r2, r3
		tcph->check = (u16)sum;
    65f0:	e1c631b0 	strh	r3, [r6, #16]
    65f4:	eafffe9d 	b	6070 <sfe_ipv4_recv_tcp.constprop.0+0x220>
		iph->tos = (iph->tos & SFE_IPV4_DSCP_MASK) | cm->dscp;
    65f8:	e5d53001 	ldrb	r3, [r5, #1]
    65fc:	e597206c 	ldr	r2, [r7, #108]	@ 0x6c
    6600:	e2033003 	and	r3, r3, #3
    6604:	e1833002 	orr	r3, r3, r2
    6608:	e5c53001 	strb	r3, [r5, #1]
    660c:	eafffe8f 	b	6050 <sfe_ipv4_recv_tcp.constprop.0+0x200>
			struct sfe_ipv4_connection *c = cm->connection;
    6610:	e5974008 	ldr	r4, [r7, #8]
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    6614:	e1a00004 	mov	r0, r4
    6618:	ebffef0e 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    661c:	e59f3094 	ldr	r3, [pc, #148]	@ 66b8 <sfe_ipv4_recv_tcp.constprop.0+0x868>
			si->packets_not_forwarded++;
    6620:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    6624:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    6628:	e593209c 	ldr	r2, [r3, #156]	@ 0x9c
			si->packets_not_forwarded++;
    662c:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    6630:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    6634:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
    6638:	e583209c 	str	r2, [r3, #156]	@ 0x9c
    663c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    6640:	e1a00004 	mov	r0, r4
    6644:	ebffed6a 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
			return 0;
    6648:	eaffff05 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
	if (!dev->header_ops || !dev->header_ops->create)
    664c:	e5912158 	ldr	r2, [r1, #344]	@ 0x158
    6650:	e3520000 	cmp	r2, #0
    6654:	0afffec2 	beq	6164 <sfe_ipv4_recv_tcp.constprop.0+0x314>
    6658:	e5925000 	ldr	r5, [r2]
    665c:	e3550000 	cmp	r5, #0
    6660:	0afffebf 	beq	6164 <sfe_ipv4_recv_tcp.constprop.0+0x314>
	return dev->header_ops->create(skb, dev, type, daddr, saddr, len);
    6664:	e51b2038 	ldr	r2, [fp, #-56]	@ 0xffffffc8
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
    6668:	e287307c 	add	r3, r7, #124	@ 0x7c
    666c:	e1a00004 	mov	r0, r4
    6670:	e58d3000 	str	r3, [sp]
    6674:	e2873076 	add	r3, r7, #118	@ 0x76
    6678:	e58d2004 	str	r2, [sp, #4]
    667c:	e3a02b02 	mov	r2, #2048	@ 0x800
    6680:	e12fff35 	blx	r5
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
    6684:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
    6688:	eafffeb5 	b	6164 <sfe_ipv4_recv_tcp.constprop.0+0x314>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS]++;
    668c:	e593106c 	ldr	r1, [r3, #108]	@ 0x6c
		si->packets_not_forwarded++;
    6690:	e5832050 	str	r2, [r3, #80]	@ 0x50
    6694:	e1a00009 	mov	r0, r9
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS]++;
    6698:	e2812001 	add	r2, r1, #1
    669c:	e583206c 	str	r2, [r3, #108]	@ 0x6c
    66a0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    66a4:	eafffeee 	b	6264 <sfe_ipv4_recv_tcp.constprop.0+0x414>
	kfree_skb_reason(skb, SKB_DROP_REASON_NOT_SPECIFIED);
    66a8:	e3a01001 	mov	r1, #1
    66ac:	e1a00004 	mov	r0, r4
    66b0:	ebfffffe 	bl	0 <kfree_skb_reason>
                if (!skb) {
    66b4:	eaffff43 	b	63c8 <sfe_ipv4_recv_tcp.constprop.0+0x578>
    66b8:	000182c8 	.word	0x000182c8

000066bc <sfe_ipv4_create_rule>:
{
    66bc:	e1a0c00d 	mov	ip, sp
    66c0:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    66c4:	e24cb004 	sub	fp, ip, #4
    66c8:	e24dd034 	sub	sp, sp, #52	@ 0x34
	src_dev = sic->src_dev;
    66cc:	e590a004 	ldr	sl, [r0, #4]
{
    66d0:	e1a04000 	mov	r4, r0
	dest_dev = sic->dest_dev;
    66d4:	e5909008 	ldr	r9, [r0, #8]
	if (unlikely((dest_dev->reg_state != NETREG_REGISTERED) ||
    66d8:	e5d9331c 	ldrb	r3, [r9, #796]	@ 0x31c
    66dc:	e3530001 	cmp	r3, #1
    66e0:	1a0001c0 	bne	6de8 <sfe_ipv4_create_rule+0x72c>
    66e4:	e5da331c 	ldrb	r3, [sl, #796]	@ 0x31c
    66e8:	e3530001 	cmp	r3, #1
    66ec:	1a0001bd 	bne	6de8 <sfe_ipv4_create_rule+0x72c>
	raw_spin_lock_bh(&lock->rlock);
    66f0:	e3008000 	movw	r8, #0
    66f4:	e3408000 	movt	r8, #0
    66f8:	e1a00008 	mov	r0, r8
    66fc:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_create_requests++;
    6700:	e288e902 	add	lr, r8, #32768	@ 0x8000
    6704:	e59e3030 	ldr	r3, [lr, #48]	@ 0x30
    6708:	e2833001 	add	r3, r3, #1
    670c:	e58e3030 	str	r3, [lr, #48]	@ 0x30
	c = sfe_ipv4_find_sfe_ipv4_connection(si,
    6710:	e1d415b8 	ldrh	r1, [r4, #88]	@ 0x58
    6714:	e1d405bc 	ldrh	r0, [r4, #92]	@ 0x5c
					      sic->protocol,
    6718:	e5946000 	ldr	r6, [r4]
	c = sfe_ipv4_find_sfe_ipv4_connection(si,
    671c:	e594c018 	ldr	ip, [r4, #24]
    6720:	e5945038 	ldr	r5, [r4, #56]	@ 0x38
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6724:	e0212000 	eor	r2, r1, r0
    6728:	e6ef3076 	uxtb	r3, r6
    672c:	e6bf2fb2 	rev16	r2, r2
    6730:	e6ff2072 	uxth	r2, r2
    6734:	e02c7005 	eor	r7, ip, r5
    6738:	e6bf7f37 	rev	r7, r7
    673c:	e0233007 	eor	r3, r3, r7
    6740:	e0233002 	eor	r3, r3, r2
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    6744:	e0233623 	eor	r3, r3, r3, lsr #12
    6748:	e7eb3053 	ubfx	r3, r3, #0, #12
	c = si->conn_hash[conn_idx];
    674c:	e283300c 	add	r3, r3, #12
    6750:	e7983103 	ldr	r3, [r8, r3, lsl #2]
	if (unlikely(!c)) {
    6754:	e3530000 	cmp	r3, #0
    6758:	1a000003 	bne	676c <sfe_ipv4_create_rule+0xb0>
    675c:	ea00001c 	b	67d4 <sfe_ipv4_create_rule+0x118>
		c = c->next;
    6760:	e5933000 	ldr	r3, [r3]
	} while (c && (c->src_port != src_port
    6764:	e3530000 	cmp	r3, #0
    6768:	0a000019 	beq	67d4 <sfe_ipv4_create_rule+0x118>
	if ((c->src_port == src_port)
    676c:	e1d321bc 	ldrh	r2, [r3, #28]
    6770:	e1520001 	cmp	r2, r1
    6774:	1afffff9 	bne	6760 <sfe_ipv4_create_rule+0xa4>
		 || c->dest_port != dest_port
    6778:	e1d322b0 	ldrh	r2, [r3, #32]
    677c:	e1520000 	cmp	r2, r0
    6780:	1afffff6 	bne	6760 <sfe_ipv4_create_rule+0xa4>
		 || c->src_ip != src_ip
    6784:	e593200c 	ldr	r2, [r3, #12]
    6788:	e15c0002 	cmp	ip, r2
    678c:	1afffff3 	bne	6760 <sfe_ipv4_create_rule+0xa4>
		 || c->dest_ip != dest_ip
    6790:	e5932014 	ldr	r2, [r3, #20]
    6794:	e1550002 	cmp	r5, r2
    6798:	1afffff0 	bne	6760 <sfe_ipv4_create_rule+0xa4>
		 || c->protocol != protocol));
    679c:	e5932008 	ldr	r2, [r3, #8]
    67a0:	e1560002 	cmp	r6, r2
    67a4:	1affffed 	bne	6760 <sfe_ipv4_create_rule+0xa4>
		si->connection_create_collisions++;
    67a8:	e59e2034 	ldr	r2, [lr, #52]	@ 0x34
    67ac:	e2822001 	add	r2, r2, #1
    67b0:	e58e2034 	str	r2, [lr, #52]	@ 0x34
	switch (sic->protocol) {
    67b4:	e5942000 	ldr	r2, [r4]
    67b8:	e3520006 	cmp	r2, #6
    67bc:	0a00019f 	beq	6e40 <sfe_ipv4_create_rule+0x784>
	raw_spin_unlock_bh(&lock->rlock);
    67c0:	e3000000 	movw	r0, #0
    67c4:	e3400000 	movt	r0, #0
    67c8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return -EADDRINUSE;
    67cc:	e3e00061 	mvn	r0, #97	@ 0x61
    67d0:	ea00015c 	b	6d48 <sfe_ipv4_create_rule+0x68c>
    67d4:	e3007000 	movw	r7, #0
    67d8:	e3a02050 	mov	r2, #80	@ 0x50
    67dc:	e3407000 	movt	r7, #0
    67e0:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    67e4:	e597001c 	ldr	r0, [r7, #28]
    67e8:	ebfffffe 	bl	0 <kmalloc_trace>
	if (unlikely(!c)) {
    67ec:	e2506000 	subs	r6, r0, #0
    67f0:	0a00017f 	beq	6df4 <sfe_ipv4_create_rule+0x738>
    67f4:	e5970008 	ldr	r0, [r7, #8]
    67f8:	e3a02098 	mov	r2, #152	@ 0x98
    67fc:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    6800:	ebfffffe 	bl	0 <kmalloc_trace>
	if (unlikely(!original_cm)) {
    6804:	e2505000 	subs	r5, r0, #0
    6808:	0a00017e 	beq	6e08 <sfe_ipv4_create_rule+0x74c>
    680c:	e5970008 	ldr	r0, [r7, #8]
    6810:	e3a02098 	mov	r2, #152	@ 0x98
    6814:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    6818:	ebfffffe 	bl	0 <kmalloc_trace>
	if (unlikely(!reply_cm)) {
    681c:	e2507000 	subs	r7, r0, #0
    6820:	0a00017e 	beq	6e20 <sfe_ipv4_create_rule+0x764>
	original_cm->match_protocol = sic->protocol;
    6824:	e5942000 	ldr	r2, [r4]
	original_cm->match_dev = src_dev;
    6828:	e585a01c 	str	sl, [r5, #28]
	original_cm->flags = 0;
    682c:	e3a01000 	mov	r1, #0
	original_cm->xmit_dev_mtu = sic->dest_mtu;
    6830:	e5943014 	ldr	r3, [r4, #20]
	original_cm->xmit_dev = dest_dev;
    6834:	e5859070 	str	r9, [r5, #112]	@ 0x70
	original_cm->match_src_port = sic->src_port;
    6838:	e594e05c 	ldr	lr, [r4, #92]	@ 0x5c
	original_cm->match_protocol = sic->protocol;
    683c:	e6ef0072 	uxtb	r0, r2
    6840:	e50b2048 	str	r2, [fp, #-72]	@ 0xffffffb8
	original_cm->xlate_dest_port = sic->dest_port_xlate;
    6844:	e1d425be 	ldrh	r2, [r4, #94]	@ 0x5e
	original_cm->xmit_dev_mtu = sic->dest_mtu;
    6848:	e1c537b4 	strh	r3, [r5, #116]	@ 0x74
	original_cm->match_src_ip = sic->src_ip.ip;
    684c:	e5943018 	ldr	r3, [r4, #24]
	original_cm->xlate_dest_port = sic->dest_port_xlate;
    6850:	e50b204c 	str	r2, [fp, #-76]	@ 0xffffffb4
	original_cm->rx_packet_count64 = 0;
    6854:	e3a02000 	mov	r2, #0
	original_cm->match_protocol = sic->protocol;
    6858:	e5c50020 	strb	r0, [r5, #32]
	original_cm->match_src_ip = sic->src_ip.ip;
    685c:	e50b3030 	str	r3, [fp, #-48]	@ 0xffffffd0
    6860:	e5853024 	str	r3, [r5, #36]	@ 0x24
	original_cm->xlate_src_ip = sic->src_ip_xlate.ip;
    6864:	e5943028 	ldr	r3, [r4, #40]	@ 0x28
    6868:	e50b3038 	str	r3, [fp, #-56]	@ 0xffffffc8
    686c:	e5853050 	str	r3, [r5, #80]	@ 0x50
	original_cm->match_dest_ip = sic->dest_ip.ip;
    6870:	e5943038 	ldr	r3, [r4, #56]	@ 0x38
    6874:	e50b3034 	str	r3, [fp, #-52]	@ 0xffffffcc
    6878:	e5853028 	str	r3, [r5, #40]	@ 0x28
	original_cm->xlate_dest_ip = sic->dest_ip_xlate.ip;
    687c:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    6880:	e50b303c 	str	r3, [fp, #-60]	@ 0xffffffc4
    6884:	e585305c 	str	r3, [r5, #92]	@ 0x5c
	original_cm->match_src_port = sic->src_port;
    6888:	e1d435b8 	ldrh	r3, [r4, #88]	@ 0x58
    688c:	e1c532bc 	strh	r3, [r5, #44]	@ 0x2c
	original_cm->match_dest_port = sic->dest_port;
    6890:	e1d435bc 	ldrh	r3, [r4, #92]	@ 0x5c
    6894:	e1c532be 	strh	r3, [r5, #46]	@ 0x2e
	original_cm->xlate_src_port = sic->src_port_xlate;
    6898:	e1d435ba 	ldrh	r3, [r4, #90]	@ 0x5a
    689c:	e50b3040 	str	r3, [fp, #-64]	@ 0xffffffc0
    68a0:	e1c535b4 	strh	r3, [r5, #84]	@ 0x54
    68a4:	e7ef385e 	ubfx	r3, lr, #16, #16
	original_cm->xlate_dest_port = sic->dest_port_xlate;
    68a8:	e1c536b0 	strh	r3, [r5, #96]	@ 0x60
	original_cm->match_src_port = sic->src_port;
    68ac:	e5943058 	ldr	r3, [r4, #88]	@ 0x58
    68b0:	e50b3054 	str	r3, [fp, #-84]	@ 0xffffffac
    68b4:	e6ff3073 	uxth	r3, r3
    68b8:	e50b3044 	str	r3, [fp, #-68]	@ 0xffffffbc
	original_cm->rx_packet_count64 = 0;
    68bc:	e3a03000 	mov	r3, #0
	original_cm->rx_packet_count = 0;
    68c0:	e1c524f8 	strd	r2, [r5, #72]	@ 0x48
	original_cm->rx_packet_count64 = 0;
    68c4:	e1c528f8 	strd	r2, [r5, #136]	@ 0x88
	original_cm->rx_byte_count64 = 0;
    68c8:	e1c529f0 	strd	r2, [r5, #144]	@ 0x90
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
    68cc:	e5993210 	ldr	r3, [r9, #528]	@ 0x210
    68d0:	e5932000 	ldr	r2, [r3]
    68d4:	e585207c 	str	r2, [r5, #124]	@ 0x7c
    68d8:	e1d330b4 	ldrh	r3, [r3, #4]
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    68dc:	e1d427b6 	ldrh	r2, [r4, #118]	@ 0x76
	original_cm->counter_match = reply_cm;
    68e0:	e1c560f8 	strd	r6, [r5, #8]
	original_cm->flags = 0;
    68e4:	e5851030 	str	r1, [r5, #48]	@ 0x30
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
    68e8:	e1c538b0 	strh	r3, [r5, #128]	@ 0x80
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    68ec:	e5943072 	ldr	r3, [r4, #114]	@ 0x72
    68f0:	e5853076 	str	r3, [r5, #118]	@ 0x76
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    68f4:	e594300c 	ldr	r3, [r4, #12]
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    68f8:	e1c527ba 	strh	r2, [r5, #122]	@ 0x7a
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    68fc:	e2132002 	ands	r2, r3, #2
    6900:	e1a0c002 	mov	ip, r2
    6904:	1a000111 	bne	6d50 <sfe_ipv4_create_rule+0x694>
    6908:	e1a01002 	mov	r1, r2
    690c:	e3a02040 	mov	r2, #64	@ 0x40
    6910:	e50b2058 	str	r2, [fp, #-88]	@ 0xffffffa8
	if (sic->flags & SFE_CREATE_FLAG_REMARK_DSCP) {
    6914:	e2133004 	ands	r3, r3, #4
    6918:	e50b3050 	str	r3, [fp, #-80]	@ 0xffffffb0
    691c:	0a000004 	beq	6934 <sfe_ipv4_create_rule+0x278>
		original_cm->dscp = sic->src_dscp << SFE_IPV4_DSCP_SHIFT;
    6920:	e59430ac 	ldr	r3, [r4, #172]	@ 0xac
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK;
    6924:	e51b1058 	ldr	r1, [fp, #-88]	@ 0xffffffa8
		original_cm->dscp = sic->src_dscp << SFE_IPV4_DSCP_SHIFT;
    6928:	e1a03103 	lsl	r3, r3, #2
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK;
    692c:	e5851030 	str	r1, [r5, #48]	@ 0x30
		original_cm->dscp = sic->src_dscp << SFE_IPV4_DSCP_SHIFT;
    6930:	e585306c 	str	r3, [r5, #108]	@ 0x6c
	original_cm->flow_accel = sic->original_accel;
    6934:	e594309c 	ldr	r3, [r4, #156]	@ 0x9c
	original_cm->active_next = NULL;
    6938:	e3a02000 	mov	r2, #0
	original_cm->flow_accel = sic->original_accel;
    693c:	e5853034 	str	r3, [r5, #52]	@ 0x34
	original_cm->active = false;
    6940:	e3a03000 	mov	r3, #0
    6944:	e5c53018 	strb	r3, [r5, #24]
	original_cm->active_next = NULL;
    6948:	e3a03000 	mov	r3, #0
    694c:	e1c521f0 	strd	r2, [r5, #16]
	if (!(dest_dev->flags & IFF_POINTOPOINT)) {
    6950:	e5993068 	ldr	r3, [r9, #104]	@ 0x68
    6954:	e3130010 	tst	r3, #16
    6958:	1a00000a 	bne	6988 <sfe_ipv4_create_rule+0x2cc>
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;
    695c:	e3813010 	orr	r3, r1, #16
    6960:	e5853030 	str	r3, [r5, #48]	@ 0x30
		if (dest_dev->header_ops) {
    6964:	e5993158 	ldr	r3, [r9, #344]	@ 0x158
    6968:	e3530000 	cmp	r3, #0
    696c:	0a000005 	beq	6988 <sfe_ipv4_create_rule+0x2cc>
			if (dest_dev->header_ops->create == eth_header) {
    6970:	e5932000 	ldr	r2, [r3]
    6974:	e3003000 	movw	r3, #0
    6978:	e3403000 	movt	r3, #0
    697c:	e1520003 	cmp	r2, r3
				original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
    6980:	03813018 	orreq	r3, r1, #24
    6984:	05853030 	streq	r3, [r5, #48]	@ 0x30
	reply_cm->match_src_port = sic->dest_port_xlate;
    6988:	e51b304c 	ldr	r3, [fp, #-76]	@ 0xffffffb4
	reply_cm->rx_packet_count64 = 0;
    698c:	e3a02000 	mov	r2, #0
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    6990:	e35c0000 	cmp	ip, #0
	reply_cm->match_dev = dest_dev;
    6994:	e587901c 	str	r9, [r7, #28]
	reply_cm->match_protocol = sic->protocol;
    6998:	e5c70020 	strb	r0, [r7, #32]
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
    699c:	e1d406b4 	ldrh	r0, [r4, #100]	@ 0x64
	reply_cm->match_src_port = sic->dest_port_xlate;
    69a0:	e1c732bc 	strh	r3, [r7, #44]	@ 0x2c
	reply_cm->match_src_ip = sic->dest_ip_xlate.ip;
    69a4:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
	reply_cm->xmit_dev = src_dev;
    69a8:	e587a070 	str	sl, [r7, #112]	@ 0x70
	reply_cm->match_src_ip = sic->dest_ip_xlate.ip;
    69ac:	e5873024 	str	r3, [r7, #36]	@ 0x24
	reply_cm->match_dest_ip = sic->src_ip_xlate.ip;
    69b0:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
    69b4:	e5873028 	str	r3, [r7, #40]	@ 0x28
	reply_cm->match_dest_port = sic->src_port_xlate;
    69b8:	e51b3040 	ldr	r3, [fp, #-64]	@ 0xffffffc0
    69bc:	e1c732be 	strh	r3, [r7, #46]	@ 0x2e
	reply_cm->xlate_src_ip = sic->dest_ip.ip;
    69c0:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    69c4:	e5873050 	str	r3, [r7, #80]	@ 0x50
	reply_cm->xlate_src_port = sic->dest_port;
    69c8:	e6ff307e 	uxth	r3, lr
    69cc:	e1c735b4 	strh	r3, [r7, #84]	@ 0x54
	reply_cm->xlate_dest_ip = sic->src_ip.ip;
    69d0:	e51b3030 	ldr	r3, [fp, #-48]	@ 0xffffffd0
    69d4:	e587305c 	str	r3, [r7, #92]	@ 0x5c
	reply_cm->xlate_dest_port = sic->src_port;
    69d8:	e51b3044 	ldr	r3, [fp, #-68]	@ 0xffffffbc
    69dc:	e1c736b0 	strh	r3, [r7, #96]	@ 0x60
	reply_cm->xmit_dev_mtu = sic->src_mtu;
    69e0:	e5943010 	ldr	r3, [r4, #16]
    69e4:	e1c737b4 	strh	r3, [r7, #116]	@ 0x74
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
    69e8:	e5943060 	ldr	r3, [r4, #96]	@ 0x60
    69ec:	e1a0c003 	mov	ip, r3
	reply_cm->rx_packet_count64 = 0;
    69f0:	e3a03000 	mov	r3, #0
	reply_cm->rx_packet_count = 0;
    69f4:	e1c724f8 	strd	r2, [r7, #72]	@ 0x48
	reply_cm->rx_packet_count64 = 0;
    69f8:	e1c728f8 	strd	r2, [r7, #136]	@ 0x88
	reply_cm->rx_byte_count64 = 0;
    69fc:	e1c729f0 	strd	r2, [r7, #144]	@ 0x90
	memcpy(reply_cm->xmit_src_mac, src_dev->dev_addr, ETH_ALEN);
    6a00:	e59a3210 	ldr	r3, [sl, #528]	@ 0x210
    6a04:	e5932000 	ldr	r2, [r3]
    6a08:	e587207c 	str	r2, [r7, #124]	@ 0x7c
    6a0c:	e1d330b4 	ldrh	r3, [r3, #4]
	reply_cm->flags = 0;
    6a10:	e3a02000 	mov	r2, #0
    6a14:	e5872030 	str	r2, [r7, #48]	@ 0x30
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
    6a18:	e587c076 	str	ip, [r7, #118]	@ 0x76
    6a1c:	e1c707ba 	strh	r0, [r7, #122]	@ 0x7a
	memcpy(reply_cm->xmit_src_mac, src_dev->dev_addr, ETH_ALEN);
    6a20:	e1c738b0 	strh	r3, [r7, #128]	@ 0x80
	reply_cm->connection = c;
    6a24:	e5876008 	str	r6, [r7, #8]
	reply_cm->counter_match = original_cm;
    6a28:	e587500c 	str	r5, [r7, #12]
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    6a2c:	0a000003 	beq	6a40 <sfe_ipv4_create_rule+0x384>
		reply_cm->priority = sic->dest_priority;
    6a30:	e59430a8 	ldr	r3, [r4, #168]	@ 0xa8
    6a34:	e5873068 	str	r3, [r7, #104]	@ 0x68
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
    6a38:	e3a03020 	mov	r3, #32
    6a3c:	e5873030 	str	r3, [r7, #48]	@ 0x30
	if (sic->flags & SFE_CREATE_FLAG_REMARK_DSCP) {
    6a40:	e51b3050 	ldr	r3, [fp, #-80]	@ 0xffffffb0
    6a44:	e3530000 	cmp	r3, #0
    6a48:	0a000004 	beq	6a60 <sfe_ipv4_create_rule+0x3a4>
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK;
    6a4c:	e51b3058 	ldr	r3, [fp, #-88]	@ 0xffffffa8
    6a50:	e5873030 	str	r3, [r7, #48]	@ 0x30
		reply_cm->dscp = sic->dest_dscp << SFE_IPV4_DSCP_SHIFT;
    6a54:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
    6a58:	e1a03103 	lsl	r3, r3, #2
    6a5c:	e587306c 	str	r3, [r7, #108]	@ 0x6c
	reply_cm->flow_accel = sic->reply_accel;
    6a60:	e59430a0 	ldr	r3, [r4, #160]	@ 0xa0
	reply_cm->active_next = NULL;
    6a64:	e3a02000 	mov	r2, #0
	reply_cm->flow_accel = sic->reply_accel;
    6a68:	e5873034 	str	r3, [r7, #52]	@ 0x34
	reply_cm->active = false;
    6a6c:	e3a03000 	mov	r3, #0
    6a70:	e5c73018 	strb	r3, [r7, #24]
	reply_cm->active_next = NULL;
    6a74:	e3a03000 	mov	r3, #0
    6a78:	e1c721f0 	strd	r2, [r7, #16]
	if (!(src_dev->flags & IFF_POINTOPOINT)) {
    6a7c:	e59a3068 	ldr	r3, [sl, #104]	@ 0x68
    6a80:	e3130010 	tst	r3, #16
    6a84:	1a00000c 	bne	6abc <sfe_ipv4_create_rule+0x400>
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;
    6a88:	e3812010 	orr	r2, r1, #16
    6a8c:	e5872030 	str	r2, [r7, #48]	@ 0x30
		if (src_dev->header_ops) {
    6a90:	e59a3158 	ldr	r3, [sl, #344]	@ 0x158
    6a94:	e3530000 	cmp	r3, #0
    6a98:	0a000006 	beq	6ab8 <sfe_ipv4_create_rule+0x3fc>
			if (src_dev->header_ops->create == eth_header) {
    6a9c:	e5930000 	ldr	r0, [r3]
    6aa0:	e3003000 	movw	r3, #0
    6aa4:	e3403000 	movt	r3, #0
    6aa8:	e1500003 	cmp	r0, r3
				reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
    6aac:	03811018 	orreq	r1, r1, #24
    6ab0:	05871030 	streq	r1, [r7, #48]	@ 0x30
			if (src_dev->header_ops->create == eth_header) {
    6ab4:	0a000000 	beq	6abc <sfe_ipv4_create_rule+0x400>
    6ab8:	e1a01002 	mov	r1, r2
	if (sic->dest_ip.ip != sic->dest_ip_xlate.ip || sic->dest_port != sic->dest_port_xlate) {
    6abc:	e51bc03c 	ldr	ip, [fp, #-60]	@ 0xffffffc4
    6ac0:	e1a0282e 	lsr	r2, lr, #16
    6ac4:	e6ff007e 	uxth	r0, lr
    6ac8:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    6acc:	e1520000 	cmp	r2, r0
    6ad0:	0153000c 	cmpeq	r3, ip
    6ad4:	0a000004 	beq	6aec <sfe_ipv4_create_rule+0x430>
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST;
    6ad8:	e5953030 	ldr	r3, [r5, #48]	@ 0x30
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC;
    6adc:	e3811001 	orr	r1, r1, #1
    6ae0:	e5871030 	str	r1, [r7, #48]	@ 0x30
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST;
    6ae4:	e3833002 	orr	r3, r3, #2
    6ae8:	e5853030 	str	r3, [r5, #48]	@ 0x30
	if (sic->src_ip.ip != sic->src_ip_xlate.ip || sic->src_port != sic->src_port_xlate) {
    6aec:	e14b24d4 	ldrd	r2, [fp, #-68]	@ 0xffffffbc
    6af0:	e51bc038 	ldr	ip, [fp, #-56]	@ 0xffffffc8
    6af4:	e51b0030 	ldr	r0, [fp, #-48]	@ 0xffffffd0
    6af8:	e150000c 	cmp	r0, ip
    6afc:	01530002 	cmpeq	r3, r2
    6b00:	0a000004 	beq	6b18 <sfe_ipv4_create_rule+0x45c>
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC;
    6b04:	e5953030 	ldr	r3, [r5, #48]	@ 0x30
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST;
    6b08:	e3811002 	orr	r1, r1, #2
    6b0c:	e5871030 	str	r1, [r7, #48]	@ 0x30
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC;
    6b10:	e3833001 	orr	r3, r3, #1
    6b14:	e5853030 	str	r3, [r5, #48]	@ 0x30
	c->protocol = sic->protocol;
    6b18:	e51b3048 	ldr	r3, [fp, #-72]	@ 0xffffffb8
    6b1c:	e5863008 	str	r3, [r6, #8]
	c->src_ip = sic->src_ip.ip;
    6b20:	e51b3030 	ldr	r3, [fp, #-48]	@ 0xffffffd0
	c->src_port = sic->src_port;
    6b24:	e586e020 	str	lr, [r6, #32]
	c->original_match = original_cm;
    6b28:	e5865024 	str	r5, [r6, #36]	@ 0x24
	c->original_dev = src_dev;
    6b2c:	e586a028 	str	sl, [r6, #40]	@ 0x28
	c->src_ip = sic->src_ip.ip;
    6b30:	e586300c 	str	r3, [r6, #12]
	c->src_ip_xlate = sic->src_ip_xlate.ip;
    6b34:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
	c->reply_match = reply_cm;
    6b38:	e586702c 	str	r7, [r6, #44]	@ 0x2c
	c->reply_dev = dest_dev;
    6b3c:	e5869030 	str	r9, [r6, #48]	@ 0x30
	c->src_ip_xlate = sic->src_ip_xlate.ip;
    6b40:	e5863010 	str	r3, [r6, #16]
	c->dest_ip = sic->dest_ip.ip;
    6b44:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    6b48:	e5863014 	str	r3, [r6, #20]
	c->dest_ip_xlate = sic->dest_ip_xlate.ip;
    6b4c:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
    6b50:	e5863018 	str	r3, [r6, #24]
	c->src_port = sic->src_port;
    6b54:	e51b3054 	ldr	r3, [fp, #-84]	@ 0xffffffac
    6b58:	e586301c 	str	r3, [r6, #28]
	c->mark = sic->mark;
    6b5c:	e5943098 	ldr	r3, [r4, #152]	@ 0x98
    6b60:	e5863048 	str	r3, [r6, #72]	@ 0x48
	c->debug_read_seq = 0;
    6b64:	e3a03000 	mov	r3, #0
    6b68:	e586304c 	str	r3, [r6, #76]	@ 0x4c
	c->last_sync_jiffies = get_jiffies_64();
    6b6c:	ebfffffe 	bl	0 <get_jiffies_64>
	dev_hold(c->original_dev);
    6b70:	e5963028 	ldr	r3, [r6, #40]	@ 0x28
	c->last_sync_jiffies = get_jiffies_64();
    6b74:	e1c603f8 	strd	r0, [r6, #56]	@ 0x38
	if (dev) {
    6b78:	e3530000 	cmp	r3, #0
    6b7c:	0a000007 	beq	6ba0 <sfe_ipv4_create_rule+0x4e4>
	asm volatile(
    6b80:	e10f0000 	mrs	r0, CPSR
    6b84:	f10c0080 	cpsid	i
		this_cpu_inc(*dev->pcpu_refcnt);
    6b88:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    6b8c:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    6b90:	e7932001 	ldr	r2, [r3, r1]
    6b94:	e2822001 	add	r2, r2, #1
    6b98:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    6b9c:	e121f000 	msr	CPSR_c, r0
	dev_hold(c->reply_dev);
    6ba0:	e5963030 	ldr	r3, [r6, #48]	@ 0x30
	if (dev) {
    6ba4:	e3530000 	cmp	r3, #0
    6ba8:	0a000007 	beq	6bcc <sfe_ipv4_create_rule+0x510>
	asm volatile(
    6bac:	e10f0000 	mrs	r0, CPSR
    6bb0:	f10c0080 	cpsid	i
		this_cpu_inc(*dev->pcpu_refcnt);
    6bb4:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    6bb8:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    6bbc:	e7932001 	ldr	r2, [r3, r1]
    6bc0:	e2822001 	add	r2, r2, #1
    6bc4:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    6bc8:	e121f000 	msr	CPSR_c, r0
	switch (sic->protocol) {
    6bcc:	e5943000 	ldr	r3, [r4]
    6bd0:	e3530006 	cmp	r3, #6
    6bd4:	0a000065 	beq	6d70 <sfe_ipv4_create_rule+0x6b4>
	sfe_ipv4_connection_match_compute_translations(original_cm);
    6bd8:	e1a00005 	mov	r0, r5
    6bdc:	ebffe507 	bl	0 <sfe_ipv4_connection_match_compute_translations>
	sfe_ipv4_connection_match_compute_translations(reply_cm);
    6be0:	e1a00007 	mov	r0, r7
    6be4:	ebffe505 	bl	0 <sfe_ipv4_connection_match_compute_translations>
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6be8:	e1d632b0 	ldrh	r3, [r6, #32]
	c->prev = NULL;
    6bec:	e3a00000 	mov	r0, #0
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6bf0:	e1d621bc 	ldrh	r2, [r6, #28]
    6bf4:	e596100c 	ldr	r1, [r6, #12]
	c->prev = NULL;
    6bf8:	e5860004 	str	r0, [r6, #4]
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6bfc:	e5960014 	ldr	r0, [r6, #20]
    6c00:	e0222003 	eor	r2, r2, r3
    6c04:	e5d63008 	ldrb	r3, [r6, #8]
    6c08:	e6bf2fb2 	rev16	r2, r2
    6c0c:	e0211000 	eor	r1, r1, r0
    6c10:	e6ff2072 	uxth	r2, r2
    6c14:	e6bf1f31 	rev	r1, r1
    6c18:	e0233001 	eor	r3, r3, r1
	c->all_connections_next = NULL;
    6c1c:	e3a00000 	mov	r0, #0
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6c20:	e0233002 	eor	r3, r3, r2
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    6c24:	e0233623 	eor	r3, r3, r3, lsr #12
    6c28:	e7eb3053 	ubfx	r3, r3, #0, #12
	prev_head = *hash_head;
    6c2c:	e283300c 	add	r3, r3, #12
    6c30:	e7982103 	ldr	r2, [r8, r3, lsl #2]
	if (prev_head) {
    6c34:	e3520000 	cmp	r2, #0
		prev_head->prev = c;
    6c38:	15826004 	strne	r6, [r2, #4]
	if (si->all_connections_tail) {
    6c3c:	e5981010 	ldr	r1, [r8, #16]
	*hash_head = c;
    6c40:	e7886103 	str	r6, [r8, r3, lsl #2]
	si->num_connections++;
    6c44:	e5983014 	ldr	r3, [r8, #20]
	c->next = prev_head;
    6c48:	e5862000 	str	r2, [r6]
	si->all_connections_tail = c;
    6c4c:	e5886010 	str	r6, [r8, #16]
	if (si->all_connections_tail) {
    6c50:	e3510000 	cmp	r1, #0
		c->all_connections_prev = si->all_connections_tail;
    6c54:	e5861044 	str	r1, [r6, #68]	@ 0x44
	si->num_connections++;
    6c58:	e2833001 	add	r3, r3, #1
		si->all_connections_tail->all_connections_next = c;
    6c5c:	15816040 	strne	r6, [r1, #64]	@ 0x40
	sfe_ipv4_insert_sfe_ipv4_connection_match(si, c->original_match);
    6c60:	e5961024 	ldr	r1, [r6, #36]	@ 0x24
	si->num_connections++;
    6c64:	e5883014 	str	r3, [r8, #20]
	c->all_connections_next = NULL;
    6c68:	e5860040 	str	r0, [r6, #64]	@ 0x40
		si->all_connections_head = c;
    6c6c:	0588600c 	streq	r6, [r8, #12]
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6c70:	e1d132be 	ldrh	r3, [r1, #46]	@ 0x2e
    6c74:	e1d122bc 	ldrh	r2, [r1, #44]	@ 0x2c
    6c78:	e591c024 	ldr	ip, [r1, #36]	@ 0x24
    6c7c:	e5d1e020 	ldrb	lr, [r1, #32]
    6c80:	e0222003 	eor	r2, r2, r3
    6c84:	e5913028 	ldr	r3, [r1, #40]	@ 0x28
    6c88:	e6bf2fb2 	rev16	r2, r2
    6c8c:	e6ff2072 	uxth	r2, r2
    6c90:	e02cc003 	eor	ip, ip, r3
    6c94:	e591301c 	ldr	r3, [r1, #28]
    6c98:	e6bfcf3c 	rev	ip, ip
    6c9c:	e023300e 	eor	r3, r3, lr
    6ca0:	e023300c 	eor	r3, r3, ip
    6ca4:	e0233002 	eor	r3, r3, r2
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    6ca8:	e0233623 	eor	r3, r3, r3, lsr #12
    6cac:	e7eb3053 	ubfx	r3, r3, #0, #12
	prev_head = *hash_head;
    6cb0:	e2833a01 	add	r3, r3, #4096	@ 0x1000
    6cb4:	e283300c 	add	r3, r3, #12
    6cb8:	e7982103 	ldr	r2, [r8, r3, lsl #2]
	cm->prev = NULL;
    6cbc:	e5810004 	str	r0, [r1, #4]
	if (prev_head) {
    6cc0:	e1520000 	cmp	r2, r0
		prev_head->prev = cm;
    6cc4:	15821004 	strne	r1, [r2, #4]
	cm->next = prev_head;
    6cc8:	e5812000 	str	r2, [r1]
	sfe_ipv4_insert_sfe_ipv4_connection_match(si, c->reply_match);
    6ccc:	e596202c 	ldr	r2, [r6, #44]	@ 0x2c
	*hash_head = cm;
    6cd0:	e7881103 	str	r1, [r8, r3, lsl #2]
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6cd4:	e5923028 	ldr	r3, [r2, #40]	@ 0x28
    6cd8:	e5920024 	ldr	r0, [r2, #36]	@ 0x24
    6cdc:	e1d2c2be 	ldrh	ip, [r2, #46]	@ 0x2e
    6ce0:	e1d212bc 	ldrh	r1, [r2, #44]	@ 0x2c
    6ce4:	e0200003 	eor	r0, r0, r3
    6ce8:	e592301c 	ldr	r3, [r2, #28]
    6cec:	e6bf0f30 	rev	r0, r0
    6cf0:	e021100c 	eor	r1, r1, ip
    6cf4:	e5d2c020 	ldrb	ip, [r2, #32]
    6cf8:	e6bf1fb1 	rev16	r1, r1
    6cfc:	e6ff1071 	uxth	r1, r1
    6d00:	e023300c 	eor	r3, r3, ip
	cm->prev = NULL;
    6d04:	e3a0c000 	mov	ip, #0
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    6d08:	e0233000 	eor	r3, r3, r0
    6d0c:	e3000000 	movw	r0, #0
    6d10:	e0233001 	eor	r3, r3, r1
    6d14:	e3400000 	movt	r0, #0
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    6d18:	e0233623 	eor	r3, r3, r3, lsr #12
    6d1c:	e7eb3053 	ubfx	r3, r3, #0, #12
	prev_head = *hash_head;
    6d20:	e2833a01 	add	r3, r3, #4096	@ 0x1000
    6d24:	e283300c 	add	r3, r3, #12
    6d28:	e7981103 	ldr	r1, [r8, r3, lsl #2]
	cm->prev = NULL;
    6d2c:	e582c004 	str	ip, [r2, #4]
	if (prev_head) {
    6d30:	e151000c 	cmp	r1, ip
		prev_head->prev = cm;
    6d34:	15812004 	strne	r2, [r1, #4]
	cm->next = prev_head;
    6d38:	e5821000 	str	r1, [r2]
	*hash_head = cm;
    6d3c:	e7882103 	str	r2, [r8, r3, lsl #2]
    6d40:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	return 0;
    6d44:	e3a00000 	mov	r0, #0
}
    6d48:	e24bd028 	sub	sp, fp, #40	@ 0x28
    6d4c:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		original_cm->priority = sic->src_priority;
    6d50:	e59420a4 	ldr	r2, [r4, #164]	@ 0xa4
    6d54:	e5852068 	str	r2, [r5, #104]	@ 0x68
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
    6d58:	e3a02020 	mov	r2, #32
    6d5c:	e1a01002 	mov	r1, r2
    6d60:	e5852030 	str	r2, [r5, #48]	@ 0x30
    6d64:	e3a02060 	mov	r2, #96	@ 0x60
    6d68:	e50b2058 	str	r2, [fp, #-88]	@ 0xffffffa8
    6d6c:	eafffee8 	b	6914 <sfe_ipv4_create_rule+0x258>
		original_cm->protocol_state.tcp.win_scale = sic->src_td_window_scale;
    6d70:	e5d43078 	ldrb	r3, [r4, #120]	@ 0x78
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    6d74:	e594200c 	ldr	r2, [r4, #12]
		original_cm->protocol_state.tcp.win_scale = sic->src_td_window_scale;
    6d78:	e5c53038 	strb	r3, [r5, #56]	@ 0x38
		original_cm->protocol_state.tcp.max_win = sic->src_td_max_window ? sic->src_td_max_window : 1;
    6d7c:	e594307c 	ldr	r3, [r4, #124]	@ 0x7c
    6d80:	e3530001 	cmp	r3, #1
    6d84:	33a03001 	movcc	r3, #1
    6d88:	e585303c 	str	r3, [r5, #60]	@ 0x3c
		original_cm->protocol_state.tcp.end = sic->src_td_end;
    6d8c:	e5943080 	ldr	r3, [r4, #128]	@ 0x80
    6d90:	e5853040 	str	r3, [r5, #64]	@ 0x40
		original_cm->protocol_state.tcp.max_end = sic->src_td_max_end;
    6d94:	e5943084 	ldr	r3, [r4, #132]	@ 0x84
    6d98:	e5853044 	str	r3, [r5, #68]	@ 0x44
		reply_cm->protocol_state.tcp.win_scale = sic->dest_td_window_scale;
    6d9c:	e5d43088 	ldrb	r3, [r4, #136]	@ 0x88
    6da0:	e5c73038 	strb	r3, [r7, #56]	@ 0x38
		reply_cm->protocol_state.tcp.max_win = sic->dest_td_max_window ? sic->dest_td_max_window : 1;
    6da4:	e594308c 	ldr	r3, [r4, #140]	@ 0x8c
    6da8:	e3530001 	cmp	r3, #1
    6dac:	33a03001 	movcc	r3, #1
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    6db0:	e3120001 	tst	r2, #1
		reply_cm->protocol_state.tcp.max_win = sic->dest_td_max_window ? sic->dest_td_max_window : 1;
    6db4:	e587303c 	str	r3, [r7, #60]	@ 0x3c
		reply_cm->protocol_state.tcp.end = sic->dest_td_end;
    6db8:	e5943090 	ldr	r3, [r4, #144]	@ 0x90
    6dbc:	e5873040 	str	r3, [r7, #64]	@ 0x40
		reply_cm->protocol_state.tcp.max_end = sic->dest_td_max_end;
    6dc0:	e5943094 	ldr	r3, [r4, #148]	@ 0x94
    6dc4:	e5873044 	str	r3, [r7, #68]	@ 0x44
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    6dc8:	0affff82 	beq	6bd8 <sfe_ipv4_create_rule+0x51c>
			original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    6dcc:	e5952030 	ldr	r2, [r5, #48]	@ 0x30
			reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    6dd0:	e5973030 	ldr	r3, [r7, #48]	@ 0x30
			original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    6dd4:	e3822004 	orr	r2, r2, #4
			reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    6dd8:	e3833004 	orr	r3, r3, #4
			original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    6ddc:	e5852030 	str	r2, [r5, #48]	@ 0x30
			reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    6de0:	e5873030 	str	r3, [r7, #48]	@ 0x30
    6de4:	eaffff7b 	b	6bd8 <sfe_ipv4_create_rule+0x51c>
		return -EINVAL;
    6de8:	e3e00015 	mvn	r0, #21
}
    6dec:	e24bd028 	sub	sp, fp, #40	@ 0x28
    6df0:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
    6df4:	e3000000 	movw	r0, #0
    6df8:	e3400000 	movt	r0, #0
    6dfc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return -ENOMEM;
    6e00:	e3e0000b 	mvn	r0, #11
    6e04:	eaffffcf 	b	6d48 <sfe_ipv4_create_rule+0x68c>
    6e08:	e3000000 	movw	r0, #0
    6e0c:	e3400000 	movt	r0, #0
    6e10:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		kfree(c);
    6e14:	e1a00006 	mov	r0, r6
    6e18:	ebfffffe 	bl	0 <kfree>
		return -ENOMEM;
    6e1c:	eafffff7 	b	6e00 <sfe_ipv4_create_rule+0x744>
    6e20:	e3000000 	movw	r0, #0
    6e24:	e3400000 	movt	r0, #0
    6e28:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		kfree(original_cm);
    6e2c:	e1a00005 	mov	r0, r5
    6e30:	ebfffffe 	bl	0 <kfree>
		kfree(c);
    6e34:	e1a00006 	mov	r0, r6
    6e38:	ebfffffe 	bl	0 <kfree>
		return -ENOMEM;
    6e3c:	eaffffef 	b	6e00 <sfe_ipv4_create_rule+0x744>
		sfe_ipv4_update_tcp_state(c, sic);
    6e40:	e5930024 	ldr	r0, [r3, #36]	@ 0x24
    6e44:	e1a02004 	mov	r2, r4
    6e48:	e593102c 	ldr	r1, [r3, #44]	@ 0x2c
    6e4c:	ebffead3 	bl	19a0 <sfe_ipv4_update_tcp_state.isra.0>
		break;
    6e50:	eafffe5a 	b	67c0 <sfe_ipv4_create_rule+0x104>

00006e54 <sfe_ipv6_flush_connection.constprop.0>:
static void sfe_ipv6_flush_connection(struct sfe_ipv6 *si,
    6e54:	e1a0c00d 	mov	ip, sp
    6e58:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    6e5c:	e24cb004 	sub	fp, ip, #4
    6e60:	e24dd0cc 	sub	sp, sp, #204	@ 0xcc
    6e64:	e1a04000 	mov	r4, r0
	raw_spin_lock_bh(&lock->rlock);
    6e68:	e3005000 	movw	r5, #0
    6e6c:	e3405000 	movt	r5, #0
    6e70:	e1a00005 	mov	r0, r5
    6e74:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_flushes++;
    6e78:	e2852902 	add	r2, r5, #32768	@ 0x8000
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
    6e7c:	e595602c 	ldr	r6, [r5, #44]	@ 0x2c
	raw_spin_unlock_bh(&lock->rlock);
    6e80:	e1a00005 	mov	r0, r5
	si->connection_flushes++;
    6e84:	e5923048 	ldr	r3, [r2, #72]	@ 0x48
    6e88:	e2833001 	add	r3, r3, #1
    6e8c:	e5823048 	str	r3, [r2, #72]	@ 0x48
    6e90:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (sync_rule_callback) {
    6e94:	e3560000 	cmp	r6, #0
    6e98:	0a00006d 	beq	7054 <sfe_ipv6_flush_connection.constprop.0+0x200>
		now_jiffies = get_jiffies_64();
    6e9c:	ebfffffe 	bl	0 <get_jiffies_64>
	sis->protocol = c->protocol;
    6ea0:	e5943008 	ldr	r3, [r4, #8]
	sis->is_v6 = 1;
    6ea4:	e3a02001 	mov	r2, #1
		now_jiffies = get_jiffies_64();
    6ea8:	e1a05000 	mov	r5, r0
	sis->dest_port = c->dest_port;
    6eac:	e5940050 	ldr	r0, [r4, #80]	@ 0x50
	cm->rx_packet_count64 += cm->rx_packet_count;
    6eb0:	e3a09000 	mov	r9, #0
	sis->reason = reason;
    6eb4:	e3a0a002 	mov	sl, #2
	sis->protocol = c->protocol;
    6eb8:	e14b2dfc 	strd	r2, [fp, #-220]	@ 0xffffff24
	sis->src_ip.ip6[0] = c->src_ip[0];
    6ebc:	e1c420dc 	ldrd	r2, [r4, #12]
    6ec0:	e14b2df4 	strd	r2, [fp, #-212]	@ 0xffffff2c
    6ec4:	e1c421d4 	ldrd	r2, [r4, #20]
	sis->dest_port = c->dest_port;
    6ec8:	e50b0090 	str	r0, [fp, #-144]	@ 0xffffff70
		sync_rule_callback(&sis);
    6ecc:	e24b00e4 	sub	r0, fp, #228	@ 0xe4
	sis->src_ip.ip6[0] = c->src_ip[0];
    6ed0:	e14b2cfc 	strd	r2, [fp, #-204]	@ 0xffffff34
	sis->src_ip_xlate.ip6[0] = c->src_ip_xlate[0];
    6ed4:	e1c421dc 	ldrd	r2, [r4, #28]
    6ed8:	e14b2cf4 	strd	r2, [fp, #-196]	@ 0xffffff3c
    6edc:	e1c422d4 	ldrd	r2, [r4, #36]	@ 0x24
    6ee0:	e14b2bfc 	strd	r2, [fp, #-188]	@ 0xffffff44
	sis->dest_ip.ip6[0] = c->dest_ip[0];
    6ee4:	e1c422dc 	ldrd	r2, [r4, #44]	@ 0x2c
    6ee8:	e14b2bf0 	strd	r2, [fp, #-176]	@ 0xffffff50
    6eec:	e1c423d4 	ldrd	r2, [r4, #52]	@ 0x34
    6ef0:	e14b2af8 	strd	r2, [fp, #-168]	@ 0xffffff58
	sis->dest_ip_xlate.ip6[0] = c->dest_ip_xlate[0];
    6ef4:	e1c423dc 	ldrd	r2, [r4, #60]	@ 0x3c
    6ef8:	e14b2af0 	strd	r2, [fp, #-160]	@ 0xffffff60
    6efc:	e1c424d4 	ldrd	r2, [r4, #68]	@ 0x44
    6f00:	e14b29f8 	strd	r2, [fp, #-152]	@ 0xffffff68
	sis->src_port = c->src_port;
    6f04:	e594204c 	ldr	r2, [r4, #76]	@ 0x4c
	original_cm = c->original_match;
    6f08:	e5943054 	ldr	r3, [r4, #84]	@ 0x54
	sis->src_port = c->src_port;
    6f0c:	e50b20b4 	str	r2, [fp, #-180]	@ 0xffffff4c
	sis->src_td_max_window = original_cm->protocol_state.tcp.max_win;
    6f10:	e5932054 	ldr	r2, [r3, #84]	@ 0x54
    6f14:	e50b208c 	str	r2, [fp, #-140]	@ 0xffffff74
	sis->src_td_end = original_cm->protocol_state.tcp.end;
    6f18:	e5932058 	ldr	r2, [r3, #88]	@ 0x58
    6f1c:	e50b2088 	str	r2, [fp, #-136]	@ 0xffffff78
	sis->src_td_max_end = original_cm->protocol_state.tcp.max_end;
    6f20:	e593205c 	ldr	r2, [r3, #92]	@ 0x5c
    6f24:	e50b2084 	str	r2, [fp, #-132]	@ 0xffffff7c
	reply_cm = c->reply_match;
    6f28:	e594205c 	ldr	r2, [r4, #92]	@ 0x5c
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
    6f2c:	e592c054 	ldr	ip, [r2, #84]	@ 0x54
    6f30:	e50bc064 	str	ip, [fp, #-100]	@ 0xffffff9c
	sis->dest_td_end = reply_cm->protocol_state.tcp.end;
    6f34:	e592c058 	ldr	ip, [r2, #88]	@ 0x58
    6f38:	e50bc060 	str	ip, [fp, #-96]	@ 0xffffffa0
	sis->dest_td_max_end = reply_cm->protocol_state.tcp.max_end;
    6f3c:	e592c05c 	ldr	ip, [r2, #92]	@ 0x5c
    6f40:	e50bc05c 	str	ip, [fp, #-92]	@ 0xffffffa4
	sis->src_new_packet_count = original_cm->rx_packet_count;
    6f44:	e593c060 	ldr	ip, [r3, #96]	@ 0x60
    6f48:	e50bc06c 	str	ip, [fp, #-108]	@ 0xffffff94
	sis->src_new_byte_count = original_cm->rx_byte_count;
    6f4c:	e593c064 	ldr	ip, [r3, #100]	@ 0x64
    6f50:	e50bc068 	str	ip, [fp, #-104]	@ 0xffffff98
	sis->dest_new_packet_count = reply_cm->rx_packet_count;
    6f54:	e592c060 	ldr	ip, [r2, #96]	@ 0x60
    6f58:	e50bc044 	str	ip, [fp, #-68]	@ 0xffffffbc
	sis->dest_new_byte_count = reply_cm->rx_byte_count;
    6f5c:	e592c064 	ldr	ip, [r2, #100]	@ 0x64
    6f60:	e50bc040 	str	ip, [fp, #-64]	@ 0xffffffc0
	cm->rx_packet_count64 += cm->rx_packet_count;
    6f64:	e593e060 	ldr	lr, [r3, #96]	@ 0x60
    6f68:	e593c0b0 	ldr	ip, [r3, #176]	@ 0xb0
	cm->rx_packet_count = 0;
    6f6c:	e5839060 	str	r9, [r3, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    6f70:	e09cc00e 	adds	ip, ip, lr
	cm->rx_byte_count64 += cm->rx_byte_count;
    6f74:	e593e064 	ldr	lr, [r3, #100]	@ 0x64
	cm->rx_byte_count = 0;
    6f78:	e5839064 	str	r9, [r3, #100]	@ 0x64
	cm->rx_packet_count64 += cm->rx_packet_count;
    6f7c:	e583c0b0 	str	ip, [r3, #176]	@ 0xb0
    6f80:	e593c0b4 	ldr	ip, [r3, #180]	@ 0xb4
    6f84:	e2acc000 	adc	ip, ip, #0
    6f88:	e583c0b4 	str	ip, [r3, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    6f8c:	e593c0b8 	ldr	ip, [r3, #184]	@ 0xb8
    6f90:	e09cc00e 	adds	ip, ip, lr
    6f94:	e583c0b8 	str	ip, [r3, #184]	@ 0xb8
    6f98:	e593c0bc 	ldr	ip, [r3, #188]	@ 0xbc
    6f9c:	e2acc000 	adc	ip, ip, #0
    6fa0:	e583c0bc 	str	ip, [r3, #188]	@ 0xbc
	cm->rx_packet_count64 += cm->rx_packet_count;
    6fa4:	e592e060 	ldr	lr, [r2, #96]	@ 0x60
    6fa8:	e592c0b0 	ldr	ip, [r2, #176]	@ 0xb0
	cm->rx_packet_count = 0;
    6fac:	e5829060 	str	r9, [r2, #96]	@ 0x60
	cm->rx_packet_count64 += cm->rx_packet_count;
    6fb0:	e59280b4 	ldr	r8, [r2, #180]	@ 0xb4
    6fb4:	e09ee00c 	adds	lr, lr, ip
    6fb8:	e2a8c000 	adc	ip, r8, #0
    6fbc:	e582e0b0 	str	lr, [r2, #176]	@ 0xb0
    6fc0:	e50bc0e8 	str	ip, [fp, #-232]	@ 0xffffff18
    6fc4:	e582c0b4 	str	ip, [r2, #180]	@ 0xb4
	cm->rx_byte_count64 += cm->rx_byte_count;
    6fc8:	e592c064 	ldr	ip, [r2, #100]	@ 0x64
    6fcc:	e59270b8 	ldr	r7, [r2, #184]	@ 0xb8
	cm->rx_byte_count = 0;
    6fd0:	e5829064 	str	r9, [r2, #100]	@ 0x64
	cm->rx_byte_count64 += cm->rx_byte_count;
    6fd4:	e09cc007 	adds	ip, ip, r7
    6fd8:	e59270bc 	ldr	r7, [r2, #188]	@ 0xbc
    6fdc:	e582c0b8 	str	ip, [r2, #184]	@ 0xb8
    6fe0:	e2a77000 	adc	r7, r7, #0
    6fe4:	e58270bc 	str	r7, [r2, #188]	@ 0xbc
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    6fe8:	e5949068 	ldr	r9, [r4, #104]	@ 0x68
    6fec:	e0558009 	subs	r8, r5, r9
    6ff0:	e594906c 	ldr	r9, [r4, #108]	@ 0x6c
    6ff4:	e50b80ec 	str	r8, [fp, #-236]	@ 0xffffff14
    6ff8:	e0c18009 	sbc	r8, r1, r9
	sis->src_dev = original_cm->match_dev;
    6ffc:	e593901c 	ldr	r9, [r3, #28]
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    7000:	e50b80f0 	str	r8, [fp, #-240]	@ 0xffffff10
	sis->src_dev = original_cm->match_dev;
    7004:	e50b90e4 	str	r9, [fp, #-228]	@ 0xffffff1c
	sis->src_packet_count = original_cm->rx_packet_count64;
    7008:	e1c38bd0 	ldrd	r8, [r3, #176]	@ 0xb0
    700c:	e14b87fc 	strd	r8, [fp, #-124]	@ 0xffffff84
	sis->src_byte_count = original_cm->rx_byte_count64;
    7010:	e1c38bd8 	ldrd	r8, [r3, #184]	@ 0xb8
    7014:	e14b87f4 	strd	r8, [fp, #-116]	@ 0xffffff8c
	sis->dest_dev = reply_cm->match_dev;
    7018:	e592301c 	ldr	r3, [r2, #28]
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    701c:	e51b80ec 	ldr	r8, [fp, #-236]	@ 0xffffff14
	c->last_sync_jiffies = now_jiffies;
    7020:	e5845068 	str	r5, [r4, #104]	@ 0x68
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    7024:	e51b20e8 	ldr	r2, [fp, #-232]	@ 0xffffff18
	c->last_sync_jiffies = now_jiffies;
    7028:	e584106c 	str	r1, [r4, #108]	@ 0x6c
	sis->dest_dev = reply_cm->match_dev;
    702c:	e50b30e0 	str	r3, [fp, #-224]	@ 0xffffff20
	sis->dest_packet_count = reply_cm->rx_packet_count64;
    7030:	e50be054 	str	lr, [fp, #-84]	@ 0xffffffac
    7034:	e50b2050 	str	r2, [fp, #-80]	@ 0xffffffb0
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    7038:	e51b20f0 	ldr	r2, [fp, #-240]	@ 0xffffff10
	sis->dest_byte_count = reply_cm->rx_byte_count64;
    703c:	e50bc04c 	str	ip, [fp, #-76]	@ 0xffffffb4
    7040:	e50b7048 	str	r7, [fp, #-72]	@ 0xffffffb8
	sis->reason = reason;
    7044:	e50ba03c 	str	sl, [fp, #-60]	@ 0xffffffc4
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
    7048:	e50b8034 	str	r8, [fp, #-52]	@ 0xffffffcc
    704c:	e50b2030 	str	r2, [fp, #-48]	@ 0xffffffd0
		sync_rule_callback(&sis);
    7050:	e12fff36 	blx	r6
	dev_put(c->original_dev);
    7054:	e5943058 	ldr	r3, [r4, #88]	@ 0x58
	if (dev) {
    7058:	e3530000 	cmp	r3, #0
    705c:	0a000007 	beq	7080 <sfe_ipv6_flush_connection.constprop.0+0x22c>
	asm volatile(
    7060:	e10f0000 	mrs	r0, CPSR
    7064:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    7068:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    706c:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    7070:	e7932001 	ldr	r2, [r3, r1]
    7074:	e2422001 	sub	r2, r2, #1
    7078:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    707c:	e121f000 	msr	CPSR_c, r0
	dev_put(c->reply_dev);
    7080:	e5943060 	ldr	r3, [r4, #96]	@ 0x60
	if (dev) {
    7084:	e3530000 	cmp	r3, #0
    7088:	0a000007 	beq	70ac <sfe_ipv6_flush_connection.constprop.0+0x258>
	asm volatile(
    708c:	e10f0000 	mrs	r0, CPSR
    7090:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    7094:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    7098:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    709c:	e7932001 	ldr	r2, [r3, r1]
    70a0:	e2422001 	sub	r2, r2, #1
    70a4:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    70a8:	e121f000 	msr	CPSR_c, r0
	kfree(c->original_match);
    70ac:	e5940054 	ldr	r0, [r4, #84]	@ 0x54
    70b0:	ebfffffe 	bl	0 <kfree>
	kfree(c->reply_match);
    70b4:	e594005c 	ldr	r0, [r4, #92]	@ 0x5c
    70b8:	ebfffffe 	bl	0 <kfree>
	kfree(c);
    70bc:	e1a00004 	mov	r0, r4
    70c0:	ebfffffe 	bl	0 <kfree>
}
    70c4:	e24bd028 	sub	sp, fp, #40	@ 0x28
    70c8:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}

000070cc <fast_classifier_set_defunct_all>:

static ssize_t fast_classifier_set_defunct_all(struct device *dev,
                                      struct device_attribute *attr,
                                      const char *buf, size_t count)
{
    70cc:	e1a0c00d 	mov	ip, sp
    70d0:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
    70d4:	e24cb004 	sub	fp, ip, #4
	struct sfe_ipv4 *si = &__si;
    70d8:	e3005000 	movw	r5, #0
    70dc:	e1a06003 	mov	r6, r3
    70e0:	e3405000 	movt	r5, #0
    70e4:	ea000004 	b	70fc <fast_classifier_set_defunct_all+0x30>
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    70e8:	ebffec5a 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
    70ec:	e1a00005 	mov	r0, r5
    70f0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
    70f4:	e1a00004 	mov	r0, r4
    70f8:	ebffeaf5 	bl	1cd4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0>
	raw_spin_lock_bh(&lock->rlock);
    70fc:	e1a00005 	mov	r0, r5
    7100:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    7104:	e595400c 	ldr	r4, [r5, #12]
    7108:	e2540000 	subs	r0, r4, #0
    710c:	1afffff5 	bne	70e8 <fast_classifier_set_defunct_all+0x1c>
	raw_spin_unlock_bh(&lock->rlock);
    7110:	e3000000 	movw	r0, #0
    7114:	e3005000 	movw	r5, #0
    7118:	e3400000 	movt	r0, #0
    711c:	e3405000 	movt	r5, #0
    7120:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (c) {
    7124:	ea000004 	b	713c <fast_classifier_set_defunct_all+0x70>
		sfe_ipv6_remove_connection(si, c);
    7128:	ebffeccd 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
    712c:	e1a00005 	mov	r0, r5
    7130:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
    7134:	e1a00004 	mov	r0, r4
    7138:	ebffff45 	bl	6e54 <sfe_ipv6_flush_connection.constprop.0>
	raw_spin_lock_bh(&lock->rlock);
    713c:	e1a00005 	mov	r0, r5
    7140:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    7144:	e595400c 	ldr	r4, [r5, #12]
    7148:	e2540000 	subs	r0, r4, #0
    714c:	1afffff5 	bne	7128 <fast_classifier_set_defunct_all+0x5c>
	raw_spin_unlock_bh(&lock->rlock);
    7150:	e3000000 	movw	r0, #0
    7154:	e3400000 	movt	r0, #0
    7158:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	sfe_ipv4_destroy_all_rules_for_dev(NULL);
#ifdef SFE_SUPPORT_IPV6
	sfe_ipv6_destroy_all_rules_for_dev(NULL);
#endif
	return count;
}
    715c:	e1a00006 	mov	r0, r6
    7160:	e24bd01c 	sub	sp, fp, #28
    7164:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

00007168 <fast_classifier_inet6_event>:
{
    7168:	e1a0c00d 	mov	ip, sp
    716c:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
    7170:	e24cb004 	sub	fp, ip, #4
	struct net_device *dev = ((struct inet6_ifaddr *)ptr)->idev->dev;
    7174:	e5923074 	ldr	r3, [r2, #116]	@ 0x74
    7178:	e5935000 	ldr	r5, [r3]
	if (dev && (event == NETDEV_DOWN)) {
    717c:	e2553000 	subs	r3, r5, #0
    7180:	13a03001 	movne	r3, #1
    7184:	e3510002 	cmp	r1, #2
    7188:	13a03000 	movne	r3, #0
    718c:	e3530000 	cmp	r3, #0
    7190:	1a000002 	bne	71a0 <fast_classifier_inet6_event+0x38>
}
    7194:	e3a00000 	mov	r0, #0
    7198:	e24bd01c 	sub	sp, fp, #28
    719c:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
    71a0:	e3006000 	movw	r6, #0
    71a4:	e3406000 	movt	r6, #0
	raw_spin_lock_bh(&lock->rlock);
    71a8:	e1a00006 	mov	r0, r6
    71ac:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
    71b0:	e596400c 	ldr	r4, [r6, #12]
    71b4:	e3540000 	cmp	r4, #0
    71b8:	1a000006 	bne	71d8 <fast_classifier_inet6_event+0x70>
    71bc:	ea00000f 	b	7200 <fast_classifier_inet6_event+0x98>
		    || (dev == c->reply_dev)) {
    71c0:	e5943060 	ldr	r3, [r4, #96]	@ 0x60
    71c4:	e1550003 	cmp	r5, r3
    71c8:	0a000005 	beq	71e4 <fast_classifier_inet6_event+0x7c>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    71cc:	e5944070 	ldr	r4, [r4, #112]	@ 0x70
    71d0:	e3540000 	cmp	r4, #0
    71d4:	0a000009 	beq	7200 <fast_classifier_inet6_event+0x98>
		    || (dev == c->original_dev)
    71d8:	e5943058 	ldr	r3, [r4, #88]	@ 0x58
    71dc:	e1550003 	cmp	r5, r3
    71e0:	1afffff6 	bne	71c0 <fast_classifier_inet6_event+0x58>
		sfe_ipv6_remove_connection(si, c);
    71e4:	e1a00004 	mov	r0, r4
    71e8:	ebffec9d 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
    71ec:	e1a00006 	mov	r0, r6
    71f0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
    71f4:	e1a00004 	mov	r0, r4
    71f8:	ebffff15 	bl	6e54 <sfe_ipv6_flush_connection.constprop.0>
		goto another_round;
    71fc:	eaffffe9 	b	71a8 <fast_classifier_inet6_event+0x40>
    7200:	e3000000 	movw	r0, #0
    7204:	e3400000 	movt	r0, #0
    7208:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    720c:	e3a00000 	mov	r0, #0
    7210:	e24bd01c 	sub	sp, fp, #28
    7214:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

00007218 <fast_classifier_device_event>:
{
    7218:	e1a0c00d 	mov	ip, sp
    721c:	e92dd8f0 	push	{r4, r5, r6, r7, fp, ip, lr, pc}
    7220:	e24cb004 	sub	fp, ip, #4
	return info->dev;
    7224:	e5925000 	ldr	r5, [r2]
	if (dev && (event == NETDEV_DOWN)) {
    7228:	e2553000 	subs	r3, r5, #0
    722c:	13a03001 	movne	r3, #1
    7230:	e3510002 	cmp	r1, #2
    7234:	13a03000 	movne	r3, #0
    7238:	e3530000 	cmp	r3, #0
    723c:	1a000002 	bne	724c <fast_classifier_device_event+0x34>
}
    7240:	e3a00000 	mov	r0, #0
    7244:	e24bd01c 	sub	sp, fp, #28
    7248:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}
    724c:	e3006000 	movw	r6, #0
    7250:	e3406000 	movt	r6, #0
	raw_spin_lock_bh(&lock->rlock);
    7254:	e1a00006 	mov	r0, r6
    7258:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    725c:	e596400c 	ldr	r4, [r6, #12]
    7260:	e3540000 	cmp	r4, #0
    7264:	1a000006 	bne	7284 <fast_classifier_device_event+0x6c>
    7268:	ea00000f 	b	72ac <fast_classifier_device_event+0x94>
		    || (dev == c->reply_dev)) {
    726c:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    7270:	e1550003 	cmp	r5, r3
    7274:	0a000005 	beq	7290 <fast_classifier_device_event+0x78>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    7278:	e5944040 	ldr	r4, [r4, #64]	@ 0x40
    727c:	e3540000 	cmp	r4, #0
    7280:	0a000009 	beq	72ac <fast_classifier_device_event+0x94>
		    || (dev == c->original_dev)
    7284:	e5943028 	ldr	r3, [r4, #40]	@ 0x28
    7288:	e1550003 	cmp	r5, r3
    728c:	1afffff6 	bne	726c <fast_classifier_device_event+0x54>
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    7290:	e1a00004 	mov	r0, r4
    7294:	ebffebef 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
    7298:	e1a00006 	mov	r0, r6
    729c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
    72a0:	e1a00004 	mov	r0, r4
    72a4:	ebffea8a 	bl	1cd4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0>
		goto another_round;
    72a8:	eaffffe9 	b	7254 <fast_classifier_device_event+0x3c>
    72ac:	e3000000 	movw	r0, #0
    72b0:	e3006000 	movw	r6, #0
    72b4:	e3400000 	movt	r0, #0
    72b8:	e3406000 	movt	r6, #0
    72bc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	raw_spin_lock_bh(&lock->rlock);
    72c0:	e1a00006 	mov	r0, r6
    72c4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    72c8:	e596400c 	ldr	r4, [r6, #12]
    72cc:	e3540000 	cmp	r4, #0
    72d0:	1a000006 	bne	72f0 <fast_classifier_device_event+0xd8>
    72d4:	ea00000f 	b	7318 <fast_classifier_device_event+0x100>
		    || (dev == c->reply_dev)) {
    72d8:	e5943060 	ldr	r3, [r4, #96]	@ 0x60
    72dc:	e1550003 	cmp	r5, r3
    72e0:	0a000005 	beq	72fc <fast_classifier_device_event+0xe4>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
    72e4:	e5944070 	ldr	r4, [r4, #112]	@ 0x70
    72e8:	e3540000 	cmp	r4, #0
    72ec:	0a000009 	beq	7318 <fast_classifier_device_event+0x100>
		    || (dev == c->original_dev)
    72f0:	e5943058 	ldr	r3, [r4, #88]	@ 0x58
    72f4:	e1550003 	cmp	r5, r3
    72f8:	1afffff6 	bne	72d8 <fast_classifier_device_event+0xc0>
		sfe_ipv6_remove_connection(si, c);
    72fc:	e1a00004 	mov	r0, r4
    7300:	ebffec57 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
    7304:	e1a00006 	mov	r0, r6
    7308:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
    730c:	e1a00004 	mov	r0, r4
    7310:	ebfffecf 	bl	6e54 <sfe_ipv6_flush_connection.constprop.0>
		goto another_round;
    7314:	eaffffe9 	b	72c0 <fast_classifier_device_event+0xa8>
    7318:	e3000000 	movw	r0, #0
    731c:	e3400000 	movt	r0, #0
    7320:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    7324:	e3a00000 	mov	r0, #0
    7328:	e24bd01c 	sub	sp, fp, #28
    732c:	e89da8f0 	ldm	sp, {r4, r5, r6, r7, fp, sp, pc}

00007330 <sfe_ipv6_recv>:
{
    7330:	e1a0c00d 	mov	ip, sp
    7334:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
    7338:	e24cb004 	sub	fp, ip, #4
    733c:	e24dd008 	sub	sp, sp, #8
	len = skb->len;
    7340:	e591705c 	ldr	r7, [r1, #92]	@ 0x5c
{
    7344:	e1a04001 	mov	r4, r1
    7348:	e1a06000 	mov	r6, r0
	return skb->len - skb->data_len;
    734c:	e5911060 	ldr	r1, [r1, #96]	@ 0x60
    7350:	e0473001 	sub	r3, r7, r1
	if (likely(len <= skb_headlen(skb)))
    7354:	e353002f 	cmp	r3, #47	@ 0x2f
    7358:	9a00009b 	bls	75cc <sfe_ipv6_recv+0x29c>
	iph = (struct sfe_ipv6_ip_hdr *)skb->data;
    735c:	e59480b0 	ldr	r8, [r4, #176]	@ 0xb0
	if (unlikely(iph->version != 6)) {
    7360:	e5d83000 	ldrb	r3, [r8]
    7364:	e20330f0 	and	r3, r3, #240	@ 0xf0
    7368:	e3530060 	cmp	r3, #96	@ 0x60
    736c:	1a0000ab 	bne	7620 <sfe_ipv6_recv+0x2f0>
	payload_len = ntohs(iph->payload_len);
    7370:	e1d8c0b4 	ldrh	ip, [r8, #4]
	if (unlikely(payload_len > (len - ihl))) {
    7374:	e2473028 	sub	r3, r7, #40	@ 0x28
	payload_len = ntohs(iph->payload_len);
    7378:	e6bfcfbc 	rev16	ip, ip
    737c:	e6ffc07c 	uxth	ip, ip
	if (unlikely(payload_len > (len - ihl))) {
    7380:	e153000c 	cmp	r3, ip
    7384:	3a0000b2 	bcc	7654 <sfe_ipv6_recv+0x324>
	next_hdr = iph->nexthdr;
    7388:	e5d8c006 	ldrb	ip, [r8, #6]
	bool flush_on_find = false;
    738c:	e3a0e000 	mov	lr, #0
	unsigned int ihl = sizeof(struct sfe_ipv6_ip_hdr);
    7390:	e3a05028 	mov	r5, #40	@ 0x28
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
    7394:	e35c003c 	cmp	ip, #60	@ 0x3c
    7398:	8a00001d 	bhi	7414 <sfe_ipv6_recv+0xe4>
    739c:	e35c002a 	cmp	ip, #42	@ 0x2a
    73a0:	9a00002c 	bls	7458 <sfe_ipv6_recv+0x128>
    73a4:	e24c202b 	sub	r2, ip, #43	@ 0x2b
    73a8:	e3003103 	movw	r3, #259	@ 0x103
    73ac:	e6ef2072 	uxtb	r2, r2
    73b0:	e3403002 	movt	r3, #2
    73b4:	e1a03233 	lsr	r3, r3, r2
    73b8:	e3130001 	tst	r3, #1
    73bc:	0a000035 	beq	7498 <sfe_ipv6_recv+0x168>
		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
    73c0:	e59490b0 	ldr	r9, [r4, #176]	@ 0xb0
		if (next_hdr == SFE_IPV6_EXT_HDR_FRAG) {
    73c4:	e35c002c 	cmp	ip, #44	@ 0x2c
		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
    73c8:	e0899005 	add	r9, r9, r5
		if (next_hdr == SFE_IPV6_EXT_HDR_FRAG) {
    73cc:	1a000004 	bne	73e4 <sfe_ipv6_recv+0xb4>
			unsigned int frag_off = ntohs(frag_hdr->frag_off);
    73d0:	e1d930b2 	ldrh	r3, [r9, #2]
    73d4:	e6bf3fb3 	rev16	r3, r3
    73d8:	e6ff3073 	uxth	r3, r3
			if (frag_off & SFE_IPV6_FRAG_OFFSET) {
    73dc:	e3530007 	cmp	r3, #7
    73e0:	8a000109 	bhi	780c <sfe_ipv6_recv+0x4dc>
		ext_hdr_len = ext_hdr->hdr_len;
    73e4:	e5d93001 	ldrb	r3, [r9, #1]
		ihl += ext_hdr_len;
    73e8:	e2855008 	add	r5, r5, #8
    73ec:	e0855183 	add	r5, r5, r3, lsl #3
	return skb->len - skb->data_len;
    73f0:	e1c425dc 	ldrd	r2, [r4, #92]	@ 0x5c
		if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {
    73f4:	e2851008 	add	r1, r5, #8
    73f8:	e0420003 	sub	r0, r2, r3
	if (likely(len <= skb_headlen(skb)))
    73fc:	e1510000 	cmp	r1, r0
    7400:	8a0000a0 	bhi	7688 <sfe_ipv6_recv+0x358>
		next_hdr = ext_hdr->next_hdr;
    7404:	e5d9c000 	ldrb	ip, [r9]
		flush_on_find = true;
    7408:	e3a0e001 	mov	lr, #1
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
    740c:	e35c003c 	cmp	ip, #60	@ 0x3c
    7410:	9affffe1 	bls	739c <sfe_ipv6_recv+0x6c>
    7414:	e35c0087 	cmp	ip, #135	@ 0x87
    7418:	0a00001b 	beq	748c <sfe_ipv6_recv+0x15c>
	raw_spin_lock_bh(&lock->rlock);
    741c:	e3000000 	movw	r0, #0
    7420:	e3400000 	movt	r0, #0
    7424:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    7428:	e59f34b0 	ldr	r3, [pc, #1200]	@ 78e0 <sfe_ipv6_recv+0x5b0>
	si->packets_not_forwarded++;
    742c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7430:	e2430902 	sub	r0, r3, #32768	@ 0x8000
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    7434:	e59320e0 	ldr	r2, [r3, #224]	@ 0xe0
	si->packets_not_forwarded++;
    7438:	e2811001 	add	r1, r1, #1
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    743c:	e2822001 	add	r2, r2, #1
	si->packets_not_forwarded++;
    7440:	e5831050 	str	r1, [r3, #80]	@ 0x50
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    7444:	e58320e0 	str	r2, [r3, #224]	@ 0xe0
    7448:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    744c:	e3a00000 	mov	r0, #0
}
    7450:	e24bd024 	sub	sp, fp, #36	@ 0x24
    7454:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
    7458:	e35c0000 	cmp	ip, #0
    745c:	0a00000a 	beq	748c <sfe_ipv6_recv+0x15c>
	if (IPPROTO_UDP == next_hdr) {
    7460:	e35c0011 	cmp	ip, #17
    7464:	0a000051 	beq	75b0 <sfe_ipv6_recv+0x280>
	if (IPPROTO_TCP == next_hdr) {
    7468:	e35c0006 	cmp	ip, #6
    746c:	1affffea 	bne	741c <sfe_ipv6_recv+0xec>
		return sfe_ipv6_recv_tcp(si, skb, dev, len, iph, ihl, flush_on_find);
    7470:	e1a03008 	mov	r3, r8
    7474:	e1a02007 	mov	r2, r7
    7478:	e88d4020 	stm	sp, {r5, lr}
    747c:	e1a01006 	mov	r1, r6
    7480:	e1a00004 	mov	r0, r4
    7484:	ebfff86c 	bl	563c <sfe_ipv6_recv_tcp.constprop.0>
    7488:	eafffff0 	b	7450 <sfe_ipv6_recv+0x120>
		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
    748c:	e59490b0 	ldr	r9, [r4, #176]	@ 0xb0
    7490:	e0899005 	add	r9, r9, r5
		if (next_hdr == SFE_IPV6_EXT_HDR_FRAG) {
    7494:	eaffffd2 	b	73e4 <sfe_ipv6_recv+0xb4>
	if (IPPROTO_ICMPV6 == next_hdr) {
    7498:	e35c003a 	cmp	ip, #58	@ 0x3a
    749c:	1affffde 	bne	741c <sfe_ipv6_recv+0xec>
	return skb->len - skb->data_len;
    74a0:	e1c425dc 	ldrd	r2, [r4, #92]	@ 0x5c
	if (!pskb_may_pull(skb, ihl + sizeof(struct icmp6hdr))) {
    74a4:	e2851008 	add	r1, r5, #8
    74a8:	e0420003 	sub	r0, r2, r3
	if (likely(len <= skb_headlen(skb)))
    74ac:	e1510000 	cmp	r1, r0
    74b0:	8a0000e2 	bhi	7840 <sfe_ipv6_recv+0x510>
	icmph = (struct icmp6hdr *)(skb->data + ihl);
    74b4:	e59420b0 	ldr	r2, [r4, #176]	@ 0xb0
	    && (icmph->icmp6_type != ICMPV6_TIME_EXCEED)) {
    74b8:	e7d23005 	ldrb	r3, [r2, r5]
	icmph = (struct icmp6hdr *)(skb->data + ihl);
    74bc:	e0827005 	add	r7, r2, r5
	if ((icmph->icmp6_type != ICMPV6_DEST_UNREACH)
    74c0:	e20330fd 	and	r3, r3, #253	@ 0xfd
    74c4:	e3530001 	cmp	r3, #1
    74c8:	1a000099 	bne	7734 <sfe_ipv6_recv+0x404>
	return skb->len - skb->data_len;
    74cc:	e1c425dc 	ldrd	r2, [r4, #92]	@ 0x5c
	if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ip_hdr) + sizeof(struct sfe_ipv6_ext_hdr))) {
    74d0:	e2851038 	add	r1, r5, #56	@ 0x38
    74d4:	e0420003 	sub	r0, r2, r3
	if (likely(len <= skb_headlen(skb)))
    74d8:	e1510000 	cmp	r1, r0
    74dc:	8a000072 	bhi	76ac <sfe_ipv6_recv+0x37c>
	if (unlikely(icmp_iph->version != 6)) {
    74e0:	e5d73008 	ldrb	r3, [r7, #8]
    74e4:	e20330f0 	and	r3, r3, #240	@ 0xf0
    74e8:	e3530060 	cmp	r3, #96	@ 0x60
    74ec:	1a000083 	bne	7700 <sfe_ipv6_recv+0x3d0>
	next_hdr = icmp_iph->nexthdr;
    74f0:	e5d7300e 	ldrb	r3, [r7, #14]
	ihl += sizeof(struct sfe_ipv6_ip_hdr);
    74f4:	e2855030 	add	r5, r5, #48	@ 0x30
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
    74f8:	e353003c 	cmp	r3, #60	@ 0x3c
    74fc:	8a00001c 	bhi	7574 <sfe_ipv6_recv+0x244>
    7500:	e353002a 	cmp	r3, #42	@ 0x2a
    7504:	9a000097 	bls	7768 <sfe_ipv6_recv+0x438>
    7508:	e243102b 	sub	r1, r3, #43	@ 0x2b
    750c:	e3002103 	movw	r2, #259	@ 0x103
    7510:	e6ef1071 	uxtb	r1, r1
    7514:	e3402002 	movt	r2, #2
    7518:	e1a02132 	lsr	r2, r2, r1
    751c:	e3120001 	tst	r2, #1
    7520:	0a000015 	beq	757c <sfe_ipv6_recv+0x24c>
		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
    7524:	e59480b0 	ldr	r8, [r4, #176]	@ 0xb0
		if (next_hdr == SFE_IPV6_EXT_HDR_FRAG) {
    7528:	e353002c 	cmp	r3, #44	@ 0x2c
		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
    752c:	e0888005 	add	r8, r8, r5
		if (next_hdr == SFE_IPV6_EXT_HDR_FRAG) {
    7530:	1a000004 	bne	7548 <sfe_ipv6_recv+0x218>
			unsigned int frag_off = ntohs(frag_hdr->frag_off);
    7534:	e1d830b2 	ldrh	r3, [r8, #2]
    7538:	e6bf3fb3 	rev16	r3, r3
    753c:	e6ff3073 	uxth	r3, r3
			if (frag_off & SFE_IPV6_FRAG_OFFSET) {
    7540:	e3530007 	cmp	r3, #7
    7544:	8a0000b0 	bhi	780c <sfe_ipv6_recv+0x4dc>
		ext_hdr_len = ext_hdr->hdr_len;
    7548:	e5d83001 	ldrb	r3, [r8, #1]
		ext_hdr_len += sizeof(struct sfe_ipv6_ext_hdr);
    754c:	e2855008 	add	r5, r5, #8
		ihl += ext_hdr_len;
    7550:	e0855183 	add	r5, r5, r3, lsl #3
	return skb->len - skb->data_len;
    7554:	e1c425dc 	ldrd	r2, [r4, #92]	@ 0x5c
		if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {
    7558:	e2851008 	add	r1, r5, #8
    755c:	e0420003 	sub	r0, r2, r3
	if (likely(len <= skb_headlen(skb)))
    7560:	e1510000 	cmp	r1, r0
    7564:	8a0000ca 	bhi	7894 <sfe_ipv6_recv+0x564>
		next_hdr = ext_hdr->next_hdr;
    7568:	e5d83000 	ldrb	r3, [r8]
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
    756c:	e353003c 	cmp	r3, #60	@ 0x3c
    7570:	9affffe2 	bls	7500 <sfe_ipv6_recv+0x1d0>
    7574:	e3530087 	cmp	r3, #135	@ 0x87
    7578:	0a0000a0 	beq	7800 <sfe_ipv6_recv+0x4d0>
	raw_spin_lock_bh(&lock->rlock);
    757c:	e3000000 	movw	r0, #0
    7580:	e3400000 	movt	r0, #0
    7584:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UNHANDLED_PROTOCOL]++;
    7588:	e59f3350 	ldr	r3, [pc, #848]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    758c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7590:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UNHANDLED_PROTOCOL]++;
    7594:	e59320bc 	ldr	r2, [r3, #188]	@ 0xbc
		si->packets_not_forwarded++;
    7598:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UNHANDLED_PROTOCOL]++;
    759c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    75a0:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UNHANDLED_PROTOCOL]++;
    75a4:	e58320bc 	str	r2, [r3, #188]	@ 0xbc
    75a8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    75ac:	eaffffa6 	b	744c <sfe_ipv6_recv+0x11c>
		return sfe_ipv6_recv_udp(si, skb, dev, len, iph, ihl, flush_on_find);
    75b0:	e1a03008 	mov	r3, r8
    75b4:	e1a02007 	mov	r2, r7
    75b8:	e88d4020 	stm	sp, {r5, lr}
    75bc:	e1a01006 	mov	r1, r6
    75c0:	e1a00004 	mov	r0, r4
    75c4:	ebfff440 	bl	46cc <sfe_ipv6_recv_udp.constprop.0>
    75c8:	eaffffa0 	b	7450 <sfe_ipv6_recv+0x120>
	if (unlikely(len > skb->len))
    75cc:	e357002f 	cmp	r7, #47	@ 0x2f
    75d0:	9a000005 	bls	75ec <sfe_ipv6_recv+0x2bc>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    75d4:	e2811030 	add	r1, r1, #48	@ 0x30
    75d8:	e1a00004 	mov	r0, r4
    75dc:	e0411007 	sub	r1, r1, r7
    75e0:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {
    75e4:	e3500000 	cmp	r0, #0
    75e8:	1affff5b 	bne	735c <sfe_ipv6_recv+0x2c>
	raw_spin_lock_bh(&lock->rlock);
    75ec:	e3000000 	movw	r0, #0
    75f0:	e3400000 	movt	r0, #0
    75f4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    75f8:	e59f32e0 	ldr	r3, [pc, #736]	@ 78e0 <sfe_ipv6_recv+0x5b0>
			si->packets_not_forwarded++;
    75fc:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7600:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    7604:	e59320c8 	ldr	r2, [r3, #200]	@ 0xc8
			si->packets_not_forwarded++;
    7608:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    760c:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    7610:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    7614:	e58320c8 	str	r2, [r3, #200]	@ 0xc8
    7618:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    761c:	eaffff8a 	b	744c <sfe_ipv6_recv+0x11c>
	raw_spin_lock_bh(&lock->rlock);
    7620:	e3000000 	movw	r0, #0
    7624:	e3400000 	movt	r0, #0
    7628:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_V6]++;
    762c:	e59f32ac 	ldr	r3, [pc, #684]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    7630:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7634:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_V6]++;
    7638:	e59320d0 	ldr	r2, [r3, #208]	@ 0xd0
		si->packets_not_forwarded++;
    763c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_V6]++;
    7640:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    7644:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_V6]++;
    7648:	e58320d0 	str	r2, [r3, #208]	@ 0xd0
    764c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    7650:	eaffff7d 	b	744c <sfe_ipv6_recv+0x11c>
	raw_spin_lock_bh(&lock->rlock);
    7654:	e3000000 	movw	r0, #0
    7658:	e3400000 	movt	r0, #0
    765c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    7660:	e59f3278 	ldr	r3, [pc, #632]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    7664:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7668:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    766c:	e59320d8 	ldr	r2, [r3, #216]	@ 0xd8
		si->packets_not_forwarded++;
    7670:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    7674:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    7678:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    767c:	e58320d8 	str	r2, [r3, #216]	@ 0xd8
    7680:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    7684:	eaffff70 	b	744c <sfe_ipv6_recv+0x11c>
	if (unlikely(len > skb->len))
    7688:	e1510002 	cmp	r1, r2
    768c:	8affffd6 	bhi	75ec <sfe_ipv6_recv+0x2bc>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    7690:	e0433002 	sub	r3, r3, r2
    7694:	e1a00004 	mov	r0, r4
    7698:	e0831001 	add	r1, r3, r1
    769c:	ebfffffe 	bl	0 <__pskb_pull_tail>
		if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {
    76a0:	e3500000 	cmp	r0, #0
    76a4:	1affff56 	bne	7404 <sfe_ipv6_recv+0xd4>
    76a8:	eaffffcf 	b	75ec <sfe_ipv6_recv+0x2bc>
	if (unlikely(len > skb->len))
    76ac:	e1510002 	cmp	r1, r2
    76b0:	8a000005 	bhi	76cc <sfe_ipv6_recv+0x39c>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    76b4:	e0433002 	sub	r3, r3, r2
    76b8:	e1a00004 	mov	r0, r4
    76bc:	e0831001 	add	r1, r3, r1
    76c0:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ip_hdr) + sizeof(struct sfe_ipv6_ext_hdr))) {
    76c4:	e3500000 	cmp	r0, #0
    76c8:	1affff84 	bne	74e0 <sfe_ipv6_recv+0x1b0>
	raw_spin_lock_bh(&lock->rlock);
    76cc:	e3000000 	movw	r0, #0
    76d0:	e3400000 	movt	r0, #0
    76d4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_HEADER_INCOMPLETE]++;
    76d8:	e59f3200 	ldr	r3, [pc, #512]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    76dc:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    76e0:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_HEADER_INCOMPLETE]++;
    76e4:	e59320a8 	ldr	r2, [r3, #168]	@ 0xa8
		si->packets_not_forwarded++;
    76e8:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_HEADER_INCOMPLETE]++;
    76ec:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    76f0:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_HEADER_INCOMPLETE]++;
    76f4:	e58320a8 	str	r2, [r3, #168]	@ 0xa8
    76f8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    76fc:	eaffff52 	b	744c <sfe_ipv6_recv+0x11c>
	raw_spin_lock_bh(&lock->rlock);
    7700:	e3000000 	movw	r0, #0
    7704:	e3400000 	movt	r0, #0
    7708:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_NON_V6]++;
    770c:	e59f31cc 	ldr	r3, [pc, #460]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    7710:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7714:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_NON_V6]++;
    7718:	e59320ac 	ldr	r2, [r3, #172]	@ 0xac
		si->packets_not_forwarded++;
    771c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_NON_V6]++;
    7720:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    7724:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_NON_V6]++;
    7728:	e58320ac 	str	r2, [r3, #172]	@ 0xac
    772c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    7730:	eaffff45 	b	744c <sfe_ipv6_recv+0x11c>
	raw_spin_lock_bh(&lock->rlock);
    7734:	e3000000 	movw	r0, #0
    7738:	e3400000 	movt	r0, #0
    773c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    7740:	e59f3198 	ldr	r3, [pc, #408]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    7744:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7748:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    774c:	e59320a4 	ldr	r2, [r3, #164]	@ 0xa4
		si->packets_not_forwarded++;
    7750:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    7754:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    7758:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    775c:	e58320a4 	str	r2, [r3, #164]	@ 0xa4
    7760:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    7764:	eaffff38 	b	744c <sfe_ipv6_recv+0x11c>
    7768:	e3530000 	cmp	r3, #0
    776c:	0a000023 	beq	7800 <sfe_ipv6_recv+0x4d0>
	switch (next_hdr) {
    7770:	e3530006 	cmp	r3, #6
    7774:	0a000001 	beq	7780 <sfe_ipv6_recv+0x450>
    7778:	e3530011 	cmp	r3, #17
    777c:	1affff7e 	bne	757c <sfe_ipv6_recv+0x24c>
		icmp_tcph = (struct sfe_ipv6_tcp_hdr *)(skb->data + ihl);
    7780:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
	raw_spin_lock_bh(&lock->rlock);
    7784:	e3000000 	movw	r0, #0
    7788:	e3400000 	movt	r0, #0
    778c:	e0832005 	add	r2, r3, r5
		src_port = icmp_tcph->source;
    7790:	e19340b5 	ldrh	r4, [r3, r5]
		dest_port = icmp_tcph->dest;
    7794:	e1d250b2 	ldrh	r5, [r2, #2]
    7798:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	src_ip = &icmp_iph->saddr;
    779c:	e287c010 	add	ip, r7, #16
	cm = sfe_ipv6_find_connection_match(si, dev, icmp_iph->nexthdr, dest_ip, dest_port, src_ip, src_port);
    77a0:	e5d7100e 	ldrb	r1, [r7, #14]
    77a4:	e2872020 	add	r2, r7, #32
    77a8:	e1a00006 	mov	r0, r6
    77ac:	e1a03005 	mov	r3, r5
    77b0:	e58dc000 	str	ip, [sp]
    77b4:	e58d4004 	str	r4, [sp, #4]
    77b8:	ebffe5e3 	bl	f4c <sfe_ipv6_find_connection_match.constprop.0>
	if (unlikely(!cm)) {
    77bc:	e3500000 	cmp	r0, #0
    77c0:	0a00003c 	beq	78b8 <sfe_ipv6_recv+0x588>
	c = cm->connection;
    77c4:	e5904008 	ldr	r4, [r0, #8]
	sfe_ipv6_remove_connection(si, c);
    77c8:	e1a00004 	mov	r0, r4
    77cc:	ebffeb24 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    77d0:	e59f3108 	ldr	r3, [pc, #264]	@ 78e0 <sfe_ipv6_recv+0x5b0>
	si->packets_not_forwarded++;
    77d4:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    77d8:	e2430902 	sub	r0, r3, #32768	@ 0x8000
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    77dc:	e59320c4 	ldr	r2, [r3, #196]	@ 0xc4
	si->packets_not_forwarded++;
    77e0:	e2811001 	add	r1, r1, #1
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    77e4:	e2822001 	add	r2, r2, #1
	si->packets_not_forwarded++;
    77e8:	e5831050 	str	r1, [r3, #80]	@ 0x50
	si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    77ec:	e58320c4 	str	r2, [r3, #196]	@ 0xc4
    77f0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
    77f4:	e1a00004 	mov	r0, r4
    77f8:	ebffe7c9 	bl	1724 <sfe_ipv6_flush_connection.constprop.1>
	return 0;
    77fc:	eaffff12 	b	744c <sfe_ipv6_recv+0x11c>
		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
    7800:	e59480b0 	ldr	r8, [r4, #176]	@ 0xb0
    7804:	e0888005 	add	r8, r8, r5
		if (next_hdr == SFE_IPV6_EXT_HDR_FRAG) {
    7808:	eaffff4e 	b	7548 <sfe_ipv6_recv+0x218>
	raw_spin_lock_bh(&lock->rlock);
    780c:	e3000000 	movw	r0, #0
    7810:	e3400000 	movt	r0, #0
    7814:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
				si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    7818:	e59f30c0 	ldr	r3, [pc, #192]	@ 78e0 <sfe_ipv6_recv+0x5b0>
				si->packets_not_forwarded++;
    781c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7820:	e2430902 	sub	r0, r3, #32768	@ 0x8000
				si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    7824:	e59320d4 	ldr	r2, [r3, #212]	@ 0xd4
				si->packets_not_forwarded++;
    7828:	e2811001 	add	r1, r1, #1
				si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    782c:	e2822001 	add	r2, r2, #1
				si->packets_not_forwarded++;
    7830:	e5831050 	str	r1, [r3, #80]	@ 0x50
				si->exception_events[SFE_IPV6_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    7834:	e58320d4 	str	r2, [r3, #212]	@ 0xd4
    7838:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
				return 0;
    783c:	eaffff02 	b	744c <sfe_ipv6_recv+0x11c>
	if (unlikely(len > skb->len))
    7840:	e1510002 	cmp	r1, r2
    7844:	8a000005 	bhi	7860 <sfe_ipv6_recv+0x530>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    7848:	e0433002 	sub	r3, r3, r2
    784c:	e1a00004 	mov	r0, r4
    7850:	e0831001 	add	r1, r3, r1
    7854:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, ihl + sizeof(struct icmp6hdr))) {
    7858:	e3500000 	cmp	r0, #0
    785c:	1affff14 	bne	74b4 <sfe_ipv6_recv+0x184>
	raw_spin_lock_bh(&lock->rlock);
    7860:	e3000000 	movw	r0, #0
    7864:	e3400000 	movt	r0, #0
    7868:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    786c:	e59f306c 	ldr	r3, [pc, #108]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    7870:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    7874:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    7878:	e59320a0 	ldr	r2, [r3, #160]	@ 0xa0
		si->packets_not_forwarded++;
    787c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    7880:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    7884:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    7888:	e58320a0 	str	r2, [r3, #160]	@ 0xa0
    788c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    7890:	eafffeed 	b	744c <sfe_ipv6_recv+0x11c>
	if (unlikely(len > skb->len))
    7894:	e1510002 	cmp	r1, r2
    7898:	8affff53 	bhi	75ec <sfe_ipv6_recv+0x2bc>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    789c:	e0433002 	sub	r3, r3, r2
    78a0:	e1a00004 	mov	r0, r4
    78a4:	e0831001 	add	r1, r3, r1
    78a8:	ebfffffe 	bl	0 <__pskb_pull_tail>
		if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {
    78ac:	e3500000 	cmp	r0, #0
    78b0:	1affff2c 	bne	7568 <sfe_ipv6_recv+0x238>
    78b4:	eaffff4c 	b	75ec <sfe_ipv6_recv+0x2bc>
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    78b8:	e59f3020 	ldr	r3, [pc, #32]	@ 78e0 <sfe_ipv6_recv+0x5b0>
		si->packets_not_forwarded++;
    78bc:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    78c0:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    78c4:	e59320c0 	ldr	r2, [r3, #192]	@ 0xc0
		si->packets_not_forwarded++;
    78c8:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    78cc:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    78d0:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV6_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    78d4:	e58320c0 	str	r2, [r3, #192]	@ 0xc0
    78d8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    78dc:	eafffeda 	b	744c <sfe_ipv6_recv+0x11c>
    78e0:	00010050 	.word	0x00010050

000078e4 <sfe_ipv6_create_rule>:
{
    78e4:	e1a0c00d 	mov	ip, sp
    78e8:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    78ec:	e24cb004 	sub	fp, ip, #4
    78f0:	e24dd044 	sub	sp, sp, #68	@ 0x44
	src_dev = sic->src_dev;
    78f4:	e590a004 	ldr	sl, [r0, #4]
{
    78f8:	e1a04000 	mov	r4, r0
	dest_dev = sic->dest_dev;
    78fc:	e5909008 	ldr	r9, [r0, #8]
	if (unlikely((dest_dev->reg_state != NETREG_REGISTERED) ||
    7900:	e5d9331c 	ldrb	r3, [r9, #796]	@ 0x31c
    7904:	e3530001 	cmp	r3, #1
    7908:	1a000244 	bne	8220 <sfe_ipv6_create_rule+0x93c>
    790c:	e5da331c 	ldrb	r3, [sl, #796]	@ 0x31c
    7910:	e3530001 	cmp	r3, #1
    7914:	1a000241 	bne	8220 <sfe_ipv6_create_rule+0x93c>
	raw_spin_lock_bh(&lock->rlock);
    7918:	e3008000 	movw	r8, #0
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    791c:	e284501c 	add	r5, r4, #28
    7920:	e3408000 	movt	r8, #0
    7924:	e1a00008 	mov	r0, r8
    7928:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->connection_create_requests++;
    792c:	e2882902 	add	r2, r8, #32768	@ 0x8000
    7930:	e5923030 	ldr	r3, [r2, #48]	@ 0x30
    7934:	e2833001 	add	r3, r3, #1
    7938:	e5823030 	str	r3, [r2, #48]	@ 0x30
				     sic->protocol,
    793c:	e5943000 	ldr	r3, [r4]
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7940:	e5942038 	ldr	r2, [r4, #56]	@ 0x38
    7944:	e89500e0 	ldm	r5, {r5, r6, r7}
    7948:	e594103c 	ldr	r1, [r4, #60]	@ 0x3c
				     sic->protocol,
    794c:	e50b3044 	str	r3, [fp, #-68]	@ 0xffffffbc
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7950:	e50b2038 	str	r2, [fp, #-56]	@ 0xffffffc8
    7954:	e594c018 	ldr	ip, [r4, #24]
    7958:	e5942044 	ldr	r2, [r4, #68]	@ 0x44
    795c:	e5940040 	ldr	r0, [r4, #64]	@ 0x40
    7960:	e50b1034 	str	r1, [fp, #-52]	@ 0xffffffcc
	c = sfe_ipv6_find_connection(si,
    7964:	e1d415b8 	ldrh	r1, [r4, #88]	@ 0x58
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7968:	e1a0e002 	mov	lr, r2
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    796c:	e6ef2073 	uxtb	r2, r3
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7970:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
    7974:	e50b0030 	str	r0, [fp, #-48]	@ 0xffffffd0
	c = sfe_ipv6_find_connection(si,
    7978:	e1d405bc 	ldrh	r0, [r4, #92]	@ 0x5c
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    797c:	e50be04c 	str	lr, [fp, #-76]	@ 0xffffffb4
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    7980:	e50b203c 	str	r2, [fp, #-60]	@ 0xffffffc4
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7984:	e51b2034 	ldr	r2, [fp, #-52]	@ 0xffffffcc
    7988:	e023300c 	eor	r3, r3, ip
    798c:	e0233005 	eor	r3, r3, r5
    7990:	e0233002 	eor	r3, r3, r2
    7994:	e51b2030 	ldr	r2, [fp, #-48]	@ 0xffffffd0
    7998:	e0233006 	eor	r3, r3, r6
    799c:	e0233002 	eor	r3, r3, r2
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    79a0:	e51b203c 	ldr	r2, [fp, #-60]	@ 0xffffffc4
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    79a4:	e0233007 	eor	r3, r3, r7
    79a8:	e023300e 	eor	r3, r3, lr
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    79ac:	e0233002 	eor	r3, r3, r2
    79b0:	e0212000 	eor	r2, r1, r0
    79b4:	e6bf2fb2 	rev16	r2, r2
    79b8:	e6ff2072 	uxth	r2, r2
    79bc:	e0222003 	eor	r2, r2, r3
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    79c0:	e0222622 	eor	r2, r2, r2, lsr #12
    79c4:	e7eb3052 	ubfx	r3, r2, #0, #12
	c = si->conn_hash[conn_idx];
    79c8:	e283300c 	add	r3, r3, #12
    79cc:	e7983103 	ldr	r3, [r8, r3, lsl #2]
	if (unlikely(!c)) {
    79d0:	e3530000 	cmp	r3, #0
    79d4:	1a000003 	bne	79e8 <sfe_ipv6_create_rule+0x104>
    79d8:	ea000034 	b	7ab0 <sfe_ipv6_create_rule+0x1cc>
		c = c->next;
    79dc:	e5933000 	ldr	r3, [r3]
	} while (c && (c->src_port != src_port
    79e0:	e3530000 	cmp	r3, #0
    79e4:	0a000031 	beq	7ab0 <sfe_ipv6_create_rule+0x1cc>
	if ((c->src_port == src_port)
    79e8:	e1d324bc 	ldrh	r2, [r3, #76]	@ 0x4c
    79ec:	e1520001 	cmp	r2, r1
    79f0:	1afffff9 	bne	79dc <sfe_ipv6_create_rule+0xf8>
		 || c->dest_port != dest_port
    79f4:	e1d325b0 	ldrh	r2, [r3, #80]	@ 0x50
    79f8:	e1520000 	cmp	r2, r0
    79fc:	1afffff6 	bne	79dc <sfe_ipv6_create_rule+0xf8>
    7a00:	e593200c 	ldr	r2, [r3, #12]
    7a04:	e15c0002 	cmp	ip, r2
    7a08:	1afffff3 	bne	79dc <sfe_ipv6_create_rule+0xf8>
	return a->addr[0] == b->addr[0] &&
    7a0c:	e5932010 	ldr	r2, [r3, #16]
    7a10:	e1550002 	cmp	r5, r2
    7a14:	1afffff0 	bne	79dc <sfe_ipv6_create_rule+0xf8>
	       a->addr[1] == b->addr[1] &&
    7a18:	e5932014 	ldr	r2, [r3, #20]
    7a1c:	e1560002 	cmp	r6, r2
    7a20:	1affffed 	bne	79dc <sfe_ipv6_create_rule+0xf8>
	       a->addr[2] == b->addr[2] &&
    7a24:	e5932018 	ldr	r2, [r3, #24]
    7a28:	e1570002 	cmp	r7, r2
    7a2c:	1affffea 	bne	79dc <sfe_ipv6_create_rule+0xf8>
    7a30:	e51be038 	ldr	lr, [fp, #-56]	@ 0xffffffc8
    7a34:	e593202c 	ldr	r2, [r3, #44]	@ 0x2c
    7a38:	e15e0002 	cmp	lr, r2
    7a3c:	1affffe6 	bne	79dc <sfe_ipv6_create_rule+0xf8>
	return a->addr[0] == b->addr[0] &&
    7a40:	e51be034 	ldr	lr, [fp, #-52]	@ 0xffffffcc
    7a44:	e5932030 	ldr	r2, [r3, #48]	@ 0x30
    7a48:	e15e0002 	cmp	lr, r2
    7a4c:	1affffe2 	bne	79dc <sfe_ipv6_create_rule+0xf8>
	       a->addr[1] == b->addr[1] &&
    7a50:	e51be030 	ldr	lr, [fp, #-48]	@ 0xffffffd0
    7a54:	e5932034 	ldr	r2, [r3, #52]	@ 0x34
    7a58:	e15e0002 	cmp	lr, r2
    7a5c:	1affffde 	bne	79dc <sfe_ipv6_create_rule+0xf8>
	       a->addr[2] == b->addr[2] &&
    7a60:	e51be04c 	ldr	lr, [fp, #-76]	@ 0xffffffb4
    7a64:	e5932038 	ldr	r2, [r3, #56]	@ 0x38
    7a68:	e15e0002 	cmp	lr, r2
    7a6c:	1affffda 	bne	79dc <sfe_ipv6_create_rule+0xf8>
		 || c->protocol != protocol));
    7a70:	e51be044 	ldr	lr, [fp, #-68]	@ 0xffffffbc
    7a74:	e5932008 	ldr	r2, [r3, #8]
    7a78:	e15e0002 	cmp	lr, r2
    7a7c:	1affffd6 	bne	79dc <sfe_ipv6_create_rule+0xf8>
		si->connection_create_collisions++;
    7a80:	e59f1804 	ldr	r1, [pc, #2052]	@ 828c <sfe_ipv6_create_rule+0x9a8>
    7a84:	e5912034 	ldr	r2, [r1, #52]	@ 0x34
    7a88:	e2822001 	add	r2, r2, #1
    7a8c:	e5812034 	str	r2, [r1, #52]	@ 0x34
	switch (sic->protocol) {
    7a90:	e5942000 	ldr	r2, [r4]
    7a94:	e3520006 	cmp	r2, #6
    7a98:	0a0001f6 	beq	8278 <sfe_ipv6_create_rule+0x994>
	raw_spin_unlock_bh(&lock->rlock);
    7a9c:	e3000000 	movw	r0, #0
    7aa0:	e3400000 	movt	r0, #0
    7aa4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return -EADDRINUSE;
    7aa8:	e3e00061 	mvn	r0, #97	@ 0x61
    7aac:	ea000193 	b	8100 <sfe_ipv6_create_rule+0x81c>
    7ab0:	e3007000 	movw	r7, #0
    7ab4:	e3a02080 	mov	r2, #128	@ 0x80
    7ab8:	e3407000 	movt	r7, #0
    7abc:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    7ac0:	e597001c 	ldr	r0, [r7, #28]
    7ac4:	ebfffffe 	bl	0 <kmalloc_trace>
	if (unlikely(!c)) {
    7ac8:	e2505000 	subs	r5, r0, #0
    7acc:	0a0001d6 	beq	822c <sfe_ipv6_create_rule+0x948>
    7ad0:	e5970008 	ldr	r0, [r7, #8]
    7ad4:	e3a020c0 	mov	r2, #192	@ 0xc0
    7ad8:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    7adc:	ebfffffe 	bl	0 <kmalloc_trace>
	if (unlikely(!original_cm)) {
    7ae0:	e2506000 	subs	r6, r0, #0
    7ae4:	0a0001d5 	beq	8240 <sfe_ipv6_create_rule+0x95c>
    7ae8:	e5970008 	ldr	r0, [r7, #8]
    7aec:	e3a020c0 	mov	r2, #192	@ 0xc0
    7af0:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    7af4:	ebfffffe 	bl	0 <kmalloc_trace>
	if (unlikely(!reply_cm)) {
    7af8:	e2507000 	subs	r7, r0, #0
    7afc:	0a0001d5 	beq	8258 <sfe_ipv6_create_rule+0x974>
	original_cm->match_src_port = sic->src_port;
    7b00:	e1d425b8 	ldrh	r2, [r4, #88]	@ 0x58
	original_cm->match_protocol = sic->protocol;
    7b04:	e1a0c004 	mov	ip, r4
	original_cm->match_dev = src_dev;
    7b08:	e586a01c 	str	sl, [r6, #28]
	original_cm->match_protocol = sic->protocol;
    7b0c:	e49c3018 	ldr	r3, [ip], #24
	original_cm->xmit_dev_mtu = sic->dest_mtu;
    7b10:	e594e014 	ldr	lr, [r4, #20]
	original_cm->match_src_port = sic->src_port;
    7b14:	e1c624b4 	strh	r2, [r6, #68]	@ 0x44
    7b18:	e50b2030 	str	r2, [fp, #-48]	@ 0xffffffd0
	original_cm->match_dest_port = sic->dest_port;
    7b1c:	e1d425bc 	ldrh	r2, [r4, #92]	@ 0x5c
	original_cm->match_protocol = sic->protocol;
    7b20:	e6ef0073 	uxtb	r0, r3
    7b24:	e50b3060 	str	r3, [fp, #-96]	@ 0xffffffa0
    7b28:	e5c60020 	strb	r0, [r6, #32]
	original_cm->match_dest_port = sic->dest_port;
    7b2c:	e50b2034 	str	r2, [fp, #-52]	@ 0xffffffcc
    7b30:	e1c624b6 	strh	r2, [r6, #70]	@ 0x46
	original_cm->xlate_src_port = sic->src_port_xlate;
    7b34:	e1d425ba 	ldrh	r2, [r4, #90]	@ 0x5a
    7b38:	e50b2038 	str	r2, [fp, #-56]	@ 0xffffffc8
    7b3c:	e1c627b8 	strh	r2, [r6, #120]	@ 0x78
	original_cm->xlate_dest_port = sic->dest_port_xlate;
    7b40:	e1d425be 	ldrh	r2, [r4, #94]	@ 0x5e
	original_cm->xmit_dev = dest_dev;
    7b44:	e5869098 	str	r9, [r6, #152]	@ 0x98
	original_cm->xlate_dest_port = sic->dest_port_xlate;
    7b48:	e50b203c 	str	r2, [fp, #-60]	@ 0xffffffc4
    7b4c:	e1c628bc 	strh	r2, [r6, #140]	@ 0x8c
	original_cm->match_src_ip[0] = sic->src_ip.ip6[0];
    7b50:	e1c421d8 	ldrd	r2, [r4, #24]
    7b54:	e14b24f4 	strd	r2, [fp, #-68]	@ 0xffffffbc
    7b58:	e1c622f4 	strd	r2, [r6, #36]	@ 0x24
    7b5c:	e1cc20d8 	ldrd	r2, [ip, #8]
    7b60:	e1c622fc 	strd	r2, [r6, #44]	@ 0x2c
	original_cm->match_dest_ip[0] = sic->dest_ip.ip6[0];
    7b64:	e1c423d8 	ldrd	r2, [r4, #56]	@ 0x38
    7b68:	e14b25f4 	strd	r2, [fp, #-84]	@ 0xffffffac
    7b6c:	e1c623f4 	strd	r2, [r6, #52]	@ 0x34
    7b70:	e1c424d0 	ldrd	r2, [r4, #64]	@ 0x40
    7b74:	e1c623fc 	strd	r2, [r6, #60]	@ 0x3c
	original_cm->xlate_src_ip[0] = sic->src_ip_xlate.ip6[0];
    7b78:	e1c422d8 	ldrd	r2, [r4, #40]	@ 0x28
    7b7c:	e14b24fc 	strd	r2, [fp, #-76]	@ 0xffffffb4
    7b80:	e1c626f8 	strd	r2, [r6, #104]	@ 0x68
    7b84:	e1c423d0 	ldrd	r2, [r4, #48]	@ 0x30
    7b88:	e1c627f0 	strd	r2, [r6, #112]	@ 0x70
	original_cm->xlate_dest_ip[0] = sic->dest_ip_xlate.ip6[0];
    7b8c:	e1c424d8 	ldrd	r2, [r4, #72]	@ 0x48
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    7b90:	e1d417b6 	ldrh	r1, [r4, #118]	@ 0x76
	original_cm->xlate_dest_ip[0] = sic->dest_ip_xlate.ip6[0];
    7b94:	e1c627fc 	strd	r2, [r6, #124]	@ 0x7c
    7b98:	e14b25fc 	strd	r2, [fp, #-92]	@ 0xffffffa4
    7b9c:	e1c425d0 	ldrd	r2, [r4, #80]	@ 0x50
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    7ba0:	e50b1064 	str	r1, [fp, #-100]	@ 0xffffff9c
	original_cm->xlate_dest_ip[0] = sic->dest_ip_xlate.ip6[0];
    7ba4:	e1c628f4 	strd	r2, [r6, #132]	@ 0x84
	original_cm->rx_packet_count64 = 0;
    7ba8:	e3a02000 	mov	r2, #0
    7bac:	e3a03000 	mov	r3, #0
	original_cm->xmit_dev_mtu = sic->dest_mtu;
    7bb0:	e1c6e9bc 	strh	lr, [r6, #156]	@ 0x9c
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    7bb4:	e594e072 	ldr	lr, [r4, #114]	@ 0x72
	original_cm->rx_packet_count = 0;
    7bb8:	e1c626f0 	strd	r2, [r6, #96]	@ 0x60
	original_cm->rx_packet_count64 = 0;
    7bbc:	e1c62bf0 	strd	r2, [r6, #176]	@ 0xb0
	original_cm->rx_byte_count64 = 0;
    7bc0:	e1c62bf8 	strd	r2, [r6, #184]	@ 0xb8
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
    7bc4:	e5992210 	ldr	r2, [r9, #528]	@ 0x210
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    7bc8:	e594300c 	ldr	r3, [r4, #12]
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
    7bcc:	e5921000 	ldr	r1, [r2]
    7bd0:	e58610a4 	str	r1, [r6, #164]	@ 0xa4
	original_cm->flags = 0;
    7bd4:	e3a01000 	mov	r1, #0
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
    7bd8:	e1d220b4 	ldrh	r2, [r2, #4]
	original_cm->connection = c;
    7bdc:	e5865008 	str	r5, [r6, #8]
	original_cm->counter_match = reply_cm;
    7be0:	e586700c 	str	r7, [r6, #12]
	original_cm->flags = 0;
    7be4:	e5861048 	str	r1, [r6, #72]	@ 0x48
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
    7be8:	e51b1064 	ldr	r1, [fp, #-100]	@ 0xffffff9c
    7bec:	e586e09e 	str	lr, [r6, #158]	@ 0x9e
    7bf0:	e1c61ab2 	strh	r1, [r6, #162]	@ 0xa2
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    7bf4:	e2131002 	ands	r1, r3, #2
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
    7bf8:	e1c62ab8 	strh	r2, [r6, #168]	@ 0xa8
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    7bfc:	1a000141 	bne	8108 <sfe_ipv6_create_rule+0x824>
    7c00:	e3a02040 	mov	r2, #64	@ 0x40
    7c04:	e1a0e001 	mov	lr, r1
    7c08:	e50b2068 	str	r2, [fp, #-104]	@ 0xffffff98
	if (sic->flags & SFE_CREATE_FLAG_REMARK_DSCP) {
    7c0c:	e2133004 	ands	r3, r3, #4
    7c10:	e50b3064 	str	r3, [fp, #-100]	@ 0xffffff9c
    7c14:	0a000004 	beq	7c2c <sfe_ipv6_create_rule+0x348>
		original_cm->dscp = sic->src_dscp << SFE_IPV6_DSCP_SHIFT;
    7c18:	e59430ac 	ldr	r3, [r4, #172]	@ 0xac
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK;
    7c1c:	e51be068 	ldr	lr, [fp, #-104]	@ 0xffffff98
		original_cm->dscp = sic->src_dscp << SFE_IPV6_DSCP_SHIFT;
    7c20:	e1a03103 	lsl	r3, r3, #2
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK;
    7c24:	e586e048 	str	lr, [r6, #72]	@ 0x48
		original_cm->dscp = sic->src_dscp << SFE_IPV6_DSCP_SHIFT;
    7c28:	e5863094 	str	r3, [r6, #148]	@ 0x94
	original_cm->flow_accel = sic->original_accel;
    7c2c:	e594309c 	ldr	r3, [r4, #156]	@ 0x9c
	original_cm->active_next = NULL;
    7c30:	e3a02000 	mov	r2, #0
	original_cm->flow_accel = sic->original_accel;
    7c34:	e586304c 	str	r3, [r6, #76]	@ 0x4c
	original_cm->active = false;
    7c38:	e3a03000 	mov	r3, #0
    7c3c:	e5c63018 	strb	r3, [r6, #24]
	original_cm->active_next = NULL;
    7c40:	e3a03000 	mov	r3, #0
    7c44:	e1c621f0 	strd	r2, [r6, #16]
	if (!(dest_dev->flags & IFF_POINTOPOINT)) {
    7c48:	e5993068 	ldr	r3, [r9, #104]	@ 0x68
    7c4c:	e3130010 	tst	r3, #16
    7c50:	1a00000a 	bne	7c80 <sfe_ipv6_create_rule+0x39c>
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;
    7c54:	e38e3010 	orr	r3, lr, #16
    7c58:	e5863048 	str	r3, [r6, #72]	@ 0x48
		if (dest_dev->header_ops) {
    7c5c:	e5993158 	ldr	r3, [r9, #344]	@ 0x158
    7c60:	e3530000 	cmp	r3, #0
    7c64:	0a000005 	beq	7c80 <sfe_ipv6_create_rule+0x39c>
			if (dest_dev->header_ops->create == eth_header) {
    7c68:	e5932000 	ldr	r2, [r3]
    7c6c:	e3003000 	movw	r3, #0
    7c70:	e3403000 	movt	r3, #0
    7c74:	e1520003 	cmp	r2, r3
				original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
    7c78:	038e3018 	orreq	r3, lr, #24
    7c7c:	05863048 	streq	r3, [r6, #72]	@ 0x48
	reply_cm->match_src_ip[0] = sic->dest_ip_xlate.ip6[0];
    7c80:	e14b25dc 	ldrd	r2, [fp, #-92]	@ 0xffffffa4
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    7c84:	e3510000 	cmp	r1, #0
	reply_cm->match_dev = dest_dev;
    7c88:	e587901c 	str	r9, [r7, #28]
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
    7c8c:	e5941060 	ldr	r1, [r4, #96]	@ 0x60
	reply_cm->match_protocol = sic->protocol;
    7c90:	e5c70020 	strb	r0, [r7, #32]
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
    7c94:	e1d406b4 	ldrh	r0, [r4, #100]	@ 0x64
	reply_cm->match_src_ip[0] = sic->dest_ip_xlate.ip6[0];
    7c98:	e1c722f4 	strd	r2, [r7, #36]	@ 0x24
	reply_cm->match_dest_ip[0] = sic->src_ip_xlate.ip6[0];
    7c9c:	e14b24dc 	ldrd	r2, [fp, #-76]	@ 0xffffffb4
    7ca0:	e1c723f4 	strd	r2, [r7, #52]	@ 0x34
	reply_cm->match_src_port = sic->dest_port_xlate;
    7ca4:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
	reply_cm->xmit_dev = src_dev;
    7ca8:	e587a098 	str	sl, [r7, #152]	@ 0x98
	reply_cm->match_src_port = sic->dest_port_xlate;
    7cac:	e1c734b4 	strh	r3, [r7, #68]	@ 0x44
	reply_cm->match_dest_port = sic->src_port_xlate;
    7cb0:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
    7cb4:	e1c734b6 	strh	r3, [r7, #70]	@ 0x46
	reply_cm->xlate_src_ip[0] = sic->dest_ip.ip6[0];
    7cb8:	e14b25d4 	ldrd	r2, [fp, #-84]	@ 0xffffffac
    7cbc:	e1c726f8 	strd	r2, [r7, #104]	@ 0x68
	reply_cm->xlate_src_port = sic->dest_port;
    7cc0:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    7cc4:	e1c737b8 	strh	r3, [r7, #120]	@ 0x78
	reply_cm->xlate_dest_ip[0] = sic->src_ip.ip6[0];
    7cc8:	e14b24d4 	ldrd	r2, [fp, #-68]	@ 0xffffffbc
    7ccc:	e1c727fc 	strd	r2, [r7, #124]	@ 0x7c
	reply_cm->xlate_dest_port = sic->src_port;
    7cd0:	e51b3030 	ldr	r3, [fp, #-48]	@ 0xffffffd0
    7cd4:	e1c738bc 	strh	r3, [r7, #140]	@ 0x8c
	reply_cm->xmit_dev_mtu = sic->src_mtu;
    7cd8:	e5943010 	ldr	r3, [r4, #16]
    7cdc:	e1c739bc 	strh	r3, [r7, #156]	@ 0x9c
	reply_cm->match_src_ip[0] = sic->dest_ip_xlate.ip6[0];
    7ce0:	e1c425d0 	ldrd	r2, [r4, #80]	@ 0x50
    7ce4:	e1c722fc 	strd	r2, [r7, #44]	@ 0x2c
	reply_cm->match_dest_ip[0] = sic->src_ip_xlate.ip6[0];
    7ce8:	e1c423d0 	ldrd	r2, [r4, #48]	@ 0x30
    7cec:	e1c723fc 	strd	r2, [r7, #60]	@ 0x3c
	reply_cm->xlate_src_ip[0] = sic->dest_ip.ip6[0];
    7cf0:	e1c424d0 	ldrd	r2, [r4, #64]	@ 0x40
    7cf4:	e1c727f0 	strd	r2, [r7, #112]	@ 0x70
	reply_cm->xlate_dest_ip[0] = sic->src_ip.ip6[0];
    7cf8:	e1cc20d8 	ldrd	r2, [ip, #8]
    7cfc:	e1c728f4 	strd	r2, [r7, #132]	@ 0x84
	reply_cm->rx_packet_count64 = 0;
    7d00:	e3a02000 	mov	r2, #0
    7d04:	e3a03000 	mov	r3, #0
	reply_cm->rx_packet_count = 0;
    7d08:	e1c726f0 	strd	r2, [r7, #96]	@ 0x60
	reply_cm->rx_packet_count64 = 0;
    7d0c:	e1c72bf0 	strd	r2, [r7, #176]	@ 0xb0
	reply_cm->rx_byte_count64 = 0;
    7d10:	e1c72bf8 	strd	r2, [r7, #184]	@ 0xb8
	memcpy(reply_cm->xmit_src_mac, src_dev->dev_addr, ETH_ALEN);
    7d14:	e59a3210 	ldr	r3, [sl, #528]	@ 0x210
    7d18:	e5932000 	ldr	r2, [r3]
    7d1c:	e58720a4 	str	r2, [r7, #164]	@ 0xa4
    7d20:	e1d330b4 	ldrh	r3, [r3, #4]
	reply_cm->connection = c;
    7d24:	e5875008 	str	r5, [r7, #8]
	reply_cm->counter_match = original_cm;
    7d28:	e587600c 	str	r6, [r7, #12]
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
    7d2c:	e587109e 	str	r1, [r7, #158]	@ 0x9e
    7d30:	e1c70ab2 	strh	r0, [r7, #162]	@ 0xa2
	memcpy(reply_cm->xmit_src_mac, src_dev->dev_addr, ETH_ALEN);
    7d34:	e1c73ab8 	strh	r3, [r7, #168]	@ 0xa8
	reply_cm->flags = 0;
    7d38:	e3a03000 	mov	r3, #0
    7d3c:	e5873048 	str	r3, [r7, #72]	@ 0x48
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
    7d40:	0a000003 	beq	7d54 <sfe_ipv6_create_rule+0x470>
		reply_cm->priority = sic->dest_priority;
    7d44:	e59430a8 	ldr	r3, [r4, #168]	@ 0xa8
    7d48:	e5873090 	str	r3, [r7, #144]	@ 0x90
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
    7d4c:	e3a03020 	mov	r3, #32
    7d50:	e5873048 	str	r3, [r7, #72]	@ 0x48
	if (sic->flags & SFE_CREATE_FLAG_REMARK_DSCP) {
    7d54:	e51b3064 	ldr	r3, [fp, #-100]	@ 0xffffff9c
    7d58:	e3530000 	cmp	r3, #0
    7d5c:	0a000004 	beq	7d74 <sfe_ipv6_create_rule+0x490>
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK;
    7d60:	e51b3068 	ldr	r3, [fp, #-104]	@ 0xffffff98
    7d64:	e5873048 	str	r3, [r7, #72]	@ 0x48
		reply_cm->dscp = sic->dest_dscp << SFE_IPV6_DSCP_SHIFT;
    7d68:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
    7d6c:	e1a03103 	lsl	r3, r3, #2
    7d70:	e5873094 	str	r3, [r7, #148]	@ 0x94
	reply_cm->flow_accel = sic->reply_accel;
    7d74:	e59430a0 	ldr	r3, [r4, #160]	@ 0xa0
	reply_cm->active_next = NULL;
    7d78:	e3a02000 	mov	r2, #0
	reply_cm->flow_accel = sic->reply_accel;
    7d7c:	e587304c 	str	r3, [r7, #76]	@ 0x4c
	reply_cm->active = false;
    7d80:	e3a03000 	mov	r3, #0
    7d84:	e5c73018 	strb	r3, [r7, #24]
	reply_cm->active_next = NULL;
    7d88:	e3a03000 	mov	r3, #0
    7d8c:	e1c721f0 	strd	r2, [r7, #16]
	if (!(src_dev->flags & IFF_POINTOPOINT)) {
    7d90:	e59a3068 	ldr	r3, [sl, #104]	@ 0x68
    7d94:	e3130010 	tst	r3, #16
    7d98:	1a00000c 	bne	7dd0 <sfe_ipv6_create_rule+0x4ec>
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;
    7d9c:	e38e3010 	orr	r3, lr, #16
    7da0:	e5873048 	str	r3, [r7, #72]	@ 0x48
		if (src_dev->header_ops) {
    7da4:	e59a2158 	ldr	r2, [sl, #344]	@ 0x158
    7da8:	e3520000 	cmp	r2, #0
    7dac:	0a000006 	beq	7dcc <sfe_ipv6_create_rule+0x4e8>
			if (src_dev->header_ops->create == eth_header) {
    7db0:	e5921000 	ldr	r1, [r2]
    7db4:	e3002000 	movw	r2, #0
    7db8:	e3402000 	movt	r2, #0
    7dbc:	e1510002 	cmp	r1, r2
				reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
    7dc0:	038ee018 	orreq	lr, lr, #24
    7dc4:	0587e048 	streq	lr, [r7, #72]	@ 0x48
			if (src_dev->header_ops->create == eth_header) {
    7dc8:	0a000000 	beq	7dd0 <sfe_ipv6_create_rule+0x4ec>
    7dcc:	e1a0e003 	mov	lr, r3
    7dd0:	e5942038 	ldr	r2, [r4, #56]	@ 0x38
    7dd4:	e5943048 	ldr	r3, [r4, #72]	@ 0x48
    7dd8:	e1520003 	cmp	r2, r3
    7ddc:	0a0000ff 	beq	81e0 <sfe_ipv6_create_rule+0x8fc>
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST;
    7de0:	e5963048 	ldr	r3, [r6, #72]	@ 0x48
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC;
    7de4:	e38ee001 	orr	lr, lr, #1
    7de8:	e587e048 	str	lr, [r7, #72]	@ 0x48
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST;
    7dec:	e3833002 	orr	r3, r3, #2
    7df0:	e5863048 	str	r3, [r6, #72]	@ 0x48
    7df4:	e5942018 	ldr	r2, [r4, #24]
    7df8:	e5943028 	ldr	r3, [r4, #40]	@ 0x28
    7dfc:	e1520003 	cmp	r2, r3
    7e00:	0a0000e6 	beq	81a0 <sfe_ipv6_create_rule+0x8bc>
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC;
    7e04:	e5963048 	ldr	r3, [r6, #72]	@ 0x48
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST;
    7e08:	e38ee002 	orr	lr, lr, #2
    7e0c:	e587e048 	str	lr, [r7, #72]	@ 0x48
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC;
    7e10:	e3833001 	orr	r3, r3, #1
    7e14:	e5863048 	str	r3, [r6, #72]	@ 0x48
	c->src_ip[0] = sic->src_ip.ip6[0];
    7e18:	e14b24d4 	ldrd	r2, [fp, #-68]	@ 0xffffffbc
    7e1c:	e1c520fc 	strd	r2, [r5, #12]
	c->src_ip_xlate[0] = sic->src_ip_xlate.ip6[0];
    7e20:	e14b24dc 	ldrd	r2, [fp, #-76]	@ 0xffffffb4
	c->original_match = original_cm;
    7e24:	e5856054 	str	r6, [r5, #84]	@ 0x54
	c->original_dev = src_dev;
    7e28:	e585a058 	str	sl, [r5, #88]	@ 0x58
	c->reply_match = reply_cm;
    7e2c:	e585705c 	str	r7, [r5, #92]	@ 0x5c
	c->src_ip_xlate[0] = sic->src_ip_xlate.ip6[0];
    7e30:	e1c521fc 	strd	r2, [r5, #28]
	c->dest_ip[0] = sic->dest_ip.ip6[0];
    7e34:	e14b25d4 	ldrd	r2, [fp, #-84]	@ 0xffffffac
	c->reply_dev = dest_dev;
    7e38:	e5859060 	str	r9, [r5, #96]	@ 0x60
	c->dest_ip[0] = sic->dest_ip.ip6[0];
    7e3c:	e1c522fc 	strd	r2, [r5, #44]	@ 0x2c
	c->dest_ip_xlate[0] = sic->dest_ip_xlate.ip6[0];
    7e40:	e14b25dc 	ldrd	r2, [fp, #-92]	@ 0xffffffa4
    7e44:	e1c523fc 	strd	r2, [r5, #60]	@ 0x3c
	c->protocol = sic->protocol;
    7e48:	e51b3060 	ldr	r3, [fp, #-96]	@ 0xffffffa0
    7e4c:	e5853008 	str	r3, [r5, #8]
	c->src_port = sic->src_port;
    7e50:	e51b3030 	ldr	r3, [fp, #-48]	@ 0xffffffd0
    7e54:	e1c534bc 	strh	r3, [r5, #76]	@ 0x4c
	c->src_port_xlate = sic->src_port_xlate;
    7e58:	e51b3038 	ldr	r3, [fp, #-56]	@ 0xffffffc8
    7e5c:	e1c534be 	strh	r3, [r5, #78]	@ 0x4e
	c->dest_port = sic->dest_port;
    7e60:	e51b3034 	ldr	r3, [fp, #-52]	@ 0xffffffcc
    7e64:	e1c535b0 	strh	r3, [r5, #80]	@ 0x50
	c->dest_port_xlate = sic->dest_port_xlate;
    7e68:	e51b303c 	ldr	r3, [fp, #-60]	@ 0xffffffc4
    7e6c:	e1c535b2 	strh	r3, [r5, #82]	@ 0x52
	c->mark = sic->mark;
    7e70:	e5943098 	ldr	r3, [r4, #152]	@ 0x98
    7e74:	e5853078 	str	r3, [r5, #120]	@ 0x78
	c->src_ip[0] = sic->src_ip.ip6[0];
    7e78:	e1cc20d8 	ldrd	r2, [ip, #8]
    7e7c:	e1c521f4 	strd	r2, [r5, #20]
	c->src_ip_xlate[0] = sic->src_ip_xlate.ip6[0];
    7e80:	e1c423d0 	ldrd	r2, [r4, #48]	@ 0x30
    7e84:	e1c522f4 	strd	r2, [r5, #36]	@ 0x24
	c->dest_ip[0] = sic->dest_ip.ip6[0];
    7e88:	e1c424d0 	ldrd	r2, [r4, #64]	@ 0x40
    7e8c:	e1c523f4 	strd	r2, [r5, #52]	@ 0x34
	c->dest_ip_xlate[0] = sic->dest_ip_xlate.ip6[0];
    7e90:	e1c425d0 	ldrd	r2, [r4, #80]	@ 0x50
    7e94:	e1c524f4 	strd	r2, [r5, #68]	@ 0x44
	c->debug_read_seq = 0;
    7e98:	e3a03000 	mov	r3, #0
    7e9c:	e585307c 	str	r3, [r5, #124]	@ 0x7c
	c->last_sync_jiffies = get_jiffies_64();
    7ea0:	ebfffffe 	bl	0 <get_jiffies_64>
	dev_hold(c->original_dev);
    7ea4:	e5953058 	ldr	r3, [r5, #88]	@ 0x58
	c->last_sync_jiffies = get_jiffies_64();
    7ea8:	e1c506f8 	strd	r0, [r5, #104]	@ 0x68
	if (dev) {
    7eac:	e3530000 	cmp	r3, #0
    7eb0:	0a000007 	beq	7ed4 <sfe_ipv6_create_rule+0x5f0>
	asm volatile(
    7eb4:	e10f0000 	mrs	r0, CPSR
    7eb8:	f10c0080 	cpsid	i
		this_cpu_inc(*dev->pcpu_refcnt);
    7ebc:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    7ec0:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    7ec4:	e7932001 	ldr	r2, [r3, r1]
    7ec8:	e2822001 	add	r2, r2, #1
    7ecc:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    7ed0:	e121f000 	msr	CPSR_c, r0
	dev_hold(c->reply_dev);
    7ed4:	e5953060 	ldr	r3, [r5, #96]	@ 0x60
	if (dev) {
    7ed8:	e3530000 	cmp	r3, #0
    7edc:	0a000007 	beq	7f00 <sfe_ipv6_create_rule+0x61c>
	asm volatile(
    7ee0:	e10f0000 	mrs	r0, CPSR
    7ee4:	f10c0080 	cpsid	i
		this_cpu_inc(*dev->pcpu_refcnt);
    7ee8:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    7eec:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    7ef0:	e7932001 	ldr	r2, [r3, r1]
    7ef4:	e2822001 	add	r2, r2, #1
    7ef8:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    7efc:	e121f000 	msr	CPSR_c, r0
	switch (sic->protocol) {
    7f00:	e5943000 	ldr	r3, [r4]
    7f04:	e3530006 	cmp	r3, #6
    7f08:	0a000086 	beq	8128 <sfe_ipv6_create_rule+0x844>
	sfe_ipv6_connection_match_compute_translations(original_cm);
    7f0c:	e1a00006 	mov	r0, r6
    7f10:	ebffe111 	bl	35c <sfe_ipv6_connection_match_compute_translations>
	sfe_ipv6_connection_match_compute_translations(reply_cm);
    7f14:	e1a00007 	mov	r0, r7
    7f18:	ebffe10f 	bl	35c <sfe_ipv6_connection_match_compute_translations>
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7f1c:	e595300c 	ldr	r3, [r5, #12]
	c->prev = NULL;
    7f20:	e3a00000 	mov	r0, #0
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7f24:	e5951010 	ldr	r1, [r5, #16]
    7f28:	e595202c 	ldr	r2, [r5, #44]	@ 0x2c
	c->prev = NULL;
    7f2c:	e5850004 	str	r0, [r5, #4]
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7f30:	e5950014 	ldr	r0, [r5, #20]
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    7f34:	e1d5c5b0 	ldrh	ip, [r5, #80]	@ 0x50
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7f38:	e0222003 	eor	r2, r2, r3
    7f3c:	e5953030 	ldr	r3, [r5, #48]	@ 0x30
    7f40:	e0222001 	eor	r2, r2, r1
    7f44:	e5951018 	ldr	r1, [r5, #24]
    7f48:	e0222003 	eor	r2, r2, r3
    7f4c:	e5953034 	ldr	r3, [r5, #52]	@ 0x34
    7f50:	e0222000 	eor	r2, r2, r0
    7f54:	e5950038 	ldr	r0, [r5, #56]	@ 0x38
    7f58:	e0222003 	eor	r2, r2, r3
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    7f5c:	e1d534bc 	ldrh	r3, [r5, #76]	@ 0x4c
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7f60:	e0222001 	eor	r2, r2, r1
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    7f64:	e5d51008 	ldrb	r1, [r5, #8]
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    7f68:	e0222000 	eor	r2, r2, r0
	c->all_connections_next = NULL;
    7f6c:	e3a00000 	mov	r0, #0
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    7f70:	e023300c 	eor	r3, r3, ip
    7f74:	e6bf3fb3 	rev16	r3, r3
    7f78:	e0222001 	eor	r2, r2, r1
    7f7c:	e6ff3073 	uxth	r3, r3
    7f80:	e0233002 	eor	r3, r3, r2
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    7f84:	e0233623 	eor	r3, r3, r3, lsr #12
    7f88:	e7eb3053 	ubfx	r3, r3, #0, #12
	prev_head = *hash_head;
    7f8c:	e283300c 	add	r3, r3, #12
    7f90:	e7982103 	ldr	r2, [r8, r3, lsl #2]
	if (prev_head) {
    7f94:	e3520000 	cmp	r2, #0
		prev_head->prev = c;
    7f98:	15825004 	strne	r5, [r2, #4]
	if (si->all_connections_tail) {
    7f9c:	e5981010 	ldr	r1, [r8, #16]
	*hash_head = c;
    7fa0:	e7885103 	str	r5, [r8, r3, lsl #2]
	si->num_connections++;
    7fa4:	e5983014 	ldr	r3, [r8, #20]
	c->next = prev_head;
    7fa8:	e5852000 	str	r2, [r5]
	si->all_connections_tail = c;
    7fac:	e5885010 	str	r5, [r8, #16]
	if (si->all_connections_tail) {
    7fb0:	e3510000 	cmp	r1, #0
		c->all_connections_prev = si->all_connections_tail;
    7fb4:	e5851074 	str	r1, [r5, #116]	@ 0x74
	si->num_connections++;
    7fb8:	e2833001 	add	r3, r3, #1
		si->all_connections_tail->all_connections_next = c;
    7fbc:	15815070 	strne	r5, [r1, #112]	@ 0x70
	sfe_ipv6_insert_connection_match(si, c->original_match);
    7fc0:	e5951054 	ldr	r1, [r5, #84]	@ 0x54
	si->num_connections++;
    7fc4:	e5883014 	str	r3, [r8, #20]
	c->all_connections_next = NULL;
    7fc8:	e5850070 	str	r0, [r5, #112]	@ 0x70
		si->all_connections_head = c;
    7fcc:	0588500c 	streq	r5, [r8, #12]
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
    7fd0:	e5913024 	ldr	r3, [r1, #36]	@ 0x24
    7fd4:	e5912034 	ldr	r2, [r1, #52]	@ 0x34
    7fd8:	e1d1c4b4 	ldrh	ip, [r1, #68]	@ 0x44
    7fdc:	e0222003 	eor	r2, r2, r3
    7fe0:	e5913028 	ldr	r3, [r1, #40]	@ 0x28
    7fe4:	e0222003 	eor	r2, r2, r3
    7fe8:	e5913038 	ldr	r3, [r1, #56]	@ 0x38
    7fec:	e0222003 	eor	r2, r2, r3
    7ff0:	e591302c 	ldr	r3, [r1, #44]	@ 0x2c
    7ff4:	e0222003 	eor	r2, r2, r3
    7ff8:	e591303c 	ldr	r3, [r1, #60]	@ 0x3c
    7ffc:	e0222003 	eor	r2, r2, r3
    8000:	e5913030 	ldr	r3, [r1, #48]	@ 0x30
    8004:	e0222003 	eor	r2, r2, r3
    8008:	e1d134b6 	ldrh	r3, [r1, #70]	@ 0x46
    800c:	e02cc003 	eor	ip, ip, r3
    8010:	e5913040 	ldr	r3, [r1, #64]	@ 0x40
    8014:	e6bfcfbc 	rev16	ip, ip
    8018:	e6ffc07c 	uxth	ip, ip
    801c:	e0222003 	eor	r2, r2, r3
    8020:	e591301c 	ldr	r3, [r1, #28]
    8024:	e0233002 	eor	r3, r3, r2
    8028:	e5d12020 	ldrb	r2, [r1, #32]
    802c:	e0233002 	eor	r3, r3, r2
    8030:	e023300c 	eor	r3, r3, ip
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    8034:	e0233623 	eor	r3, r3, r3, lsr #12
    8038:	e7eb3053 	ubfx	r3, r3, #0, #12
	prev_head = *hash_head;
    803c:	e2833a01 	add	r3, r3, #4096	@ 0x1000
    8040:	e283300c 	add	r3, r3, #12
    8044:	e7982103 	ldr	r2, [r8, r3, lsl #2]
	cm->prev = NULL;
    8048:	e5810004 	str	r0, [r1, #4]
	if (prev_head) {
    804c:	e1520000 	cmp	r2, r0
	cm->prev = NULL;
    8050:	e3a00000 	mov	r0, #0
		prev_head->prev = cm;
    8054:	15821004 	strne	r1, [r2, #4]
	cm->next = prev_head;
    8058:	e5812000 	str	r2, [r1]
	sfe_ipv6_insert_connection_match(si, c->reply_match);
    805c:	e595205c 	ldr	r2, [r5, #92]	@ 0x5c
	*hash_head = cm;
    8060:	e7881103 	str	r1, [r8, r3, lsl #2]
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
    8064:	e592e024 	ldr	lr, [r2, #36]	@ 0x24
    8068:	e5923028 	ldr	r3, [r2, #40]	@ 0x28
    806c:	e5921034 	ldr	r1, [r2, #52]	@ 0x34
    8070:	e1d2c4b4 	ldrh	ip, [r2, #68]	@ 0x44
    8074:	e021100e 	eor	r1, r1, lr
    8078:	e0211003 	eor	r1, r1, r3
    807c:	e5923038 	ldr	r3, [r2, #56]	@ 0x38
    8080:	e0211003 	eor	r1, r1, r3
    8084:	e592302c 	ldr	r3, [r2, #44]	@ 0x2c
    8088:	e0211003 	eor	r1, r1, r3
    808c:	e592303c 	ldr	r3, [r2, #60]	@ 0x3c
    8090:	e0211003 	eor	r1, r1, r3
    8094:	e5923030 	ldr	r3, [r2, #48]	@ 0x30
    8098:	e0211003 	eor	r1, r1, r3
    809c:	e5923040 	ldr	r3, [r2, #64]	@ 0x40
    80a0:	e0211003 	eor	r1, r1, r3
    80a4:	e592301c 	ldr	r3, [r2, #28]
    80a8:	e0233001 	eor	r3, r3, r1
    80ac:	e5d21020 	ldrb	r1, [r2, #32]
    80b0:	e0233001 	eor	r3, r3, r1
    80b4:	e1d214b6 	ldrh	r1, [r2, #70]	@ 0x46
    80b8:	e021100c 	eor	r1, r1, ip
    80bc:	e6bf1fb1 	rev16	r1, r1
    80c0:	e6ff1071 	uxth	r1, r1
    80c4:	e0233001 	eor	r3, r3, r1
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    80c8:	e0233623 	eor	r3, r3, r3, lsr #12
    80cc:	e7eb3053 	ubfx	r3, r3, #0, #12
	prev_head = *hash_head;
    80d0:	e2833a01 	add	r3, r3, #4096	@ 0x1000
    80d4:	e283300c 	add	r3, r3, #12
    80d8:	e7981103 	ldr	r1, [r8, r3, lsl #2]
	cm->prev = NULL;
    80dc:	e5820004 	str	r0, [r2, #4]
	if (prev_head) {
    80e0:	e1510000 	cmp	r1, r0
    80e4:	e3000000 	movw	r0, #0
    80e8:	e3400000 	movt	r0, #0
		prev_head->prev = cm;
    80ec:	15812004 	strne	r2, [r1, #4]
	cm->next = prev_head;
    80f0:	e5821000 	str	r1, [r2]
	*hash_head = cm;
    80f4:	e7882103 	str	r2, [r8, r3, lsl #2]
    80f8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	return 0;
    80fc:	e3a00000 	mov	r0, #0
}
    8100:	e24bd028 	sub	sp, fp, #40	@ 0x28
    8104:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		original_cm->priority = sic->src_priority;
    8108:	e59420a4 	ldr	r2, [r4, #164]	@ 0xa4
    810c:	e5862090 	str	r2, [r6, #144]	@ 0x90
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
    8110:	e3a02020 	mov	r2, #32
    8114:	e1a0e002 	mov	lr, r2
    8118:	e5862048 	str	r2, [r6, #72]	@ 0x48
    811c:	e3a02060 	mov	r2, #96	@ 0x60
    8120:	e50b2068 	str	r2, [fp, #-104]	@ 0xffffff98
    8124:	eafffeb8 	b	7c0c <sfe_ipv6_create_rule+0x328>
		original_cm->protocol_state.tcp.win_scale = sic->src_td_window_scale;
    8128:	e5d43078 	ldrb	r3, [r4, #120]	@ 0x78
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    812c:	e594200c 	ldr	r2, [r4, #12]
		original_cm->protocol_state.tcp.win_scale = sic->src_td_window_scale;
    8130:	e5c63050 	strb	r3, [r6, #80]	@ 0x50
		original_cm->protocol_state.tcp.max_win = sic->src_td_max_window ? sic->src_td_max_window : 1;
    8134:	e594307c 	ldr	r3, [r4, #124]	@ 0x7c
    8138:	e3530001 	cmp	r3, #1
    813c:	33a03001 	movcc	r3, #1
    8140:	e5863054 	str	r3, [r6, #84]	@ 0x54
		original_cm->protocol_state.tcp.end = sic->src_td_end;
    8144:	e5943080 	ldr	r3, [r4, #128]	@ 0x80
    8148:	e5863058 	str	r3, [r6, #88]	@ 0x58
		original_cm->protocol_state.tcp.max_end = sic->src_td_max_end;
    814c:	e5943084 	ldr	r3, [r4, #132]	@ 0x84
    8150:	e586305c 	str	r3, [r6, #92]	@ 0x5c
		reply_cm->protocol_state.tcp.win_scale = sic->dest_td_window_scale;
    8154:	e5d43088 	ldrb	r3, [r4, #136]	@ 0x88
    8158:	e5c73050 	strb	r3, [r7, #80]	@ 0x50
		reply_cm->protocol_state.tcp.max_win = sic->dest_td_max_window ? sic->dest_td_max_window : 1;
    815c:	e594308c 	ldr	r3, [r4, #140]	@ 0x8c
    8160:	e3530001 	cmp	r3, #1
    8164:	33a03001 	movcc	r3, #1
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    8168:	e3120001 	tst	r2, #1
		reply_cm->protocol_state.tcp.max_win = sic->dest_td_max_window ? sic->dest_td_max_window : 1;
    816c:	e5873054 	str	r3, [r7, #84]	@ 0x54
		reply_cm->protocol_state.tcp.end = sic->dest_td_end;
    8170:	e5943090 	ldr	r3, [r4, #144]	@ 0x90
    8174:	e5873058 	str	r3, [r7, #88]	@ 0x58
		reply_cm->protocol_state.tcp.max_end = sic->dest_td_max_end;
    8178:	e5943094 	ldr	r3, [r4, #148]	@ 0x94
    817c:	e587305c 	str	r3, [r7, #92]	@ 0x5c
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
    8180:	0affff61 	beq	7f0c <sfe_ipv6_create_rule+0x628>
			original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    8184:	e5962048 	ldr	r2, [r6, #72]	@ 0x48
			reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    8188:	e5973048 	ldr	r3, [r7, #72]	@ 0x48
			original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    818c:	e3822004 	orr	r2, r2, #4
			reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    8190:	e3833004 	orr	r3, r3, #4
			original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    8194:	e5862048 	str	r2, [r6, #72]	@ 0x48
			reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
    8198:	e5873048 	str	r3, [r7, #72]	@ 0x48
    819c:	eaffff5a 	b	7f0c <sfe_ipv6_create_rule+0x628>
	return a->addr[0] == b->addr[0] &&
    81a0:	e594201c 	ldr	r2, [r4, #28]
    81a4:	e594302c 	ldr	r3, [r4, #44]	@ 0x2c
    81a8:	e1520003 	cmp	r2, r3
    81ac:	1affff14 	bne	7e04 <sfe_ipv6_create_rule+0x520>
	       a->addr[1] == b->addr[1] &&
    81b0:	e5942020 	ldr	r2, [r4, #32]
    81b4:	e5943030 	ldr	r3, [r4, #48]	@ 0x30
    81b8:	e1520003 	cmp	r2, r3
    81bc:	1affff10 	bne	7e04 <sfe_ipv6_create_rule+0x520>
	if (!sfe_ipv6_addr_equal(sic->src_ip.ip6, sic->src_ip_xlate.ip6) || sic->src_port != sic->src_port_xlate) {
    81c0:	e51b0038 	ldr	r0, [fp, #-56]	@ 0xffffffc8
    81c4:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
	       a->addr[2] == b->addr[2] &&
    81c8:	e5942024 	ldr	r2, [r4, #36]	@ 0x24
    81cc:	e5943034 	ldr	r3, [r4, #52]	@ 0x34
    81d0:	e1510000 	cmp	r1, r0
    81d4:	01520003 	cmpeq	r2, r3
    81d8:	1affff09 	bne	7e04 <sfe_ipv6_create_rule+0x520>
    81dc:	eaffff0d 	b	7e18 <sfe_ipv6_create_rule+0x534>
	return a->addr[0] == b->addr[0] &&
    81e0:	e594203c 	ldr	r2, [r4, #60]	@ 0x3c
    81e4:	e594304c 	ldr	r3, [r4, #76]	@ 0x4c
    81e8:	e1520003 	cmp	r2, r3
    81ec:	1afffefb 	bne	7de0 <sfe_ipv6_create_rule+0x4fc>
	       a->addr[1] == b->addr[1] &&
    81f0:	e5942040 	ldr	r2, [r4, #64]	@ 0x40
    81f4:	e5943050 	ldr	r3, [r4, #80]	@ 0x50
    81f8:	e1520003 	cmp	r2, r3
    81fc:	1afffef7 	bne	7de0 <sfe_ipv6_create_rule+0x4fc>
	if (!sfe_ipv6_addr_equal(sic->dest_ip.ip6, sic->dest_ip_xlate.ip6) || sic->dest_port != sic->dest_port_xlate) {
    8200:	e51b003c 	ldr	r0, [fp, #-60]	@ 0xffffffc4
    8204:	e51b1034 	ldr	r1, [fp, #-52]	@ 0xffffffcc
	       a->addr[2] == b->addr[2] &&
    8208:	e5942044 	ldr	r2, [r4, #68]	@ 0x44
    820c:	e5943054 	ldr	r3, [r4, #84]	@ 0x54
    8210:	e1510000 	cmp	r1, r0
    8214:	01520003 	cmpeq	r2, r3
    8218:	1afffef0 	bne	7de0 <sfe_ipv6_create_rule+0x4fc>
    821c:	eafffef4 	b	7df4 <sfe_ipv6_create_rule+0x510>
		return -EINVAL;
    8220:	e3e00015 	mvn	r0, #21
}
    8224:	e24bd028 	sub	sp, fp, #40	@ 0x28
    8228:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
    822c:	e3000000 	movw	r0, #0
    8230:	e3400000 	movt	r0, #0
    8234:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return -ENOMEM;
    8238:	e3e0000b 	mvn	r0, #11
    823c:	eaffffaf 	b	8100 <sfe_ipv6_create_rule+0x81c>
    8240:	e3000000 	movw	r0, #0
    8244:	e3400000 	movt	r0, #0
    8248:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		kfree(c);
    824c:	e1a00005 	mov	r0, r5
    8250:	ebfffffe 	bl	0 <kfree>
		return -ENOMEM;
    8254:	eafffff7 	b	8238 <sfe_ipv6_create_rule+0x954>
    8258:	e3000000 	movw	r0, #0
    825c:	e3400000 	movt	r0, #0
    8260:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		kfree(original_cm);
    8264:	e1a00006 	mov	r0, r6
    8268:	ebfffffe 	bl	0 <kfree>
		kfree(c);
    826c:	e1a00005 	mov	r0, r5
    8270:	ebfffffe 	bl	0 <kfree>
		return -ENOMEM;
    8274:	eaffffef 	b	8238 <sfe_ipv6_create_rule+0x954>
		sfe_ipv6_update_tcp_state(c, sic);
    8278:	e5930054 	ldr	r0, [r3, #84]	@ 0x54
    827c:	e1a02004 	mov	r2, r4
    8280:	e593105c 	ldr	r1, [r3, #92]	@ 0x5c
    8284:	ebffe4f6 	bl	1664 <sfe_ipv6_update_tcp_state.isra.0>
		break;
    8288:	eafffe03 	b	7a9c <sfe_ipv6_create_rule+0x1b8>
    828c:	00010050 	.word	0x00010050

00008290 <fast_classifier_post_routing>:
{
    8290:	e1a0c00d 	mov	ip, sp
    8294:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    8298:	e24cb004 	sub	fp, ip, #4
    829c:	e24ddf59 	sub	sp, sp, #356	@ 0x164
	if (unlikely(skb->pkt_type == PACKET_BROADCAST)) {
    82a0:	e5d0306c 	ldrb	r3, [r0, #108]	@ 0x6c
{
    82a4:	e1a04000 	mov	r4, r0
    82a8:	e1a05001 	mov	r5, r1
	if (unlikely(skb->pkt_type == PACKET_BROADCAST)) {
    82ac:	e2033007 	and	r3, r3, #7
	if (unlikely(skb->pkt_type == PACKET_MULTICAST)) {
    82b0:	e2433001 	sub	r3, r3, #1
    82b4:	e3530001 	cmp	r3, #1
    82b8:	9a000007 	bls	82dc <fast_classifier_post_routing+0x4c>
	return (struct dst_entry *)(skb->_skb_refdst & SKB_DST_PTRMASK);
    82bc:	e590304c 	ldr	r3, [r0, #76]	@ 0x4c
    82c0:	e3c33001 	bic	r3, r3, #1
	if (unlikely(skb_dst(skb)->xfrm)) {
    82c4:	e5933010 	ldr	r3, [r3, #16]
    82c8:	e3530000 	cmp	r3, #0
    82cc:	1a000002 	bne	82dc <fast_classifier_post_routing+0x4c>
	if (skb->sk) {
    82d0:	e590300c 	ldr	r3, [r0, #12]
    82d4:	e3530000 	cmp	r3, #0
    82d8:	0a000002 	beq	82e8 <fast_classifier_post_routing+0x58>
}
    82dc:	e3a00001 	mov	r0, #1
    82e0:	e24bd028 	sub	sp, fp, #40	@ 0x28
    82e4:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
	in = dev_get_by_index(&init_net, skb->skb_iif);
    82e8:	e3000000 	movw	r0, #0
    82ec:	e594107c 	ldr	r1, [r4, #124]	@ 0x7c
    82f0:	e3400000 	movt	r0, #0
    82f4:	ebfffffe 	bl	0 <dev_get_by_index>
	if (!in) {
    82f8:	e3500000 	cmp	r0, #0
    82fc:	0afffff6 	beq	82dc <fast_classifier_post_routing+0x4c>
	asm volatile(
    8300:	e10fc000 	mrs	ip, CPSR
    8304:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    8308:	e5903310 	ldr	r3, [r0, #784]	@ 0x310
    830c:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8310:	e7932001 	ldr	r2, [r3, r1]
    8314:	e2422001 	sub	r2, r2, #1
    8318:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    831c:	e121f00c 	msr	CPSR_c, ip
	return skb->_nfct;
    8320:	e5943054 	ldr	r3, [r4, #84]	@ 0x54
	*ctinfo = nfct & NFCT_INFOMASK;
    8324:	e2031007 	and	r1, r3, #7
	return (struct nf_conn *)(nfct & NFCT_PTRMASK);
    8328:	e3c36007 	bic	r6, r3, #7
	if (unlikely(ctinfo == IP_CT_UNTRACKED)) {
    832c:	e3510007 	cmp	r1, #7
    8330:	13530007 	cmpne	r3, #7
    8334:	93a01001 	movls	r1, #1
    8338:	83a01000 	movhi	r1, #0
    833c:	9affffe6 	bls	82dc <fast_classifier_post_routing+0x4c>
    8340:	e596306c 	ldr	r3, [r6, #108]	@ 0x6c
	if (!nf_ct_is_confirmed(ct)) {
    8344:	e3130008 	tst	r3, #8
    8348:	0affffe3 	beq	82dc <fast_classifier_post_routing+0x4c>

void nf_ct_helper_destroy(struct nf_conn *ct);

static inline struct nf_conn_help *nfct_help(const struct nf_conn *ct)
{
	return nf_ct_ext_find(ct, NF_CT_EXT_HELPER);
    834c:	e5960080 	ldr	r0, [r6, #128]	@ 0x80
	if (!ext || !__nf_ct_ext_exist(ext, id))
    8350:	e3500000 	cmp	r0, #0
    8354:	0a000008 	beq	837c <fast_classifier_post_routing+0xec>
	return !!ext->offset[id];
    8358:	e5d03000 	ldrb	r3, [r0]
	if (!ext || !__nf_ct_ext_exist(ext, id))
    835c:	e3530000 	cmp	r3, #0
    8360:	0a000005 	beq	837c <fast_classifier_post_routing+0xec>
	if (unlikely(ext->gen_id))
    8364:	e5902008 	ldr	r2, [r0, #8]
    8368:	e3520000 	cmp	r2, #0
	return (void *)ct->ext + ct->ext->offset[id];
    836c:	00800003 	addeq	r0, r0, r3
	if (unlikely(ext->gen_id))
    8370:	1a00007f 	bne	8574 <fast_classifier_post_routing+0x2e4>
	if (unlikely(nfct_help(ct))) {
    8374:	e3500000 	cmp	r0, #0
    8378:	1affffd7 	bne	82dc <fast_classifier_post_routing+0x4c>
	memset(&sic, 0, sizeof(sic));
    837c:	e3a020b4 	mov	r2, #180	@ 0xb4
    8380:	e3a01000 	mov	r1, #0
    8384:	e24b00e4 	sub	r0, fp, #228	@ 0xe4
    8388:	ebfffffe 	bl	0 <memset>
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    838c:	e1c601d4 	ldrd	r0, [r6, #20]
    8390:	e24b3f5a 	sub	r3, fp, #360	@ 0x168
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
    8394:	e24b2d05 	sub	r2, fp, #320	@ 0x140
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    8398:	e5d6c03a 	ldrb	ip, [r6, #58]	@ 0x3a
	if (likely(is_v4)) {
    839c:	e3550000 	cmp	r5, #0
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    83a0:	e596e014 	ldr	lr, [r6, #20]
    83a4:	e5967028 	ldr	r7, [r6, #40]	@ 0x28
    83a8:	e1c300f0 	strd	r0, [r3]
    83ac:	e1c601dc 	ldrd	r0, [r6, #28]
    83b0:	e1d683b8 	ldrh	r8, [r6, #56]	@ 0x38
	sic.protocol = (s32)orig_tuple.dst.protonum;
    83b4:	e50bc0e4 	str	ip, [fp, #-228]	@ 0xffffff1c
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
    83b8:	e1d695b4 	ldrh	r9, [r6, #84]	@ 0x54
    83bc:	e1d6a6b8 	ldrh	sl, [r6, #104]	@ 0x68
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    83c0:	e1c300f8 	strd	r0, [r3, #8]
    83c4:	e1c602d4 	ldrd	r0, [r6, #36]	@ 0x24
    83c8:	e1c301f0 	strd	r0, [r3, #16]
    83cc:	e1c602dc 	ldrd	r0, [r6, #44]	@ 0x2c
    83d0:	e1c301f8 	strd	r0, [r3, #24]
    83d4:	e1c603d4 	ldrd	r0, [r6, #52]	@ 0x34
    83d8:	e1c302f0 	strd	r0, [r3, #32]
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
    83dc:	e5961044 	ldr	r1, [r6, #68]	@ 0x44
    83e0:	e50b1180 	str	r1, [fp, #-384]	@ 0xfffffe80
    83e4:	e5961058 	ldr	r1, [r6, #88]	@ 0x58
    83e8:	e50b117c 	str	r1, [fp, #-380]	@ 0xfffffe84
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    83ec:	e1d612b4 	ldrh	r1, [r6, #36]	@ 0x24
    83f0:	e50b1178 	str	r1, [fp, #-376]	@ 0xfffffe88
	reply_tuple = ct->tuplehash[IP_CT_DIR_REPLY].tuple;
    83f4:	e1c604d4 	ldrd	r0, [r6, #68]	@ 0x44
    83f8:	e1c200f0 	strd	r0, [r2]
    83fc:	e1c604dc 	ldrd	r0, [r6, #76]	@ 0x4c
    8400:	e1c200f8 	strd	r0, [r2, #8]
    8404:	e1c605d4 	ldrd	r0, [r6, #84]	@ 0x54
    8408:	e1c201f0 	strd	r0, [r2, #16]
    840c:	e1c605dc 	ldrd	r0, [r6, #92]	@ 0x5c
    8410:	e1c201f8 	strd	r0, [r2, #24]
    8414:	e1c606d4 	ldrd	r0, [r6, #100]	@ 0x64
    8418:	e1c202f0 	strd	r0, [r2, #32]
	if (likely(is_v4)) {
    841c:	0a00005c 	beq	8594 <fast_classifier_post_routing+0x304>
		sic.src_ip.ip = (__be32)orig_tuple.src.u3.ip;
    8420:	e50be0cc 	str	lr, [fp, #-204]	@ 0xffffff34
	return (addr & htonl(0xff000000)) == htonl(0x7f000000);
}

static inline bool ipv4_is_multicast(__be32 addr)
{
	return (addr & htonl(0xf0000000)) == htonl(0xe0000000);
    8424:	e20ee0f0 	and	lr, lr, #240	@ 0xf0
		if (ipv4_is_multicast(sic.src_ip.ip) || ipv4_is_multicast(sic.dest_ip.ip)) {
    8428:	e35e00e0 	cmp	lr, #224	@ 0xe0
		sic.dest_ip.ip = (__be32)orig_tuple.dst.u3.ip;
    842c:	e50b70ac 	str	r7, [fp, #-172]	@ 0xffffff54
		if (ipv4_is_multicast(sic.src_ip.ip) || ipv4_is_multicast(sic.dest_ip.ip)) {
    8430:	0affffa9 	beq	82dc <fast_classifier_post_routing+0x4c>
    8434:	e20770f0 	and	r7, r7, #240	@ 0xf0
    8438:	e35700e0 	cmp	r7, #224	@ 0xe0
    843c:	0affffa6 	beq	82dc <fast_classifier_post_routing+0x4c>
		sic.src_ip_xlate.ip = (__be32)reply_tuple.dst.u3.ip;
    8440:	e51b317c 	ldr	r3, [fp, #-380]	@ 0xfffffe84
	return skb->head + skb->network_header;
    8444:	e1d42ab0 	ldrh	r2, [r4, #160]	@ 0xa0
    8448:	e50b30bc 	str	r3, [fp, #-188]	@ 0xffffff44
		sic.dest_ip_xlate.ip = (__be32)reply_tuple.src.u3.ip;
    844c:	e51b3180 	ldr	r3, [fp, #-384]	@ 0xfffffe80
    8450:	e50b309c 	str	r3, [fp, #-156]	@ 0xffffff64
#include <asm/byteorder.h>


static inline __u8 ipv4_get_dsfield(const struct iphdr *iph)
{
	return iph->tos;
    8454:	e59430ac 	ldr	r3, [r4, #172]	@ 0xac
    8458:	e0833002 	add	r3, r3, r2
		dscp = ipv4_get_dsfield(ip_hdr(skb)) >> XT_DSCP_SHIFT;
    845c:	e5d33001 	ldrb	r3, [r3, #1]
    8460:	e1a03123 	lsr	r3, r3, #2
			sic.flags |= SFE_CREATE_FLAG_REMARK_DSCP;
    8464:	e51b1178 	ldr	r1, [fp, #-376]	@ 0xfffffe88
    8468:	e3a02004 	mov	r2, #4
	switch (sic.protocol) {
    846c:	e35c0006 	cmp	ip, #6
			sic.src_dscp = sic.dest_dscp;
    8470:	e50b3038 	str	r3, [fp, #-56]	@ 0xffffffc8
    8474:	e50b3034 	str	r3, [fp, #-52]	@ 0xffffffcc
    8478:	e3a03000 	mov	r3, #0
			sic.flags |= SFE_CREATE_FLAG_REMARK_DSCP;
    847c:	e50b20d8 	str	r2, [fp, #-216]	@ 0xffffff28
	switch (sic.protocol) {
    8480:	e1a02003 	mov	r2, r3
    8484:	e7cf3011 	bfi	r3, r1, #0, #16
    8488:	e7cf2018 	bfi	r2, r8, #0, #16
    848c:	e7df381a 	bfi	r3, sl, #16, #16
    8490:	e7df2819 	bfi	r2, r9, #16, #16
    8494:	0a000038 	beq	857c <fast_classifier_post_routing+0x2ec>
    8498:	e35c0011 	cmp	ip, #17
    849c:	1affff8e 	bne	82dc <fast_classifier_post_routing+0x4c>
		sic.src_port = orig_tuple.src.u.udp.port;
    84a0:	e50b308c 	str	r3, [fp, #-140]	@ 0xffffff74
		if (ntohs(sic.dest_port) == 4500 || ntohs(sic.dest_port) == 500) {
    84a4:	e30f3401 	movw	r3, #62465	@ 0xf401
		sic.src_port = orig_tuple.src.u.udp.port;
    84a8:	e50b2088 	str	r2, [fp, #-136]	@ 0xffffff78
		if (ntohs(sic.dest_port) == 4500 || ntohs(sic.dest_port) == 500) {
    84ac:	e3092411 	movw	r2, #37905	@ 0x9411
    84b0:	e1580003 	cmp	r8, r3
    84b4:	11580002 	cmpne	r8, r2
    84b8:	0affff87 	beq	82dc <fast_classifier_post_routing+0x4c>
	if (skb->priority) {
    84bc:	e5942078 	ldr	r2, [r4, #120]	@ 0x78
	sic.original_accel = 1;
    84c0:	e3a03001 	mov	r3, #1
    84c4:	e50b3048 	str	r3, [fp, #-72]	@ 0xffffffb8
    84c8:	e50b3044 	str	r3, [fp, #-68]	@ 0xffffffbc
	if (skb->priority) {
    84cc:	e3520000 	cmp	r2, #0
    84d0:	0a000003 	beq	84e4 <fast_classifier_post_routing+0x254>
		sic.flags |= SFE_CREATE_FLAG_REMARK_PRIORITY;
    84d4:	e3a03006 	mov	r3, #6
    84d8:	e50b30d8 	str	r3, [fp, #-216]	@ 0xffffff28
		sic.src_priority = sic.dest_priority;
    84dc:	e50b2040 	str	r2, [fp, #-64]	@ 0xffffffc0
    84e0:	e50b203c 	str	r2, [fp, #-60]	@ 0xffffffc4
	raw_spin_lock_bh(&lock->rlock);
    84e4:	e59f09e4 	ldr	r0, [pc, #2532]	@ 8ed0 <fast_classifier_post_routing+0xc40>
    84e8:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	conn = fast_classifier_find_conn(&sic.src_ip, &sic.dest_ip, sic.src_port, sic.dest_port, sic.protocol, is_v4);
    84ec:	e58d5004 	str	r5, [sp, #4]
    84f0:	e55b30e4 	ldrb	r3, [fp, #-228]	@ 0xffffff1c
    84f4:	e24b10ac 	sub	r1, fp, #172	@ 0xac
    84f8:	e24b00cc 	sub	r0, fp, #204	@ 0xcc
    84fc:	e58d3000 	str	r3, [sp]
    8500:	e15b38b8 	ldrh	r3, [fp, #-136]	@ 0xffffff78
    8504:	e15b28bc 	ldrh	r2, [fp, #-140]	@ 0xffffff74
    8508:	ebffed0e 	bl	3948 <fast_classifier_find_conn>
	if (conn) {
    850c:	e2507000 	subs	r7, r0, #0
    8510:	0a00009f 	beq	8794 <fast_classifier_post_routing+0x504>
		conn->hits++;
    8514:	e5973010 	ldr	r3, [r7, #16]
		if (!conn->offloaded) {
    8518:	e5972018 	ldr	r2, [r7, #24]
		conn->hits++;
    851c:	e2833001 	add	r3, r3, #1
		if (!conn->offloaded) {
    8520:	e3520000 	cmp	r2, #0
		conn->hits++;
    8524:	e5873010 	str	r3, [r7, #16]
		if (!conn->offloaded) {
    8528:	1a000061 	bne	86b4 <fast_classifier_post_routing+0x424>
			if (conn->offload_permit || conn->hits >= offload_at_pkts) {
    852c:	e5972014 	ldr	r2, [r7, #20]
    8530:	e3520000 	cmp	r2, #0
    8534:	1a000004 	bne	854c <fast_classifier_post_routing+0x2bc>
    8538:	e3002000 	movw	r2, #0
    853c:	e3402000 	movt	r2, #0
    8540:	e5922048 	ldr	r2, [r2, #72]	@ 0x48
    8544:	e1530002 	cmp	r3, r2
    8548:	ba000059 	blt	86b4 <fast_classifier_post_routing+0x424>
				if (fast_classifier_update_protocol(conn->sic, conn->ct) == 0) {
    854c:	e5973008 	ldr	r3, [r7, #8]
    8550:	e597400c 	ldr	r4, [r7, #12]
	switch (p_sic->protocol) {
    8554:	e5932000 	ldr	r2, [r3]
    8558:	e3520006 	cmp	r2, #6
    855c:	0a00016e 	beq	8b1c <fast_classifier_post_routing+0x88c>
    8560:	e3520011 	cmp	r2, #17
    8564:	0a00002e 	beq	8624 <fast_classifier_post_routing+0x394>
	raw_spin_unlock_bh(&lock->rlock);
    8568:	e59f0960 	ldr	r0, [pc, #2400]	@ 8ed0 <fast_classifier_post_routing+0xc40>
    856c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
					return NF_ACCEPT;
    8570:	eaffff59 	b	82dc <fast_classifier_post_routing+0x4c>
		return __nf_ct_ext_find(ext, id);
    8574:	ebfffffe 	bl	0 <__nf_ct_ext_find>
    8578:	eaffff7d 	b	8374 <fast_classifier_post_routing+0xe4>
		sic.src_port = orig_tuple.src.u.tcp.port;
    857c:	e50b308c 	str	r3, [fp, #-140]	@ 0xffffff74
    8580:	e596306c 	ldr	r3, [r6, #108]	@ 0x6c
    8584:	e50b2088 	str	r2, [fp, #-136]	@ 0xffffff78
		if (!test_bit(IPS_ASSURED_BIT, &ct->status)) {
    8588:	e3130004 	tst	r3, #4
    858c:	1affffca 	bne	84bc <fast_classifier_post_routing+0x22c>
    8590:	eaffff51 	b	82dc <fast_classifier_post_routing+0x4c>
		sic.src_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.src.u3.in6);
    8594:	e1c300d0 	ldrd	r0, [r3]
		sic.dest_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.dst.u3.in6);
    8598:	e24bef55 	sub	lr, fp, #340	@ 0x154
		sic.src_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.src.u3.in6);
    859c:	e14b0cfc 	strd	r0, [fp, #-204]	@ 0xffffff34
	return (a->s6_addr32[0] & htonl(0xfffffff0)) == htonl(0x20010010);
}

static inline bool ipv6_addr_is_multicast(const struct in6_addr *addr)
{
	return (addr->s6_addr32[0] & htonl(0xFF000000)) == htonl(0xFF000000);
    85a0:	e6ef0070 	uxtb	r0, r0
		if (ipv6_addr_is_multicast((struct in6_addr *)sic.src_ip.ip6) ||
    85a4:	e35000ff 	cmp	r0, #255	@ 0xff
		sic.src_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.src.u3.in6);
    85a8:	e1c300d8 	ldrd	r0, [r3, #8]
    85ac:	e14b0cf4 	strd	r0, [fp, #-196]	@ 0xffffff3c
		sic.dest_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.dst.u3.in6);
    85b0:	e1ce00d0 	ldrd	r0, [lr]
    85b4:	e14b0afc 	strd	r0, [fp, #-172]	@ 0xffffff54
    85b8:	e1ce00d8 	ldrd	r0, [lr, #8]
    85bc:	e14b0af4 	strd	r0, [fp, #-164]	@ 0xffffff5c
		if (ipv6_addr_is_multicast((struct in6_addr *)sic.src_ip.ip6) ||
    85c0:	0affff45 	beq	82dc <fast_classifier_post_routing+0x4c>
    85c4:	e55b30ac 	ldrb	r3, [fp, #-172]	@ 0xffffff54
    85c8:	e35300ff 	cmp	r3, #255	@ 0xff
    85cc:	0affff42 	beq	82dc <fast_classifier_post_routing+0x4c>
		sic.src_ip_xlate.ip6[0] = *((struct sfe_ipv6_addr *)&reply_tuple.dst.u3.in6);
    85d0:	e24b3f4b 	sub	r3, fp, #300	@ 0x12c
}


static inline __u8 ipv6_get_dsfield(const struct ipv6hdr *ipv6h)
{
	return ntohs(*(__force const __be16 *)ipv6h) >> 4;
    85d4:	e59470ac 	ldr	r7, [r4, #172]	@ 0xac
    85d8:	e1c300d0 	ldrd	r0, [r3]
    85dc:	e1d4eab0 	ldrh	lr, [r4, #160]	@ 0xa0
    85e0:	e14b0bfc 	strd	r0, [fp, #-188]	@ 0xffffff44
    85e4:	e1c300d8 	ldrd	r0, [r3, #8]
    85e8:	e14b0bf4 	strd	r0, [fp, #-180]	@ 0xffffff4c
		sic.dest_ip_xlate.ip6[0] = *((struct sfe_ipv6_addr *)&reply_tuple.src.u3.in6);
    85ec:	e1c200d0 	ldrd	r0, [r2]
    85f0:	e1c220d8 	ldrd	r2, [r2, #8]
    85f4:	e14b09fc 	strd	r0, [fp, #-156]	@ 0xffffff64
    85f8:	e14b29f4 	strd	r2, [fp, #-148]	@ 0xffffff6c
    85fc:	e19730be 	ldrh	r3, [r7, lr]
    8600:	e6bf3fb3 	rev16	r3, r3
		dscp = ipv6_get_dsfield(ipv6_hdr(skb)) >> XT_DSCP_SHIFT;
    8604:	e7e53353 	ubfx	r3, r3, #6, #6
			sic.flags |= SFE_CREATE_FLAG_REMARK_DSCP;
    8608:	eaffff95 	b	8464 <fast_classifier_post_routing+0x1d4>
	}
}

static inline void arch_spin_unlock(arch_spinlock_t *lock)
{
	smp_mb();
    860c:	f57ff05b 	dmb	ish
	lock->tickets.owner++;
    8610:	e1d430b4 	ldrh	r3, [r4, #4]
    8614:	e2833001 	add	r3, r3, #1
    8618:	e1c430b4 	strh	r3, [r4, #4]
	dsb(ishst);
    861c:	f57ff04a 	dsb	ishst
	__asm__(SEV);
    8620:	e320f004 	sev
    8624:	e59f08a4 	ldr	r0, [pc, #2212]	@ 8ed0 <fast_classifier_post_routing+0xc40>
    8628:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
				ret = is_v4 ? sfe_ipv4_create_rule(conn->sic) : sfe_ipv6_create_rule(conn->sic);
    862c:	e3550000 	cmp	r5, #0
    8630:	e5970008 	ldr	r0, [r7, #8]
    8634:	0a000125 	beq	8ad0 <fast_classifier_post_routing+0x840>
    8638:	ebfff81f 	bl	66bc <sfe_ipv4_create_rule>
				if ((ret == 0) || (ret == -EADDRINUSE)) {
    863c:	e3700062 	cmn	r0, #98	@ 0x62
    8640:	13500000 	cmpne	r0, #0
    8644:	1affff24 	bne	82dc <fast_classifier_post_routing+0x4c>
						fc_msg.src_saddr.in = *((struct in_addr *)&sic.src_ip);
    8648:	e51b20cc 	ldr	r2, [fp, #-204]	@ 0xffffff34
						fc_msg.ethertype = AF_INET;
    864c:	e3a01002 	mov	r1, #2
    8650:	e24b0f46 	sub	r0, fp, #280	@ 0x118
						fc_msg.dst_saddr.in = *((struct in_addr *)&sic.dest_ip_xlate);
    8654:	e51b309c 	ldr	r3, [fp, #-156]	@ 0xffffff64
						fc_msg.ethertype = AF_INET;
    8658:	e1c010b0 	strh	r1, [r0]
						fc_msg.src_saddr.in = *((struct in_addr *)&sic.src_ip);
    865c:	e50b2114 	str	r2, [fp, #-276]	@ 0xfffffeec
						fc_msg.dst_saddr.in = *((struct in_addr *)&sic.dest_ip_xlate);
    8660:	e50b3104 	str	r3, [fp, #-260]	@ 0xfffffefc
					fc_msg.proto = sic.protocol;
    8664:	e51bc0e4 	ldr	ip, [fp, #-228]	@ 0xffffff1c
					fast_classifier_send_genl_msg(FAST_CLASSIFIER_C_OFFLOADED, &fc_msg);
    8668:	e24b1f46 	sub	r1, fp, #280	@ 0x118
    866c:	e3a00002 	mov	r0, #2
					fc_msg.dport = sic.dest_port_xlate;
    8670:	e15b38b6 	ldrh	r3, [fp, #-134]	@ 0xffffff7a
					fc_msg.sport = sic.src_port;
    8674:	e15b28bc 	ldrh	r2, [fp, #-140]	@ 0xffffff74
					fc_msg.proto = sic.protocol;
    8678:	e54bc116 	strb	ip, [fp, #-278]	@ 0xfffffeea
					fc_msg.sport = sic.src_port;
    867c:	e14b2fb4 	strh	r2, [fp, #-244]	@ 0xffffff0c
					fc_msg.dport = sic.dest_port_xlate;
    8680:	e14b3fb2 	strh	r3, [fp, #-242]	@ 0xffffff0e
					memcpy(fc_msg.smac, conn->smac, ETH_ALEN);
    8684:	e597301d 	ldr	r3, [r7, #29]
    8688:	e50b30f0 	str	r3, [fp, #-240]	@ 0xffffff10
    868c:	e1d732b1 	ldrh	r3, [r7, #33]	@ 0x21
    8690:	e14b3ebc 	strh	r3, [fp, #-236]	@ 0xffffff14
					memcpy(fc_msg.dmac, conn->dmac, ETH_ALEN);
    8694:	e5973023 	ldr	r3, [r7, #35]	@ 0x23
    8698:	e50b30ea 	str	r3, [fp, #-234]	@ 0xffffff16
    869c:	e1d732b7 	ldrh	r3, [r7, #39]	@ 0x27
    86a0:	e14b3eb6 	strh	r3, [fp, #-230]	@ 0xffffff1a
					fast_classifier_send_genl_msg(FAST_CLASSIFIER_C_OFFLOADED, &fc_msg);
    86a4:	ebffe080 	bl	8ac <fast_classifier_send_genl_msg>
					conn->offloaded = 1;
    86a8:	e3a03001 	mov	r3, #1
    86ac:	e5873018 	str	r3, [r7, #24]
    86b0:	eaffff09 	b	82dc <fast_classifier_post_routing+0x4c>
    86b4:	e59f0814 	ldr	r0, [pc, #2068]	@ 8ed0 <fast_classifier_post_routing+0xc40>
    86b8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		if (conn->offloaded) {
    86bc:	e5973018 	ldr	r3, [r7, #24]
    86c0:	e3530000 	cmp	r3, #0
    86c4:	0affff04 	beq	82dc <fast_classifier_post_routing+0x4c>
			is_v4 ? sfe_ipv4_update_rule(conn->sic) : sfe_ipv6_update_rule(conn->sic);
    86c8:	e3550000 	cmp	r5, #0
    86cc:	e5974008 	ldr	r4, [r7, #8]
    86d0:	0a000136 	beq	8bb0 <fast_classifier_post_routing+0x920>
	raw_spin_lock_bh(&lock->rlock);
    86d4:	e3007000 	movw	r7, #0
    86d8:	e3407000 	movt	r7, #0
    86dc:	e1a00007 	mov	r0, r7
    86e0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	c = sfe_ipv4_find_sfe_ipv4_connection(si,
    86e4:	e1d415b8 	ldrh	r1, [r4, #88]	@ 0x58
    86e8:	e1d4c5bc 	ldrh	ip, [r4, #92]	@ 0x5c
					      sic->protocol,
    86ec:	e594e000 	ldr	lr, [r4]
	c = sfe_ipv4_find_sfe_ipv4_connection(si,
    86f0:	e5945018 	ldr	r5, [r4, #24]
    86f4:	e5946038 	ldr	r6, [r4, #56]	@ 0x38
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    86f8:	e021200c 	eor	r2, r1, ip
    86fc:	e6ef307e 	uxtb	r3, lr
    8700:	e6bf2fb2 	rev16	r2, r2
    8704:	e6ff2072 	uxth	r2, r2
    8708:	e0250006 	eor	r0, r5, r6
    870c:	e6bf0f30 	rev	r0, r0
    8710:	e0233000 	eor	r3, r3, r0
    8714:	e0233002 	eor	r3, r3, r2
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    8718:	e0233623 	eor	r3, r3, r3, lsr #12
    871c:	e7eb3053 	ubfx	r3, r3, #0, #12
	c = si->conn_hash[conn_idx];
    8720:	e283300c 	add	r3, r3, #12
    8724:	e7973103 	ldr	r3, [r7, r3, lsl #2]
	if (unlikely(!c)) {
    8728:	e3530000 	cmp	r3, #0
    872c:	1a000003 	bne	8740 <fast_classifier_post_routing+0x4b0>
    8730:	ea000013 	b	8784 <fast_classifier_post_routing+0x4f4>
		c = c->next;
    8734:	e5933000 	ldr	r3, [r3]
	} while (c && (c->src_port != src_port
    8738:	e3530000 	cmp	r3, #0
    873c:	0a000010 	beq	8784 <fast_classifier_post_routing+0x4f4>
	if ((c->src_port == src_port)
    8740:	e1d321bc 	ldrh	r2, [r3, #28]
    8744:	e1520001 	cmp	r2, r1
    8748:	1afffff9 	bne	8734 <fast_classifier_post_routing+0x4a4>
		 || c->dest_port != dest_port
    874c:	e1d322b0 	ldrh	r2, [r3, #32]
    8750:	e152000c 	cmp	r2, ip
    8754:	1afffff6 	bne	8734 <fast_classifier_post_routing+0x4a4>
		 || c->src_ip != src_ip
    8758:	e593200c 	ldr	r2, [r3, #12]
    875c:	e1550002 	cmp	r5, r2
    8760:	1afffff3 	bne	8734 <fast_classifier_post_routing+0x4a4>
		 || c->dest_ip != dest_ip
    8764:	e5932014 	ldr	r2, [r3, #20]
    8768:	e1560002 	cmp	r6, r2
    876c:	1afffff0 	bne	8734 <fast_classifier_post_routing+0x4a4>
		 || c->protocol != protocol));
    8770:	e5932008 	ldr	r2, [r3, #8]
    8774:	e15e0002 	cmp	lr, r2
    8778:	1affffed 	bne	8734 <fast_classifier_post_routing+0x4a4>
	switch (sic->protocol) {
    877c:	e35e0006 	cmp	lr, #6
    8780:	0a0001bb 	beq	8e74 <fast_classifier_post_routing+0xbe4>
	raw_spin_unlock_bh(&lock->rlock);
    8784:	e3000000 	movw	r0, #0
    8788:	e3400000 	movt	r0, #0
    878c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
    8790:	eafffed1 	b	82dc <fast_classifier_post_routing+0x4c>
    8794:	e59f0734 	ldr	r0, [pc, #1844]	@ 8ed0 <fast_classifier_post_routing+0xc40>
    8798:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (!fast_classifier_find_dev_and_mac_addr(NULL, &sic.src_ip, &src_dev_tmp, sic.src_mac, is_v4)) {
    879c:	e24b10cc 	sub	r1, fp, #204	@ 0xcc
    87a0:	e24b3084 	sub	r3, fp, #132	@ 0x84
    87a4:	e58d5000 	str	r5, [sp]
    87a8:	e24b2f5b 	sub	r2, fp, #364	@ 0x16c
    87ac:	e1a00007 	mov	r0, r7
    87b0:	ebffea14 	bl	3008 <fast_classifier_find_dev_and_mac_addr>
    87b4:	e3500000 	cmp	r0, #0
    87b8:	0afffec7 	beq	82dc <fast_classifier_post_routing+0x4c>
	if (!fast_classifier_find_dev_and_mac_addr(NULL, &sic.src_ip_xlate, &dev, sic.src_mac_xlate, is_v4)) {
    87bc:	e24b307e 	sub	r3, fp, #126	@ 0x7e
    87c0:	e24b2e17 	sub	r2, fp, #368	@ 0x170
    87c4:	e58d5000 	str	r5, [sp]
    87c8:	e24b10bc 	sub	r1, fp, #188	@ 0xbc
    87cc:	e1a00007 	mov	r0, r7
	src_dev = src_dev_tmp;
    87d0:	e51b916c 	ldr	r9, [fp, #-364]	@ 0xfffffe94
	if (!fast_classifier_find_dev_and_mac_addr(NULL, &sic.src_ip_xlate, &dev, sic.src_mac_xlate, is_v4)) {
    87d4:	ebffea0b 	bl	3008 <fast_classifier_find_dev_and_mac_addr>
    87d8:	e3500000 	cmp	r0, #0
    87dc:	0a0000af 	beq	8aa0 <fast_classifier_post_routing+0x810>
	dev_put(dev);
    87e0:	e51b3170 	ldr	r3, [fp, #-368]	@ 0xfffffe90
	if (dev) {
    87e4:	e3530000 	cmp	r3, #0
    87e8:	0a000007 	beq	880c <fast_classifier_post_routing+0x57c>
	asm volatile(
    87ec:	e10f0000 	mrs	r0, CPSR
    87f0:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    87f4:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    87f8:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    87fc:	e7932001 	ldr	r2, [r3, r1]
    8800:	e2422001 	sub	r2, r2, #1
    8804:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    8808:	e121f000 	msr	CPSR_c, r0
	struct sk_buff *tmp_skb = NULL;
    880c:	e3550000 	cmp	r5, #0
	if (!fast_classifier_find_dev_and_mac_addr(tmp_skb, &sic.dest_ip, &dev, sic.dest_mac, is_v4)) {
    8810:	e24b2e17 	sub	r2, fp, #368	@ 0x170
    8814:	e58d5000 	str	r5, [sp]
    8818:	01a00004 	moveq	r0, r4
    881c:	13a00000 	movne	r0, #0
    8820:	e24b3078 	sub	r3, fp, #120	@ 0x78
    8824:	e24b10ac 	sub	r1, fp, #172	@ 0xac
    8828:	ebffe9f6 	bl	3008 <fast_classifier_find_dev_and_mac_addr>
    882c:	e3500000 	cmp	r0, #0
    8830:	0a00009a 	beq	8aa0 <fast_classifier_post_routing+0x810>
	dev_put(dev);
    8834:	e51b3170 	ldr	r3, [fp, #-368]	@ 0xfffffe90
	if (dev) {
    8838:	e3530000 	cmp	r3, #0
    883c:	0a000007 	beq	8860 <fast_classifier_post_routing+0x5d0>
	asm volatile(
    8840:	e10f0000 	mrs	r0, CPSR
    8844:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    8848:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    884c:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8850:	e7932001 	ldr	r2, [r3, r1]
    8854:	e2422001 	sub	r2, r2, #1
    8858:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    885c:	e121f000 	msr	CPSR_c, r0
	return is_v4 ? sfe_ipv4_addr_equal(a->ip, b->ip) : sfe_ipv6_addr_equal(a->ip6, b->ip6);
    8860:	e3550000 	cmp	r5, #0
    8864:	0a00014e 	beq	8da4 <fast_classifier_post_routing+0xb14>
	if (likely(sfe_addr_equal(&sic.dest_ip_xlate, &sic.dest_ip, is_v4)))
    8868:	e51b30ac 	ldr	r3, [fp, #-172]	@ 0xffffff54
    886c:	e51b209c 	ldr	r2, [fp, #-156]	@ 0xffffff64
    8870:	e1520003 	cmp	r2, r3
		tmp_skb = NULL;
    8874:	13a00000 	movne	r0, #0
    8878:	124b809c 	subne	r8, fp, #156	@ 0x9c
	if (likely(sfe_addr_equal(&sic.dest_ip_xlate, &sic.dest_ip, is_v4)))
    887c:	1a000001 	bne	8888 <fast_classifier_post_routing+0x5f8>
		tmp_skb = skb;
    8880:	e1a00004 	mov	r0, r4
    8884:	e24b809c 	sub	r8, fp, #156	@ 0x9c
	if (!fast_classifier_find_dev_and_mac_addr(tmp_skb, &sic.dest_ip_xlate, &dest_dev_tmp, sic.dest_mac_xlate, is_v4)) {
    8888:	e1a01008 	mov	r1, r8
    888c:	e24b2f46 	sub	r2, fp, #280	@ 0x118
    8890:	e58d5000 	str	r5, [sp]
    8894:	e24b3072 	sub	r3, fp, #114	@ 0x72
    8898:	ebffe9da 	bl	3008 <fast_classifier_find_dev_and_mac_addr>
    889c:	e3500000 	cmp	r0, #0
    88a0:	0a00007e 	beq	8aa0 <fast_classifier_post_routing+0x810>
	if (src_dev->priv_flags & IFF_BRIDGE_PORT) {
    88a4:	e5993070 	ldr	r3, [r9, #112]	@ 0x70
	dest_dev = dest_dev_tmp;
    88a8:	e51ba118 	ldr	sl, [fp, #-280]	@ 0xfffffee8
	if (src_dev->priv_flags & IFF_BRIDGE_PORT) {
    88ac:	e3130c02 	tst	r3, #512	@ 0x200
    88b0:	e2033c02 	and	r3, r3, #512	@ 0x200
	struct net_device *src_br_dev = NULL;
    88b4:	050b3178 	streq	r3, [fp, #-376]	@ 0xfffffe88
	if (src_dev->priv_flags & IFF_BRIDGE_PORT) {
    88b8:	1a00017e 	bne	8eb8 <fast_classifier_post_routing+0xc28>
	if (dest_dev->priv_flags & IFF_BRIDGE_PORT) {
    88bc:	e59a3070 	ldr	r3, [sl, #112]	@ 0x70
    88c0:	e3130c02 	tst	r3, #512	@ 0x200
    88c4:	0a00013e 	beq	8dc4 <fast_classifier_post_routing+0xb34>
		dest_br_dev = sfe_dev_get_master(dest_dev);
    88c8:	e1a0000a 	mov	r0, sl
    88cc:	ebffdfdc 	bl	844 <sfe_dev_get_master>
		if (!dest_br_dev) {
    88d0:	e2507000 	subs	r7, r0, #0
    88d4:	0a00005b 	beq	8a48 <fast_classifier_post_routing+0x7b8>
	sic.src_mtu = src_dev->mtu;
    88d8:	e5993084 	ldr	r3, [r9, #132]	@ 0x84
	sic.src_dev = src_dev;
    88dc:	e50b90e0 	str	r9, [fp, #-224]	@ 0xffffff20
    88e0:	e3009000 	movw	r9, #0
	sic.mark = skb->mark;
    88e4:	e5942090 	ldr	r2, [r4, #144]	@ 0x90
    88e8:	e3409000 	movt	r9, #0
    88ec:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    88f0:	e5990018 	ldr	r0, [r9, #24]
	sic.dest_dev = dest_dev;
    88f4:	e50b70dc 	str	r7, [fp, #-220]	@ 0xffffff24
	sic.src_mtu = src_dev->mtu;
    88f8:	e50b30d4 	str	r3, [fp, #-212]	@ 0xffffff2c
	sic.dest_mtu = dest_dev->mtu;
    88fc:	e5973084 	ldr	r3, [r7, #132]	@ 0x84
	sic.mark = skb->mark;
    8900:	e50b204c 	str	r2, [fp, #-76]	@ 0xffffffb4
    8904:	e3a0202c 	mov	r2, #44	@ 0x2c
	sic.dest_mtu = dest_dev->mtu;
    8908:	e50b30d0 	str	r3, [fp, #-208]	@ 0xffffff30
    890c:	ebfffffe 	bl	0 <kmalloc_trace>
	if (!conn) {
    8910:	e2504000 	subs	r4, r0, #0
    8914:	0a000043 	beq	8a28 <fast_classifier_post_routing+0x798>
	memcpy(conn->smac, sic.src_mac, ETH_ALEN);
    8918:	e15b38b0 	ldrh	r3, [fp, #-128]	@ 0xffffff80
	conn->hits = 0;
    891c:	e3a08000 	mov	r8, #0
    8920:	e3a020b4 	mov	r2, #180	@ 0xb4
    8924:	e5990008 	ldr	r0, [r9, #8]
    8928:	e3a09000 	mov	r9, #0
    892c:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    8930:	e1c481f0 	strd	r8, [r4, #16]
	conn->is_v4 = is_v4;
    8934:	e5c4501c 	strb	r5, [r4, #28]
	memcpy(conn->smac, sic.src_mac, ETH_ALEN);
    8938:	e1c432b1 	strh	r3, [r4, #33]	@ 0x21
	memcpy(conn->dmac, sic.dest_mac_xlate, ETH_ALEN);
    893c:	e15b36be 	ldrh	r3, [fp, #-110]	@ 0xffffff92
    8940:	e1c432b7 	strh	r3, [r4, #39]	@ 0x27
	memcpy(conn->smac, sic.src_mac, ETH_ALEN);
    8944:	e51b3084 	ldr	r3, [fp, #-132]	@ 0xffffff7c
    8948:	e584301d 	str	r3, [r4, #29]
	memcpy(conn->dmac, sic.dest_mac_xlate, ETH_ALEN);
    894c:	e51b3072 	ldr	r3, [fp, #-114]	@ 0xffffff8e
    8950:	e5843023 	str	r3, [r4, #35]	@ 0x23
	conn->offloaded = 0;
    8954:	e3a03000 	mov	r3, #0
    8958:	e5843018 	str	r3, [r4, #24]
    895c:	ebfffffe 	bl	0 <kmalloc_trace>
	if (!p_sic) {
    8960:	e2505000 	subs	r5, r0, #0
    8964:	0a000150 	beq	8eac <fast_classifier_post_routing+0xc1c>
	memcpy(p_sic, &sic, sizeof(sic));
    8968:	e24b10e4 	sub	r1, fp, #228	@ 0xe4
    896c:	e3a020b4 	mov	r2, #180	@ 0xb4
    8970:	ebfffffe 	bl	0 <memcpy>
	raw_spin_lock_bh(&lock->rlock);
    8974:	e59f0554 	ldr	r0, [pc, #1364]	@ 8ed0 <fast_classifier_post_routing+0xc40>
	conn->sic = p_sic;
    8978:	e5845008 	str	r5, [r4, #8]
	conn->ct = ct;
    897c:	e584600c 	str	r6, [r4, #12]
    8980:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	if (conn->is_v4) {
    8984:	e5d4101c 	ldrb	r1, [r4, #28]
		if (fast_classifier_find_conn(&sic->src_ip, &sic->dest_ip, sic->src_port,
    8988:	e1d525b8 	ldrh	r2, [r5, #88]	@ 0x58
	if (conn->is_v4) {
    898c:	e3510000 	cmp	r1, #0
    8990:	0a00011c 	beq	8e08 <fast_classifier_post_routing+0xb78>
		if (fast_classifier_find_conn(&sic->src_ip, &sic->dest_ip, sic->src_port,
    8994:	e3a01001 	mov	r1, #1
    8998:	e1a00005 	mov	r0, r5
    899c:	e1d535bc 	ldrh	r3, [r5, #92]	@ 0x5c
    89a0:	e58d1004 	str	r1, [sp, #4]
    89a4:	e4d0c018 	ldrb	ip, [r0], #24
    89a8:	e2851038 	add	r1, r5, #56	@ 0x38
    89ac:	e58dc000 	str	ip, [sp]
    89b0:	ebffebe4 	bl	3948 <fast_classifier_find_conn>
    89b4:	e3500000 	cmp	r0, #0
    89b8:	1a000137 	bne	8e9c <fast_classifier_post_routing+0xc0c>
	u32 hash = saddr->ip ^ daddr->ip;
    89bc:	e5951038 	ldr	r1, [r5, #56]	@ 0x38
	return hash ^ (sport | (dport << 16));
    89c0:	e1822803 	orr	r2, r2, r3, lsl #16
	u32 hash = saddr->ip ^ daddr->ip;
    89c4:	e5953018 	ldr	r3, [r5, #24]
    89c8:	e0233001 	eor	r3, r3, r1
	return hash ^ (sport | (dport << 16));
    89cc:	e0233002 	eor	r3, r3, r2
	return val * GOLDEN_RATIO_32;
    89d0:	e3081647 	movw	r1, #34375	@ 0x8647
	hash_add(fc_conn_ht, &conn->hl, key);
    89d4:	e3002000 	movw	r2, #0
    89d8:	e34611c8 	movt	r1, #25032	@ 0x61c8
    89dc:	e3402000 	movt	r2, #0
    89e0:	e0030391 	mul	r3, r1, r3
	return __hash_32(val) >> (32 - bits);
    89e4:	e1a039a3 	lsr	r3, r3, #19
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
    89e8:	e7921103 	ldr	r1, [r2, r3, lsl #2]
    89ec:	e0820103 	add	r0, r2, r3, lsl #2
	WRITE_ONCE(n->next, first);
	if (first)
    89f0:	e3510000 	cmp	r1, #0
	WRITE_ONCE(n->next, first);
    89f4:	e5841000 	str	r1, [r4]
		WRITE_ONCE(first->pprev, &n->next);
    89f8:	15814004 	strne	r4, [r1, #4]
	WRITE_ONCE(h->first, n);
    89fc:	e7824103 	str	r4, [r2, r3, lsl #2]
	sfe_connections_size++;
    8a00:	e3003000 	movw	r3, #0
    8a04:	e3403000 	movt	r3, #0
	WRITE_ONCE(n->pprev, &h->first);
    8a08:	e5840004 	str	r0, [r4, #4]
    8a0c:	e5932020 	ldr	r2, [r3, #32]
	raw_spin_unlock_bh(&lock->rlock);
    8a10:	e2830014 	add	r0, r3, #20
    8a14:	e2822001 	add	r2, r2, #1
    8a18:	e5832020 	str	r2, [r3, #32]
    8a1c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (dest_br_dev) {
    8a20:	e3570000 	cmp	r7, #0
    8a24:	0a000007 	beq	8a48 <fast_classifier_post_routing+0x7b8>
	asm volatile(
    8a28:	e10f0000 	mrs	r0, CPSR
    8a2c:	f10c0080 	cpsid	i
    8a30:	e5973310 	ldr	r3, [r7, #784]	@ 0x310
    8a34:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8a38:	e7932001 	ldr	r2, [r3, r1]
    8a3c:	e2422001 	sub	r2, r2, #1
    8a40:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    8a44:	e121f000 	msr	CPSR_c, r0
	if (src_br_dev) {
    8a48:	e51b3178 	ldr	r3, [fp, #-376]	@ 0xfffffe88
    8a4c:	e3530000 	cmp	r3, #0
    8a50:	0a000007 	beq	8a74 <fast_classifier_post_routing+0x7e4>
	asm volatile(
    8a54:	e10f0000 	mrs	r0, CPSR
    8a58:	f10c0080 	cpsid	i
    8a5c:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    8a60:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8a64:	e7932001 	ldr	r2, [r3, r1]
    8a68:	e2422001 	sub	r2, r2, #1
    8a6c:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    8a70:	e121f000 	msr	CPSR_c, r0
	dev_put(dest_dev_tmp);
    8a74:	e51b3118 	ldr	r3, [fp, #-280]	@ 0xfffffee8
	if (dev) {
    8a78:	e3530000 	cmp	r3, #0
    8a7c:	0a000007 	beq	8aa0 <fast_classifier_post_routing+0x810>
	asm volatile(
    8a80:	e10f0000 	mrs	r0, CPSR
    8a84:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    8a88:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    8a8c:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8a90:	e7932001 	ldr	r2, [r3, r1]
    8a94:	e2422001 	sub	r2, r2, #1
    8a98:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    8a9c:	e121f000 	msr	CPSR_c, r0
	dev_put(src_dev_tmp);
    8aa0:	e51b316c 	ldr	r3, [fp, #-364]	@ 0xfffffe94
	if (dev) {
    8aa4:	e3530000 	cmp	r3, #0
    8aa8:	0afffe0b 	beq	82dc <fast_classifier_post_routing+0x4c>
	asm volatile(
    8aac:	e10f0000 	mrs	r0, CPSR
    8ab0:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    8ab4:	e5933310 	ldr	r3, [r3, #784]	@ 0x310
    8ab8:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8abc:	e7932001 	ldr	r2, [r3, r1]
    8ac0:	e2422001 	sub	r2, r2, #1
    8ac4:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    8ac8:	e121f000 	msr	CPSR_c, r0
}
    8acc:	eafffe02 	b	82dc <fast_classifier_post_routing+0x4c>
				ret = is_v4 ? sfe_ipv4_create_rule(conn->sic) : sfe_ipv6_create_rule(conn->sic);
    8ad0:	ebfffb83 	bl	78e4 <sfe_ipv6_create_rule>
				if ((ret == 0) || (ret == -EADDRINUSE)) {
    8ad4:	e3700062 	cmn	r0, #98	@ 0x62
    8ad8:	13500000 	cmpne	r0, #0
    8adc:	1afffdfe 	bne	82dc <fast_classifier_post_routing+0x4c>
						fc_msg.src_saddr.in6 = *((struct in6_addr *)&sic.src_ip);
    8ae0:	e14b0cdc 	ldrd	r0, [fp, #-204]	@ 0xffffff34
    8ae4:	e24bcf43 	sub	ip, fp, #268	@ 0x10c
    8ae8:	e24bef45 	sub	lr, fp, #276	@ 0x114
    8aec:	e14b4cd4 	ldrd	r4, [fp, #-196]	@ 0xffffff3c
						fc_msg.dst_saddr.in6 = *((struct in6_addr *)&sic.dest_ip_xlate);
    8af0:	e14b29dc 	ldrd	r2, [fp, #-156]	@ 0xffffff64
						fc_msg.src_saddr.in6 = *((struct in6_addr *)&sic.src_ip);
    8af4:	e1cc40f0 	strd	r4, [ip]
						fc_msg.ethertype = AF_INET6;
    8af8:	e3a0c00a 	mov	ip, #10
						fc_msg.src_saddr.in6 = *((struct in6_addr *)&sic.src_ip);
    8afc:	e1ce00f0 	strd	r0, [lr]
						fc_msg.dst_saddr.in6 = *((struct in6_addr *)&sic.dest_ip_xlate);
    8b00:	e24b1f41 	sub	r1, fp, #260	@ 0x104
    8b04:	e1c120f0 	strd	r2, [r1]
    8b08:	e14b29d4 	ldrd	r2, [fp, #-148]	@ 0xffffff6c
						fc_msg.ethertype = AF_INET6;
    8b0c:	e24b1f46 	sub	r1, fp, #280	@ 0x118
    8b10:	e1c1c0b0 	strh	ip, [r1]
						fc_msg.dst_saddr.in6 = *((struct in6_addr *)&sic.dest_ip_xlate);
    8b14:	e14b2ffc 	strd	r2, [fp, #-252]	@ 0xffffff04
    8b18:	eafffed1 	b	8664 <fast_classifier_post_routing+0x3d4>
		p_sic->src_td_window_scale = ct->proto.tcp.seen[0].td_scale;
    8b1c:	e5d420a0 	ldrb	r2, [r4, #160]	@ 0xa0
    8b20:	e5c32078 	strb	r2, [r3, #120]	@ 0x78
		p_sic->src_td_max_window = ct->proto.tcp.seen[0].td_maxwin;
    8b24:	e5942098 	ldr	r2, [r4, #152]	@ 0x98
    8b28:	e583207c 	str	r2, [r3, #124]	@ 0x7c
		p_sic->src_td_end = ct->proto.tcp.seen[0].td_end;
    8b2c:	e5942090 	ldr	r2, [r4, #144]	@ 0x90
    8b30:	e5832080 	str	r2, [r3, #128]	@ 0x80
		p_sic->src_td_max_end = ct->proto.tcp.seen[0].td_maxend;
    8b34:	e5942094 	ldr	r2, [r4, #148]	@ 0x94
    8b38:	e5832084 	str	r2, [r3, #132]	@ 0x84
		p_sic->dest_td_window_scale = ct->proto.tcp.seen[1].td_scale;
    8b3c:	e5d420b4 	ldrb	r2, [r4, #180]	@ 0xb4
    8b40:	e5c32088 	strb	r2, [r3, #136]	@ 0x88
		p_sic->dest_td_max_window = ct->proto.tcp.seen[1].td_maxwin;
    8b44:	e59420ac 	ldr	r2, [r4, #172]	@ 0xac
    8b48:	e583208c 	str	r2, [r3, #140]	@ 0x8c
		p_sic->dest_td_end = ct->proto.tcp.seen[1].td_end;
    8b4c:	e59420a4 	ldr	r2, [r4, #164]	@ 0xa4
    8b50:	e5832090 	str	r2, [r3, #144]	@ 0x90
		p_sic->dest_td_max_end = ct->proto.tcp.seen[1].td_maxend;
    8b54:	e59420a8 	ldr	r2, [r4, #168]	@ 0xa8
    8b58:	e5832094 	str	r2, [r3, #148]	@ 0x94
		if ((ct->proto.tcp.seen[0].flags & IP_CT_TCP_FLAG_BE_LIBERAL)
    8b5c:	e5d420a1 	ldrb	r2, [r4, #161]	@ 0xa1
    8b60:	e3120008 	tst	r2, #8
    8b64:	1a000002 	bne	8b74 <fast_classifier_post_routing+0x8e4>
		    || (ct->proto.tcp.seen[1].flags & IP_CT_TCP_FLAG_BE_LIBERAL)) {
    8b68:	e5d420b5 	ldrb	r2, [r4, #181]	@ 0xb5
    8b6c:	e3120008 	tst	r2, #8
    8b70:	0a000002 	beq	8b80 <fast_classifier_post_routing+0x8f0>
			p_sic->flags |= SFE_CREATE_FLAG_NO_SEQ_CHECK;
    8b74:	e593200c 	ldr	r2, [r3, #12]
    8b78:	e3822001 	orr	r2, r2, #1
    8b7c:	e583200c 	str	r2, [r3, #12]
	raw_spin_lock(&lock->rlock);
    8b80:	e2840004 	add	r0, r4, #4
    8b84:	ebfffffe 	bl	0 <_raw_spin_lock>
		if (ct->proto.tcp.state != TCP_CONNTRACK_ESTABLISHED) {
    8b88:	e5d430b8 	ldrb	r3, [r4, #184]	@ 0xb8
    8b8c:	e3530003 	cmp	r3, #3
    8b90:	0afffe9d 	beq	860c <fast_classifier_post_routing+0x37c>
	smp_mb();
    8b94:	f57ff05b 	dmb	ish
	lock->tickets.owner++;
    8b98:	e1d430b4 	ldrh	r3, [r4, #4]
    8b9c:	e2833001 	add	r3, r3, #1
    8ba0:	e1c430b4 	strh	r3, [r4, #4]
	dsb(ishst);
    8ba4:	f57ff04a 	dsb	ishst
	__asm__(SEV);
    8ba8:	e320f004 	sev
			return 0;
    8bac:	eafffe6d 	b	8568 <fast_classifier_post_routing+0x2d8>
	raw_spin_lock_bh(&lock->rlock);
    8bb0:	e3006000 	movw	r6, #0
    8bb4:	e3406000 	movt	r6, #0
    8bb8:	e1a00006 	mov	r0, r6
    8bbc:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	c = sfe_ipv6_find_connection(si,
    8bc0:	e1d435b8 	ldrh	r3, [r4, #88]	@ 0x58
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    8bc4:	e5945018 	ldr	r5, [r4, #24]
    8bc8:	e594a038 	ldr	sl, [r4, #56]	@ 0x38
    8bcc:	e5942040 	ldr	r2, [r4, #64]	@ 0x40
	c = sfe_ipv6_find_connection(si,
    8bd0:	e50b3178 	str	r3, [fp, #-376]	@ 0xfffffe88
    8bd4:	e1d4c5bc 	ldrh	ip, [r4, #92]	@ 0x5c
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    8bd8:	e594901c 	ldr	r9, [r4, #28]
    8bdc:	e594803c 	ldr	r8, [r4, #60]	@ 0x3c
    8be0:	e5941020 	ldr	r1, [r4, #32]
    8be4:	e1a03002 	mov	r3, r2
    8be8:	e025200a 	eor	r2, r5, sl
				     sic->protocol,
    8bec:	e594e000 	ldr	lr, [r4]
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    8bf0:	e0222009 	eor	r2, r2, r9
    8bf4:	e5940024 	ldr	r0, [r4, #36]	@ 0x24
    8bf8:	e0222008 	eor	r2, r2, r8
    8bfc:	e5947044 	ldr	r7, [r4, #68]	@ 0x44
    8c00:	e0222001 	eor	r2, r2, r1
    8c04:	e50b1180 	str	r1, [fp, #-384]	@ 0xfffffe80
    8c08:	e0222003 	eor	r2, r2, r3
    8c0c:	e50b317c 	str	r3, [fp, #-380]	@ 0xfffffe84
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    8c10:	e51b3178 	ldr	r3, [fp, #-376]	@ 0xfffffe88
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    8c14:	e0222000 	eor	r2, r2, r0
    8c18:	e0222007 	eor	r2, r2, r7
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    8c1c:	e023100c 	eor	r1, r3, ip
    8c20:	e6ef307e 	uxtb	r3, lr
    8c24:	e6bf1fb1 	rev16	r1, r1
    8c28:	e6ff1071 	uxth	r1, r1
    8c2c:	e0222003 	eor	r2, r2, r3
    8c30:	e0211002 	eor	r1, r1, r2
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    8c34:	e0211621 	eor	r1, r1, r1, lsr #12
    8c38:	e7eb2051 	ubfx	r2, r1, #0, #12
	c = si->conn_hash[conn_idx];
    8c3c:	e282200c 	add	r2, r2, #12
    8c40:	e7962102 	ldr	r2, [r6, r2, lsl #2]
	if (unlikely(!c)) {
    8c44:	e3520000 	cmp	r2, #0
    8c48:	0a000030 	beq	8d10 <fast_classifier_post_routing+0xa80>
	if ((c->src_port == src_port)
    8c4c:	e51b3178 	ldr	r3, [fp, #-376]	@ 0xfffffe88
    8c50:	e1d214bc 	ldrh	r1, [r2, #76]	@ 0x4c
    8c54:	e1510003 	cmp	r1, r3
    8c58:	0a000030 	beq	8d20 <fast_classifier_post_routing+0xa90>
		c = c->next;
    8c5c:	e5922000 	ldr	r2, [r2]
	} while (c && (c->src_port != src_port
    8c60:	e3520000 	cmp	r2, #0
    8c64:	151b3178 	ldrne	r3, [fp, #-376]	@ 0xfffffe88
    8c68:	1a000003 	bne	8c7c <fast_classifier_post_routing+0x9ec>
    8c6c:	ea000027 	b	8d10 <fast_classifier_post_routing+0xa80>
		c = c->next;
    8c70:	e5922000 	ldr	r2, [r2]
	} while (c && (c->src_port != src_port
    8c74:	e3520000 	cmp	r2, #0
    8c78:	0a000024 	beq	8d10 <fast_classifier_post_routing+0xa80>
    8c7c:	e1d214bc 	ldrh	r1, [r2, #76]	@ 0x4c
    8c80:	e1510003 	cmp	r1, r3
    8c84:	1afffff9 	bne	8c70 <fast_classifier_post_routing+0x9e0>
		 || c->dest_port != dest_port
    8c88:	e1d215b0 	ldrh	r1, [r2, #80]	@ 0x50
    8c8c:	e151000c 	cmp	r1, ip
    8c90:	1afffff6 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	       a->addr[2] == b->addr[2] &&
    8c94:	e592100c 	ldr	r1, [r2, #12]
    8c98:	e1550001 	cmp	r5, r1
    8c9c:	1afffff3 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	return a->addr[0] == b->addr[0] &&
    8ca0:	e5921010 	ldr	r1, [r2, #16]
    8ca4:	e1510009 	cmp	r1, r9
    8ca8:	1afffff0 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	       a->addr[1] == b->addr[1] &&
    8cac:	e51b6180 	ldr	r6, [fp, #-384]	@ 0xfffffe80
    8cb0:	e5921014 	ldr	r1, [r2, #20]
    8cb4:	e1560001 	cmp	r6, r1
    8cb8:	1affffec 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	       a->addr[2] == b->addr[2] &&
    8cbc:	e5921018 	ldr	r1, [r2, #24]
    8cc0:	e1500001 	cmp	r0, r1
    8cc4:	1affffe9 	bne	8c70 <fast_classifier_post_routing+0x9e0>
    8cc8:	e592102c 	ldr	r1, [r2, #44]	@ 0x2c
    8ccc:	e15a0001 	cmp	sl, r1
    8cd0:	1affffe6 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	return a->addr[0] == b->addr[0] &&
    8cd4:	e5921030 	ldr	r1, [r2, #48]	@ 0x30
    8cd8:	e1510008 	cmp	r1, r8
    8cdc:	1affffe3 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	       a->addr[1] == b->addr[1] &&
    8ce0:	e51b617c 	ldr	r6, [fp, #-380]	@ 0xfffffe84
    8ce4:	e5921034 	ldr	r1, [r2, #52]	@ 0x34
    8ce8:	e1560001 	cmp	r6, r1
    8cec:	1affffdf 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	       a->addr[2] == b->addr[2] &&
    8cf0:	e5921038 	ldr	r1, [r2, #56]	@ 0x38
    8cf4:	e1570001 	cmp	r7, r1
    8cf8:	1affffdc 	bne	8c70 <fast_classifier_post_routing+0x9e0>
		 || c->protocol != protocol));
    8cfc:	e5921008 	ldr	r1, [r2, #8]
    8d00:	e15e0001 	cmp	lr, r1
    8d04:	1affffd9 	bne	8c70 <fast_classifier_post_routing+0x9e0>
	switch (sic->protocol) {
    8d08:	e35e0006 	cmp	lr, #6
    8d0c:	0a00005d 	beq	8e88 <fast_classifier_post_routing+0xbf8>
	raw_spin_unlock_bh(&lock->rlock);
    8d10:	e3000000 	movw	r0, #0
    8d14:	e3400000 	movt	r0, #0
    8d18:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
}
    8d1c:	eafffd6e 	b	82dc <fast_classifier_post_routing+0x4c>
	    && (c->dest_port == dest_port)
    8d20:	e1d215b0 	ldrh	r1, [r2, #80]	@ 0x50
    8d24:	e151000c 	cmp	r1, ip
    8d28:	1affffcb 	bne	8c5c <fast_classifier_post_routing+0x9cc>
    8d2c:	e592100c 	ldr	r1, [r2, #12]
    8d30:	e1550001 	cmp	r5, r1
    8d34:	1affffc8 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	return a->addr[0] == b->addr[0] &&
    8d38:	e5921010 	ldr	r1, [r2, #16]
    8d3c:	e1510009 	cmp	r1, r9
    8d40:	1affffc5 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	       a->addr[1] == b->addr[1] &&
    8d44:	e51b3180 	ldr	r3, [fp, #-384]	@ 0xfffffe80
    8d48:	e5921014 	ldr	r1, [r2, #20]
    8d4c:	e1530001 	cmp	r3, r1
    8d50:	1affffc1 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	       a->addr[2] == b->addr[2] &&
    8d54:	e5921018 	ldr	r1, [r2, #24]
    8d58:	e1500001 	cmp	r0, r1
    8d5c:	1affffbe 	bne	8c5c <fast_classifier_post_routing+0x9cc>
    8d60:	e592102c 	ldr	r1, [r2, #44]	@ 0x2c
    8d64:	e15a0001 	cmp	sl, r1
    8d68:	1affffbb 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	return a->addr[0] == b->addr[0] &&
    8d6c:	e5921030 	ldr	r1, [r2, #48]	@ 0x30
    8d70:	e1510008 	cmp	r1, r8
    8d74:	1affffb8 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	       a->addr[1] == b->addr[1] &&
    8d78:	e51b317c 	ldr	r3, [fp, #-380]	@ 0xfffffe84
    8d7c:	e5921034 	ldr	r1, [r2, #52]	@ 0x34
    8d80:	e1530001 	cmp	r3, r1
    8d84:	1affffb4 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	       a->addr[2] == b->addr[2] &&
    8d88:	e5921038 	ldr	r1, [r2, #56]	@ 0x38
    8d8c:	e1570001 	cmp	r7, r1
    8d90:	1affffb1 	bne	8c5c <fast_classifier_post_routing+0x9cc>
	    && (c->protocol == protocol)) {
    8d94:	e5921008 	ldr	r1, [r2, #8]
    8d98:	e15e0001 	cmp	lr, r1
    8d9c:	1affffae 	bne	8c5c <fast_classifier_post_routing+0x9cc>
    8da0:	eaffffd8 	b	8d08 <fast_classifier_post_routing+0xa78>
    8da4:	e24b809c 	sub	r8, fp, #156	@ 0x9c
    8da8:	e24b10ac 	sub	r1, fp, #172	@ 0xac
    8dac:	e1a00008 	mov	r0, r8
    8db0:	ebffe02c 	bl	e68 <sfe_addr_equal.part.0>
		tmp_skb = NULL;
    8db4:	e3500000 	cmp	r0, #0
    8db8:	11a00004 	movne	r0, r4
    8dbc:	03a00000 	moveq	r0, #0
    8dc0:	eafffeb0 	b	8888 <fast_classifier_post_routing+0x5f8>
	sic.src_mtu = src_dev->mtu;
    8dc4:	e5993084 	ldr	r3, [r9, #132]	@ 0x84
	sic.src_dev = src_dev;
    8dc8:	e50b90e0 	str	r9, [fp, #-224]	@ 0xffffff20
    8dcc:	e3009000 	movw	r9, #0
	sic.mark = skb->mark;
    8dd0:	e5942090 	ldr	r2, [r4, #144]	@ 0x90
    8dd4:	e3409000 	movt	r9, #0
    8dd8:	e3a01ea2 	mov	r1, #2592	@ 0xa20
    8ddc:	e5990018 	ldr	r0, [r9, #24]
	sic.dest_dev = dest_dev;
    8de0:	e50ba0dc 	str	sl, [fp, #-220]	@ 0xffffff24
	sic.src_mtu = src_dev->mtu;
    8de4:	e50b30d4 	str	r3, [fp, #-212]	@ 0xffffff2c
	sic.dest_mtu = dest_dev->mtu;
    8de8:	e59a3084 	ldr	r3, [sl, #132]	@ 0x84
	sic.mark = skb->mark;
    8dec:	e50b204c 	str	r2, [fp, #-76]	@ 0xffffffb4
    8df0:	e3a0202c 	mov	r2, #44	@ 0x2c
	sic.dest_mtu = dest_dev->mtu;
    8df4:	e50b30d0 	str	r3, [fp, #-208]	@ 0xffffff30
    8df8:	ebfffffe 	bl	0 <kmalloc_trace>
	if (!conn) {
    8dfc:	e2504000 	subs	r4, r0, #0
    8e00:	1afffec4 	bne	8918 <fast_classifier_post_routing+0x688>
    8e04:	eaffff0f 	b	8a48 <fast_classifier_post_routing+0x7b8>
		if (fast_classifier_find_conn(&sic->src_ip, &sic->dest_ip, sic->src_port,
    8e08:	e1a00005 	mov	r0, r5
    8e0c:	e1d535bc 	ldrh	r3, [r5, #92]	@ 0x5c
    8e10:	e58d1004 	str	r1, [sp, #4]
    8e14:	e4d01018 	ldrb	r1, [r0], #24
    8e18:	e58d1000 	str	r1, [sp]
    8e1c:	e2851038 	add	r1, r5, #56	@ 0x38
    8e20:	ebffeac8 	bl	3948 <fast_classifier_find_conn>
    8e24:	e3500000 	cmp	r0, #0
    8e28:	1a00001b 	bne	8e9c <fast_classifier_post_routing+0xc0c>
	hash ^= saddr->ip6[0].addr[3] ^ daddr->ip6[0].addr[3];
    8e2c:	e5951038 	ldr	r1, [r5, #56]	@ 0x38
	return hash ^ (sport | (dport << 16));
    8e30:	e1822803 	orr	r2, r2, r3, lsl #16
	hash ^= saddr->ip6[0].addr[3] ^ daddr->ip6[0].addr[3];
    8e34:	e5953018 	ldr	r3, [r5, #24]
    8e38:	e0233001 	eor	r3, r3, r1
    8e3c:	e595101c 	ldr	r1, [r5, #28]
    8e40:	e0233001 	eor	r3, r3, r1
    8e44:	e595103c 	ldr	r1, [r5, #60]	@ 0x3c
    8e48:	e0233001 	eor	r3, r3, r1
    8e4c:	e5951020 	ldr	r1, [r5, #32]
    8e50:	e0233001 	eor	r3, r3, r1
    8e54:	e5951040 	ldr	r1, [r5, #64]	@ 0x40
    8e58:	e0233001 	eor	r3, r3, r1
    8e5c:	e5951024 	ldr	r1, [r5, #36]	@ 0x24
    8e60:	e0233001 	eor	r3, r3, r1
    8e64:	e5951044 	ldr	r1, [r5, #68]	@ 0x44
    8e68:	e0233001 	eor	r3, r3, r1
	return hash ^ (sport | (dport << 16));
    8e6c:	e0233002 	eor	r3, r3, r2
    8e70:	eafffed6 	b	89d0 <fast_classifier_post_routing+0x740>
		sfe_ipv4_update_tcp_state(c, sic);
    8e74:	e5930024 	ldr	r0, [r3, #36]	@ 0x24
    8e78:	e1a02004 	mov	r2, r4
    8e7c:	e593102c 	ldr	r1, [r3, #44]	@ 0x2c
    8e80:	ebffe2c6 	bl	19a0 <sfe_ipv4_update_tcp_state.isra.0>
		break;
    8e84:	eafffe3e 	b	8784 <fast_classifier_post_routing+0x4f4>
		sfe_ipv6_update_tcp_state(c, sic);
    8e88:	e5920054 	ldr	r0, [r2, #84]	@ 0x54
    8e8c:	e592105c 	ldr	r1, [r2, #92]	@ 0x5c
    8e90:	e1a02004 	mov	r2, r4
    8e94:	ebffe1f2 	bl	1664 <sfe_ipv6_update_tcp_state.isra.0>
		break;
    8e98:	eaffff9c 	b	8d10 <fast_classifier_post_routing+0xa80>
    8e9c:	e59f002c 	ldr	r0, [pc, #44]	@ 8ed0 <fast_classifier_post_routing+0xc40>
    8ea0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		kfree(conn->sic);
    8ea4:	e5940008 	ldr	r0, [r4, #8]
    8ea8:	ebfffffe 	bl	0 <kfree>
		kfree(conn);
    8eac:	e1a00004 	mov	r0, r4
    8eb0:	ebfffffe 	bl	0 <kfree>
    8eb4:	eafffed9 	b	8a20 <fast_classifier_post_routing+0x790>
		src_br_dev = sfe_dev_get_master(src_dev);
    8eb8:	e1a00009 	mov	r0, r9
    8ebc:	ebffde60 	bl	844 <sfe_dev_get_master>
		if (!src_br_dev) {
    8ec0:	e2509000 	subs	r9, r0, #0
		src_br_dev = sfe_dev_get_master(src_dev);
    8ec4:	150b9178 	strne	r9, [fp, #-376]	@ 0xfffffe88
		if (!src_br_dev) {
    8ec8:	1afffe7b 	bne	88bc <fast_classifier_post_routing+0x62c>
    8ecc:	eafffee8 	b	8a74 <fast_classifier_post_routing+0x7e4>
    8ed0:	00000014 	.word	0x00000014

00008ed4 <__fast_classifier_ipv6_post_routing_hook>:
{
    8ed4:	e1a0c00d 	mov	ip, sp
    8ed8:	e92dd800 	push	{fp, ip, lr, pc}
    8edc:	e24cb004 	sub	fp, ip, #4
    8ee0:	e1a00001 	mov	r0, r1
	return fast_classifier_post_routing(skb, false);
    8ee4:	e3a01000 	mov	r1, #0
    8ee8:	ebfffce8 	bl	8290 <fast_classifier_post_routing>
}
    8eec:	e24bd00c 	sub	sp, fp, #12
    8ef0:	e89da800 	ldm	sp, {fp, sp, pc}

00008ef4 <__fast_classifier_ipv4_post_routing_hook>:
{
    8ef4:	e1a0c00d 	mov	ip, sp
    8ef8:	e92dd800 	push	{fp, ip, lr, pc}
    8efc:	e24cb004 	sub	fp, ip, #4
    8f00:	e1a00001 	mov	r0, r1
	return fast_classifier_post_routing(skb, true);
    8f04:	e3a01001 	mov	r1, #1
    8f08:	ebfffce0 	bl	8290 <fast_classifier_post_routing>
}
    8f0c:	e24bd00c 	sub	sp, fp, #12
    8f10:	e89da800 	ldm	sp, {fp, sp, pc}

00008f14 <fast_classifier_recv>:
{
    8f14:	e1a0c00d 	mov	ip, sp
    8f18:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    8f1c:	e24cb004 	sub	fp, ip, #4
    8f20:	e24dd00c 	sub	sp, sp, #12
	prefetch(skb->data + 32);
    8f24:	e59030b0 	ldr	r3, [r0, #176]	@ 0xb0
{
    8f28:	e1a04000 	mov	r4, r0
    8f2c:	f5d3f020 	pld	[r3, #32]
	if (skip_to_bridge_ingress &&
    8f30:	e3003000 	movw	r3, #0
	dev = skb->dev;
    8f34:	e5906008 	ldr	r6, [r0, #8]
	if (skip_to_bridge_ingress &&
    8f38:	e3403000 	movt	r3, #0
    8f3c:	e5d35010 	ldrb	r5, [r3, #16]
    8f40:	e3550000 	cmp	r5, #0
    8f44:	0a000003 	beq	8f58 <fast_classifier_recv+0x44>
	    (dev->priv_flags & IFF_BRIDGE_PORT)) {
    8f48:	e5965070 	ldr	r5, [r6, #112]	@ 0x70
	if (skip_to_bridge_ingress &&
    8f4c:	e3150c02 	tst	r5, #512	@ 0x200
	    (dev->priv_flags & IFF_BRIDGE_PORT)) {
    8f50:	e2055c02 	and	r5, r5, #512	@ 0x200
	if (skip_to_bridge_ingress &&
    8f54:	1a00004d 	bne	9090 <fast_classifier_recv+0x17c>
	if (dev->ingress_queue && !skb_skip_tc_classify(skb)) {
    8f58:	e5963240 	ldr	r3, [r6, #576]	@ 0x240
    8f5c:	e3530000 	cmp	r3, #0
    8f60:	0a000011 	beq	8fac <fast_classifier_recv+0x98>
}

static inline bool skb_skip_tc_classify(struct sk_buff *skb)
{
#ifdef CONFIG_NET_CLS_ACT
	if (skb->tc_skip_classify) {
    8f64:	e5d4306e 	ldrb	r3, [r4, #110]	@ 0x6e
    8f68:	e3130040 	tst	r3, #64	@ 0x40
    8f6c:	1a00000c 	bne	8fa4 <fast_classifier_recv+0x90>
	int ret = 0;
    8f70:	e3a00000 	mov	r0, #0
	if (master_dev) {
    8f74:	e3550000 	cmp	r5, #0
    8f78:	0a000007 	beq	8f9c <fast_classifier_recv+0x88>
	asm volatile(
    8f7c:	e10fc000 	mrs	ip, CPSR
    8f80:	f10c0080 	cpsid	i
		this_cpu_dec(*dev->pcpu_refcnt);
    8f84:	e5953310 	ldr	r3, [r5, #784]	@ 0x310
    8f88:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    8f8c:	e7932001 	ldr	r2, [r3, r1]
    8f90:	e2422001 	sub	r2, r2, #1
    8f94:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    8f98:	e121f00c 	msr	CPSR_c, ip
}
    8f9c:	e24bd028 	sub	sp, fp, #40	@ 0x28
    8fa0:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
		skb->tc_skip_classify = 0;
    8fa4:	e3c33040 	bic	r3, r3, #64	@ 0x40
    8fa8:	e5c4306e 	strb	r3, [r4, #110]	@ 0x6e
	if (likely(htons(ETH_P_IP) == skb->protocol)) {
    8fac:	e1d439bc 	ldrh	r3, [r4, #156]	@ 0x9c
    8fb0:	e3530008 	cmp	r3, #8
    8fb4:	1a000043 	bne	90c8 <fast_classifier_recv+0x1b4>
		in_dev = (struct in_device *)dev->ip_ptr;
    8fb8:	e59631f4 	ldr	r3, [r6, #500]	@ 0x1f4
		if (unlikely(!in_dev)) {
    8fbc:	e3530000 	cmp	r3, #0
    8fc0:	0affffea 	beq	8f70 <fast_classifier_recv+0x5c>
		if (unlikely(!in_dev->ifa_list)) {
    8fc4:	e593300c 	ldr	r3, [r3, #12]
    8fc8:	e3530000 	cmp	r3, #0
    8fcc:	0affffe7 	beq	8f70 <fast_classifier_recv+0x5c>
	len = skb->len;
    8fd0:	e594705c 	ldr	r7, [r4, #92]	@ 0x5c
	return skb->len - skb->data_len;
    8fd4:	e5941060 	ldr	r1, [r4, #96]	@ 0x60
    8fd8:	e0473001 	sub	r3, r7, r1
	if (likely(len <= skb_headlen(skb)))
    8fdc:	e3530013 	cmp	r3, #19
    8fe0:	9a00004e 	bls	9120 <fast_classifier_recv+0x20c>
	iph = (struct sfe_ipv4_ip_hdr *)skb->data;
    8fe4:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
	tot_len = ntohs(iph->tot_len);
    8fe8:	e1d310b2 	ldrh	r1, [r3, #2]
    8fec:	e6bf1fb1 	rev16	r1, r1
    8ff0:	e6ff1071 	uxth	r1, r1
	if (unlikely(tot_len < sizeof(struct sfe_ipv4_ip_hdr))) {
    8ff4:	e3510013 	cmp	r1, #19
    8ff8:	9a0000a5 	bls	9294 <fast_classifier_recv+0x380>
	if (unlikely(iph->version != 4)) {
    8ffc:	e5d32000 	ldrb	r2, [r3]
    9000:	e20200f0 	and	r0, r2, #240	@ 0xf0
    9004:	e3500040 	cmp	r0, #64	@ 0x40
    9008:	1a0000ae 	bne	92c8 <fast_classifier_recv+0x3b4>
	if (unlikely(tot_len > len)) {
    900c:	e1570001 	cmp	r7, r1
    9010:	3a0000b9 	bcc	92fc <fast_classifier_recv+0x3e8>
	frag_off = ntohs(iph->frag_off);
    9014:	e1d310b6 	ldrh	r1, [r3, #6]
    9018:	e6bf1fb1 	rev16	r1, r1
	if (unlikely(frag_off & IP_OFFSET)) {
    901c:	e7ec0051 	ubfx	r0, r1, #0, #13
	frag_off = ntohs(iph->frag_off);
    9020:	e6ff1071 	uxth	r1, r1
	if (unlikely(frag_off & IP_OFFSET)) {
    9024:	e3500000 	cmp	r0, #0
    9028:	1a0000c0 	bne	9330 <fast_classifier_recv+0x41c>
	ihl = iph->ihl << 2;
    902c:	e202900f 	and	r9, r2, #15
	flush_on_find = unlikely(frag_off & IP_MF) ? true : false;
    9030:	e7e016d1 	ubfx	r1, r1, #13, #1
	if (unlikely(ip_options)) {
    9034:	e3590005 	cmp	r9, #5
	ihl = iph->ihl << 2;
    9038:	e1a08109 	lsl	r8, r9, #2
	if (unlikely(ip_options)) {
    903c:	1a0000cf 	bne	9380 <fast_classifier_recv+0x46c>
	protocol = iph->protocol;
    9040:	e5d30009 	ldrb	r0, [r3, #9]
	if (IPPROTO_UDP == protocol) {
    9044:	e3500011 	cmp	r0, #17
    9048:	0a00002b 	beq	90fc <fast_classifier_recv+0x1e8>
	if (IPPROTO_TCP == protocol) {
    904c:	e3500006 	cmp	r0, #6
    9050:	0a0000c3 	beq	9364 <fast_classifier_recv+0x450>
	if (IPPROTO_ICMP == protocol) {
    9054:	e3500001 	cmp	r0, #1
    9058:	0a000045 	beq	9174 <fast_classifier_recv+0x260>
	raw_spin_lock_bh(&lock->rlock);
    905c:	e3000000 	movw	r0, #0
    9060:	e3400000 	movt	r0, #0
    9064:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    9068:	e59f35d8 	ldr	r3, [pc, #1496]	@ 9648 <fast_classifier_recv+0x734>
	si->packets_not_forwarded++;
    906c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9070:	e2430902 	sub	r0, r3, #32768	@ 0x8000
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    9074:	e59320e0 	ldr	r2, [r3, #224]	@ 0xe0
	si->packets_not_forwarded++;
    9078:	e2811001 	add	r1, r1, #1
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    907c:	e2822001 	add	r2, r2, #1
	si->packets_not_forwarded++;
    9080:	e5831050 	str	r1, [r3, #80]	@ 0x50
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
    9084:	e58320e0 	str	r2, [r3, #224]	@ 0xe0
    9088:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	return 0;
    908c:	eaffffb7 	b	8f70 <fast_classifier_recv+0x5c>
	master = netdev_master_upper_dev_get_rcu(dev);
    9090:	e1a00006 	mov	r0, r6
    9094:	ebfffffe 	bl	0 <netdev_master_upper_dev_get_rcu>
	if (master)
    9098:	e2506000 	subs	r6, r0, #0
    909c:	0a00001d 	beq	9118 <fast_classifier_recv+0x204>
	asm volatile(
    90a0:	e10f0000 	mrs	r0, CPSR
    90a4:	f10c0080 	cpsid	i
		this_cpu_inc(*dev->pcpu_refcnt);
    90a8:	e5963310 	ldr	r3, [r6, #784]	@ 0x310
    90ac:	ee1d1f90 	mrc	15, 0, r1, cr13, cr0, {4}
    90b0:	e7932001 	ldr	r2, [r3, r1]
    90b4:	e2822001 	add	r2, r2, #1
    90b8:	e7832001 	str	r2, [r3, r1]
	asm volatile(
    90bc:	e121f000 	msr	CPSR_c, r0
		master_dev = sfe_dev_get_master(dev);
    90c0:	e1a05006 	mov	r5, r6
    90c4:	eaffffa3 	b	8f58 <fast_classifier_recv+0x44>
	else if (likely(htons(ETH_P_IPV6) == skb->protocol)) {
    90c8:	e30d2d86 	movw	r2, #56710	@ 0xdd86
    90cc:	e1530002 	cmp	r3, r2
    90d0:	1affffa6 	bne	8f70 <fast_classifier_recv+0x5c>
		in_dev = (struct inet6_dev *)dev->ip6_ptr;
    90d4:	e59631f8 	ldr	r3, [r6, #504]	@ 0x1f8
		if (unlikely(!in_dev)) {
    90d8:	e3530000 	cmp	r3, #0
    90dc:	0affffa3 	beq	8f70 <fast_classifier_recv+0x5c>
	return READ_ONCE(head->next) == head;
    90e0:	e5b32004 	ldr	r2, [r3, #4]!
		if (unlikely(list_empty(&in_dev->addr_list))) {
    90e4:	e1520003 	cmp	r2, r3
    90e8:	0affffa0 	beq	8f70 <fast_classifier_recv+0x5c>
		ret = sfe_ipv6_recv(dev, skb);
    90ec:	e1a01004 	mov	r1, r4
    90f0:	e1a00006 	mov	r0, r6
    90f4:	ebfff88d 	bl	7330 <sfe_ipv6_recv>
    90f8:	eaffff9d 	b	8f74 <fast_classifier_recv+0x60>
		return sfe_ipv4_recv_udp(si, skb, dev, len, iph, ihl, flush_on_find);
    90fc:	e1a02007 	mov	r2, r7
    9100:	e1a00004 	mov	r0, r4
    9104:	e58d8000 	str	r8, [sp]
    9108:	e58d1004 	str	r1, [sp, #4]
    910c:	e1a01006 	mov	r1, r6
    9110:	ebffeea3 	bl	4ba4 <sfe_ipv4_recv_udp.constprop.0>
    9114:	eaffff96 	b	8f74 <fast_classifier_recv+0x60>
	int ret = 0;
    9118:	e1a00006 	mov	r0, r6
    911c:	eaffff9e 	b	8f9c <fast_classifier_recv+0x88>
	if (unlikely(len > skb->len))
    9120:	e3570013 	cmp	r7, #19
    9124:	9a000005 	bls	9140 <fast_classifier_recv+0x22c>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    9128:	e2811014 	add	r1, r1, #20
    912c:	e1a00004 	mov	r0, r4
    9130:	e0411007 	sub	r1, r1, r7
    9134:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (unlikely(!pskb_may_pull(skb, sizeof(struct sfe_ipv4_ip_hdr)))) {
    9138:	e3500000 	cmp	r0, #0
    913c:	1affffa8 	bne	8fe4 <fast_classifier_recv+0xd0>
	raw_spin_lock_bh(&lock->rlock);
    9140:	e3000000 	movw	r0, #0
    9144:	e3400000 	movt	r0, #0
    9148:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    914c:	e59f34f4 	ldr	r3, [pc, #1268]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    9150:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9154:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    9158:	e59320c8 	ldr	r2, [r3, #200]	@ 0xc8
		si->packets_not_forwarded++;
    915c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    9160:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    9164:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
    9168:	e58320c8 	str	r2, [r3, #200]	@ 0xc8
    916c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    9170:	eaffff7e 	b	8f70 <fast_classifier_recv+0x5c>
	return skb->len - skb->data_len;
    9174:	e594c05c 	ldr	ip, [r4, #92]	@ 0x5c
	u32 pull_len = sizeof(struct icmphdr) + ihl;
    9178:	e2881008 	add	r1, r8, #8
    917c:	e5940060 	ldr	r0, [r4, #96]	@ 0x60
    9180:	e04ce000 	sub	lr, ip, r0
	if (likely(len <= skb_headlen(skb)))
    9184:	e151000e 	cmp	r1, lr
    9188:	8a0000c1 	bhi	9494 <fast_classifier_recv+0x580>
	    && (icmph->type != ICMP_TIME_EXCEEDED)) {
    918c:	e7d32109 	ldrb	r2, [r3, r9, lsl #2]
	icmph = (struct icmphdr *)(skb->data + ihl);
    9190:	e0839008 	add	r9, r3, r8
	if ((icmph->type != ICMP_DEST_UNREACH)
    9194:	e20220f7 	and	r2, r2, #247	@ 0xf7
    9198:	e3520003 	cmp	r2, #3
    919c:	1a000087 	bne	93c0 <fast_classifier_recv+0x4ac>
	return skb->len - skb->data_len;
    91a0:	e1c425dc 	ldrd	r2, [r4, #92]	@ 0x5c
	pull_len += sizeof(struct sfe_ipv4_ip_hdr);
    91a4:	e288801c 	add	r8, r8, #28
    91a8:	e0421003 	sub	r1, r2, r3
	if (likely(len <= skb_headlen(skb)))
    91ac:	e1580001 	cmp	r8, r1
    91b0:	8a0000ce 	bhi	94f0 <fast_classifier_recv+0x5dc>
	if (unlikely(icmp_iph->version != 4)) {
    91b4:	e5d92008 	ldrb	r2, [r9, #8]
    91b8:	e20230f0 	and	r3, r2, #240	@ 0xf0
    91bc:	e3530040 	cmp	r3, #64	@ 0x40
    91c0:	1a0000df 	bne	9544 <fast_classifier_recv+0x630>
	icmp_ihl_words = icmp_iph->ihl;
    91c4:	e202200f 	and	r2, r2, #15
	icmp_ihl = icmp_ihl_words << 2;
    91c8:	e1a07102 	lsl	r7, r2, #2
	return skb->len - skb->data_len;
    91cc:	e1c425dc 	ldrd	r2, [r4, #92]	@ 0x5c
	pull_len += icmp_ihl - sizeof(struct sfe_ipv4_ip_hdr);
    91d0:	e0888007 	add	r8, r8, r7
    91d4:	e2481014 	sub	r1, r8, #20
    91d8:	e0420003 	sub	r0, r2, r3
	if (likely(len <= skb_headlen(skb)))
    91dc:	e1510000 	cmp	r1, r0
    91e0:	8a0000e4 	bhi	9578 <fast_classifier_recv+0x664>
	switch (icmp_iph->protocol) {
    91e4:	e5d92011 	ldrb	r2, [r9, #17]
	icmp_trans_h = ((u32 *)icmp_iph) + icmp_ihl_words;
    91e8:	e2877008 	add	r7, r7, #8
    91ec:	e089a007 	add	sl, r9, r7
	switch (icmp_iph->protocol) {
    91f0:	e3520006 	cmp	r2, #6
    91f4:	0a00008b 	beq	9428 <fast_classifier_recv+0x514>
    91f8:	e3520011 	cmp	r2, #17
    91fc:	1a00007c 	bne	93f4 <fast_classifier_recv+0x4e0>
	return skb->len - skb->data_len;
    9200:	e594305c 	ldr	r3, [r4, #92]	@ 0x5c
		pull_len += 8;
    9204:	e248800c 	sub	r8, r8, #12
    9208:	e5942060 	ldr	r2, [r4, #96]	@ 0x60
    920c:	e0431002 	sub	r1, r3, r2
	if (likely(len <= skb_headlen(skb)))
    9210:	e1580001 	cmp	r8, r1
    9214:	8a0000f6 	bhi	95f4 <fast_classifier_recv+0x6e0>
		dest_port = icmp_tcph->dest;
    9218:	e1da40b2 	ldrh	r4, [sl, #2]
	raw_spin_lock_bh(&lock->rlock);
    921c:	e3000000 	movw	r0, #0
		src_port = icmp_tcph->source;
    9220:	e19970b7 	ldrh	r7, [r9, r7]
    9224:	e3400000 	movt	r0, #0
	src_ip = icmp_iph->saddr;
    9228:	e599a014 	ldr	sl, [r9, #20]
	dest_ip = icmp_iph->daddr;
    922c:	e5998018 	ldr	r8, [r9, #24]
    9230:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, icmp_iph->protocol, dest_ip, dest_port, src_ip, src_port);
    9234:	e5d91011 	ldrb	r1, [r9, #17]
    9238:	e1a03004 	mov	r3, r4
    923c:	e1a00006 	mov	r0, r6
    9240:	e58da000 	str	sl, [sp]
    9244:	e1a02008 	mov	r2, r8
    9248:	e58d7004 	str	r7, [sp, #4]
    924c:	ebffdfda 	bl	11bc <sfe_ipv4_find_sfe_ipv4_connection_match.constprop.0>
	if (unlikely(!cm)) {
    9250:	e3500000 	cmp	r0, #0
    9254:	0a0000dc 	beq	95cc <fast_classifier_recv+0x6b8>
	c = cm->connection;
    9258:	e5904008 	ldr	r4, [r0, #8]
	sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    925c:	e1a00004 	mov	r0, r4
    9260:	ebffe3fc 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    9264:	e59f33dc 	ldr	r3, [pc, #988]	@ 9648 <fast_classifier_recv+0x734>
	si->packets_not_forwarded++;
    9268:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    926c:	e2430902 	sub	r0, r3, #32768	@ 0x8000
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    9270:	e59320c4 	ldr	r2, [r3, #196]	@ 0xc4
	si->packets_not_forwarded++;
    9274:	e2811001 	add	r1, r1, #1
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    9278:	e2822001 	add	r2, r2, #1
	si->packets_not_forwarded++;
    927c:	e5831050 	str	r1, [r3, #80]	@ 0x50
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
    9280:	e58320c4 	str	r2, [r3, #196]	@ 0xc4
    9284:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
    9288:	e1a00004 	mov	r0, r4
    928c:	ebffe258 	bl	1bf4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.1>
	return 0;
    9290:	eaffff36 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    9294:	e3000000 	movw	r0, #0
    9298:	e3400000 	movt	r0, #0
    929c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH]++;
    92a0:	e59f33a0 	ldr	r3, [pc, #928]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    92a4:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    92a8:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH]++;
    92ac:	e59320cc 	ldr	r2, [r3, #204]	@ 0xcc
		si->packets_not_forwarded++;
    92b0:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH]++;
    92b4:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    92b8:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH]++;
    92bc:	e58320cc 	str	r2, [r3, #204]	@ 0xcc
    92c0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    92c4:	eaffff29 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    92c8:	e3000000 	movw	r0, #0
    92cc:	e3400000 	movt	r0, #0
    92d0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_V4]++;
    92d4:	e59f336c 	ldr	r3, [pc, #876]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    92d8:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    92dc:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_V4]++;
    92e0:	e59320d0 	ldr	r2, [r3, #208]	@ 0xd0
		si->packets_not_forwarded++;
    92e4:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_V4]++;
    92e8:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    92ec:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_V4]++;
    92f0:	e58320d0 	str	r2, [r3, #208]	@ 0xd0
    92f4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    92f8:	eaffff1c 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    92fc:	e3000000 	movw	r0, #0
    9300:	e3400000 	movt	r0, #0
    9304:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    9308:	e59f3338 	ldr	r3, [pc, #824]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    930c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9310:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    9314:	e59320d8 	ldr	r2, [r3, #216]	@ 0xd8
		si->packets_not_forwarded++;
    9318:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    931c:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    9320:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
    9324:	e58320d8 	str	r2, [r3, #216]	@ 0xd8
    9328:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    932c:	eaffff0f 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    9330:	e3000000 	movw	r0, #0
    9334:	e3400000 	movt	r0, #0
    9338:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    933c:	e59f3304 	ldr	r3, [pc, #772]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    9340:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9344:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    9348:	e59320d4 	ldr	r2, [r3, #212]	@ 0xd4
		si->packets_not_forwarded++;
    934c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    9350:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    9354:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
    9358:	e58320d4 	str	r2, [r3, #212]	@ 0xd4
    935c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    9360:	eaffff02 	b	8f70 <fast_classifier_recv+0x5c>
		return sfe_ipv4_recv_tcp(si, skb, dev, len, iph, ihl, flush_on_find);
    9364:	e1a02007 	mov	r2, r7
    9368:	e1a00004 	mov	r0, r4
    936c:	e58d8000 	str	r8, [sp]
    9370:	e58d1004 	str	r1, [sp, #4]
    9374:	e1a01006 	mov	r1, r6
    9378:	ebfff2b4 	bl	5e50 <sfe_ipv4_recv_tcp.constprop.0>
    937c:	eafffefc 	b	8f74 <fast_classifier_recv+0x60>
		if (unlikely(len < ihl)) {
    9380:	e1570008 	cmp	r7, r8
		flush_on_find = true;
    9384:	23a01001 	movcs	r1, #1
		if (unlikely(len < ihl)) {
    9388:	2affff2c 	bcs	9040 <fast_classifier_recv+0x12c>
	raw_spin_lock_bh(&lock->rlock);
    938c:	e3000000 	movw	r0, #0
    9390:	e3400000 	movt	r0, #0
    9394:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE]++;
    9398:	e59f32a8 	ldr	r3, [pc, #680]	@ 9648 <fast_classifier_recv+0x734>
			si->packets_not_forwarded++;
    939c:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    93a0:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE]++;
    93a4:	e59320dc 	ldr	r2, [r3, #220]	@ 0xdc
			si->packets_not_forwarded++;
    93a8:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE]++;
    93ac:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    93b0:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE]++;
    93b4:	e58320dc 	str	r2, [r3, #220]	@ 0xdc
    93b8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    93bc:	eafffeeb 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    93c0:	e3000000 	movw	r0, #0
    93c4:	e3400000 	movt	r0, #0
    93c8:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    93cc:	e59f3274 	ldr	r3, [pc, #628]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    93d0:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    93d4:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    93d8:	e59320a4 	ldr	r2, [r3, #164]	@ 0xa4
		si->packets_not_forwarded++;
    93dc:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    93e0:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    93e4:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
    93e8:	e58320a4 	str	r2, [r3, #164]	@ 0xa4
    93ec:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    93f0:	eafffede 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    93f4:	e3000000 	movw	r0, #0
    93f8:	e3400000 	movt	r0, #0
    93fc:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL]++;
    9400:	e59f3240 	ldr	r3, [pc, #576]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    9404:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9408:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL]++;
    940c:	e59320bc 	ldr	r2, [r3, #188]	@ 0xbc
		si->packets_not_forwarded++;
    9410:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL]++;
    9414:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    9418:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL]++;
    941c:	e58320bc 	str	r2, [r3, #188]	@ 0xbc
    9420:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    9424:	eafffed1 	b	8f70 <fast_classifier_recv+0x5c>
	return skb->len - skb->data_len;
    9428:	e594105c 	ldr	r1, [r4, #92]	@ 0x5c
		pull_len += 8;
    942c:	e248800c 	sub	r8, r8, #12
    9430:	e5942060 	ldr	r2, [r4, #96]	@ 0x60
    9434:	e0410002 	sub	r0, r1, r2
	if (likely(len <= skb_headlen(skb)))
    9438:	e1580000 	cmp	r8, r0
    943c:	9affff75 	bls	9218 <fast_classifier_recv+0x304>
	if (unlikely(len > skb->len))
    9440:	e1580001 	cmp	r8, r1
    9444:	8a000005 	bhi	9460 <fast_classifier_recv+0x54c>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    9448:	e0421001 	sub	r1, r2, r1
    944c:	e1a00004 	mov	r0, r4
    9450:	e0811008 	add	r1, r1, r8
    9454:	ebfffffe 	bl	0 <__pskb_pull_tail>
		if (!pskb_may_pull(skb, pull_len)) {
    9458:	e3500000 	cmp	r0, #0
    945c:	1affff6d 	bne	9218 <fast_classifier_recv+0x304>
	raw_spin_lock_bh(&lock->rlock);
    9460:	e3000000 	movw	r0, #0
    9464:	e3400000 	movt	r0, #0
    9468:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE]++;
    946c:	e59f31d4 	ldr	r3, [pc, #468]	@ 9648 <fast_classifier_recv+0x734>
			si->packets_not_forwarded++;
    9470:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9474:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE]++;
    9478:	e59320b8 	ldr	r2, [r3, #184]	@ 0xb8
			si->packets_not_forwarded++;
    947c:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE]++;
    9480:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    9484:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE]++;
    9488:	e58320b8 	str	r2, [r3, #184]	@ 0xb8
    948c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    9490:	eafffeb6 	b	8f70 <fast_classifier_recv+0x5c>
	if (unlikely(len > skb->len))
    9494:	e151000c 	cmp	r1, ip
    9498:	8a000007 	bhi	94bc <fast_classifier_recv+0x5a8>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    949c:	e040000c 	sub	r0, r0, ip
    94a0:	e0801001 	add	r1, r0, r1
    94a4:	e1a00004 	mov	r0, r4
    94a8:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, pull_len)) {
    94ac:	e3500000 	cmp	r0, #0
    94b0:	0a000001 	beq	94bc <fast_classifier_recv+0x5a8>
	icmph = (struct icmphdr *)(skb->data + ihl);
    94b4:	e59430b0 	ldr	r3, [r4, #176]	@ 0xb0
    94b8:	eaffff33 	b	918c <fast_classifier_recv+0x278>
	raw_spin_lock_bh(&lock->rlock);
    94bc:	e3000000 	movw	r0, #0
    94c0:	e3400000 	movt	r0, #0
    94c4:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    94c8:	e59f3178 	ldr	r3, [pc, #376]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    94cc:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    94d0:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    94d4:	e59320a0 	ldr	r2, [r3, #160]	@ 0xa0
		si->packets_not_forwarded++;
    94d8:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    94dc:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    94e0:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
    94e4:	e58320a0 	str	r2, [r3, #160]	@ 0xa0
    94e8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    94ec:	eafffe9f 	b	8f70 <fast_classifier_recv+0x5c>
	if (unlikely(len > skb->len))
    94f0:	e1580002 	cmp	r8, r2
    94f4:	8a000005 	bhi	9510 <fast_classifier_recv+0x5fc>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    94f8:	e0433002 	sub	r3, r3, r2
    94fc:	e1a00004 	mov	r0, r4
    9500:	e0831008 	add	r1, r3, r8
    9504:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, pull_len)) {
    9508:	e3500000 	cmp	r0, #0
    950c:	1affff28 	bne	91b4 <fast_classifier_recv+0x2a0>
	raw_spin_lock_bh(&lock->rlock);
    9510:	e3000000 	movw	r0, #0
    9514:	e3400000 	movt	r0, #0
    9518:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE]++;
    951c:	e59f3124 	ldr	r3, [pc, #292]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    9520:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9524:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE]++;
    9528:	e59320a8 	ldr	r2, [r3, #168]	@ 0xa8
		si->packets_not_forwarded++;
    952c:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE]++;
    9530:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    9534:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE]++;
    9538:	e58320a8 	str	r2, [r3, #168]	@ 0xa8
    953c:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    9540:	eafffe8a 	b	8f70 <fast_classifier_recv+0x5c>
	raw_spin_lock_bh(&lock->rlock);
    9544:	e3000000 	movw	r0, #0
    9548:	e3400000 	movt	r0, #0
    954c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4]++;
    9550:	e59f30f0 	ldr	r3, [pc, #240]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    9554:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9558:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4]++;
    955c:	e59320ac 	ldr	r2, [r3, #172]	@ 0xac
		si->packets_not_forwarded++;
    9560:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4]++;
    9564:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    9568:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4]++;
    956c:	e58320ac 	str	r2, [r3, #172]	@ 0xac
    9570:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    9574:	eafffe7d 	b	8f70 <fast_classifier_recv+0x5c>
	if (unlikely(len > skb->len))
    9578:	e1510002 	cmp	r1, r2
    957c:	8a000005 	bhi	9598 <fast_classifier_recv+0x684>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    9580:	e0433002 	sub	r3, r3, r2
    9584:	e1a00004 	mov	r0, r4
    9588:	e0831001 	add	r1, r3, r1
    958c:	ebfffffe 	bl	0 <__pskb_pull_tail>
	if (!pskb_may_pull(skb, pull_len)) {
    9590:	e3500000 	cmp	r0, #0
    9594:	1affff12 	bne	91e4 <fast_classifier_recv+0x2d0>
	raw_spin_lock_bh(&lock->rlock);
    9598:	e3000000 	movw	r0, #0
    959c:	e3400000 	movt	r0, #0
    95a0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE]++;
    95a4:	e59f309c 	ldr	r3, [pc, #156]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    95a8:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    95ac:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE]++;
    95b0:	e59320b0 	ldr	r2, [r3, #176]	@ 0xb0
		si->packets_not_forwarded++;
    95b4:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE]++;
    95b8:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    95bc:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE]++;
    95c0:	e58320b0 	str	r2, [r3, #176]	@ 0xb0
    95c4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    95c8:	eafffe68 	b	8f70 <fast_classifier_recv+0x5c>
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    95cc:	e59f3074 	ldr	r3, [pc, #116]	@ 9648 <fast_classifier_recv+0x734>
		si->packets_not_forwarded++;
    95d0:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
    95d4:	e2430902 	sub	r0, r3, #32768	@ 0x8000
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    95d8:	e59320c0 	ldr	r2, [r3, #192]	@ 0xc0
		si->packets_not_forwarded++;
    95dc:	e2811001 	add	r1, r1, #1
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    95e0:	e2822001 	add	r2, r2, #1
		si->packets_not_forwarded++;
    95e4:	e5831050 	str	r1, [r3, #80]	@ 0x50
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
    95e8:	e58320c0 	str	r2, [r3, #192]	@ 0xc0
    95ec:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return 0;
    95f0:	eafffe5e 	b	8f70 <fast_classifier_recv+0x5c>
	if (unlikely(len > skb->len))
    95f4:	e1580003 	cmp	r8, r3
    95f8:	8a000005 	bhi	9614 <fast_classifier_recv+0x700>
	return __pskb_pull_tail(skb, len - skb_headlen(skb)) != NULL;
    95fc:	e0422003 	sub	r2, r2, r3
    9600:	e1a00004 	mov	r0, r4
    9604:	e0821008 	add	r1, r2, r8
    9608:	ebfffffe 	bl	0 <__pskb_pull_tail>
		if (!pskb_may_pull(skb, pull_len)) {
    960c:	e3500000 	cmp	r0, #0
    9610:	1affff00 	bne	9218 <fast_classifier_recv+0x304>
	raw_spin_lock_bh(&lock->rlock);
    9614:	e3000000 	movw	r0, #0
    9618:	e3400000 	movt	r0, #0
    961c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE]++;
    9620:	e59f3020 	ldr	r3, [pc, #32]	@ 9648 <fast_classifier_recv+0x734>
			si->packets_not_forwarded++;
    9624:	e5931050 	ldr	r1, [r3, #80]	@ 0x50
	raw_spin_unlock_bh(&lock->rlock);
    9628:	e2430902 	sub	r0, r3, #32768	@ 0x8000
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE]++;
    962c:	e59320b4 	ldr	r2, [r3, #180]	@ 0xb4
			si->packets_not_forwarded++;
    9630:	e2811001 	add	r1, r1, #1
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE]++;
    9634:	e2822001 	add	r2, r2, #1
			si->packets_not_forwarded++;
    9638:	e5831050 	str	r1, [r3, #80]	@ 0x50
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE]++;
    963c:	e58320b4 	str	r2, [r3, #180]	@ 0xb4
    9640:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
			return 0;
    9644:	eafffe49 	b	8f70 <fast_classifier_recv+0x5c>
    9648:	000182c8 	.word	0x000182c8

0000964c <fast_classifier_conntrack_event>:
{
    964c:	e1a0c00d 	mov	ip, sp
    9650:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
    9654:	e24cb004 	sub	fp, ip, #4
    9658:	e24dd0ac 	sub	sp, sp, #172	@ 0xac
	struct nf_conn *ct = item->ct;
    965c:	e592c000 	ldr	ip, [r2]
{
    9660:	e1a05001 	mov	r5, r1
	if (unlikely(!ct)) {
    9664:	e35c0000 	cmp	ip, #0
    9668:	0a000019 	beq	96d4 <fast_classifier_conntrack_event+0x88>
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    966c:	e1cc21dc 	ldrd	r2, [ip, #28]
    9670:	e1cc01d4 	ldrd	r0, [ip, #20]
    9674:	e5dc403a 	ldrb	r4, [ip, #58]	@ 0x3a
    9678:	e59ce028 	ldr	lr, [ip, #40]	@ 0x28
    967c:	e1dca2b4 	ldrh	sl, [ip, #36]	@ 0x24
    9680:	e14b28f0 	strd	r2, [fp, #-128]	@ 0xffffff80
    9684:	e1cc22d4 	ldrd	r2, [ip, #36]	@ 0x24
    9688:	e1dc93b8 	ldrh	r9, [ip, #56]	@ 0x38
	sid.protocol = (s32)orig_tuple.dst.protonum;
    968c:	e50b40b0 	str	r4, [fp, #-176]	@ 0xffffff50
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    9690:	e14b08f8 	strd	r0, [fp, #-136]	@ 0xffffff78
	return ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.l3num;
    9694:	e1dc12b6 	ldrh	r1, [ip, #38]	@ 0x26
    9698:	e59c0014 	ldr	r0, [ip, #20]
    969c:	e14b27f8 	strd	r2, [fp, #-120]	@ 0xffffff88
    96a0:	e1cc22dc 	ldrd	r2, [ip, #44]	@ 0x2c
	if (likely(nf_ct_l3num(ct) == AF_INET)) {
    96a4:	e3510002 	cmp	r1, #2
	orig_tuple = ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple;
    96a8:	e14b27f0 	strd	r2, [fp, #-112]	@ 0xffffff90
    96ac:	e1cc23d4 	ldrd	r2, [ip, #52]	@ 0x34
    96b0:	e14b26f8 	strd	r2, [fp, #-104]	@ 0xffffff98
	if (likely(nf_ct_l3num(ct) == AF_INET)) {
    96b4:	1a00018b 	bne	9ce8 <fast_classifier_conntrack_event+0x69c>
		is_v4 = true;
    96b8:	e3a07001 	mov	r7, #1
		sid.src_ip.ip = (__be32)orig_tuple.src.u3.ip;
    96bc:	e50b00ac 	str	r0, [fp, #-172]	@ 0xffffff54
		sid.dest_ip.ip = (__be32)orig_tuple.dst.u3.ip;
    96c0:	e50be09c 	str	lr, [fp, #-156]	@ 0xffffff64
	switch (sid.protocol) {
    96c4:	e3540006 	cmp	r4, #6
    96c8:	0a000004 	beq	96e0 <fast_classifier_conntrack_event+0x94>
    96cc:	e3540011 	cmp	r4, #17
    96d0:	0a000002 	beq	96e0 <fast_classifier_conntrack_event+0x94>
}
    96d4:	e3a00000 	mov	r0, #0
    96d8:	e24bd028 	sub	sp, fp, #40	@ 0x28
    96dc:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
	if ((events & (1 << IPCT_MARK)) && (ct->mark != 0)) {
    96e0:	e3150080 	tst	r5, #128	@ 0x80
		sid.src_port = orig_tuple.src.u.tcp.port;
    96e4:	e14ba8bc 	strh	sl, [fp, #-140]	@ 0xffffff74
		sid.dest_port = orig_tuple.dst.u.tcp.port;
    96e8:	e14b98ba 	strh	r9, [fp, #-138]	@ 0xffffff76
	if ((events & (1 << IPCT_MARK)) && (ct->mark != 0)) {
    96ec:	1a00003a 	bne	97dc <fast_classifier_conntrack_event+0x190>
	if (unlikely(!(events & (1 << IPCT_DESTROY)))) {
    96f0:	e3150004 	tst	r5, #4
    96f4:	0afffff6 	beq	96d4 <fast_classifier_conntrack_event+0x88>
	raw_spin_lock_bh(&lock->rlock);
    96f8:	e3006000 	movw	r6, #0
    96fc:	e3406000 	movt	r6, #0
    9700:	e2860014 	add	r0, r6, #20
    9704:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	u32 hash = saddr->ip ^ daddr->ip;
    9708:	e51b80ac 	ldr	r8, [fp, #-172]	@ 0xffffff54
	if (is_v4)
    970c:	e3570000 	cmp	r7, #0
	u32 hash = saddr->ip ^ daddr->ip;
    9710:	e51b309c 	ldr	r3, [fp, #-156]	@ 0xffffff64
    9714:	e0285003 	eor	r5, r8, r3
    9718:	e50b30bc 	str	r3, [fp, #-188]	@ 0xffffff44
	if (is_v4)
    971c:	0a000097 	beq	9980 <fast_classifier_conntrack_event+0x334>
	return hash ^ (sport | (dport << 16));
    9720:	e51b108c 	ldr	r1, [fp, #-140]	@ 0xffffff74
	return val * GOLDEN_RATIO_32;
    9724:	e3080647 	movw	r0, #34375	@ 0x8647
    9728:	e34601c8 	movt	r0, #25032	@ 0x61c8
    972c:	e0211005 	eor	r1, r1, r5
    9730:	e0010190 	mul	r1, r0, r1
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    9734:	e3000000 	movw	r0, #0
    9738:	e3400000 	movt	r0, #0
	return __hash_32(val) >> (32 - bits);
    973c:	e1a019a1 	lsr	r1, r1, #19
    9740:	e7901101 	ldr	r1, [r0, r1, lsl #2]
    9744:	e3510000 	cmp	r1, #0
    9748:	1a000003 	bne	975c <fast_classifier_conntrack_event+0x110>
    974c:	ea00022c 	b	a004 <fast_classifier_conntrack_event+0x9b8>
    9750:	e5911000 	ldr	r1, [r1]
    9754:	e3510000 	cmp	r1, #0
    9758:	0a000229 	beq	a004 <fast_classifier_conntrack_event+0x9b8>
		if (conn->is_v4 != is_v4) {
    975c:	e5d1301c 	ldrb	r3, [r1, #28]
    9760:	e3530000 	cmp	r3, #0
    9764:	0afffff9 	beq	9750 <fast_classifier_conntrack_event+0x104>
		p_sic = conn->sic;
    9768:	e5910008 	ldr	r0, [r1, #8]
		if (p_sic->protocol == proto &&
    976c:	e5903000 	ldr	r3, [r0]
    9770:	e1540003 	cmp	r4, r3
    9774:	1afffff5 	bne	9750 <fast_classifier_conntrack_event+0x104>
    9778:	e1d035b8 	ldrh	r3, [r0, #88]	@ 0x58
    977c:	e153000a 	cmp	r3, sl
    9780:	1afffff2 	bne	9750 <fast_classifier_conntrack_event+0x104>
		    p_sic->src_port == sport &&
    9784:	e1d035bc 	ldrh	r3, [r0, #92]	@ 0x5c
    9788:	e1530009 	cmp	r3, r9
    978c:	1affffef 	bne	9750 <fast_classifier_conntrack_event+0x104>
		    p_sic->dest_port == dport &&
    9790:	e5903018 	ldr	r3, [r0, #24]
    9794:	e1530008 	cmp	r3, r8
    9798:	1affffec 	bne	9750 <fast_classifier_conntrack_event+0x104>
		    sfe_addr_equal(&p_sic->src_ip, saddr, is_v4) &&
    979c:	e51b20bc 	ldr	r2, [fp, #-188]	@ 0xffffff44
    97a0:	e5903038 	ldr	r3, [r0, #56]	@ 0x38
    97a4:	e1530002 	cmp	r3, r2
    97a8:	1affffe8 	bne	9750 <fast_classifier_conntrack_event+0x104>
	if (conn && conn->offloaded) {
    97ac:	e5913018 	ldr	r3, [r1, #24]
    97b0:	e3530000 	cmp	r3, #0
    97b4:	e50b30b8 	str	r3, [fp, #-184]	@ 0xffffff48
    97b8:	0a0001f3 	beq	9f8c <fast_classifier_conntrack_event+0x940>
			fc_msg.ethertype = AF_INET;
    97bc:	e3a00002 	mov	r0, #2
    97c0:	e14b06b0 	strh	r0, [fp, #-96]	@ 0xffffffa0
			fc_msg.src_saddr.in = *((struct in_addr *)&conn->sic->src_ip);
    97c4:	e5910008 	ldr	r0, [r1, #8]
    97c8:	e590c018 	ldr	ip, [r0, #24]
    97cc:	e50bc05c 	str	ip, [fp, #-92]	@ 0xffffffa4
			fc_msg.dst_saddr.in = *((struct in_addr *)&conn->sic->dest_ip_xlate);
    97d0:	e5900048 	ldr	r0, [r0, #72]	@ 0x48
    97d4:	e50b004c 	str	r0, [fp, #-76]	@ 0xffffffb4
    97d8:	ea0000d1 	b	9b24 <fast_classifier_conntrack_event+0x4d8>
	if ((events & (1 << IPCT_MARK)) && (ct->mark != 0)) {
    97dc:	e59c607c 	ldr	r6, [ip, #124]	@ 0x7c
    97e0:	e3560000 	cmp	r6, #0
    97e4:	0affffc1 	beq	96f0 <fast_classifier_conntrack_event+0xa4>
		mark.src_port = sid.src_port;
    97e8:	e3a01000 	mov	r1, #0
		is_v4 ? sfe_ipv4_mark_rule(&mark) : sfe_ipv6_mark_rule(&mark);
    97ec:	e3570000 	cmp	r7, #0
		mark.protocol = sid.protocol;
    97f0:	e50b4060 	str	r4, [fp, #-96]	@ 0xffffffa0
		mark.src_port = sid.src_port;
    97f4:	e7cf101a 	bfi	r1, sl, #0, #16
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    97f8:	e02a3009 	eor	r3, sl, r9
		mark.mark = ct->mark;
    97fc:	e50b6038 	str	r6, [fp, #-56]	@ 0xffffffc8
		mark.src_port = sid.src_port;
    9800:	e7df1819 	bfi	r1, r9, #16, #16
    9804:	e50b103c 	str	r1, [fp, #-60]	@ 0xffffffc4
		mark.src_ip = sid.src_ip;
    9808:	e14b0adc 	ldrd	r0, [fp, #-172]	@ 0xffffff54
    980c:	e14b05fc 	strd	r0, [fp, #-92]	@ 0xffffffa4
    9810:	e14b0ad4 	ldrd	r0, [fp, #-164]	@ 0xffffff5c
    9814:	e14b05f4 	strd	r0, [fp, #-84]	@ 0xffffffac
		mark.dest_ip = sid.dest_ip;
    9818:	e14b09dc 	ldrd	r0, [fp, #-156]	@ 0xffffff64
    981c:	e14b04fc 	strd	r0, [fp, #-76]	@ 0xffffffb4
    9820:	e14b09d4 	ldrd	r0, [fp, #-148]	@ 0xffffff6c
    9824:	e14b04f4 	strd	r0, [fp, #-68]	@ 0xffffffbc
		is_v4 ? sfe_ipv4_mark_rule(&mark) : sfe_ipv6_mark_rule(&mark);
    9828:	0a000148 	beq	9d50 <fast_classifier_conntrack_event+0x704>
    982c:	e3000000 	movw	r0, #0
    9830:	e50b30bc 	str	r3, [fp, #-188]	@ 0xffffff44
    9834:	e3400000 	movt	r0, #0
    9838:	e50b00b8 	str	r0, [fp, #-184]	@ 0xffffff48
    983c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
    9840:	e51b30bc 	ldr	r3, [fp, #-188]	@ 0xffffff44
	c = si->conn_hash[conn_idx];
    9844:	e51b00b8 	ldr	r0, [fp, #-184]	@ 0xffffff48
	c = sfe_ipv4_find_sfe_ipv4_connection(si, mark->protocol,
    9848:	e51b805c 	ldr	r8, [fp, #-92]	@ 0xffffffa4
    984c:	e51b204c 	ldr	r2, [fp, #-76]	@ 0xffffffb4
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    9850:	e6bf1fb3 	rev16	r1, r3
    9854:	e6ffc071 	uxth	ip, r1
    9858:	e0283002 	eor	r3, r8, r2
    985c:	e6bf1f33 	rev	r1, r3
    9860:	e0211004 	eor	r1, r1, r4
    9864:	e021100c 	eor	r1, r1, ip
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    9868:	e0211621 	eor	r1, r1, r1, lsr #12
    986c:	e7eb1051 	ubfx	r1, r1, #0, #12
	c = si->conn_hash[conn_idx];
    9870:	e281100c 	add	r1, r1, #12
    9874:	e7901101 	ldr	r1, [r0, r1, lsl #2]
	if (unlikely(!c)) {
    9878:	e3510000 	cmp	r1, #0
    987c:	1a000003 	bne	9890 <fast_classifier_conntrack_event+0x244>
    9880:	ea000012 	b	98d0 <fast_classifier_conntrack_event+0x284>
		c = c->next;
    9884:	e5911000 	ldr	r1, [r1]
	} while (c && (c->src_port != src_port
    9888:	e3510000 	cmp	r1, #0
    988c:	0a00000f 	beq	98d0 <fast_classifier_conntrack_event+0x284>
    9890:	e1d101bc 	ldrh	r0, [r1, #28]
    9894:	e150000a 	cmp	r0, sl
    9898:	1afffff9 	bne	9884 <fast_classifier_conntrack_event+0x238>
		 || c->dest_port != dest_port
    989c:	e1d102b0 	ldrh	r0, [r1, #32]
    98a0:	e1500009 	cmp	r0, r9
    98a4:	1afffff6 	bne	9884 <fast_classifier_conntrack_event+0x238>
		 || c->src_ip != src_ip
    98a8:	e591000c 	ldr	r0, [r1, #12]
    98ac:	e1580000 	cmp	r8, r0
    98b0:	1afffff3 	bne	9884 <fast_classifier_conntrack_event+0x238>
		 || c->dest_ip != dest_ip
    98b4:	e5910014 	ldr	r0, [r1, #20]
    98b8:	e1520000 	cmp	r2, r0
    98bc:	1afffff0 	bne	9884 <fast_classifier_conntrack_event+0x238>
		 || c->protocol != protocol));
    98c0:	e5910008 	ldr	r0, [r1, #8]
    98c4:	e1540000 	cmp	r4, r0
    98c8:	1affffed 	bne	9884 <fast_classifier_conntrack_event+0x238>
		c->mark = mark->mark;
    98cc:	e5816048 	str	r6, [r1, #72]	@ 0x48
	raw_spin_unlock_bh(&lock->rlock);
    98d0:	e3000000 	movw	r0, #0
    98d4:	e50b30bc 	str	r3, [fp, #-188]	@ 0xffffff44
    98d8:	e3400000 	movt	r0, #0
    98dc:	e50b20b8 	str	r2, [fp, #-184]	@ 0xffffff48
    98e0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	raw_spin_lock_bh(&lock->rlock);
    98e4:	e59f0724 	ldr	r0, [pc, #1828]	@ a010 <fast_classifier_conntrack_event+0x9c4>
    98e8:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	return hash ^ (sport | (dport << 16));
    98ec:	e51b30bc 	ldr	r3, [fp, #-188]	@ 0xffffff44
    98f0:	e18a1809 	orr	r1, sl, r9, lsl #16
	return val * GOLDEN_RATIO_32;
    98f4:	e3080647 	movw	r0, #34375	@ 0x8647
    98f8:	e34601c8 	movt	r0, #25032	@ 0x61c8
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    98fc:	e51b20b8 	ldr	r2, [fp, #-184]	@ 0xffffff48
	return hash ^ (sport | (dport << 16));
    9900:	e0211003 	eor	r1, r1, r3
    9904:	e0010190 	mul	r1, r0, r1
	sfe_hash_for_each_possible(fc_conn_ht, conn, node, hl, key) {
    9908:	e3000000 	movw	r0, #0
    990c:	e3400000 	movt	r0, #0
	return __hash_32(val) >> (32 - bits);
    9910:	e1a019a1 	lsr	r1, r1, #19
    9914:	e7901101 	ldr	r1, [r0, r1, lsl #2]
    9918:	e3510000 	cmp	r1, #0
    991c:	1a000003 	bne	9930 <fast_classifier_conntrack_event+0x2e4>
    9920:	ea000172 	b	9ef0 <fast_classifier_conntrack_event+0x8a4>
    9924:	e5911000 	ldr	r1, [r1]
    9928:	e3510000 	cmp	r1, #0
    992c:	0a00016f 	beq	9ef0 <fast_classifier_conntrack_event+0x8a4>
		if (conn->is_v4 != is_v4) {
    9930:	e5d1301c 	ldrb	r3, [r1, #28]
    9934:	e3530000 	cmp	r3, #0
    9938:	0afffff9 	beq	9924 <fast_classifier_conntrack_event+0x2d8>
		p_sic = conn->sic;
    993c:	e5913008 	ldr	r3, [r1, #8]
		if (p_sic->protocol == proto &&
    9940:	e5930000 	ldr	r0, [r3]
    9944:	e1540000 	cmp	r4, r0
    9948:	1afffff5 	bne	9924 <fast_classifier_conntrack_event+0x2d8>
    994c:	e1d305b8 	ldrh	r0, [r3, #88]	@ 0x58
    9950:	e150000a 	cmp	r0, sl
    9954:	1afffff2 	bne	9924 <fast_classifier_conntrack_event+0x2d8>
		    p_sic->src_port == sport &&
    9958:	e1d305bc 	ldrh	r0, [r3, #92]	@ 0x5c
    995c:	e1500009 	cmp	r0, r9
    9960:	1affffef 	bne	9924 <fast_classifier_conntrack_event+0x2d8>
		    p_sic->dest_port == dport &&
    9964:	e5930018 	ldr	r0, [r3, #24]
    9968:	e1580000 	cmp	r8, r0
    996c:	1affffec 	bne	9924 <fast_classifier_conntrack_event+0x2d8>
		    sfe_addr_equal(&p_sic->src_ip, saddr, is_v4) &&
    9970:	e5933038 	ldr	r3, [r3, #56]	@ 0x38
    9974:	e1520003 	cmp	r2, r3
    9978:	1affffe9 	bne	9924 <fast_classifier_conntrack_event+0x2d8>
    997c:	ea000159 	b	9ee8 <fast_classifier_conntrack_event+0x89c>
		conn = fast_classifier_find_conn(&sid.src_ip, &sid.dest_ip, sid.src_port, sid.dest_port, sid.protocol, false);
    9980:	e24b109c 	sub	r1, fp, #156	@ 0x9c
    9984:	e1a03009 	mov	r3, r9
    9988:	e88d0090 	stm	sp, {r4, r7}
    998c:	e1a0200a 	mov	r2, sl
    9990:	e24b00ac 	sub	r0, fp, #172	@ 0xac
    9994:	ebffe7eb 	bl	3948 <fast_classifier_find_conn>
	if (conn && conn->offloaded) {
    9998:	e2501000 	subs	r1, r0, #0
    999c:	0a00007f 	beq	9ba0 <fast_classifier_conntrack_event+0x554>
    99a0:	e5913018 	ldr	r3, [r1, #24]
    99a4:	e3530000 	cmp	r3, #0
    99a8:	e50b30b8 	str	r3, [fp, #-184]	@ 0xffffff48
    99ac:	1a000050 	bne	9af4 <fast_classifier_conntrack_event+0x4a8>
	return !h->pprev;
    99b0:	e591c004 	ldr	ip, [r1, #4]
	if (!hlist_unhashed(n)) {
    99b4:	e35c0000 	cmp	ip, #0
    99b8:	0a00006f 	beq	9b7c <fast_classifier_conntrack_event+0x530>
	struct hlist_node *next = n->next;
    99bc:	e5910000 	ldr	r0, [r1]
	if (next)
    99c0:	e3500000 	cmp	r0, #0
	WRITE_ONCE(*pprev, next);
    99c4:	e58c0000 	str	r0, [ip]
		WRITE_ONCE(next->pprev, pprev);
    99c8:	1580c004 	strne	ip, [r0, #4]
	h->next = NULL;
    99cc:	e3a00000 	mov	r0, #0
    99d0:	e5810000 	str	r0, [r1]
	h->pprev = NULL;
    99d4:	e5810004 	str	r0, [r1, #4]
		sfe_connections_size--;
    99d8:	e5960020 	ldr	r0, [r6, #32]
		kfree(conn->sic);
    99dc:	e50b10c0 	str	r1, [fp, #-192]	@ 0xffffff40
		sfe_connections_size--;
    99e0:	e2400001 	sub	r0, r0, #1
    99e4:	e5860020 	str	r0, [r6, #32]
		kfree(conn->sic);
    99e8:	e5910008 	ldr	r0, [r1, #8]
    99ec:	ebfffffe 	bl	0 <kfree>
		kfree(conn);
    99f0:	e51b10c0 	ldr	r1, [fp, #-192]	@ 0xffffff40
    99f4:	e1a00001 	mov	r0, r1
    99f8:	ebfffffe 	bl	0 <kfree>
	raw_spin_unlock_bh(&lock->rlock);
    99fc:	e59f060c 	ldr	r0, [pc, #1548]	@ a010 <fast_classifier_conntrack_event+0x9c4>
    9a00:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	is_v4 ? sfe_ipv4_destroy_rule(&sid) : sfe_ipv6_destroy_rule(&sid);
    9a04:	e3570000 	cmp	r7, #0
    9a08:	0a000068 	beq	9bb0 <fast_classifier_conntrack_event+0x564>
	raw_spin_lock_bh(&lock->rlock);
    9a0c:	e3006000 	movw	r6, #0
    9a10:	e3406000 	movt	r6, #0
    9a14:	e1a00006 	mov	r0, r6
    9a18:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
    9a1c:	e02a0009 	eor	r0, sl, r9
    9a20:	e6bf1f35 	rev	r1, r5
    9a24:	e6bf0fb0 	rev16	r0, r0
    9a28:	e0211004 	eor	r1, r1, r4
    9a2c:	e6ff0070 	uxth	r0, r0
    9a30:	e0211000 	eor	r1, r1, r0
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
    9a34:	e0211621 	eor	r1, r1, r1, lsr #12
    9a38:	e7eb1051 	ubfx	r1, r1, #0, #12
	c = si->conn_hash[conn_idx];
    9a3c:	e281100c 	add	r1, r1, #12
    9a40:	e7965101 	ldr	r5, [r6, r1, lsl #2]
	si->connection_destroy_requests++;
    9a44:	e2866902 	add	r6, r6, #32768	@ 0x8000
    9a48:	e5961038 	ldr	r1, [r6, #56]	@ 0x38
	if (unlikely(!c)) {
    9a4c:	e3550000 	cmp	r5, #0
	si->connection_destroy_requests++;
    9a50:	e2811001 	add	r1, r1, #1
    9a54:	e5861038 	str	r1, [r6, #56]	@ 0x38
	if (unlikely(!c)) {
    9a58:	0a000162 	beq	9fe8 <fast_classifier_conntrack_event+0x99c>
	if ((c->src_port == src_port)
    9a5c:	e1d511bc 	ldrh	r1, [r5, #28]
    9a60:	e151000a 	cmp	r1, sl
    9a64:	0a0000ab 	beq	9d18 <fast_classifier_conntrack_event+0x6cc>
		c = c->next;
    9a68:	e5955000 	ldr	r5, [r5]
	} while (c && (c->src_port != src_port
    9a6c:	e3550000 	cmp	r5, #0
    9a70:	0a00015c 	beq	9fe8 <fast_classifier_conntrack_event+0x99c>
    9a74:	e1d531bc 	ldrh	r3, [r5, #28]
    9a78:	e153000a 	cmp	r3, sl
    9a7c:	1afffff9 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
		 || c->dest_port != dest_port
    9a80:	e1d532b0 	ldrh	r3, [r5, #32]
    9a84:	e1530009 	cmp	r3, r9
    9a88:	1afffff6 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
		 || c->src_ip != src_ip
    9a8c:	e595300c 	ldr	r3, [r5, #12]
    9a90:	e1580003 	cmp	r8, r3
    9a94:	1afffff3 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
		 || c->dest_ip != dest_ip
    9a98:	e51b20bc 	ldr	r2, [fp, #-188]	@ 0xffffff44
    9a9c:	e5953014 	ldr	r3, [r5, #20]
    9aa0:	e1520003 	cmp	r2, r3
    9aa4:	1affffef 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
		 || c->protocol != protocol));
    9aa8:	e5953008 	ldr	r3, [r5, #8]
    9aac:	e1540003 	cmp	r4, r3
    9ab0:	1affffec 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
	sfe_ipv4_remove_sfe_ipv4_connection(si, c);
    9ab4:	e1a00005 	mov	r0, r5
    9ab8:	ebffe1e6 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
    9abc:	e3000000 	movw	r0, #0
    9ac0:	e3400000 	movt	r0, #0
    9ac4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
    9ac8:	e1a00005 	mov	r0, r5
    9acc:	ebffe080 	bl	1cd4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0>
	if (offloaded) {
    9ad0:	e51b30b8 	ldr	r3, [fp, #-184]	@ 0xffffff48
    9ad4:	e3530000 	cmp	r3, #0
    9ad8:	0afffefd 	beq	96d4 <fast_classifier_conntrack_event+0x88>
		fast_classifier_send_genl_msg(FAST_CLASSIFIER_C_DONE, &fc_msg);
    9adc:	e24b1060 	sub	r1, fp, #96	@ 0x60
    9ae0:	e3a00003 	mov	r0, #3
    9ae4:	ebffdb70 	bl	8ac <fast_classifier_send_genl_msg>
}
    9ae8:	e3a00000 	mov	r0, #0
    9aec:	e24bd028 	sub	sp, fp, #40	@ 0x28
    9af0:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
			fc_msg.ethertype = AF_INET6;
    9af4:	e3a0000a 	mov	r0, #10
    9af8:	e14b06b0 	strh	r0, [fp, #-96]	@ 0xffffffa0
			fc_msg.src_saddr.in6 = *((struct in6_addr *)&conn->sic->src_ip);
    9afc:	e5910008 	ldr	r0, [r1, #8]
    9b00:	e1c021d8 	ldrd	r2, [r0, #24]
    9b04:	e14b25fc 	strd	r2, [fp, #-92]	@ 0xffffffa4
    9b08:	e1c022d0 	ldrd	r2, [r0, #32]
    9b0c:	e14b25f4 	strd	r2, [fp, #-84]	@ 0xffffffac
			fc_msg.dst_saddr.in6 = *((struct in6_addr *)&conn->sic->dest_ip_xlate);
    9b10:	e5910008 	ldr	r0, [r1, #8]
    9b14:	e1c024d8 	ldrd	r2, [r0, #72]	@ 0x48
    9b18:	e14b24fc 	strd	r2, [fp, #-76]	@ 0xffffffb4
    9b1c:	e1c025d0 	ldrd	r2, [r0, #80]	@ 0x50
    9b20:	e14b24f4 	strd	r2, [fp, #-68]	@ 0xffffffbc
		fc_msg.proto = conn->sic->protocol;
    9b24:	e5910008 	ldr	r0, [r1, #8]
    9b28:	e3a03001 	mov	r3, #1
    9b2c:	e50b30b8 	str	r3, [fp, #-184]	@ 0xffffff48
    9b30:	e590c000 	ldr	ip, [r0]
    9b34:	e54bc05e 	strb	ip, [fp, #-94]	@ 0xffffffa2
		fc_msg.sport = conn->sic->src_port;
    9b38:	e1d005b8 	ldrh	r0, [r0, #88]	@ 0x58
    9b3c:	e14b03bc 	strh	r0, [fp, #-60]	@ 0xffffffc4
		fc_msg.dport = conn->sic->dest_port_xlate;
    9b40:	e5910008 	ldr	r0, [r1, #8]
    9b44:	e1d005be 	ldrh	r0, [r0, #94]	@ 0x5e
    9b48:	e14b03ba 	strh	r0, [fp, #-58]	@ 0xffffffc6
		memcpy(fc_msg.smac, conn->smac, ETH_ALEN);
    9b4c:	e591001d 	ldr	r0, [r1, #29]
    9b50:	e50b0038 	str	r0, [fp, #-56]	@ 0xffffffc8
    9b54:	e1d102b1 	ldrh	r0, [r1, #33]	@ 0x21
    9b58:	e14b03b4 	strh	r0, [fp, #-52]	@ 0xffffffcc
		memcpy(fc_msg.dmac, conn->dmac, ETH_ALEN);
    9b5c:	e5910023 	ldr	r0, [r1, #35]	@ 0x23
    9b60:	e50b0032 	str	r0, [fp, #-50]	@ 0xffffffce
    9b64:	e1d102b7 	ldrh	r0, [r1, #39]	@ 0x27
    9b68:	e14b02be 	strh	r0, [fp, #-46]	@ 0xffffffd2
	return !h->pprev;
    9b6c:	e591c004 	ldr	ip, [r1, #4]
	if (!hlist_unhashed(n)) {
    9b70:	e35c0000 	cmp	ip, #0
    9b74:	1affff90 	bne	99bc <fast_classifier_conntrack_event+0x370>
    9b78:	eaffff96 	b	99d8 <fast_classifier_conntrack_event+0x38c>
		sfe_connections_size--;
    9b7c:	e5960020 	ldr	r0, [r6, #32]
		kfree(conn->sic);
    9b80:	e50b10b8 	str	r1, [fp, #-184]	@ 0xffffff48
		sfe_connections_size--;
    9b84:	e2400001 	sub	r0, r0, #1
    9b88:	e5860020 	str	r0, [r6, #32]
		kfree(conn->sic);
    9b8c:	e5910008 	ldr	r0, [r1, #8]
    9b90:	ebfffffe 	bl	0 <kfree>
		kfree(conn);
    9b94:	e51b10b8 	ldr	r1, [fp, #-184]	@ 0xffffff48
    9b98:	e1a00001 	mov	r0, r1
    9b9c:	ebfffffe 	bl	0 <kfree>
    9ba0:	e2860014 	add	r0, r6, #20
    9ba4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	int offloaded = 0;
    9ba8:	e3a03000 	mov	r3, #0
    9bac:	e50b30b8 	str	r3, [fp, #-184]	@ 0xffffff48
	raw_spin_lock_bh(&lock->rlock);
    9bb0:	e3006000 	movw	r6, #0
    9bb4:	e3406000 	movt	r6, #0
    9bb8:	e1a00006 	mov	r0, r6
    9bbc:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    9bc0:	e24b00a8 	sub	r0, fp, #168	@ 0xa8
    9bc4:	e51b7098 	ldr	r7, [fp, #-152]	@ 0xffffff68
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9bc8:	e02a1009 	eor	r1, sl, r9
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    9bcc:	e8905001 	ldm	r0, {r0, ip, lr}
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9bd0:	e6bf1fb1 	rev16	r1, r1
    9bd4:	e6ff1071 	uxth	r1, r1
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    9bd8:	e51b3094 	ldr	r3, [fp, #-148]	@ 0xffffff6c
    9bdc:	e51b2090 	ldr	r2, [fp, #-144]	@ 0xffffff70
    9be0:	e50b70c0 	str	r7, [fp, #-192]	@ 0xffffff40
    9be4:	e0277000 	eor	r7, r7, r0
    9be8:	e027700c 	eor	r7, r7, ip
    9bec:	e0277003 	eor	r7, r7, r3
    9bf0:	e027700e 	eor	r7, r7, lr
    9bf4:	e0277002 	eor	r7, r7, r2
    9bf8:	e0277005 	eor	r7, r7, r5
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9bfc:	e0277004 	eor	r7, r7, r4
    9c00:	e0211007 	eor	r1, r1, r7
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    9c04:	e0211621 	eor	r1, r1, r1, lsr #12
    9c08:	e7eb1051 	ubfx	r1, r1, #0, #12
	c = si->conn_hash[conn_idx];
    9c0c:	e281100c 	add	r1, r1, #12
    9c10:	e7965101 	ldr	r5, [r6, r1, lsl #2]
	si->connection_destroy_requests++;
    9c14:	e2866902 	add	r6, r6, #32768	@ 0x8000
    9c18:	e5961038 	ldr	r1, [r6, #56]	@ 0x38
	if (unlikely(!c)) {
    9c1c:	e3550000 	cmp	r5, #0
	si->connection_destroy_requests++;
    9c20:	e2811001 	add	r1, r1, #1
    9c24:	e5861038 	str	r1, [r6, #56]	@ 0x38
	if (unlikely(!c)) {
    9c28:	1a000003 	bne	9c3c <fast_classifier_conntrack_event+0x5f0>
    9c2c:	ea0000e6 	b	9fcc <fast_classifier_conntrack_event+0x980>
		c = c->next;
    9c30:	e5955000 	ldr	r5, [r5]
	} while (c && (c->src_port != src_port
    9c34:	e3550000 	cmp	r5, #0
    9c38:	0a0000e3 	beq	9fcc <fast_classifier_conntrack_event+0x980>
    9c3c:	e1d514bc 	ldrh	r1, [r5, #76]	@ 0x4c
    9c40:	e151000a 	cmp	r1, sl
    9c44:	1afffff9 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
		 || c->dest_port != dest_port
    9c48:	e1d515b0 	ldrh	r1, [r5, #80]	@ 0x50
    9c4c:	e1510009 	cmp	r1, r9
    9c50:	1afffff6 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
    9c54:	e595100c 	ldr	r1, [r5, #12]
    9c58:	e1580001 	cmp	r8, r1
    9c5c:	1afffff3 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	return a->addr[0] == b->addr[0] &&
    9c60:	e5951010 	ldr	r1, [r5, #16]
    9c64:	e1500001 	cmp	r0, r1
    9c68:	1afffff0 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	       a->addr[1] == b->addr[1] &&
    9c6c:	e5951014 	ldr	r1, [r5, #20]
    9c70:	e15c0001 	cmp	ip, r1
    9c74:	1affffed 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	       a->addr[2] == b->addr[2] &&
    9c78:	e5951018 	ldr	r1, [r5, #24]
    9c7c:	e15e0001 	cmp	lr, r1
    9c80:	1affffea 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
    9c84:	e51b70bc 	ldr	r7, [fp, #-188]	@ 0xffffff44
    9c88:	e595102c 	ldr	r1, [r5, #44]	@ 0x2c
    9c8c:	e1570001 	cmp	r7, r1
    9c90:	1affffe6 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	return a->addr[0] == b->addr[0] &&
    9c94:	e51b70c0 	ldr	r7, [fp, #-192]	@ 0xffffff40
    9c98:	e5951030 	ldr	r1, [r5, #48]	@ 0x30
    9c9c:	e1570001 	cmp	r7, r1
    9ca0:	1affffe2 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	       a->addr[1] == b->addr[1] &&
    9ca4:	e5951034 	ldr	r1, [r5, #52]	@ 0x34
    9ca8:	e1530001 	cmp	r3, r1
    9cac:	1affffdf 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	       a->addr[2] == b->addr[2] &&
    9cb0:	e5951038 	ldr	r1, [r5, #56]	@ 0x38
    9cb4:	e1520001 	cmp	r2, r1
    9cb8:	1affffdc 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
		 || c->protocol != protocol));
    9cbc:	e5951008 	ldr	r1, [r5, #8]
    9cc0:	e1540001 	cmp	r4, r1
    9cc4:	1affffd9 	bne	9c30 <fast_classifier_conntrack_event+0x5e4>
	sfe_ipv6_remove_connection(si, c);
    9cc8:	e1a00005 	mov	r0, r5
    9ccc:	ebffe1e4 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
    9cd0:	e3000000 	movw	r0, #0
    9cd4:	e3400000 	movt	r0, #0
    9cd8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
    9cdc:	e1a00005 	mov	r0, r5
    9ce0:	ebfff45b 	bl	6e54 <sfe_ipv6_flush_connection.constprop.0>
    9ce4:	eaffff79 	b	9ad0 <fast_classifier_conntrack_event+0x484>
	else if (likely(nf_ct_l3num(ct) == AF_INET6)) {
    9ce8:	e351000a 	cmp	r1, #10
    9cec:	1afffe78 	bne	96d4 <fast_classifier_conntrack_event+0x88>
		sid.src_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.src.u3.in6);
    9cf0:	e14b08d0 	ldrd	r0, [fp, #-128]	@ 0xffffff80
		is_v4 = false;
    9cf4:	e3a07000 	mov	r7, #0
		sid.src_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.src.u3.in6);
    9cf8:	e14b0af4 	strd	r0, [fp, #-164]	@ 0xffffff5c
		sid.dest_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.dst.u3.in6);
    9cfc:	e14b07d4 	ldrd	r0, [fp, #-116]	@ 0xffffff8c
    9d00:	e14b09fc 	strd	r0, [fp, #-156]	@ 0xffffff64
    9d04:	e14b06dc 	ldrd	r0, [fp, #-108]	@ 0xffffff94
    9d08:	e14b09f4 	strd	r0, [fp, #-148]	@ 0xffffff6c
		sid.src_ip.ip6[0] = *((struct sfe_ipv6_addr *)&orig_tuple.src.u3.in6);
    9d0c:	e14b08d8 	ldrd	r0, [fp, #-136]	@ 0xffffff78
    9d10:	e14b0afc 	strd	r0, [fp, #-172]	@ 0xffffff54
    9d14:	eafffe6a 	b	96c4 <fast_classifier_conntrack_event+0x78>
	    && (c->dest_port == dest_port)
    9d18:	e1d512b0 	ldrh	r1, [r5, #32]
    9d1c:	e1510009 	cmp	r1, r9
    9d20:	1affff50 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
	    && (c->src_ip == src_ip)
    9d24:	e595100c 	ldr	r1, [r5, #12]
    9d28:	e1580001 	cmp	r8, r1
    9d2c:	1affff4d 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
	    && (c->dest_ip == dest_ip)
    9d30:	e51b30bc 	ldr	r3, [fp, #-188]	@ 0xffffff44
    9d34:	e5951014 	ldr	r1, [r5, #20]
    9d38:	e1530001 	cmp	r3, r1
    9d3c:	1affff49 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
	    && (c->protocol == protocol)) {
    9d40:	e5951008 	ldr	r1, [r5, #8]
    9d44:	e1540001 	cmp	r4, r1
    9d48:	1affff46 	bne	9a68 <fast_classifier_conntrack_event+0x41c>
    9d4c:	eaffff58 	b	9ab4 <fast_classifier_conntrack_event+0x468>
	raw_spin_lock_bh(&lock->rlock);
    9d50:	e3008000 	movw	r8, #0
    9d54:	e50b30b8 	str	r3, [fp, #-184]	@ 0xffffff48
    9d58:	e3408000 	movt	r8, #0
    9d5c:	e1a00008 	mov	r0, r8
    9d60:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9d64:	e51b30b8 	ldr	r3, [fp, #-184]	@ 0xffffff48
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    9d68:	e51b005c 	ldr	r0, [fp, #-92]	@ 0xffffffa4
    9d6c:	e51be058 	ldr	lr, [fp, #-88]	@ 0xffffffa8
    9d70:	e51bc04c 	ldr	ip, [fp, #-76]	@ 0xffffffb4
    9d74:	e51b2054 	ldr	r2, [fp, #-84]	@ 0xffffffac
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9d78:	e6bf1fb3 	rev16	r1, r3
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    9d7c:	e51b3048 	ldr	r3, [fp, #-72]	@ 0xffffffb8
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9d80:	e6ff1071 	uxth	r1, r1
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
    9d84:	e50bc0c8 	str	ip, [fp, #-200]	@ 0xffffff38
    9d88:	e02cc000 	eor	ip, ip, r0
    9d8c:	e02cc00e 	eor	ip, ip, lr
    9d90:	e02cc003 	eor	ip, ip, r3
    9d94:	e50b30c4 	str	r3, [fp, #-196]	@ 0xffffff3c
    9d98:	e50b20b8 	str	r2, [fp, #-184]	@ 0xffffff48
    9d9c:	e51b2044 	ldr	r2, [fp, #-68]	@ 0xffffffbc
    9da0:	e51b30b8 	ldr	r3, [fp, #-184]	@ 0xffffff48
    9da4:	e50b20c0 	str	r2, [fp, #-192]	@ 0xffffff40
    9da8:	e51b2050 	ldr	r2, [fp, #-80]	@ 0xffffffb0
    9dac:	e02cc003 	eor	ip, ip, r3
    9db0:	e51b30c0 	ldr	r3, [fp, #-192]	@ 0xffffff40
    9db4:	e50b20bc 	str	r2, [fp, #-188]	@ 0xffffff44
    9db8:	e51b2040 	ldr	r2, [fp, #-64]	@ 0xffffffc0
    9dbc:	e02cc003 	eor	ip, ip, r3
    9dc0:	e51b30bc 	ldr	r3, [fp, #-188]	@ 0xffffff44
    9dc4:	e02cc003 	eor	ip, ip, r3
    9dc8:	e02cc002 	eor	ip, ip, r2
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
    9dcc:	e02cc004 	eor	ip, ip, r4
    9dd0:	e021100c 	eor	r1, r1, ip
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
    9dd4:	e0211621 	eor	r1, r1, r1, lsr #12
    9dd8:	e7eb1051 	ubfx	r1, r1, #0, #12
	c = si->conn_hash[conn_idx];
    9ddc:	e281100c 	add	r1, r1, #12
    9de0:	e7981101 	ldr	r1, [r8, r1, lsl #2]
	if (unlikely(!c)) {
    9de4:	e3510000 	cmp	r1, #0
    9de8:	0a000030 	beq	9eb0 <fast_classifier_conntrack_event+0x864>
	if ((c->src_port == src_port)
    9dec:	e1d1c4bc 	ldrh	ip, [r1, #76]	@ 0x4c
    9df0:	e15c000a 	cmp	ip, sl
    9df4:	0a000040 	beq	9efc <fast_classifier_conntrack_event+0x8b0>
		c = c->next;
    9df8:	e5911000 	ldr	r1, [r1]
	} while (c && (c->src_port != src_port
    9dfc:	e3510000 	cmp	r1, #0
    9e00:	151b30b8 	ldrne	r3, [fp, #-184]	@ 0xffffff48
    9e04:	1a000003 	bne	9e18 <fast_classifier_conntrack_event+0x7cc>
    9e08:	ea000028 	b	9eb0 <fast_classifier_conntrack_event+0x864>
		c = c->next;
    9e0c:	e5911000 	ldr	r1, [r1]
	} while (c && (c->src_port != src_port
    9e10:	e3510000 	cmp	r1, #0
    9e14:	0a000025 	beq	9eb0 <fast_classifier_conntrack_event+0x864>
    9e18:	e1d1c4bc 	ldrh	ip, [r1, #76]	@ 0x4c
    9e1c:	e15c000a 	cmp	ip, sl
    9e20:	1afffff9 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
		 || c->dest_port != dest_port
    9e24:	e1d1c5b0 	ldrh	ip, [r1, #80]	@ 0x50
    9e28:	e15c0009 	cmp	ip, r9
    9e2c:	1afffff6 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
    9e30:	e591c00c 	ldr	ip, [r1, #12]
    9e34:	e15c0000 	cmp	ip, r0
    9e38:	1afffff3 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
	return a->addr[0] == b->addr[0] &&
    9e3c:	e591c010 	ldr	ip, [r1, #16]
    9e40:	e15c000e 	cmp	ip, lr
    9e44:	1afffff0 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
	       a->addr[1] == b->addr[1] &&
    9e48:	e591c014 	ldr	ip, [r1, #20]
    9e4c:	e15c0003 	cmp	ip, r3
    9e50:	1affffed 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
	       a->addr[2] == b->addr[2] &&
    9e54:	e51b80bc 	ldr	r8, [fp, #-188]	@ 0xffffff44
    9e58:	e591c018 	ldr	ip, [r1, #24]
    9e5c:	e158000c 	cmp	r8, ip
    9e60:	1affffe9 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
    9e64:	e51b80c8 	ldr	r8, [fp, #-200]	@ 0xffffff38
    9e68:	e591c02c 	ldr	ip, [r1, #44]	@ 0x2c
    9e6c:	e15c0008 	cmp	ip, r8
    9e70:	1affffe5 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
	return a->addr[0] == b->addr[0] &&
    9e74:	e51b80c4 	ldr	r8, [fp, #-196]	@ 0xffffff3c
    9e78:	e591c030 	ldr	ip, [r1, #48]	@ 0x30
    9e7c:	e15c0008 	cmp	ip, r8
    9e80:	1affffe1 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
	       a->addr[1] == b->addr[1] &&
    9e84:	e51b80c0 	ldr	r8, [fp, #-192]	@ 0xffffff40
    9e88:	e591c034 	ldr	ip, [r1, #52]	@ 0x34
    9e8c:	e15c0008 	cmp	ip, r8
    9e90:	1affffdd 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
	       a->addr[2] == b->addr[2] &&
    9e94:	e591c038 	ldr	ip, [r1, #56]	@ 0x38
    9e98:	e152000c 	cmp	r2, ip
    9e9c:	1affffda 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
		 || c->protocol != protocol));
    9ea0:	e591c008 	ldr	ip, [r1, #8]
    9ea4:	e154000c 	cmp	r4, ip
    9ea8:	1affffd7 	bne	9e0c <fast_classifier_conntrack_event+0x7c0>
		c->mark = mark->mark;
    9eac:	e5816078 	str	r6, [r1, #120]	@ 0x78
	raw_spin_unlock_bh(&lock->rlock);
    9eb0:	e3000000 	movw	r0, #0
    9eb4:	e3400000 	movt	r0, #0
    9eb8:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	raw_spin_lock_bh(&lock->rlock);
    9ebc:	e59f014c 	ldr	r0, [pc, #332]	@ a010 <fast_classifier_conntrack_event+0x9c4>
    9ec0:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	conn = fast_classifier_find_conn(&mark->src_ip, &mark->dest_ip,
    9ec4:	e3a0c000 	mov	ip, #0
    9ec8:	e24b104c 	sub	r1, fp, #76	@ 0x4c
    9ecc:	e24b005c 	sub	r0, fp, #92	@ 0x5c
    9ed0:	e1a03009 	mov	r3, r9
    9ed4:	e1a0200a 	mov	r2, sl
    9ed8:	e88d1010 	stm	sp, {r4, ip}
    9edc:	ebffe699 	bl	3948 <fast_classifier_find_conn>
	if (conn) {
    9ee0:	e2501000 	subs	r1, r0, #0
    9ee4:	0a000001 	beq	9ef0 <fast_classifier_conntrack_event+0x8a4>
		conn->sic->mark = mark->mark;
    9ee8:	e5911008 	ldr	r1, [r1, #8]
    9eec:	e5816098 	str	r6, [r1, #152]	@ 0x98
	raw_spin_unlock_bh(&lock->rlock);
    9ef0:	e59f0118 	ldr	r0, [pc, #280]	@ a010 <fast_classifier_conntrack_event+0x9c4>
    9ef4:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    9ef8:	eafffdfc 	b	96f0 <fast_classifier_conntrack_event+0xa4>
	    && (c->dest_port == dest_port)
    9efc:	e1d1c5b0 	ldrh	ip, [r1, #80]	@ 0x50
    9f00:	e15c0009 	cmp	ip, r9
    9f04:	1affffbb 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
    9f08:	e591c00c 	ldr	ip, [r1, #12]
    9f0c:	e15c0000 	cmp	ip, r0
    9f10:	1affffb8 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	return a->addr[0] == b->addr[0] &&
    9f14:	e591c010 	ldr	ip, [r1, #16]
    9f18:	e15c000e 	cmp	ip, lr
    9f1c:	1affffb5 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	       a->addr[1] == b->addr[1] &&
    9f20:	e51b30b8 	ldr	r3, [fp, #-184]	@ 0xffffff48
    9f24:	e591c014 	ldr	ip, [r1, #20]
    9f28:	e15c0003 	cmp	ip, r3
    9f2c:	1affffb1 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	       a->addr[2] == b->addr[2] &&
    9f30:	e51b30bc 	ldr	r3, [fp, #-188]	@ 0xffffff44
    9f34:	e591c018 	ldr	ip, [r1, #24]
    9f38:	e153000c 	cmp	r3, ip
    9f3c:	1affffad 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
    9f40:	e51b30c8 	ldr	r3, [fp, #-200]	@ 0xffffff38
    9f44:	e591c02c 	ldr	ip, [r1, #44]	@ 0x2c
    9f48:	e15c0003 	cmp	ip, r3
    9f4c:	1affffa9 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	return a->addr[0] == b->addr[0] &&
    9f50:	e51b30c4 	ldr	r3, [fp, #-196]	@ 0xffffff3c
    9f54:	e591c030 	ldr	ip, [r1, #48]	@ 0x30
    9f58:	e15c0003 	cmp	ip, r3
    9f5c:	1affffa5 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	       a->addr[1] == b->addr[1] &&
    9f60:	e51b30c0 	ldr	r3, [fp, #-192]	@ 0xffffff40
    9f64:	e591c034 	ldr	ip, [r1, #52]	@ 0x34
    9f68:	e15c0003 	cmp	ip, r3
    9f6c:	1affffa1 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	       a->addr[2] == b->addr[2] &&
    9f70:	e591c038 	ldr	ip, [r1, #56]	@ 0x38
    9f74:	e152000c 	cmp	r2, ip
    9f78:	1affff9e 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
	    && (c->protocol == protocol)) {
    9f7c:	e591c008 	ldr	ip, [r1, #8]
    9f80:	e154000c 	cmp	r4, ip
    9f84:	1affff9b 	bne	9df8 <fast_classifier_conntrack_event+0x7ac>
    9f88:	eaffffc7 	b	9eac <fast_classifier_conntrack_event+0x860>
	return !h->pprev;
    9f8c:	e591c004 	ldr	ip, [r1, #4]
	if (!hlist_unhashed(n)) {
    9f90:	e35c0000 	cmp	ip, #0
    9f94:	1afffe88 	bne	99bc <fast_classifier_conntrack_event+0x370>
		sfe_connections_size--;
    9f98:	e596c020 	ldr	ip, [r6, #32]
    9f9c:	e50b10b8 	str	r1, [fp, #-184]	@ 0xffffff48
    9fa0:	e24cc001 	sub	ip, ip, #1
    9fa4:	e586c020 	str	ip, [r6, #32]
		kfree(conn->sic);
    9fa8:	ebfffffe 	bl	0 <kfree>
		kfree(conn);
    9fac:	e51b10b8 	ldr	r1, [fp, #-184]	@ 0xffffff48
    9fb0:	e1a00001 	mov	r0, r1
    9fb4:	ebfffffe 	bl	0 <kfree>
    9fb8:	e59f0050 	ldr	r0, [pc, #80]	@ a010 <fast_classifier_conntrack_event+0x9c4>
    9fbc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	is_v4 ? sfe_ipv4_destroy_rule(&sid) : sfe_ipv6_destroy_rule(&sid);
    9fc0:	e3a03000 	mov	r3, #0
    9fc4:	e50b30b8 	str	r3, [fp, #-184]	@ 0xffffff48
    9fc8:	eafffe8f 	b	9a0c <fast_classifier_conntrack_event+0x3c0>
		si->connection_destroy_misses++;
    9fcc:	e596303c 	ldr	r3, [r6, #60]	@ 0x3c
    9fd0:	e3000000 	movw	r0, #0
    9fd4:	e3400000 	movt	r0, #0
    9fd8:	e2833001 	add	r3, r3, #1
    9fdc:	e586303c 	str	r3, [r6, #60]	@ 0x3c
    9fe0:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return;
    9fe4:	eafffeb9 	b	9ad0 <fast_classifier_conntrack_event+0x484>
		si->connection_destroy_misses++;
    9fe8:	e596303c 	ldr	r3, [r6, #60]	@ 0x3c
    9fec:	e3000000 	movw	r0, #0
    9ff0:	e3400000 	movt	r0, #0
    9ff4:	e2833001 	add	r3, r3, #1
    9ff8:	e586303c 	str	r3, [r6, #60]	@ 0x3c
    9ffc:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
		return;
    a000:	eafffeb2 	b	9ad0 <fast_classifier_conntrack_event+0x484>
    a004:	e59f0004 	ldr	r0, [pc, #4]	@ a010 <fast_classifier_conntrack_event+0x9c4>
    a008:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
    a00c:	eaffffeb 	b	9fc0 <fast_classifier_conntrack_event+0x974>
    a010:	00000014 	.word	0x00000014

Disassembly of section .init.text:

00000000 <init_module>:
#endif
/*
 * fast_classifier_init()
 */
static int __init fast_classifier_init(void)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddff0 	push	{r4, r5, r6, r7, r8, r9, sl, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
   c:	e24dd014 	sub	sp, sp, #20
	DEBUG_INFO("SFE IPv4 init\n");

	/*
	 * Create sys/sfe_ipv4
	 */
	si->sys_sfe_ipv4 = kobject_create_and_add("sfe_ipv4", NULL);
  10:	e3000000 	movw	r0, #0
  14:	e3a01000 	mov	r1, #0
  18:	e3400000 	movt	r0, #0
  1c:	e3007000 	movw	r7, #0
  20:	ebfffffe 	bl	0 <kobject_create_and_add>
  24:	e3407000 	movt	r7, #0
	if (!si->sys_sfe_ipv4) {
  28:	e3500000 	cmp	r0, #0
	si->sys_sfe_ipv4 = kobject_create_and_add("sfe_ipv4", NULL);
  2c:	e2875902 	add	r5, r7, #32768	@ 0x8000
  30:	e5850258 	str	r0, [r5, #600]	@ 0x258
	if (!si->sys_sfe_ipv4) {
  34:	0a000024 	beq	cc <init_module+0xcc>
#endif /* CONFIG_SYSFS */

static inline int __must_check sysfs_create_file(struct kobject *kobj,
						 const struct attribute *attr)
{
	return sysfs_create_file_ns(kobj, attr, NULL);
  38:	e59f14a8 	ldr	r1, [pc, #1192]	@ 4e8 <init_module+0x4e8>
  3c:	e3a02000 	mov	r2, #0
  40:	ebfffffe 	bl	0 <sysfs_create_file_ns>

	/*
	 * Create files, one for each parameter supported by this module.
	 */
	result = sysfs_create_file(si->sys_sfe_ipv4, &sfe_ipv4_debug_dev_attr.attr);
	if (result) {
  44:	e2504000 	subs	r4, r0, #0
  48:	1a00001d 	bne	c4 <init_module+0xc4>
extern void chrdev_show(struct seq_file *,off_t);

static inline int register_chrdev(unsigned int major, const char *name,
				  const struct file_operations *fops)
{
	return __register_chrdev(major, 0, 256, name, fops);
  4c:	e59f2498 	ldr	r2, [pc, #1176]	@ 4ec <init_module+0x4ec>
  50:	e3003000 	movw	r3, #0
  54:	e1a01004 	mov	r1, r4
  58:	e3403000 	movt	r3, #0
  5c:	e58d2000 	str	r2, [sp]
  60:	e3a02c01 	mov	r2, #256	@ 0x100
  64:	ebfffffe 	bl	0 <__register_chrdev>

	/*
	 * Register our debug char device.
	 */
	result = register_chrdev(0, "sfe_ipv4", &sfe_ipv4_debug_dev_fops);
	if (result < 0) {
  68:	e3500000 	cmp	r0, #0
  6c:	aa000004 	bge	84 <init_module+0x84>
}

static inline void sysfs_remove_file(struct kobject *kobj,
				     const struct attribute *attr)
{
	sysfs_remove_file_ns(kobj, attr, NULL);
  70:	e5950258 	ldr	r0, [r5, #600]	@ 0x258
  74:	e1a02004 	mov	r2, r4
  78:	e59f1468 	ldr	r1, [pc, #1128]	@ 4e8 <init_module+0x4e8>
  7c:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
}
  80:	ea00000f 	b	c4 <init_module+0xc4>
	si->debug_dev = result;

	/*
	 * Create a timer to handle periodic statistics.
	 */
	timer_setup(&si->timer, sfe_ipv4_periodic_sync, 0);
  84:	e3001000 	movw	r1, #0
  88:	e1a03004 	mov	r3, r4
  8c:	e58d4000 	str	r4, [sp]
  90:	e3401000 	movt	r1, #0
  94:	e1a02004 	mov	r2, r4
	si->debug_dev = result;
  98:	e585025c 	str	r0, [r5, #604]	@ 0x25c
	timer_setup(&si->timer, sfe_ipv4_periodic_sync, 0);
  9c:	e2870018 	add	r0, r7, #24
  a0:	ebfffffe 	bl	0 <init_timer_key>
	mod_timer(&si->timer, jiffies + ((HZ + 99) / 100));
  a4:	e3003000 	movw	r3, #0
  a8:	e2870018 	add	r0, r7, #24
  ac:	e3403000 	movt	r3, #0
  b0:	e5931000 	ldr	r1, [r3]
  b4:	e2811001 	add	r1, r1, #1
  b8:	ebfffffe 	bl	0 <mod_timer>

	spin_lock_init(&si->lock);
  bc:	e5874000 	str	r4, [r7]

	return 0;
  c0:	ea000001 	b	cc <init_module+0xcc>
exit3:
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_debug_dev_attr.attr);

exit2:
	kobject_put(si->sys_sfe_ipv4);
  c4:	e5950258 	ldr	r0, [r5, #600]	@ 0x258
  c8:	ebfffffe 	bl	0 <kobject_put>
	DEBUG_INFO("SFE IPv6 init\n");

	/*
	 * Create sys/sfe_ipv6
	 */
	si->sys_sfe_ipv6 = kobject_create_and_add("sfe_ipv6", NULL);
  cc:	e3000000 	movw	r0, #0
  d0:	e3a01000 	mov	r1, #0
  d4:	e3400000 	movt	r0, #0
  d8:	e3006000 	movw	r6, #0
  dc:	ebfffffe 	bl	0 <kobject_create_and_add>
  e0:	e3406000 	movt	r6, #0
	if (!si->sys_sfe_ipv6) {
  e4:	e3500000 	cmp	r0, #0
	si->sys_sfe_ipv6 = kobject_create_and_add("sfe_ipv6", NULL);
  e8:	e2865902 	add	r5, r6, #32768	@ 0x8000
  ec:	e5850268 	str	r0, [r5, #616]	@ 0x268
	if (!si->sys_sfe_ipv6) {
  f0:	0a000021 	beq	17c <init_module+0x17c>
	return sysfs_create_file_ns(kobj, attr, NULL);
  f4:	e59f13f4 	ldr	r1, [pc, #1012]	@ 4f0 <init_module+0x4f0>
  f8:	e3a02000 	mov	r2, #0
  fc:	ebfffffe 	bl	0 <sysfs_create_file_ns>

	/*
	 * Create files, one for each parameter supported by this module.
	 */
	result = sysfs_create_file(si->sys_sfe_ipv6, &sfe_ipv6_debug_dev_attr.attr);
	if (result) {
 100:	e2504000 	subs	r4, r0, #0
 104:	1a00002b 	bne	1b8 <init_module+0x1b8>
 108:	e59f33e4 	ldr	r3, [pc, #996]	@ 4f4 <init_module+0x4f4>
 10c:	e3a02c01 	mov	r2, #256	@ 0x100
 110:	e1a01004 	mov	r1, r4
 114:	e58d3000 	str	r3, [sp]
 118:	e3003000 	movw	r3, #0
 11c:	e3403000 	movt	r3, #0
 120:	ebfffffe 	bl	0 <__register_chrdev>

	/*
	 * Register our debug char device.
	 */
	result = register_chrdev(0, "sfe_ipv6", &sfe_ipv6_debug_dev_fops);
	if (result < 0) {
 124:	e3500000 	cmp	r0, #0
 128:	aa000004 	bge	140 <init_module+0x140>
	sysfs_remove_file_ns(kobj, attr, NULL);
 12c:	e5950268 	ldr	r0, [r5, #616]	@ 0x268
 130:	e1a02004 	mov	r2, r4
 134:	e59f13b4 	ldr	r1, [pc, #948]	@ 4f0 <init_module+0x4f0>
 138:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
}
 13c:	ea00001d 	b	1b8 <init_module+0x1b8>
	si->debug_dev = result;

	/*
	 * Create a timer to handle periodic statistics.
	 */
	timer_setup(&si->timer, sfe_ipv6_periodic_sync, 0);
 140:	e3001000 	movw	r1, #0
 144:	e1a03004 	mov	r3, r4
 148:	e58d4000 	str	r4, [sp]
 14c:	e3401000 	movt	r1, #0
 150:	e1a02004 	mov	r2, r4
	si->debug_dev = result;
 154:	e585026c 	str	r0, [r5, #620]	@ 0x26c
	timer_setup(&si->timer, sfe_ipv6_periodic_sync, 0);
 158:	e2860018 	add	r0, r6, #24
 15c:	ebfffffe 	bl	0 <init_timer_key>
	mod_timer(&si->timer, jiffies + ((HZ + 99) / 100));
 160:	e3003000 	movw	r3, #0
 164:	e2860018 	add	r0, r6, #24
 168:	e3403000 	movt	r3, #0
 16c:	e5931000 	ldr	r1, [r3]
 170:	e2811001 	add	r1, r1, #1
 174:	ebfffffe 	bl	0 <mod_timer>

	spin_lock_init(&si->lock);
 178:	e5864000 	str	r4, [r6]
static inline void __hash_init(struct hlist_head *ht, unsigned int sz)
{
	unsigned int i;

	for (i = 0; i < sz; i++)
		INIT_HLIST_HEAD(&ht[i]);
 17c:	e3000000 	movw	r0, #0
 180:	e3a02902 	mov	r2, #32768	@ 0x8000
 184:	e3a01000 	mov	r1, #0
 188:	e3400000 	movt	r0, #0
 18c:	ebfffffe 	bl	0 <memset>
	hash_init(fc_conn_ht);

	/*
	 * Create sys/fast_classifier
	 */
	sc->sys_fast_classifier = kobject_create_and_add("fast_classifier", NULL);
 190:	e3000000 	movw	r0, #0
 194:	e3a01000 	mov	r1, #0
 198:	e3400000 	movt	r0, #0
 19c:	e3005000 	movw	r5, #0
 1a0:	ebfffffe 	bl	0 <kobject_create_and_add>
 1a4:	e3405000 	movt	r5, #0
	if (!sc->sys_fast_classifier) {
 1a8:	e3500000 	cmp	r0, #0
	sc->sys_fast_classifier = kobject_create_and_add("fast_classifier", NULL);
 1ac:	e5850028 	str	r0, [r5, #40]	@ 0x28
	if (!sc->sys_fast_classifier) {
 1b0:	0a0000c8 	beq	4d8 <init_module+0x4d8>
 1b4:	ea000002 	b	1c4 <init_module+0x1c4>
exit3:
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_sfe_ipv6, &sfe_ipv6_debug_dev_attr.attr);

exit2:
	kobject_put(si->sys_sfe_ipv6);
 1b8:	e5950268 	ldr	r0, [r5, #616]	@ 0x268
 1bc:	ebfffffe 	bl	0 <kobject_put>
 1c0:	eaffffed 	b	17c <init_module+0x17c>
	return sysfs_create_file_ns(kobj, attr, NULL);
 1c4:	e59f132c 	ldr	r1, [pc, #812]	@ 4f8 <init_module+0x4f8>
 1c8:	e3a02000 	mov	r2, #0
 1cc:	ebfffffe 	bl	0 <sysfs_create_file_ns>
		DEBUG_ERROR("failed to register fast_classifier\n");
		goto exit1;
	}

	result = sysfs_create_file(sc->sys_fast_classifier, &fast_classifier_offload_at_pkts_attr.attr);
	if (result) {
 1d0:	e2504000 	subs	r4, r0, #0
 1d4:	1a0000bc 	bne	4cc <init_module+0x4cc>
 1d8:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 1dc:	e1a02004 	mov	r2, r4
 1e0:	e59f1314 	ldr	r1, [pc, #788]	@ 4fc <init_module+0x4fc>
 1e4:	ebfffffe 	bl	0 <sysfs_create_file_ns>
		DEBUG_ERROR("failed to register offload at pkgs: %d\n", result);
		goto exit2;
	}

	result = sysfs_create_file(sc->sys_fast_classifier, &fast_classifier_debug_info_attr.attr);
	if (result) {
 1e8:	e2504000 	subs	r4, r0, #0
	sysfs_remove_file_ns(kobj, attr, NULL);
 1ec:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 1f0:	0a000003 	beq	204 <init_module+0x204>
 1f4:	e59f12fc 	ldr	r1, [pc, #764]	@ 4f8 <init_module+0x4f8>
 1f8:	e3a02000 	mov	r2, #0
 1fc:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
		DEBUG_ERROR("failed to register debug dev: %d\n", result);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_offload_at_pkts_attr.attr);
		goto exit2;
 200:	ea0000b1 	b	4cc <init_module+0x4cc>
	return sysfs_create_file_ns(kobj, attr, NULL);
 204:	e1a02004 	mov	r2, r4
 208:	e59f12f0 	ldr	r1, [pc, #752]	@ 500 <init_module+0x500>
 20c:	ebfffffe 	bl	0 <sysfs_create_file_ns>
	}

	result = sysfs_create_file(sc->sys_fast_classifier, &fast_classifier_skip_bridge_ingress.attr);
	if (result) {
 210:	e2504000 	subs	r4, r0, #0
	sysfs_remove_file_ns(kobj, attr, NULL);
 214:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 218:	0a000007 	beq	23c <init_module+0x23c>
 21c:	e59f12d4 	ldr	r1, [pc, #724]	@ 4f8 <init_module+0x4f8>
 220:	e3a02000 	mov	r2, #0
 224:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 228:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 22c:	e3a02000 	mov	r2, #0
 230:	e59f12c4 	ldr	r1, [pc, #708]	@ 4fc <init_module+0x4fc>
 234:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
		DEBUG_ERROR("failed to register skip bridge on ingress: %d\n", result);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_offload_at_pkts_attr.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_debug_info_attr.attr);
		goto exit2;
 238:	ea0000a3 	b	4cc <init_module+0x4cc>
	return sysfs_create_file_ns(kobj, attr, NULL);
 23c:	e1a02004 	mov	r2, r4
 240:	e59f12bc 	ldr	r1, [pc, #700]	@ 504 <init_module+0x504>
 244:	ebfffffe 	bl	0 <sysfs_create_file_ns>
	}

	result = sysfs_create_file(sc->sys_fast_classifier, &fast_classifier_stop.attr);
	if (result) {
 248:	e2504000 	subs	r4, r0, #0
	sysfs_remove_file_ns(kobj, attr, NULL);
 24c:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 250:	0a00000b 	beq	284 <init_module+0x284>
 254:	e59f129c 	ldr	r1, [pc, #668]	@ 4f8 <init_module+0x4f8>
 258:	e3a02000 	mov	r2, #0
 25c:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 260:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 264:	e3a02000 	mov	r2, #0
 268:	e59f128c 	ldr	r1, [pc, #652]	@ 4fc <init_module+0x4fc>
 26c:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 270:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 274:	e3a02000 	mov	r2, #0
 278:	e59f1280 	ldr	r1, [pc, #640]	@ 500 <init_module+0x500>
 27c:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
		DEBUG_ERROR("failed to register skip bridge on ingress: %d\n", result);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_offload_at_pkts_attr.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_debug_info_attr.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_skip_bridge_ingress.attr);
		goto exit2;
 280:	ea000091 	b	4cc <init_module+0x4cc>
	return sysfs_create_file_ns(kobj, attr, NULL);
 284:	e1a02004 	mov	r2, r4
 288:	e59f1278 	ldr	r1, [pc, #632]	@ 508 <init_module+0x508>
 28c:	ebfffffe 	bl	0 <sysfs_create_file_ns>
	}

	result = sysfs_create_file(sc->sys_fast_classifier, &fast_classifier_defunct_all.attr);
	if (result) {
 290:	e2504000 	subs	r4, r0, #0
 294:	0a000010 	beq	2dc <init_module+0x2dc>
	sysfs_remove_file_ns(kobj, attr, NULL);
 298:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 29c:	e3a02000 	mov	r2, #0
 2a0:	e59f1250 	ldr	r1, [pc, #592]	@ 4f8 <init_module+0x4f8>
 2a4:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 2a8:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 2ac:	e3a02000 	mov	r2, #0
 2b0:	e59f1244 	ldr	r1, [pc, #580]	@ 4fc <init_module+0x4fc>
 2b4:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 2b8:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 2bc:	e3a02000 	mov	r2, #0
 2c0:	e59f1238 	ldr	r1, [pc, #568]	@ 500 <init_module+0x500>
 2c4:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 2c8:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 2cc:	e3a02000 	mov	r2, #0
 2d0:	e59f122c 	ldr	r1, [pc, #556]	@ 504 <init_module+0x504>
 2d4:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
		DEBUG_ERROR("failed to register skip bridge on ingress: %d\n", result);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_offload_at_pkts_attr.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_debug_info_attr.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_skip_bridge_ingress.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_stop.attr);
		goto exit2;
 2d8:	ea00007b 	b	4cc <init_module+0x4cc>
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_stop.attr);
		sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_defunct_all.attr);
		goto exit2;
	}
#endif
	sc->dev_notifier.notifier_call = fast_classifier_device_event;
 2dc:	e3003000 	movw	r3, #0
	sc->dev_notifier.priority = 1;
 2e0:	e3a04001 	mov	r4, #1
	sc->dev_notifier.notifier_call = fast_classifier_device_event;
 2e4:	e3403000 	movt	r3, #0
	register_netdevice_notifier(&sc->dev_notifier);
 2e8:	e285002c 	add	r0, r5, #44	@ 0x2c
	register_inet6addr_notifier(&sc->inet6_notifier);
#endif
	/*
	 * Register our netfilter hooks.
	 */
	for_each_net(net) {
 2ec:	e3008000 	movw	r8, #0
 2f0:	e285a038 	add	sl, r5, #56	@ 0x38
	sc->dev_notifier.notifier_call = fast_classifier_device_event;
 2f4:	e585302c 	str	r3, [r5, #44]	@ 0x2c
	for_each_net(net) {
 2f8:	e3408000 	movt	r8, #0
	sc->dev_notifier.priority = 1;
 2fc:	e5854034 	str	r4, [r5, #52]	@ 0x34
	register_netdevice_notifier(&sc->dev_notifier);
 300:	ebfffffe 	bl	0 <register_netdevice_notifier>
	sc->inet_notifier.notifier_call = fast_classifier_inet_event;
 304:	e3003000 	movw	r3, #0
	register_inetaddr_notifier(&sc->inet_notifier);
 308:	e2850038 	add	r0, r5, #56	@ 0x38
	sc->inet_notifier.priority = 1;
 30c:	e5854040 	str	r4, [r5, #64]	@ 0x40
	sc->inet_notifier.notifier_call = fast_classifier_inet_event;
 310:	e3403000 	movt	r3, #0
 314:	e5853038 	str	r3, [r5, #56]	@ 0x38
	register_inetaddr_notifier(&sc->inet_notifier);
 318:	ebfffffe 	bl	0 <register_inetaddr_notifier>
	sc->inet6_notifier.notifier_call = fast_classifier_inet6_event;
 31c:	e3003000 	movw	r3, #0
	register_inet6addr_notifier(&sc->inet6_notifier);
 320:	e2850044 	add	r0, r5, #68	@ 0x44
	sc->inet6_notifier.priority = 1;
 324:	e585404c 	str	r4, [r5, #76]	@ 0x4c
	sc->inet6_notifier.notifier_call = fast_classifier_inet6_event;
 328:	e3403000 	movt	r3, #0
 32c:	e5853044 	str	r3, [r5, #68]	@ 0x44
	register_inet6addr_notifier(&sc->inet6_notifier);
 330:	ebfffffe 	bl	0 <register_inet6addr_notifier>
	for_each_net(net) {
 334:	e5983000 	ldr	r3, [r8]
	result = nf_register_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
 338:	e3001000 	movw	r1, #0
 33c:	e3401000 	movt	r1, #0
	for_each_net(net) {
 340:	e243901c 	sub	r9, r3, #28
 344:	e289301c 	add	r3, r9, #28
 348:	e1530008 	cmp	r3, r8
 34c:	0a000009 	beq	378 <init_module+0x378>
	result = nf_register_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
 350:	e3a02002 	mov	r2, #2
 354:	e1a00009 	mov	r0, r9
 358:	e50b1030 	str	r1, [fp, #-48]	@ 0xffffffd0
 35c:	ebfffffe 	bl	0 <nf_register_net_hooks>
	if (result < 0) {
 360:	e2504000 	subs	r4, r0, #0
 364:	e51b1030 	ldr	r1, [fp, #-48]	@ 0xffffffd0
 368:	ba00003d 	blt	464 <init_module+0x464>
	for_each_net(net) {
 36c:	e599901c 	ldr	r9, [r9, #28]
 370:	e249901c 	sub	r9, r9, #28
 374:	eafffff2 	b	344 <init_module+0x344>

#ifdef CONFIG_NF_CONNTRACK_EVENTS
	/*
	 * Register a notifier hook to get fast notifications of expired connections.
	 */
	result = nf_conntrack_register_notifier(&init_net, &fast_classifier_conntrack_notifier);
 378:	e3000000 	movw	r0, #0
 37c:	e59f1188 	ldr	r1, [pc, #392]	@ 50c <init_module+0x50c>
 380:	e3400000 	movt	r0, #0
 384:	ebfffffe 	bl	0 <nf_conntrack_register_notifier>
	if (result < 0) {
 388:	e2504000 	subs	r4, r0, #0
 38c:	ba000026 	blt	42c <init_module+0x42c>
		DEBUG_ERROR("can't register nf notifier hook: %d\n", result);
		goto exit4;
	}
#endif

	result = genl_register_family(&fast_classifier_gnl_family);
 390:	e3000000 	movw	r0, #0
 394:	e3400000 	movt	r0, #0
 398:	ebfffffe 	bl	0 <genl_register_family>
	if (result) {
 39c:	e2504000 	subs	r4, r0, #0
 3a0:	0a000004 	beq	3b8 <init_module+0x3b8>
	genl_unregister_family(&fast_classifier_gnl_family);
#endif

exit5:
#ifdef CONFIG_NF_CONNTRACK_EVENTS
	nf_conntrack_unregister_notifier(&init_net, &fast_classifier_conntrack_notifier);
 3a4:	e3000000 	movw	r0, #0
 3a8:	e59f115c 	ldr	r1, [pc, #348]	@ 50c <init_module+0x50c>
 3ac:	e3400000 	movt	r0, #0
 3b0:	ebfffffe 	bl	0 <nf_conntrack_unregister_notifier>
 3b4:	ea00001c 	b	42c <init_module+0x42c>
	BUG_ON(fast_nat_recv);
 3b8:	e3003000 	movw	r3, #0
	spin_lock_init(&sc->lock);
 3bc:	e5854024 	str	r4, [r5, #36]	@ 0x24
	BUG_ON(fast_nat_recv);
 3c0:	e3403000 	movt	r3, #0
 3c4:	e5932000 	ldr	r2, [r3]
 3c8:	e3520000 	cmp	r2, #0
 3cc:	0a000000 	beq	3d4 <init_module+0x3d4>
 3d0:	e7f001f2 	.word	0xe7f001f2
	RCU_INIT_POINTER(fast_nat_recv, fast_classifier_recv);
 3d4:	e3002000 	movw	r2, #0
	raw_spin_lock_bh(&lock->rlock);
 3d8:	e3000000 	movw	r0, #0
 3dc:	e3402000 	movt	r2, #0
 3e0:	e3400000 	movt	r0, #0
 3e4:	e5832000 	str	r2, [r3]
 3e8:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	rcu_assign_pointer(si->sync_rule_callback, sync_rule_callback);
 3ec:	f57ff05b 	dmb	ish
 3f0:	e3005000 	movw	r5, #0
	raw_spin_unlock_bh(&lock->rlock);
 3f4:	e3000000 	movw	r0, #0
 3f8:	e3405000 	movt	r5, #0
 3fc:	e3400000 	movt	r0, #0
 400:	e587502c 	str	r5, [r7, #44]	@ 0x2c
 404:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	raw_spin_lock_bh(&lock->rlock);
 408:	e3000000 	movw	r0, #0
 40c:	e3400000 	movt	r0, #0
 410:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	rcu_assign_pointer(si->sync_rule_callback, sync_rule_callback);
 414:	f57ff05b 	dmb	ish
	raw_spin_unlock_bh(&lock->rlock);
 418:	e3000000 	movw	r0, #0
 41c:	e586502c 	str	r5, [r6, #44]	@ 0x2c
 420:	e3400000 	movt	r0, #0
 424:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	return 0;
 428:	ea00002b 	b	4dc <init_module+0x4dc>

exit4:
#endif
	for_each_net(net) {
 42c:	e5986000 	ldr	r6, [r8]
	nf_unregister_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
 430:	e3007000 	movw	r7, #0
 434:	e3407000 	movt	r7, #0
	for_each_net(net) {
 438:	e246601c 	sub	r6, r6, #28
 43c:	e286301c 	add	r3, r6, #28
 440:	e1530008 	cmp	r3, r8
 444:	0a000006 	beq	464 <init_module+0x464>
	nf_unregister_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
 448:	e1a00006 	mov	r0, r6
 44c:	e3a02002 	mov	r2, #2
 450:	e1a01007 	mov	r1, r7
 454:	ebfffffe 	bl	0 <nf_unregister_net_hooks>
	for_each_net(net) {
 458:	e596601c 	ldr	r6, [r6, #28]
 45c:	e246601c 	sub	r6, r6, #28
 460:	eafffff5 	b	43c <init_module+0x43c>
	}
exit3:
	unregister_inetaddr_notifier(&sc->inet_notifier);
 464:	e1a0000a 	mov	r0, sl
 468:	ebfffffe 	bl	0 <unregister_inetaddr_notifier>
#ifdef SFE_SUPPORT_IPV6
	unregister_inet6addr_notifier(&sc->inet6_notifier);
 46c:	e59f009c 	ldr	r0, [pc, #156]	@ 510 <init_module+0x510>
 470:	ebfffffe 	bl	0 <unregister_inet6addr_notifier>
#endif
	unregister_netdevice_notifier(&sc->dev_notifier);
 474:	e59f0098 	ldr	r0, [pc, #152]	@ 514 <init_module+0x514>
 478:	ebfffffe 	bl	0 <unregister_netdevice_notifier>
 47c:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 480:	e3a02000 	mov	r2, #0
 484:	e59f106c 	ldr	r1, [pc, #108]	@ 4f8 <init_module+0x4f8>
 488:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 48c:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 490:	e3a02000 	mov	r2, #0
 494:	e59f1060 	ldr	r1, [pc, #96]	@ 4fc <init_module+0x4fc>
 498:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 49c:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 4a0:	e3a02000 	mov	r2, #0
 4a4:	e59f1054 	ldr	r1, [pc, #84]	@ 500 <init_module+0x500>
 4a8:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 4ac:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 4b0:	e3a02000 	mov	r2, #0
 4b4:	e59f1048 	ldr	r1, [pc, #72]	@ 504 <init_module+0x504>
 4b8:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
 4bc:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 4c0:	e3a02000 	mov	r2, #0
 4c4:	e59f103c 	ldr	r1, [pc, #60]	@ 508 <init_module+0x508>
 4c8:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
	sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_defunct_all.attr);
#if (DEBUG_LEVEL > 0)
	sysfs_remove_file(sc->sys_fast_classifier, &fast_classifier_exceptions_attr.attr);
#endif
exit2:
	kobject_put(sc->sys_fast_classifier);
 4cc:	e5950028 	ldr	r0, [r5, #40]	@ 0x28
 4d0:	ebfffffe 	bl	0 <kobject_put>
 4d4:	ea000000 	b	4dc <init_module+0x4dc>
	int result = -1;
 4d8:	e3e04000 	mvn	r4, #0

exit1:
	return result;
}
 4dc:	e1a00004 	mov	r0, r4
 4e0:	e24bd028 	sub	sp, fp, #40	@ 0x28
 4e4:	e89daff0 	ldm	sp, {r4, r5, r6, r7, r8, r9, sl, fp, sp, pc}
 4e8:	00000048 	.word	0x00000048
 4ec:	0000004c 	.word	0x0000004c
 4f0:	00000058 	.word	0x00000058
 4f4:	000000d8 	.word	0x000000d8
 4f8:	00000068 	.word	0x00000068
 4fc:	00000078 	.word	0x00000078
 500:	00000088 	.word	0x00000088
 504:	00000098 	.word	0x00000098
 508:	000000a8 	.word	0x000000a8
 50c:	00000164 	.word	0x00000164
 510:	00000044 	.word	0x00000044
 514:	0000002c 	.word	0x0000002c

Disassembly of section .text.unlikely:

00000000 <sfe_ipv4_destroy_all_rules_for_dev.constprop.0>:
void sfe_ipv4_destroy_all_rules_for_dev(struct net_device *dev)
   0:	e1a0c00d 	mov	ip, sp
   4:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock_bh(&lock->rlock);
   c:	e3004000 	movw	r4, #0
  10:	e3404000 	movt	r4, #0
  14:	e3000000 	movw	r0, #0
  18:	e3400000 	movt	r0, #0
  1c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
  20:	e594500c 	ldr	r5, [r4, #12]
	if (c) {
  24:	e3550000 	cmp	r5, #0
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
  28:	e1a00005 	mov	r0, r5
	if (c) {
  2c:	0a000000 	beq	34 <sfe_ipv4_destroy_all_rules_for_dev.constprop.0+0x34>
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
  30:	ebfffffe 	bl	2258 <sfe_ipv4_remove_sfe_ipv4_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
  34:	e1a00004 	mov	r0, r4
  38:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (c) {
  3c:	e3550000 	cmp	r5, #0
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
  40:	e1a00005 	mov	r0, r5
	if (c) {
  44:	089da830 	ldmeq	sp, {r4, r5, fp, sp, pc}
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
  48:	ebfffffe 	bl	1cd4 <sfe_ipv4_flush_sfe_ipv4_connection.constprop.0>
		goto another_round;
  4c:	eafffff0 	b	14 <sfe_ipv4_destroy_all_rules_for_dev.constprop.0+0x14>

00000050 <sfe_ipv6_destroy_all_rules_for_dev.constprop.0>:
void sfe_ipv6_destroy_all_rules_for_dev(struct net_device *dev)
  50:	e1a0c00d 	mov	ip, sp
  54:	e92dd830 	push	{r4, r5, fp, ip, lr, pc}
  58:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock_bh(&lock->rlock);
  5c:	e3004000 	movw	r4, #0
  60:	e3404000 	movt	r4, #0
  64:	e3000000 	movw	r0, #0
  68:	e3400000 	movt	r0, #0
  6c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	for (c = si->all_connections_head; c; c = c->all_connections_next) {
  70:	e594500c 	ldr	r5, [r4, #12]
	if (c) {
  74:	e3550000 	cmp	r5, #0
		sfe_ipv6_remove_connection(si, c);
  78:	e1a00005 	mov	r0, r5
	if (c) {
  7c:	0a000000 	beq	84 <sfe_ipv6_destroy_all_rules_for_dev.constprop.0+0x34>
		sfe_ipv6_remove_connection(si, c);
  80:	ebfffffe 	bl	2464 <sfe_ipv6_remove_connection.constprop.0>
	raw_spin_unlock_bh(&lock->rlock);
  84:	e1a00004 	mov	r0, r4
  88:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	if (c) {
  8c:	e3550000 	cmp	r5, #0
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
  90:	e1a00005 	mov	r0, r5
	if (c) {
  94:	089da830 	ldmeq	sp, {r4, r5, fp, sp, pc}
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
  98:	ebfffffe 	bl	6e54 <sfe_ipv6_flush_connection.constprop.0>
		goto another_round;
  9c:	eafffff0 	b	64 <sfe_ipv6_destroy_all_rules_for_dev.constprop.0+0x14>

Disassembly of section .exit.text:

00000000 <cleanup_module>:

/*
 * fast_classifier_exit()
 */
static void __exit fast_classifier_exit(void)
{
   0:	e1a0c00d 	mov	ip, sp
   4:	e92ddbf0 	push	{r4, r5, r6, r7, r8, r9, fp, ip, lr, pc}
   8:	e24cb004 	sub	fp, ip, #4
	raw_spin_lock_bh(&lock->rlock);
   c:	e3004000 	movw	r4, #0
  10:	e3404000 	movt	r4, #0
  14:	e1a00004 	mov	r0, r4
  18:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	rcu_assign_pointer(si->sync_rule_callback, sync_rule_callback);
  1c:	f57ff05b 	dmb	ish
  20:	e3a06000 	mov	r6, #0
  24:	e3005000 	movw	r5, #0
	raw_spin_unlock_bh(&lock->rlock);
  28:	e1a00004 	mov	r0, r4
	raw_spin_lock_bh(&lock->rlock);
  2c:	e3405000 	movt	r5, #0
  30:	e584602c 	str	r6, [r4, #44]	@ 0x2c
	raw_spin_unlock_bh(&lock->rlock);
  34:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	raw_spin_lock_bh(&lock->rlock);
  38:	e1a00005 	mov	r0, r5
  3c:	ebfffffe 	bl	0 <_raw_spin_lock_bh>
	rcu_assign_pointer(si->sync_rule_callback, sync_rule_callback);
  40:	f57ff05b 	dmb	ish
	raw_spin_unlock_bh(&lock->rlock);
  44:	e1a00005 	mov	r0, r5
  48:	e585602c 	str	r6, [r5, #44]	@ 0x2c
	}

#ifdef CONFIG_NF_CONNTRACK_EVENTS
	nf_conntrack_unregister_notifier(&init_net, &fast_classifier_conntrack_notifier);
#endif
	for_each_net(net) {
  4c:	e3007000 	movw	r7, #0
  50:	ebfffffe 	bl	0 <_raw_spin_unlock_bh>
	RCU_INIT_POINTER(fast_nat_recv, NULL);
  54:	e3003000 	movw	r3, #0
	for_each_net(net) {
  58:	e3407000 	movt	r7, #0
	RCU_INIT_POINTER(fast_nat_recv, NULL);
  5c:	e3403000 	movt	r3, #0
		nf_unregister_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
  60:	e3008000 	movw	r8, #0
  64:	e3408000 	movt	r8, #0
	RCU_INIT_POINTER(fast_nat_recv, NULL);
  68:	e5836000 	str	r6, [r3]
	rcu_barrier();
  6c:	ebfffffe 	bl	0 <rcu_barrier>
	sfe_ipv4_destroy_all_rules_for_dev(NULL);
  70:	ebfffffe 	bl	0 <cleanup_module>
	sfe_ipv6_destroy_all_rules_for_dev(NULL);
  74:	ebfffffe 	bl	50 <cleanup_module+0x50>
	result = genl_unregister_family(&fast_classifier_gnl_family);
  78:	e3000000 	movw	r0, #0
  7c:	e3400000 	movt	r0, #0
  80:	ebfffffe 	bl	0 <genl_unregister_family>
	nf_conntrack_unregister_notifier(&init_net, &fast_classifier_conntrack_notifier);
  84:	e3000000 	movw	r0, #0
  88:	e59f10e0 	ldr	r1, [pc, #224]	@ 170 <cleanup_module+0x170>
  8c:	e3400000 	movt	r0, #0
  90:	ebfffffe 	bl	0 <nf_conntrack_unregister_notifier>
	for_each_net(net) {
  94:	e5976000 	ldr	r6, [r7]
  98:	e246601c 	sub	r6, r6, #28
  9c:	e286301c 	add	r3, r6, #28
		nf_unregister_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
  a0:	e3a02002 	mov	r2, #2
	for_each_net(net) {
  a4:	e1530007 	cmp	r3, r7
		nf_unregister_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
  a8:	e1a01008 	mov	r1, r8
  ac:	e1a00006 	mov	r0, r6
	for_each_net(net) {
  b0:	0a000003 	beq	c4 <cleanup_module+0xc4>
		nf_unregister_net_hooks(net, fast_classifier_ops_post_routing, ARRAY_SIZE(fast_classifier_ops_post_routing));
  b4:	ebfffffe 	bl	0 <nf_unregister_net_hooks>
	for_each_net(net) {
  b8:	e596601c 	ldr	r6, [r6, #28]
  bc:	e246601c 	sub	r6, r6, #28
  c0:	eafffff5 	b	9c <cleanup_module+0x9c>
	}
#ifdef SFE_SUPPORT_IPV6
	unregister_inet6addr_notifier(&sc->inet6_notifier);
  c4:	e3006000 	movw	r6, #0
	 */
	sfe_ipv6_destroy_all_rules_for_dev(NULL);

	del_timer_sync(&si->timer);

	unregister_chrdev(si->debug_dev, "sfe_ipv6");
  c8:	e2855902 	add	r5, r5, #32768	@ 0x8000
  cc:	e3406000 	movt	r6, #0
	 */
	sfe_ipv4_destroy_all_rules_for_dev(NULL);

	del_timer_sync(&si->timer);

	unregister_chrdev(si->debug_dev, "sfe_ipv4");
  d0:	e2844902 	add	r4, r4, #32768	@ 0x8000
  d4:	e2860044 	add	r0, r6, #68	@ 0x44
  d8:	ebfffffe 	bl	0 <unregister_inet6addr_notifier>
#endif
	unregister_inetaddr_notifier(&sc->inet_notifier);
  dc:	e2860038 	add	r0, r6, #56	@ 0x38
  e0:	ebfffffe 	bl	0 <unregister_inetaddr_notifier>
	unregister_netdevice_notifier(&sc->dev_notifier);
  e4:	e286002c 	add	r0, r6, #44	@ 0x2c
  e8:	ebfffffe 	bl	0 <unregister_netdevice_notifier>

	kobject_put(sc->sys_fast_classifier);
  ec:	e5960028 	ldr	r0, [r6, #40]	@ 0x28
  f0:	ebfffffe 	bl	0 <kobject_put>
	sfe_ipv6_destroy_all_rules_for_dev(NULL);
  f4:	ebfffffe 	bl	50 <cleanup_module+0x50>
	del_timer_sync(&si->timer);
  f8:	e59f0074 	ldr	r0, [pc, #116]	@ 174 <cleanup_module+0x174>
  fc:	ebfffffe 	bl	0 <del_timer_sync>
}

static inline void unregister_chrdev(unsigned int major, const char *name)
{
	__unregister_chrdev(major, 0, 256, name);
 100:	e3003000 	movw	r3, #0
 104:	e595026c 	ldr	r0, [r5, #620]	@ 0x26c
 108:	e3a02c01 	mov	r2, #256	@ 0x100
 10c:	e3403000 	movt	r3, #0
 110:	e3a01000 	mov	r1, #0
 114:	ebfffffe 	bl	0 <__unregister_chrdev>
 118:	e5950268 	ldr	r0, [r5, #616]	@ 0x268
 11c:	e3a02000 	mov	r2, #0
 120:	e59f1050 	ldr	r1, [pc, #80]	@ 178 <cleanup_module+0x178>
 124:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
#ifdef CONFIG_NF_FLOW_COOKIE
	sysfs_remove_file(si->sys_sfe_ipv6, &sfe_ipv6_flow_cookie_attr.attr);
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_sfe_ipv6, &sfe_ipv6_debug_dev_attr.attr);

	kobject_put(si->sys_sfe_ipv6);
 128:	e5950268 	ldr	r0, [r5, #616]	@ 0x268
 12c:	ebfffffe 	bl	0 <kobject_put>
	sfe_ipv4_destroy_all_rules_for_dev(NULL);
 130:	ebfffffe 	bl	0 <cleanup_module>
	del_timer_sync(&si->timer);
 134:	e59f0040 	ldr	r0, [pc, #64]	@ 17c <cleanup_module+0x17c>
 138:	ebfffffe 	bl	0 <del_timer_sync>
 13c:	e3003000 	movw	r3, #0
 140:	e594025c 	ldr	r0, [r4, #604]	@ 0x25c
 144:	e3a02c01 	mov	r2, #256	@ 0x100
 148:	e3403000 	movt	r3, #0
 14c:	e3a01000 	mov	r1, #0
 150:	ebfffffe 	bl	0 <__unregister_chrdev>
 154:	e5940258 	ldr	r0, [r4, #600]	@ 0x258
 158:	e3a02000 	mov	r2, #0
 15c:	e59f101c 	ldr	r1, [pc, #28]	@ 180 <cleanup_module+0x180>
 160:	ebfffffe 	bl	0 <sysfs_remove_file_ns>
#ifdef CONFIG_NF_FLOW_COOKIE
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_flow_cookie_attr.attr);
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_debug_dev_attr.attr);

	kobject_put(si->sys_sfe_ipv4);
 164:	e5940258 	ldr	r0, [r4, #600]	@ 0x258
 168:	ebfffffe 	bl	0 <kobject_put>
#ifdef SFE_SUPPORT_IPV6
	sfe_ipv6_exit();
#endif
	sfe_ipv4_exit();

}
 16c:	e89dabf0 	ldm	sp, {r4, r5, r6, r7, r8, r9, fp, sp, pc}
 170:	00000164 	.word	0x00000164
 174:	00008068 	.word	0x00008068
 178:	00000058 	.word	0x00000058
 17c:	000102e0 	.word	0x000102e0
 180:	00000048 	.word	0x00000048

Disassembly of section .plt:

00000184 <.plt>:
	...
