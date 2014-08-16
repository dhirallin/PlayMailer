#include <libetpan/libetpan.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include "load_msg.h"

THREAD_LOCAL MailMessage *mailmsg;
THREAD_LOCAL struct mailmime *mime;
THREAD_LOCAL char * data;
THREAD_LOCAL char * decoded_filedata;
THREAD_LOCAL BOOL file_type;
THREAD_LOCAL int field_type;

void decode_data(const char *data, size_t len, char **decoded, size_t *decoded_len, int encoding)
{
	size_t cur_token;

	cur_token = 0;
	mailmime_part_parse(data, len, &cur_token,
		encoding, decoded, decoded_len);
}

static void get_mime_content(struct mailmime_content * content_type);

static void get_mime_data(struct mailmime_data * data)
{
  const char *encoded_data;
  size_t encoded_len, decoded_len;
	
  switch (data->dt_type) {
  case MAILMIME_DATA_TEXT:
    if(file_type)
	{	
		encoded_data = data->dt_data.dt_text.dt_data;
		encoded_len = (unsigned int) data->dt_data.dt_text.dt_length;

		decode_data(encoded_data, encoded_len, &decoded_filedata, &decoded_len, data->dt_encoding);

		mailmsg->filedata = decoded_filedata;
		mailmsg->filesize = decoded_len;
	}
	else
	{
		mailmsg->text = data->dt_data.dt_text.dt_data;
		mailmsg->textsize = (unsigned int) data->dt_data.dt_text.dt_length;
	}
	//printf("data : %u bytes\n", (unsigned int) data->dt_data.dt_text.dt_length);
    break;
  case MAILMIME_DATA_FILE:
    //printf("data (file) : %s\n", data->dt_data.dt_filename);
    break;
  }
}

static void get_mime_parameter(struct mailmime_parameter * param)
{
  //printf("%s = %s\n", param->pa_name, param->pa_value);
}

static void get_mime_dsp_parm(struct mailmime_disposition_parm * param)
{
  switch (param->pa_type) {
  case MAILMIME_DISPOSITION_PARM_FILENAME:
	mailmsg->filename = param->pa_data.pa_filename;
	file_type = TRUE;
    //printf("filename: %s\n", param->pa_data.pa_filename);
    break;
  }
}

static void get_mime_disposition(struct mailmime_disposition * disposition)
{
  clistiter * cur;

  for(cur = clist_begin(disposition->dsp_parms) ;
    cur != NULL ; cur = clist_next(cur)) {
    struct mailmime_disposition_parm * param;

    param = (struct mailmime_disposition_parm *)clist_content(cur);
    get_mime_dsp_parm(param);
  }
}

static void get_mime_field(struct mailmime_field * field)
{
	switch (field->fld_type) {
		case MAILMIME_FIELD_TYPE:
		//printf("content-type: ");
		get_mime_content(field->fld_data.fld_content);
	    //printf("\n");
		break;
		case MAILMIME_FIELD_DISPOSITION:
		get_mime_disposition(field->fld_data.fld_disposition);
		break;
	}
}

static void get_mime_fields(struct mailmime_fields * fields)
{
	clistiter * cur;

	for(cur = clist_begin(fields->fld_list) ; cur != NULL ; cur = clist_next(cur)) {
		struct mailmime_field * field;

		field = (struct mailmime_field *)clist_content(cur);
		get_mime_field(field);
	}
}

static void get_date_time(struct mailimf_date_time * d)
{
  //printf("%02i/%02i/%i %02i:%02i:%02i %+04i",
    //d->dt_day, d->dt_month, d->dt_year,
    //d->dt_hour, d->dt_min, d->dt_sec, d->dt_zone);
}

static void get_orig_date(struct mailimf_orig_date * orig_date)
{
  get_date_time(orig_date->dt_date_time);
}

static void get_mailbox(struct mailimf_mailbox * mb)
{
	if(field_type == MAILIMF_FIELD_TO && mailmsg->num_recipients < MAX_RECIPIENTS)
	{
		mailmsg->to_addresses[mailmsg->num_recipients] = mb->mb_addr_spec;
		
		if(mb->mb_display_name != NULL)
			mailmsg->to_names[mailmsg->num_recipients] = mb->mb_display_name;
		else
			mailmsg->to_names[mailmsg->num_recipients] = "";		

		mailmsg->num_recipients++;
	}
	else if(field_type == MAILIMF_FIELD_FROM && mailmsg->from_address == NULL)
	{
		mailmsg->from_address = mb->mb_addr_spec;
		
		if(mb->mb_display_name != NULL)
			mailmsg->from_name = mb->mb_display_name;
		else
			mailmsg->from_name = "";
	}
}

static void get_mailbox_list(struct mailimf_mailbox_list * mb_list)
{
  clistiter * cur;

  for(cur = clist_begin(mb_list->mb_list) ; cur != NULL ;
    cur = clist_next(cur)) {
    struct mailimf_mailbox * mb;
    
    mb = (struct mailimf_mailbox *)clist_content(cur);
    
    get_mailbox(mb);

		if (clist_next(cur) != NULL) {
			//printf(", ");
		}
  }
}

static void get_group(struct mailimf_group * group)
{
	clistiter * cur;
	
  //printf("%s: ", group->grp_display_name);
  for(cur = clist_begin(group->grp_mb_list->mb_list) ; cur != NULL ; cur = clist_next(cur)) {
    struct mailimf_mailbox * mb;

    mb = (struct mailimf_mailbox *)clist_content(cur);
    get_mailbox(mb);
  }
	//printf("; ");
}

static void get_address(struct mailimf_address * a)
{
  switch (a->ad_type) {
    case MAILIMF_ADDRESS_GROUP:
      get_group(a->ad_data.ad_group);
      break;

    case MAILIMF_ADDRESS_MAILBOX:
      get_mailbox(a->ad_data.ad_mailbox);
      break;
  }
}

static void get_address_list(struct mailimf_address_list * addr_list)
{
  clistiter * cur;

  for(cur = clist_begin(addr_list->ad_list) ; cur != NULL ;
    cur = clist_next(cur)) {
    struct mailimf_address * addr;
    
    addr = clist_content(cur);
    
    get_address(addr);
		
		if (clist_next(cur) != NULL) {
			//printf(", ");
		}
  }
}

static void get_from(struct mailimf_from * from)
{
  field_type = MAILIMF_FIELD_FROM;
  get_mailbox_list(from->frm_mb_list);
}

static void get_to(struct mailimf_to * to)
{
  field_type = MAILIMF_FIELD_TO;
  get_address_list(to->to_addr_list);
}

static void get_cc(struct mailimf_cc * cc)
{
  field_type = MAILIMF_FIELD_CC;
  get_address_list(cc->cc_addr_list);
}

static void get_subject(struct mailimf_subject * subject)
{
	mailmsg->subject = subject->sbj_value;
	//printf("%s", subject->sbj_value);
}

static void get_field(struct mailimf_field * field)
{
  switch (field->fld_type) {
  case MAILIMF_FIELD_ORIG_DATE:
    //printf("Date: ");
    get_orig_date(field->fld_data.fld_orig_date);
		//printf("\n");
    break;
  case MAILIMF_FIELD_FROM:
    //printf("From: ");
    get_from(field->fld_data.fld_from);
		//printf("\n");
    break;
  case MAILIMF_FIELD_TO:
    //printf("To: ");
    get_to(field->fld_data.fld_to);
		//printf("\n");
    break;
  case MAILIMF_FIELD_CC:
    //printf("Cc: ");
    get_cc(field->fld_data.fld_cc);
		//printf("\n");
    break;
  case MAILIMF_FIELD_SUBJECT:
    //printf("Subject: ");
    get_subject(field->fld_data.fld_subject);
		//printf("\n");
    break;
  }
}

static void get_fields(struct mailimf_fields * fields)
{
  clistiter * cur;

  for(cur = clist_begin(fields->fld_list) ; cur != NULL ;
    cur = clist_next(cur)) {
    struct mailimf_field * f;
    
    f = clist_content(cur);
    
    get_field(f);
  }
}

static void get_mime_discrete_type(struct mailmime_discrete_type * discrete_type)
{
  switch (discrete_type->dt_type) {
  case MAILMIME_DISCRETE_TYPE_TEXT:
    //printf("text");
    break;
  case MAILMIME_DISCRETE_TYPE_IMAGE:
    //printf("image");
    break;
  case MAILMIME_DISCRETE_TYPE_AUDIO:
    //printf("audio");
    break;
  case MAILMIME_DISCRETE_TYPE_VIDEO:
    //printf("video");
    break;
  case MAILMIME_DISCRETE_TYPE_APPLICATION:
    //printf("application");
    break;
  case MAILMIME_DISCRETE_TYPE_EXTENSION:
    //printf("%s", discrete_type->dt_extension);
    break;
  }
}

static void get_mime_composite_type(struct mailmime_composite_type * ct)
{
  switch (ct->ct_type) {
  case MAILMIME_COMPOSITE_TYPE_MESSAGE:
    //printf("message");
    break;
  case MAILMIME_COMPOSITE_TYPE_MULTIPART:
    //printf("multipart");
    break;
  case MAILMIME_COMPOSITE_TYPE_EXTENSION:
    //printf("%s", ct->ct_token);
    break;
  }
}

static void get_mime_type(struct mailmime_type * type)
{
  switch (type->tp_type) {
  case MAILMIME_TYPE_DISCRETE_TYPE:
    get_mime_discrete_type(type->tp_data.tp_discrete_type);
    break;
  case MAILMIME_TYPE_COMPOSITE_TYPE:
    get_mime_composite_type(type->tp_data.tp_composite_type);
    break;
  }
}

static void get_mime_content(struct mailmime_content * content_type)
{
  //printf("type: ");
  get_mime_type(content_type->ct_type);
  //printf("/%s\n", content_type->ct_subtype);
}

static void get_mime(struct mailmime * mime)
{
	clistiter * cur;

	file_type = FALSE;

	switch (mime->mm_type) {
		case MAILMIME_SINGLE:
		//printf("single part\n");
		break;
		case MAILMIME_MULTIPLE:
		//printf("multipart\n");
		break;
		case MAILMIME_MESSAGE:
		//printf("message\n");
		break;
	}

	if (mime->mm_mime_fields != NULL) {
		if (clist_begin(mime->mm_mime_fields->fld_list) != NULL) {
			//printf("MIME headers begin\n");
			get_mime_fields(mime->mm_mime_fields);
			//printf("MIME headers end\n");
		}
	}

	get_mime_content(mime->mm_content_type);

	switch (mime->mm_type) {
		case MAILMIME_SINGLE:
		get_mime_data(mime->mm_data.mm_single);
		break;

		case MAILMIME_MULTIPLE:
		for(cur = clist_begin(mime->mm_data.mm_multipart.mm_mp_list) ; cur != NULL ; cur = clist_next(cur)) {
			get_mime((struct mailmime *)clist_content(cur));
		}
		break;

		case MAILMIME_MESSAGE:
		if (mime->mm_data.mm_message.mm_fields) {
			if (clist_begin(mime->mm_data.mm_message.mm_fields->fld_list) != NULL) {
				//printf("headers begin\n");
				get_fields(mime->mm_data.mm_message.mm_fields);
				//printf("headers end\n");
			}

			if (mime->mm_data.mm_message.mm_msg_mime != NULL) {
				get_mime(mime->mm_data.mm_message.mm_msg_mime);
			}
			break;
		}
	}
}

int load_msg(char *filename, MailMessage *msg)
{
	FILE * f;
	int r;
	size_t current_index, fileSize;
	
	field_type = 0;
	file_type = FALSE;
	decoded_filedata = NULL;

	mailmsg = msg;
	memset(mailmsg, 0, sizeof(MailMessage));

	fopen_s(&f, filename, "r+");
	if (f == NULL) 
		return EXIT_FAILURE;
	
	fseek(f , 0 , SEEK_END);
    fileSize = ftell(f);
	rewind(f);

	data = (char *)malloc(fileSize);
	fileSize = fread(data, 1, fileSize, f);
	fclose(f);
	
	/*for(i = stat_info.st_size - 1; i >= 0; i--)
	{
		if(data[i] && !isspace(data[i]))
		{
			data[i+1] = 0;
			break;
		}
	}*/

	current_index = 0;
	r = mailmime_parse(data, fileSize,	&current_index, &mime);
	if (r != MAILIMF_NO_ERROR) {
		free(data);
		return r;
	}
	
	get_mime(mime);

	return 0;
}

void free_msg()
{
	mailmime_free(mime);
	free(data);
	if(decoded_filedata) free(decoded_filedata);
	decoded_filedata = NULL;
}