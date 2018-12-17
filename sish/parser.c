#include "parser.h"

bool 
initialize_parser(struct parser *commandparser, struct commandlist *commands)
{
	if (commandparser == NULL || commands == NULL)
		return false;
	(void)memset(commandparser, 0, sizeof(struct parser));
	commandparser->commands = commands;
	return true;
}

int
parse_command(struct parser *commandparser, const char *commandtext, struct commandlist *commands)
{
	if (commandparser == NULL || commandtext == NULL || commands == NULL)
		return -1;
	if (!initialize_parser(commandparser, commands))
		return -1;
	if (!lexical_analysis(commandparser, commandtext))
		return -2;
	if (!syntactic_analysis(commandparser))
		return -3;
	return 0;
	
}

bool
lexical_analysis(struct parser *commandparser, const char *commandtext)
{
	size_t textlen;
	commandparser->state = PLAINSTATE;
	textlen = strlen(commandtext);
	for (size_t n = 0; n != textlen; ++n) {
		if (is_plaintext(commandtext[n])) {
			if (!flushopbuffer(commandparser))
				return false;
			if (!write_currenttoken(commandparser, commandtext[n]))
				return false;
			commandparser->state = PLAINSTATE;
			continue;
		}
		if (is_controlop(commandtext[n])) {
			if (!flushopbuffer(commandparser))
				return false;
			if (!set_currentctrop(commandparser, commandtext[n]))
				return false;
			commandparser->state = CTROPSTATE;
			continue;
		}
		if (is_singlecommonop(commandtext[n])) {
			if (!flushopbuffer(commandparser))
				return false;
			if (!set_currentctrop(commandparser, commandtext[n]))
				return false;
			commandparser->state = CTROPSTATE;
			continue;
		}
		if (is_doublecommonop(commandtext[n])) {
			if (commandparser->state == D_COMOPSTATE) {
				commandparser->opbuffer[1] = commandtext[n];
				++commandparser->opbufferlen;
				if (!flushopbuffer(commandparser))
					return false;
			}
			else {
				commandparser->opbuffer[0] = commandtext[n];
				++commandparser->opbufferlen;				
			}
			commandparser->state = D_COMOPSTATE;
			continue;
		}
		if (is_seperator(commandtext[n])) {
			if (commandparser->state != SPSTATE) {
				if (!flushopbuffer(commandparser))
					return false;
				if (!set_nexttoken(commandparser))
					return false;
			}
			commandparser->state = SPSTATE;
			continue;
		}
		return false;
	}
	return true;
}

bool
syntactic_analysis(struct parser *commandparser)
{
	size_t commandnum, tokennum;
	struct token *currenttoken, *pretoken, *nexttoken;
	commandnum = commandparser->commands->commandnum;
	for (int i = 0; i != commandnum; ++i) {
		tokennum = commandparser->commands->comlist[i].tokennum;
		for (int n = 0; n != tokennum; ++n) {
			currenttoken = &commandparser->commands->comlist[i].tokens[n];
			if (currenttoken->comop != NOCTROP) {
				if (n == tokennum - 1)
					return false;
				if (n != 0) {
					pretoken = currenttoken - 1;
					if (is_digits(pretoken->content))
						return false;
					nexttoken = currenttoken + 1;
					if (nexttoken->comop == NOCTROP)
						return false;
				}
			}
		}
	}
	return true;
}

bool
is_plaintext(char c)
{
	//cant be ' or " or `
	if(strpbrk(&c, "`\'\"") != NULL)
		return false;
	if (is_controlop(c) || 
		is_singlecommonop(c) || 
		is_singlecommonop(c) || 
		is_seperator(c))
		return false;
	return true;
}

bool 
is_controlop(char c)
{
	if (strpbrk(&c, "&|\n") == NULL)
		return false;
	return true;
}

bool 
is_singlecommonop(char c)
{
	if (strpbrk(&c, "<") == NULL)
		return false;
	return true;
}

bool 
is_doublecommonop(char c)
{
	if (strpbrk(&c, ">") == NULL)
		return false;
	return true;
}

bool 
is_seperator(char c)
{
	if (strpbrk(&c, " \t") == NULL)
		return false;
	return true;
}

bool
is_digits(const char *text)
{	
	if (strspn(text, "0123456789") != strlen(text))
		return false;
	return true;
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
		return NOCTROP;
	}
}

commonop_type
get_singlecommontype(char c)
{
	switch (c) {
	case '<':
		return RDFROMFILE;
	default:
		return NOCOMOP;
	}
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

bool
set_currentctrop(struct parser *commandparser, char c)
{
	commandparser->currentcommand->ctrop = get_ctroptype(c);
	if (!set_nextcommand(commandparser))
		return false;
	return true;
}

bool
set_nexttoken(struct parser *commandparser)
{
	++commandparser->tokenindex;
	if (commandparser->tokenindex > MAX_TOKEN_NUM)
		return false;
	commandparser->characterindex = 0;
	commandparser->currenttoken = &commandparser->currentcommand->tokens[commandparser->tokenindex];
	commandparser->currentcharacter = &commandparser->currenttoken->content[0];
	++commandparser->currentcommand->tokennum;
	return true;
}

bool 
set_nextcommand(struct parser *commandparser)
{
	++commandparser->commandindex;
	if (commandparser->commandindex > MAX_COMMAND_SIZE)
		return false;
	commandparser->tokenindex = 0;
	commandparser->characterindex = 0;
	commandparser->currentcommand = &commandparser->commands->comlist[commandparser->commandindex];
	commandparser->currenttoken = &commandparser->currentcommand->tokens[0];
	commandparser->currentcharacter = &commandparser->currenttoken->content[0];
	++commandparser->commands->commandnum;
	return true;
}

bool 
flushopbuffer(struct parser *commandparser)
{
	commonop_type type;
	if (commandparser->state == S_COMOPSTATE)	
		type = get_singlecommontype(commandparser->opbuffer[0]);
	if (commandparser->state == D_COMOPSTATE)
		type = get_doublecommontype(commandparser->opbuffer);
	commandparser->currenttoken->comop = type;
	commandparser->opbufferlen = 0;
	if (!set_nexttoken(commandparser))
		return false;
	return true;
}

bool 
write_currenttoken(struct parser *commandparser, char c)
{
	if (commandparser->characterindex >= MAX_COMMANDTEXT_SIZE)
		return false;
	commandparser->currenttoken->content[commandparser->characterindex] = c;
	++commandparser->currenttoken->contentlenth;
	++commandparser->characterindex;
	return true;
}