/*
 * Copyright 2021. Heekuck Oh, all rights reserved
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H

int pool_submit(void (*f)(void *p), void *p);
void pool_init(void);
void pool_shutdown(void);

#endif
