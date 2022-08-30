int generic_proc_close(struct ndpi_net *n,
			int (*parse_line)(struct ndpi_net *n,char *cmd),
			write_buf_id_t id);
struct write_proc_cmd * alloc_proc_wbuf(struct ndpi_net *n,
			write_buf_id_t id,size_t cmd_len_max);
ssize_t generic_proc_write(struct ndpi_net *n, const char __user *buffer,
			size_t length, loff_t *loff, 
			int (*parse_line)(struct ndpi_net *n,char *cmd),
			size_t cmd_size,write_buf_id_t id);
