#include "Exception.h"

const char *Exception::what() const noexcept {
    return exception::what();
}
