/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// protocol.h -*- C++ -*-
// OWL C++ API v2.0

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define OWL_PROTOCOL_VERSION 2

// define struct packet ahead

namespace OWL {

  //// Header ////

  struct Header {

    uint16_t id;  // bytes 0 and 1
    uint8_t type; // byte 2
    uint8_t cksum; // header only
    uint32_t size; // payload only
    int64_t time;

    Header(uint8_t type=0, uint16_t id=0, uint32_t size=0, int64_t time=0) :
      id(id), type(type), cksum(0), size(size), time(time)
    { set(); }

    void set() { cksum = 0; cksum = -sum(); }

    // 24 bit 'combined' type
    operator uint32_t() const { return (type << 16) | id; }

    bool valid() const { return sum() == 0; }

    uint8_t sum() const
    {
      volatile const uint8_t *p = (const uint8_t*)this;
      return p[0]+p[1]+p[2]+p[3]+p[4]+p[5]+p[6]+p[7];
    }
  };

#ifdef SOCKET_H

  //// recv ///

  inline int recv(basic_socket &s, Header &h, packet &data)
  {
    if(!s) return -1;

    // peek header
    int peek = s.peek((char*)&h, sizeof(Header));
    if(peek < 0) return peek;
    if(peek < (int)sizeof(Header)) return 0;

    if(!h.valid())
      {
        std::cerr << "error: recv(" << s.sock() << "): invalid checksum!" << std::endl;
        s.close();
        return -1;
      }

    if(s.type() == SOCK_DGRAM)
      {
        // recv entire packet
        peek = s.peek();
        if(peek < (int)(sizeof(Header) + h.size)) return 0;

        data.clear();
        data.resize(peek);
        int ret = s.recv(data.data(), data.size());
        if(ret < 0) return ret;
        if(ret != (int)data.size())
          {
            std::cerr << "error: recv(" << s.sock() << "): invalid size!" << std::endl;
            s.close();
            return -1;
          }
        h = *(Header*)data.data();
        data.index = sizeof(Header);
        return 1;
      }

    // peek data
    if(s.peek() < (int)(sizeof(Header) + h.size)) return 0;

    // read header
    int ret = s.recv((char*)&h, sizeof(Header));
    if(ret < 0) return ret;
    if(peek != ret)
      {
        std::cerr << "error: peek() and recv() don't match!" << std::endl;
        s.close();
        return -1;
      }

    data.clear();
    data.resize(h.size);
    ret = s.recv(data.data(), data.size());
    if(ret < 0) return ret;
    if(ret != (int)data.size())
      {
        std::cerr << "error: recv(" << s.sock() << "): invalid size!" << std::endl;
        s.close();
        return -1;
      }
    return 1;
  }

  inline int recv(const char *in, size_t len, Header &h, packet &data)
  {
    if(!in || !len) return -1;

    if(len < (int)sizeof(Header)) return -1;

    const Header *p = (const Header*)in;
    if(!p->valid())
      {
        std::cerr << "error: recv(): invalid checksum!" << std::endl;
        return -1;
      }

    if(len - sizeof(Header) != p->size)
      {
        std::cerr << "error: recv(): invalid size!" << std::endl;
        return -1;
      }

    h = *p;
    data.assign(in+sizeof(Header), in+len);

    return 1;
  }

#endif // SOCKET_H

} // namespace OWL

////

#endif // PROTOCOL_H
