#ifndef IMAP_H
#define IMAP_H

int imap_fetch(char *server, unsigned short port, char *login, char *password, int security, char *search_str);
void imap_logout();
int imap_idle();
void imap_idle_done();

#endif