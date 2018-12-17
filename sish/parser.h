#ifndef _PARSER_
#define _PARSER_

#include <stdbool.h>
#include "config.h"

#define MAX_OP_LEN 2

typedef enum parsestate { 
	PLAINSTATE = 1,
	CTROPSTATE,
	S_COMOPSTATE,
	D_COMOPSTATE,
	SPSTATE
	
}parsestate;

typedef enum ctrop_type { 
	NOCTROP = 1,
	NEWLINE,
	PIPE,
	AMPER,
	SEMICOLON
}ctrop_type;

typedef enum commonop_type { 
	NOCOMOP = 1,
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
	struct command comlist[MAX_COMMAND_SIZE];
	size_t commandnum;
};

struct parser {
	struct commandlist *commands;
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

bool initialize_parser(struct parser *, struct commandlist *);
int parse_command(struct parser *, const char *, struct commandlist *);
bool lexical_analysis();
bool syntactic_analysis();
bool is_commonoptoken(struct token *);
bool is_plaintext(char);
bool is_controlop(char);
bool is_singlecommonop(char);
bool is_doublecommonop(char);
bool is_seperator(char);
bool is_digits(const char *);
ctrop_type get_ctroptype(char);
commonop_type get_singlecommontype(char);
commonop_type get_doublecommontype(const char *);
bool write_currenttoken(struct parser *, char);
bool set_currentctrop(struct parser *, char);
bool set_currentsinglecomop(struct parser *, char);
bool set_nexttoken(struct parser *);
bool set_nextcommand(struct parser *);
bool flushopbuffer(struct parser *);

#endif // !_PARSER_