#ifndef CHILD_PROCESS
#define CHILD_PROCESS

#include <stdio.h>
#include <stdlib.h>

#include "cbuffer.h"
#include "settings.h"

#define ERROR_CREATING_CHILD (-1)
#define SUCCESS 1

enum {
  EX_R,  // external read
  EX_W,  // external write
  CH_W,  // children write
  CH_R,  // children read
};

enum {  // enum utilizado para identificar las puntas de los pipes.
  R = 0,
  W = 1,
};

typedef struct {
  int fds[4];

} child_process;

/**
 * Inits the child process and pipes.
 * @param cp The child process sctruct.
 * @return ERROR_CREATING_CHILD, SUCCESS
 */
int init_child(child_process *cp, settings *st);

/**
 * Sets the values of the file desctiptors maps depending if the child needs to
 * read or write. Also gets the buffers form which and to where it needs to
 * write or read. The idea is that it is posible to pass the request buffer of
 * the client or server and use it as a read buffer, so when the child
 * transforms the buffer it writes it to a third buffer to write to the server
 * or client.
 * @param cp The child process struct.
 * @param max_fd The max file descriptor, overwrite a new one if a bigger one is
 * used here.
 * @param readfds The read file descriptor map.
 * @param writefds The write file descriptor map.
 * @param write_to The buffer struct to which this writes.
 * @param read_from The buffer struct to read from.
 */
void set_fd_for_child(child_process *cp, int *max_fd, fd_set *readfds,
                      fd_set *writefds, struct buffer *write_to,
                      struct buffer *read_from);

/**
 * Resolve the buffers after the select was unblocked.
 * @param cp The child process struct.
 * @param readfds The read file descriptor map.
 * @param writefds The write file descriptor map.
 * @param write_to The buffer struct to which this writes.
 * @param read_from The buffer struct to read from.
 */
void resolve_child(child_process *cp, fd_set *readfds, fd_set *writefds,
                   struct buffer *write_to, struct buffer *read_from);

/**
 * Frees the child and the pipes.
 * @param cp The child process struct.
 */
void free_child(child_process *cp);

#endif
