#ifndef _RPC_SERVICE_H_
#define _RPC_SERVICE_H_

int rpc_server_new();
void rpc_server_run();
void rpc_server_stop();
void rpc_server_free();

char *rpc_error_message(int err);

#endif