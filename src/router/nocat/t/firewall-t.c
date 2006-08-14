# include "conf.h"
# include "firewall.h"

int main (int argc, char **argv) {
    read_conf_file("nocat.conf");
    fw_init(nocat_conf);
}
