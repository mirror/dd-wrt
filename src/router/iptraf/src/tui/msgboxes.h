#ifndef IPTRAF_NG_TUI_MSGBOXES_H
#define IPTRAF_NG_TUI_MSGBOXES_H

#define ANYKEY_MSG "Press a key to continue"

void tx_init_error_attrs(int border, int text, int prompt);
void tx_init_info_attrs(int border, int text, int prompt);
void tx_infobox(char *text, char *prompt);
void tui_error(const char *prompt, const char *err, ...) __printf(2,3);
void tui_error_va(const char *prompt, const char *err, va_list vararg);

#endif	/* IPTRAF_NG_TUI_MSGBOXES_H */
