/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// socket.cc
// generic socket class

#include <iostream>
#include <sstream>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x501
#endif
#include <ws2tcpip.h>
#include <winsock2.h>
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif
#else // !WIN32
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif // WIN32

#include "socket.h"

#define ADDRLEN sizeof(sockaddr)

// NET_ERROR(POSIX, WSA)
// IS_NET_ERROR(POSIX, WSA)
// SET_NET_ERROR(POSIX, WSA)
#ifdef WIN32
#define NET_ERROR(p, w) (w)
#define IS_NET_ERROR(p, w) (WSAGetLastError() == (w))
#define SET_NET_ERROR(p, w) WSASetLastError(w)
#define NET_ERROR_STR() wsastrerror(WSAGetLastError())
inline std::string wsastrerror(int errnum)
{ std::ostringstream out; out << "error=" << errnum; return out.str(); }
#else // !WIN32
#define NET_ERROR(p, w) (p)
#define IS_NET_ERROR(p, w) (errno == (p))
#define SET_NET_ERROR(p, w) (errno = (p))
#define NET_ERROR_STR() strerror(errno)
#endif // WIN32

using namespace std;

#define dout if(verbose && out) *out
#define derr if(verbose && err) *err

/* notes

struct sockaddr {
  sa_family_t sa_family; // address family, AF_xxx
  char sa_data[14];      //  14 bytes of protocol address
};

struct sockaddr_in {
  sa_family_t sin_family;  // address family, AF_xxx
  in_port_t sin_port;      // Port number
  struct in_addr sin_addr; // Internet address
}

struct in_addr {
  __u32   s_addr;
};

*/

//// local utils ////

inline bool set_flags(size_t &flag, size_t bit, int value=1)
{
  // toggle
  if(value == -1) value = 1 - ((flag>>bit) & 1);
  flag &= ~(1 << bit);
  flag |= (value << bit);
  return value;
}

inline bool get_flags(size_t flag, size_t bit)
{
  return (flag & (1 << bit));
}

int get_net_addr(unsigned int *net_addr, const char *netaddress, int family, int type, int protocol)
{
  if(!net_addr) return -1;
  addrinfo hints;
  addrinfo *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_family = family;
  hints.ai_socktype = type;
  hints.ai_protocol = protocol;
  if(int ret = getaddrinfo(netaddress, NULL, &hints, &res)) // reentrant
    {
      cerr << "getaddrinfo: " << gai_strerror(ret) << endl;
      return -1;
    }
  *net_addr = ((sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  return 1;
}

// set addr to net_addr and port
void set_addr(sockaddr *addr, int domain, unsigned int net_addr, int port)
{
  if(domain == AF_INET)
    {
      sockaddr_in *addr_in = (sockaddr_in*)addr;
      memset((char*)addr_in, 0, ADDRLEN);
      addr_in->sin_family = AF_INET;
      addr_in->sin_port = port;
      addr_in->sin_addr.s_addr = net_addr;
    }
  else
    {
      cerr << "set_addr: unsupported domain" << endl;
      exit(-1);
    }
}

ostream &operator<<(ostream &out, const sockaddr &addr)
{
  if(addr.sa_family == AF_INET)
    {
      const sockaddr_in *in = (const sockaddr_in*)&addr;
      out << inet_ntoa(in->sin_addr) << ":" << ntohs(in->sin_port);
    }
  return out;
}

//// basic_socket ////

basic_socket::basic_socket(int domain, int type, int sock, const sockaddr &iaddr) :
  _domain(domain), _type(type), _protocol(0), _sock(sock), _flags(0),
  send_flags(MSG_NOSIGNAL), recv_flags(MSG_NOSIGNAL), iaddr(iaddr),
  verbose(1), out(&cout), err(&cerr)
{
  if(_sock > -1) getsaddr();
}

basic_socket::basic_socket(const basic_socket &s) :
  _domain(s.domain()), _type(s.type()), _protocol(s.protocol()), _sock(s.sock()), _flags(s._flags),
  send_flags(s.send_flags), recv_flags(s.recv_flags), saddr(s.saddr), oaddr(s.oaddr), iaddr(s.iaddr),
  verbose(s.verbose), out(s.out), err(s.err)
{
}

void basic_socket::set(const basic_socket &s)
{
  *this = s;
}

bool basic_socket::setf(size_t bit, int value)
{
  return set_flags(_flags, bit, value);
}

bool basic_socket::getf(size_t bit) const
{
  return get_flags(_flags, bit);
}

void basic_socket::nonblock(bool value) const
{
#ifdef WIN32
  ULONG argp = value;
  if(ioctlsocket(_sock, FIONBIO, &argp))
    {
      perror("ioctlsocket");
      exit(-1);
    }
#else // !WIN32
  int flags = fcntl(_sock, F_GETFL, 0);
  if(value) flags |= O_NONBLOCK;
  else flags &= (~O_NONBLOCK);
  if(fcntl(_sock, F_SETFL, flags) < 0)
    {
      perror("fcntl");
      exit(-1);
    }
#endif // WIN32
}

int basic_socket::setsockopt(int level, int opt, int value) const
{
  return ::setsockopt(_sock, level, opt, (char*)&value, sizeof(value));
}

int basic_socket::setsockopt(int level, int opt, const char *value, int len) const
{
  return ::setsockopt(_sock, level, opt, value, len);
}

int basic_socket::getsockopt(int level, int opt, int &value) const
{
  socklen_t l = sizeof(value);
  return ::getsockopt(_sock, level, opt, (char*)&value, &l);
}

int basic_socket::getsockopt(int level, int opt) const
{
  int value = 0;
  return getsockopt(level, opt, value) ? 0 : value;
}

int basic_socket::geterror() const
{
  int err = 0;
  return getsockopt(SOL_SOCKET, SO_ERROR, err) == 0 ? err : -1;
}

int basic_socket::getnetaddr(const char *netaddress, unsigned int *net_addr) const
{
  if(!netaddress || !net_addr) return -1;
  addrinfo hints;
  addrinfo *res = NULL;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG;
  hints.ai_family = domain();
  hints.ai_socktype = type();
  hints.ai_protocol = protocol();
  if(int ret = getaddrinfo(netaddress, NULL, &hints, &res)) // reentrant
    {
      derr << "error: " << netaddress << ": " << gai_strerror(ret) << endl;
      return -1;
    }
  *net_addr = ((sockaddr_in*)res->ai_addr)->sin_addr.s_addr;
  freeaddrinfo(res);
  return 1;
}

int basic_socket::getbroadcast(unsigned int *net_addr) const
{
  if(net_addr) *net_addr = htonl(INADDR_BROADCAST);
  return 1;
}

int basic_socket::broadcast(int flag) const
{
  return setsockopt(SOL_SOCKET, SO_BROADCAST, flag);
}

int basic_socket::send_bufsize(int size) const
{
  return setsockopt(SOL_SOCKET, SO_SNDBUF, size);
}

int basic_socket::recv_bufsize(int size) const
{
  return setsockopt(SOL_SOCKET, SO_RCVBUF, size);
}

// set addr to netaddress:port, or broadcast:port
int basic_socket::setaddr(sockaddr *addr, const char *netaddress, int port) const
{
  unsigned int net_addr = INADDR_ANY;

  if(netaddress)
    {
      if(getnetaddr(netaddress, &net_addr) == -1)
        return -1;
    }
  else if(getbroadcast(&net_addr) == -1)
    {
      derr << "error: could not get broadcast address" << endl;
      return -1;
    }

  set_addr(addr, domain(), net_addr, htons(port));

  return 1;
}

const char* basic_socket::getaddr(const sockaddr *addr, int *port) const
{
  if(addr->sa_family != AF_INET) return 0;
  if(port) *port = ntohs(((const sockaddr_in*)addr)->sin_port);
  return inet_ntoa(((const sockaddr_in*)addr)->sin_addr);
}

const char* basic_socket::getsaddr(int *port)
{
  socklen_t addrlen = ADDRLEN;
  getsockname(_sock, &saddr, &addrlen);
  return getaddr(&saddr, port);
}

int basic_socket::setoaddr(const char *netaddress, int port)
{
  int ret = setaddr(&oaddr, netaddress, port);
  if(ret < 0) close();
  return ret;
}

int basic_socket::setiaddr(const char *netaddress, int port)
{
  int ret = setaddr(&iaddr, netaddress, port);
  if(ret < 0) close();
  return ret;
}

const char* basic_socket::getoaddr(int *port) const { return getaddr(&oaddr, port); }

const char* basic_socket::getiaddr(int *port) const { return getaddr(&iaddr, port); }

// create socket, set options
int basic_socket::socket()
{
  int reuse_addr = 1;
  int nodelay = 1;

  // create socket
  _sock = ::socket(domain(), type(), 0);
  if(_sock < 0)
    {
      perror("socket");
      exit(-1);
    }

  nonexec(_sock);

  // reuse the address
  if(reuse_addr && setsockopt(SOL_SOCKET, SO_REUSEADDR, reuse_addr) == -1)
    {
      perror("setsockopt(SO_REUSEADDR)");
      exit(-1);
    }

  // no delay
  if(type() == SOCK_STREAM && nodelay && setsockopt(SOL_TCP, TCP_NODELAY, nodelay) == -1)
    {
      perror("setsockopt(TCP_NODELAY)");
      exit(-1);
    }

  dout << "new socket (" << _sock << ")" << endl;

  return _sock;
}

// set oaddr to netaddress:port, or broadcast:port
int basic_socket::socket(const char *netaddress, unsigned short port)
{
  if(socket() < 0) return _sock;

  setoaddr(netaddress, port);

  return _sock;
}

int basic_socket::bind(const char *netaddress, unsigned short port)
{
  if(socket() < 0) return _sock;

  int ret = setaddr(&saddr, netaddress, port);
  if(ret < 0) { close(); return ret; }

  // bind socket to netaddress:port
  ret = ::bind(_sock, &saddr, ADDRLEN);
  if(ret < 0)
    {
      perror("bind");
      close();
      return -1;
    }

  dout << "bind (" << _sock << "): " << saddr << endl;

  return _sock;
}

int basic_socket::listen(unsigned short port)
{
  if(socket() < 0) return _sock;

  set_addr(&saddr, domain(), INADDR_ANY, htons(port));

  // bind socket to INADDR_ANY:port
  int ret = ::bind(_sock, &saddr, ADDRLEN);
  if(ret < 0)
    {
      perror("bind");
      close();
      return -1;
    }

  if(type() == SOCK_STREAM) ::listen(_sock, 5);

  dout << "listen (" << _sock << "): " << saddr << endl;

  return _sock;
}

int basic_socket::connect(const char *netaddress, unsigned short port)
{
  if(socket() < 0) return _sock;

  if(setoaddr(netaddress, port) < 0) return _sock;

  int ret = ::connect(_sock, &oaddr, ADDRLEN);
  if(ret < 0)
    {
      derr << "error: connect to " << netaddress << ": " << NET_ERROR_STR() << endl;
      close();
      return ret;
    }

  getsaddr();
  dout << "connect (" << _sock << "): " << saddr << " to " << oaddr << endl;

  return _sock;
}

// nonblocking connect, use with connected()
// return 0 on waiting
int basic_socket::connect(const char *netaddress, unsigned short port, long timeout)
{
  if(socket() < 0) return _sock;

  if(setoaddr(netaddress, port) < 0) return _sock;

  nonblock();

  int ret = ::connect(_sock, &oaddr, ADDRLEN);
  if(ret < 0 && IS_NET_ERROR(EINPROGRESS, WSAEWOULDBLOCK) && (ret = connected(timeout)) == 0)
    return 0; // return 0 on waiting
  if(ret < 0)
    {
      derr << "error: connect to " << netaddress << ": " << NET_ERROR_STR() << endl;
      close();
      return ret;
    }

  return _sock;
}

// return
//  sock on success
//  0 on waiting
// -1 on error
int basic_socket::connected(long timeout)
{
  // wait for timeout
  int ret = select(timeout, 2);
  if(ret == 0)
    {
      // no timeout -- no error
      if(timeout == 0) return 0;

      // socket not ready -- not connected in time
      SET_NET_ERROR(ETIMEDOUT, WSAETIMEDOUT);
      return -1;
    }

  // have socket, check for any error
  ret = geterror();
  if(ret)
    {
      if(ret > 0) SET_NET_ERROR(ret, ret);
      return -1;
    }

  getsaddr();
  dout << "connect (" << _sock << "): " << saddr << " to " << oaddr << endl;

  return _sock;
}

// flag: 1 - read
// flag: 2 - write
// flag: 3 - read+write
int basic_socket::select(timeval &tv, int flag)
{
  if(_sock < 0) return -1;
  fd_set rfds, wfds;
  FD_ZERO(&rfds);
  FD_ZERO(&wfds);
#ifndef WIN32
  if(flag & 1) FD_SET(_sock, &rfds);
  if(flag & 2) FD_SET(_sock, &wfds);
#else
  if(flag & 1) FD_SET(u_int(_sock), &rfds);
  if(flag & 2) FD_SET(u_int(_sock), &wfds);
#endif
  int ret = ::select(flag ? _sock+1 : 0, &rfds, &wfds, 0, &tv);

  if(ret < 0)
    {
      if(errno == EINTR) return 0;
      perror("select");
      close();
      return ret;
    }

  return ret;
}

int basic_socket::select(long timeout, int flag)
{
  if(_sock < 0) return -1;
  timeval tv = {timeout / 1000000, timeout % 1000000};
  return select(tv, flag);
}

int basic_socket::peek() const
{
  if(_sock < 0) return -1;
#ifdef WIN32
  u_long value = 0;
  int ret = ioctlsocket(_sock, FIONREAD, &value);
#else // !WIN32
  int value = 0;
  int ret = ioctl(_sock, FIONREAD, &value);
#endif
  return ret < 0 ? ret : value;
}

basic_socket basic_socket::accept(long timeout)
{
  int new_sock = -1;
  int ret = select(timeout, 1);

  if(ret > 0)
    {
      socklen_t addrlen = ADDRLEN;
      new_sock = ::accept(_sock, &iaddr, &addrlen);
      if(new_sock < 0) perror("accept");
      else dout << "accept (" << new_sock << "): " << iaddr << endl;
    }
  return basic_socket(domain(), type(), new_sock, iaddr);
}

int basic_socket::close()
{
  if(_sock < 0) return 0;
  dout << "close socket (" << _sock << ")" << endl;
#ifdef WIN32
  int ret = closesocket(_sock);
#else // !WIN32
  int ret = ::close(_sock);
#endif
  _sock = -1;
  return ret;
}

// return:
// -1: error
//  0: no data
// >0: data sent
int basic_socket::send(const char *buf, size_t count)
{
  if(_sock < 0) return -1;
  if(buf == 0 || count == 0) return 0;

  //int ret = ::send(_sock, buf, count, 0);
  int ret = sendto(_sock, buf, count, send_flags, &oaddr, ADDRLEN);

  if(ret < 0)
    {
      if(IS_NET_ERROR(EAGAIN, -1) || IS_NET_ERROR(EWOULDBLOCK, WSAEWOULDBLOCK)) return 0;
      derr << "error: sendto (" << _sock << "): " << NET_ERROR_STR() << endl;
      close();
      return ret;
    }

  return ret;
}

// return:
// -1: error
//  0: no data
// >0: data received
int basic_socket::recv(char *buf, size_t count)
{
  if(_sock < 0) return -1;
  if(buf == 0 || count == 0) return 0;

  //int ret = ::recv(_sock, buf, count, 0);
  socklen_t addrlen = ADDRLEN;
  int ret = recvfrom(_sock, buf, count, recv_flags, &iaddr, &addrlen);

  if(ret == 0)
    {
      derr << "Connection closed by foreign host (" << _sock << ")" << endl;
      close();
      return -1;
    }

  if(ret < 0)
    {
      if(IS_NET_ERROR(EAGAIN, -1) || IS_NET_ERROR(EWOULDBLOCK, WSAEWOULDBLOCK)) return 0;
      derr << "error: recv (" << _sock << "): " << NET_ERROR_STR() << endl;
      close();
      return ret;
    }

  return ret;
}

// same as recv()
int basic_socket::peek(char *buf, size_t count)
{
  recv_flags |= MSG_PEEK;
  int ret = recv(buf, count);
  recv_flags &= (~MSG_PEEK);
  return ret;
}

int basic_socket::send()
{
  return 0;
}

int basic_socket::recv()
{
  return 0;
}

void basic_socket::clear()
{
  _flags = 0;
}

//// selector ////

selector::selector() : ds(-1) { clear(); }

bool selector::read(int s) const { return FD_ISSET(s, &rfds); }
bool selector::write(int s) const { return FD_ISSET(s, &wfds); }

selector& selector::clear() { FD_ZERO(&rfds); FD_ZERO(&wfds); return *this; }

selector& selector::operator<<(int s)
{
  if(s < 0) return *this;
  if(s > ds) ds = s;
#ifndef WIN32
  FD_SET(s, &rfds);
  FD_SET(s, &wfds);
#else
  FD_SET(u_int(s), &rfds);
  FD_SET(u_int(s), &wfds);
#endif
  return *this;
}

// flag: 1 - read, 2 - write, 3 - read+write
int selector::operator()(timeval &tv, int flag)
{
  int ret = ::select((flag & 3) ? ds+1 : 0, (flag & 1) ? &rfds : 0, (flag & 2) ? &wfds : 0, 0, &tv);
  if(ret < 0)
    {
      if(errno == EINTR) return 0;
      perror("select");
      return ret;
    }
  return ret;
}

int selector::operator()(long timeout, int flag)
{
  timeval tv = {timeout / 1000000, timeout % 1000000};
  return operator()(tv, flag);
}

//// utils ///

void nonexec(int fd)
{
#ifndef WIN32
  if(fcntl(fd, F_SETFD, FD_CLOEXEC) == -1)
    {
      perror("fcntl");
      exit(-1);
    }
#endif
}

#ifdef WIN32
void winsock_init()
{
  WSADATA wsadata;
  int ret = WSAStartup(2, &wsadata);
  if(ret)
    {
      perror("WSAStartup");
      exit(-1);
    }
}

void winsock_done()
{
  WSACleanup();
}
#endif // WIN32

#undef NET_ERROR
#undef SET_NET_ERROR

////
