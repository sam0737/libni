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
#include <ni/logging/sink.hh>

#include <fcntl.h>
#include <stdio_ext.h>
#include <unistd.h>

#include <ni/logging/logging.hh>

namespace ni
{
namespace logging
{

StdStreamSink::StdStreamSink(LogSeverity level, FILE* stream,
                             bool unlocked) noexcept : Sink(level),
                                                       m_stream(stream),
                                                       m_unlocked(unlocked)
{
  if (unlocked)
    __fsetlocking(m_stream, FSETLOCKING_BYCALLER);
}

StdStreamSink::~StdStreamSink()
{
  flush();
}

void StdStreamSink::write(LogMessage* msg)
{
  assert(msg);
  if (should_accept(msg))
    fwrite(msg->writer.data(), msg->writer.size(), 1, m_stream);
}

void StdStreamSink::flush()
{
  fflush(m_stream);
}

int FileSink::open(int fd)
{
  if (posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED) == -1)
  {
    NI_PERROR("posix_fadvise");
    return -1;
  }

  m_stream = fdopen(fd, "a");
  if (!m_stream)
    NI_FATAL_ERRNO("fdopen");

  // Assuming the FILE* is accessed exclusively by the logger thread, internal
  // locking is not needed.
  //  __fsetlocking(m_stream, FSETLOCKING_BYCALLER);
  return 0;
}

int FileSink::open(string_view filename)
{
  int fd = ::open(filename.data(), O_WRONLY | O_APPEND | O_CREAT, 0644);
  if (fd == -1)
    NI_FATAL_ERRNO("Failed to open {}", filename.data());
  return open(fd) == -1 ? -1 : fd;
}

FileSink::~FileSink()
{
  if (m_stream)
  {
    flush();
    fsync(fileno_unlocked(m_stream));
    fclose(m_stream);
  }
}

void FileSink::write(LogMessage* msg)
{
  assert(m_stream);
  assert(msg);
  if (should_accept(msg))
    fwrite_unlocked(msg->writer.data(), msg->writer.size(), 1, m_stream);
}

void FileSink::flush()
{
  fflush_unlocked(m_stream);
}

} // namespace logging
} // namespace ni
