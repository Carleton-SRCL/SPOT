/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// libowl.h -*- C++ -*-
// OWL C++ API v2.0
// common utilities

#ifndef LIBOWL_H
#define LIBOWL_H

#include <string>
#include <vector>
#include <deque>
#include <list>
#include <map>
#include <sstream>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include <stdint.h>
#include <stdlib.h>

#include "owl.hpp"

#ifdef WIN32
#include "winthread.h"
#undef ERROR
#else
#include "thread.h"
#endif

typedef std::vector<std::string> strings;

struct stringmap : public std::map<std::string,std::string> {
  stringmap(const std::string &str=std::string());
};

struct ostring {
  std::ostringstream out;
  inline ostring() { }
  template <typename T> ostring(const T &t) { out << t; }
  inline operator std::string() const { return out.str(); }
  template <typename T> ostring& operator<<(const T &t) { out << t; return *this; }
};

namespace std {
  template <typename T>
  ostream &operator<<(ostream &out, const vector<T> &v)
  {
    for(typename vector<T>::const_iterator i = v.begin(); i != v.end(); i++)
      out << (i==v.begin()?"":",") << *i;
    return out;
  }

  template <unsigned N>
  ostream& operator<<(ostream& out, const float (&pose)[N])
  { for(size_t i = 0; i < N; i++) out << (i==0?"":",") << pose[i]; return out; }

  template <typename T1, typename T2>
  ostream& operator<<(ostream& out, const map<T1,T2> &m)
  {
    for(typename map<T1,T2>::const_iterator i = m.begin(); i != m.end(); i++)
      out << (i==m.begin()?"":" ") << i->first << "=" << i->second;
    return out;
  }
}

#define STRTO_(expr) char *p=0; expr; if(ok&&p)*ok=!*p; return v;

// std::sto?() throw exceptions, wrap strto?() instread
#if defined _MSC_VER && _MSC_VER <= 1600
inline long long strtoll(const char *s, char **p, int base) { return _strtoi64(s, p, base); }
inline unsigned long long strtoull(const char *s, char **p, int base) { return _strtoui64(s, p, base); }
inline long double strtold(const char *s, char **p) { return std::stold(s); }
inline float strtof(const char*s, char **p) { return strtod(s, p); }
#endif

inline int strtoi(const std::string &str, bool *ok=0, int base=10)
{ STRTO_(int v=strtol(str.c_str(), &p, base)); }
inline long strtol(const std::string &str, bool *ok=0, int base=10)
{ STRTO_(long v=strtol(str.c_str(), &p, base)); }
inline unsigned long strtoul(const std::string &str, bool *ok=0, int base=10)
{ STRTO_(unsigned long v=strtoul(str.c_str(), &p, base)); }
inline long long strtoll(const std::string &str, bool *ok=0, int base=10)
{ STRTO_(long long v=strtoll(str.c_str(), &p, base)); }
inline unsigned long long strtoull(const std::string &str, bool *ok=0, int base=10)
{ STRTO_(unsigned long long v=strtoull(str.c_str(), &p, base)); }
inline float strtof(const std::string &str, bool *ok=0)
{ STRTO_(float v=strtof(str.c_str(), &p)); }
inline double strtod(const std::string &str, bool *ok=0)
{ STRTO_(double v=strtod(str.c_str(), &p)); }
inline long double strtold(const std::string &str, bool *ok=0)
{ STRTO_(long double v=strtold(str.c_str(), &p)); }

#if __cplusplus < 201103L
namespace std {
  template <typename T>
  string to_string(const T &t) { ostringstream s; s << t; return s.str(); }
} // namespace std
#endif // __cplusplus

//// default value ////

template <typename T> T get(const std::string &str, const T &def)
{ return str; }
template <> inline bool get<bool>(const std::string &str, const bool &def)
{ return str=="1"?true:false; }
template <> inline int get<int>(const std::string &str, const int &def)
{ char *p=0; int v=strtol(str.c_str(), &p, 10); return *p==0?v:def; }
template <> inline unsigned get<unsigned>(const std::string &str, const unsigned int &def)
{ char *p=0; unsigned v=strtoul(str.c_str(), &p, 10); return *p==0?v:def; }
template <> inline long get<long>(const std::string &str, const long &def)
{ char *p=0; long v=strtol(str.c_str(), &p, 10); return *p==0?v:def; }
template <> inline float get<float>(const std::string &str, const float &def)
{ char *p=0; float v=strtof(str.c_str(), &p); return *p==0?v:def; }

//// return success ////
template <typename T> bool get(const std::string &str, T &v)
{ v=str; return true; }
template <> inline bool get<bool>(const std::string &str, bool &v)
{ return str=="1"?(v=true):(v=false); }
template <> inline bool get<int>(const std::string &str, int &v)
{ char *p=0; v=strtol(str.c_str(), &p, 10); return *p==0; }
template <> inline bool get<unsigned int>(const std::string &str, unsigned int &v)
{ char *p=0; v=strtoul(str.c_str(), &p, 10); return *p==0; }
template <> inline bool get<long>(const std::string &str, long &v)
{ char *p=0; v=strtol(str.c_str(), &p, 10); return *p==0; }
template <> inline bool get<float>(const std::string &str, float &v)
{ char *p=0; v=strtof(str.c_str(), &p); return *p==0; }

template <typename T>
size_t get(const std::string &value, std::vector<T> &v)
{
  v.clear();
  std::istringstream in(value);
  std::string s;
  T t;
  while(std::getline(in, s, ',')) if(!s.empty() && get(s, t)) v.push_back(t);
  return v.size();
}

//// default value ////

template <typename T>
T get(const stringmap &m, const std::string &name, const T &def)
{
  stringmap::const_iterator i = m.find(name);
  return i != m.end() ? get(i->second, def) : def;
}

//// success ////

inline bool get(const stringmap &m, const std::string &name, std::string &v)
{
  stringmap::const_iterator i = m.find(name);
  return i != m.end() ? !(v = i->second).empty() : 0;
}

template <typename T>
bool get(const stringmap &m, const std::string &name, T &v)
{
  stringmap::const_iterator i = m.find(name);
  return i != m.end() ? get(i->second, v) : false;
}

template <typename T>
size_t get(const stringmap &m, const std::string &name, std::vector<T> &v)
{
  stringmap::const_iterator i = m.find(name);
  return i != m.end() ? get(i->second, v) : 0;
}

template <typename T>
bool get(const stringmap &m, const std::string &name, T &v, stringmap &out)
{
  stringmap::const_iterator i = m.find(name);
  if(i == m.end() || !get(i->second, v)) return false;
  out[i->first] = i->second;
  return true;
}

inline const char* strdup(const std::string &str)
{
  if(str.empty()) return 0;
  char *ptr = new char[str.size()+1];
  if(ptr) str.copy(ptr, str.size()), ptr[str.size()] = 0;
  return ptr;
}

inline strings split(const std::string &str, char sep, size_t pos=0, strings s=strings())
{
  for(size_t end = pos; (end = str.find(sep, pos)) != std::string::npos; pos = end+1)
    s.push_back(str.substr(pos, end - pos));
  s.push_back(str.substr(pos));
  return s;
}

inline std::string& replace_all(std::string &str, const std::string &in, const std::string &out, size_t pos=0)
{
  for(;(pos = str.find(in, pos)) != std::string::npos; pos += out.length())
    str.replace(pos, in.length(), out);
  return str;
}

inline stringmap::stringmap(const std::string &str)
{
  strings l = split(str, ' '), s;
  for(strings::iterator i = l.begin(); i != l.end(); i++)
    if((s = split(*i, '=')).size() == 2)
      (*this)[s[0]] = s[1];
}

inline stringmap& operator+=(stringmap &m, const std::string &str)
{ strings s = split(str, '='); if(s.size() == 2) m[s[0]] = s[1]; return m; }

inline stringmap merge(stringmap &m, const std::string &options)
{
  const strings opts = split(options, ' ');
  for(size_t i = 0; i < opts.size(); i++)
    {
      strings o = split(opts[i], '=');
      if(o.size() != 2) continue;
      m[o[0]] = o[1];
    }
  return m;
}

inline stringmap merge(const stringmap &m1, const stringmap &m2)
{
  stringmap m = m1;
  for(stringmap::const_iterator i = m2.begin(); i != m2.end(); i++)
    m[i->first] = i->second;
  return m;
}

template <typename InputIterator>
std::string join(InputIterator first, InputIterator last, const std::string &sep)
{
  std::string out;
  for(InputIterator i = first; i != last; i++)
    out += (out.empty()?"":sep) + *i;
  return out;
}

inline std::string join(const strings &s, const std::string &sep)
{
  std::string out;
  for(strings::const_iterator i = s.begin(); i != s.end(); i++)
    out += (out.empty()?"":sep) + *i;
  return out;
}

inline std::string join(const stringmap &m)
{
  std::string out;
  for(stringmap::const_iterator i = m.begin(); i != m.end(); i++)
    out += (out.empty()?"":" ") + i->first + "=" + i->second;
  return out;
}

inline std::string find(const stringmap &m, const std::string &name)
{
  stringmap::const_iterator i = m.find(name);
  return i != m.end() ? i->second : std::string();
}

// options: name1=value1 [name2=value2 ...] [unused]
// names: name1[,name2 ...]
// out: value1[,value2 ...] [unused]
// return names.size <= out.size
inline bool parse(const std::string &options, const std::string &names, strings &out)
{
  out.clear();
  const strings opts = split(options, ' '), nms = split(names, ' ');
  strings::const_iterator i = opts.begin(), j = nms.begin();
  for(; i != opts.end() && j != nms.end(); i++, j++)
    {
      strings o = split(*i, '=');
      if(o.size() != 2 || o[0] != *j) return false;
      out.push_back(o[1]);
    }
  if(nms.size() != out.size()) return false;
  std::string unused = join(i, opts.end(), std::string(" "));
  if(!unused.empty()) out.push_back(unused);
  return true;
}

inline std::ostream& printb(std::ostream &out, const char *data, size_t size)
{
  out << size << ": " << std::hex;
  for(size_t i = 0; i < size; i++) out << (int)(uint8_t)data[i] << " ";
  return out << std::dec;
}

inline std::ostream& printb(std::ostream &out, const std::vector<char> &data)
{ return printb(out, &data[0], data.size()); }

//// OWL ////

namespace OWL {

  inline bool valid(uint16_t id) { return id != (uint16_t)-1; }
  inline bool valid(uint32_t id) { return id != (uint32_t)-1; }

  inline int64_t rescale(int64_t t, int64_t src, int64_t dst)
  { return (t * dst + src/2) / src; }

  inline int64_t rescale(int64_t t, int src_num, int src_den, int dst_num, int dst_den)
  { int64_t src = dst_num * src_den, dst = src_num * dst_den; return (t * dst + src/2) / src; }

  //// TypeID ////

#define OWL_TYPEID(type, id) template <> inline TypeID<type>::operator uint32_t() const { return id; }

  template <typename T> struct TypeID {
    operator uint32_t() const { return Type::INVALID; }
  };

  OWL_TYPEID(char, Type::BYTE);
  OWL_TYPEID(int, Type::INT);
  OWL_TYPEID(unsigned int, Type::INT);
  OWL_TYPEID(float, Type::FLOAT);
  OWL_TYPEID(Event, Type::EVENT);
  OWL_TYPEID(Camera, Type::CAMERA);
  OWL_TYPEID(Peak, Type::PEAK);
  OWL_TYPEID(Plane, Type::PLANE);
  OWL_TYPEID(Marker, Type::MARKER);
  OWL_TYPEID(Rigid, Type::RIGID);
  OWL_TYPEID(Input, Type::INPUT);
  OWL_TYPEID(MarkerInfo, Type::MARKERINFO);
  OWL_TYPEID(TrackerInfo, Type::TRACKERINFO);
  OWL_TYPEID(FilterInfo, Type::FILTERINFO);
  OWL_TYPEID(DeviceInfo, Type::DEVICEINFO);

  //// create ////

  template <typename T>
  void create_(void **begin, void **end, const T *first, const T *last)
  {
    if(first == last) return;
    T *p = new T[last - first];
    *begin = (void*)p;
    *end = (void*)std::copy(first, last, p);
  }

  template <typename T>
  void create_(void **begin, void **end, void *first, void *last)
  { create_(begin, end, (const T*)first, (const T*)last); }

  template <typename T>
  void create_(void **begin, void **end, const void *first, const void *last)
  { create_(begin, end, (const T*)first, (const T*)last); }

  template <typename T>
  void create_(void **begin, void **end, size_t n)
  {
    if(n == 0) return;
    T *p = new T[n];
    *begin = (void*)p;
    *end = (void*)(p+n);
  }

  template <typename T>
  void destroy_(void **begin, void **end)
  {
    delete[] (T*)*begin;
    *begin = 0;
    *end = 0;
  }

  template <typename T>
  void append_(void **begin, void **end, const T *first, const T *last)
  {
    if(begin == end || first == last) return;
    const T *orig = (const T*)*begin, *orig_end = (const T*)*end;
    size_t n = (orig_end - orig) + (last - first);
    T *p = new T[n];
    *begin = (void*)p;
    *end = (void*)std::copy(first, last, std::copy(orig, orig_end, p));
    delete[] orig;
  }

  // contents of l are assumed valid
  template <typename T>
  void erase_(void **begin, void **end, const std::list<void*> &l)
  {
    if(begin == end || l.empty()) return;
    const T *first = (const T*)*begin, *last = (const T*)*end;
    size_t n = last - first, s = l.size();
    if(n <= s) return;

    T *p = new T[n - s], *p_end = p + (n - s);
    *begin = (void*)p; *end = (void*)p_end;
    // sparse copy
    std::list<void*>::const_iterator e = l.begin();
    for(const T *src = first; p != p_end && src != last; src++)
      if(src == *e) e++;
      else *p++ = *src;

    delete[] first;
  }

  template <typename T>
  void create(uint16_t id, void **begin, void **end, const T *first, const T *last)
  {
    assert(OWL::Type::ID<T>() == id);
    return create_(begin, end, first, last);
  }

  inline void create(uint16_t id, void **begin, void **end, const void *first, const void *last)
  {
    switch(id)
      {
      case Type::INVALID: return;
      case Type::BYTE: return create_<char>(begin, end, first, last);
      case Type::INT: return create_<int>(begin, end, first, last);
      case Type::FLOAT: return create_<float>(begin, end, first, last);
      case Type::ERROR: return create_<char>(begin, end, first, last);
      case Type::EVENT: return create_<Event>(begin, end, first, last);
      case Type::CAMERA: return create_<Camera>(begin, end, first, last);
      case Type::PEAK: return create_<Peak>(begin, end, first, last);
      case Type::PLANE: return create_<Plane>(begin, end, first, last);
      case Type::MARKER: return create_<Marker>(begin, end, first, last);
      case Type::RIGID: return create_<Rigid>(begin, end, first, last);
      case Type::INPUT: return create_<Input>(begin, end, first, last);
      case Type::MARKERINFO: return create_<MarkerInfo>(begin, end, first, last);
      case Type::TRACKERINFO: return create_<TrackerInfo>(begin, end, first, last);
      case Type::FILTERINFO: return create_<FilterInfo>(begin, end, first, last);
      case Type::DEVICEINFO: return create_<DeviceInfo>(begin, end, first, last);
      default: return create_<char>(begin, end, first, last);
      }
  }

  inline void create(uint16_t id, void **begin, void **end, void *first, void *last)
  { return create(id, begin, end, (const void*)first, (const void*)last); }

  inline void destroy(uint16_t id, void **begin, void **end)
  {
    switch(id)
      {
      case Type::INVALID: return;
      case Type::BYTE: return destroy_<char>(begin, end);
      case Type::INT: return destroy_<int>(begin, end);
      case Type::FLOAT: return destroy_<float>(begin, end);
      case Type::ERROR: return destroy_<char>(begin, end);
      case Type::EVENT: return destroy_<Event>(begin, end);
      case Type::CAMERA: return destroy_<Camera>(begin, end);
      case Type::PEAK: return destroy_<Peak>(begin, end);
      case Type::PLANE: return destroy_<Plane>(begin, end);
      case Type::MARKER: return destroy_<Marker>(begin, end);
      case Type::RIGID: return destroy_<Rigid>(begin, end);
      case Type::INPUT: return destroy_<Input>(begin, end);
      case Type::MARKERINFO: return destroy_<MarkerInfo>(begin, end);
      case Type::TRACKERINFO: return destroy_<TrackerInfo>(begin, end);
      case Type::FILTERINFO: return destroy_<FilterInfo>(begin, end);
      case Type::DEVICEINFO: return destroy_<DeviceInfo>(begin, end);
      default: destroy_<char>(begin, end);
      }
  }

  //// table ////

  template <typename T>
  class table : public std::vector<T> {
    typedef std::vector<T> base;
    T none;
  public:

    const T& operator[](size_t n) const { return n < base::size() ? base::operator[](n) : none; }
    T* at(size_t n) { return n < base::size() ? &base::operator[](n) : 0; }
    size_t operator[](const std::string &name) const { return -1; }

    void clear() { base::clear(); }
    T* set(size_t n) { if(n >= base::size()) base::resize(n+1); return &base::operator[](n); }

    bool parse(const std::string &name, const std::string &options) { return false; }
  };

  //// idmap ////

  template <typename T>
  class idmap : public std::map<uint64_t,T> {
    typedef std::map<uint64_t,T> base;
    T none;
  public:

    const T& operator[](uint64_t n) const
    { typename base::const_iterator i = base::find(n); return i != base::end() ? i->second : none; }
    T* at(uint64_t n) { typename base::iterator i = base::find(n); return i != base::end() ? &i->second : 0; }
    size_t operator[](const std::string &name) const { return -1; }

    void clear() { base::clear(); }
    T* set(uint64_t n) { return &base::operator[](n); }

    bool parse(const std::string &name, const std::string &options) { return false; }
  };

  //// TypeInfo ////

  struct TypeInfo {
    uint32_t flags;
    uint32_t mode;
    const char *name;
    inline TypeInfo(uint32_t flags=0, uint32_t mode=0, const char *name=0) : flags(flags), mode(mode), name(name) { }
  };

  //// VariantPrivate ////

  class VariantPrivate : public Variant {
  public:
    VariantPrivate();
    template <typename T> VariantPrivate(uint32_t id, uint32_t flags, const T *first, const T *last, const char *name);
  };

  //// EventPrivate ////

  class EventPrivate : public Event {
  public:
    using Event::time;

    EventPrivate();
    EventPrivate(const Event &e) : Event(e) { }
    template <typename T> EventPrivate(uint16_t type, uint16_t id, uint32_t flags, int64_t time, const T *first, const T *last);
    template <typename T> EventPrivate(uint16_t type, uint16_t id, uint32_t flags, int64_t time, const std::vector<T> &v);
    EventPrivate(uint16_t type, uint16_t id, uint32_t flags, int64_t time, const std::string &s);
    // 'lazy' name
    template <typename T> EventPrivate(uint16_t type, const char *name, uint32_t flags, int64_t time, const T *first, const T *last);
    template <typename T> EventPrivate(uint16_t type, const char *name, uint32_t flags, int64_t time, const std::vector<T> &v);
    EventPrivate(uint16_t type, const char *name, uint32_t flags, int64_t time, const std::string &s);

    template <typename T> void create(size_t n);
    template <typename T> void create(const T *first, const T *last);
    void create(const void *first, const void *last);
    void destroy();
    template <typename T> inline void append(const T *first, const T *last);
    template <typename T> inline void erase(const std::list<void*> &l);

    template <typename T> T* begin() const;
    template <typename T> T* end() const;

    inline void rescale(int src_num, int src_den, int dst_num, int dst_den);
    inline void time(int64_t time);

    inline void swap(EventPrivate &e);
  };

  typedef std::list<EventPrivate> Events;

  template <> inline bool Type::ID<EventPrivate>::operator==(uint32_t id) const { return id == EVENT; }

  //// Properties ////

  class Properties : public std::map<std::string,Variant> {
    Variant none;
  public:

    const table<TypeInfo> &types;

    Properties(const table<TypeInfo> &types);

    inline const Variant& operator()(const std::string &name, bool *ok=0) const;
    template <typename T> size_t get(const std::string &name, T &v) const;

    inline bool set(const std::string &name, const VariantPrivate &v);

    inline bool set(const std::string &name, const std::string &value, uint32_t flags=0);
    template <typename T> bool set(const std::string &name, const T &value, uint32_t flags=0);
    template <typename T> bool set(const std::string &name, const std::vector<T> &v, uint32_t flags=0);
    template <typename T> bool set(const std::string &name, const T *first, const T *last, uint32_t flags=0);

    inline bool autoset(const std::string &name, const std::string &value, uint32_t flags=0);
  };

  //// Device ////

  class Device {
  public:

    int64_t time;

    std::string error;

    Events events;

    inline Device() : time(-1) { }
    virtual ~Device() { }

    virtual int open(const std::string &device, const std::string &device_options) = 0;
    virtual bool close() = 0;
    virtual bool is_open() const = 0;
    virtual int read(long timeout) = 0;
    virtual int write(uint16_t type, uint16_t id, uint32_t flags, const char *data, size_t size) = 0;

    OWLAPI static Device* create();
  };

  //// Frame ////

  struct Frame {

    template <typename T> struct Entry : public T {
      uint16_t type;
      uint16_t id;
      uint32_t flags;
      int64_t time;
      inline Entry(uint16_t type=0, uint16_t id=0, uint32_t flags=0, int64_t time=-1) :
        type(type), id(id), flags(flags), time(time) { }
      inline void set(const Event &e)
      {
        type = e.type_id(); id = e.id(); flags = e.flags(); time = e.time();
        T::assign((typename T::const_pointer)e.begin(), (typename T::const_pointer)e.end());
      }
      inline void append(const Event &e)
      { T::insert(T::end(), (typename T::const_pointer)e.begin(), (typename T::const_pointer)e.end()); }
    };

    int64_t time;

    Entry<Peaks> peaks;
    Entry<Planes> planes;
    Entry<Markers> markers;
    Entry<Rigids> rigids;
    Entry< std::vector<float> > marker_vel;
    Entry< std::vector<float> > rigid_vel;

    Events events; // frame events

    Events miscEvents; // non-frame events

    Frame(int64_t time=-1);

    bool set(const Event &e);
    bool append(const Event &e);
    bool get(uint32_t id, EventPrivate &e);
    bool get(uint32_t id, OWL::Events &events);
  };

  //// Frames ////

  class Frames : public std::deque<Frame> {
  public:

    uint32_t id;
    int64_t capacity;

    Frames(uint32_t id=0, int64_t capacity=0);

    bool push(const EventPrivate &frame);
    bool merge(const EventPrivate &frame);
    bool get(EventPrivate &frame, int64_t p);
    bool pop(EventPrivate *frame=0);
  };

  //// Filter ////

  class Filter {
  public:

    const std::string type;
    stringmap options;

    Filter(const std::string &type=std::string());
    virtual ~Filter();

    virtual bool set(const strings &o);

    virtual void apply(const Frames &in, Frames &out, size_t id, uint32_t period);
  };

  //// FilterGroup ////

  class FilterGroup : public std::list<Filter*> {
  public:

    bool enabled;
    uint32_t period;

    std::string name;

    Frames in, out;

    FilterGroup(uint32_t id=0, const std::string &name=std::string(), uint32_t period=0, bool enabled=false);

    void clear();

    void setPeriod(uint32_t n);

    void push(const EventPrivate &frame);
    bool merge(const EventPrivate &frame);
    bool pop(EventPrivate &frame);

    void apply();

    std::string options() const;
  };

  //// Filters ////

  class Filters : public std::list<FilterGroup*> {
    bool shared;
  public:
    typedef std::list<FilterGroup*> base;

    bool enabled;

    Filters(bool shared=false);
    Filters(const Filters &f);
    ~Filters();

    operator bool() const { return enabled; }

    void clear();

    FilterGroup* find(const std::string &name);

    void push(const EventPrivate &frame);
    bool merge(const EventPrivate &frame);
  };

  //// ContextBase ////

  class ContextBase {
  public:

    std::string name;
    std::string error;

    table<TypeInfo> types;

    Properties properties;
    stringmap options;

    bool enableEventMask;
    table<int> eventMask;

    ContextBase(const std::string &name=std::string("libowl"));
    ~ContextBase();

    template <typename T> T property(const std::string &name, bool *ok=0) const
    { return properties(name, ok); }

    void clear();

    std::string enable(const table<TypeInfo> &names, const std::string &options);

    /// events ///

    bool handle_all(EventPrivate &e);
    void handle_byte(EventPrivate &e);
    void handle_int(EventPrivate &e);
    void handle_float(EventPrivate &e);
    void handle_frame(EventPrivate &e);
    void handle_camera(EventPrivate &e);
    void handle_peak(EventPrivate &e);
    void handle_plane(EventPrivate &e);
    void handle_marker(EventPrivate &e);
    void handle_rigid(EventPrivate &e);
    void handle_input(EventPrivate &e);
  };

  //// ContextData ////

  class ContextData : public ContextBase {
  public:

    Device *dev;

    bool internal;

    table<TypeInfo> names;
    table<MarkerInfo> markers;
    table<TrackerInfo> trackers;
    idmap<DeviceInfo> devices;
    Filters filters;

    // handled
    Events events;
    EventPrivate currentEvent;

    Mutex mutex, deviceMutex;

    ContextData(Device *dev=0, const std::string &name=std::string());
    ~ContextData();

    int open(const std::string &device, const std::string &device_options);
    bool close();
    bool isOpen();

    void clear();

    std::string enable(const std::string &options);

    EventPrivate* peekEvent(long timeout);
    EventPrivate* nextEvent(long timeout);

    /// messages ///

    int recv(long timeout);
    int send(uint16_t type, uint16_t id, uint32_t flags, const char *data, size_t size);

    bool send(const std::string &name);
    bool send(const std::string &name, const std::string &s);
    bool send(const std::string &name, const std::vector<int> &v);
    bool send(const std::string &name, const std::vector<float> &v);

    Filters filter(const std::string &options, bool enable);
    Filters filter(const FilterInfo *first, const FilterInfo *last, bool enable);
    const FilterInfo filterInfo(const std::string &name);

    /// events ///

    int new_event(EventPrivate &e, bool flag=true);

  protected:

    int handle_internal(EventPrivate &e);

    void update_filter_info(int64_t time);
  };

  ////

  //// VariantPrivate ////

  inline VariantPrivate::VariantPrivate() : Variant() { }

  template <typename T>
  VariantPrivate::VariantPrivate(uint32_t id, uint32_t flags, const T *first, const T *last, const char *name) : Variant()
  { _id = id; _flags = flags; create(id, &_data, &_data_end, first, last); _type_name = name; }

  //// EventPrivate ////

  inline EventPrivate::EventPrivate() : Event() { }

  template <typename T>
  EventPrivate::EventPrivate(uint16_t type, uint16_t id, uint32_t flags, int64_t time, const T *first, const T *last) : Event()
  { _id = type | (id << 16); _flags = flags; create(first, last); _time = time; }

  template <typename T>
  EventPrivate::EventPrivate(uint16_t type, uint16_t id, uint32_t flags, int64_t time, const std::vector<T> &v) : Event()
  { _id = type | (id << 16); _flags = flags; create(v.data(), v.data()+v.size()); _time = time; }

  inline EventPrivate::EventPrivate(uint16_t type, uint16_t id, uint32_t flags, int64_t time, const std::string &s) : Event()
  { _id = type | (id << 16); _flags = flags; create_(&_data, &_data_end, s.data(), s.data()+s.size()); _time = time; }

  // 'lazy' name
  template <typename T>
  EventPrivate::EventPrivate(uint16_t type, const char *name, uint32_t flags, int64_t time, const T *first, const T *last) : Event()
  { _id = type | (0xffff << 16); _flags = flags; create(first, last); _time = time; _name = name; }

  template <typename T>
  EventPrivate::EventPrivate(uint16_t type, const char *name, uint32_t flags, int64_t time, const std::vector<T> &v) : Event()
  { _id = type | (0xffff << 16); _flags = flags; create(v.data(), v.data()+v.size()); _time = time; _name = name; }

  inline EventPrivate::EventPrivate(uint16_t type, const char *name, uint32_t flags, int64_t time, const std::string &s) : Event()
  { _id = type | (0xffff << 16); _flags = flags; create_(&_data, &_data_end, s.data(), s.data()+s.size()); _time = time; _name = name; }

  template <typename T> void EventPrivate::create(size_t n)
  { create_<T>(&_data, &_data_end, n); }

  template <typename T> void EventPrivate::create(const T *first, const T *last)
  { OWL::create(type_id(), &_data, &_data_end, first, last); }

  inline void EventPrivate::create(const void *first, const void *last)
  { OWL::create(type_id(), &_data, &_data_end, first, last); }

  inline void EventPrivate::destroy()
  { OWL::destroy(type_id(), &_data, &_data_end); }

  template <typename T> inline void EventPrivate::append(const T *first, const T *last)
  { append_<T>(&_data, &_data_end, first, last); }

  template <typename T> inline void EventPrivate::erase(const std::list<void*> &l)
  { erase_<T>(&_data, &_data_end, l); }

  template <typename T> T* EventPrivate::begin() const { return Type::ID<T>() == type_id() ? (T*)_data: 0; }
  template <typename T> T* EventPrivate::end() const { return Type::ID<T>() == type_id() ? (T*)_data_end: 0; }

  inline void EventPrivate::rescale(int src_num, int src_den, int dst_num, int dst_den)
  { _time = OWL::rescale(_time, src_num, src_den, dst_num, dst_den); }

  inline void EventPrivate::time(int64_t time) { _time = time; }

  inline void EventPrivate::swap(EventPrivate &e)
  {
    std::swap(_id, e._id);
    std::swap(_flags, e._flags);
    std::swap(_type_name, e._type_name);
    std::swap(_data, e._data);
    std::swap(_data_end, e._data_end);
    std::swap(_name, e._name);
    std::swap(_time, e._time);
  }

  //// Properties ////

  inline Properties::Properties(const table<TypeInfo> &types) : types(types) { }

  inline const Variant& Properties::operator()(const std::string &name, bool *ok) const
  {
    const_iterator i = find(name);
    if(ok) *ok = (i != end());
    return i != end() ? i->second : none;
  }

  template <typename T>
  size_t Properties::get(const std::string &name, T &v) const
  {
    const_iterator i = find(name);
    return i != end() ? i->second.get(v) : 0;
  }

  inline bool Properties::set(const std::string &name, const VariantPrivate &v)
  {
    iterator i = find(name);
    if(i == end()) { operator[](name) = v; return true; }
    if(v.flags() > i->second.flags()) return false;
    if(i->second.valid() && i->second.type_id() != v.type_id()) return false;
    i->second = v;
    return true;
  }

  inline bool Properties::set(const std::string &name, const std::string &value, uint32_t flags)
  { return set(name, VariantPrivate(Type::BYTE, flags, value.data(), value.data()+value.size(), types[Type::BYTE].name)); }

  template <typename T> bool Properties::set(const std::string &name, const T &value, uint32_t flags)
  { return set(name, VariantPrivate(TypeID<T>(), flags, &value, &value+1, types[TypeID<T>()].name)); }

  template <typename T> bool Properties::set(const std::string &name, const std::vector<T> &v, uint32_t flags)
  { return set(name, VariantPrivate(TypeID<T>(), flags, v.data(), v.data()+v.size(), types[TypeID<T>()].name)); }

  template <typename T> bool Properties::set(const std::string &name, const T *first, const T *last, uint32_t flags)
  { return set(name, VariantPrivate(TypeID<T>(), flags, first, last, types[TypeID<T>()].name)); }

  inline bool Properties::autoset(const std::string &name, const std::string &value, uint32_t flags)
  {
    if(value.empty()) return false;
    std::vector<float> f; ::get(value, f);
    std::vector<int> n; ::get(value, n);

    uint16_t type_id = (*this)(name).type_id();

    if(type_id == Type::BYTE || (f.empty() && n.empty())) return set(name, value, flags);
    if(type_id == Type::FLOAT || f.size() > n.size()) return set(name, f, flags);
    if(type_id == Type::INT || !type_id) return set(name, n, flags);

    std::cout << "warning: " << name << ": failed to set property: " << value << std::endl;
    return false;
  }

  //// TypeInfo ////

  template <> inline size_t table<TypeInfo>::operator[](const std::string &name) const
  { for(size_t i = 0; i < base::size(); i++) if(name == base::operator[](i).name) return i; return -1; }

  template <> inline void table<TypeInfo>::clear() // delete names
  { for(table<TypeInfo>::iterator i = begin(); i != end(); i++) delete[] i->name; base::clear(); }

  // table=name num=name[,flags,mode]
  template <> inline bool table<TypeInfo>::parse(const std::string &name, const std::string &options)
  {
    strings opts(split(options, ' '));
    if(opts.empty() || opts[0] != std::string("table=") + name) return false;

    for(size_t i = 1; i < opts.size(); i++)
      {
        strings o = split(opts[i], '=');
        if(o.size() != 2) continue;
        uint32_t n = strtoi(o[0]);
        if(n > 65535) continue;

        strings v = split(o[1], ',');
        if(v.empty()) continue;

        TypeInfo *t = at(n);
        if(t && t->name)
          {
            // do not overwrite name
            if(v.size() > 1) t->flags = strtoi(v[1]);
            if(v.size() > 2) t->mode = strtoi(v[2]);
            continue;
          }

        t = set(n);
        *t = TypeInfo(v.size() > 1 ? strtoi(v[1]) : 0, v.size() > 2 ? strtoi(v[2]) : 0, strdup(v[0]));
      }
    return true;
  }

  //// MarkerInfo ////

  // table=markers id=id,tid,name [options]
  template <> inline bool table<MarkerInfo>::parse(const std::string &name, const std::string &options)
  {
    strings opts(split(options, ' '));
    if(opts.empty() || opts[0] != std::string("table=") + name) return false;

    MarkerInfo *d = 0;
    for(size_t i = 1; i < opts.size(); i++)
      {
        strings o = split(opts[i], '=');
        if(o.size() != 2) continue;
        if(o[0] == "id")
          {
            strings v = split(o[1], ',');
            if(v.size() > 1)
              {
                d = 0;
                uint32_t id = strtoi(v[0]);
                if(id > 65535) continue;
                if(v.size() == 3) // id,tid,name
                  {
                    d = set(id);
                    d->id = id;
                    if(!v[1].empty()) d->tracker_id = strtoi(v[1]);
                    if(!v[2].empty()) d->name = v[2];
                    d->options.clear();
                  }
              }
          }
        else if(d)
          {
            d->options += (d->options.empty()?"":" ")+opts[i];
          }
      }
    return true;
  }

  //// TrackerInfo ////

  // table=trackers id=n,id,type,name [options]
  template <> inline bool table<TrackerInfo>::parse(const std::string &name, const std::string &options)
  {
    strings opts(split(options, ' '));
    if(opts.empty() || opts[0] != std::string("table=") + name) return false;

    TrackerInfo *d = 0;
    for(size_t i = 1; i < opts.size(); i++)
      {
        strings o = split(opts[i], '=');
        if(o.size() != 2) continue;
        if(o[0] == "id")
          {
            strings v = split(o[1], ',');
            if(v.size() > 1)
              {
                d = 0;
                uint32_t n = strtoi(v[0]);
                if(n > 65535) continue;
                if(v.size() == 4) // n,id,type,name
                  {
                    d = set(n);
                    d->id = strtoi(v[1]);
                    if(!v[2].empty()) d->type = v[2];
                    if(!v[3].empty()) d->name = v[3];
                    d->options.clear();
                  }
              }
          }
        else if(d)
          {
            d->options += (d->options.empty() ? "":" ")+opts[i];
          }
      }
    return true;
  }

  //// DeviceInfo ////

  // table=devices id=hwid,id,type,name [options]
  // status=devices id=hwid,time [status]
  template <> inline bool idmap<DeviceInfo>::parse(const std::string &name, const std::string &options)
  {
    strings opts(split(options, ' '));
    if(opts.empty()) return false;

    DeviceInfo *d = 0;
    if(opts[0] == std::string("table=") + name)
      {
        for(size_t i = 1; i < opts.size(); i++)
          {
            strings o = split(opts[i], '=');
            if(o.size() != 2) continue;
            if(o[0] == "id")
              {
                strings v = split(o[1], ',');
                if(v.size() > 1)
                  {
                    d = 0;
                    uint64_t hwid = strtoull(v[0], 0, 0);
                    if(v.size() == 4) // hwid,id,type,name
                      {
                        d = set(hwid);
                        d->hw_id = hwid;
                        d->id = strtoi(v[1]);
                        if(!v[2].empty()) d->type = v[2];
                        if(!v[3].empty()) d->name = v[3];
                        d->options.clear();
                      }
                  }
              }
            else if(d)
              {
                d->options += (d->options.empty() ? "":" ")+opts[i];
              }
          }
        return true;
      }
    else if(opts[0] == std::string("status=") + name)
      {
        for(size_t i = 1; i < opts.size(); i++)
          {
            strings o = split(opts[i], '=');
            if(o.size() != 2) continue;
            if(o[0] == "id")
              {
                strings v = split(o[1], ',');
                if(v.size() > 1)
                  {
                    d = 0;
                    uint64_t hwid = strtoull(v[0], 0, 0);
                    if(v.size() == 2) // hwid,time
                      {
                        d = set(hwid);
                        d->time = strtoll(v[1]);
                        d->status.clear();
                      }
                  }
              }
            else if(d)
              {
                d->status += (d->status.empty() ? "":" ")+opts[i];
              }
          }
        return true;
      }
    return false;
  }

  ////

#undef STRTO_

} // namespace OWL

#endif // LIBOWL_H
