
#define ANYKEY_MSG "Press a key to continue"

void tx_init_error_attrs(int border, int text, int prompt);
void tx_init_info_attrs(int border, int text, int prompt);
void tx_errbox(char *message, char *prompt, int *response); 
void tx_infobox(char *text, char *prompt);
