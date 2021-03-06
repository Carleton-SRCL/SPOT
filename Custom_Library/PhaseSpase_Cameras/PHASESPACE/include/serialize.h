/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// serialize.h -*- C++ -*-

#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <string>
#include <vector>
#include <stdexcept>

// define struct packet ahead

namespace OWL {

  //// decoder ////

  struct decoder {
    const char *head, *end;
    inline decoder(const char *head, const char *end) : head(head), end(end) { }
    inline size_t size() const { return end - head; }
  };

  template <typename T>
  inline decoder& operator>>(decoder &in, T &t)
  {
    if(sizeof(T) >= in.size()) throw std::out_of_range("decoder");
    t = *((T*)in.head);
    in.head += sizeof(T);
    return in;
  }

  template <typename T>
  inline decoder& operator>>(decoder &in, std::vector<T> &data)
  {
    if(sizeof(uint32_t) >= in.size()) throw std::out_of_range("decoder");
    uint32_t s;
    in >> s;
    if(sizeof(T) * s > in.size()) throw std::out_of_range("decoder");
    if(s > 0)
      {
        data.assign((T*)in.head, (T*)in.head + s);
        in.head += sizeof(T) * s;
      }
    return in;
  }

  template <typename T>
  inline decoder& operator>>(decoder &in, std::string &data)
  {
    if(sizeof(uint32_t) >= in.size()) throw std::out_of_range("decoder");
    uint32_t s;
    in >> s;
    if(sizeof(T) * s > in.size()) throw std::out_of_range("decoder");
    if(s > 0)
      {
        data.assign((T*)in.head, s);
        in.head += sizeof(T) * s;
      }
    return in;
  }

  //// encode / decdoe ////

  template <typename T>
  inline void encode(packet &out, const void *first, const void *last)
  {
    const T *begin = (const T*)first, *end = (const T*)last;
    out << (uint32_t)(end - begin); // write size
    for(const T *i = begin; i != end; i++) out << *i;
  }

#ifdef LIBOWL_H
  template <typename T>
  inline EventPrivate decode(const packet &in, size_t size, EventPrivate &e)
  {
    decoder d(in.data()+in.index, in.data()+in.index+size);
    uint32_t s = 0;
    try {
      d >> s; // read size
      e.create<T>(s);
      for(uint32_t i = 0; i < s; i++) d >> e.begin<T>()[i];
    }
    catch(const std::out_of_range &oor) {
      e.destroy();
      return EventPrivate();
    }
    return e;
  }
#endif // LIBOWL_H

  //// serialization ////

  inline packet& operator<<(packet &out, const Input &i)
  { return out << i.hw_id << i.flags << i.time << (uint32_t)i.data.size() << i.data; }

  inline decoder& operator>>(decoder &in, Input &i)
  { return in >> i.hw_id >> i.flags >> i.time >> i.data; }

  //// OWL encode / decode ////

  inline void encode(packet &out, uint16_t type, const void *first, const void *last)
  {
    switch(type)
      {
      case Type::INPUT: return encode<Input>(out, first, last);
      }
    if(first != last) out.insert(out.end(), (const char*)first, (const char*)last);
  }

#ifdef LIBOWL_H
  inline EventPrivate decode(const packet &in, size_t size, uint16_t type, uint16_t id, uint32_t flags, int64_t time)
  {
    EventPrivate e(type, id, flags, time, (void*)0, (void*)0);
    switch(e.type_id())
      {
      case Type::INPUT: return decode<Input>(in, size, e);
      }
    e.create((const void*)(in.data()+in.index), (const void*)(in.data()+in.index+size));
    return e;
  }

  inline EventPrivate decode(const packet &in, size_t size, uint16_t type, const char *name, uint32_t flags, int64_t time)
  {
    EventPrivate e(type, name, flags, time, (void*)0, (void*)0);
    switch(e.type_id())
      {
      case Type::INPUT: return decode<Input>(in, size, e);
      }
    e.create((const void*)(in.data()+in.index), (const void*)(in.data()+in.index+size));
    return e;
  }
#endif // LIBOWL_H

} // namespace OWL

#endif // SERIALIZE_H
