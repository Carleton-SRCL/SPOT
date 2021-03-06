/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// filters.cc -*- C++ -*-
// OWL C++ API v2.0

#include <iostream>

#include <assert.h>

#include "libowl.h"
#include "vector.h"

using namespace OWL;

////

template <> inline Vector::Vector(const Marker &m) : x(m.x), y(m.y), z(m.z) {}

inline void set_marker(const Vector &v, float cond, Marker &m)
{ m.x = v.x; m.y = v.y; m.z = v.z; m.cond = cond; }

inline size_t find_valid_markers(const Frames &frames, uint32_t id, std::vector<size_t> &indexes)
{
  indexes.clear();
  for(size_t f = 0; f < frames.size(); f++)
    if(frames[f].markers[id].cond > 0 && (frames[f].markers[id].flags & 0x10) == 0)
      indexes.push_back(f);
  return indexes.size();
}

//// FilterGroup ////

FilterGroup::FilterGroup(uint32_t id, const std::string &name, uint32_t period, bool enable) :
  enabled(enable), period(period), name(name), in(id, period*2), out(id, period*2)
{ }

void FilterGroup::clear()
{
  enabled = false;
  period = 0;
  while(!empty())
    {
      delete back();
      pop_back();
    }
}

void FilterGroup::setPeriod(uint32_t n)
{
  period = n;
  in.capacity = out.capacity = 2 * n;
  while(in.pop());
  while(out.pop());
}

void FilterGroup::push(const EventPrivate &frame)
{
  if(!enabled || in.capacity <= 0 || !in.push(frame)) return;

  for(Markers::iterator i = in.back().markers.begin(); i != in.back().markers.end(); i++)
    if(i->flags & 0x10) i->cond = -1;

  out.push_back(in.back());

  apply();
}

bool FilterGroup::merge(const EventPrivate &frame)
{
  if(!enabled || out.capacity <= 0) return false;
  return out.merge(frame);
}

bool FilterGroup::pop(EventPrivate &frame)
{
  bool ret = out.get(frame, period);
  while(in.pop());
  while(out.pop());
  return ret;
}

void FilterGroup::apply()
{
  if(in.empty() || out.empty()) return;

  size_t size = in.back().markers.size();

  // idiot check
  assert(out.size() == in.size());
  for(Frames::iterator i = in.begin(); i != in.end(); i++) assert(i->markers.size() == size);
  for(Frames::iterator i = out.begin(); i != out.end(); i++) assert(i->markers.size() == size);

  Frames *in = &this->in, *out = &this->out, tmp;

  for(size_t id = 0; id < size; id++)
    for(iterator i = begin(); i != end(); i++)
      {
        if(i != begin()) { tmp = *out; in = &tmp; }
        if(Filter *f = *i) f->apply(*in, *out, id, period);
        else erase(i--);
      }
}

//// Filters ////

Filters::Filters(bool shared) : shared(shared), enabled(false)
{
}

Filters::Filters(const Filters &f) : base(f), shared(f.shared), enabled(f.enabled)
{
}

Filters::~Filters()
{
  if(!shared) clear();
}

void Filters::clear()
{
  while(!empty())
    {
      delete back();
      pop_back();
    }
}

FilterGroup* Filters::find(const std::string &name)
{
  for(iterator i = begin(); i != end(); i++)
    {
      if(FilterGroup *g = *i) { if(g->name == name) return g; }
      else erase(i--);
    }
  return 0;
}

void Filters::push(const EventPrivate &frame)
{
  for(iterator i = begin(); i != end(); i++)
    {
      if(FilterGroup *g = *i) g->push(frame);
      else erase(i--);
    }
}

bool Filters::merge(const EventPrivate &frame)
{
  bool ret = false;
  for(iterator i = begin(); i != end(); i++)
    {
      if(FilterGroup *g = *i) ret |= g->merge(frame);
      else erase(i--);
    }
  return ret;
}

//// Filter ////

Filter::Filter(const std::string &type) : type(type) { }

Filter::~Filter() { }

bool Filter::set(const strings &o)
{ return false; }

void Filter::apply(const Frames &in, Frames &out, size_t id, uint32_t period)
{ }

////

namespace OWL {

  /*
  class FILTER : public Filter {
  public:

    FILTER() : Filter("FILTER") { }

    bool set(const strings &o) { return false; }

    void apply(const Frames &in, Frames &out, size_t id, uint32_t period) { }
  };
  */

  //// NoOpFilter ////

  class NoOpFilter : public Filter {
  public:

    NoOpFilter() : Filter("noop") { }
  };

  //// LERP ////

  class LERPFilter : public Filter {
  public:

    LERPFilter() : Filter("lerp") { }

    bool set(const strings &o) { return false; }

    void apply(const Frames &in, Frames &out, size_t id, uint32_t period)
    {
      std::vector<size_t> valid;
      if(find_valid_markers(out, id, valid) < 2) return;

      for(size_t i = 0; i+1 < valid.size(); i++)
        fill(out, id, valid[i], valid[i+1]);
    }

    // lerp
    void fill(Frames &out, size_t id, size_t t0, size_t t1)
    {
      // dv = (v1 - v0) / (t1 - t0), o = dv * t + v0
      size_t dt = t1 - t0;
      const Vector &v0 = out[t0].markers[id], v1 = out[t1].markers[id];
      const Vector dv = (v1 - v0) / dt;

      for(size_t t = 1; t < dt && t0+t < out.size(); t++)
        if(out[t0+t].markers[id].cond < 0)
          {
            const Vector o = dv * t + v0;
            set_marker(o, 1, out[t0+t].markers[id]);
          }
    }

  };

  //// SplineFilter ////

  // p=4, n=4*3+1=12+1 (minimal)
  // 0 x x x 1 x x x 2 x x x 3
  //         |-------|

  // first: X 0 1 2, last: 0 1 2 X

  // cubic hermite spline, finite difference slopes

  class SplineFilter : public Filter {
  public:

    int order;

    SplineFilter() : Filter("spline"), order(3) { }

    bool set(const strings &o)
    {
      if(o.size() != 2) return false;
      if(o[0] == "order") order = strtoi(o[1]);
      else return false;

      this->options[o[0]] = o[1];

      return true;
    }

    void apply(const Frames &in, Frames &out, size_t id, uint32_t period)
    {
      std::vector<size_t> valid;
      if(find_valid_markers(out, id, valid) < 3) return;

      if(valid.size() >= 4)
        for(size_t i = 0; i+3 < valid.size(); i++)
          spline(out, id, valid[i], valid[i+1], valid[i+2], valid[i+3]);
      else if(valid.size() == 3)
        {
          if(valid[0] != 0 && valid[2] == (uint32_t)in.capacity) first(out, id, valid[0], valid[1], valid[2]);
          else if(valid[0] == 0 && valid[2] != (uint32_t)in.capacity) last(out, id, valid[0], valid[1], valid[2]);
        }
      else if(valid.size() == 2)
        lerp(out, id, valid[0], valid[1]);
    }

    void spline(Frames &out, size_t id, size_t i0, size_t i1, size_t i2, size_t i3)
    {
      // hermite spline t=[0,1], i1-i2
      // mk = finite_difference(pk-1, pk, pk+1) * (tk+1 - tk)
      // o = p1 * F0(t) + m1 * F1(t) * size + p2 * F2(t) + m2 * F3(t) * size, t=[0,1]
      real size = i2 - i1, step = 1.0 / size;
      const Vector &p0 = out[i0].markers[id], &p1 = out[i1].markers[id], &p2 = out[i2].markers[id], &p3 = out[i3].markers[id];
      const Vector m1 = finite_difference(p0, p1, p2, i0, i1, i2) * size;
      const Vector m2 = finite_difference(p1, p2, p3, i1, i2, i3) * size;

      real t = step;
      for(size_t i = i1+1; i < i2; i++, t+=step)
        if(out[i].markers[id].cond < 0)
          {
            const Vector o = p1 * F0(t) + m1 * F1(t) + p2 * F2(t) + m2 * F3(t);
            set_marker(o, 1, out[i].markers[id]);
          }
    }

    void first(Frames &out, size_t id, size_t i0, size_t i1, size_t i2)
    {
      // hermite spline t=[0,1], i0-i1
      // mk = finite_difference(pk-1, pk, pk+1) * (tk+1 - tk)
      // o = p1 * F0(t) + m1 * F1(t) * size + p2 * F2(t) + m2 * F3(t) * size, t=[0,1]
      real size = i1 - i0, step = 1.0 / size;
      const Vector &p0 = out[i0].markers[id], &p1 = out[i1].markers[id], &p2 = out[i2].markers[id];
      const Vector &m0 = (p1 - p0); // same as slope(p0, p1, i0, i1) * size;
      const Vector &m1 = finite_difference(p0, p1, p2, i0, i1, i2) * size;

      real t = step;
      for(size_t i = i0+1; i < i1; i++, t+=step)
        if(out[i].markers[id].cond < 0)
          {
            const Vector o = p0 * F0(t) + m0 * F1(t) + p1 * F2(t) + m1 * F3(t);
            set_marker(o, 1, out[i].markers[id]);
          }
    }

    void last(Frames &out, size_t id, size_t i0, size_t i1, size_t i2)
    {
      // hermite spline t=[0,1], i1-i2
      // mk = finite_difference(pk-1, pk, pk+1) * (tk+1 - tk)
      // o = p1 * F0(t) + m1 * F1(t) * size + p2 * F2(t) + m2 * F3(t) * size, t=[0,1]
      real size = i2 - i1, step = 1.0 / size;
      const Vector &p0 = out[i0].markers[id], &p1 = out[i1].markers[id], &p2 = out[i2].markers[id];
      const Vector m1 = finite_difference(p0, p1, p2, i0, i1, i2) * size;
      const Vector m2 = (p2 - p1); // same as slope(p1, p2, i1, i2) * size;

      real t = step;
      for(size_t i = i1+1; i < i2; i++, t+=step)
        if(out[i].markers[i].cond < 0)
          {
            const Vector o = p1 * F0(t) + m1 * F1(t) + p2 * F2(t) + m2 * F3(t);
            set_marker(o, 1, out[i].markers[id]);
          }
    }

    void lerp(Frames &out, size_t id, size_t i0, size_t i1)
    {
      // lerp t=[0,1]
      // m = (p1 - p0) / (t1 - t0) * (t1 - t0), o = m * t + v0
      real size = i1 - i0, step = 1.0 / size;
      const Vector &p0 = out[i0].markers[id], &p1 = out[i1].markers[id];
      const Vector m = (p1 - p0); // same as slope(p0, p1, i0, i1) * size;

      real t = step;
      for(size_t i = i0+1; i < i1; i++, t+=step)
        if(out[i].markers[id].cond < 0)
          {
            const Vector o = m * t + p0;
            set_marker(o, 1, out[i].markers[id]);
          }
    }

    static const Vector slope(const Vector &p0, const Vector &p1, size_t t0, size_t t1)
    {
      // first_mk = (pk+1 - pk) / (tk+1 - tk), last_mk = (pk - pk-1) / (tk - tk-1)
      // first_m0 = (p1 - p0) / (t1 - t0), last_m1 = (p1 - p0) / (t1 - t0)
      // m = (p1 - p0) / (t1 - t0)
      return (p1 - p0) / (t1 - t0);
    }

    static const Vector finite_difference(const Vector &p0, const Vector &p1, const Vector &p2, size_t t0, size_t t1, size_t t2)
    {
      // mk = 0.5 * (pk+1 - pk) / (tk+1 - tk) + 0.5 * (pk - pk-1) / (tk - tk-1)
      // m1 = 0.5 * (p2 - p1) / (t2 - t1) + 0.5 * (p1 - p0) / (t1 - t0)
      return 0.5 * (p2 - p1) / (t2 - t1) + 0.5 * (p1 - p0) / (t1 - t0);
    }

    static const Vector cardinal(const Vector &p0, const Vector &p1, const Vector &p2, size_t t0, size_t t1, size_t t2)
    {
      // mk = (pk+1 - pk-1) / (tk+1 - tk-1)
      // m1 = (p2 - p0) / (t2 - t0)
      return (p2 - p0) / (t2 - t0);
    }

    // basis functions
    static real F0(real t) { return 2*t*t*t - 3*t*t + 1; }
    static real F1(real t) { return t*t*t - 2*t*t + t; }
    static real F2(real t) { return -2*t*t*t + 3*t*t; }
    static real F3(real t) { return t*t*t - t*t; }

  };

  //// AverageFilter ////

  class AverageFilter : public Filter {
  public:

    AverageFilter() : Filter("average") { }

    bool set(const strings &o) { return false; }

    void apply(const Frames &in, Frames &out, size_t id, uint32_t period)
    {
      if(period == 0 || period*2+1 >= in.size() || period*2+1 >= out.size()) return;
      size_t start = in.size() - period*2-1, now = in.size() - period-1;

      Vector a;
      size_t n = 0;
      for(size_t i = start; i < in.size(); i++)
        if(in[i].markers[id].cond > 0)
          {
            a += Vector(in[i].markers[id]);
            n++;
          }
      if(n == 0) return;

      set_marker(a / n, 1, out[now].markers[id]);
    }
  };

  //// create_filter() ////

  Filter* create_filter(const strings &s)
  {
    if(s.empty()) return 0;
    if(s[0] == "noop") return new NoOpFilter();
    if(s[0] == "lerp") return new LERPFilter();
    if(s[0] == "spline") return new SplineFilter();
    if(s[0] == "average") return new AverageFilter();

    return 0;
  }

} // namespace OWL

////
