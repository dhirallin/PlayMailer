#include <Windows.h>
#include "smtpsend.h"
#include "libetpan_helper.h"
#include "international.h"
#include "compose_msg.h"
#include "load_msg.h"
#include "imap.h"
#include "pop3.h"

int smtpsend_w(TCHAR *server, unsigned short port, TCHAR *user, TCHAR *password, TCHAR *from, TCHAR **recipients, int security, BOOL no_esmtp, int email)
{
	int err, numRcpts=0;
	char server_u[SMTP_STRING_SIZE];
	char user_u[SMTP_STRING_SIZE];
	char password_u[SMTP_STRING_SIZE];
	char from_u[SMTP_STRING_SIZE];
	char **recipients_u;
	TCHAR **r;
	char **ru;

	UTF8_Encode(server, server_u, SMTP_STRING_SIZE);
	UTF8_Encode(user, user_u, SMTP_STRING_SIZE);
	UTF8_Encode(password, password_u, SMTP_STRING_SIZE);
	UTF8_Encode(from, from_u, SMTP_STRING_SIZE);

	r = recipients;
	while(*r++ != NULL)
		numRcpts++;

	recipients_u = (char **)malloc(sizeof(char *) * (numRcpts + 1));
	
	ru = recipients_u;
	while(*recipients != NULL) 
		*ru++ = UTF8_Encode_Dyn( *recipients++ );
	*ru = NULL;
	
	err = smtpsend(server_u, port, user_u, password_u, from_u, recipients_u, security, no_esmtp, email);

	ru = recipients_u;
	while(*ru != NULL)
		free(*ru++);
	free(recipients_u);

	return err;
}

int compose_msg_w(const TCHAR *from, const TCHAR *to, const TCHAR *subject, const TCHAR *text, const TCHAR *filepath, FILE *msg)
{
	int err;

	char *text_u, *from_u, *to_u, *subject_u;
	char *filepath_u = NULL;

	if(filepath)
		filepath_u = UTF8_Encode_Dyn(filepath);
	
	text_u = UTF8_Encode_Dyn(text);
	from_u = UTF8_Encode_Dyn(from);
	to_u = UTF8_Encode_Dyn(to);
	subject_u = UTF8_Encode_Dyn(subject);

	err = compose_msg(from_u, to_u, subject_u, text_u, filepath_u, msg);

	free(text_u);
	if(filepath_u) free(filepath_u);
	free(from_u);
	free(to_u);
	free(subject_u);

	return err;
}

int imap_fetch_w(TCHAR *server, unsigned short port, TCHAR *login, TCHAR *password, int security, TCHAR *search_str)
{
	int err;

	char *server_u, *login_u, *password_u, *search_str_u = NULL;

	if(search_str)
		search_str_u = UTF8_Encode_Dyn(search_str);
	
	server_u = UTF8_Encode_Dyn(server);
	login_u = UTF8_Encode_Dyn(login);
	password_u = UTF8_Encode_Dyn(password);

	err = imap_fetch(server_u, port, login_u, password_u, security, search_str_u);

	if(search_str_u) free(search_str_u);
	free(server_u);
	free(login_u);
	free(password_u);

	return err;
}

int pop3_fetch_w(TCHAR *server, unsigned short port, TCHAR *login, TCHAR *password, int security)
{
	int err;

	char *server_u, *login_u, *password_u;

	server_u = UTF8_Encode_Dyn(server);
	login_u = UTF8_Encode_Dyn(login);
	password_u = UTF8_Encode_Dyn(password);

	err = pop3_fetch(server_u, port, login_u, password_u, security);

	free(server_u);
	free(login_u);
	free(password_u);

	return err;
}

int load_msg_w(TCHAR *filename, MailMessageW *msg)
{
	int err, i;
	char *filename_u;
	MailMessage msg_u; 
	char *tempStr;

	memset(msg, 0, sizeof(MailMessageW));

	filename_u = UTF8_Encode_Dyn(filename);

	if(!(err = load_msg(filename_u, &msg_u)))
	{
		msg->num_recipients = msg_u.num_recipients;
		for(i = 0; i < msg->num_recipients; i++)
		{
			msg->to_names[i] = UTF8_Decode_Dyn(msg_u.to_names[i]);
			msg->to_addresses[i] = UTF8_Decode_Dyn(msg_u.to_addresses[i]);
		}
		
		msg->from_name = UTF8_Decode_Dyn(msg_u.from_name);
		msg->from_address = UTF8_Decode_Dyn(msg_u.from_address);
		msg->subject = UTF8_Decode_Dyn(msg_u.subject);
		tempStr = (char *)malloc(msg_u.textsize + 1);
		strncpy_s(tempStr, msg_u.textsize + 1, msg_u.text, msg_u.textsize);
		tempStr[msg_u.textsize] = 0;
		msg->text = UTF8_Decode_Dyn(tempStr);
		free(tempStr);
		msg->textsize = msg_u.textsize;
		
		if(msg_u.filename) 
			msg->filename = UTF8_Decode_Dyn(msg_u.filename);
				
		msg->filedata = msg_u.filedata;
		msg->filesize = msg_u.filesize;
	}

	free(filename_u);

	return err;
}

void free_msg_w(MailMessageW *msg)
{
	int i;

	free(msg->from_name);
	free(msg->from_address);
	free(msg->subject);
	free(msg->text);

	if(msg->filename) 
		free(msg->filename);

	for(i = 0; i < msg->num_recipients; i++)
	{
		free(msg->to_names[i]);
		free(msg->to_addresses[i]);
	}

	free_msg();
}