#ifndef _PARSER_
#define _PARSER_

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "config.h"

#define MAX_TOKEN_NUM 50
#define MAX_COMMAND_NUM 15
#define MAX_OP_LEN 2

typedef enum parsestate { 
	PLAINSTATE = 0,
	CTROPSTATE,
	S_COMOPSTATE,
	D_COMOPSTATE,
	SPSTATE
	
}parsestate;

typedef enum ctrop_type { 
	NOCTROP = 0,
	NEWLINE,
	PIPE,
	AMPER,
	SEMICOLON
}ctrop_type;

typedef enum commonop_type { 
	NOCOMOP = 0,
	RDTOFILE,
	APTOFILE,
	RDFROMFILE
}commonop_type;

struct token {
	char content[MAX_COMMANDTEXT_SIZE];
	size_t contentlenth;
	commonop_type comop;
};

struct command {
	struct token tokens[MAX_TOKEN_NUM];
	size_t tokennum;
	ctrop_type ctrop;
};

struct commandlist {
	struct command comlist[MAX_COMMAND_NUM];
	size_t commandnum;
};

struct parser {
	struct commandlist *p_commlist;
	size_t commandindex;
	size_t tokenindex;
	size_t characterindex;
	struct command *currentcommand;
	struct token *currenttoken;
	char *currentcharacter;
	char opbuffer[2];
	size_t opbufferlen;
	parsestate state;
};

int initialize_commandlist(struct commandlist *);
int initialize_parser(struct parser *);
int parse_command(struct parser *, const char *, struct commandlist *);
int lexical_analysis(struct parser *, const char *);
int syntactic_analysis(struct parser *);
int set_nextcharacter(struct parser *);
int set_nexttoken(struct parser *);
int set_nextcommand(struct parser *);
int write_currentplaintext(struct parser *, char);
int write_currentctrop(struct parser *, char);
int write_currentsinglecomop(struct parser *, char);
int flush_opbuffer(struct parser *);

/* util functions */
int is_plaintext(char);
int is_controlop(char);
int is_singlecommonop(char);
int is_doublecommonop(char);
int is_seperator(char);
int is_digits(const char *);
ctrop_type get_ctroptype(char);
commonop_type get_singlecommontype(char);
commonop_type get_doublecommontype(const char *);

#endif
