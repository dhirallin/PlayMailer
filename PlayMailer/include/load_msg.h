#ifndef LOAD_MSG_H
#define LOAD_MSG_H

#define THREAD_LOCAL		__declspec(thread)

#define MAX_RECIPIENTS		100

typedef struct MailMessage_t {
	char *from_name;
	char *from_address;
	char *to_names[MAX_RECIPIENTS+1];
	char *to_addresses[MAX_RECIPIENTS+1];
	int num_recipients;
	char *subject;
	const char *text;
	unsigned int textsize;
	char *filename;
	const char *filedata;
	unsigned int filesize;
} MailMessage;

int load_msg(char *filename, MailMessage *msg);
void free_msg();

#endif