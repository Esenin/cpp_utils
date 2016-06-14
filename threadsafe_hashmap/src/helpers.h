#ifndef THREADSAFE_HASHMAP_HELPERS_H
#define THREADSAFE_HASHMAP_HELPERS_H

#include <iostream>
#include <stdlib.h>
#include <thread>

#ifndef NDEBUG
#define DEBUG(x) do { std::cerr << "DBG: " << x << "\n"; } while (false)
#define DEBUG2(x) do { std::cerr << "DBG: t_id:" << std::this_thread::get_id() << ": " << __func__ << ": " \
      << #x << ": " << x << "\n"; } while (false)
#define WARNING(x) do { std::cerr << "WARN: " << __FILE__ << " > " << __func__ << ": " << x << "\n"; } while (false)
#else
#define DEBUG(x) do {} while (false)
#define DEBUG2(x) do {} while (false)
#define WARNING(x) do {} while (false)
#endif
#define ERROR(x) do { std::cerr << "ERR: " << __FILE__ << " > " << __func__ << ": " << x << "\n"; \
                      exit(EXIT_FAILURE); } while (false)


#endif //THREADSAFE_HASHMAP_HELPERS_H
