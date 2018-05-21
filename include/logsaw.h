/*
MIT License

Copyright (c) 2018 kepler0

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <type_traits>
#include <tuple>
#include <iostream>
#include <utility>
#include <sstream>
#include <string>
#include <functional>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <vector>

#define LOGSAW_STR(str) decltype(str##_logstr)
#define LOGSAW_LEFT(_width) logsaw::align::width< _width, logsaw::align::left>
#define LOGSAW_RIGHT(_width) logsaw::align::width< _width, logsaw::align::right>

namespace logsaw {
    struct field {
        friend std::ostream& operator<<(std::ostream&, field& other);
    protected:
        friend class log;
        virtual std::ostream& out(std::ostream& os) { return os; };
    };

    struct runtime_field {};

    struct scoped_field;

    struct scoped_field_list {
        static scoped_field_list& instance() {
            static scoped_field_list sfl;
            return sfl;
        }

    private:
        friend scoped_field;
        friend class log;
        std::vector<scoped_field*> field_list;
    };

    struct scoped_field : public field {
        scoped_field() {
            scoped_field_list::instance().field_list.push_back(this);
        }

        ~scoped_field() {
            scoped_field_list::instance().field_list.erase(std::remove(
                scoped_field_list::instance().field_list.begin(),
                scoped_field_list::instance().field_list.end(), this
            ), scoped_field_list::instance().field_list.end());
        }
    private:
        virtual std::ostream& out(std::ostream& os) {
            return os;
        }
    };

    std::ostream& operator<<(std::ostream& os, field& other) {
        return other.out(os);
    }

    template<char... chars>
    using logstr = std::integer_sequence<char, chars...>;

    namespace text_literal {
        template<typename T, T... chars>
        constexpr logstr<chars...> operator""_logstr() { return {}; }
    }

    template<typename = void>
    struct text;

    template<char... chars>
    struct text<logstr<chars...>> : public field {
        const char * get() const {
            static constexpr char str[sizeof...(chars)+1] = { chars..., '\0' };
            return str;
        }
    protected:
        virtual std::ostream& out(std::ostream& os) {
            os << get();
            return os;
        }
    };

    template<>
    struct text<void> : public field, public runtime_field {
        text() : str("") {}
        constexpr text(const char * string) : str(string) {}
        text(char * string) : str(string) {}

        const char * get() const {
            return str;
        }


    protected:
        virtual std::ostream& out(std::ostream& os) {
            os << get();
            return os;
        }
        const char * str;
    };

    struct number : public field, public runtime_field {
        number() : n(0) {}
        number(long nmb) : n(nmb) {}

    protected:
        virtual std::ostream& out(std::ostream& os) {
            os << std::to_string(n);
            return os;
        }
        long n;
    };

    template<char separator_char>
    struct separator : public field {
    protected:
        virtual std::ostream& out(std::ostream& os) {
            os << separator_char;
            return os;
        }
    };

    struct timestamp : public field {
    protected:
        virtual std::ostream& out(std::ostream& os) {
            std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::ctime(&time);
            auto s = ss.str();
            s.pop_back();
            os << s;
            return os;
        }
    };

    template<int width>
    struct indent : public field {
    protected:
        virtual std::ostream& out(std::ostream& os) {
            os << std::string(width, ' ');
            return os;
        }
    };

    namespace align {
        enum side {
            left,
            right
        };

        template<int w, side s>
        struct width : public logsaw::field {
        protected:
            virtual std::ostream& out(std::ostream& os) {
                os << (s == left ? std::left : std::right);
                os << std::setw(w);
                return os;
            }
        };
    }

    template<int width>
    struct scoped_indent : public scoped_field {
    protected:
        virtual std::ostream& out(std::ostream& os) { 
            os << std::string(width, ' ');
            return os;
        }
    };

    template<int begin, int offset>
    struct scoped_index : public scoped_field {
    protected:
        virtual std::ostream& out(std::ostream& os) {
            os << i;
            i += offset;
            return os;
        }
        int i = begin;
    };

    template<typename>
    struct scoped_text;

    template<char... chars>
    struct scoped_text<logstr<chars...>> : public scoped_field {
    protected:
        virtual std::ostream& out(std::ostream& os) {
            constexpr static char str[sizeof...(chars) + 1] = { chars..., '\0' };
            os << str;
            return os;
        }
    };

    namespace align {
        template<int w, side s>
        struct scoped_width : public logsaw::scoped_field {
        protected:
            virtual std::ostream& out(std::ostream& os) {
                os << (s == left ? std::left : std::right);
                os << std::setw(w);
                return os;
            }
        };
    }

    template<typename, typename> struct field_filter_cons;

    template<typename T, typename... Args>
    struct field_filter_cons<T, std::tuple<Args...>> {
        using type = std::tuple<T, Args...>;
    };

    template<typename T>
    using field_filter_predicate_1 = std::is_base_of<runtime_field, T>;

    template<typename T>
    using field_filter_predicate_2 = std::is_base_of<field, T>;

    template<typename...> struct field_filter;
    
    template<> struct field_filter<> { using type = std::tuple<>; };

    template<typename First, typename... Rest>
    struct field_filter<First, Rest...> {
        using type = typename std::conditional<
            field_filter_predicate_1<First>::value && field_filter_predicate_2<First>::value, 
            typename field_filter_cons<First, typename field_filter<Rest...>::type>::type,
            typename field_filter<Rest...>::type
        >::type;

        using nontype = typename std::conditional<
            !field_filter_predicate_1<First>::value && field_filter_predicate_2<First>::value, 
            typename field_filter_cons<First, typename field_filter<Rest...>::type>::type,
            typename field_filter<Rest...>::type
        >::type;
    };

    template<typename First, typename... Rest>
    struct format {
        using all_fields_t = std::tuple<First, Rest...>;
        using runtime_fields_t = typename field_filter<First, Rest...>::type;
        using static_fields_t = typename field_filter<First, Rest...>::nontype;
    };

    template<typename TupleT>
    constexpr auto TupleSize = std::tuple_size_v<std::decay_t<TupleT>>;

    template<size_t I, typename TupleT>
    using TupleElement = std::tuple_element_t<I, std::decay_t<TupleT>>;

    template<
        size_t I = 0, size_t J = 0,
        typename Tuple1T, typename Tuple2T,
        typename FuncT>
    auto for_each_field(Tuple1T &&, Tuple2T &&, FuncT &&) -> std::enable_if_t<(I >= TupleSize<Tuple1T>)> {}

    template<
        size_t I = 0, size_t J = 0,
        typename Tuple1T, typename Tuple2T,
        typename FuncT>
    auto for_each_field(Tuple1T &&t1, Tuple2T &&t2, FuncT &&f) -> std::enable_if_t<(I < TupleSize<Tuple1T>)> {
        if constexpr (std::is_base_of_v<runtime_field, std::decay_t<TupleElement<I, Tuple1T>>>) {
            f(std::get<J>(std::forward<Tuple2T>(t2)));
            for_each_field<I + 1, J + 1>(std::forward<Tuple1T>(t1), std::forward<Tuple2T>(t2), std::forward<FuncT>(f));
        } else {
            f(std::get<I>(std::forward<Tuple1T>(t1)));
            for_each_field<I + 1, J>(std::forward<Tuple1T>(t1), std::forward<Tuple2T>(t2), std::forward<FuncT>(f));
        }
    }

    struct log {
    public:
        template<typename Format, typename... Fields>
        std::string add(Fields... _rt_fields) {
            static std::stringstream ss;
            ss.str("");
            
            for (scoped_field* f : scoped_field_list::instance().field_list) {
                ss << *f;
            }
            
            typename Format::runtime_fields_t rt_fields = 
                std::make_tuple(_rt_fields...);
            typename Format::all_fields_t fields;
            for_each_field(
                fields,
                rt_fields,
                [&](field & f) {
                    ss << f;
                }
            );
            
            std::string ln = ss.str();
            log_arr.push_back(ln);
            return ln;
        }

        friend std::ostream& operator<<(std::ostream& os, log& other);

    protected:
        std::ostream& out(std::ostream& os) {
            for (auto & ln : log_arr) {
                os << ln << '\n';
            }
            return os;
        }

        std::vector<std::string> log_arr;
    };

    std::ostream& operator<<(std::ostream& os, log& other) {
        return other.out(os);
    }
}
