#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "lua.h"
#include "lualib.h"
#include <string.h>
#include <errno.h>

#define MAX_EVENTS 1024

int vlm_create_socket(lua_State *L) {
    size_t size = 0;
    const char *spath = luaL_checklstring(L, 1, &size);

    struct sockaddr_un path = { AF_UNIX };
    memcpy(path.sun_path, spath, size);

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
    bind(sock, (struct sockaddr*)&path, offsetof(struct sockaddr_un, sun_path) + size + 1);
    listen(sock, 4096);

    lua_pushinteger(L, sock);
    return 1;
}

int vlm_epoll_create(lua_State *L) {
    printf("EPOLLIN = %d\nEPOLLRDHUP = %d\n", EPOLLIN, EPOLLRDHUP);

    int socket = luaL_checkinteger(L, 1);
    int epoll_fd = epoll_create1(0);

    struct epoll_event ev = {
        .events = EPOLLIN,
        .data = {
            .fd = socket
        }
    };
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket, &ev);

    lua_pushinteger(L, epoll_fd);
    return 1;
}

int vlm_epoll_wait(lua_State *L) {
    int epoll_fd = luaL_checkinteger(L, 1);
    int timeout = luaL_checkinteger(L, 2);

    struct epoll_event events[MAX_EVENTS] = {};
    int n_events = epoll_wait(epoll_fd, events, MAX_EVENTS, timeout);

    lua_createtable(L, n_events, 0);
    for(int i = 0; i < n_events; i++) {
        lua_pushinteger(L, i+1);
        lua_newtable(L);
        lua_pushstring(L, "fd");
        lua_pushinteger(L, events[i].data.fd);
        lua_settable(L, -3);
        lua_pushstring(L, "type");
        lua_pushinteger(L, events[i].events);
        lua_settable(L, -3);
        lua_settable(L, -3);
    }
    return 1;
}

int vlm_epoll_add(lua_State *L) {
    int epoll_id = luaL_checkinteger(L, 1);
    int fd = luaL_checkinteger(L, 2);
    struct epoll_event ev = {
        .events = EPOLLIN | EPOLLRDHUP,
        .data = {
            .fd = fd
        }
    };
    int status = epoll_ctl(epoll_id, EPOLL_CTL_ADD, fd, &ev);

    lua_pushinteger(L, status);
    return 1;
}

int vlm_accept_client(lua_State *L) {
    int sock = luaL_checkinteger(L, 1);
    int connection = accept(sock, NULL, NULL);

    lua_pushinteger(L, connection);
    return 1;
}

int vlm_recv(lua_State *L) {
    int connection = luaL_checkinteger(L, 1);
    char *data = malloc(1024);
    if(!data) {
        luaL_error(L, "Failed to allocate [%d] bytes of memory for socket [%d]\n",
                   1024, connection);
    }
    char *handle = data;
    ssize_t length = 0;
    ssize_t size = 1024;
    ssize_t received = 0;

    while((received = recv(connection, handle, size-length, 0)) == 1024) {
        length += received;
        if(length == size) {
            data = realloc(data, size + 1024);
            size += 1024;
        }
        handle = data + length;
    }
    length += received;
    if(length == 0 && received < 0)
        luaL_error(L, "Failed to receive data: %s", strerror(errno));
    if(length)
        lua_pushlstring(L, data, length);
    else
        lua_pushnil(L);

    free(data);
    return 1;
}

int vlm_send(lua_State *L) {
    size_t size = 0;
    const char *msg = luaL_checklstring(L, 2, &size);
    int sock = luaL_checkinteger(L, 1);
    send(sock, msg, size, 0);
    return 0;
}

int vlm_close(lua_State *L) {
    close(luaL_checkinteger(L, 1));
    return 0;
}
