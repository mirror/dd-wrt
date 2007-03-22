void init_filter_table(struct filterlist *fl)
{
    fl->head = fl->tail = NULL;
}

/*
 * Loads the filter from the filter file
 */

int loadfilter(char *filename, struct filterlist *fl, int resolve)
{
    struct filterent *fe;
    int pfd;
    unsigned int idx = 0;
    int br;
    int resolv_err = 0;
    char err_msg[80];

    init_filter_table(fl);

    pfd = open(filename, O_RDONLY);

    if (pfd < 0) {
        memset(err_msg, 0, 80);
        snprintf(err_msg, 80, "Error opening IP filter data file");
        write_error(err_msg, daemonized);
        fl->head = NULL;
        return 1;
    }
    do {
        fe = malloc(sizeof(struct filterent));
        br = read(pfd, &(fe->hp), sizeof(struct hostparams));

        if (br > 0) {
            fe->index = idx;
            if (resolve) {
                fe->saddr = nametoaddr(fe->hp.s_fqdn, &resolv_err);
                fe->daddr = nametoaddr(fe->hp.d_fqdn, &resolv_err);

                if (resolv_err) {
                    free(fe);
                    continue;
                }

                fe->smask = inet_addr(fe->hp.s_mask);
                fe->dmask = inet_addr(fe->hp.d_mask);
            }
            if (fl->head == NULL) {
                fl->head = fe;
                fe->prev_entry = NULL;
            } else {
                fl->tail->next_entry = fe;
                fe->prev_entry = fl->tail;
            }
            fe->next_entry = NULL;
            fl->tail = fe;
            idx++;
        } else {
            free(fe);
        }
    } while (br > 0);

    if (br == 0)
        close(pfd);

    return 0;
}

void savefilter(char *filename, struct filterlist *fl)
{
    struct filterent *fe = fl->head;
    int pfd;
    int bw;
    int resp;

    pfd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);

    while (fe != NULL) {
        bw = write(pfd, &(fe->hp), sizeof(struct hostparams));

        if (bw < 0) {
            tx_errbox("Unable to save filter changes", ANYKEY_MSG, &resp);
            clear_flt_tag();
            return;
        }
        fe = fe->next_entry;
    }

    close(pfd);
}
