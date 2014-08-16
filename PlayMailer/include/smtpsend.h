#ifndef _SMTPSEND_H
#define _SMTPSEND_H

#include <stdio.h>

#define SMTP_STRING_SIZE	513

int smtpsend(char *server, unsigned short port, char *user, char *password, char *from, char **recipients, int security, BOOL no_esmtp, int email);

#endif