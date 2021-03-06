/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// owl_rpd.cc
// OWL C++ API v2.0

#include <iostream>
#include <vector>

#include <stdio.h>
#include <fcntl.h>
#ifndef _MSC_VER
#include <string.h>
#include <unistd.h>
#else
#include <string>
#include <io.h>
#include <stdlib.h>
#endif
#include <errno.h>

#include "socket.h"
#include "owl_rpd.hpp"

using namespace std;
using namespace OWL;

//// globals ////

OWLAPI size_t _tcp_sndbuf_size = 0x400000;
OWLAPI size_t _tcp_rcvbuf_size = 0x400000;
OWLAPI size_t _rpd_chunk_size = 0x10000;
OWLAPI size_t _tcp_connect_timeout = 5000000;

//// RPD ////

RPD::RPD() : fd(-1), sock(-1), mode(0),  _write(0), _read(0), _send(0), _recv(0)
{ WINSOCK_INIT(); }

RPD::~RPD()
{
  WINSOCK_DONE();
  close();
}

int RPD::open(const char *servername, const char *filename, int mode)
{
  close();

#ifdef WIN32
  int flags = O_BINARY;
#else
  int flags = O_LARGEFILE;
#endif

  if(mode == RPD::SAVE)
    fd = ::open(filename, O_WRONLY|O_CREAT|O_TRUNC|flags, 0644);
  else if(mode == RPD::LOAD)
    fd = ::open(filename, O_RDONLY|flags);
  else
    {
      cerr << "error: RPD::open: invalid mode " << mode << endl;
      return -1;
    }

  if(fd < 0)
    {
      cerr << "error: RPD::open: could not open file " << filename << ": " << strerror(errno) << endl;
      return -2;
    }

  this->mode = mode;

  nonexec(fd);

  // separate server name into name:port
  char name[1024] = "localhost";
  int port = 0;
  if(servername) sscanf(servername, "%[^:]:%d", name, &port);

  tcp_socket sock;
  this->sock = sock.connect(name, 9000+port, 4*1000000);
  if(this->sock < 0)
    {
      cerr << "error: RPD::open: could not connect to " << servername << endl;
      return -3;
    }

  sock.nonblock();
  sock.recv_bufsize(_tcp_rcvbuf_size);

  std::string address = sock.getoaddr()?sock.getoaddr():"";
  if(!address.empty()) cout << "RPD: connect: " << address << " (" << sock.sock() << ")" << endl;

  int ret = sock.send((char*)&mode, sizeof(mode));
  if(ret != sizeof(mode))
    {
      cerr << "error: RPD::open: send mode failed" << endl;
      sock.close(); this->sock = sock.sock();
      return -4;
    }

  if(mode == RPD::LOAD)
    {
      cout << "RPD: sending header" << endl;
      // retry send
      for(int i = 0, count = 0; count < 0x100000 && i < 1000; i++)
        {
          ret = send(1000);
          if(ret < 0) break;
          if(ret > 0) count += ret;
        }
      if(ret < 0) { sock.close(); this->sock = sock.sock(); return ret; }
    }

  cout << "RPD: opened: mode=" << (mode==RPD::SAVE?"save":"load") << endl;

  return 1;
}

bool RPD::close()
{
  if(fd > -1) ::close(fd);
  fd = -1;
  mode = 0;

  tcp_socket sock(this->sock);
  if(sock) cout << "RPD: close (" << sock.sock() << ")" << endl;
  sock.close(); this->sock = sock.sock();

  if(_write || _read || _send || _recv)
    cout << "RPD: write=" << _write << " read=" << _read << " send=" << _send << " recv=" << _recv << endl;

  _write = _read = _send = _recv = 0;
  return true;
}

bool RPD::flush()
{
  if(sock == -1 || mode != RPD::SAVE) return false;
  while(sock != -1 && mode == RPD::SAVE && recv() > 0);
  return true;
}

bool RPD::done()
{
  tcp_socket sock(this->sock);
  if(!sock || mode != RPD::SAVE) return false;

  // set mode=0
  int mode = 0;
  int ret = sock.send((char*)&mode, sizeof(mode));
  if(ret != sizeof(mode))
    {
      cerr << "error: RPD: done: send failed" << endl;
      this->sock = sock.sock();
      return false;
    }
  return true;
}

int RPD::recv(long timeout)
{
  tcp_socket sock(this->sock);
  if(!sock || mode != RPD::SAVE) return -1;

  int ret = sock.select(timeout, 1);
  if(ret <= 0) { this->sock = sock.sock(); return ret; }

  buffer.resize(_rpd_chunk_size ? _rpd_chunk_size : 0x10000);

  ret = sock.recv(buffer.data(), buffer.size());
  if(ret <= 0) { this->sock = sock.sock(); return ret; }
  _recv += ret;

  int size = ret;

  ret = write(fd, buffer.data(), size);
  if(ret < 0) return ret;

  // write exactly size bytes
  if(ret != size)
    {
      cerr << "error: RPD::write: failed to write " << size << " bytes" << endl;
      return -1;
    }
  _write += ret;

  return ret;
}

int RPD::send(long timeout)
{
  tcp_socket sock(this->sock);
  if(!sock || mode != RPD::LOAD) return -1;

  int ret = sock.select(timeout, 2);
  if(ret <= 0) { this->sock = sock.sock(); return ret; }

  buffer.resize(_rpd_chunk_size ? _rpd_chunk_size : 0x10000);

  // save current file position
  off_t pos = lseek(fd, 0, SEEK_CUR);

  ret = read(fd, buffer.data(), buffer.size());
  if(ret <= 0) return ret ? ret : -1;
  _read += ret;

  int size = ret, sent = 0;

  ret = sock.send(buffer.data(), size-sent);
  if(ret < 0) this->sock = sock.sock();
  if(ret > 0) _send += ret, sent = ret;

  // move file position, if buffer not fully sent
  if(size != sent) lseek(fd, pos+sent, SEEK_SET);

  return ret;
}

#if _MSC_VER > 1600

//// RPD ////

struct OWLRPD : public OWL::RPD {
  OWLRPD() : OWL::RPD() { }
};

#include "owl_rpd.h"

OWLRPD* owlRPDCreate()
{ return new OWLRPD(); }

bool owlRPDRelease(struct OWLRPD **rpd)
{
  if(!rpd) return false;
  delete *rpd;
  *rpd = 0;
  return true;
}

bool owlRPDOpen(struct OWLRPD *rpd, const char *servername, const char *filename, int mode)
{
  if(!rpd) return false;
  bool ret = rpd->open(servername, filename, mode);
  if(!ret) rpd->close();
  return ret;
}

bool owlRPDClose(struct OWLRPD *rpd)
{
  if(!rpd) return false;
  rpd->done();
  rpd->flush();
  return rpd->close();
}

int owlRPDSend(struct OWLRPD *rpd, long timeout)
{
  if(!rpd) return -1;
  int ret = rpd->send(timeout);
  if(ret < 0) rpd->close();
  return ret;
}

int owlRPDRecv(struct OWLRPD *rpd, long timeout)
{
  if(!rpd) return -1;
  int ret = rpd->recv(timeout);
  if(ret < 0) rpd->close();
  return ret;
}

#endif // _MSC_VER > 1600
