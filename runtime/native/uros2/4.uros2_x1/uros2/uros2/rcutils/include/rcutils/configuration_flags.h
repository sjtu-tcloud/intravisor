
#ifndef RCUTILS__CONFIGURATION_FLAGS_H_
#define RCUTILS__CONFIGURATION_FLAGS_H_

#ifdef __cplusplus
extern "C"
{
#endif

//#cmakedefine RCUTILS_NO_FILESYSTEM
//#cmakedefine RCUTILS_AVOID_DYNAMIC_ALLOCATION
//#cmakedefine RCUTILS_NO_THREAD_SUPPORT

#define RCUTILS_NO_FILESYSTEM
#define RCUTILS_AVOID_DYNAMIC_ALLOCATION
#define RCUTILS_NO_THREAD_SUPPORT


#ifdef __cplusplus
}
#endif

#endif  // RCUTILS__CONFIGURATION_FLAGS_H_