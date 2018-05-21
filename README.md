# `logsaw`
`Modern, header-only, compile-time formatted, tiny, & `*`super`*` flexible C++ logging library.`

`Inspired by the` [reckless](https://github.com/mattiasflodin/reckless) `logging library`

##### `C++17 is required`

## `features`
- `Compile-time formatting`
- `Scoped fields`
- `Column aligning`
- `Extendable (see "extending logsaw")`
- `Utilizes TMP to do most work at compile-time`
- `Header-only (only ~300 lines!)`

## `logsaw in a nutshell`
```cpp
#include "logsaw.h"
#include <iostream>

using namespace logsaw::text_literal; // required for LOGSAW_STR

using log_warning_t = logsaw::format<
        logsaw::text<LOGSAW_STR("WARNING:")>,
        logsaw::separator<' '>,
        LOGSAW_LEFT(30),
        logsaw::text<>,
        logsaw::text<LOGSAW_STR(" // logged at ")>,
        logsaw::timestamp>;

using log_error_t = logsaw::format<
        logsaw::text<LOGSAW_STR("ERROR:")>,
        logsaw::separator<' '>,
        LOGSAW_LEFT(30),
        logsaw::text<>,
        logsaw::text<LOGSAW_STR(" // logged at ")>,
        logsaw::timestamp>;

int main() {
    logsaw::log the_log;
    the_log.add<log_warning_t>("This was printed with logsaw!");
    {
        logsaw::scoped_text<LOGSAW_STR("-")> text;
        logsaw::scoped_indent<1> indent;
        for (int i = 1; i < 5; ++i) {
            if (i % 2) the_log.add<log_error_t>("Hello!");
            else the_log.add<log_error_t>("Goodbye!");
        }
    }
    the_log.add<log_warning_t>("Farewell, and happy logging!");
    std::cout << the_log;
    return 0;
}
```
### `Output:`
```
WARNING: This was printed with logsaw!  // logged at Mon May 21 17:17:03 2018
- ERROR: Hello!                         // logged at Mon May 21 17:17:03 2018
- ERROR: Goodbye!                       // logged at Mon May 21 17:17:03 2018
- ERROR: Hello!                         // logged at Mon May 21 17:17:03 2018
- ERROR: Goodbye!                       // logged at Mon May 21 17:17:03 2018
WARNING: Farewell, and happy logging!   // logged at Mon May 21 17:17:03 2018
```
## `extending logsaw`
### `creating your own static field`
```cpp
struct hello_world : public logsaw::field {
protected:
  virtual std::ostream& out(std::ostream& os) {
    os << "Hello, World!";
    return os;
  }
};
```
`now just use hello_world like any other field in logsaw::format`
### `creating your own runtime field`
```cpp
struct minus_one : public logsaw::field, public logsaw::runtime_field {
  minus_one() : number(0) {} // runtime_fields must have a default constructor
  minus_one(int the_number) : number(the_number) {}
protected:
  int number;
  virtual std::ostream& out(std::ostream& os) {
    os << std::to_string(number - 1);
    return os;
  }
};
```
`now use it like any other runtime field`
