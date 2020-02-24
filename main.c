#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define N 4096

typedef struct Node *link1;
typedef struct Node {
    char* data;
    link1 next;
} node;
typedef link1 list;

int chmode(char c) /*returns type of the char*/
{
    const char* normal = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890_/.-"; /*array of chars of the 1st type of words*/
    const char* special = "|&()<>;"; /*array of chars of the 2nd type of words*/
    if (strchr(normal, c) != NULL)
        return 0;
    else
    if (strchr(special, c) != NULL)
        return 1;
    else
        return -1;
}

void wordtolist(list *l, char* c) /*adds the word c to the list l*/
{
    if (*l == NULL)
    {
        (*l) = (link1) malloc(sizeof(node));
        (*l)->data = (char*) malloc(sizeof(char)*(strlen(c)+1));
        int i;
        for (i = 0; i <= strlen(c) + 1; (*l)->data[i] = c[i++]);
        (*l)->next = NULL;
    }
    else
        wordtolist(&((*l)->next), c);
}

void rmlist(list *l) /*makes list's memory free*/
{
    if (*l != NULL)
    {
        rmlist(&((*l)->next));
        free((*l)->data);
        free(*l);
    }
}

list mklist(char* p) /*makes list l of the words from string p; returns length of the list l* or -1 if smth wrong*/
{
    int i = 0, mode, len, start, finish, j, k = 0;
    list l = NULL;
    char space[3] = " \t\n";
    char* w;
    while (strchr(space, p[i]) != NULL) /*missing space chars*/
        ++i;
    while (i <= strlen(p)) /*working while not the end of string*/
    {
        mode = chmode(p[i]); /*change mode of current word*/
        finish = start = i; /*initialization of the word's borders*/
        switch (mode)
        {
            case 0:  while (mode == 0) /*looking for the end of current word*/
                {
                    finish++;
                    mode = chmode(p[++i]);
                }
                break;
            case 1:  if (strchr("|&>", p[i]) != NULL && p[i] == p[i+1]) /*checking for the 2nd type words with length = 2*/
                {
                    finish++;
                    i++;
                }
                finish++;
                i++;
                break;
            case -1: rmlist(&l);
                return NULL; /*wrong input*/
                break;
        }
        len = finish - start; /*length of the current word*/
        w = (char*) malloc(sizeof(char)*(len+1));
        for (j = 0; j <= len-1; w[j] = p[start+j], j++); /*initializating of current word for transmittal to the function*/
        w[j] = '\0'; /*this is the string!*/
        wordtolist(&l, w), k++; /*adding word to list and counting the added words*/
        free(w); /*don't need more this word*/
        while (strchr(space, p[i]) != NULL) /*missing space chars*/
            ++i;
    }
    return l;
}







void listop(list* l) /*output the list l*/
{
    list p = *l;
    while (p != NULL)
    {
        fprintf(stdout, "%s ", p->data);
        p = p->next;
    }
    printf("\n");
}



typedef struct cmd_inf *slink;
typedef struct cmd_inf {
    char **argv;
    char *infile;
    char *outfile;
    int apnd;
    slink *backgrnd;
    slink *psubcmd;
    slink *pipe;
    slink *next;
} snode;
typedef slink slist;


void slistop(slist *l)
{
    slist sp = *l;
    int i;
    while (sp != NULL)
    {
        printf("argv: ");
        i = 0;
        while (sp->argv[i] != NULL)
        {
            printf("%s ", sp->argv[i]);
            i++;
        }
        i = 0;
        putchar('\n');

        printf("infile: ");
        if (sp->infile != NULL)
            printf("%s", sp->infile);
        else
            printf("0");
        putchar('\n');

        printf("outfile: ");
        if (sp->outfile != NULL)
            printf("%s", sp->outfile);
        else
            printf("0");
        putchar('\n');

        printf("backgrnd: ");
        if (sp->backgrnd != NULL)
        {
            i = 1;
            printf("1");
        }
        else
            printf("0");
        putchar('\n');

        printf("psubcmd: ");
        if (sp->psubcmd != NULL)
        {
            i = 2;
            printf("1");
        }
        else
            printf("0");
        putchar('\n');

        printf("pipe: ");
        if (sp->pipe != NULL)
        {
            i = 3;
            printf("1");
        }
        else
            printf("0");
        putchar('\n');

        printf("next: ");
        if (sp->next != NULL)
        {
            i = 4;
            printf("1");
        }
        else
            printf("0");
        putchar('\n');

        putchar('\n');
        switch (i)
        {
            case 0: sp = NULL;
                break;
            case 1: sp = sp->backgrnd;
                break;
            case 2: sp = sp->psubcmd;
                break;
            case 3: sp = sp->pipe;
                break;
            case 4: sp = sp->next;
                break;
        }
    }
}

void rmslist(slist *sl)
{
    if (*sl != NULL)
    {
        rmslist(&((*sl)->backgrnd));
        rmslist(&((*sl)->psubcmd));
        rmslist(&((*sl)->pipe));
        rmslist(&((*sl)->next));
        free((*sl)->argv);
        free(*sl);
    }
}

void init_slist(slist *l)
{
    slist sl = *l;
    sl->argv = (char **) malloc(sizeof(char *));
    sl->argv[0] = NULL;
    sl->infile = NULL;
    sl->outfile = NULL;
    sl->apnd = 0;
    sl->backgrnd = NULL;
    sl->psubcmd = NULL;
    sl->pipe = NULL;
    sl->next = NULL;
}

slist mkslist(list *l1)
{
    int i;
    list p, l = *l1;
    slist sp, sl;
    p = l;
    sl = (slink) malloc(sizeof(snode));
    init_slist(&sl);
    i = 0;
    sp = sl;
    while (p != NULL)
    {
        if (!strcmp(p->data, "<"))
        {
            p = p->next;
            sp->infile = p->data;
            p = p->next;
            continue;
        }
        if (!strcmp(p->data, ">"))
        {
            p = p->next;
            sp->outfile = p->data;
            p = p->next;
            continue;
        }
        if (!strcmp(p->data, ">>"))
        {
            p = p->next;
            sp->outfile = p->data;
            sp->apnd = 1;
            p = p->next;
            continue;
        }
        if (!strcmp(p->data, "&"))
        {
            sp->backgrnd = (slink) malloc(sizeof(snode));
            sp = sp->backgrnd;
            init_slist(&sp);
            i = 0;
            p = p->next;
            continue;
        }
        if (!strcmp(p->data, "|"))
        {
            sp->pipe = (slink) malloc(sizeof(snode));
            sp = sp->pipe;
            init_slist(&sp);
            i = 0;
            p = p->next;
            continue;
        }
        if (!strcmp(p->data, ";"))
        {
            sp->next = (slink) malloc(sizeof(snode));
            sp = sp->next;
            init_slist(&sp);
            i = 0;
            p = p->next;
            continue;
        }
        sp->argv = (char **) realloc(sp->argv, sizeof(char *)*(i+2));
        sp->argv[i] = p->data;
        sp->argv[i+1] = NULL;
        i++;
        p = p->next;
    }
    return sl;
}

void chdr(list *l)
{
  char* s;
  if (*l == NULL)
  {
    s = getenv("HOME");
    chdir(s);
  }
  else
  {
    s = (*l)->data;
    chdir(s);
  }
}

void openinf(slist *sl)
{
  int f;
  f = open((*sl)->infile, O_RDONLY);
  dup2(f, 0);
  close(f);
  return;
}

void openoutf(slist *sl)
{
  FILE* f1;
  int f;
  if ((*sl)->apnd)
    f1 = fopen((*sl)->outfile, "a");
  else
    f1 = fopen((*sl)->outfile, "w");
  f = fileno(f1);
  dup2(f, 1);
  close(f);
  return;
}

void exc(slist *sl)
{
  if ((*sl)->backgrnd == NULL)
    signal(SIGINT, SIG_DFL);
  execvp((*sl)->argv[0], (*sl)->argv);
  perror(NULL);
  exit(-1);
}

pid_t pid[100];
pid_t fpid[100];
int i, j, pn;

void killzomb()
{
  int k = j;
  while (i >= 0) 
  { 
    waitpid(pid[i], NULL, 0); 
    i--; 
  }
  while (k <= 99)
  {
    waitpid(pid[k], NULL, WNOHANG);
    k++;
  }
  return;
}

void chpn(slist *sl)
{
  if ((*sl)->next == NULL)
    pn = 1;
  else
  {
    pn = 0;
    waitpid(pid[i], NULL, 0);
    i--;
  }
  return;
}

void shell(slist *l)
{
  slist sl = *l;
  FILE* f1;
  int fd[2], in, out, k, next_in;
  i = -1;
  j = 100;
  pipe(fd); 
  out = fd[1]; 
  next_in = fd[0];
  if (sl->backgrnd != NULL)
  {
    j--;
    k = j;
  }
  else
  {
    i++;
    k = i;
  }
  if (!(pid[k] = fork())) {
      close(next_in);
      if (sl->infile != NULL)
          openinf(&sl);
      if (sl->outfile != NULL)
          openoutf(&sl);
      else if (sl->pipe != NULL)
          dup2(out, 1);
      close(out);
      exc(&sl);
  }
  chpn(&sl);
  in = next_in;
  if (sl->pipe != NULL || sl->next != NULL || sl->backgrnd != NULL)
  {
    if (sl->pipe != NULL)
      sl = sl->pipe;
    if (sl->next != NULL)
      sl = sl->next;
    if (sl->backgrnd != NULL)
      sl = sl->backgrnd;
    while (sl->pipe != NULL || sl->next != NULL || sl->backgrnd != NULL)
    {
      close(out);
      pipe(fd);
      out = fd[1];     
      next_in = fd[0];
      if (sl->backgrnd != NULL)
      {
        j--;
        k = j;
      }
      else
      {
        i++;
        k = i;
      }
      if (!(pid[k] = fork()))
      {
        close(next_in);
        if (sl->infile != NULL)
          openinf(&sl);
        else
          if (pn)
            dup2(in, 0);
        close(in);
        if (sl->outfile != NULL)
          openoutf(&sl);
        else
          if (sl->pipe != NULL)
            dup2(out, 1);
        close(out);
        exc(&sl);
      }
      chpn(&sl);
      close(in);
      in = next_in;
      if (sl->pipe != NULL)
        sl = sl->pipe;
      if (sl->next != NULL)
        sl = sl->next;
      if (sl->backgrnd != NULL)
        sl = sl->backgrnd;
    }
    close(out); 
    if (sl->backgrnd != NULL)
    {
      j--;
      k = j;
    }
    else
    {
      i++;
      k = i;
    }
    if (!(pid[k] = fork()))
    {
      if (sl->infile != NULL)
        openinf(&sl);
      else
        if (pn)
          dup2(in, 0);
      close(in);
      if (sl->outfile != NULL)
        openoutf(&sl);
      exc(&sl);
    }
  }
  close(in); 
  killzomb();
  return;
}

int main(int argc, char *argv[])
{
  char buf[N] = "";
  list l;
  int i, pidsh;
  slist sl;
  signal(SIGINT, SIG_IGN);
  for (;;) {
    fprintf(stderr, "> ");
    i = read(0, buf, N);
    buf[i-1] = '\0';
    if (!strcmp(buf, "exit") || !i)
      break;
    if (i == 1)
      continue;
    if (l != NULL)
      sl = mkslist(&l);
    if (!strcmp(l->data, "cd"))
      chdr(&(l->next));
    else
      shell(&sl);
    rmslist(&sl);
    rmlist(&l);
  }
}
