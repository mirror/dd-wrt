#include "config.h"

#include <stdio.h>

#ifdef unix
# include <signal.h>
#endif

#include "slang.h"
#include "jdmacros.h"

#ifndef HELP_DIR
#define HELP_DIR ""
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#endif


typedef struct SLhlp_Node_Type
{
   char *name;
   struct SLhlp_Node_Type *child;
   struct SLhlp_Node_Type *sister;
   unsigned long pos;
} SLhlp_Node_Type;

typedef struct
{
   FILE *fp;
   SLhlp_Node_Type *root;
   SLhlp_Node_Type *sub;	       /* current subtopic node */
   SLhlp_Node_Type *now;	       /* Node we are currently reading from */
   SLhlp_Node_Type *path[10];	       /* path to current node */
   int level;
} SLhlp_File_Type;

#define MAX_HELP_FILES 10

SLhlp_File_Type Help_Files[MAX_HELP_FILES];


static int add_level (int level, int fd, char *name, unsigned long pos)
{
   SLhlp_Node_Type *node, *parent, *new_node;
   int len;
   
   while (*name == ' ') name++;
   len = strlen(name);
   
   if (len) while (name[len - 1] == ' ') len--;
   if (len == 0) return -1;
   name[len] = 0;
   
   parent = node = Help_Files[fd].root;
   while (level-- > 0)
     {
	if (node == NULL)
	  {
	     return -1;
	  }
	while (node->sister != NULL) node = node->sister;
	parent = node;
	node = node->child;
     }
   
   
   if ((NULL == (new_node = (SLhlp_Node_Type *) SLMALLOC (sizeof (SLhlp_Node_Type))))
       || (NULL == (new_node->name = (char *) SLMALLOC (len + 1))))
     {
	SLang_Error = SL_MALLOC_ERROR;
	return -1;
     }
   
   new_node->sister = NULL;
   new_node->child = NULL;
   new_node->pos = pos;
   strcpy (new_node->name, name);
   
   if (node == NULL)
     {
	if (parent == NULL) 
	  {
	     Help_Files[fd].root = new_node;
	  }
	else parent->child = new_node;
     }
   else
     {
	while (node->sister != NULL) node = node->sister;
	node->sister = new_node;
     }
   return 0;
}


static void free_nodes (SLhlp_Node_Type *node)
{
   if (node->child != NULL) free_nodes (node->child);
   if (node->sister != NULL) free_nodes (node->sister);
   if (node->name != NULL) SLFREE (node->name);
   SLFREE (node);
}



static int check_fd (int fd)
{
   if (((fd >= MAX_HELP_FILES) || (fd < 0)) || (Help_Files[fd].fp == NULL))
     {
	if (SLang_Error == 0) SLang_Error = INTRINSIC_ERROR;
	return 0;
     }
   return 1;
}


static void SLhlp_close_help (int fd)
{
   if (!check_fd (fd)) return;
   fclose (Help_Files[fd].fp);  Help_Files[fd].fp = NULL;
   if (Help_Files[fd].root != NULL) free_nodes (Help_Files[fd].root);
}


static int SLhlp_open_help (char *file)
{  
   int fd = 0, ch;
   FILE *fp;
   int level;
   char topic[256], *b;
   unsigned long pos;
   
   while ((fd < MAX_HELP_FILES) && (Help_Files[fd].fp != NULL))
     fd++;
   
   if (fd == MAX_HELP_FILES) return -1;
   if (NULL == (Help_Files[fd].fp = fp = fopen (file, "r"))) return -1;
   
   
   while (1)
     {
	ch = getc (fp);
	
	bypass_getc:
	
	if ((ch > '9') || (ch == ' ')) continue;
	if (ch == '\n')
	  {
	     ch = getc (fp);
	     if ((ch == ' ') || (ch > '9')) continue;
	     if (ch >= '1')
	       {
		  level = ch - '1';
		  pos = ftell (fp);
		  b = topic;
		  while (('\n' != (ch = getc (fp))) && (ch != EOF))
		    {
		       *b++ = ch;
		    }
		  *b = 0;
		  if (add_level (level, fd, topic, pos)) goto error;
	       }
	     goto bypass_getc;
	  }
	if (ch == EOF) break;
     }

   Help_Files[fd].now = NULL;
   Help_Files[fd].sub = Help_Files[fd].root;
   Help_Files[fd].path[0] = NULL;
   
   return fd;

   
   error:
   SLhlp_close_help (fd);
   return -1;
}

static char *SLhlp_get_subtopic (int fd)
{
   SLhlp_Node_Type *node;
   
   if (!check_fd(fd)) return NULL;
   if ((node = Help_Files[fd].sub) == NULL) return NULL;
   
   Help_Files[fd].sub = node->sister;
   return node->name;
}
   

static char *SLhlp_gets (int fd, char *buf)
{
   SLhlp_Node_Type *node;
   char ch;
   
   
   if (!check_fd(fd)) return NULL;
   
   node = Help_Files[fd].now;
   if (node == NULL) return NULL;

   if ((NULL == fgets(buf, 255, Help_Files[fd].fp))
       || ((*buf <= '9') && (*buf >= '0')))
     {
	if ((node->child == NULL) && Help_Files[fd].level)
	  {
	     Help_Files[fd].level--;
	  }
	Help_Files[fd].now = NULL;
	return NULL;
     }
   
   /* First character is reserved. */
   if (*buf > ' ')
     {
	ch = *buf | 0x20;
	if ((ch < 'a') || (ch > 'z'))
	  {
	     *buf = '\n';
	     *(buf + 1) = 0;
	  }
     }
   
   return buf;
}

static int my_strncasecmp (char *a, char *b, int n)
{
   register char cha, chb;
   
   while (n--)
     {
	cha = *a++;
	chb = *b++;
	if (cha != 0) 
	  {
	     if ((cha == chb)
		 || ((cha | 0x20) == (chb | 0x20))) continue;
	  }
	return cha - chb;
     }
   return 0;
}



static int SLhlp_select_topic (int fd, char *topic)
{
   char topic_buf[256];
   char *t, ch;
   int level = 0;
   int len;
   
   SLhlp_Node_Type *node, *parent;
   
   if (!check_fd (fd)) return -1;

   parent = node = Help_Files[fd].root;

   ch = 0;
   if (topic != NULL) 
     while (((ch = *topic) <= ' ') && ch) topic++;
   
   if (ch != 0) while (node != NULL)
     {
	parent = node;
	t = topic_buf;
	
	while ((ch = *topic) > ' ')
	  {
	     *t++ = ch;
	     topic++;
	  }
	while (((ch = *topic) <= ' ') && ch) topic++;
	*t = 0;
	
	len = strlen (topic_buf);
	
   
	if (len) while (my_strncasecmp (node->name, topic_buf, len))
	  {
	     node = node->sister;
	     if (node == NULL) break;
	  }
	
	if (node == NULL) break;
	Help_Files[fd].path[level++] = node;
	if ((ch == 0) || (ch == '\n')) break;
	node = node -> child;
     }

   /* level comes out at 1 greater than last valid one */
   Help_Files[fd].level = level;
   
   if (node != NULL)		       /* success */
     {
	Help_Files[fd].sub = node->child;
	Help_Files[fd].now = node;
	fseek (Help_Files[fd].fp, node->pos, SEEK_SET);
	return 0;
     }
   else				       /* failure */
     {
	Help_Files[fd].now = NULL;
	Help_Files[fd].sub = parent;
	return -1;
     }
}

static int SLhlp_what_topic (int fd, char *buf)
{
   int level, i;
   char *b;
   if (!check_fd(fd)) return -1;
   
   level = Help_Files[fd].level;
   *buf = 0;
   if (level == 0) return 0;
   
   b = buf;
   for (i = 0; i < level; i++)
     {
	strcpy (b, Help_Files[fd].path[i]->name);
	b = b + strlen (b);
	*b++ = ' ';
     }
   *(b - 1) = 0;
   return level;
}

static int SLhlp_up_topic (int fd)
{
   if (!check_fd(fd)) return -1;
   
   if (Help_Files[fd].level) Help_Files[fd].level--;
   return 0;
}




/* --------------------------------------------------------------------- */

static void cls (void)
{
   SLtt_cls ();
   SLtt_flush_output ();
}


static void newline (int nl, int reset, int cls_flag)
{
   static int n;
   
   if (nl) putc('\n', stdout);
   n++;
      
   if (n == SLtt_Screen_Rows - 1)
     {
	SLtt_reverse_video (1);
	SLtt_write_string ("---Press RETURN to continue.---");
	SLtt_normal_video ();
	SLtt_flush_output ();
	
	SLang_getkey ();
	if (cls_flag) cls ();
	else
	  fputs("\r                               \r", stdout);

	cls_flag = 0;
	n = 0;
     }

   if (reset) 
     {
	n = 0;
	if (cls_flag) cls ();
	return;
     }
}

SLang_RLine_Info_Type *Help_Rli;

static SLang_RLine_Info_Type  *init_readline (void)
{
   unsigned char *buf = NULL;
   SLang_RLine_Info_Type *rli;
   
   if ((NULL == (rli = (SLang_RLine_Info_Type *) SLMALLOC (sizeof(SLang_RLine_Info_Type))))
       || (NULL == (buf = (unsigned char *) SLMALLOC (256))))
     {
	fprintf(stderr, "malloc error.\n");
	exit(-1);
     }
   
   SLMEMSET ((char *) rli, 0, sizeof (SLang_RLine_Info_Type));
   rli->buf = buf;
   rli->buf_len = 255;
   rli->tab = 8;
   rli->dhscroll = 20;
   rli->getkey = SLang_getkey;
   rli->tt_goto_column = NULL;
   rli->update_hook = NULL;
   
   if (SLang_init_readline (rli) < 0)
     {
	fprintf(stderr, "Unable to initialize readline library.\n");
	exit (-1);
     }
   
   return rli;
}

static char *hlp_get_input (char *prompt)
{
   int i;
   
   if (Help_Rli == NULL) 
     {
	Help_Rli = init_readline ();
	Help_Rli->update_hook = NULL;
     }
   
   Help_Rli->edit_width = SLtt_Screen_Cols - 1;
   Help_Rli->prompt = prompt;
   *Help_Rli->buf = 0;
   
   
   i = SLang_read_line (Help_Rli);
   
   if ((i >= 0) && !SLang_Error && !SLKeyBoard_Quit)
     {
	SLang_rline_save_line (Help_Rli);
     }
   else return NULL;
   
   return (char *) Help_Rli->buf;
}

	
static void do_subtopics (int fd)
{
   char *s;
   int len, dlen;
   
   if ((s = SLhlp_get_subtopic (fd)) != NULL)
     {
	len = 0;
	newline (1, 0, 0);
	fprintf(stdout, "Available Topics/Subtopics:");
	newline (1, 0, 0);
	newline (1, 0, 0);
	do
	  {
	     dlen = strlen (s);
	     len += dlen;
	     
	     if (len >= 80)
	       {
		  len = dlen;
		  newline (1, 0, 0);
	       }
	     fputs(s, stdout);
	     dlen = 21 - len % 20;
	     len += dlen;
	     if (len < 80)
	       {
		  while (dlen--) putc(' ', stdout);
	       }
	  }
	while (NULL != (s = SLhlp_get_subtopic(fd)));
	newline (1, 0, 0);
     }
}

static void get_screen_size (int sig)
{
   SLtt_get_screen_size ();
#ifdef SIGWINCH
   signal (SIGWINCH, get_screen_size);
#endif
}


   
int main (int argc, char **argv)
{
   char *buf, *file;
   char buffer[256];
   static int fd = -1;
   int len;
   
   if (argc != 2)
     {
	fprintf(stderr, "Usage: slhelp HELP-FILE\n");
	exit (-1);
     }
   
   file = argv[1];
   if (fd < 0) fd = SLhlp_open_help (file);
   if (fd < 0) 
     {
	fprintf (stderr, "Unable to open help file %s\n", file);
	exit (-1);
     }
   
   SLang_init_tty (7, 1, 1);
   SLtt_get_terminfo ();
   SLtt_init_video ();
   SLtt_Use_Ansi_Colors = 0;
   
#ifdef SIGWINCH
   signal (SIGWINCH, get_screen_size);
#endif
   
   get_screen_size (0);
   cls ();

   fputs ("\n\nSimply press RETURN at the 'Topic>' prompt to quit help.\n\n",
	  stdout);

   if (SLhlp_what_topic (fd, buffer) > 0)
     {
	SLhlp_select_topic (fd, buffer);
     }
      
   while (1)
     {
	while (1)
	  {
	     if (SLhlp_what_topic (fd, buffer) <= 0)
	       {
		  SLhlp_select_topic (fd, "?");
		  *buffer = 0;
	       }
	     do_subtopics (fd);
	     
	     len = strlen (buffer);
	     if (len) strcat (buffer, " Subtopic> ");
	     else strcpy (buffer, "Topic> ");
	     newline (1, 1, 0);
	     
	     if (NULL == (buf = hlp_get_input (buffer)))
	       {
		  SLang_Error = 0;
		  if (SLKeyBoard_Quit) 
		    {
		       SLKeyBoard_Quit = 0;
		       continue;
		    }
		  goto the_return;
	       }
	     
	     if ((*buf == '\n') || (*buf == 0))
	       {
		  if (len == 0) goto the_return;
		  SLhlp_up_topic (fd);
		  continue;
	       }
	     
	     buffer[len] = ' ';	       /* kill prompt */
	     strcpy (buffer + (len + 1), buf);
	     break;
	  }
	
	newline (1, 1, 1);
	if (-1 == SLhlp_select_topic (fd, buffer))
	  {
	     fprintf(stdout, "No help available on: %s", buf);
	     newline (1, 0, 0);
	  }
	else while (NULL != SLhlp_gets (fd, buffer))
	  {
	     fputs(buffer, stdout);
	     newline (0, 0, 1);
	  }
     }
   
   the_return:
   
   SLtt_reset_video ();
   SLang_reset_tty ();
   return 0;
}
