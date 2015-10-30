/*
 * file.h
 *
 * Copyright (c) 2015 Jean Guyomarc'h
 */

#ifndef _FILE_H_
#define _FILE_H_

Eina_Bool file_selector_add(Editor *ed);
Eina_Bool file_save_prompt(Editor *ed);
Eina_Bool file_load_prompt(Editor *ed);

#endif /* ! _FILE_H_ */

