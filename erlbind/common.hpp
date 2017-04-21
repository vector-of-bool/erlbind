#ifndef ERLBIND_COMMON_HPP_INCLUDED
#define ERLBIND_COMMON_HPP_INCLUDED

#ifdef _MSC_VER
#define ERLBIND_EXPORT_API __declspec(dllexport)
#else
#define ERLBIND_EXPORT_API __attribute__((visibility("default")))
#endif

#endif // ERLBIND_COMMON_HPP_INCLUDED