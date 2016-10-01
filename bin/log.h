/*
 * Copyright (c) 2015-2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef _LOG_H_
#define _LOG_H_

extern int _war2edit_log_dom;

#ifdef ERR
# undef ERR
#endif

#define DBG(...) EINA_LOG_DOM_DBG(_war2edit_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_war2edit_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_war2edit_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_war2edit_log_dom, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_war2edit_log_dom, __VA_ARGS__)

Eina_Bool log_init(void);
void log_shutdown(void);

#endif /* ! _LOG_H_ */
