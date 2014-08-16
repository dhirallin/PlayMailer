#ifndef _LIBETPAN_HELPER_H
#define _LIBETPAN_HELPER_H

#include <stdio.h>
#include "load_msg.h"

typedef struct MailMessageW_t {
	TCHAR *from_name;
	TCHAR *from_address;
	TCHAR *to_names[MAX_RECIPIENTS+1];
	TCHAR *to_addresses[MAX_RECIPIENTS+1];
	int num_recipients;
	TCHAR *subject;
	TCHAR *text;
	unsigned int textsize;
	TCHAR *filename;
	const char *filedata;
	unsigned int filesize;
} MailMessageW;

int smtpsend_w(TCHAR *server, unsigned short port, TCHAR *user, TCHAR *password, TCHAR *from, TCHAR **recipients, int security, BOOL no_esmtp, int email);
int compose_msg_w(const TCHAR *from, const TCHAR *to, const TCHAR *subject, const TCHAR *text, const TCHAR *filepath, FILE *msg);
int imap_fetch_w(TCHAR *server, unsigned short port, TCHAR *login, TCHAR *password, int security, TCHAR *search_str);
int pop3_fetch_w(TCHAR *server, unsigned short port, TCHAR *login, TCHAR *password, int security);
int load_msg_w(TCHAR *filename, MailMessageW *msg);
void free_msg_w(MailMessageW *msg);

#endif