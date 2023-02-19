
static void ndpi_nf_ct_destroy(struct nf_conntrack *nfct)
{
	struct nf_conn *ct = (struct nf_conn *)nfct;
	ndpi_conntrack_destroy_ptr notify;

	if (unlikely(nf_ct_is_template(ct))) {
		nf_ct_tmpl_free(ct);
		return;
	}
	rcu_read_lock();
        notify = rcu_dereference(nf_conntrack_destroy_cb);
	if (notify) {
		notify(ct);
	}
	rcu_read_unlock();

	if (unlikely(nf_ct_protonum(ct) == IPPROTO_GRE)) {
#ifdef CONFIG_NF_CT_PROTO_GRE
		struct nf_conn *master = ct->master;
		if (master)
			nf_ct_gre_keymap_destroy(master);
#endif
	}
	local_bh_disable();
	/* Expectations will have been removed in clean_from_lists,
	 * except TFTP can create an expectation on the first packet,
	 * before connection is in the list, so we need to clean here,
	 * too.
	 */
	nf_ct_remove_expectations(ct);

	if (unlikely(!nf_ct_is_confirmed(ct))) {
		/* nf_ct_del_from_unconfirmed_list() */
		struct ct_pcpu *pcpu;

		/* We overload first tuple to link into unconfirmed or dying list.*/
		pcpu = per_cpu_ptr(nf_ct_net(ct)->ct.pcpu_lists, ct->cpu);

		spin_lock(&pcpu->lock);
		BUG_ON(hlist_nulls_unhashed(&ct->tuplehash[IP_CT_DIR_ORIGINAL].hnnode));
		hlist_nulls_del_rcu(&ct->tuplehash[IP_CT_DIR_ORIGINAL].hnnode);
		spin_unlock(&pcpu->lock);
	}

	local_bh_enable();

	if (ct->master)
		nf_ct_put(ct->master);

	nf_conntrack_free(ct);
}
