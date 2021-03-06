/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// owl.cc -*- C++ -*-
// OWL C++ API v2.0
// OW::Type, OWL::Variant, OWL::Event and OWL::ContextData implementations

// Event queue notes:
// Device::events are unhandled.
// ContextData:: events and currentEvent are handled.
// try to keep at most 1 event in handled event queue
// in order to keep events and internal state in sync.
// Context:: open() and initialize() are special cases,
// and are allowed to hold multiple events in handled queue

#include <iostream>
#include <cstring>
#include <stdexcept>

#include "libowl.h"
#include "owl_math.h"

using namespace std;
using namespace OWL;

//// utils ////

namespace OWL {

  template <typename T>
  inline bool rescale(T *first, T *last, const std::vector<int> &tb_src, const std::vector<int> &tb_dst)
  {
    if(tb_src.size() != 2 || tb_dst.size() != 2) return false;
    for(T *i = first; i != last; i++)
      i->time = rescale(i->time, tb_src[0], tb_src[1], tb_dst[0], tb_dst[1]);
    return first != last;
  }

  inline bool subsample(int64_t t, float systemfrequency, float frequency)
  {
    if(systemfrequency <= 0) return false;
    if(frequency <= 0) return true;
    int r = systemfrequency / frequency;
    if(r <= 0) r = 1;
    return (t % r);
  }

  extern Filter* create_filter(const strings &s);
} // namespace OWL

//// MarkerInfo ////

MarkerInfo::MarkerInfo(uint32_t id, uint32_t tracker_id, const std::string &name, const std::string &options) :
  id(id), tracker_id(tracker_id), name(name), options(options) { }

//// TrackerInfo ////

TrackerInfo::TrackerInfo(uint32_t id, const std::string &type, const std::string &name,
			 const std::string &options, const std::vector<uint32_t> &marker_ids) :
  id(id), type(type), name(name), options(options), marker_ids(marker_ids) { }

TrackerInfo::TrackerInfo(uint32_t id, const std::string &type, const std::string &name,
			 const std::string &options, const std::string &marker_ids) :
  id(id), type(type), name(name), options(options) { get(marker_ids, this->marker_ids); }

//// FilterInfo ////

FilterInfo::FilterInfo(uint32_t period, const std::string &name, const std::string &options) :
  period(period), name(name), options(options) { }

//// DeviceInfo ////

DeviceInfo::DeviceInfo(uint64_t hw_id, uint32_t id) : hw_id(hw_id), id(id), time(0) { }

//// Type ////

Type::Type(uint32_t id, const void *data) : id(id), data(data) { }

//// Variant ////

Variant::Variant() : _id(0), _flags(0), _data(0), _data_end(0), _type_name(0) { }

Variant::Variant(const Variant &v) :
  _id(v._id), _flags(v._flags), _data(0), _data_end(0), _type_name(v._type_name)
{ create(type_id(), &_data, &_data_end, v._data, v._data_end); }

Variant::~Variant()
{ destroy(type_id(), &_data, &_data_end); }

Variant& Variant::operator=(const Variant &v)
{
  _id = v._id;
  _flags = v._flags;
  _type_name = v._type_name;
  _data = 0;
  _data_end = 0;
  create(type_id(), &_data, &_data_end, v._data, v._data_end);
  return *this;
}

uint16_t Variant::type_id() const { return _id & 0xFFFF; }
uint32_t Variant::flags() const { return _flags; }

const char* Variant::type_name() const { return _type_name; }

bool Variant::valid() const { return type_id(); }
bool Variant::empty() const { return _data == _data_end; }

const Type Variant::begin() const { return Type(type_id(), _data); }
const Type Variant::end() const { return Type(type_id(), _data_end); }

std::string Variant::str() const
{
  switch(type_id())
    {
    case Type::INT: return ostring() << vector<int>((const int*)begin(), (const int*)end());
    case Type::FLOAT: return ostring() << vector<float>((const float*)begin(), (const float*)end());
    }
  return std::string((const char*)begin(), (const char*)end());
}

//// Event ////

Event::Event() : Variant(), _name(0), _time(-1) { }

uint16_t Event::type_id() const { return Variant::type_id(); }
uint16_t Event::id() const { return _id >> 16; }
uint32_t Event::flags() const { return _flags; }
int64_t Event::time() const { return _time; }

const char* Event::type_name() const { return _type_name; }
const char* Event::name() const { return _name; }

std::string Event::str() const { return Variant::str(); }

const Event* Event::find(uint16_t type_id, const std::string &name) const
{
  if(type_id == 0 && name.empty()) return 0;
  if(this->type_id() != Type::FRAME) return 0;
  for(const Event *e = begin(); e != end(); e++)
    if(e && (type_id == 0 || type_id == e->type_id()) && (name.empty() || name == e->name()))
      return e;
  return 0;
}

const Event* Event::find(const std::string &name) const
{ return find(0, name); }

//// Frame ////

Frame::Frame(int64_t time) :
  time(time),
  peaks(Type::PEAK, 0, 0, time),
  planes(Type::PLANE, 0, 0, time),
  markers(Type::MARKER, 0, 0, time),
  rigids(Type::RIGID, 0, 0, time),
  marker_vel(Type::FLOAT, 0, 0, time),
  rigid_vel(Type::FLOAT, 0, 0, time)
{
}

bool Frame::set(const Event &e)
{
  switch(e.type_id())
    {
    case Type::PEAK: peaks.set(e); return true;
    case Type::PLANE: planes.set(e); return true;
    case Type::MARKER: markers.set(e); return true;
    case Type::RIGID: rigids.set(e); return true;
    default: events.push_back(e); return true;
    }
  return false;
}

bool Frame::append(const Event &e)
{
  switch(e.type_id())
    {
    case Type::PEAK: peaks.append(e); return true;
    case Type::PLANE: planes.append(e); return true;
    case Type::MARKER: markers.append(e); return true;
    case Type::RIGID: rigids.append(e); return true;
    default: events.insert(events.end(), (const Event*)e.begin(), (const Event*)e.end()); return true;
    }
  return false;
}

#define VALID(d) d.id && !d.empty()
#define EVENT(e) EventPrivate(e.type, e.id, e.flags, e.time, e)

bool Frame::get(uint32_t id, EventPrivate &e)
{
  if(time == -1 || id == 0) return false;
  vector<Event> frame;
  if(VALID(peaks)) frame.push_back(EVENT(peaks));
  if(VALID(planes)) frame.push_back(EVENT(planes));
  if(VALID(markers)) frame.push_back(EVENT(markers));
  if(VALID(rigids)) frame.push_back(EVENT(rigids));
  if(VALID(marker_vel)) frame.push_back(EVENT(marker_vel));
  if(VALID(rigid_vel)) frame.push_back(EVENT(rigid_vel));
  if(!events.empty()) frame.insert(frame.end(), events.begin(), events.end());
  e = EventPrivate(Type::FRAME, id, 0, time, frame);
  return true;
}

bool Frame::get(uint32_t id, OWL::Events &e)
{
  if(time == -1 || id == 0) return false;
  e.push_back(EventPrivate());
  return get(id, e.back());
}

//// Frames ////

Frames::Frames(uint32_t id, int64_t capacity) : id(id), capacity(capacity)
{
}

bool Frames::push(const EventPrivate &frame)
{
  if(capacity <= 0) return false;

  //if(frame.type_id() != Type::FRAME || frame.empty() == 0) return false;
  if(frame.type_id() != Type::FRAME) return false;

  push_back(Frame(frame.time()));

  for(const Event *e = frame.begin<Event>(), *end = frame.end<Event>(); e != end; e++)
    back().set(*e);

  return true;
}

bool Frames::merge(const EventPrivate &frame)
{
  if(capacity <= 0) return false;

  for(iterator f = begin(); f != end(); f++)
    if(f->time == frame.time())
      {
        if(frame.type_id() == Type::FRAME)
          for(const Event *e = frame.begin<Event>(), *end = frame.end<Event>(); e != end; e++)
            f->append(*e);
        else f->append(frame);
        return true;
      }
  return false;
}

bool Frames::get(EventPrivate &frame, int64_t p)
{
  if(empty()) return false;

  for(reverse_iterator f = rbegin(); f != rend(); f++)
    {
      int64_t dt = back().time - f->time;
      if(dt < 0 || dt >= p) return f->get(id, frame);
    }

  return false;
}

bool Frames::pop(EventPrivate *frame)
{
  if(empty()) return false;

  int64_t dt = back().time - front().time;
  if(dt < 0 || dt >= capacity)
    {
      if(frame) front().get(id, *frame);
      pop_front();
      return true;
    }

  return false;
}

//// ContextBase ////

ContextBase::ContextBase(const std::string &name) : name(name), properties(types), enableEventMask(true)
{
}

ContextBase::~ContextBase()
{
}

void ContextBase::clear()
{
  error.clear();
  properties.clear();

  properties.set("opened", int(0));
  properties.set("initialized", int(0));
  properties.set("streaming", int(0));
  properties.set("name", string(""));
  properties.set("profile", string(""));
  properties.set("local", int(0));
  properties.set("systemtimebase", (int*)0, (int*)0);
  properties.set("timebase", (int*)0, (int*)0);
  properties.set("maxfrequency", (float)OWL_MAX_FREQUENCY);
  properties.set("systemfrequency", float(0));
  properties.set("frequency", float(0));
  properties.set("scale", float(1));
  float p[7] = {0, 0, 0, 1, 0, 0, 0};
  properties.set("systempose", p, p+7);
  properties.set("pose", p, p+7);
  properties.set("options", string(""));
  properties.set("systemcameras", (Camera*)0, (Camera*)0);
  properties.set("cameras", (Camera*)0, (Camera*)0);
  properties.set("markers", int(0));
  properties.set("markerinfo", (MarkerInfo*)0, (MarkerInfo*)0);
  properties.set("trackers", (int*)0, (int*)0);
  properties.set("trackerinfo", (TrackerInfo*)0, (TrackerInfo*)0);
  properties.set("filters", string(""));
  properties.set("filterinfo", (FilterInfo*)0, (FilterInfo*)0);
  properties.set("deviceinfo", (DeviceInfo*)0, (DeviceInfo*)0);
  properties.set("profiles", string(""));
  properties.set("defaultprofile", string(""));
  properties.set("profiles.json", string(""));

  options.clear();

  eventMask.clear();
}

// return changed enable options
// used independent of enableEventMask value
std::string ContextBase::enable(const table<TypeInfo> &names, const std::string &options)
{
  string o, ret;
  stringmap m(options);
  for(size_t i = 0; i < names.size(); i++)
    {
      bool changed = false;
      string n = string("event.")+names[i].name;
      if(i >= eventMask.size()) *eventMask.set(i) = !(names[i].flags & 1), changed = true;
      if(names[i].flags & 1)
        {
          if(get(m, n, o)) *eventMask.set(i) = strtoi(o), changed = true;
          this->options[n] = eventMask[i]?"1":"0";
          if(changed) ret += n+"="+(eventMask[i]?"1":"0")+" ";
        }
    }
  //cout << "### " << name << " enable local=" << property<int>("local") << " [" << options << "] " << ret << endl;
  return ret;
}

bool ContextBase::handle_all(EventPrivate &e)
{
  if(enableEventMask && !eventMask.empty() && eventMask[e.id()] == 0) return true;
  if(property<int>("local") == 0 || e.time() == -1) return false;

  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");
  float systemfrequency = properties("systemfrequency"), frequency = properties("frequency");

  bool ret = (e.type_id() == Type::FRAME && subsample(e.time(), systemfrequency, frequency));
  if(tbs.size() == 2 && tbd.size() == 2) e.rescale(tbs[0], tbs[1], tbd[0], tbd[1]);

  return ret;
}

// options name=value [name=value] ...
// name=value[,value...]
void ContextBase::handle_byte(EventPrivate &e)
{
  const string s = e;

  if(strcmp("options", e.name()) == 0)
    {
      //std::cout << "# " << name << " event: char options: " << s << std::endl;
      stringmap m(s);
      for(stringmap::iterator o = m.begin(); o != m.end(); o++)
        options[o->first] = o->second;
      properties.set("options", join(options));
    }
  else if(strcmp("initialize", e.name()) == 0)
    {
      //std::cout << "# " << name << " event: char initialize: " << s << std::endl;
      stringmap m(s);
      for(stringmap::iterator o = m.begin(); o != m.end(); o++)
        properties.autoset(o->first, o->second);
    }
  else if(strcmp("done", e.name()) == 0)
    {
      stringmap m(s);
      for(stringmap::iterator o = m.begin(); o != m.end(); o++)
        properties.autoset(o->first, o->second);
    }
}

void ContextBase::handle_int(EventPrivate &e)
{
  if(strcmp(e.name(), "streaming") == 0)
    {
      vector<int> n;
      if(e.get(n) == 1) properties.set("streaming", n[0]);
    }
  else if(strcmp(e.name(), "timebase") == 0)
    {
      vector<int> n;
      if(e.get(n) == 2) properties.set("timebase", n);
    }
}

void ContextBase::handle_float(EventPrivate &e)
{
  if(strcmp(e.name(), "frequency") == 0)
    {
      vector<float> f;
      if(e.get(f) == 1) properties.set("frequency", f[0]);
    }
  else if(strcmp(e.name(), "scale") == 0)
    {
      vector<float> s;
      if(e.get(s) == 1) properties.set("scale", s[0]);
    }
  else if(strcmp(e.name(), "pose") == 0)
    {
      vector<float> p;
      if(e.get(p) == 7) properties.set("pose", p);
    }
  else if(strcmp(e.name(), "systempose") == 0)
    {
      vector<float> p;
      if(e.get(p) == 7) properties.set("systempose", p);
    }
  else if(strcmp(e.name(), "markervelocities") == 0)
    {
      float pose[7] = {0, 0, 0, 1, 0, 0, 0};
      float t[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
      float scale = property<float>("scale");
      vector<float> systempose(pose, pose+7), localpose(pose, pose+7);
      properties.get("systempose", systempose); properties.get("pose", localpose);

      if(systempose.size() == 7 && localpose.size() == 7)
        {
          owl_mult_qq(&localpose[3], &systempose[3], pose+3); // accumulate
          owl_convert_pm(pose, t); // convert
        }

      if((e.size<float>() % 3) == 0)
        for(float *p = e.begin<float>(); p != e.end<float>(); p+=3)
          {
            float v[3];
            owl_mult_v3s(p, scale, v); // scale
            owl_mult_mv3_v3(t, v, p); // transform
          }
    }
  else if(strcmp(e.name(), "rigidvelocities") == 0)
    {
      float pose[7] = {0, 0, 0, 1, 0, 0, 0};
      float t[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
      float scale = property<float>("scale");
      vector<float> systempose(pose, pose+7), localpose(pose, pose+7);
      properties.get("systempose", systempose); properties.get("pose", localpose);

      if(systempose.size() == 7 && localpose.size() == 7)
        {
          owl_mult_qq(&localpose[3], &systempose[3], pose+3); // accumulate
          owl_convert_pm(pose, t); // convert
        }

      if((e.size<float>() % 6) == 0)
        for(float *p = e.begin<float>(); p != e.end<float>(); p+=6)
          {
            float v[3];
            owl_mult_v3s(p, scale, v); // scale
            owl_mult_mv3_v3(t, v, p); // transform
            // ignore rotational velocity
          }
    }
}

void ContextBase::handle_camera(EventPrivate &e)
{
  Camera *begin = e.begin<Camera>(), *end = e.end<Camera>();
  properties.set("systemcameras", begin, end);

  if(property<int>("local") == 1)
    {
      float pose[7] = {0, 0, 0, 1, 0, 0, 0};
      float scale = property<float>("scale");
      vector<float> systempose(pose, pose+7), localpose(pose, pose+7);
      properties.get("systempose", systempose); properties.get("pose", localpose);

      // pose = local * (system * scale)
      if(systempose.size() == 7 && localpose.size() == 7)
        owl_mult_pps(&localpose[0], &systempose[0], scale, pose); // scale and accumulate

      for(Camera *c = begin; c != end; c++)
        if(c->cond > 0)
          {
            // camera = pose * (camera * scale)
            float p[7];
            copy(c->pose, c->pose+7, p);
            owl_mult_pps(&pose[0], p, scale, c->pose); // scale and transform
          }
    }

  properties.set("cameras", begin, end);
}

void ContextBase::handle_frame(EventPrivate &f)
{
  if(property<int>("local") == 0) return;

  EventPrivate *begin = f.begin<EventPrivate>(), *end = f.end<EventPrivate>();
  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");

  list<void*> erase;
  for(EventPrivate *e = begin; e != end; e++)
    {
      if(!eventMask.empty() && eventMask[e->id()] == 0) { erase.push_back(e); continue; }

      if(e->time() != -1 && tbs.size() == 2 && tbd.size() == 2)
        e->rescale(tbs[0], tbs[1], tbd[0], tbd[1]);

      switch(e->type_id())
        {
        case Type::BYTE: handle_byte(*e); break;
        case Type::INT: handle_int(*e); break;
        case Type::FLOAT: handle_float(*e); break;
        case Type::FRAME: handle_frame(*e); break;
        case Type::PEAK: handle_peak(*e); break;
        case Type::PLANE: handle_plane(*e); break;
        case Type::MARKER: handle_marker(*e); break;
        case Type::RIGID: handle_rigid(*e); break;
        case Type::INPUT: handle_input(*e); break;
        }
    }
  if(!erase.empty()) f.erase<Event>(erase);
}

void ContextBase::handle_peak(EventPrivate &e)
{
  if(property<int>("local") == 0) return;

  Peak *begin = e.begin<Peak>(), *end = e.end<Peak>();
  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");
  rescale(begin, end, tbs, tbd);
}

void ContextBase::handle_plane(EventPrivate &e)
{
  if(property<int>("local") == 0) return;

  Plane *begin = e.begin<Plane>(), *end = e.end<Plane>();
  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");
  rescale(begin, end, tbs, tbd);

  float pose[7] = {0, 0, 0, 1, 0, 0, 0};
  float t[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  float scale = property<float>("scale");
  vector<float> systempose(pose, pose+7), localpose(pose, pose+7);
  properties.get("systempose", systempose); properties.get("pose", localpose);

  if(systempose.size() == 7 && localpose.size() == 7)
    {
      owl_mult_pps(&localpose[0], &systempose[0], scale, pose); // scale and accumulate
      owl_convert_pm(pose, t); // convert
    }

  float pl[4] = {0,0,0,0};
  for(Plane *p = begin; p != end; p++)
    {
      std::copy(p->plane, p->plane+4, pl);
      pl[3] *= scale; // scale
      owl_mult_mpl_pl(t, pl, p->plane); // transform
      p->offset *= scale; // scale
    }
}

void ContextBase::handle_marker(EventPrivate &e)
{
  if(property<int>("local") == 0) return;

  Marker *begin = e.begin<Marker>(), *end = e.end<Marker>();
  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");
  rescale(begin, end, tbs, tbd);

  float pose[7] = {0, 0, 0, 1, 0, 0, 0};
  float t[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
  float scale = property<float>("scale");
  vector<float> systempose(pose, pose+7), localpose(pose, pose+7);
  properties.get("systempose", systempose); properties.get("pose", localpose);

  if(systempose.size() == 7 && localpose.size() == 7)
    {
      owl_mult_pps(&localpose[0], &systempose[0], scale, pose); // scale and accumulate
      owl_convert_pm(pose, t); // convert
    }

  for(Marker *m = begin; m != end; m++)
    if(m->cond > 0)
      {
        float v[3];
        owl_mult_v3s(&m->x, scale, v); // scale
        owl_mult_mv3_v3(t, v, &m->x); // transform
      }
}

void ContextBase::handle_rigid(EventPrivate &e)
{
  if(property<int>("local") == 0) return;

  Rigid *begin = e.begin<Rigid>(), *end = e.end<Rigid>();
  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");
  rescale(begin, end, tbs, tbd);

  float pose[7] = {0, 0, 0, 1, 0, 0, 0};
  float scale = property<float>("scale");
  vector<float> systempose(pose, pose+7), localpose(pose, pose+7);
  properties.get("systempose", systempose); properties.get("pose", localpose);

  if(systempose.size() == 7 && localpose.size() == 7)
    owl_mult_pps(&localpose[0], &systempose[0], scale, pose); // scale and accumulate

  for(Rigid *r = begin; r != end; r++)
    if(r->cond > 0)
      {
        float p[7];
        copy(r->pose, r->pose+7, p);
        owl_mult_pps(pose, p, scale, r->pose); // scale and transform
      }
}

void ContextBase::handle_input(EventPrivate &e)
{
  if(property<int>("local") == 0) return;

  Input *begin = e.begin<Input>(), *end = e.end<Input>();
  vector<int> tbs = properties("systemtimebase"), tbd = properties("timebase");
  rescale(begin, end, tbs, tbd);
}

//// ContextData ////

ContextData::ContextData(Device *dev, const std::string &name) : ContextBase(name), dev(dev), internal(false)
{
  properties.set("maxfrequency", (float)OWL_MAX_FREQUENCY);
}

ContextData::~ContextData()
{
  close();
  MutexLocker l(&deviceMutex);
  delete dev;
  dev = 0;
}

int ContextData::open(const std::string &device, const std::string &device_options)
{
  MutexLocker l(&deviceMutex);
  if(!dev) { error = "fatal: invalid device"; return -1; }
  int ret = dev->open(device, device_options);
  if(ret < 0) error = dev->error;
  return ret;
}

bool ContextData::close()
{
  MutexLocker l(&deviceMutex);
  if(dev) dev->close();

  error.clear();

  properties.clear();
  properties.set("maxfrequency", (float)OWL_MAX_FREQUENCY);

  options.clear();

  eventMask.clear();

  markers.clear();
  trackers.clear();
  devices.clear();
  filters.clear();

  events.clear();
  currentEvent = Event();

  names.clear();
  types.clear();

  return true;
}

bool ContextData::isOpen()
{ MutexLocker l(&deviceMutex); return dev ? dev->is_open() : false; }

void ContextData::clear()
{
  ContextBase::clear();

  MutexLocker l(&deviceMutex);
  if(dev) dev->events.clear();

  events.clear();
  currentEvent = Event();

  markers.clear();
  trackers.clear();
  devices.clear();
  filters.clear();
}

std::string ContextData::enable(const std::string &options)
{
  if(enableEventMask && property<int>("local") == 0) return string();
  string ret = ContextBase::enable(names, options);
  if(!ret.empty() && enableEventMask)
    {
      MutexLocker l(&deviceMutex);
      int64_t time = dev ? dev->time : -1;
      l.unlock();
      EventPrivate e(Type::BYTE, "options", 0, time, ret);
      new_event(e);
    }
  return ret;
}

EventPrivate* ContextData::peekEvent(long timeout)
{
  if(events.empty()) recv(timeout);
  return !events.empty() ? &events.front() : 0;
}

EventPrivate* ContextData::nextEvent(long timeout)
{
  if(events.empty()) recv(timeout);
  if(events.empty()) return 0;

  //currentEvent = events.front();
  currentEvent.swap(events.front());
  events.pop_front();

  return &currentEvent;
}

// messages //

int ContextData::recv(long timeout)
{
  MutexLocker l(&deviceMutex);
  if(!dev) { error = "fatal: invalid device"; return -1; }
  int ret = dev->read(dev->events.empty() ? timeout : 0);
  if(ret < 0) error = dev->error;
  // try to have at least one event in handled events buffer
  do {
    if(!dev->events.empty())
      {
        new_event(dev->events.front());
        dev->events.pop_front();
      }
  } while(!dev->events.empty() && events.empty());
  return ret;
}

int ContextData::send(uint16_t type, uint16_t id, uint32_t flags, const char *data, size_t size)
{
  MutexLocker l(&deviceMutex);
  if(!dev) { error = "fatal: invalid device"; return -1; }
  if(!valid(id)) { error = ostring() << "error: invalid command id: " << id; return -1; }
  uint32_t mode = property<int>("slave");
  if(mode && mode < names[id].mode) { error = "error: permission denied"; return -1; }
  int ret = 0;
  if(ret <= 0)
    {
      ret = dev->write(type, id, flags, data, size);
      if(ret < 0) error = dev->error;
    }
  return ret;
}

bool ContextData::send(const std::string &name, const std::string &s)
{ return send(Type::BYTE, names[name], 0, s.data(), s.size()) > 0; }

bool ContextData::send(const std::string &name, const std::vector<int> &v)
{ return send(Type::INT, names[name], 0, (const char*)v.data(), v.size() * sizeof(int)) > 0; }

bool ContextData::send(const std::string &name, const std::vector<float> &v)
{ return send(Type::FLOAT, names[name], 0, (const char*)v.data(), v.size() * sizeof(float)) > 0; }

// FilterGroup::options(): type=type [options] [type=type] ...
std::string FilterGroup::options() const
{
  ostring out;
  for(const_iterator i = begin(); i != end(); i++)
    if(Filter *f = *i)
      out << (i==begin()?"":" ") << "type=" << f->type << (f->options.empty()?"":" ") << f->options;
  return out;
}

// filter=name period=n type=type [options] [type=type] ... [filter=name] ...
Filters ContextData::filter(const std::string &options, bool enable)
{
  // filters can exist independent of local

  vector<FilterInfo> fi;
  strings opts = split(options, ' ');
  for(size_t i = 0; i < opts.size(); i++)
    {
      strings o = split(opts[i], '=');
      if(o.size() != 2) { error = "filter: invalid option: " + opts[i]; return Filters(); }

      if(o[0] == "filter")
        {
          string name = o[1];
          if(name.empty()) { error = "filter: empty name"; return Filters(); }
          uint32_t id = names[name];
          if(valid(id) && !(names[id].flags & 2))
            { error = "filter: invalid name: " + name; return Filters(); }
          fi.push_back(FilterInfo(0, name));
        }
      else if(fi.empty())
        { error = "filter: no filter"; return Filters(); }
      else if(o[0] == "period")
        {
          int32_t p = strtoll(o[1]);
          if(p < 0) { error = "filter: invalid period: " + o[1]; return Filters(); }
          fi.back().period = p;
        }
      else fi.back().options += (fi.back().options.empty()?"":" ") + opts[i];
    }

  return filter(fi.data(), fi.data()+fi.size(), enable);
}

// FilterInfo::options: type=type [options] [type=type] ...
Filters ContextData::filter(const FilterInfo *first, const FilterInfo *last, bool enable)
{
  if(!first || !last || first == last) return Filters();

  int local = property<int>("local");
  error.clear();
  Filters new_filters(true), changed_filters(true); // shared
  for(const FilterInfo *i = first; i != last; i++)
    {
      uint32_t id = names[i->name];
      if(i->name.empty()) { error = "filter: empty name"; break; }
      if(valid(id) && !(names[id].flags & 2))
        { error = "filter: invalid name: " + i->name; break; }
      if(!valid(id)) send("internal", "name=" + i->name);
      FilterGroup *g = filters.find(i->name);
      if(local == 1 && !enable && g && g->enabled) continue; // keep local filter
      if(!g) g = new_filters.find(i->name);
      if(g) { g->clear(); g->setPeriod(i->period); changed_filters.push_back(g); }
      else new_filters.push_back(g = new FilterGroup(id, i->name, i->period));

      if(i->options.empty()) continue;

      Filter *f = 0;
      strings opts = split(i->options, ' ');
      for(size_t i = 0; i < opts.size(); i++)
        {
          strings o = split(opts[i], '=');
          if(o.size() != 2) { error = "filter: invalid option: " + opts[i]; break; }

          if(o[0] == "type")
            {
              strings s = split(o[1], ',');
              if(s.empty()) { error = "filter: no type"; break; }
              f = create_filter(s);
              if(!f) { error = "filter: invalid type: " + s[0]; break; }
              g->push_back(f);
            }
          else if(!f)
            { error = "filter: no filter"; break; }
          else if(!f->set(o))
            { error = "filter: set options failed: " + opts[i]; break; }
        }
      g->enabled = true;
    }

  if(!error.empty() || (new_filters.empty() && changed_filters.empty())) { new_filters.clear(); return Filters(); }

  new_filters.enabled = true; // success
  for(Filters::iterator i = new_filters.begin(); i != new_filters.end(); i++)
    if(FilterGroup *g = *i)
      filters.push_back(g), changed_filters.push_back(g);

  string changed;
  for(Filters::iterator i = changed_filters.begin(); i != changed_filters.end(); i++)
    if(FilterGroup *g = *i)
      changed += "event."+g->name+"="+(g->enabled?"1":"0");

  filters.enabled = false;
  if(enable)
    for(Filters::iterator i = filters.begin(); i != filters.end(); i++)
      if(FilterGroup *g = *i)
        if(g->enabled) filters.enabled = true;

  ContextData::enable(changed);

  MutexLocker l(&deviceMutex);
  int64_t time = dev ? dev->time : -1;
  l.unlock();
  update_filter_info(time);

  return changed_filters;
}

const FilterInfo ContextData::filterInfo(const std::string &name)
{
  FilterGroup *g = filters.find(name);
  if(!g) return FilterInfo();

  return FilterInfo(g->period, g->name, g->options());
}

/// events ///

int ContextData::new_event(EventPrivate &e, bool flag)
{
  // lazy name event
  if(e.id() == 0xffff && e.name())
    {
      // lookup actual id by lazy name
      uint16_t id = names[e.name()];
      if(!valid(id))
        {
          cerr << "warning: unknown event: type=" << e.type_id() << " name=" << e.name() << endl;
          return 1;
        }
      e._id = (e._id & 0x0000ffff) | (id << 16);
    }

  // unknown event
  if(e.id() != 0 && (e.type_id() >= types.size() || e.id() >= names.size()))
    {
      cerr << "warning: unknown event: type=" << e.type_id() << " id=" << e.id() << endl;
      return 1;
    }

  if(e.id() != 0 && e.time() != -1 && property<int>("initialized") == 0)
    {
      //cout << "# drop event: type=" << e.type_id() << " id=" << e.id() << " time=" << e.time() << endl;
      return 1;
    }

  e._type_name = types[e.type_id()].name;
  e._flags = names[e.id()].flags;
  e._name = names[e.id()].name;

  //cout << "# " << name << " new event: t=" << e.time() << " " << (e.type_name()?e.type_name():"") << " " << (e.name()?e.name():"") << endl;

  if(e.type_id() == Type::FRAME)
    for(Event *i = e.begin<Event>(); i != e.end<Event>(); i++)
      if(i->type_id() && i->type_id() < types.size() && i->id())
        {
          if(i->id() == 0xffff && i->name()) // lazy name event
            {
              uint16_t id = names[i->name()];
              if(valid(id)) i->_id = (i->_id & 0x0000ffff) | (id << 16);
              else cerr << "warning: unknown event: type=" << e.type_id() << " name=" << e.name() << endl;
            }
          if(i->id() < names.size())
            {
              i->_type_name = types[i->type_id()].name;
              i->_flags = names[i->id()].flags;
              i->_name = names[i->id()].name;
            }
          else cerr << "warning: unknown event: type=" << e.type_id() << " id=" << e.id() << endl;
        }

  // handle internal event
  if(e.id() == 0)
    {
      return handle_internal(e);
    }

  // filter is applied only to "raw" frames
  bool filter = (flag && filters.enabled && e.type_id() == Type::FRAME && strcmp("raw", e.name()) == 0);

  // filter push and apply
  if(filter) filters.push(e);

  // merge event into filters
  if(flag && filters.enabled && (e.flags() & 0x80)) filters.merge(e);

  int ret = e.valid();
  if(!handle_all(e))
    {
      switch(e.type_id())
        {
        case Type::BYTE: handle_byte(e); break;
        case Type::INT: handle_int(e); break;
        case Type::FLOAT: handle_float(e); break;
        case Type::FRAME: handle_frame(e); break;
        case Type::CAMERA: handle_camera(e); break;
        case Type::PEAK: handle_peak(e); break;
        case Type::PLANE: handle_plane(e); break;
        case Type::MARKER: handle_marker(e); break;
        case Type::RIGID: handle_rigid(e); break;
        case Type::INPUT: handle_input(e); break;
        }

      //events.push_back(e);
      events.push_back(EventPrivate());
      events.back().swap(e);
    }

  // filter pop
  if(filter)
    {
      for(Filters::iterator i = filters.begin(); i != filters.end(); i++)
        if(FilterGroup *g = *i)
          {
            if(g->out.capacity <= 0) continue;

            EventPrivate frame;
            if(g->pop(frame)) new_event(frame, false);
          }
    }

  return ret;
}

// protected //

// table=name num=name[,flags]
// table=types 0=void 1=char 2=int 3=float 127=error 128=Camera 129=Marker 130=Rigid
// table=names 0=internal 1=warning 2=error 3=fatal ...
// table=enable name=value
// table=trackers id=tid,tid,type,name [options ...]
// table=markers id=mid,tid,type,name [options ...]
// table=devices id=hwid,id,type,name [options ...]
// status=devices id=hwid,time [status ...]
// property=value
int ContextData::handle_internal(EventPrivate &e)
{
  string s = e;
  if(s.empty()) return 0;

  //cout << "# " << name << " event: t=" << e.time() << " char internal: " << s << endl;

  if(internal)
    {
      // keep internal events
      events.push_back(e);
    }

  if(s.find("table=") == 0)
    {
      if(types.parse("types", s));
      else if(names.parse("names", s))
        {
          // update filters
          string changed;
          for(Filters::iterator i = filters.begin(); i != filters.end(); i++)
            if(FilterGroup *g = *i)
              if(!valid(g->in.id) || !valid(g->out.id))
                {
                  g->in.id = g->out.id = names[g->name];
                  changed += "event."+g->name+"="+(g->enabled?"1":"0");
                  //cout << "# filtergroup " << name << " name=" << g->name << " id=" << names[g->name] << endl;
                }

          string ret = enable(changed);
          if(!enableEventMask && !ret.empty() && property<int>("initialized") == 1)
            {
              EventPrivate e2(Type::BYTE, "internal", 0, e.time(), "table=enable "+ret);
              new_event(e2);
            }

          if(!changed.empty()) update_filter_info(e.time());
        }
      else if(s.find("table=enable ") == 0)
        {
          if(enableEventMask) enable(s.substr(strlen("table=enable ")));;
        }
      else if(trackers.parse("trackers", s))
        {
          vector<int> t;
          vector<TrackerInfo> ti;
          for(size_t i = 0; i < trackers.size(); i++)
            {
              if(trackers[i].id != (uint32_t)-1) t.push_back(i);
              ti.push_back(trackers[i]);
            }
          properties.set("trackers", t);
          properties.set("trackerinfo", ti);

          {
            EventPrivate e2(Type::TRACKERINFO, "info", 0, e.time(), ti);
            new_event(e2);
          }
        }
      else if(markers.parse("markers", s))
        {
          for(size_t i = 0; i < trackers.size(); i++)
            trackers.at(i)->marker_ids.clear();
          vector<MarkerInfo> mi;
          for(size_t i = 0; i < markers.size(); i++)
            {
              mi.push_back(markers[i]);
              TrackerInfo *t = trackers.at(markers[i].tracker_id);
              if(t) t->marker_ids.push_back(i);
            }
          vector<TrackerInfo> ti;
          for(size_t i = 0; i < trackers.size(); i++)
            ti.push_back(trackers[i]);
          properties.set("markers", (int)markers.size());
          properties.set("markerinfo", mi);
          properties.set("trackerinfo", ti);

          {
            EventPrivate e2(Type::MARKERINFO, "info", 0, e.time(), markers);
            new_event(e2);
            EventPrivate e3(Type::TRACKERINFO, "info", 0, e.time(), ti);
            new_event(e3);
          }
        }
      else if(devices.parse("devices", s))
        {
          vector<DeviceInfo> di; di.reserve(devices.size());
          for(idmap<DeviceInfo>::iterator i = devices.begin(); i != devices.end(); i++)
            di.push_back(i->second);
          properties.set("deviceinfo", di);

          {
            EventPrivate e2(Type::DEVICEINFO, "info", 0, e.time(), di);
            new_event(e2);
          }
        }
    }
  else if(s.find("status=") == 0)
    {
      if(devices.parse("devices", s))
        {
          vector<DeviceInfo> di; di.reserve(devices.size());
          for(idmap<DeviceInfo>::iterator i = devices.begin(); i != devices.end(); i++)
            di.push_back(i->second);

          properties.set("deviceinfo", di);

          {
            EventPrivate e2(Type::DEVICEINFO, "info", 0, e.time(), di);
            new_event(e2);
          }
        }
    }
  else if(s.find("filter=") == 0)
    {
      if(!filter(s, false) && !error.empty())
        {
          EventPrivate e2(Type::ERROR, "warning", 0, e.time(), error);
          new_event(e2);
        }
    }
  else
    {
      stringmap m(s);
      for(stringmap::iterator o = m.begin(); o != m.end(); o++)
        properties.autoset(o->first, o->second);
    }

  return 1;
}

void ContextData::update_filter_info(int64_t time)
{
  string fn;
  vector<FilterInfo> fi;
  for(Filters::iterator i = filters.begin(); i != filters.end(); i++)
    if(FilterGroup *g = *i)
      if(valid(g->in.id) && valid(g->out.id) && !g->options().empty())
        {
          fn += (fn.empty()?"":",") + g->name;
          fi.push_back(FilterInfo(g->period, g->name, g->options()));
        }

  properties.set("filters", fn);
  properties.set("filterinfo", fi);

  {
    EventPrivate e(Type::FILTERINFO, "info", 0, time, fi);
    new_event(e);
  }
}

////
