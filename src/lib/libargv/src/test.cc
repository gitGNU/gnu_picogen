
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <sstream>

bool starts_with (std::string const& str, std::string const &seq) {
        auto a = str.begin(), b = str.end(),
             x = seq.begin(), y = seq.end();
        for (; a != b && x != y; ++a, ++x) {
                if (*a != *x) return false;
        }
        if (a != b && x != y) return false;

        return true;
}

struct ArgumentMeta {
        bool flag;
};


struct Argument {
        bool has_name;
        std::string name;

        bool has_value;
        std::string value;

        static Argument Positional (std::string value) {
                Argument ret;
                ret.has_value = true;
                ret.value     = value;
                return ret;
        }

        static Argument NameValue (std::string name, std::string value) {
                Argument ret;
                ret.has_name  = true;
                ret.name      = name;
                ret.has_value = true;
                ret.value     = value;
                return ret;
        }

        static Argument NameOnly (std::string name) {
                Argument ret;
                ret.has_name = true;
                ret.name     = name;
                return ret;
        }

private:
        Argument () : has_name(false), has_value(false) {}
};

typedef std::vector<Argument> Arguments;


namespace detail {
        Argument long_argument (std::string str)
        {
                str = str.substr(2);
                const auto e = str.find_first_of('=');
                if (e == std::string::npos)
                        return Argument::NameOnly (str);
                return Argument::NameValue(str.substr(0, e), str.substr(1+e));
        }
        Argument short_argument (std::string str)
        {
                if (str.size() <= 2)
                        return Argument::NameOnly(str.substr(1));
                return Argument::NameValue(str.substr(1,1), str.substr(2));
        }
        Argument positional_argument (std::string str)
        {
                return Argument::Positional(str);
        }

        Argument argument (std::string const &curr)
        {
                if (starts_with (curr, "--")) return long_argument (curr);
                if (starts_with (curr, "-"))  return short_argument (curr);
                return positional_argument (curr);
        }

        std::vector<std::string> catenate (int argc, char *argv[])
        {
                std::list<std::string> raw_args (argv, argv+argc);
                std::vector<std::string> args;
                for (auto it = raw_args.begin(), end = raw_args.end(); it!=end; ++it) {
                        if (!args.empty()
                           && (*it == "=" || args.back().back() == '=')
                        ) {
                                args.back() += *it;
                        }
                        else {
                                args.push_back(*it);
                        }
                }
                return args;
        }
}

Arguments
parse (int argc, char *argv[])
{
        Arguments ret;
        for (auto s : detail::catenate (argc, argv))
                ret.push_back (detail::argument(s));
        return ret;
}

class names {
public:
        names (std::string const &short_opt,
               std::string const &long_opt)
                : short_opt (short_opt)
                , long_opt (long_opt)
        {
        }

        std::string short_opt, long_opt;
};
inline std::ostream& operator<< (std::ostream &os, names const& n)
{
        return os << "\"--" << n.long_opt << "\" (or alias \"-" << n.short_opt << "\")";
}

Arguments::const_iterator find (Arguments const &args, names const &n)
{
        auto it = std::find_if (args.begin(), args.end(), [&](Argument const &arg)
        {
                return arg.has_name && arg.name == n.short_opt;
        });
        if (it != args.end()) return it;

        return std::find_if (args.begin(), args.end(), [&](Argument const &arg)
        {
                return arg.has_name && arg.name == n.long_opt;
        });
}
#include <limits>
namespace detail {


        template <typename T>
        struct convert_helper {
                static bool convert (std::string const &str, T &ret) {
                        std::stringstream ss;
                        ss.str (str);
                        ss >> ret;

                        // The following is needed because streams accept negative signs
                        // even for unsigned types.
                        constexpr bool forbid_minus = std::numeric_limits<T>::is_integer &&
                                                      !std::numeric_limits<T>::is_signed;
                        if (forbid_minus) {
                                if (std::string::npos != str.find_first_of('-'))
                                        return false;
                        }

                        // fail() is set if nothing can be extracted (e.g.: int <- abc)
                        // but we may also have partial extraction (e.g. int <- 2a), which
                        // is covered by eof().
                        if (ss.fail() || !ss.eof()) {
                                return false;
                        }
                        return true;
                }
        };

        template <>
        struct convert_helper<std::string> {
                static bool convert (std::string const &str, std::string &ret) {
                        ret = str;
                        return true;
                }
        };

        template <typename T>
        bool convert (std::string const &str, T &ret)
        {
                return convert_helper<T>::convert (str, ret);
        }


        template <typename T> struct type_helper;

        template <> struct type_helper<int> {
                static const char* format_example() { return "1, 0, -99, +20"; }
                static const char* type_name () { return "integer"; }
        };
        template <> struct type_helper<unsigned int> {
                static const char* format_example() { return "10, +500, 99"; }
                static const char* type_name () { return "positive integer"; }
        };
        template <> struct type_helper<float> {
                static const char* format_example() { return "0.5, -2.3, 1e10"; }
                static const char* type_name () { return "real"; }
        };
        template <> struct type_helper<bool> {
                static const char* format_example() { return "1, 0"; }
                static const char* type_name() { return "boolean"; }
        };
        template <> struct type_helper<std::string> {
                static const char* format_example() { return "\"foobar\""; }
                static const char* type_name() { return "string"; }
        };

        template <typename T> const char* format_example()
        {
                return type_helper<T>::format_example();
        }

        template <typename T> const char* type_name()
        {
                return type_helper<T>::type_name();
        }

}

template <typename T>
T mandatory (Arguments &args, names const &n)
{
        auto it = find (args, n);
        if (it == args.end())
                throw std::runtime_error("Mandatory argument '--" + n.long_opt +
                                         "' (or '-" + n.short_opt + "') missing.");

        if (!it->has_value)
                throw std::runtime_error("Missing value for argument '--" + n.long_opt +
                                         "' (or '-" + n.short_opt + "'). "
                                         "Write  '--" + n.long_opt + "=<value>' "
                                         "or '-" + n.short_opt + "<value>'");

        T ret;
        if (!detail::convert (it->value, ret)) {
                std::stringstream ss;
                ss << "Format error: Passed value '" << it->value << "' for option "
                   << n << ", but " << detail::type_name<T>() << " is expected"
                   << ". Examples for valid formats: " << detail::format_example<T>();
                throw std::runtime_error (ss.str());
        }
        return ret;
}


int main (int argc, char *argv[]) {

        auto parsed = parse (argc, argv);
        auto const x = mandatory<std::string>(parsed, names("x", "Coeff"));

        std::cout << "{" << x << "}" << std::endl;
        /*
        int y = optional(parsed,
                         default_value(11),
                         names("y", "yankee"));
                         */
}
