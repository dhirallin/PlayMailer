#include <libetpan/libetpan.h>
#include <string.h>
#include <stdlib.h>

#define DEST_CHARSET "iso-8859-1"

/* build sample fields */

static struct mailimf_fields * build_fields(char *from_str, char *to_str, char *subject_str)
{
  struct mailimf_mailbox_list * from;
  struct mailimf_address_list * to;
  char * subject;
  int r, index;
  struct mailimf_fields * new_fields;

  /* subject field */

  subject = _strdup(subject_str);
  if (subject == NULL) {
    goto err;
  }

  /* from field */

  from = mailimf_mailbox_list_new_empty();
  if (from == NULL) {
    goto free_subject;
  }

  r = mailimf_mailbox_list_add_parse(from, from_str);
  if (r != MAILIMF_NO_ERROR) {
    goto free_from;
  }

  /* to field */

  to = mailimf_address_list_new_empty();
  if (to == NULL) {
    goto free_from;
  }

  index = 0;
  r = mailimf_address_list_parse(to_str, strlen(to_str), &index, &to);
  if (r != MAILIMF_NO_ERROR) {
    goto free_from;
  }

  /*r = mailimf_address_list_add_parse(to, to_str);
  if (r != MAILIMF_NO_ERROR) {
    goto free_to;
  }*/

  new_fields = mailimf_fields_new_with_data(from /* from */,
      NULL /* sender */, NULL /* reply-to */, 
      to, NULL /* cc */, NULL /* bcc */, NULL /* in-reply-to */,
      NULL /* references */,
      subject);
  if (new_fields == NULL)
    goto free_to;

  return new_fields;

 free_to:
  mailimf_address_list_free(to);
 free_from:
  mailimf_mailbox_list_free(from);
 free_subject:
  free(subject);
 err:
  return NULL;
}



/* text is a string, build a mime part containing this string */

static struct mailmime * build_body_text(char * text)
{
  struct mailmime_fields * mime_fields;
  struct mailmime * mime_sub;
  struct mailmime_content * content;
  struct mailmime_parameter * param;
  int r;

  /* text/plain part */

  mime_fields = mailmime_fields_new_encoding(MAILMIME_MECHANISM_8BIT);
  if (mime_fields == NULL) {
    goto err;
  }

  content = mailmime_content_new_with_str("text/plain");
  if (content == NULL) {
    goto free_fields;
  }

  param = mailmime_param_new_with_data("charset", DEST_CHARSET);
  if (param == NULL) {
    goto free_content;
  }

  r = clist_append(content->ct_parameters, param);
  if (r < 0) {
    mailmime_parameter_free(param);
    goto free_content;
  }

  mime_sub = mailmime_new_empty(content, mime_fields);
  if (mime_sub == NULL) {
    goto free_content;
  }

  r = mailmime_set_body_text(mime_sub, text, strlen(text));
  if (r != MAILIMF_NO_ERROR) {
    goto free_mime;
  }

  return mime_sub;

 free_mime:
  mailmime_free(mime_sub);
  goto err;
 free_content:
  mailmime_content_free(content);
 free_fields:
  mailmime_fields_free(mime_fields);
 err:
  return NULL;
}


/* build a mime part containing the given file */

static struct mailmime * build_body_file(char * filename)
{
  struct mailmime_fields * mime_fields;
  struct mailmime * mime_sub;
  struct mailmime_content * content;
  struct mailmime_parameter * param;
  char * dup_filename;
  int r;

  /* text/plain part */

  dup_filename = _strdup(filename);
  if (dup_filename == NULL)
    goto err;

  mime_fields =
    mailmime_fields_new_filename(MAILMIME_DISPOSITION_TYPE_ATTACHMENT,
        dup_filename, MAILMIME_MECHANISM_BASE64);
  if (mime_fields == NULL)
    goto free_dup_filename;

  content = mailmime_content_new_with_str("application/octet-stream");
  if (content == NULL) {
    goto free_fields;
  }

  param = mailmime_param_new_with_data("charset", DEST_CHARSET);
  if (param == NULL) {
    goto free_content;
  }

  r = clist_append(content->ct_parameters, param);
  if (r < 0) {
    mailmime_parameter_free(param);
    goto free_content;
  }

  mime_sub = mailmime_new_empty(content, mime_fields);
  if (mime_sub == NULL) {
    goto free_content;
  }

  dup_filename = _strdup(filename);
  if (dup_filename == NULL)
    goto free_mime;

  r = mailmime_set_body_file(mime_sub, dup_filename);
  if (r != MAILIMF_NO_ERROR) {
    goto free_mime;
  }

  return mime_sub;

 free_mime:
  mailmime_free(mime_sub);
  goto err;
 free_content:
  mailmime_content_free(content);
 free_fields:
  mailmime_fields_free(mime_fields);
  goto err;
 free_dup_filename:
  free(dup_filename);
 err:
  return NULL;
}


/* build an empty message */

static struct mailmime * build_message(struct mailimf_fields * fields)
{
  struct mailmime * mime;
  
  /* message */
  
  mime = mailmime_new_message_data(NULL);
  if (mime == NULL) {
    goto err;
  }

  mailmime_set_imf_fields(mime, fields);

  return mime;

 err:
  return NULL;
}

size_t read_file(char **buffer, char *filename) {
	long lSize;
	size_t result;
	FILE *pFile;

	if(fopen_s ( &pFile, filename, "rb" ))
		return 0;

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	*buffer = (char*) malloc (sizeof(char)*lSize);
	if (*buffer == NULL) 
	{
		fclose(pFile);
		return 0;
	}

	// copy the file into the buffer:
	result = fread (*buffer,1,lSize,pFile);
	if (result != lSize)	
	{
		fclose(pFile);
		free(*buffer);
		return 0;
	}

	/* the whole file is now loaded in the memory buffer. */

	// terminate

	fclose (pFile);
	return lSize;
}

int attach_file(struct mailmime *message, char *filepath)
{
  struct mailmime * file_part;
  char *file_text = 0, *filename;
  int file_size;
  int r;

  filename = strrchr(filepath, '\\') + 1;

  file_part = build_body_file(filename);
  if (file_part == NULL)
    return -1;

  if(!(file_size = read_file(&file_text, filepath)))
  {
	  r = -1;
	  goto free_file_without_text;
  }

  mailmime_set_body_text(file_part, file_text, file_size);

  r = mailmime_smart_add_part(message, file_part);
  if (r != MAILIMF_NO_ERROR)
    goto free_file;

  return 0;

free_file:
  free(file_text);
free_file_without_text:
  mailmime_free(file_part);

  return r;
}

int compose_msg(char *from, char *to, char *subject, char *text, char *filepath, FILE *fileout)
{
  struct mailimf_fields * fields;  
  struct mailmime * message;
  struct mailmime * text_part;
  int r;
  int col;

  fields = build_fields(from, to, subject);
  if (fields == NULL)
    goto err;

  message = build_message(fields);
  if (message == NULL)
    goto free_fields;

  text_part = build_body_text(text);
  if (text_part == NULL)
    goto free_message;

  r = mailmime_smart_add_part(message, text_part);
  if (r != MAILIMF_NO_ERROR)
    goto free_text;

  if(filepath && attach_file(message, filepath))
	goto free_message;

  col = 0;
  mailmime_write(fileout, &col, message);
  rewind(fileout);

  mailmime_free(message);

  return 0;

 free_text:
  mailmime_free(text_part);
 free_message:
  mailmime_free(message);
  goto err;
 free_fields:
  mailimf_fields_free(fields);
 err:
  printf("error memory\n");
  return 1;
}


