
/**
 *
 * Author  : D.Dinesh
 *           dinesh@techybook.com
 * Licence : Refer licence file
 *
 **/

/* Compilation command g++ cinout.cpp -o cppio */

# include <cstdio>
# include <string.h>
# include <strings.h>

# include <ctype.h>
# include <errno.h>
# include <sys/stat.h>

# include <string>

#define skip_space(cr) while(isspace(*cr)) cr++

#define skip_char(cr) skip_untill(cr, '\'')
#define skip_string(cr) skip_untill(cr, '"')

#define skip_block(cr) skip_within(cr, '{', '}')

#define skip_single_line(cr) while(*cr++ != '\n')

typedef std::string Stdstr;

struct Tokens
{
   enum Value
   {
      Undef             ,

      Colon             = ':' ,
      Space             = ' ' ,
      LineEnd           = ';' ,
      Asterisk          = '*' ,
      CharBegin         = '\'',
      Directive         = '#' ,
      ScopeResol        = 'R' ,
      StringBegin       = '"' ,

      OpenParan         = '(' ,   /* open paranthesis   */
      CloseParan        = ')' ,   /* close paranthesis  */
      OpenBraces        = '{' ,   /* open curly braces  */
      CloseBraces       = '}' ,   /* close curly braces */

      Identifier        = 'I' ,
      MultiLineComment  = 'M' ,
      SingleLineComment = 'S' ,
   };
};

typedef Tokens::Value Tokens_t;

struct Buffer_t
{
   char *sp; /* starting position */
   char *cp; /* current position  */

   size_t ncol;  /* column number */
   size_t nline; /* line number   */

   Buffer_t(char *p)
   {
      sp = cp = p;
      ncol = nline = 1;
   }

   char operator * ()
   { return *cp; }

   Buffer_t & operator ++ (int)
   {
      if('\n' == *cp)
      {
         ncol = 0;
         nline++;
      }
      cp++; ncol++;
      return *this;
   }

   char operator [] (int I)
   { return cp[I]; }

   char operator += (int I)
   {
      int J = 0;
      for( ; J < I; J++)
         (*this)++;
      return *cp;
   }

   operator bool ()
   { return bool(*cp); }

   size_t offset()
   { return size_t(cp-sp); }
};

/* function declarations */
Tokens_t next_token(Buffer_t &, char []);
void default_actions(Buffer_t &, Tokens_t, int &, char * &);

void skip_multiline_comment(Buffer_t &cr)
{
   while(*cr && cr[1])
   {
     if('*' == *cr && '/' == cr[1])
     { cr+=2; return; }
     cr++;
   }
}

/* cr - cursor od - open delimeter cd - close delimeter */
void skip_within(Buffer_t &cr, const char od, const char cd)
{
   int c = 1; /* count */
   do {
     if(od == *cr) c++;
     else if(cd == *cr) c--;
   }while(c && *cr++);
}

void skip_untill(Buffer_t &cr, const char dl)
{
   for( ; dl != *cr; cr++)
      if('\\' == *cr) cr++;   /* skip this and next characted
                           as this can be escape sequence */
   cr++;                  /* skip the delimeter at the last */
}

void skip_args(Buffer_t &cr)
{
   int sh;        /* space holder */
   char *cp;      /* char pointer */
   Tokens_t tt;   /* token type   */

   for( ; ; )
   {
      char tk[256] = {};
      tt=next_token(cr, tk);
      switch(tt)
      {
         case Tokens::Directive : break;
         case Tokens::CloseParan : return;
         default : default_actions(cr, tt, sh, cp);
      }
   }
}

void skip_function_def(Buffer_t &cr)
{
   int sh;        /* space holder */
   char *cp;      /* char pointer */
   Tokens_t tt;   /* token type   */

   /* number of open curly barces */
   for(int nocb = 1; nocb; )
   {
      char tk[256] = {};
      tt=next_token(cr, tk);
      switch(tt)
      {
         case Tokens::OpenBraces :
         {
            nocb++;
            break;
         }
         case Tokens::CloseBraces :
         {
            nocb--;
            break;
         }
         case Tokens::Identifier :
         {
            if(0 == strcasecmp(tk, "return"))
               printf("   return : %u, %d : %d\n", cr.nline, cr.ncol, cr.offset());
            break;
         }
         default : default_actions(cr, tt, sh, cp);
      }
   }
}

/* is - identifier string */
void get_identifier(Buffer_t &cr, char is[])
{
   *is = *cr; /* copy the first character as it is */
   while('_' == cr[1] or isalnum(cr[1]))
   { is++; cr++; *is = *cr; }
}

void default_actions(Buffer_t &cr, Tokens_t tt, int &rtsl, char * &rtsp)
{
   char tk[256] = {};
   switch(tt) /* ttb - token type branch */
   {
      case Tokens::Colon :
      case Tokens::Space :
      {
         skip_space(cr);
         break;
      }
      case Tokens::OpenParan :
      {
         skip_args(cr);
         break;
      }
      case Tokens::Directive :
      {
         skip_single_line(cr);
         skip_space(cr);
         rtsl=cr.nline;
         rtsp=cr.cp;
         break;
      }
      case Tokens::CharBegin :
      {
         skip_char(cr);
         break;
      }
      case Tokens::StringBegin :
      {
         skip_string(cr);
         break;
      }
      case Tokens::MultiLineComment :
      {
         skip_multiline_comment(cr);
         break;
      }
      case Tokens::SingleLineComment :
      {
         skip_single_line(cr);
         break;
      }
      case Tokens::ScopeResol :
      {
         skip_space(cr);
         tt=next_token(cr, tk);
         break;
      }
      default :
      {
         skip_space(cr);
         rtsl=cr.nline;
         rtsp=cr.cp;
      }
   }
}

/* st -> string token */
Tokens_t next_token(Buffer_t &cr, char st[])
{
   bool bwl = 0; /* break while loop */
   char *tk = st;
   Tokens_t tt = Tokens::Undef; /* token type */
   while(*cr && 0==bwl)
   {
      bwl=1;
     *tk = *cr;
      switch(*cr)
      {
         case ' '  :
         case '\t' :
         case '\r' :
         case '\n' : tt=Tokens::Space;
                  break;

         case ';'  : tt=Tokens::LineEnd;
                 break;
         case '('  : tt=Tokens::OpenParan;
                 break;
         case ')'  : tt=Tokens::CloseParan;
                 break;
         case '{'  : tt=Tokens::OpenBraces;
                 break;
         case '}'  : tt=Tokens::CloseBraces;
                 break;

         case '\'' : tt=Tokens::CharBegin;
                break;
         case '"'  : tt=Tokens::StringBegin;
                break;

         case ':'  :
         {
            if(cr[1] != ':')
            { tt = Tokens::Colon; }
            else { cr++; tt = Tokens::ScopeResol; }
         }
         break;

         case '/'  : if('*' == cr[1])
                  tt=Tokens::MultiLineComment;
                else if('/' == cr[1])
                  tt=Tokens::SingleLineComment;
               break;

         case '#'  : tt=Tokens::Directive; break;

         default : if('_' == *cr or
                ('a' <= *cr and *cr <= 'z') or
                ('A' <= *cr and *cr <= 'Z'))
               {
                  get_identifier(cr, tk);
                  tt=Tokens::Identifier;
                  break;
               }
               bwl=0;
      }
     tk++; cr++;
   }
   return tt;
}

void parse_buffer(char *hay)
{
   int   rtsl; /* return type starting line     */
   char *bssp; /* blank space startnig position */
   char *rtsp; /* return type starting position */
   char *iend; /* identifier end                */

   Stdstr ct;        /* construct type      */
   Tokens_t tt;      /* token type          */
   int fcount = 0;   /* number of functions */
   Stdstr previden;  /* previous identifier */

   rtsl = 1;
   Buffer_t cr(hay);
   skip_space(cr);

   rtsp = cr.cp;
   static char hl[128];   /* horizontal line */
   memset(hl, '-', 100);

   while(*cr)
   {
      char tk[256] = {};
      tt=next_token(cr, tk);
      #if(1==DEBUG)
      printf("Token (%d %c) : <%s> %u,%d\n", tt, tt, tk, cr.nline, cr.ncol);
      #endif
      TTB : switch(tt) /* ttb - token type branch */
      {
         case Tokens::Identifier :
         {
            if(0 == strcmp(tk, "class") or 0 == strcmp(tk, "struct"))
            {
               ct=tk;
               rtsl=cr.nline;
               skip_space(cr);
               tt=next_token(cr, tk); /* This gets the class name */
               if(tt == Tokens::OpenBraces)
               { strcpy(tk, "<anonymous>"); }
               printf("\n%s\n\n", hl);
               printf("%s %s\n\n", ct.data(), tk);
               printf("   %centry : %d, %u, %d : %d\n", ct.at(0),
                           rtsl, cr.nline, cr.ncol, cr.offset());
            }
            previden=tk;
            iend=cr.cp;
            break;
         }

         case Tokens::OpenParan :
         {
            skip_args(cr);
            for(int cpl = 1; cpl; )
            {
               cpl = 0;                /* close paranthesis loop */
               bssp = cr.cp;
               skip_space(cr);
               tt=next_token(cr, tk);
               if(Tokens::OpenBraces == tt
                  and previden != "if"
                  and previden != "for"
                  and previden != "while"
                  and previden != "switch")
               {
                  fcount++;
                  printf("\n%s\n\n", hl);
                  printf("%.*s%.*s\n\n", iend-rtsp, rtsp, bssp-iend, iend);
                  printf("   fentry : %d, %u, %d : %d\n", rtsl, cr.nline, cr.ncol, cr.offset());
                  skip_function_def(cr);
                  skip_space(cr);
                  rtsl=cr.nline;
                  rtsp=cr.cp;
               }
               else switch(tt)
               {
                  case Tokens::Identifier :
                  if(0 == strcmp(tk, "const"))
                  {
                     cpl=1;
                     break;
                  }
                  else goto TTB;
                  case Tokens::MultiLineComment :
                  {
                     skip_multiline_comment(cr);
                     cpl=1;
                     break;
                  }
                  case Tokens::SingleLineComment :
                  {
                     skip_single_line(cr);
                     cpl=1;
                     break;
                  }
                  default : goto TTB;
               }
            }
            break;
         }

         default : default_actions(cr, tt, rtsl, rtsp);
      }
   }
   fcount && printf("\n");
}

char filebody[8*1024];

int main(int argc, char *argv[])
{
   int len = 0;
   if(argc < 2)
   {
      printf("usage> %s <C/C++ file name>\n", argv[0]);
      return 1;
   }

   FILE *fh = fopen(argv[1], "r");
   if(NULL == fh)
   {
      printf("%s : %s\n", strerror(errno), argv[1]);
      return 1;
   }

   char *hay = 0;
   struct stat st;
   stat(argv[1], &st);

   hay = new (std::nothrow) char [st.st_size]();
   if(0 == hay)
   {
      printf("Request for %ld byte memory failed\n", st.st_size);
      return 1;
   }

   while(len < st.st_size)
     len += fread(hay + len, sizeof(char), st.st_size, fh);
   hay[len] = 0;

   parse_buffer(hay);

   return 0;
}
