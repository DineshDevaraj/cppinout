
/**
 *
 * Author  : D.Dinesh
 *             dinesh@techybook.com
 * Licence : Refer licence file
 *
 **/

/* Compilation command g++ cinout.cpp -o cinout */
 
# include <cstdio>
# include <string.h>
# include <ctype.h>
# include <errno.h>

# include <string>

#define skip_space(cr) while(isspace(*cr)) cr++

#define skip_char(cr) skip_untill(cr, '\'')
#define skip_string(cr) skip_untill(cr, '"')

#define skip_args(cr) skip_within(cr, '(', ')')
#define skip_block(cr) skip_within(cr, '{', '}')

#define skip_single_line(cr) while(*cr++ != '\n')

typedef std::string Stdstr;

struct Tokens
{
    enum Value
    {
        Space                = ' ' ,
        LineEnd                = ';' ,
        CharBegin            = '\'',
        Directive            = '#' ,
        ScopeResol            = ':' ,
        StringBegin            = '"' ,
        
        OpenParan            = '(' ,    /* open paranthesis   */
        CloseParan            = ')' ,    /* close paranthesis  */
        OpenBraces            = '{' ,    /* open curly braces  */
        CloseBraces            = '}' ,    /* close curly braces */
        
        Identifier            ,
        MultiLineComment    ,
        SingleLineComment    ,
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
        if('\\' == *cr) cr++;    /* skip this and next characted 
                                   as this can be escape sequence */
    cr++;                        /* skip the delimeter at the last */
}

/* is - identifier string */
void get_identifier(Buffer_t &cr, char is[])
{
    *is = *cr; /* copy the first character as it is */
    while('_' == cr[1] or isalnum(cr[1]))
    { is++; cr++; *is = *cr; }
}

/* st -> string token */
Tokens_t next_token(Buffer_t &cr, char st[])
{
    bool bwl = 0; /* break while loop */
   char *tk = st;
   Tokens_t tt; /* token type */
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
         
         case ':'  : if(':' == cr[1])
                        tt = Tokens::ScopeResol;
                     break;
                     
         case '/'  : if('*' == cr[1])
                        tt=Tokens::MultiLineComment;
                     else if('/' == cr[1])
                        tt=Tokens::SingleLineComment;
                    break;

         case '#'  : tt=Tokens::Directive;
                     break;

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
   Tokens_t tt; /* token type */
   Buffer_t cr(hay);
   Stdstr previden;
   while(*cr)
   {
       char tk[256] = {};
       tt=next_token(cr, tk);
       #if(1==DEBUG)
       printf("Token (%d %c) : <%s> %u,%d\n", tt, tt, tk, cr.nline, cr.ncol);
       #endif
       switch(tt)
       {
           case Tokens::Identifier :
                if(0 == strcasecmp(tk, "return")) 
                {
                    printf("         ");
                    printf("return : %u, %d : %d\n", cr.nline, cr.ncol, cr.offset());
                }
                previden=tk;
                break;

           case Tokens::Space   :
           case Tokens::LineEnd :        
                skip_space(cr);
                break;

           case Tokens::OpenParan :     
                   skip_args(cr);
                break;

           case Tokens::CloseParan :
           {
              skip_space(cr);
               tt=next_token(cr, tk);
              if(Tokens::OpenBraces == tt 
                 and previden != "if"
                 and previden != "for"
                 and previden != "while"
                 and previden != "switch")
                  printf("Function entry  : %u, %d : %d\n", cr.nline, cr.ncol, cr.offset());
              break;
           }

           case Tokens::Directive :
                skip_single_line(cr);
                break;           
           case Tokens::CharBegin : 
                skip_char(cr);
                break;
           case Tokens::StringBegin : 
                skip_string(cr);
                break;
           case Tokens::MultiLineComment : 
                skip_multiline_comment(cr);
                break;
           case Tokens::SingleLineComment : 
                skip_single_line(cr);
                break;
           #if(2==DEBUG)
           default : printf("Invalid token (%d - %c) : %u, %d : %d\n", 
                             tt, tt, cr.nline, cr.ncol, cr.offset());
           #endif
      }
   }
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
   len = fread(filebody, sizeof(char), sizeof filebody, fh);
   filebody[len] = 0;
   parse_buffer(filebody);
   return 0;
}
