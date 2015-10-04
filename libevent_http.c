#include "evhttp.h"
#include "http-internal.h"
#include <string.h>
#include <iostream>
 
using namespace std;
 
char sendStr[1024]={0}; 
int sendStrLen=0; 
int sendStrPos=0;
 
void http_connection_write_handle(struct evhttp_connection *evconn, void *arg); 
void http_connection_finish_handle(struct evhttp_connection *evconn, void *arg);
 
void http_connection_write_handle(struct evhttp_connection *evconn, void *arg) { 
    struct evhttp_request *req=(struct evhttp_request *)arg; 
    struct evbuffer *databuf=evbuffer_new(); 
    char str[8]={0}; 
    str[0]=sendStr[sendStrPos]; 
    cout<<"sent a letter,will send "<<str<<endl; 
    evbuffer_add_printf(databuf,str); 
    evhttp_send_reply_chunk(req,databuf); 
    sendStrPos++; 
    if(sendStrPos==sendStrLen) 
        evconn->cb=http_connection_finish_handle; 
    else 
        evconn->cb=http_connection_write_handle; 
    evconn->cb_arg=req; 
    evbuffer_free(databuf); 
}
 
void http_connection_finish_handle(struct evhttp_connection *evconn, void *arg) { 
    struct evhttp_request *req=(struct evhttp_request *)arg; 
    evhttp_send_reply_end(req); 
}
 
void http_request_handler(struct evhttp_request *req, void *arg) { 
    struct evbuffer *databuf=evbuffer_new(); 
    evbuffer_add_printf(databuf, "hello world");
 
    evhttp_send_reply_start(req,200,"OK");
    req->evcon->cb=http_connection_write_handle;
    req->evcon->cb_arg=req;
} 
 
int main() { 
    sprintf(sendStr,"hello world"); 
    sendStrLen=strlen(sendStr); 
    struct event_base *base = event_base_new(); 
    struct evhttp *httpd = evhttp_new(base); 
    evhttp_bind_socket(httpd,"0.0.0.0",19800); 
    evhttp_set_gencb(httpd, http_request_handler, NULL); 
    event_base_dispatch(base); 
}
 