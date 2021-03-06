/***
Copyright (c) PhaseSpace, Inc 2016

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL PHASESPACE, INC
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
***/

// owl_rpd.h
// OWL C API v2.0

#ifndef OWL_RPD_H
#define OWL_RPD_H

#ifdef __cplusplus
extern "C" {
#endif

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

#define OWL_RPD_SAVE 1
#define OWL_RPD_LOAD 2

struct OWLRPD;

OWLAPI OWLRPD* owlRPDCreate();
OWLAPI bool owlRPDRelease(struct OWLRPD **rpd);
OWLAPI bool owlRPDOpen(struct OWLRPD *rpd, const char *servername, const char *filename, int mode);
OWLAPI bool owlRPDClose(struct OWLRPD *rpd);
OWLAPI int owlRPDSend(struct OWLRPD *rpd, long timeout);
OWLAPI int owlRPDRecv(struct OWLRPD *rpd, long timeout);

#ifdef __cplusplus
}
#endif

#endif // OWL_RPD_H
