#include <libetpan/libetpan.h>
#include <stdlib.h>
#include <direct.h>
#include <sys/stat.h>
#include "imap.h"

#define NUM_SECURITY_PROTOCOLS	3
enum SecurityProtocol {
	SECURITY_SSL,
	SECURITY_TLS,
	SECURITY_NONE
};

struct mailimap * imap = NULL;
int NumExists;
BOOL IMAPMailExists = FALSE;

static int check_error(int r, char * msg)
{
	if (r == MAILIMAP_NO_ERROR)
		return 0;
	if (r == MAILIMAP_NO_ERROR_AUTHENTICATED)
		return 0;
	if (r == MAILIMAP_NO_ERROR_NON_AUTHENTICATED)
		return 0;
	
	fprintf(stderr, "%s\n", msg);
	return(r);
}

static char * get_msg_att_msg_content(struct mailimap_msg_att * msg_att, size_t * p_msg_size)
{
	clistiter * cur;
	
  /* iterate on each result of one given message */
	for(cur = clist_begin(msg_att->att_list) ; cur != NULL ; cur = clist_next(cur)) {
		struct mailimap_msg_att_item * item;
		
		item = clist_content(cur);
		if (item->att_type != MAILIMAP_MSG_ATT_ITEM_STATIC) {
			continue;
		}
		
    if (item->att_data.att_static->att_type != MAILIMAP_MSG_ATT_BODY_SECTION) {
			continue;
    }
		
		* p_msg_size = item->att_data.att_static->att_data.att_body_section->sec_length;
		return item->att_data.att_static->att_data.att_body_section->sec_body_part;
	}
	
	return NULL;
}

static char * get_msg_content(clist * fetch_result, size_t * p_msg_size)
{
	clistiter * cur;
	
  /* for each message (there will be probably only on message) */
	for(cur = clist_begin(fetch_result) ; cur != NULL ; cur = clist_next(cur)) {
		struct mailimap_msg_att * msg_att;
		size_t msg_size;
		char * msg_content;
		
		msg_att = clist_content(cur);
		msg_content = get_msg_att_msg_content(msg_att, &msg_size);
		if (msg_content == NULL) {
			continue;
		}
		
		* p_msg_size = msg_size;
		return msg_content;
	}
	
	return NULL;
}

static int fetch_msg(struct mailimap * imap, uint32_t uid)
{
	struct mailimap_set * set;
	struct mailimap_section * section;
	char filename[512];
	size_t msg_len;
	char * msg_content;
	FILE * f;
	struct mailimap_fetch_type * fetch_type;
	struct mailimap_fetch_att * fetch_att;
	int r;
	clist * fetch_result;
	struct stat stat_info;
	
	sprintf_s(filename, sizeof(filename), "incoming/%u.eml", (unsigned int) uid);
	r = stat(filename, &stat_info);
	if (r == 0) {
		// already cachde
		printf("%u is already fetched\n", (unsigned int) uid);
		return 0;
	}
	
	set = mailimap_set_new_single(uid);
	fetch_type = mailimap_fetch_type_new_fetch_att_list_empty();
	section = mailimap_section_new(NULL);
	//fetch_att = mailimap_fetch_att_new_body_peek_section(section);
	fetch_att = mailimap_fetch_att_new_body_section(section);
	mailimap_fetch_type_new_fetch_att_list_add(fetch_type, fetch_att);

	r = mailimap_uid_fetch(imap, set, fetch_type, &fetch_result);
	if(check_error(r, "could not fetch"))
		return r;

	printf("fetch %u\n", (unsigned int) uid);
	
	msg_content = get_msg_content(fetch_result, &msg_len);
	if (msg_content == NULL) {
		fprintf(stderr, "no content\n");
		mailimap_fetch_list_free(fetch_result);
		return 1;
	}
	
	fopen_s(&f, filename, "w");
	if (f == NULL) {
		fprintf(stderr, "could not write\n");
		mailimap_fetch_list_free(fetch_result);
		return 1;
	}
	
	fwrite(msg_content, 1, msg_len, f);
	fclose(f);
	
	printf("%u has been fetched\n", (unsigned int) uid);

	mailimap_fetch_list_free(fetch_result);

	return 0;
}

static uint32_t get_uid(struct mailimap_msg_att * msg_att)
{
	clistiter * cur;
	
  /* iterate on each result of one given message */
	for(cur = clist_begin(msg_att->att_list) ; cur != NULL ; cur = clist_next(cur)) {
		struct mailimap_msg_att_item * item;
		
		item = clist_content(cur);
		if (item->att_type != MAILIMAP_MSG_ATT_ITEM_STATIC) {
			continue;
		}
		
		if (item->att_data.att_static->att_type != MAILIMAP_MSG_ATT_UID) {
			continue;
		}
		
		return item->att_data.att_static->att_data.att_uid;
	}
	
	return 0;
}

// could just send the command directly to the stream, but I'm humouring the abstraction.
void delete_msg(struct mailimap * imap, uint32_t uid)
{
	struct mailimap_store_att_flags * store_att;
	struct mailimap_set * set;
	struct mailimap_flag * delete_flag;
	struct mailimap_flag_list * flag_list;

	flag_list = mailimap_flag_list_new_empty();
	delete_flag = mailimap_flag_new_deleted();
	mailimap_flag_list_add(flag_list, delete_flag);
	store_att = mailimap_store_att_flags_new_add_flags_silent(flag_list);

	set = mailimap_set_new_single(uid);
	
	mailimap_uid_store(imap, set, store_att);

	mailimap_store_att_flags_free(store_att);
	mailimap_set_free(set);

	NumExists--;
}

// Search for messages with from field containing search_str. Fetch messages.
static int fetch_messages_with_search(struct mailimap * imap, char *search_str)
{
	clist * search_result;
	struct mailimap_search_key * key;
	clistiter * cur;
	int r;
	BOOL messages_deleted = FALSE, done = FALSE;
	
	key = mailimap_search_key_new_from(search_str);

	r = mailimap_uid_search(imap, "UTF-8", key, &search_result);
	if(check_error(r, "could not search"))
		return r;

	/* for each message */
	for(cur = clist_begin(search_result) ; cur != NULL ; cur = clist_next(cur)) {
		uint32_t uid;
		done = FALSE;

		uid = *((uint32_t *)clist_content(cur));
		if (uid == 0)
			continue;

		r = fetch_msg(imap, uid);
		if(check_error(r, "could not fetch"))
		{
			mailimap_search_result_free(search_result);
			return r;
		}

		delete_msg(imap, uid);
		messages_deleted = TRUE;
	}

	mailimap_search_result_free(search_result);
	if(messages_deleted) mailimap_expunge(imap);

	return 0;
}

static int fetch_messages(struct mailimap * imap)
{
	struct mailimap_set * set;
	struct mailimap_fetch_type * fetch_type;
	struct mailimap_fetch_att * fetch_att;
	clist * fetch_result;
	clistiter * cur;
	int r;
	BOOL messages_deleted = FALSE;
	
	/* as improvement UIDVALIDITY should be read and the message cache should be cleaned
	   if the UIDVALIDITY is not the same */
	
	set = mailimap_set_new_interval(1, 0); /* fetch in interval 1:* */
	fetch_type = mailimap_fetch_type_new_fetch_att_list_empty();
	fetch_att = mailimap_fetch_att_new_uid();
	mailimap_fetch_type_new_fetch_att_list_add(fetch_type, fetch_att);

	r = mailimap_fetch(imap, set, fetch_type, &fetch_result);
	if(check_error(r, "could not fetch"))
		return r;
	
  /* for each message */
	for(cur = clist_begin(fetch_result) ; cur != NULL ; cur = clist_next(cur)) {
		struct mailimap_msg_att * msg_att;
		uint32_t uid;
	
		msg_att = clist_content(cur);
		uid = get_uid(msg_att);
		if (uid == 0)
			continue;

		r = fetch_msg(imap, uid);
		if(check_error(r, "could not fetch"))
		{
			mailimap_fetch_list_free(fetch_result);
			return r;
		}

		delete_msg(imap, uid);
		messages_deleted = TRUE;
	}

	mailimap_fetch_list_free(fetch_result);
	if(messages_deleted) mailimap_expunge(imap);
	return 0;
}

int imap_fetch(char *server, unsigned short port, char *login, char *password, int security, char *search_str)
{
	int r;
	BOOL alreadyConnected = TRUE;

	IMAPMailExists = FALSE;

#ifdef _DEBUG
	mailstream_debug=1;
#endif

	_mkdir("incoming");
		
	/*if(imap && ((r = mailimap_noop(imap)) && check_error(r, "could not send command")))
	{
		mailimap_free(imap);
		imap = NULL;
	}*/

connect:
	if(!imap)
	{
		alreadyConnected = FALSE;
		imap = mailimap_new(0, NULL);
		
		if(security == SECURITY_SSL)
			r = mailimap_ssl_connect(imap, server, port);
		else
			r = mailimap_socket_connect(imap, server, port);

		fprintf(stderr, "connect: %i\n", r);
		if(check_error(r, "could not connect to server"))
			goto free_mem;
	
		if(security == SECURITY_TLS)
		{
			r = mailimap_socket_starttls(imap);
			if(check_error(r, "could not switch to TLS connection"))
				goto logout;
		}

		r = mailimap_login(imap, login, password);
		if(check_error(r, "could not login"))
			goto logout;

		r = mailimap_select(imap, "INBOX");
		if(check_error(r, "could not select INBOX"))
			goto logout;
	}
	
	do
	{
		NumExists = imap->imap_selection_info->sel_exists;
		
		if(search_str)
			r = fetch_messages_with_search(imap, search_str);
		else
			r = fetch_messages(imap);

		if(check_error(r, "could not fetch"))
		{
			if(alreadyConnected)
			{
				mailimap_free(imap);
				imap = NULL;
				goto connect;
			}
			goto logout;
		}
	
	} while(NumExists != imap->imap_selection_info->sel_exists);
			
	return 0;

logout:
	mailimap_logout(imap);
free_mem:
	mailimap_free(imap);
	imap = NULL;
	return r;
}

void imap_logout()
{
	if(imap)
	{
		mailimap_logout(imap);
		mailimap_free(imap);
		imap = NULL;
	}
}

int imap_idle()
{
	int r;

	IMAPMailExists = FALSE;

	if(!imap || !mailimap_has_idle(imap))
		return -1;

	r = mailimap_idle(imap);
	if(check_error(r, "could not enter IDLE mode"))
		return -1;

	if (NumExists != imap->imap_selection_info->sel_exists) 
	{
		imap_idle_done();
		IMAPMailExists = TRUE;
    }

	return mailimap_idle_get_fd(imap);
}

void imap_idle_done()
{
	if(imap) 
		mailimap_idle_done(imap);
}