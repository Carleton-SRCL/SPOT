/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// owl.h -*- C -*-
// OWL C API v2.0

#ifndef OWL_H
#define OWL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifdef WIN32
#ifdef __DLL
#define OWLAPI __declspec(dllexport)
#else // !__DLL
#define OWLAPI __declspec(dllimport)
#endif // __DLL
#else // ! WIN32
#define OWLAPI
#endif // WIN32

#define OWL_MAX_FREQUENCY 960.0

/* id is unsigned 32-bit */
/* time is signed 64-bit, 1 count per frame or to be specified */
/* pose: pos, rot -- [x y z], [s x y z] */
/* options format: [opt1=value opt2=value1,value2 ...] */

/* Data Types */

struct OWLCamera {
  uint32_t id;
  uint32_t flags;
  float pose[7];
  float cond;
};

struct OWLPeak {
  uint32_t id;
  uint32_t flags;
  int64_t time;
  uint16_t camera;
  uint16_t detector;
  uint32_t width;
  float pos;
  float amp;
};

struct OWLPlane {
  uint32_t id;
  uint32_t flags;
  int64_t time;
  uint16_t camera;
  uint16_t detector;
  float plane[4];
  float offset;
};

struct OWLMarker {
  uint32_t id;
  uint32_t flags;
  int64_t time;
  float x, y, z;
  float cond;
};

struct OWLRigid {
  uint32_t id;
  uint32_t flags;
  int64_t time;
  float pose[7];
  float cond;
};

struct OWLInput {
  uint64_t hw_id;
  uint64_t flags;
  int64_t time;
  const uint8_t *data;
  const uint8_t *data_end;
};

/* Event */

enum {
  OWL_TYPE_INVALID = 0, OWL_TYPE_BYTE, OWL_TYPE_STRING = OWL_TYPE_BYTE, OWL_TYPE_INT, OWL_TYPE_FLOAT,
  OWL_TYPE_ERROR = 0x7F,
  OWL_TYPE_EVENT = 0x80, OWL_TYPE_FRAME = OWL_TYPE_EVENT, OWL_TYPE_CAMERA, OWL_TYPE_PEAK, OWL_TYPE_PLANE,
  OWL_TYPE_MARKER, OWL_TYPE_RIGID, OWL_TYPE_INPUT,
  OWL_TYPE_MARKERINFO, OWL_TYPE_TRACKERINFO, OWL_TYPE_FILTERINFO, OWL_TYPE_DEVICEINFO
};

struct OWLEvent {
  uint16_t type_id;
  uint16_t id;
  uint32_t flags;
  int64_t time;
  const char *type_name;
  const char *name;
  /* private */
  const void *data;
  const void *data_end;
};

/* Info */

struct OWLMarkerInfo {
  uint32_t id;
  uint32_t tracker_id;
  const char *name;
  const char *options;
};

struct OWLTrackerInfo {
  uint32_t id;
  const char *type;
  const char *name;
  const char *options;
  const uint32_t *marker_ids;
  const uint32_t *marker_ids_end;
};

struct OWLFilterInfo {
  uint32_t period;
  const char *name;
  const char *options;
};

struct OWLDeviceInfo {
  uint64_t hw_id;
  uint32_t id;
  int64_t time;
  const char *type;
  const char *name;
  const char *options;
  const char *status;
};

/* Initialization */

struct OWLAPI OWLContext;

OWLAPI struct OWLContext* owlCreateContext();
OWLAPI bool owlReleaseContext(struct OWLContext **ctx);

OWLAPI int owlOpen(struct OWLContext *ctx, const char *name, const char *open_options);
OWLAPI bool owlClose(struct OWLContext *ctx);
OWLAPI bool owlIsOpen(const struct OWLContext *ctx);

OWLAPI int owlInitialize(struct OWLContext *ctx, const char *init_options);
OWLAPI int owlDone(struct OWLContext *ctx, const char *done_options);

OWLAPI int owlStreaming(const struct OWLContext *ctx);
OWLAPI bool owlSetStreaming(struct OWLContext *ctx, int enable);

OWLAPI float owlFrequency(const struct OWLContext *ctx);
OWLAPI bool owlSetFrequency(struct OWLContext *ctx, float frequency);

OWLAPI const int* owlTimeBase(const struct OWLContext *ctx);
OWLAPI bool owlSetTimeBase(struct OWLContext *ctx, int num, int den);

OWLAPI float owlScale(const struct OWLContext *ctx);
OWLAPI bool owlSetScale(struct OWLContext *ctx, float scale);

OWLAPI const float* owlPose(const struct OWLContext *ctx);
OWLAPI bool owlSetPose(struct OWLContext *ctx, const float *pose);

OWLAPI const char* owlOption(const struct OWLContext *ctx, const char *option);
OWLAPI const char* owlOptions(const struct OWLContext *ctx);
OWLAPI bool owlSetOption(struct OWLContext *ctx, const char *option, const char *value);
OWLAPI bool owlSetOptions(struct OWLContext *ctx, const char *options);

OWLAPI const char* owlLastError(const struct OWLContext *ctx);

/* Markers */

OWLAPI bool owlSetMarkerName(struct OWLContext *ctx, uint32_t marker_id, const char *marker_name);
OWLAPI bool owlSetMarkerOptions(struct OWLContext *ctx, uint32_t marker_id, const char *marker_options);

OWLAPI const struct OWLMarkerInfo owlMarkerInfo(const struct OWLContext *ctx, uint32_t marker_id);

/* Trackers */

OWLAPI bool owlCreateTracker(struct OWLContext *ctx, uint32_t tracker_id, const char *tracker_type,
                      const char *tracker_name, const char *tracker_options);
OWLAPI bool owlCreateTrackers(struct OWLContext *ctx, const struct OWLTrackerInfo *info, uint32_t count);

OWLAPI bool owlDestroyTracker(struct OWLContext *ctx, uint32_t tracker_id);
OWLAPI bool owlDestroyTrackers(struct OWLContext *ctx, const uint32_t *tracker_ids, uint32_t count);

OWLAPI bool owlAssignMarker(struct OWLContext *ctx, uint32_t tracker_id, uint32_t marker_id,
                     const char *marker_name, const char *marker_options);
OWLAPI bool owlAssignMarkers(struct OWLContext *ctx, const struct OWLMarkerInfo *info, uint32_t count);

OWLAPI bool owlSetTrackerName(struct OWLContext *ctx, uint32_t tracker_id, const char *tracker_name);
OWLAPI bool owlSetTrackerOptions(struct OWLContext *ctx, uint32_t tracker_id, const char *tracker_options);

OWLAPI const struct OWLTrackerInfo owlTrackerInfo(const struct OWLContext *ctx, uint32_t tracker_id);

/* Filters */

OWLAPI bool owlSetFilter(struct OWLContext *ctx, uint32_t period, const char *name, const char *filter_options);
OWLAPI bool owlSetFilters(struct OWLContext *ctx, const struct OWLFilterInfo *info, uint32_t count);
OWLAPI const struct OWLFilterInfo owlFilterInfo(const struct OWLContext *ctx, const char *name);

/* Devices */

OWLAPI const struct OWLDeviceInfo owlDeviceInfo(const struct OWLContext *ctx, uint64_t hw_id);

/* Events */

OWLAPI const struct OWLEvent* owlPeekEvent(struct OWLContext *ctx, long timeout);
OWLAPI const struct OWLEvent* owlNextEvent(struct OWLContext *ctx, long timeout);

OWLAPI int owlGetString(const struct OWLEvent *e, char *value, uint32_t count);
OWLAPI int owlGetIntegers(const struct OWLEvent *e, int *value, uint32_t count);
OWLAPI int owlGetFloats(const struct OWLEvent *e, float *value, uint32_t count);

OWLAPI int owlGetCameras(const struct OWLEvent *e, struct OWLCamera *cameras, uint32_t count);
OWLAPI int owlGetPeaks(const struct OWLEvent *e, struct OWLPeak *peaks, uint32_t count);
OWLAPI int owlGetPlanes(const struct OWLEvent *e, struct OWLPlane *planes, uint32_t count);
OWLAPI int owlGetMarkers(const struct OWLEvent *e, struct OWLMarker *markers, uint32_t count);
OWLAPI int owlGetRigids(const struct OWLEvent *e, struct OWLRigid *rigids, uint32_t count);
OWLAPI int owlGetInputs(const struct OWLEvent *e, struct OWLInput *imputs, uint32_t count);

OWLAPI const struct OWLEvent* owlFindEvent(const struct OWLEvent *event, uint16_t type_id, const char *name);

/* Property */

OWLAPI const char* owlProperty(const struct OWLContext *ctx, const char *name);
OWLAPI int owlPropertyi(const struct OWLContext *ctx, const char *name);
OWLAPI float owlPropertyf(const struct OWLContext *ctx, const char *name);
OWLAPI int owlPropertyiv(const struct OWLContext *ctx, const char *name, int *value, uint32_t count);
OWLAPI int owlPropertyfv(const struct OWLContext *ctx, const char *name, float *value, uint32_t count);

/**/

#ifdef __cplusplus
} // extern C
#endif

#endif // OWL_H
