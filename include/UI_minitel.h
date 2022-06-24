#ifndef UI_MINITEL_H_
#define UI_MINITEL_H_


#define NB_COLONNES 80

void print_header_fabMSTIC(void);
void print_header_livredor(void);

void init_affichage(void);

void nav_menu(int *tab_menu);
void set_cursor_menu(int index);
int valid_menu(void);
int test_fleche(void);

ssize_t minitel_raw_uart_write(const void* buffer, size_t len);
void minitel_clear_page(void);
void minitel_cursor_home(void);
void minitel_cursor_up(void);
void minitel_cursor_right(void);
void minitel_clear_line(void);
void minitel_bip(void);

#endif