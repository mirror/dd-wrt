
int phys2log(int phys, int stripe, int n, int layout)
{
    /* In an 'n' disk array using 'layout',
     * in stripe 'stripe', the physical disc 'phys'
     * stores what logical chunk?
     * -1 mean parity.
     *
     */
    switch(layout) {
    case ALGORITHM_LEFT_ASYMMETRIC:
	pd = (n-1) - (stripe % n);
	if (phys < pd)
	    return phys;
	else if (phys == pd)
	    return -1;
	else return phys-1;

    case ALGORITHM_RIGHT_ASYMMETRIC:
	pd = stripe % n;
	if (phys < pd)
	    return phys;
	else if (phys == pd)
	    return -1;
	else return phys-1;

    case ALGORITHM_LEFT_SYMMETRIC:
	pd = (n-1) - (stripe %n);
	if (phys < pd)
	    return phys+ n-1-pd;
	else if (phys == pd)
	    return -1;
	else return phys-pd-1;

    case ALGORITHM_RIGHT_SYMMETRIC:
	pd = stripe % n;
	if (phys < pd)
	    return phys+ n-1-pd;
	else if (phys == pd)
	    return -1;
	else return phys-pd-1;
    }
    return -2;
}

raid5_extend(unsigned long len, int chunksize, int layout, int n, int m, int rfds[], int wfds[])
{

    static char buf[4096];

    unsigned long blocks = len/4;
    unsigned int blocksperchunk= chunksize/4096;

    unsigned long b;

    for (b=0; b<blocks; b++) {
	unsigned long stripe = b / blocksperchunk;
	unsigned int offset = b - (stripe*blocksperchunk);
	unsigned long chunk = stripe * (n-1);
	int src;
	for (src=0; src<n; src++) {
	    int dnum, snum;
	    if (read(rfds[src], buf, sizeof(buf)) != sizeof(buf)) {
		error();
		return 0;
	    }

	    snum = phys2log(src, stripe, n, layout);

	    if (snum == -1)
		continue;
	    chunk = stripe*(n-1)+snum;

	    dstripe = chunk/(m-1);
	    dnum = log2phys(chunk-(stripe*(m-1)), dstripe, m, layout);
	    llseek(wfds[dnum], dstripe*chunksize+(offset*4096), 0);
	    write(wfds[dnum], buf, sizeof(buf));
	}
    }
}
