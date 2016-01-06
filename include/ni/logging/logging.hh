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
#pragma once
#include <ni/config.hh>
#include <ni/format.hh>
#include <ni/logging/common.hh>
#include <ni/string_view.hh>

#define NI_FILE_PATH &__FILE__[NI_LOG_FILE_PATH_IDX]

#define NI_ERR_NAME(ERRNO) ni::logging::ERRNO_NAMES[ERRNO]

#define NI_ERROR(FMT, ...) \
  do \
  { \
    fflush(stdout); \
    ni::fmt::print(stderr, "\033[31mERROR\033[0m [{}:{}] " FMT "\n", \
                   NI_FILE_PATH, __LINE__, ##__VA_ARGS__); \
  } while (0)

#define NI_PERROR(FMT, ...) \
  do \
  { \
    fflush(stdout); \
    ni::fmt::print(stderr, "\033[31mERROR\033[0m [{}:{}] " FMT ": %s\n", \
                   NI_FILE_PATH, __LINE__, ##__VA_ARGS__, NI_ERR_NAME(errno)); \
  } while (0)

#define NI_PERROR_EN(FMT, ERRNO, ...) \
  do \
  { \
    fflush(stdout); \
    ni::fmt::print(stderr, "\033[31mERROR\033[0m [{}:{}] " FMT ": %s\n", \
                   NI_FILE_PATH, __LINE__, ##__VA_ARGS__, NI_ERR_NAME(errno)); \
  } while (0)

#define NI_FATAL(ACTION, FMT, ...) \
  do \
  { \
    ACTION(FMT, ##__VA_ARGS__); \
    exit(EXIT_FAILURE); \
  } while (0)

#if !defined(LOG_LEVEL) || !defined(LOGF_LEVEL)
  #include <ni/logging/capture.hh>

  #ifndef LOG_LEVEL
  #define LOG_LEVEL(LOGGER, LEVEL) \
    ni::logging::Capture(LOGGER, ni::logging::LogSeverity::LEVEL, NI_FILE_PATH,\
                         __LINE__)
  #endif // !defined(LOG_LEVEL)

  #ifndef LOGF_LEVEL
  #define LOGF_LEVEL(LOGGER, LEVEL, FMT, ...) \
    LOG_LEVEL(LOGGER, LEVEL).log(FMT, ##__VA_ARGS__)
  #endif // !defined(LOGF_LEVEL)

#endif // !defined(LOG_LEVEL) || !defined(LOGF_LEVEL)

#define LOG_DEBUG(LOGGER) LOG_LEVEL(LOGGER, Debug)
#define LOG_INFO(LOGGER) LOG_LEVEL(LOGGER, Info)
#define LOG_NOTICE(LOGGER) LOG_LEVEL(LOGGER, Notice)
#define LOG_WARN(LOGGER) LOG_LEVEL(LOGGER, Warning)
#define LOG_ERROR(LOGGER) LOG_LEVEL(LOGGER, Error)
#define LOG_CRITICAL(LOGGER) LOG_LEVEL(LOGGER, Critical)

#define LOGF_DEBUG(LOGGER, ...) LOGF_LEVEL(LOGGER, Debug, ##__VA_ARGS__)
#define LOGF_INFO(LOGGER, ...) LOGF_LEVEL(LOGGER, Info, ##__VA_ARGS__)
#define LOGF_NOTICE(LOGGER, ...) LOGF_LEVEL(LOGGER, Notice, ##__VA_ARGS__)
#define LOGF_WARN(LOGGER, ...) LOGF_LEVEL(LOGGER, Warning, ##__VA_ARGS__)
#define LOGF_ERROR(LOGGER, ...) LOGF_LEVEL(LOGGER, Error, ##__VA_ARGS__)
#define LOGF_CRITICAL(LOGGER, ...) LOGF_LEVEL(LOGGER, Critical, ##__VA_ARGS__)

#define LOGF_PERROR_EN(LOGGER, FMT, ERRNO, ...) \
  LOGF_ERROR(LOGGER, FMT, ##__VA_ARGS__) << ": " << NI_ERR_NAME(errno)

#define LOGF_PERROR(LOGGER, FMT, ...) \
  LOGF_PERROR_EN(LOGGER, FMT, errno, ##__VA_ARGS__)

#define LOG_IF(CONDITION, ACTION, ...) \
  if (CONDITION) \
    ACTION(__VA_ARGS__);

namespace ni
{
namespace logging
{

class LogService;

// Singleton interface
LogService& init(size_t queue_size);
LogService& service();
void start();
void stop();

} // namespace logging
} // namespace ni
