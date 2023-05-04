#ifndef SFSNORT_DYNAMIC_DETECTION_LIB_H_
#define SFSNORT_DYNAMIC_DETECTION_LIB_H_

#define DETECTION_LIB_MAJOR 1
#define DETECTION_LIB_MINOR 0
#define DETECTION_LIB_BUILD 1

/* Required version and name of the engine */
#ifndef REQ_ENGINE_LIB_MAJOR
#define REQ_ENGINE_LIB_MAJOR 1
#endif
#ifndef REQ_ENGINE_LIB_MINOR
#define REQ_ENGINE_LIB_MINOR 6
#endif
#define REQ_ENGINE_LIB_NAME "SF_SNORT_DETECTION_ENGINE"


#ifdef WIN32
#ifdef SF_SNORT_DETECTION_DLL
#define DETECTION_LINKAGE __declspec(dllexport)
#else
#define DETECTION_LINKAGE __declspec(dllimport)
#endif
#else /* WIN32 */
#define DETECTION_LINKAGE
#endif /* WIN32 */

#endif /* SFSNORT_DYNAMIC_DETECTION_LIB_H_ */
