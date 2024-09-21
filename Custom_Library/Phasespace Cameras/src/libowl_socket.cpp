// libowl_socket.cc -*- C++ -*-
// OWL C++ API v2.2
// owl socket device

/***
Copyright (c) PhaseSpace, Inc 2017

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

#include <iostream>
#include <algorithm>

#ifndef WIN32
#include <unistd.h>
#else // WIN32
#include <winsock2.h>
#endif // WIN32

#include "socket.h"
#include "libowl.h"
#include "packet.h"
#include "protocol.h"
#include "serialize.h"
#include "unpacker.h"

using namespace std;

inline std::ostream& operator<<(std::ostream &out, const OWL::Header &h)
{ return out << std::hex << (int)h.type << " " << (int)h.id << " " << (int)h.cksum  << std::dec << " s=" << h.size << " t=" << h.time; }

namespace OWL {

  enum { CONNECTED = 0x1 };
  enum { INTERNAL = 0 };

  bool equal(const std::string &s, const packet &p)
  { return p.size()-p.index > s.size() && std::equal(s.begin(), s.end(), p.begin()+p.index); }

  //// SocketDevice ////

  class SocketDevice : public Device {

    struct Frame : std::vector<Event> {
      uint16_t id;
      int64_t time;

      Frame(uint16_t id=0, int64_t time=-1) : std::vector<Event>(), id(id), time(time) { }
    };

    int port;

    selector select;
    tcp_socket sock; // blocking
    udp_socket udp;
    udp_socket broadcast;

    table<TypeInfo> names;
    // ID cache
    uint16_t INITIALIZE, DONE, STREAMING, OPTIONS;

    Frame newFrame;

    ostringstream err;

    std::vector<char> buffer;

    Unpacker unpacker;

  public:

    SocketDevice() : port(0), INITIALIZE(-1), DONE(-1), STREAMING(-1), OPTIONS(-1)
    {
      sock.verbose = 1;
      sock.out = 0;
      sock.err = &err;
      udp.verbose = 1;
      udp.out = 0;
      udp.err = &err;
      broadcast.verbose = 1;
      broadcast.out = 0;
      broadcast.err = &err;
      WINSOCK_INIT();
    }

    ~SocketDevice()
    {
      close();
      WINSOCK_DONE();
    }

    int open(const std::string &device, const std::string &device_options)
    {
      if(sock && sock.getf(CONNECTED)) return 1;

      strings a = split(device, ':');
      stringmap o(device_options);

      const char *addr = a.size() > 0 && !a[0].empty() ? a[0].c_str() : "localhost";
      port = a.size() > 1 ? strtoi(a[1]) : 0;
      long t = get(o, "timeout", long(3000000));

      if(!sock)
        {
          int ret = sock.connect(addr, 8000+port, t);
          update_error();
          if(ret <= 0) return ret;
        }
      else if(!sock.getf(CONNECTED))
        {
          int ret = sock.connected(t);
          update_error();
          if(ret <= 0) return ret;
        }

      { // use tcp socket's port# as udp port#
        int port = 0;
        if(const char *address = sock.getsaddr(&port)) udp.bind(address, port);
      }

      sock.recv_bufsize(16*1024*1024);
      sock.send_bufsize(16*1024*1024);

      sock.nonblock(false);
      sock.setf(CONNECTED);
      update_error();

      time = -1;

      return 1;
    }

    bool close()
    {
      if(!sock) return false;
      events.clear();

      sock.close();
      udp.close();
      broadcast.close();

      sock.clear();
      udp.clear();
      broadcast.clear();

      names.clear();

      INITIALIZE = -1;
      DONE = -1;
      STREAMING = -1;
      OPTIONS = -1;

      unpacker = Unpacker();

      update_error();

      return true;
    }

    bool is_open() const
    { return sock ? sock.getf(CONNECTED) : false; }

    int read(int64_t timeout)
    {
      select.clear() << sock.sock() << udp.sock() << broadcast.sock();
      int ret = select(events.empty() ? timeout : 0, 1);
      if(ret <= 0) return ret;

      ret = 0;
      while(sock && sock.select(0, 1) > 0 && recv(sock) > 0) ret++;
      while(udp && udp.select(0, 1) > 0 && recv(udp) > 0) ret++;
      while(broadcast && broadcast.select(0, 1) > 0 && recv(broadcast) > 0) ret++;

      return ret ? ret : (sock ? 0 : -1);
    }

    int recv(basic_socket &s)
    {
      Header h;
      packet p;

      int ret = OWL::recv(s, h, p);
      update_error();
      if(ret <= 0) return ret;

      int count = 0;
      // multi-message packet
      while(p.index <= p.size())
        {
          ret = recv(h, p);
          if(ret <= 0) return ret;
          count++;
          p.index += h.size;

          if(p.size() < p.index + sizeof(Header)) break;
          h = *(Header*)(p.data()+p.index);
          p.index += sizeof(Header);
        }
      return count;
    }

    int recv(Header &h, packet &p)
    {
      if(!valid(h.id)) return 0;

      if(time < h.time) time = h.time;

      if(h.type == Type::BYTE && h.id == INTERNAL)
        {
          if(equal("table=names ", p) && names.parse("names", string(p.data()+p.index, h.size)))
            {
              INITIALIZE = names["initialize"];
              DONE = names["done"];
              STREAMING = names["streaming"];
              OPTIONS = names["options"];
            }
          else if(equal("table=pack ", p))
            {
              PackInfoTable packs;
              if(pack_parse(packs, "pack", string(p.data()+p.index, h.size)))
                unpacker = packs;
            }
        }
      else if(h.type == Type::BYTE && (h.id == INITIALIZE || h.id == DONE || h.id == OPTIONS))
        {
          stringmap m(string(p.data()+p.index, h.size));
          vector<int> n;
          if(get(m, "streaming", n)) toggle_broadcast(n[0]);
          if(get(m, "unpack", n)) unpacker.enable = n[0];
        }
      else if(h.type == Type::INT && h.id == STREAMING && h.size == 4)
        {
          const int *v = (const int*)(p.data()+p.index);
          toggle_broadcast(*v);
        }

      // frame events have frame id and event ids for transmission: (frame.id << 8) | event.id
      uint16_t frame_id = (h.id >> 8);
      if(frame_id)
        {
          // accumulate frame events
          if(frame_id != newFrame.id || h.time != newFrame.time) newFrame = Frame(frame_id, h.time);
          newFrame.push_back(decode(p, h.size, h.type, (h.id & 0xFF), 0, h.time));
          if(!newFrame.back().valid()) { newFrame.pop_back(); return 0; }
        }
      else if(h.type == Type::FRAME && (p.size() <= p.index) && h.id == newFrame.id && h.time == newFrame.time)
        {
          // copy frame events into frame
          events.push_back(EventPrivate(h.type, h.id, 0, h.time, newFrame));
          newFrame = Frame();
        }
      else
        {
          events.push_back(decode(p, h.size, h.type, h.id, 0, h.time));
          if(!events.back().valid()) { events.pop_back(); return 0; }
        }

      if(h.type == Type::BYTE && h.id > STREAMING && unpacker.enable > 0 && unpacker.unpack(p, h)) // last
        events.push_back(EventPrivate(Type::FRAME, h.id, 0, h.time, unpacker.events));

      return 1;
    }

    int write(uint16_t type, uint16_t id, uint32_t flags, const char *data, size_t size)
    {
      if(!sock || !valid(id)) return -1;

      Header h(uint8_t(type), id, size);

      buffer.clear();
      buffer.insert(buffer.end(), (const char*)&h, (const char*)(&h+1));
      if(size) buffer.insert(buffer.end(), data, data+size);

      int ret = sock.send(buffer.data(), buffer.size());
      update_error();
      if(ret != (int)buffer.size()) return -1;

      return ret;
    }

    void update_error()
    {
      error = err.str();
      err.str("");
      if(!error.empty() && error[error.size()-1] == '\n') error.erase(error.size()-1);
    }

    void toggle_broadcast(int v)
    {
      if(v == 3 && !broadcast)
        {
          broadcast.listen(8500+port);
          broadcast.nonblock();
          broadcast.broadcast(1);
          broadcast.recv_bufsize(16*1024*1024);
          broadcast.send_bufsize(16*1024*1024);
        }
      else if(v != 3 && broadcast) broadcast.close();
      update_error();
    }

    static SocketDevice* create() { return new SocketDevice(); }

  }; // SocketDevice

  Device* Device::create() { return new SocketDevice(); }

  //// Scan ////

  Scan::Scan() : fd(-1)
  {
    WINSOCK_INIT();
  }

  Scan::~Scan()
  {
    udp_socket sock(fd);
    sock.verbose = 0;
    sock.close();
    fd = -1;
    WINSOCK_DONE();
  }

  bool Scan::send(const std::string &message)
  {
    udp_socket sock(fd);
    sock.verbose = 0;
    if(!sock)
      {
        sock.socket();
        if(!sock) return false;
        sock.nonblock();
        sock.broadcast(1);
        fd = sock.sock();
      }
    sock.setoaddr(0, 8998);

    // format message?
    ostring out;
    char name[1024] = "";
    gethostname(name, sizeof(name));
    out << "hostname=" << name;
    out << " protocol=" << int(OWL_PROTOCOL_VERSION);
#ifdef LIBOWL_VERSION
    out << " owlversion=" << LIBOWL_VERSION;
#endif // LIBOWL_VERSION
    if(!message.empty()) out << " " << message;
    string s = out;
    bool ret = sock.send(s.c_str(), s.size()+1) > 0;
    if(!sock) fd = -1;
    return ret;
  }

  std::vector<std::string> Scan::listen(long timeout)
  {
    std::vector<std::string> out;
    udp_socket sock(fd);
    sock.verbose = 0;
    if(!sock) return out;

    int ret = sock.select(timeout, 1);
    if(!sock) fd = -1;
    if(ret <= 0) return out;

    vector<Event> events;
    char buf[9*1024];
    while(1)
      {
        ret = sock.recv(buf, sizeof(buf));
        if(!sock) fd = -1;
        if(ret <= 0) break;
        buf[ret] = 0;

        const char *iaddr = sock.getiaddr();
        if(!iaddr) continue;

        out.push_back(string("ip=") + iaddr + " " + buf);
      }

    return out;
  }
  ////

} // namespace OWL
