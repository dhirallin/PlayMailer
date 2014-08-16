#ifndef POP3_H
#define POP3_H

int pop3_fetch(char *server, unsigned short port, char *login, char *password, int security);

#endif