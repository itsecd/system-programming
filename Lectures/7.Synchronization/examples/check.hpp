#ifndef CHECK_HPP
#define CHECK_HPP 1

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <type_traits>
#include <chrono>
#include <string>
namespace DO_NOT_USE_DIRECTLY {
    [[noreturn]]
	inline void error(const char* file, int line) {
		auto tmp = errno;//fprintf may fail, so we preserve errno
		fprintf(stderr, "%s (line %d) :", file, line);
        fflush(stderr);
		errno = tmp;
		perror(nullptr);
		exit(EXIT_FAILURE);
	}

    [[noreturn]]
    inline void error(int errcode, const char* file, int line) {
        fprintf(stderr, "%s (line %d) :", file, line);
        fflush(stderr);
        errno = errcode;
        perror(nullptr);
        exit(EXIT_FAILURE);
    }

    template<bool use_result_as_errno = false, typename T>
    inline T xcheck(T value, const char* file, int line){
        static_assert(std::is_integral_v<T>, "Value must be an integral type");
        if constexpr (!use_result_as_errno) {
            if (value < 0) error(file, line);
        }
        else {
            if (value != 0)
                error(value, file, line);
        }
        return value;
    }

	template<typename T>
	inline T* xcheck(T* p, const char* file, int line) {
		if (p == nullptr)  error(file, line);
		return p;
	}

    inline bool is_error_allowed(int allowed_code){
        return errno==allowed_code;
    }

    template <typename... TErrors>
    inline bool is_error_allowed(int allowed_code, TErrors... allowed_codes){
        return errno == allowed_code || is_error_allowed(allowed_codes...);
    }

    template <typename T, typename... TErrors>
    inline int xcheck_except(T value, const char* file, int line, TErrors... allowed_codes){
        static_assert(std::is_integral<T>::value, "Value must be an integral type");
        if(value >= 0 || is_error_allowed(allowed_codes...))
            return value;
        error(file, line);
    }

    template <typename T, typename... TErrors>
    inline T xcheck_except(T* p,const char* file, int line, TErrors... allowed_codes){
        if(p|| is_error_allowed(allowed_codes...))
            return p;
        error(file, line);
    }

}

//USE ONLY THIS MACRO
//Example: int fd = check(open("file", O_CREAT|O_RDWR, S_IRWXU));
#define check(x) DO_NOT_USE_DIRECTLY::xcheck(x, __FILE__, __LINE__ )
#define check_result(x) DO_NOT_USE_DIRECTLY::xcheck<true>(x, __FILE__, __LINE__ )
#define check_except(x,...) DO_NOT_USE_DIRECTLY::xcheck_except(x, __FILE__, __LINE__, __VA_ARGS__)

//https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros

// if you wish to avoid the macro (set C++20 in CMakeLists or in Project settings in VS)
//https://en.cppreference.com/w/cpp/utility/source_location


#endif // !CHECK_HPP