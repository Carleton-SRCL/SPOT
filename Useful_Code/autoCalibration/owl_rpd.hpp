// owl_rpd.hpp
// OWL C API v2.0

#ifndef OWL_RPD_HPP
#define OWL_RPD_HPP

#ifdef WIN32
#ifdef __DLL
#define OWLAPI __declspec(dllexport)
#else // !__DLL
#define OWLAPI __declspec(dllimport)
#endif // __DLL
#else // ! WIN32
#define OWLAPI
#endif // WIN32

namespace OWL {

  //// RPD ////

  struct OWLAPI RPD {

    enum { SAVE = 1, LOAD = 2 };

    int fd;
    int sock;
    int mode;

    int _write, _read, _send, _recv;

    std::vector<char> buffer;

    RPD();
    ~RPD();

    int open(const char *servername, const char *filename, int mode);
    bool close();
    bool flush();
    bool done();
    int send(long timeout=0);
    int recv(long timeout=0);
  };

} // namespace OWL

////

#endif // OWL_RPD_HPP
