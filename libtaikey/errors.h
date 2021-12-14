#pragma once

enum retval_t {
    TK_OK,
    TK_CONSUMED,
    TK_NOT_CONSUMED,
    TK_ERROR,
    TK_TODO,
};

typedef enum retval_t retval_t;
