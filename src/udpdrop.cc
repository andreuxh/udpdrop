#include "loadsym.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

extern "C" {

int socket(int domain, int type, int protocol)
{
    static auto *sys_socket = LOADSYM(socket);
    return sys_socket(domain, type, protocol);
}

int dup(int oldfd)
{
    static auto *sys_dup = LOADSYM(dup);
    return sys_dup(oldfd);
}

int dup2(int oldfd, int newfd)
{
    static auto *sys_dup2 = LOADSYM(dup2);
    return sys_dup2(oldfd, newfd);
}

int dup3(int oldfd, int newfd, int flags)
{
    static auto *sys_dup3 = LOADSYM(dup3);
    return sys_dup3(oldfd, newfd, flags);
}

/*
    fcntl third argument can be an int, a pointer, or simply not there.
    I believe forwarding it as void * should be safe.
*/
int fcntl(int fd, int cmd, void *arg)
{
    static auto *sys_fcntl = LOADSYM(fcntl);
    return sys_fcntl(fd, cmd, arg);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    static auto *sys_send = LOADSYM(send);
    return sys_send(sockfd, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    static auto *sys_sendto = LOADSYM(sendto);
    return sys_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    static auto *sys_sendmsg = LOADSYM(sendmsg);
    return sys_sendmsg(sockfd, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    static auto *sys_write = LOADSYM(write);
    return sys_write(fd, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    static auto *sys_writev = LOADSYM(writev);
    return sys_writev(fd, iov, iovcnt);
}

}
