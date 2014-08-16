#include <libetpan/libetpan.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <direct.h>

#define NUM_SECURITY_PROTOCOLS	3
enum SecurityProtocol {
	SECURITY_SSL,
	SECURITY_TLS,
	SECURITY_NONE
};

static int check_error(int r, char * msg)
{
	if (r == MAILPOP3_NO_ERROR)
		return 0;
	
	fprintf(stderr, "%s\n", msg);
	return r;
}

int pop3_fetch(char *server, unsigned short port, char *login, char *password, int security)
{
	mailpop3 * pop3;
	int r;
	carray * list;
	unsigned int i;

#ifdef _DEBUG
	mailstream_debug=1;
#endif
	
	_mkdir("download");
	
	pop3 = mailpop3_new(0, NULL);
	
	if(security == SECURITY_SSL)
		r = mailpop3_ssl_connect(pop3, server, port);
	else
		r = mailpop3_socket_connect(pop3, server, port);
	if(check_error(r, "connect failed"))
		goto free_pop3;
	
	if(security == SECURITY_TLS)
	{
		r = mailpop3_socket_starttls(pop3);
		if(check_error(r, "could not switch to TLS connection"))
			goto logout;
	}

	r = mailpop3_user(pop3, login);
	if(check_error(r, "user failed"))
		goto logout;

	r = mailpop3_pass(pop3, password);
	if(check_error(r, "pass failed"))
		goto logout;

	r = mailpop3_list(pop3, &list);
	if(check_error(r, "list failed"))
		goto logout;

	for(i = 0 ; i < carray_count(list) ; i ++) {
		struct mailpop3_msg_info * info;
		char * msg_content;
		size_t msg_size;
		FILE * f;
		char filename[512];
		struct stat stat_info;
		
		info = carray_get(list, i);
		
		if (info->msg_uidl == NULL) {
			continue;
		}
		
		sprintf_s(filename, sizeof(filename), "download/%s.eml", info->msg_uidl);
		r = stat(filename, &stat_info);
		if (r == 0) {
			printf("already fetched %u %s\n", info->msg_index, info->msg_uidl);
			continue;
		}
		
		r = mailpop3_retr(pop3, info->msg_index, &msg_content, &msg_size);
		if(check_error(r, "get failed"))
			goto logout;

		fopen_s(&f, filename, "w");
		fwrite(msg_content, 1, msg_size, f);
		fclose(f);
		mailpop3_retr_free(msg_content);
		
		if (info->msg_uidl != NULL) {
			printf("fetched %u %s\n", info->msg_index, info->msg_uidl);
		}
		else {
			printf("fetched %u\n", info->msg_index);
		}
	}
	
	r = 0;

logout:
	mailpop3_quit(pop3);
free_pop3:
	mailpop3_free(pop3);
	return r;
}
