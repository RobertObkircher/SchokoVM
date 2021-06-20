#include <vector>
#include <string>
#include <csignal>
#include <cstring>

#include "util.hpp"

std::vector<std::string> split(std::string const &string, char separator) {
    std::vector<std::string> parts{};
    size_t start = 0;
    size_t end;
    while ((end = string.find(separator, start)) != std::string::npos) {
        parts.push_back(string.substr(start, end - start));
        start = end + 1;
    }
    parts.push_back(string.substr(start));
    return parts;
}


static const struct {
    int sig;
    const char *name;
}
        g_signal_info[] =
        {
                {SIGABRT, "SIGABRT"},
#ifdef SIGAIO
                {  SIGAIO,      "SIGAIO" },
#endif
                {SIGALRM, "SIGALRM"},
#ifdef SIGALRM1
                {  SIGALRM1,    "SIGALRM1" },
#endif
                {SIGBUS, "SIGBUS"},
#ifdef SIGCANCEL
                {  SIGCANCEL,   "SIGCANCEL" },
#endif
                {SIGCHLD, "SIGCHLD"},
#ifdef SIGCLD
                {  SIGCLD,      "SIGCLD" },
#endif
                {SIGCONT, "SIGCONT"},
#ifdef SIGCPUFAIL
                {  SIGCPUFAIL,  "SIGCPUFAIL" },
#endif
#ifdef SIGDANGER
                {  SIGDANGER,   "SIGDANGER" },
#endif
#ifdef SIGDIL
                {  SIGDIL,      "SIGDIL" },
#endif
#ifdef SIGEMT
                {SIGEMT, "SIGEMT"},
#endif
                {SIGFPE, "SIGFPE"},
#ifdef SIGFREEZE
                {  SIGFREEZE,   "SIGFREEZE" },
#endif
#ifdef SIGGFAULT
                {  SIGGFAULT,   "SIGGFAULT" },
#endif
#ifdef SIGGRANT
                {  SIGGRANT,    "SIGGRANT" },
#endif
                {SIGHUP, "SIGHUP"},
                {SIGILL, "SIGILL"},
#ifdef SIGINFO
                {SIGINFO, "SIGINFO"},
#endif
                {SIGINT, "SIGINT"},
#ifdef SIGIO
                {SIGIO, "SIGIO"},
#endif
#ifdef SIGIOINT
                {  SIGIOINT,    "SIGIOINT" },
#endif
#ifdef SIGIOT
// SIGIOT is there for BSD compatibility, but on most Unices just a
// synonym for SIGABRT. The result should be "SIGABRT", not
// "SIGIOT".
#if (SIGIOT != SIGABRT)
                {  SIGIOT,      "SIGIOT" },
#endif
#endif
#ifdef SIGKAP
                {  SIGKAP,      "SIGKAP" },
#endif
                {SIGKILL, "SIGKILL"},
#ifdef SIGLOST
                {  SIGLOST,     "SIGLOST" },
#endif
#ifdef SIGLWP
                {  SIGLWP,      "SIGLWP" },
#endif
#ifdef SIGLWPTIMER
                {  SIGLWPTIMER, "SIGLWPTIMER" },
#endif
#ifdef SIGMIGRATE
                {  SIGMIGRATE,  "SIGMIGRATE" },
#endif
#ifdef SIGMSG
                {  SIGMSG,      "SIGMSG" },
#endif
                {SIGPIPE, "SIGPIPE"},
#ifdef SIGPOLL
                {  SIGPOLL,     "SIGPOLL" },
#endif
#ifdef SIGPRE
                {  SIGPRE,      "SIGPRE" },
#endif
                {SIGPROF, "SIGPROF"},
#ifdef SIGPTY
                {  SIGPTY,      "SIGPTY" },
#endif
#ifdef SIGPWR
                {  SIGPWR,      "SIGPWR" },
#endif
                {SIGQUIT, "SIGQUIT"},
#ifdef SIGRECONFIG
                {  SIGRECONFIG, "SIGRECONFIG" },
#endif
#ifdef SIGRECOVERY
                {  SIGRECOVERY, "SIGRECOVERY" },
#endif
#ifdef SIGRESERVE
                {  SIGRESERVE,  "SIGRESERVE" },
#endif
#ifdef SIGRETRACT
                {  SIGRETRACT,  "SIGRETRACT" },
#endif
#ifdef SIGSAK
                {  SIGSAK,      "SIGSAK" },
#endif
                {SIGSEGV, "SIGSEGV"},
#ifdef SIGSOUND
                {  SIGSOUND,    "SIGSOUND" },
#endif
#ifdef SIGSTKFLT
                {  SIGSTKFLT,    "SIGSTKFLT" },
#endif
                {SIGSTOP, "SIGSTOP"},
                {SIGSYS, "SIGSYS"},
#ifdef SIGSYSERROR
                {  SIGSYSERROR, "SIGSYSERROR" },
#endif
#ifdef SIGTALRM
                {  SIGTALRM,    "SIGTALRM" },
#endif
                {SIGTERM, "SIGTERM"},
#ifdef SIGTHAW
                {  SIGTHAW,     "SIGTHAW" },
#endif
                {SIGTRAP, "SIGTRAP"},
#ifdef SIGTSTP
                {SIGTSTP, "SIGTSTP"},
#endif
                {SIGTTIN, "SIGTTIN"},
                {SIGTTOU, "SIGTTOU"},
#ifdef SIGURG
                {SIGURG, "SIGURG"},
#endif
                {SIGUSR1, "SIGUSR1"},
                {SIGUSR2, "SIGUSR2"},
#ifdef SIGVIRT
                {  SIGVIRT,     "SIGVIRT" },
#endif
                {SIGVTALRM, "SIGVTALRM"},
#ifdef SIGWAITING
                {  SIGWAITING,  "SIGWAITING" },
#endif
#ifdef SIGWINCH
                {SIGWINCH, "SIGWINCH"},
#endif
#ifdef SIGWINDOW
                {  SIGWINDOW,   "SIGWINDOW" },
#endif
                {SIGXCPU, "SIGXCPU"},
                {SIGXFSZ, "SIGXFSZ"},
#ifdef SIGXRES
                {  SIGXRES,     "SIGXRES" },
#endif
                {-1, nullptr}
        };


int get_signal_number(const char *signal_name) {
    std::string s = std::string(signal_name);
    if (s[0] != 'S' || s[1] != 'I' || s[2] != 'G') {
        s = "SIG" + s;
    }

    for (const auto &info : g_signal_info) {
        if (std::strcmp(info.name, s.c_str()) == 0) {
            return info.sig;
        }
    }
    return -1;
}
