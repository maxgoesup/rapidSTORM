#include <dStorm/helpers/thread.h>
#include <string>
#include <list>
#include <tiffio.h>
#include <stdarg.h>

namespace dStorm {

class TIFFOperation {
    static ost::Mutex mutex;
    static TIFFOperation* current;

    ost::MutexLock lock;
    std::list<std::string> errors;
    const std::string error_title;

    TIFFErrorHandler old_warning_handler;
    TIFFErrorHandler old_error_handler;

    static void make_warning(const char *module, const char *fmt, va_list ap);
    static void make_error(const char *module, const char *fmt, va_list ap);
    static void ignore(const char *module, const char *fmt, va_list ap);

  public:
    TIFFOperation( 
        std::string error_title,
        bool suppress_warnings );
    void throw_exception_for_errors();
    ~TIFFOperation();
};
}
