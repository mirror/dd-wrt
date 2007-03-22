#include <curses.h>

void tx_printkeyhelp(char *keytext, char *desc, WINDOW * win,
                  int highattr, int textattr);
void tx_menukeyhelp(int textattr, int highattr);
void tx_listkeyhelp(int textattr, int highattr);
char *tx_ltrim(char *str);

