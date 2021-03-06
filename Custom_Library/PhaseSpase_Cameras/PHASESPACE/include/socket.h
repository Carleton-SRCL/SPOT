/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// socket.h -*- C++ -*-
// generic socket class

#ifndef SOCKET_H
#define SOCKET_H

#ifndef WIN32
#include <sys/socket.h>
#define WINSOCK_INIT()
#define WINSOCK_DONE()
#else // WIN32
#include <winsock2.h>
#define WINSOCK_INIT() winsock_init()
#define WINSOCK_DONE() winsock_done()
#endif

// WARNING!
// basic_socket, tcp_socket and udp_socket do not have destructors, and
// do NOT close sockets automatically

// TCP: AF_INET, SOCK_STREAM
// UDP: AF_INET, SOCK_DGRAM

//// basic_socket ////

class basic_socket {
protected:

  int _domain;
  int _type;
  int _protocol;
  int _sock;
  size_t _flags;

public:

  int send_flags;
  int recv_flags;

  sockaddr saddr;
  sockaddr oaddr;
  sockaddr iaddr;

  int verbose;
  std::ostream *out, *err;

  basic_socket(int domain=-1, int type=-1, int sock=-1, const sockaddr &iaddr=sockaddr());
  basic_socket(const basic_socket &s);

  int domain() const { return _domain; }
  int type() const { return _type; }
  int protocol() const { return _protocol; }
  int sock() const { return _sock; }

  operator bool() const { return _sock > -1; }

  // use with accept()
  void set(const basic_socket &s);

  bool setf(size_t bit, int value=1);
  bool getf(size_t bit) const;

  void nonblock(bool value=true) const;

  int setsockopt(int level, int opt, int value) const;
  int setsockopt(int level, int opt, const char *value, int len) const;
  int getsockopt(int level, int opt, int &value) const;
  int getsockopt(int level, int opt) const;

  int geterror() const;

  int getnetaddr(const char *netaddress, unsigned int *net_addr) const;
  int getbroadcast(unsigned int *net_addr) const;

  int broadcast(int flag) const;

  int send_bufsize(int size) const;
  int recv_bufsize(int size) const;

  int setaddr(sockaddr *addr, const char *netaddress, int port) const;
  const char* getaddr(const sockaddr *addr, int *port=0) const;

  const char* getsaddr(int *port=0);

  int setoaddr(const char *netaddress, int port);
  int setiaddr(const char *netaddress, int port);
  const char* getoaddr(int *port=0) const;
  const char* getiaddr(int *port=0) const;

  int socket();
  int socket(const char *netaddress, unsigned short port);

  int bind(const char *netaddress, unsigned short port);

  int listen(unsigned short port);

  int connect(const char *netaddress, unsigned short port);
  int connect(const char *netaddress, unsigned short port, long timeout);
  int connected(long timeout);

  // flag: 1 - read
  // flag: 2 - write
  // flag: 3 - read+write
  int select(timeval &tv, int flag=0);
  int select(long timeout, int flag=0);

  int peek() const;

  basic_socket accept(long timeout);

  int close();

  template <typename T> int send(const T *buf, size_t count);
  template <typename T> int recv(T *buf, size_t count);

  // virtual interface

  virtual int send(const char *buf, size_t count);
  virtual int recv(char *buf, size_t count);
  virtual int peek(char *buf, size_t count);

  virtual int send();
  virtual int recv();

  virtual void clear();

};

//// tcp_socket ////

class tcp_socket : public basic_socket {
public:

  tcp_socket(int sock=-1) : basic_socket(AF_INET, SOCK_STREAM, sock) { }

};

//// udp_socket ////

class udp_socket : public basic_socket {
public:

  udp_socket(int sock=-1) : basic_socket(AF_INET, SOCK_DGRAM, sock) { }

};

//// selector ////

class selector {
  int ds;
  fd_set rfds, wfds;
public:

  selector();

  bool read(int s) const;
  bool write(int s) const;

  selector& clear();

  selector& operator<<(int s);

  // flag: 1 - read, 2 - write, 3 - read+write
  int operator()(timeval &tv, int flag);
  int operator()(long timeout, int flag);

};

//// basic_socket ////

template <typename T> int basic_socket::send(const T *buf, size_t count)
{ return send((const char*)buf, count*sizeof(T)); }

template <typename T> int basic_socket::recv(T *buf, size_t count)
{ return recv((char*)buf, count*sizeof(T)); }

//// misc utils ////

void nonexec(int fd);

#ifdef WIN32
void winsock_init();
void winsock_done();
#endif // WIN32

#ifdef STREAM
std::ostream &operator<<(std::ostream &out, const sockaddr &addr);
#endif

////

#endif // SOCKET_H
