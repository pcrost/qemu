/*
 * QEMU Error Objects
 *
 * Copyright IBM, Corp. 2011
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.  See
 * the COPYING.LIB file in the top-level directory.
 */

#include "qemu-common.h"
#include "qapi/error.h"
#include "qemu/error-report.h"

struct Error
{
    char *msg;
    ErrorClass err_class;
    struct Error *next;
    bool prefixed;
};

Error *error_abort;

static void do_error_set(Error **errp, ErrorClass err_class,
                         void (*mod)(Error *, void *), void *mod_opaque,
                         const char *fmt, va_list ap)
{
    Error *err;
    int saved_errno = errno;

    if (errp == NULL) {
        return;
    }

    err = g_malloc0(sizeof(*err));

    err->msg = g_strdup_vprintf(fmt, ap);
    if (mod) {
        mod(err, mod_opaque);
    }
    err->err_class = err_class;

    if (errp == &error_abort) {
        error_report_err(err);
        abort();
    }

    err->next = *errp;
    *errp = err;

    errno = saved_errno;
}

void error_set(Error **errp, ErrorClass err_class, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_error_set(errp, err_class, NULL, NULL, fmt, ap);
    va_end(ap);
}

static void error_set_errno_mod(Error *err, void *opaque) {
    int os_errno = *(int *)opaque;
    char *msg1 = err->msg;

    if (os_errno != 0) {
        err->msg = g_strdup_printf("%s: %s", msg1, strerror(os_errno));
        g_free(msg1);
    }
}

void error_set_errno(Error **errp, int os_errno, ErrorClass err_class,
                     const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_error_set(errp, err_class, error_set_errno_mod, &os_errno, fmt, ap);
    va_end(ap);
}

void error_setg_file_open(Error **errp, int os_errno, const char *filename)
{
    error_setg_errno(errp, os_errno, "Could not open '%s'", filename);
}

#ifdef _WIN32

static void error_set_win32_mod(Error *err, void *opaque) {
    int win32_err = *(int *)opaque;
    char *msg1 = err->msg;

    if (win32_err != 0) {
        char *msg2 = g_win32_error_message(win32_err);
        err->msg = g_strdup_printf("%s: %s (error: %x)", msg1, msg2,
                                   (unsigned)win32_err);
        g_free(msg2);
        g_free(msg1);
    }
}

void error_set_win32(Error **errp, int win32_err, ErrorClass err_class,
                     const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    do_error_set(errp, err_class, error_set_win32_mod, &win32_err, fmt, ap);
    va_end(ap);
}

#endif

Error *error_copy(const Error *err)
{
    Error *err_new;

    if (!err) {
        return NULL;
    }
    err_new = g_malloc0(sizeof(*err));
    err_new->msg = g_strdup(err->msg);
    err_new->err_class = err->err_class;
    err_new->next = error_copy(err->next);

    return err_new;
}

ErrorClass error_get_class(const Error *err)
{
    if (err->next) {
        return ERROR_CLASS_MULTIPLE_ERRORS;
    }
    return err->err_class;
}

const char *error_get_pretty(Error *err)
{
    return err->msg;
}

void error_prefix(Error *err, const char *fmt, ...) {
    char *msg;
    char *fmt_full;
    va_list ap;

    if (!err || err->prefixed) {
        return;
    }
    err->prefixed = true;

    msg = err->msg;
    fmt_full =  g_strdup_printf("%s%%s", fmt);

    va_start(ap, fmt);
    err->msg = g_strdup_printf(fmt_full, ap, msg);
    va_end(ap);
    g_free(fmt_full);
    g_free(msg);
}

static void error_report_err_printf(void *opaque, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    error_vreport(fmt, ap);
    va_end(ap);
}

void error_report_err(Error *err)
{
    error_printf_fn(err, error_report_err_printf, NULL);
}

void error_free(Error *err)
{
    if (err->next) {
        error_free(err->next);
    }
    if (err) {
        g_free(err->msg);
        g_free(err);
    }
}

void error_printf_fn(Error *err, void (*printf_fn)(void *, const char *, ...),
                     void *printf_opaque)
{
    if (err->next) {
        error_printf_fn(err->next, printf_fn, printf_opaque);
    }
    printf_fn(printf_opaque, "%s\n", error_get_pretty(err));
    error_free(err);
}

void error_propagate(Error **dst_errp, Error *local_err)
{
    if (local_err && dst_errp == &error_abort) {
        error_report_err(local_err);
        abort();
    } else if (dst_errp) {
        Error *i;
        Error *old_dst_err = *dst_errp;

        *dst_errp = local_err;
        for (i = local_err; i; i = i->next) {
            /* Propagation implies that the caller is no longer the owner of the
             * error. Therefore reset prefixes, so higher level handlers can
             * prefix again.
             */
            i->prefixed = false;
            dst_errp = &i->next;
        }
        *dst_errp = old_dst_err;
    } else if (local_err) {
        error_free(local_err);
    }
}
