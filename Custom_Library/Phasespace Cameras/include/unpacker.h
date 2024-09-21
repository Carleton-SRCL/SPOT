// unpacker.h -*- C++ -*-

#ifndef UNPACKER_H
#define UNPACKER_H

// usage:
// if(unpacker.unpack(p, h)) events.push_back(EventPrivate(Type::FRAME, h.id, 0, h.time, unpacker.events));

namespace OWL {

  //// PackCache ////

  struct PackCache {
    uint16_t type_id;
    uint16_t id;
    uint16_t size;
    uint16_t flags;

    uint16_t period;
    uint16_t phase;

    std::string name;
    std::vector<uint64_t> ids;

    PackCache(uint16_t type_id=-1) : type_id(type_id), id(-1), size(0), flags(0), period(0), phase(-1) { }
    PackCache(const PackInfo &p) : type_id(p.type_id), id(p.id), size(0), flags(0), period(0), phase(-1), name(p.name), ids(p.ids)
    {
      stringmap m(p.options);
      std::vector<int> n;

      if(get(m, "size", n)) size = n[0];
      if(get(m, "flags", n)) flags = n[0];
      if(get(m, "period", n)) period = n[0];
      if(get(m, "phase", n)) phase = n[0];
    }
  };

  std::ostream& operator<<(std::ostream &out, const PackInfo &p)
  {
    return out << "type=" << p.type_id << " id=" << (p.id>>8) << ":" << (p.id&0xff)
               << " " << p.name << " " << p.options << " ids=" << p.ids;
  }

  std::ostream& operator<<(std::ostream &out, const PackCache &p)
  {
    return out << "type=" << p.type_id << " id=" << (p.id>>8) << ":" << (p.id&0xff) << " size=" << p.size
               << " " << p.name << " ids=" << p.ids;
  }

  //// Unpacker ////

  struct Unpacker : public std::vector<PackCache> {

    typedef std::vector<EventPrivate> Events;

    int enable;

    std::vector<uint16_t> ids;

    Events events;

    Unpacker() : enable(0) { }

    Unpacker& operator=(const PackInfoTable &packs)
    {
      clear();
      ids.clear();
      for(PackInfoTable::const_iterator p = packs.begin(); p != packs.end(); p++)
        {
          push_back(*p);
          if(std::find(ids.begin(), ids.end(), p->id>>8) == ids.end()) ids.push_back(p->id>>8);
        }
      return *this;
    }

    Events::iterator find_event(int16_t type_id, int16_t id, int64_t time)
    {
      for(Events::iterator e = events.begin(); e != events.end(); e++)
        if(e->type_id() == type_id && e->id() == id && e->time() == time) return e;
      return events.end();
    }

    size_t unpack(const PackCache &p, const char *data, const char *end, uint64_t id, int64_t time, Marker &m)
    {
      m.id = (uint32_t)id;
      m.time = time;
      m.x = *(const float*)(data+0);
      m.y = *(const float*)(data+4);
      m.z = *(const float*)(data+8);
      m.cond = *(const int16_t*)(data+12);
      m.flags = *(const uint16_t*)(data+14);
      return p.size;
    }

    size_t unpack(const PackCache &p, const char *data, const char *end, uint64_t id, int64_t time, Rigid &r)
    {
      r.id = (uint32_t)id;
      r.time = time;
      for(int i = 0; i < 7; i++) r.pose[i] = *(const float*)(data+i*4);
      r.cond = *(const int16_t*)(data+28);
      r.flags = *(const uint16_t*)(data+30);
      return p.size;
    }

    // Inputs can have variable size payload (size=0)
    size_t unpack(const PackCache &p, const char *data, const char *end, uint64_t id, int64_t time, Input &i)
    {
      i.hw_id = id;
      i.time = time;
      i.flags = 0;
      size_t size = p.size;
      if(p.flags) memcpy(&i.flags, data, size);
      else
        {
          if(size > 0) i.data.assign(data, data+size);
          else // p->size == 0
            {
              size = 2 + *(const uint16_t*)data;
              i.data.assign(data+2, std::min(data+size, end));
            }
        }
      return size;
    }

    template <typename T>
    inline size_t unpack(EventPrivate &e, const PackCache &p, const char *data, const char *end, int64_t time);

    inline bool unpack(const packet &in, const Header &h);

  };

  //// Unpacker ////

  // Marker, Rigid, Input
  template <typename T>
  size_t Unpacker::unpack(EventPrivate &e, const PackCache &p, const char *data, const char *end, int64_t time)
  {
    size_t size = 0;
    std::vector<T> v(p.ids.size());
    for(size_t i = 0; i < p.ids.size(); i++)
      size += unpack(p, data+size, end, p.ids[i], time, v[i]);
    e = EventPrivate(p.type_id, (p.id&0xFF), 0, time, v);
    return size;
  }

  template <>
  inline size_t Unpacker::unpack<float>(EventPrivate &e, const PackCache &p, const char *data, const char *end, int64_t time)
  {
    size_t size = p.size*p.ids.size();
    e = EventPrivate(p.type_id, (p.id&0xFF), 0, time, (const float*)data, (const float*)(data+size));
    return size;
  }

  bool Unpacker::unpack(const packet &in, const Header &h)
  {
    if(std::find(ids.begin(), ids.end(), h.id) == ids.end()) return false;

    const char *data = in.data()+in.index;
    size_t index = 0;

    events.clear();
    events.resize(this->size());

    Events::iterator e = events.begin();
    for(const_iterator p = begin(); p != end(); p++)
      {
        if(h.id != (p->id >> 8)) continue;

        if(p->period > 0 && (h.time % p->period) != p->phase) continue;

        if(h.size < index + p->size*p->ids.size())
          {
            std::cout << "warning: unpack: invalid size: " << index << "," << p->size << "," << p->ids.size() << "," << h.size << std::endl;
            return false;
          }

        switch(p->type_id)
          {
          case Type::FLOAT:
            index += unpack<float>(*e, *p, data+index, data+h.size, h.time);
            break;
          case Type::MARKER:
            index += unpack<Marker>(*e, *p, data+index, data+h.size, h.time);
            break;
          case Type::RIGID:
            index += unpack<Rigid>(*e, *p, data+index, data+h.size, h.time);
            break;
          case Type::INPUT:
            index += unpack<Input>(*e, *p, data+index, data+h.size, h.time);
            break;
          default:
            std::cout << "warning: unpack: invalid type_id:" << p->type_id << std::endl;
            return false;
          }
        e++;
      }
    while(!events.empty() && events.back().id() == 0) events.pop_back();
    return events.size();
  }

  ////

} // namespace OWL

#endif // UNPACKER_H
