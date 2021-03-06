/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// libowl.cc -*- C++ -*-
// OWL C++ API v2.0
// OWL::Context implementation

#include <iostream>

#include <string.h>

#ifndef WIN32
#include <sys/time.h>
#else
#include <windows.h>
#include <algorithm>

#ifndef _MSC_VER
#include "ntdef.h"
#endif

#endif

#include "libowl.h"
#include "protocol.h"

/*
  open(name:port, "timeout=t")
    wait(t)
    return !names.empty

  initialize("timeout=t slave=0|1 local=0|1")
    wait(t)
    return slave>=0
*/

using namespace std;
using namespace OWL;

#define DATA_LOCKER() MutexLocker l(data?&data->mutex:0)
#define CLEAR_ERROR() if(data) data->error.clear()
#define SET_ERROR(e,r) { if(data) data->error = (e); return r; }
#define OPEN_ERROR(e, r) \
  { data->properties.set("opened", int(0)); data->error = (e); return r; }
#define INITIALIZE_ERROR(e, r) \
  { data->properties.erase("initializing"); data->properties.set("initialized", int(0)); data->error = (e); return r; }
#define DONE_ERROR(e, r) \
  { data->properties.erase("flushing"); data->error = (e); return r; }

/*
  errors:
  null context is handled by lastError() / owlLastError()
  const functions can't set/clear errors
*/

//// utils ////

#ifdef WIN32
inline time_t timer() // usec
{
  LARGE_INTEGER f, t; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&t);
  return t.QuadPart * 1000000 / f.QuadPart;
}
#else // WIN32
inline time_t timer() // usec
{
  timeval tv; gettimeofday(&tv, 0);
  return tv.tv_sec * 1000000 + tv.tv_usec;
}
#endif

const Event* find_error(const Events &events)
{
  for(Events::const_iterator e = events.begin(); e != events.end(); e++)
    if(e->type_id() == Type::ERROR && (strcmp(e->name(), "error") == 0 || strcmp(e->name(), "fatal") == 0))
      return &*e;
  return 0;
}

//// Context ////

Context::Context() : data(0)
{ }

Context::~Context()
{
  close();
  if(data)
    {
      data->mutex.lock();
      CLEAR_ERROR();
      // move data->mutex into this function
      Mutex m; m.swap(data->mutex);
      delete data;
      data = 0;
      m.unlock();
    }
}

Context::Context(const Context &ctr)
{ }

// initialization //

int Context::open(const std::string &name, const std::string &open_options)
{
  if(!data) data = new ContextData(Device::create(), "libowl");
  MutexLocker l(&data->mutex);
  CLEAR_ERROR();
  if(data->property<int>("opened") == 1) return 1;

  stringmap m(open_options);
  long timeout = get(m, "timeout", long(5000000)), t0 = timer(), left = 0;

  // debug internal events
  if(data->internal == 0) data->internal = get(m, "internal", int(0));

  if(!data->isOpen())
    {
      data->clear();

      data->properties.set("name", name);

      int ret = data->open(name, open_options);
      if(ret < 0) OPEN_ERROR(data->error, ret);
      if(ret == 0 && timeout > 0) OPEN_ERROR("error: Connection timed out", -1);
      if(ret == 0) return 0;
    }

  // wait for "opened" == 1
  while(left >= 0)
    {
      int ret = data->recv(left);
      if(ret < 0) OPEN_ERROR(data->error, ret);
      if(data->property<int>("opened") == 1) break;
      if(const Event *e = find_error(data->events)) OPEN_ERROR(e->str(), -1);
      if(timeout == 0 && data->dev->events.empty()) break;
      left = timeout ? timeout - (timer() - t0) : 0;
    }

  if(data->property<int>("opened") == 0)
    {
      if(timeout > 0) OPEN_ERROR("error: Connection timed out", -1);
      return 0;
    }

  // finish

  ostring out;
  out << "protocol=" << int(OWL_PROTOCOL_VERSION);
#ifdef LIBOWL_REV
  out << " libowl=5.1." << LIBOWL_REV;
#endif // LIBOWL_REV
  if(data->send("internal", out) <= 0) OPEN_ERROR(data->error, -1);

  return 1;
}

bool Context::close()
{
  if(!data) return false;
  done();
  MutexLocker l(&data->mutex);
  CLEAR_ERROR();
  return data->close();
}

bool Context::isOpen() const
{ DATA_LOCKER(); return data ? data->isOpen() > 0 && data->property<int>("opened") == 1 : false; }

int Context::initialize(const std::string &init_options)
{
  if(!data) return -1;
  if(!isOpen()) SET_ERROR("error: Closed context", -1);
  MutexLocker l(&data->mutex);
  CLEAR_ERROR();

  stringmap m(init_options);
  long timeout = get(m, "timeout", long(5000000)), t0 = timer(), left = 0;

  if(data->property<int>("initialized") == 1)
    {
      if(data->property<int>("flushing") == 1) { data->error = "error: busy"; return 0; }
      if(data->property<int>("initializing") == 1) goto finish;

      // re-initialize
      data->properties.set("initialized", int(0));
    }

  // debug internal events
  if(data->internal == 0) data->internal = get(m, "internal", int(0));

  if(data->property<int>("initializing") == 0)
    {
      string name = data->properties("name");
      string profiles = data->properties("profiles");
      string defaultprofile = data->properties("defaultprofile");
      string profiles_json = data->properties("profiles.json");

      data->clear();

      data->properties.set("opened", int(1));
      data->properties.set("initializing", int(1));
      data->properties.set("name", name);
      data->properties.set("profiles", profiles);
      data->properties.set("defaultprofile", defaultprofile);
      data->properties.set("profiles.json", profiles_json);
      data->properties.set("local", get(m, "local", 0)); // handle "local"

      string o = string("event.raw=1 event.markers=1 event.rigids=1") + (init_options.empty()?"":" ") + init_options;
      data->enable(o);
      if(data->send("initialize", o) <= 0) INITIALIZE_ERROR(data->error, -1);
    }

  // wait for "initialized" == 1
  while(left >= 0)
    {
      int ret = data->recv(left);
      if(ret < 0) INITIALIZE_ERROR(data->error, ret);
      if(data->property<int>("initialized") == 1) break;
      if(const Event *e = find_error(data->events)) INITIALIZE_ERROR(e->str(), -1);
      if(timeout == 0 && data->dev->events.empty()) break;
      left = timeout ? timeout - (timer() - t0) : 0;
    }

  if(data->property<int>("initialized") == 0)
    {
      if(timeout > 0) INITIALIZE_ERROR("error: Connection timed out", -1);
      return 0;
    }

 finish:

  data->properties.erase("initializing");

  return 1;
}

int Context::done(const std::string &done_options)
{
  if(!data) return -1;
  if(!isOpen()) SET_ERROR("error: Closed context", -1);
  MutexLocker l(&data->mutex);
  CLEAR_ERROR();

  stringmap m(done_options);
  long timeout = get(m, "timeout", long(1000000)), t0 = timer(), left = 0;
  string keepalive = get(m, "keepalive", false) ? "keepalive=1" : "";
  if(data->property<int>("initialized") == 0)
    {
      if(data->property<int>("initializing") == 1) { data->error = "error: busy"; return 0; }
      if(data->property<int>("flushing") == 1) goto finish;
      return 1;
    }

  if(data->property<int>("flushing") == 0)
    {
      data->properties.set("flushing", int(1));

      if(data->send("done", keepalive) <= 0) DONE_ERROR(data->error, -1);
    }

  // wait for "initialized" == 0
  while(left >= 0)
    {
      int ret = data->recv(left);
      if(ret < 0) DONE_ERROR(data->error, -1);
      if(data->property<int>("initialized") == 0) break;
      if(const Event *e = find_error(data->events)) DONE_ERROR(e->str(), -1);
      if(timeout == 0 && data->dev->events.empty()) break;
      left = timeout ? timeout - (timer() - t0) : 0;
    }

  if(data->property<int>("initialized") == 1)
    {
      if(timeout > 0) DONE_ERROR("error: timed out", -1);
      return 0;
    }

 finish:

  data->properties.erase("flushing");

  return 1;
}

int Context::streaming() const
{ DATA_LOCKER(); return data ? data->property<int>("streaming") : 0; }

bool Context::streaming(int enable)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("streaming", vector<int>(1, enable)) > 0;
}

float Context::frequency() const
{ DATA_LOCKER(); return data ? data->property<float>("frequency") : -1; }

bool Context::frequency(float freq)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("frequency", vector<float>(1, freq)) > 0;
}

const int* Context::timeBase() const
{ return data ? (const int*)data->properties("timebase") : 0; }

bool Context::timeBase(int num, int den)
{
  if(!data) return false;
  CLEAR_ERROR();
  int r[2] = {num, den};
  data->properties.set("timebase", r, r+2);
  // for local=0
  return data->send("timebase", vector<int>(r, r+2));
}

float Context::scale() const
{ DATA_LOCKER(); return data ? data->property<float>("scale") : -1; }

bool Context::scale(float scale)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  if(data->property<int>("local") == 1)
    {
      data->properties.set("scale", scale);
      Cameras c;
      if(data->properties.get("systemcameras", c))
        {
          EventPrivate e(Type::CAMERA, data->names["cameras"], 0, data->dev->time, c);
          data->new_event(e);
        }
    }
  // for local=0
  return data->send(Type::FLOAT, data->names["scale"], 0, (const char*)&scale, sizeof(float)) > 0;
}

const float* Context::pose() const
{ DATA_LOCKER(); return data ? data->property<const float*>("pose") : 0; }

bool Context::pose(const float *pose)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  if(data->property<int>("local") == 1)
    {
      data->properties.set("pose", pose, pose+7);
      Cameras c;
      if(data->properties.get("systemcameras", c))
        {
          EventPrivate e(Type::CAMERA, data->names["cameras"], 0, data->dev->time, c);
          data->new_event(e);
        }
    }
  // for local=0
  return data->send(Type::FLOAT, data->names["pose"], 0, (const char*)pose, 7*sizeof(float)) > 0;
}

std::string Context::option(const std::string &option) const
{
  DATA_LOCKER();
  if(!data) return string();
  stringmap::const_iterator i = data->options.find(option);
  return i != data->options.end() ? i->second : string();
}

bool Context::option(const std::string &option, const std::string &value)
{
  if(!data) return false;
  CLEAR_ERROR();
  return options(option + "=" + value);
}

std::string Context::options() const
{ DATA_LOCKER(); return data ? data->properties("options") : string(); }

bool Context::options(const std::string &options)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  data->enable(options);
  return data->send("options", options) > 0;
}

std::string Context::lastError() const
{ DATA_LOCKER(); return data ? data->error : string("error: Invalid context"); }

// markers //

bool Context::markerName(uint32_t marker_id, const std::string &marker_name)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("markername", ostring() << "mid=" << marker_id << " name=" << marker_name) > 0;
}

bool Context::markerOptions(uint32_t marker_id, const string &marker_options)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("markeroptions", ostring() << "mid=" << marker_id << " " << marker_options) > 0;
}

const MarkerInfo Context::markerInfo(uint32_t marker_id) const
{ DATA_LOCKER(); return data ? data->markers[marker_id] : MarkerInfo(); }

// trackers //

bool Context::createTracker(uint32_t tracker_id, const std::string &tracker_type,
                            const std::string &tracker_name, const std::string &tracker_options)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  ostring out;
  out << "id=" << tracker_id << " type=" << tracker_type
      << " name=" << (!tracker_name.empty() ? tracker_name : to_string(tracker_id));
  if(!tracker_options.empty()) out << " " << tracker_options;
  return data->send("createtracker", out) > 0;
}

bool Context::createTrackers(const TrackerInfo *first, const TrackerInfo *last)
{
  DATA_LOCKER();
  if(!data || !first || !last || first == last) return false;
  CLEAR_ERROR();
  ostring out;
  for(const TrackerInfo *i = first; i != last; i++)
    {
      out << (i==first?"":" ") << "id=" << i->id
	  << " type=" << i->type
	  << " name=" << (!i->name.empty() ? i->name : to_string(i->id));
      if(!i->marker_ids.empty()) out << " mid=" << i->marker_ids;
      if(!i->options.empty()) out << " " << i->options;
    }
  return data->send("createtracker", out) > 0;
}

bool Context::destroyTracker(uint32_t tracker_id)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("destroytracker", ostring() << "id=" << tracker_id) > 0;
}

bool Context::destroyTrackers(const uint32_t *first, const uint32_t *last)
{
  DATA_LOCKER();
  if(!data || !first || !last || first == last) return false;
  CLEAR_ERROR();
  ostring out; out << "id=";
  for(const uint32_t *i = first; i != last; i++)
    out << (i==first?"":",") << *i;
  return data->send("destroytracker", out) > 0;
}

bool Context::assignMarker(uint32_t tracker_id, uint32_t marker_id,
                            const std::string &marker_name, const std::string &marker_options)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  ostring out;
  out << "tid=" << tracker_id << " mid=" << marker_id
      << " name=" << (!marker_name.empty() ? marker_name : to_string(marker_id));
  if(!marker_options.empty()) out << " " << marker_options;
  return data->send("assignmarker", out) > 0;
}

bool Context::assignMarkers(const MarkerInfo *first, const MarkerInfo *last)
{
  DATA_LOCKER();
  if(!data || !first || !last || first == last) return false;
  CLEAR_ERROR();
  ostring out;
  for(const MarkerInfo *i = first; i != last; i++)
    {
      out << (i==first?"":" ") << "tid=" << i->tracker_id << " mid=" << i->id
	  << " name=" << (!i->name.empty() ? i->name : to_string(i->id));
      if(!i->options.empty()) out << " " << i->options;
    }
  return data->send("assignmarker", out) > 0;
}

bool Context::trackerName(uint32_t tracker_id, const std::string &tracker_name)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("trackername", ostring() << "id=" << tracker_id << " name=" << tracker_name) > 0;
}

bool Context::trackerOptions(uint32_t tracker_id, const std::string &tracker_options)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();
  return data->send("trackeroptions", ostring() << "id=" << tracker_id << " " << tracker_options) > 0;
}

const TrackerInfo Context::trackerInfo(uint32_t tracker_id) const
{ DATA_LOCKER(); return data ? data->trackers[tracker_id] : TrackerInfo(); }

// filter //

bool Context::filter(uint32_t period, const std::string &name, const std::string &filter_options)
{
  DATA_LOCKER();
  if(!data) return false;
  CLEAR_ERROR();

  if(data->property<int>("local") == 1)
    {
      FilterInfo fi(period, name, filter_options);
      return data->filter(&fi, &fi+1, true);
    }

  ostringstream out;
  out << "filter=" << name << " period=" << period;
  if(!filter_options.empty()) out << " " << filter_options;

  return data->send("filter", out.str());
}

bool Context::filters(const FilterInfo *first, const FilterInfo *last)
{
  DATA_LOCKER();
  if(!data || !first || !last || first == last) return false;
  CLEAR_ERROR();

  if(data->property<int>("local") == 1) return data->filter(first, last, true);

  ostring out;
  for(const FilterInfo *i = first; i != last; i++)
    {
      out << (i==first?"":" ") << "filter=" << i->name << " period=" << i->period;
      if(!i->options.empty()) out << " " << i->options;
    }

  return data->send("filter", out);
}

const FilterInfo Context::filterInfo(const std::string &name) const
{ DATA_LOCKER(); return data ? data->filterInfo(name) : FilterInfo(); }

// devices //

const DeviceInfo Context::deviceInfo(uint64_t hw_id) const
{ DATA_LOCKER(); return data ? data->devices[hw_id] : DeviceInfo(); }

// events //

const Event* Context::peekEvent(long timeout)
{
  DATA_LOCKER();
  if(!data) return 0;
  CLEAR_ERROR();
  return data->peekEvent(timeout);
}

const Event* Context::nextEvent(long timeout)
{
  DATA_LOCKER();
  if(!data) return 0;
  CLEAR_ERROR();
  return data->nextEvent(timeout);
}

// property //

const Variant Context::property(const std::string &name) const
{
  DATA_LOCKER();
  if(!data) return Variant();
  CLEAR_ERROR();
  if(name == "*")
    {
      string out;
      for(Properties::const_iterator i = data->properties.begin(); i != data->properties.end(); i++)
        if(!i->first.empty())
          {
            if(i != data->properties.begin()) out.append(",");
            out.append(i->first);
          }
      return VariantPrivate(Type::BYTE, 0, out.data(), out.data()+out.size(), data->types[Type::BYTE].name);
    }
  return data->properties(name);
}

#undef CLEAR_ERROR
#undef SET_ERROR

////

#if !defined(_MSC_VER) || (_MSC_VER > 1600)

#include "owl.h"

const OWLEvent* copy_event(const OWL::Event *e, OWLEvent &event)
{
  event.type_id = e->type_id();
  event.id = e->id();
  event.flags = e->flags();
  event.time = e->time();
  event.type_name = e->type_name();
  event.name = e->name();
  event.data = e->begin();
  event.data_end = e->end();
  return &event;
}

const OWLEvent* copy_frame(const OWL::Event *f, OWLEvent &frame, std::vector<OWLEvent> &events)
{
  events.clear();
  for(const OWL::Event *e = f->begin(); e != f->end(); e++)
    {
      events.push_back(OWLEvent());
      copy_event(e, events.back());
    }
  frame.type_id = f->type_id();
  frame.id = f->id();
  frame.flags = f->flags();
  frame.time = f->time();
  frame.type_name = f->type_name();
  frame.name = f->name();
  frame.data = events.data();
  frame.data_end = events.data()+events.size();
  return &frame;
}

template <typename T>
int get(const Variant &v, int type_id, T *value, uint32_t count)
{
  if(!value || v.type_id() != type_id || !(const T*)v.begin() || !(const T*)v.end()) return -1;
  if(count == 0) return 0;
  uint32_t size = min<long>((const T*)v.end() - (const T*)v.begin(), (long)count);
  memcpy(value, (const void*)v, size * sizeof(T));
  return size;
}

template <typename T>
int get(const OWLEvent *e, int type_id, T *value, uint32_t count)
{
  if(!e || !e->data || !e->data_end || !type_id || !value || e->type_id != type_id) return -1;
  if(count == 0) return 0;
  uint32_t size = min<long>((const T*)e->data_end - (const T*)e->data, (long)count);
  memcpy(value, (const void*)e->data, size * sizeof(T));
  return size;
}

struct OWLContext : public Context {

  OWLEvent event;

  vector<OWLEvent> frameEvents;

  OWLContext() : Context()
  { }

  ~OWLContext()
  {
    close();
  }

  const ContextData* data() const { return Context::data; }
};

OWLContext* owlCreateContext()
{ return new OWLContext(); }

bool owlReleaseContext(struct OWLContext **ctx)
{
  if(!ctx) return false;
  delete *ctx;
  *ctx = 0;
  return true;
}

int owlOpen(struct OWLContext *ctx, const char *name, const char *open_options)
{ return ctx && name ? ctx->open(name, open_options?open_options:"") : -1; }

bool owlClose(struct OWLContext *ctx)
{ return ctx ? ctx->close() : false; }

bool owlIsOpen(const struct OWLContext *ctx)
{ return ctx ? ctx->isOpen() : false; }

int owlInitialize(struct OWLContext *ctx, const char *init_options)
{ return ctx ? ctx->initialize(init_options?init_options:"") : -1; }

int owlDone(struct OWLContext *ctx, const char *done_options)
{ return ctx ? ctx->done(done_options?done_options:"") : -1; }

int owlStreaming(const struct OWLContext *ctx)
{ return ctx ? ctx->streaming() : 0; }

bool owlSetStreaming(struct OWLContext *ctx, int enable)
{ return ctx ? ctx->streaming(enable) : false; }

float owlFrequency(const struct OWLContext *ctx)
{ return ctx ? ctx->frequency() : -1; }

bool owlSetFrequency(struct OWLContext *ctx, float frequency)
{ return ctx ? ctx->frequency(frequency) : false; }

const int* owlTineBase(const struct OWLContext *ctx)
{ return ctx ? ctx->timeBase() : 0; }

bool owlSetTimeBase(struct OWLContext *ctx, int num, int den)
{ return ctx ? ctx->timeBase(num, den) : false; }

float owlScale(const struct OWLContext *ctx)
{ return ctx ? ctx->scale() : -1; }

bool owlSetScale(struct OWLContext *ctx, float scale)
{ return ctx ? ctx->scale(scale) : false; }

const float* owlPose(const struct OWLContext *ctx)
{ return ctx ? ctx->pose() : 0; }

bool owlSetPose(struct OWLContext *ctx, const float *pose)
{ return ctx && pose ? ctx->pose(pose) : false; }

const char* owlOption(const struct OWLContext *ctx, const char *option)
{ return ctx ? ctx->option(option).c_str() : 0; }

const char* owlOptions(const struct OWLContext *ctx)
{ return ctx ? ctx->options().c_str() : 0; }

bool owlSetOption(struct OWLContext *ctx, const char *option, const char *value)
{ return ctx && option && value ? ctx->option(option, value) : false; }

bool owlSetOptions(struct OWLContext *ctx, const char *options)
{ return ctx && options ? ctx->options(options) : false; }

const char* owlLastError(const struct OWLContext *ctx)
{ return ctx ? ctx->lastError().c_str() : "error: Invalid context"; }

/* Markers */

bool owlSetMarkerName(struct OWLContext *ctx, uint32_t marker_id, const char *name)
{ return ctx ? ctx->markerName(marker_id, name?name:"") : false; }

bool owlSetMarkerOptions(struct OWLContext *ctx, uint32_t marker_id, const char *marker_options)
{ return ctx && marker_options ? ctx->markerOptions(marker_id, marker_options) : false; }

const struct OWLMarkerInfo owlMarkerInfo(const struct OWLContext *ctx, uint32_t marker_id)
{
  if(!ctx) { const OWLMarkerInfo r = { (uint32_t)-1, (uint32_t)-1, 0, 0}; return r; }
  const MarkerInfo &i = ctx->data()->markers[marker_id];
  const OWLMarkerInfo r = { i.id, i.tracker_id, i.name.c_str(), i.options.c_str() }; return r;
}

/* Trackers */

bool owlCreateTracker(struct OWLContext *ctx, uint32_t tracker_id, const char *tracker_type,
                      const char *tracker_name, const char *tracker_options)
{
  return ctx && tracker_type ?
    ctx->createTracker(tracker_id, tracker_type, tracker_name?tracker_name:"", tracker_options?tracker_options:"") :
    false;
}

bool owlCreateTrackers(struct OWLContext *ctx, const OWLTrackerInfo *info, uint32_t count)
{
  if(!ctx || !info || !count) return false;
  std::vector<TrackerInfo> t;
  for(const OWLTrackerInfo *i = info; i != info+count; i++)
    {
      std::vector<uint32_t> mids;
      if(i->marker_ids && i->marker_ids_end && i->marker_ids != i->marker_ids_end)
	mids.assign(i->marker_ids, i->marker_ids_end);
      t.push_back(TrackerInfo(i->id, i->type?i->type:"", i->name?i->name:"", i->options?i->options:"", mids));
    }
  return ctx->createTrackers(t.data(), t.data()+t.size());
}

bool owlDestroyTracker(struct OWLContext *ctx, uint32_t tracker_id)
{ return ctx ? ctx->destroyTracker(tracker_id) : false; }

bool owlDestroyTrackers(struct OWLContext *ctx, const uint32_t *tracker_ids, uint32_t count)
{ return ctx && tracker_ids && count ? ctx->destroyTrackers(tracker_ids, tracker_ids + count) : false; }

bool owlTrackerAssignMarker(struct OWLContext *ctx, uint32_t tracker_id, uint32_t marker_id,
                            const char *marker_name, const char *marker_options)
{ return ctx ? ctx->assignMarker(tracker_id, marker_id, marker_name?marker_name:"", marker_options?marker_options:"") : false; }

bool owlAssignMarkers(struct OWLContext *ctx, const OWLMarkerInfo *info, uint32_t count)
{
  if(!ctx || !info || !count) return false;
  std::vector<MarkerInfo> m;
  for(const OWLMarkerInfo *i = info; i != info+count; i++)
    m.push_back(MarkerInfo(i->id, i->tracker_id, i->name?i->name:"", i->options?i->options:""));
  return ctx->assignMarkers(m.data(), m.data()+m.size());
}

bool owlSetTrackerName(struct OWLContext *ctx, uint32_t tracker_id, const char *name)
{ return ctx ? ctx->trackerName(tracker_id, name?name:"") : false; }

bool owlSetTrackerOptions(struct OWLContext *ctx, uint32_t tracker_id, const char *tracker_options)
{ return ctx && tracker_options ? ctx->trackerOptions(tracker_id, tracker_options) : false; }

const struct OWLTrackerInfo owlTrackerInfo(const struct OWLContext *ctx, uint32_t tracker_id)
{
  if(!ctx || !ctx->data()) { const OWLTrackerInfo r = { (uint32_t)-1, 0, 0, 0, 0, 0}; return r; }
  const TrackerInfo &i = ctx->data()->trackers[tracker_id];
  const OWLTrackerInfo r = { i.id, i.type.c_str(), i.name.c_str(), i.options.c_str(),
                             i.marker_ids.data(), i.marker_ids.data()+i.marker_ids.size() };
  return r;
}

/* Filters */

bool owlSetFilter(struct OWLContext *ctx, uint32_t period, const char *name, const char *filter_options)
{ return ctx ? ctx->filter(period, name, filter_options) : false; }

bool owlSetFilters(struct OWLContext *ctx, const struct OWLFilterInfo *info, uint32_t count)
{
  if(!ctx || !info || !count) return false;
  std::vector<FilterInfo> f;
  for(const OWLFilterInfo *i = info; i != info+count; i++)
    f.push_back(FilterInfo(i->period, i->name, i->options));
  return ctx->filters(f.data(), f.data()+f.size());
}

const struct OWLFilterInfo owlFilterInfo(const struct OWLContext *ctx, const char *name)
{
  if(!ctx) { const OWLFilterInfo r = { 0, 0, 0 }; return r; }
  const FilterInfo f = ctx->filterInfo(name);
  const OWLFilterInfo r = { f.period, f.name.c_str(), f.options.c_str() }; return r;
}

/* Devices */

const struct OWLDeviceInfo owlDeviceInfo(const struct OWLContext *ctx, uint64_t hw_id)
{
  if(!ctx || !ctx->data()) { const OWLDeviceInfo r = { 0, 0, 0, 0, 0, 0, 0}; return r; }
  const DeviceInfo &i = ctx->data()->devices[hw_id];
  const OWLDeviceInfo r = { i.hw_id, i.id, i.time, i.type.c_str(), i.name.c_str(), i.options.c_str(), i.status.c_str() };
  return r;
}

/* Events */

const struct OWLEvent* owlPeekEvent(struct OWLContext *ctx, long timeout)
{
  const OWL::Event *e = ctx ? ctx->peekEvent(timeout) : 0;
  return e ? (e->type_id() == Type::FRAME ? copy_frame(e, ctx->event, ctx->frameEvents) : copy_event(e, ctx->event)) : 0;
}

const struct OWLEvent* owlNextEvent(struct OWLContext *ctx, long timeout)
{
  const OWL::Event *e = ctx ? ctx->nextEvent(timeout) : 0;
  return e ? (e->type_id() == Type::FRAME ? copy_frame(e, ctx->event, ctx->frameEvents) : copy_event(e, ctx->event)) : 0;
}

int owlGetString(const struct OWLEvent *e, char *value, uint32_t count)
{
  int ret = get(e, OWL_TYPE_STRING, value, count?count-1:0);
  if(ret > -1 && value) value[ret] = 0;
  return ret;
}

int owlGetIntegers(const struct OWLEvent *e, int *value, uint32_t count)
{ return get(e, OWL_TYPE_INT, value, count); }

int owlGetFloats(const struct OWLEvent *e, float *value, uint32_t count)
{ return get(e, OWL_TYPE_FLOAT, value, count); }

int owlGetCameras(const struct OWLEvent *e, struct OWLCamera *cameras, uint32_t count)
{ return get(e, OWL_TYPE_CAMERA, cameras, count); }

int owlGetPeaks(const struct OWLEvent *e, struct OWLPeak *peaks, uint32_t count)
{ return get(e, OWL_TYPE_PEAK, peaks, count); }

int owlGetPlanes(const struct OWLEvent *e, struct OWLPlane *planes, uint32_t count)
{ return get(e, OWL_TYPE_PLANE, planes, count); }

int owlGetMarkers(const struct OWLEvent *e, struct OWLMarker *markers, uint32_t count)
{ return get(e, OWL_TYPE_MARKER, markers, count); }

int owlGetRigids(const struct OWLEvent *e, struct OWLRigid *rigids, uint32_t count)
{ return get(e, OWL_TYPE_RIGID, rigids, count); }

int owlGetInputs(const struct OWLEvent *e, struct OWLInput *inputs, uint32_t count)
{ return get(e, OWL_TYPE_INPUT, inputs, count); }

const struct OWLEvent* owlFindEvent(const struct OWLEvent *event, uint16_t type_id, const char *name)
{
  if(event == 0 || (type_id == 0 && name == 0)) return 0;
  if((type_id == 0 || type_id == event->type_id) && (name == 0 || strcmp(name, event->name) == 0)) return event;
  if(event->type_id != OWL_TYPE_FRAME) return 0;
  for(const OWLEvent *e = (const OWLEvent*)event->data; e != (const OWLEvent*)event->data_end; e++)
    if(e && (type_id == 0 || type_id == e->type_id) && (name == 0 || strcmp(name, e->name) == 0))
      return e;
  return 0;
}

/* Property */

const char* owlProperty(const struct OWLContext *ctx, const char *name)
{ return ctx && name ? (const char*)ctx->property(name) : 0; }

int owlPropertyi(const struct OWLContext *ctx, const char *name)
{ return ctx && name ? (int)ctx->property(name) : 0; }

float owlPropertyf(const struct OWLContext *ctx, const char *name)
{ return ctx && name ? (float)ctx->property(name) : 0; }

int owlPropertyiv(const struct OWLContext *ctx, const char *name, int *value, uint32_t count)
{ return ctx && name ? get(ctx->property(name), OWL_TYPE_INT, value, count) : -1; }

int owlPropertyfv(const struct OWLContext *ctx, const char *name, float *value, uint32_t count)
{ return ctx && name ? get(ctx->property(name), OWL_TYPE_FLOAT, value, count) : -1; }

#endif // _MSC_VER > 1600

////
