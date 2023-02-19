void ndpi_nf_ct_destroy(struct nf_conntrack *nfct)
{
        struct nf_conn *ct = (struct nf_conn *)nfct;
	ndpi_conntrack_destroy_ptr notify;

        pr_debug("%s(%p)\n", __func__, ct);
        WARN_ON(refcount_read(&nfct->use) != 0);

	rcu_read_lock();
        notify = rcu_dereference(nf_conntrack_destroy_cb);
	if (notify) {
		notify(nfct);
	}
	rcu_read_unlock();

        if (unlikely(nf_ct_is_template(ct))) {
                nf_ct_tmpl_free(ct);
                return;
        }

        if (unlikely(nf_ct_protonum(ct) == IPPROTO_GRE)) {
#ifdef CONFIG_NF_CT_PROTO_GRE
	        struct nf_conn *master = ct->master;

        	if (master)
                	nf_ct_gre_keymap_destroy(master);
#endif
	}

        /* Expectations will have been removed in clean_from_lists,
         * except TFTP can create an expectation on the first packet,
         * before connection is in the list, so we need to clean here,
         * too.
         */
        nf_ct_remove_expectations(ct);

        if (ct->master)
                nf_ct_put(ct->master);

        pr_debug("%s: returning ct=%p to slab\n", __func__, ct);
        nf_conntrack_free(ct);
}
