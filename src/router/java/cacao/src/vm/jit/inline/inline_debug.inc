#if 0
static void debug_dump_inline_context(inline_node *iln)
{
	assert(iln->indent);

	printf("%sinline_context @%p: \n",
			iln->indent, (void*)iln->ctx);
}
#endif


static void dump_inline_tree(inline_node *iln, s4 blocknr)
{
	char indent[100];
	int i;
	inline_node *child;
	s4 nr;
	s4 lastnr;

	if (!iln) {
		printf("(inline_node *)null\n");
		return;
	}

	for (i=0; i<iln->depth; ++i)
		indent[i] = '\t';
	indent[i] = 0;

	assert(iln->m);
	if (iln->depth) {
		if (!iln->parent) {
			printf("parent unset");
		}
		else {
			printf("%s[%d] (°%d) start L%03d %c%c (caller L%03d pc %d)"
				   " (pt=%d+%d,lofs=%d,exh %d) cum(ins %d,bb %d,etl %d) sync=%d(%d) ",
					indent, iln->depth, iln->debugnr, blocknr,
					(iln->blockbefore) ? 'B' : '-',
					(iln->blockafter) ? 'A' : '-',
					iln->callerblock->nr, iln->callerpc,
					iln->n_passthroughcount - iln->n_selfpassthroughcount,
					iln->n_selfpassthroughcount,
					iln->localsoffset,
					iln->n_handlercount,
					iln->cumul_instructioncount, iln->cumul_basicblockcount,
					iln->cumul_exceptiontablelength,
					iln->synchronize, iln->synclocal
					);
		}
	}
	else {
		printf("%s[%d] ROOT cum(ins %d,bb %d,etl %d) ",
				indent, iln->depth,
				iln->cumul_instructioncount, iln->cumul_basicblockcount,
				iln->cumul_exceptiontablelength);
	}
	method_println(iln->m);

	child = iln->children;
	lastnr = 0;
	nr = blocknr;
	if (child) {
		do {
			nr += (child->callerblock->nr - lastnr);
			lastnr = child->callerblock->nr;

			if (child->blockbefore)
				nr++;

			dump_inline_tree(child, nr);

			nr += child->cumul_basicblockcount - 1;
			if (!child->blockbefore)
				nr++;
			if (child->blockafter)
				nr++;
		}
		while ((child = child->next) != iln->children);
	}

	if (iln->depth) {
		printf("%s[%d] (°%d) end   L%03d\n",
				indent, iln->depth, iln->debugnr,
				blocknr + iln->cumul_basicblockcount
						- ((iln->blockbefore) ? 1 : 0));
	}
}


/* vim:noexpandtab:sw=4:ts=4:ft=c:
 */
