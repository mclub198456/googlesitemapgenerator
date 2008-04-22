/*************************************************
*       Perl-Compatible Regular Expressions      *
*************************************************/

/* Copyright (c) 1997-2000 University of Cambridge */

/**
 * @file include/pcreposix.h
 * @brief PCRE definitions
 */

#ifndef _AP_PCREPOSIX_H
#define _AP_PCREPOSIX_H

/* This is the header for the POSIX wrapper interface to the PCRE Perl-
Compatible Regular Expression library. It defines the things POSIX says should
be there. I hope. */

/* Have to include stdlib.h in order to ensure that size_t is defined. */

#include <stdlib.h>

/* Allow for C++ users */

#ifdef __cplusplus
extern "C" {
#endif

/* Options defined by POSIX. */

  /** Ignore case */
#define AP_REG_ICASE     0x01
  /** Don't match newlines with wildcards */
#define AP_REG_NEWLINE   0x02
  /** Don't match BOL */
#define AP_REG_NOTBOL    0x04
  /** Don't match EOL */
#define AP_REG_NOTEOL    0x08

/* These are not used by PCRE, but by defining them we make it easier
to slot PCRE into existing programs that make POSIX calls. */

  /** UNUSED! */
#define AP_REG_EXTENDED  0
  /** UNUSED! */
#define AP_REG_NOSUB     0

/* Error values. Not all these are relevant or used by the wrapper. */

enum {
  AP_REG_ASSERT = 1,  /* internal error ? */
  AP_REG_ESPACE,      /* failed to get memory */
  AP_REG_INVARG,      /* bad argument */
  AP_REG_NOMATCH      /* match failed */
};


/* The structure representing a compiled regular expression. */

typedef struct {
  void *re_pcre;
  size_t re_nsub;
  size_t re_erroffset;
} ap_regex_t;

/* The structure in which a captured offset is returned. */

typedef int ap_regoff_t;

typedef struct {
  ap_regoff_t rm_so;
  ap_regoff_t rm_eo;
} ap_regmatch_t;

#ifndef AP_DECLARE
#define AP_DECLARE(x) x
#endif /* AP_DECLARE */

/* The functions */

AP_DECLARE(int) ap_regcomp(ap_regex_t *, const char *, int);

/**
 * Match a null-terminated string against a pre-compiled regex.
 * @param preg The pre-compiled regex
 * @param string The string to match
 * @param nmatch Provide information regarding the location of any matches
 * @param pmatch Provide information regarding the location of any matches
 * @param eflags Bitwise or of any of:
 *   @li #REG_NOTBOL - match-beginning-of-line operator always
 *     fails to match
 *   @li #REG_NOTEOL - match-end-of-line operator always fails to match
 * @return 0 for successful match, #REG_NOMATCH otherwise
 */ 
AP_DECLARE(int) ap_regexec(const ap_regex_t *preg, const char *string,
		                           size_t nmatch, ap_regmatch_t *pmatch, int eflags);

/**
 * Return the error code returned by regcomp or regexec into error messages
 * @param errcode the error code returned by regexec or regcomp
 * @param preg The precompiled regex
 * @param errbuf A buffer to store the error in
 * @param errbuf_size The size of the buffer
 */
AP_DECLARE(size_t) ap_regerror(int errcode, const ap_regex_t *preg, 
		                               char *errbuf, size_t errbuf_size);

/** Destroy a pre-compiled regex.
 * @param preg The pre-compiled regex to free.
 */
AP_DECLARE(void) ap_regfree(ap_regex_t *preg);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* End of pcreposix.h */
