//#define DEBUG

#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>


#ifdef DEBUG
#define D(...) fprintf(stderr, __VA_ARGS__)
void dumpdata(const void *buf, size_t n) {
  for (int i=0; i<n; i++)
    fprintf(stderr, "%02x%c", ((const unsigned char *)buf)[i], ((i+1) % 16) ? ' ' : '\n');
  fprintf(stderr, "\n");
}

void dumpmsg(const struct msghdr *msg) {
  if (msg->msg_name)
    fprintf(stderr, "Name: %*s\n", msg->msg_namelen, (const char *)msg->msg_name);
  else
    fprintf(stderr, "Name: (null)\n");

  for (int i=0; i<msg->msg_iovlen; i++) {
    fprintf(stderr, "Chunk %i (size %ju):\n", i, (uintmax_t) msg->msg_iov[i].iov_len);
    dumpdata(msg->msg_iov[i].iov_base, msg->msg_iov[i].iov_len);
  }
}

#else
#define D(...) 1
#define dumpdata(x,y) 1
#define dumpmsg(x) 1
#endif

int fds[8];
int fdn = 0;

int socket(int domain, int type, int protocol) {
  int (*real_socket)(int, int, int) = dlsym(RTLD_NEXT, "socket");
  int fd = real_socket(domain, type, protocol);

  if (domain == AF_NETLINK)
    fds[fdn++] = fd;

  D("Called socket(domain=%d, type=%d, protocol=%d) = %d\n", domain, type, protocol, fd);
  return fd;
}

#define MAYBE_SKIP do { int ok = 0; for (int i=0; i<fdn; i++) if (fd == fds[i]) ok = 1; if (!ok) return out; } while (0)

ssize_t send(int fd, const void *buf, size_t n, int flags) {
  ssize_t (*real_send)(int, const void *, size_t, int) = dlsym(RTLD_NEXT, "send");
  ssize_t out = real_send(fd, buf, n, flags);

  MAYBE_SKIP;

  D("Called send(fd=%d, buf=%p, n=%ju, flags=%x) = %jd\n", fd, buf, (uintmax_t) n, flags, (uintmax_t) out);
  dumpdata(buf, n);
  uint32_t size = n;
//  write(3, &size, 4);
  write(3, buf, n);
  return out;
}

ssize_t sendmsg (int fd, const struct msghdr *msg, int flags) {
  ssize_t (*real_sendmsg)(int, const struct msghdr *, int) = dlsym(RTLD_NEXT, "sendmsg");
  ssize_t out = real_sendmsg(fd, msg, flags);

  MAYBE_SKIP;

  D("Called sendmsg(fd=%d, msg=%p, flags=%x) = %jd\n", fd, msg, flags, (uintmax_t) out);
  dumpmsg(msg);
  uint32_t total = 0;
  for (int i=0; i<msg->msg_iovlen; i++)
    total += msg->msg_iov[i].iov_len;

//  write(3, &total, 4);
  for (int i=0; i<msg->msg_iovlen; i++)
    write(3, msg->msg_iov[i].iov_base, msg->msg_iov[i].iov_len);

  return out;
}

#define FLWR  0x80000000

ssize_t recv(int fd, void *buf, size_t n, int flags) {
  ssize_t (*real_recv)(int, void *, size_t, int) = dlsym(RTLD_NEXT, "recv");
  ssize_t out = real_recv(fd, buf, n, flags);

  MAYBE_SKIP;

  D("Called recv(fd=%d, buf=%p, n=%ju, flags=%x) = %jd\n", fd, buf, (uintmax_t) n, flags, (uintmax_t) out);
  dumpdata(buf, out);
  uint32_t size = out | FLWR;
//  write(3, &size, 4);
  write(3, buf, out);
  return out;
}

ssize_t recvmsg (int fd, struct msghdr *msg, int flags) {
  ssize_t (*real_recvmsg)(int, struct msghdr *, int) = dlsym(RTLD_NEXT, "recvmsg");
  ssize_t out = real_recvmsg(fd, msg, flags);

  MAYBE_SKIP;

  D("Called recvmsg(fd=%d, msg=%p, flags=%x) = %jd\n", fd, msg, flags, (uintmax_t) out);
  dumpmsg(msg);
  uint32_t size = out | FLWR;
//  write(3, &size, 4);
  write(3, msg->msg_iov[0].iov_base, out);

  return out;
}
