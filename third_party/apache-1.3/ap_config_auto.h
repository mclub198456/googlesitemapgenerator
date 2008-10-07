/*
 *  ap_config_auto.h -- Automatically determined configuration stuff
 *  THIS FILE WAS AUTOMATICALLY GENERATED - DO NOT EDIT!
 */

#ifndef AP_CONFIG_AUTO_H
#define AP_CONFIG_AUTO_H

/* check: #include <dlfcn.h> */
#ifndef HAVE_DLFCN_H
#define HAVE_DLFCN_H 1
#endif

/* check: #include <dl.h> */
#ifdef HAVE_DL_H
#undef HAVE_DL_H
#endif

/* check: #include <bstring.h> */
#ifdef HAVE_BSTRING_H
#undef HAVE_BSTRING_H
#endif

/* check: #include <crypt.h> */
#ifndef HAVE_CRYPT_H
#define HAVE_CRYPT_H 1
#endif

/* check: #include <unistd.h> */
#ifndef HAVE_UNISTD_H
#define HAVE_UNISTD_H 1
#endif

/* check: #include <sys/resource.h> */
#ifndef HAVE_SYS_RESOURCE_H
#define HAVE_SYS_RESOURCE_H 1
#endif

/* check: #include <sys/select.h> */
#ifndef HAVE_SYS_SELECT_H
#define HAVE_SYS_SELECT_H 1
#endif

/* check: #include <sys/processor.h> */
#ifdef HAVE_SYS_PROCESSOR_H
#undef HAVE_SYS_PROCESSOR_H
#endif

/* check: #include <sys/param.h> */
#ifndef HAVE_SYS_PARAM_H
#define HAVE_SYS_PARAM_H 1
#endif

/* determine: longest possible integer type */
#ifndef AP_LONGEST_LONG
#define AP_LONGEST_LONG long long
#endif

/* determine: byte order of machine (12: little endian, 21: big endian) */
#ifndef AP_BYTE_ORDER
#define AP_BYTE_ORDER 12
#endif

/* determine: is off_t a quad */
#ifndef AP_OFF_T_IS_QUAD
#define AP_OFF_T_IS_QUAD 1
#endif

/* determine: is void * a quad */
#ifdef AP_VOID_P_IS_QUAD
#undef AP_VOID_P_IS_QUAD
#endif

/* build flag: -DLINUX=22 */
#ifndef LINUX
#define LINUX 22
#endif

/* build flag: -DTARGET="apache" */
#ifndef TARGET
#define TARGET "apache"
#endif

/* build flag: -DHAVE_SET_DUMPABLE */
#ifndef HAVE_SET_DUMPABLE
#define HAVE_SET_DUMPABLE 1
#endif

/* build flag: -DDB_DBM_HSEARCH=1 */
#ifndef DB_DBM_HSEARCH
#define DB_DBM_HSEARCH 1
#endif

/* build flag: -DDEV_RANDOM=/dev/random */
#ifndef DEV_RANDOM
#define DEV_RANDOM /dev/random
#endif

/* build flag: -DUSE_HSREGEX */
#ifndef USE_HSREGEX
#define USE_HSREGEX 1
#endif

/* build flag: -D_LARGEFILE_SOURCE */
#ifndef _LARGEFILE_SOURCE
#define _LARGEFILE_SOURCE 1
#endif

/* build flag: -D_FILE_OFFSET_BITS=64 */
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif

#endif /* AP_CONFIG_AUTO_H */
