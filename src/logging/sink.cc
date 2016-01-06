#include <ni/logging/sink.hh>

#include <fcntl.h>
#include <stdio_ext.h>
#include <unistd.h>

#include <ni/logging/logging.hh>

namespace ni
{
namespace logging
{

int
FileSink::open(int fd)
{
  if (posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED) == -1)
  {
    NI_PERROR("posix_fadvise");
    return -1;
  }

  m_stream = fdopen(fd, "a");
  LOG_IF(!m_stream, NI_FATAL, NI_PERROR, "fdopen");

  // Assuming the FILE* is accessed exclusively by the logger thread, internal
  // locking is not needed.
//  __fsetlocking(m_stream, FSETLOCKING_BYCALLER);
  return 0;
}

int
FileSink::open(string_view filename)
{
  int fd = ::open(filename.data(), O_WRONLY|O_APPEND|O_CREAT, 0644);
  LOG_IF(fd == -1, NI_FATAL, NI_PERROR, "Failed to open {}", filename.data());
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

void
FileSink::write(LogMessage* msg)
{
  assert(m_stream);
  assert(msg);
  if (should_accept(msg))
    fwrite_unlocked(msg->writer.data(), msg->writer.size(), 1, m_stream);
}

void
FileSink::flush()
{
  fflush_unlocked(m_stream);
}

} // namespace logging
} // namespace ni
