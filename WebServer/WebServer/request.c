#include "request.h"

/*
 *Omit the null lines ahead of request-line.
 *Trim all LWS and change them into SP.
 */
int trim_request(char *newbuffer, int newbuffersize, char *oldbuffer, int oldbuffersize)
{
	trimstate state;
	int replacestartindex, replaceendindex, newbufferindex;
	int n;
	bool omitflag;
	state = OSTATE;
	replacestartindex = 0;
	replaceendindex = 0;
	newbufferindex = 0;
	n = 0;
	omitflag = false;
	for (int checkindex = 0; checkindex != oldbuffersize; ++checkindex) {
		if (!omitflag) {
			if (oldbuffer[checkindex] != ' ' && oldbuffer[checkindex] != '\t' && oldbuffer[checkindex] != '\r' && oldbuffer[checkindex] != '\n')
				omitflag = true;
		}
		if (omitflag) {
			switch (oldbuffer[checkindex]) {
			case '\r':
				if (state == LWSSTATE) {
					if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
						newbuffer[newbufferindex] = ' ';
						++newbufferindex;
					}
				}
				else {
					for (n = 0; n != replaceendindex - replacestartindex; ++n)
						newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
					newbufferindex += n;
				}
				replacestartindex = checkindex;
				replaceendindex = checkindex + 1;
				state = RSTATE;
				break;
			case '\n':
				if (state == RSTATE) {
					++replaceendindex;
					state = NSTATE;
					break;
				}
				else {
					if (state == LWSSTATE) {
						if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
							newbuffer[newbufferindex] = ' ';
							++newbufferindex;
						}
					}
					else {
						for (n = 0; n != replaceendindex - replacestartindex; ++n)
							newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
						newbufferindex += n;
					}
					newbuffer[newbufferindex] = oldbuffer[checkindex];
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = OSTATE;
					break;
				}
			case '\t':
				if (state == OSTATE) {
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = HTSTATE;
					break;
				}
				if (state == RSTATE) {
					newbuffer[newbufferindex] = '\r';
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = HTSTATE;
					break;
				}
				if (state == SPSTATE || state == HTSTATE || state == NSTATE || state == LWSSTATE) {
					++replaceendindex;
					state = LWSSTATE;
					break;
				}
				break;
			case ' ':
				if (state == OSTATE) {
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = SPSTATE;
					break;
				}
				if (state == RSTATE) {
					newbuffer[newbufferindex] = '\r';
					++newbufferindex;
					replacestartindex = checkindex;
					replaceendindex = checkindex + 1;
					state = SPSTATE;
					break;
				}
				if (state == SPSTATE || state == HTSTATE || state == NSTATE || state == LWSSTATE) {
					++replaceendindex;
					state = LWSSTATE;
					break;
				}
				break;
			default:
				if (state == LWSSTATE) {
					if (newbufferindex != 0 && newbuffer[newbufferindex - 1] != ' ') {
						newbuffer[newbufferindex] = ' ';
						++newbufferindex;
					}
				}
				else {
					for (n = 0; n != replaceendindex - replacestartindex; ++n)
						newbuffer[newbufferindex + n] = oldbuffer[replacestartindex + n];
					newbufferindex += n;
				}
				newbuffer[newbufferindex] = oldbuffer[checkindex];
				++newbufferindex;
				replacestartindex = 0;
				replaceendindex = 0;
				state = OSTATE;
				break;
			}
			if (newbufferindex == newbuffersize)
				return newbufferindex;
		}
	}
	return newbufferindex;
}

linestatus 
readline(struct httprequest *req, char *input, int length)
{
	
}