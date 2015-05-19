#include "loadsym.hh"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <stdlib.h>


namespace {

bool is_datagram_socket(int fd)
{
    int socktype;
    socklen_t socktype_len = sizeof(socktype);

    return getsockopt(fd, SOL_SOCKET, SO_TYPE, &socktype, &socktype_len) == 0
        && socktype == SOCK_DGRAM;
}

double get_datagram_drop_rate()
{
    const char *datagram_drop_rate = getenv("UDP_DROP_RATE");
    return (datagram_drop_rate) ? atof(datagram_drop_rate) : 0.0;
}

double randu()
{
    return rand() / ((double)RAND_MAX + 1);
}

bool should_drop_datagram(int fd)
{
    if (!is_datagram_socket(fd)) {
        return false;
    }

    static const double datagram_drop_rate = get_datagram_drop_rate();
    return randu() < datagram_drop_rate;
}

size_t iovec_size(const struct iovec *iov, int iovcnt)
{
    size_t size = 0;
    while (iovcnt--) {
        size += iov[iovcnt].iov_len;
    }
    return size;
}

}


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

int close(int fd)
{
    static auto *sys_close = LOADSYM(close);
    return sys_close(fd);
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    static auto *sys_send = LOADSYM(send);

    if (should_drop_datagram(sockfd)) {
        return len;
    }
    return sys_send(sockfd, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    static auto *sys_sendto = LOADSYM(sendto);

    if (should_drop_datagram(sockfd)) {
        return len;
    }
    return sys_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    static auto *sys_sendmsg = LOADSYM(sendmsg);

    if (should_drop_datagram(sockfd)) {
        return iovec_size(msg->msg_iov, msg->msg_iovlen) + msg->msg_controllen;
    }
    return sys_sendmsg(sockfd, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    static auto *sys_write = LOADSYM(write);

    if (should_drop_datagram(fd)) {
        return count;
    }
    return sys_write(fd, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    static auto *sys_writev = LOADSYM(writev);

    if (should_drop_datagram(fd)) {
        return iovec_size(iov, iovcnt);
    }
    return sys_writev(fd, iov, iovcnt);
}

}
