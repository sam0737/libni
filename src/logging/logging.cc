#include <ni/logging/logging.hh>
#include <ni/logging/log_service.hh>

namespace ni
{

namespace logging
{

LogService* service_instance = nullptr;

LogService& init(size_t queue_size)
{
  service_instance = new LogService(queue_size);
  return *service_instance;
}

LogService& service()
{
  assert(service_instance);
  return *service_instance;
}

void start()
{
  service().start();
}

void stop()
{
  service().stop();
}

} // namespace logging
} // namespace ni
