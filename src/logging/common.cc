// Copyright (C) 2016 Zhe Wang <0x1998@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
#include <ni/logging/common.hh>

namespace ni
{
namespace logging
{

constexpr const char* ERRNO_NAMES[] = {
  /*   0 */ "",
  /*   1 */ "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO", "E2BIG",
  /*   8 */ "ENOEXEC", "EBADF", "ECHILD", "EAGAIN/EWOULDBLOCK", "ENOMEM",
  /*  13 */ "EACCES", "EFAULT", "ENOTBLK", "EBUSY", "EEXIST", "EXDEV", "ENODEV",
  /*  20 */ "ENOTDIR", "EISDIR", "EINVAL", "ENFILE", "EMFILE", "ENOTTY",
  /*  26 */ "ETXTBSY", "EFBIG", "ENOSPC", "ESPIPE", "EROFS", "EMLINK", "EPIPE",
  /*  33 */ "EDOM", "ERANGE", "EDEADLK/EDEADLOCK", "ENAMETOOLONG", "ENOLCK",
  /*  38 */ "ENOSYS", "ENOTEMPTY", "ELOOP", "", "ENOMSG", "EIDRM", "ECHRNG",
  /*  45 */ "EL2NSYNC", "EL3HLT", "EL3RST", "ELNRNG", "EUNATCH", "ENOCSI",
  /*  51 */ "EL2HLT", "EBADE", "EBADR", "EXFULL", "ENOANO", "EBADRQC",
  /*  57 */ "EBADSLT", "", "EBFONT", "ENOSTR", "ENODATA", "ETIME", "ENOSR",
  /*  64 */ "ENONET", "ENOPKG", "EREMOTE", "ENOLINK", "EADV", "ESRMNT", "ECOMM",
  /*  71 */ "EPROTO", "EMULTIHOP", "EDOTDOT", "EBADMSG", "EOVERFLOW",
  /*  76 */ "ENOTUNIQ", "EBADFD", "EREMCHG", "ELIBACC", "ELIBBAD", "ELIBSCN",
  /*  82 */ "ELIBMAX", "ELIBEXEC", "EILSEQ", "ERESTART", "ESTRPIPE", "EUSERS",
  /*  88 */ "ENOTSOCK", "EDESTADDRREQ", "EMSGSIZE", "EPROTOTYPE", "ENOPROTOOPT",
  /*  93 */ "EPROTONOSUPPORT", "ESOCKTNOSUPPORT", "EOPNOTSUPP/ENOTSUP",
  /*  96 */ "EPFNOSUPPORT", "EAFNOSUPPORT", "EADDRINUSE", "EADDRNOTAVAIL",
  /* 100 */ "ENETDOWN", "ENETUNREACH", "ENETRESET", "ECONNABORTED",
  /* 104 */ "ECONNRESET", "ENOBUFS", "EISCONN", "ENOTCONN", "ESHUTDOWN",
  /* 109 */ "ETOOMANYREFS", "ETIMEDOUT", "ECONNREFUSED", "EHOSTDOWN",
  /* 113 */ "EHOSTUNREACH", "EALREADY", "EINPROGRESS", "ESTALE", "EUCLEAN",
  /* 118 */ "ENOTNAM", "ENAVAIL", "EISNAM", "EREMOTEIO", "EDQUOT", "ENOMEDIUM",
  /* 124 */ "EMEDIUMTYPE", "ECANCELED", "ENOKEY", "EKEYEXPIRED", "EKEYREVOKED",
  /* 129 */ "EKEYREJECTED", "EOWNERDEAD", "ENOTRECOVERABLE", "ERFKILL",
  /* 133 */ "EHWPOISON"
};

} // namespace logging
} // namespace ni
