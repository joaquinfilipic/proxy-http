CFLAGS= -Wall -Wextra -Wfloat-equal -Wshadow -Wpointer-arith -Wstrict-prototypes -Wcast-align -Wstrict-overflow=5 -Waggregate-return -Wcast-qual -Wswitch-default -Wswitch-enum -Wunreachable-code -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Werror -pedantic-errors -Wmissing-prototypes -pedantic -std=c99 -D_DEFAULT_SOURCE
SCTP_FLAG=-lsctp
PTHREAD_FLAG=-pthread
CC=gcc

PROXY_OUT=proxy

PROXY_SOURCES=main.c cbuffer.c clients.c to_origin_server.c utilities.c logger.c child_process.c main_options.c clients_manager.c config_utilities.c master_socket.c config_socket.c settings.c

CONFIG_CLIENT_OUT=config_client

CONFIG_CLIENT_SOURCES=config_client.c

PARSER_SOURCES=HTTP_parser/parser.c HTTP_parser/commons.c HTTP_parser/store_manager.c HTTP_parser/parser_utils.c

all: $(PROXY_OUT) $(CONFIG_CLIENT_OUT)

$(PROXY_OUT):
	$(CC) -o $(PROXY_OUT) $(PARSER_SOURCES) $(PROXY_SOURCES) $(SCTP_FLAG) $(PTHREAD_FLAG) $(CFLAGS)

$(CONFIG_CLIENT_OUT):
	$(CC) -o $(CONFIG_CLIENT_OUT) $(CONFIG_CLIENT_SOURCES) $(SCTP_FLAG) $(CFLAGS)

clean:
	@rm -f *.o proxy config_client
	@echo clean done