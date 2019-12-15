#include "parser.h"

int
initialize_commandlist(struct commandlist *commlist)
{
	if (commlist == NULL) {
		errno = EINVAL;
		return -1;
	}
	(void)memset(&commlist, 0, sizeof(struct commandlist));
	return 0;
}

int 
initialize_parser(struct parser *commparser)
{
	if (commparser == NULL) {
		errno = EINVAL;
		return -1;
	}
	(void)memset(commparser, 0, sizeof(struct parser));
	return 0;
}

int
parse_command(struct parser *commparser, const char *commtext, struct commandlist *commlist)
{
	if (commparser == NULL || commtext == NULL || commlist == NULL) {
		errno = EINVAL;
		return -1;
	}
	
	commparser->p_commlist = commlist;
	commparser->currentcommand = &commlist->comlist[0];
	commparser->currenttoken = &commparser->currentcommand->tokens[0];
	commparser->currentcharacter = &commparser->currenttoken->content[0];
	if (lexical_analysis(commparser, commtext) == -1)
		return -1;
	return syntactic_analysis(commparser);
}

int
lexical_analysis(struct parser *commparser, const char *commtext)
{
	size_t n, textlen;
	
	commparser->state = SPSTATE;
	textlen = strlen(commtext);
	for (n = 0; n != textlen; ++n) {
		if (is_plaintext(commtext[n]) == 1) {
			if (flush_opbuffer(commparser) == -1)
				return -1;
			if (write_currentplaintext(commparser, commtext[n]) == -1)
				return -1;
			commparser->state = PLAINSTATE;
			continue;
		}
		if (is_controlop(commtext[n]) == 1) {
			if (flush_opbuffer(commparser) == -1)
				return -1;
			if (commparser->state == PLAINSTATE)
				if (set_nexttoken(commparser) == -1)
					return -1;
			if (write_currentctrop(commparser, commtext[n]) == -1)
				return -1;
			commparser->state = CTROPSTATE;
			continue;
		}
		if (is_singlecommonop(commtext[n]) == 1) {
			if (commparser->state == PLAINSTATE)
				if (set_nexttoken(commparser) == -1)
					return -1;
			if (flush_opbuffer(commparser) == -1)
				return -1;
			if (write_currentsinglecomop(commparser, commtext[n]) == -1)
				return -1;
			commparser->state = S_COMOPSTATE;
			continue;
		}
		if (is_doublecommonop(commtext[n]) == 1) {
			if (commparser->state == PLAINSTATE)
				if (set_nexttoken(commparser) == -1)
					return -1;
			if (commparser->state == D_COMOPSTATE) {
				commparser->opbuffer[1] = commtext[n];
				++commparser->opbufferlen;
				if (flush_opbuffer(commparser) == -1)
					return -1;
			}
			else {
				commparser->opbuffer[0] = commtext[n];
				++commparser->opbufferlen;
			}
			commparser->state = D_COMOPSTATE;
			continue;
		}
		if (is_seperator(commtext[n]) == 1) {
			if (commparser->state != SPSTATE) {
				if (commparser->state == D_COMOPSTATE) {
					if (flush_opbuffer(commparser) == -1)
						return -1;
				}
				else {
					if (commparser->state != CTROPSTATE)
						if (set_nexttoken(commparser) == -1)
							return -1;
				}
			}
			commparser->state = SPSTATE;
			continue;
		}
		return -1;
	}
	return 0;
}

int
syntactic_analysis(struct parser *commparser)
{
	size_t i, n, commandnum, tokennum;
	struct token *currenttoken, *pretoken, *nexttoken;
	
	commandnum = commparser->p_commlist->commandnum;
	for (i = 0; i != commandnum; ++i) {
		tokennum = commparser->p_commlist->comlist[i].tokennum;
		for (n = 0; n != tokennum; ++n) {
			currenttoken = &commparser->p_commlist->comlist[i].tokens[n];
			/* the current token is a common operator */
			if (currenttoken->comop != NOCOMOP) {
				/* operator doesnt have right-opvalue */
				if (n == tokennum - 1)
					return 1;
				if (n != 0) {
					pretoken = currenttoken - 1;
					/* left-opvalue is digits */
					if (is_digits(pretoken->content))
						return 2;
					nexttoken = currenttoken + 1;
					/* right-opvalue is an operator */
					if (nexttoken->comop != NOCOMOP)
						return 3;
				}
			}
		}
	}
	return 0;
}

int 
set_nextcommand(struct parser *commparser)
{	
	++commparser->p_commlist->commandnum;
	++commparser->commandindex;
	if (commparser->commandindex >= MAX_COMMAND_NUM) {
		errno = ENOBUFS;
		return -1;
	}
	commparser->tokenindex = 0;
	commparser->characterindex = 0;
	commparser->currentcommand = &commparser->p_commlist->comlist[commparser->commandindex];
	commparser->currenttoken = &commparser->currentcommand->tokens[0];
	commparser->currentcharacter = &commparser->currenttoken->content[0];
	
	return 0;
}

int
set_nexttoken(struct parser *commparser)
{
	++commparser->currentcommand->tokennum;
	++commparser->tokenindex;
	if (commparser->tokenindex >= MAX_TOKEN_NUM) {
		errno = ENOBUFS;
		return -1;
	}
	commparser->characterindex = 0;
	commparser->currenttoken = &commparser->currentcommand->tokens[commparser->tokenindex];
	commparser->currentcharacter = &commparser->currenttoken->content[0];	
	return 0;
}

int
set_nextcharacter(struct parser *commparser)
{
	++commparser->currenttoken->contentlenth;
	++commparser->characterindex;
	if (commparser->characterindex >= MAX_COMMANDTEXT_SIZE) {
		errno = ENOBUFS;
		return -1;
	}	
	return 0;
}

int 
write_currentplaintext(struct parser *commparser, char c)
{

	commparser->currenttoken->content[commparser->characterindex] = c;
	if (set_nextcharacter(commparser) == -1)
		return -1;	
	return 0;
}

int
write_currentsinglecomop(struct parser *commparser, char c)
{
	commparser->currenttoken->comop = get_singlecommontype(c);
	if (set_nexttoken(commparser) == -1)
		return -1;
	return 0;
}

int
write_currentctrop(struct parser *commparser, char c)
{
	commparser->currentcommand->ctrop = get_ctroptype(c);	
	if (set_nextcommand(commparser) == -1)
		return -1;
	return 0;
}

int 
flush_opbuffer(struct parser *commparser)
{
	commonop_type type;
	
	if (commparser->opbufferlen == 0)
		return 0;
	if (commparser->opbufferlen == 1)	
		type = get_singlecommontype(commparser->opbuffer[0]);
	if (commparser->opbufferlen == 2)
		type = get_doublecommontype(commparser->opbuffer);
	commparser->currenttoken->comop = type;
	commparser->opbufferlen = 0;
	if (!set_nexttoken(commparser))
		return -1;
	return 0;
}

int
is_plaintext(char c)
{	
	/* cant be ' or " or ` */
	if (strchr("`\'\"", c) != NULL)
		return 0;
	if (is_controlop(c) || 
		is_singlecommonop(c) || 
		is_doublecommonop(c) || 
		is_seperator(c))
		return 0;
	return 1;
}

int 
is_controlop(char c)
{
	if (strchr("&|\n", c) == NULL)
		return 0;
	return 1;
}

int 
is_singlecommonop(char c)
{
	if (strchr("<", c) == NULL)
		return 0;
	return 1;
}

int 
is_doublecommonop(char c)
{
	if (strchr(">", c) == NULL)
		return 0;
	return 1;
}

int 
is_seperator(char c)
{
	if (strchr(" \t", c) == NULL)
		return 0;
	return 1;
}

int
is_digits(const char *text)
{	
	if (strspn(text, "0123456789") != strlen(text))
		return 0;
	return 1;
}

ctrop_type
get_ctroptype(char c)
{
	switch (c) {
	case '&':
		return AMPER;
	case '|':
		return PIPE;
	case'\n':
		return NEWLINE;
	case ';':
		return SEMICOLON;
	default:
		break;
	}
	return NOCTROP;
}

commonop_type
get_singlecommontype(char c)
{
	switch (c) {
	case '<':
		return RDFROMFILE;
	case '>':
		return RDTOFILE;
	default:
		break;
	}
	return NOCOMOP;
}

commonop_type
get_doublecommontype(const char *buffer)
{
	if (strcmp(buffer, ">") == 0)
		return RDTOFILE;
	if (strcmp(buffer, ">>") == 0)
		return APTOFILE;
	return NOCOMOP;
}
