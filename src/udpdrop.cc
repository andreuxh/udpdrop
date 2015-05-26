#include "loadsym.hh"

#include <mutex>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>

#include <stdlib.h>
#include <math.h>


namespace {

std::mutex mutex;

bool is_datagram_socket(int fd)
{
    int socktype;
    socklen_t socktype_len = sizeof(socktype);

    return getsockopt(fd, SOL_SOCKET, SO_TYPE, &socktype, &socktype_len) == 0
        && socktype == SOCK_DGRAM;
}

int get_datagram_drop_rate()
{
    const char *srate = getenv("UDP_DROP_RATE");
    double drate = (srate) ? atof(srate) : 0.0;
    if (drate <= 0.0) {
        return -1;
    }
    else if (drate >= 1.0) {
        return RAND_MAX;
    }
    else {
        return lrint(-1.0 + (1.0 + RAND_MAX) * drate);
    }
}

bool should_drop_datagram(int fd)
{
    if (!is_datagram_socket(fd)) {
        return false;
    }

    static const int datagram_drop_rate = get_datagram_drop_rate();
    return rand() <= datagram_drop_rate;
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

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
    std::unique_lock<std::mutex> lock(mutex);

    static auto *sys_send = LOADSYM(send);

    if (should_drop_datagram(sockfd)) {
        return len;
    }
    return sys_send(sockfd, buf, len, flags);
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
    std::unique_lock<std::mutex> lock(mutex);

    static auto *sys_sendto = LOADSYM(sendto);

    if (should_drop_datagram(sockfd)) {
        return len;
    }
    return sys_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)
{
    std::unique_lock<std::mutex> lock(mutex);

    static auto *sys_sendmsg = LOADSYM(sendmsg);

    if (should_drop_datagram(sockfd)) {
        return iovec_size(msg->msg_iov, msg->msg_iovlen) + msg->msg_controllen;
    }
    return sys_sendmsg(sockfd, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count)
{
    std::unique_lock<std::mutex> lock(mutex);

    static auto *sys_write = LOADSYM(write);

    if (should_drop_datagram(fd)) {
        return count;
    }
    return sys_write(fd, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
{
    std::unique_lock<std::mutex> lock(mutex);

    static auto *sys_writev = LOADSYM(writev);

    if (should_drop_datagram(fd)) {
        return iovec_size(iov, iovcnt);
    }
    return sys_writev(fd, iov, iovcnt);
}

}
