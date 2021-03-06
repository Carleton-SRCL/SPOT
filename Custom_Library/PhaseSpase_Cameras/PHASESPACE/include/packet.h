/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// packet.h -*- C++ -*-

#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <vector>
#include <list>

namespace OWL {

  //// packet ////

  struct packet : public std::vector<char> {
    size_t index;
    packet() : index(0) { }
  };

  template <typename T>
  packet& operator<<(packet &out, const T &t)
  { out.insert(out.end(), (const char*)(&t), (const char*)(&t+1)); return out; }

  template <typename T>
  packet& operator<<(packet &out, const std::vector<T> &data)
  { out.insert(out.end(), (const char*)data.data(), (const char*)(data.data()+data.size())); return out; }

  inline packet& operator<<(packet &out, const std::string &data)
  { out.insert(out.end(), data.begin(), data.end()); return out; }

  ////

} // namespace OWL

#endif // PACKET_H
