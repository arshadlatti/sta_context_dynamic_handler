# sta_context_dynamic_handler.h


**sta_context_dynamic_handler.h** — a single-header C library for
deterministic resource cleanup and structured exception handling.

*A project by Arshad Latti.*


## Features

- **Single header.** One file. No build step, no library to link, no
  dependencies beyond `<stdlib.h>` and `<string.h>`. Define
  `STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION` in exactly one `.c` file
  and include it everywhere else.

- **RAII-style cleanup.** Register a destructor once with
  `a_handle(ptr, free_fn)` or `a_handle_var(var, free_fn)`. It runs on
  every exit path — normal return, early `a_return`, or a thrown
  exception. No `goto cleanup`, no leaks when errors propagate.

- **try / catch / final / fail.** Four markers, uniform syntax:

  ```c
  a_try
      might_fail();
  a_catch
      printf("caught: %s\n", cdh->sub_exception->data);
  a_final
 ```

## Quick start

## The `a_begin` / `a_return` pair

Every function that uses CDH opens a scope with `a_begin()` and closes
it with `a_return`. They are a matched pair — if you call `a_begin()`,
you must call `a_return` before leaving the function. No other exit
path is allowed.

```c
#include <stdio.h>

#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta_context_dynamic_handler.h"

int cdh_example(void)
{
    a_begin()

    a_return(a_true)
}

int main(void)
{
   if(cdh_example())
	   puts("All OK");
   else
	   puts("Error happended");
   
    return 0;
}

```



## Resource cleanup with `a_handle`

`a_handle(ptr, free_fn)` registers a pointer with the current cdh. The
function `free_fn` is called automatically when the cdh is deleted —
that is, when you leave the function via `a_return`, whether the path
was success or failure.

```c
#include <stdio.h>

#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta_context_dynamic_handler.h"

int even_values(int n)
{
    a_begin()

    int * p = (int*) malloc(sizeof(int) * n);
    a_handle(p, free)

    int i;
    int num = 2;
    for (i = 0; i < n; i++)
    {
        p[i] = num;
        printf("even number %d\n", num);
        num += 2;
    }

    a_return(a_true)
}

int main(void)
{
    if (even_values(10))
        puts("All OK");
    else
        puts("Error happened");

    return 0;
}
```

## Returning a value: `a_handle_r` and `a_return_ok`

Some functions acquire a resource and hand it back to the caller. If
something goes wrong along the way, the resource should be freed. If
everything succeeds, ownership should move to the caller, and the
cleanup must not fire.

`a_handle_r` and `a_return_ok` are a matched pair that does exactly
this.

```c
#include <stdio.h>

#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta_context_dynamic_handler.h"

int * create_even_values_malloc(int n)
{
    a_begin()

    int * p = (int*) malloc(sizeof(int) * n);
    a_handle_r(p, free)

    int i;
    int num = 2;
    for (i = 0; i < n; i++)
    {
        p[i] = num;
        num += 2;
    }

    a_return_ok(p)
}

int main(void)
{
    int n = 5;
    int * p = create_even_values_malloc(n);
    if (p)
    {
        int i;
        for (i = 0; i < n; i++)
            printf("even number %d\n", p[i]);

        puts("All OK");
        free(p);
    }
    else
        puts("Error happened");

    return 0;
}
```





## Reassignable resources: `a_handle_var` and `a_sure`

`a_handle(ptr, free_fn)` captures the value of `ptr` at registration
time. If the variable is later reassigned to point somewhere else, the
cleanup still frees the *original* pointer — usually not what you want.

`a_handle_var(var, free_fn)` captures the address `&var` instead. At
cleanup time, the destructor is called on whatever `var` holds **at that
moment**. This is what you want for buffers, strings, or anything that
grows or is replaced during the function.

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta_context_dynamic_handler.h"

#define DISPLAY_TEXT_BUF_SIZE 4

int display_text(const char * lines[], int n)
{
    a_begin()

    int s_max = DISPLAY_TEXT_BUF_SIZE;
    char * s = (char*) malloc(sizeof(char) * DISPLAY_TEXT_BUF_SIZE);
    a_handle_var(s, free)
    s[0] = 0;

    int i;
    char * ss;
    for (i = 0; i < n; i++)
    {
        if (strlen(s) + strlen(lines[i]) + 1 < s_max)
        {
            strcat(s, lines[i]);
        }
        else
        {
            int needed = strlen(lines[i]) + strlen(s) + 1;
            ss = (char*) malloc(sizeof(char) * needed);
            a_sure(ss)
            strcpy(ss, s);
            free(s);
            s = ss;
            strcat(s, lines[i]);
            s_max = needed;
        }
    }

    puts(s);

    a_return(a_true)
}

#define LINES 2
const char * lines[LINES] = { "Hello ", "World" };

int main(void)
{
    if (display_text(lines, LINES))
        puts("All OK");
    else
        puts("Error happened");

    return 0;
}

```
### `a_sure(expr)`

`a_sure(expr)` is a one-line "this must succeed" check:

```c
/* a_sure(expr) expands to */
if (!(expr))
    a_return(0)
```

It is what you reach for right after an allocation, before you have a
cdh handle on the pointer yet:

```c
ss = (char*) malloc(sizeof(char) * needed);
a_sure(ss)
```

If `malloc` returns `NULL`, `a_sure` calls `a_return(0)`. The cdh is
deleted, all previously registered cleanups run — including `free(s)` —
and the caller sees a failure. You do not need to write the check by
hand.

Without `a_sure`, the same code is:

```c
ss = (char*) malloc(sizeof(char) * needed);
if (!ss)
    a_return(0)
```

`a_sure` is just shorter. It works on any boolean expression, not only
pointers.















# Detailed Explanation and Usage

CDH is built around three matched pairs of macros. Each pair handles a
different resource-lifetime pattern. Pick the pair that matches what
your function does, and use the examples as templates.

| Pair | Use case | Cleanup on error | Ownership on success |
|---|---|---|---|
| `a_begin` / `a_return` | Function with no returned resource | Yes | n/a |
| `a_handle` / `a_handle_var` | Resource consumed and freed inside the function | Yes | Stays with the cdh |
| `a_handle_r` / `a_return_ok` | Resource handed to the caller | Yes | Transfers to caller |

The rest of this section explains each pair with examples and the
reasoning behind them.

---

## `a_begin` and `a_return`

`a_begin()` opens a CDH scope. It declares a `context_dynamic_handler_t
*cdh` in the current function and allocates it. `a_return(expr)` deletes
that cdh, runs every cleanup registered with `a_handle` or
`a_handle_var`, propagates any unhandled exception, and returns `expr`
from the function.

They are a matched pair. If you call `a_begin()`, you must call
`a_return` before the function exits. No other exit path is allowed.









## A complete example: propagating an error across frames

This example shows a three-level call chain. The innermost function
throws, the middle function forwards the error upward, and the outer
function catches it.

```c
#include <stdio.h>

#define STA_CONTEXT_DYNAMIC_HANDLER_IMPLEMENTATION
#include "sta_context_dynamic_handler.h"

#define worker       { worker_cdh(a_sub);      if (a_is_error) break; }
#define worker_e     { if (!worker_cdh(a_sub)) a_return(0) }
#define worker_      worker_cdh(a_sub);

#define WORKER_ERROR_CODE 2452

int worker_cdh(a_cdh)
{
    puts("doing work");
    a_throw_i(WORKER_ERROR_CODE)
}

#define deep_worker   { deep_worker_cdh(a_sub);      if (a_is_error) break; }
#define deep_worker_e { if (!deep_worker_cdh(a_sub)) a_return(0) }
#define deep_worker_  deep_worker_cdh(a_sub);

int deep_worker_cdh(a_cdh)
{
    puts("calling worker");
    worker_e
}

int exception_example(void)
{
    a_begin()

    a_try

        deep_worker

    a_catch
        printf("there is exception %d\n", cdh->sub_exception->code);
    a_fail

    a_return(a_true)
}

int main(void)
{
    if (exception_example())
        puts("All OK");
    else
        puts("Error happened");

    return 0;
}
```

Output:

```
calling worker
doing work
there is exception 2452
Error happened
```

Trace:

1. `exception_example` opens with `a_begin()`, then `a_try` allocates
   `cdh->sub_exception` (its local inbox).
2. `deep_worker` invokes `deep_worker_cdh(a_sub)`. The child cdh's
   `exception` slot points at `exception_example`'s `sub_exception`.
3. Inside `deep_worker_cdh`, `worker_e` invokes `worker_cdh(a_sub)`.
   This grandchild's `exception` slot points at `deep_worker_cdh`'s
   `exception`, which is still `exception_example`'s `sub_exception`.
4. `worker_cdh` calls `a_throw_i(WORKER_ERROR_CODE)`. The write lands
   in `exception_example`'s `sub_exception` — a single slot shared
   across the chain because no intermediate frame installed its own
   `a_try`.
5. `a_return(0)` deletes `worker_cdh`'s frame. `worker_e` in the middle
   sees `a_is_error` is true and calls `a_return(0)` itself, deleting
   `deep_worker_cdh`'s frame.
6. Back in `exception_example`, `a_is_error` is true, so the try body
   exits and `a_catch` runs. `cdh->sub_exception->code` is 2452.
7. `a_fail` returns 0 to `main`.
8. `main` prints "Error happened".

---

## `a_cdh` and the three function-macro forms

`a_cdh` is a macro that declares a cdh parameter. It expands to:

```c
context_dynamic_handler_t * cdh
```

You use it in any function that should receive a cdh from its caller.
By convention the function is named `foo_cdh`:

```c
int worker_cdh(a_cdh)
{
    /* use cdh */
    a_return_ok(a_true)
}
```

Because `a_cdh` may be `NULL` — the caller is allowed to pass `a_null`
when no cdh is available — every function must check before using it.
`a_return`, `a_return_ok`, `a_throw_*`, `a_handle*`, and `a_is_error`
all tolerate `NULL` and behave sensibly.

For every `*_cdh` function, define three call-site macros:

```c
#define worker       { worker_cdh(a_sub);      if (a_is_error) break; }
#define worker_e     { if (!worker_cdh(a_sub)) a_return(0) }
#define worker_      worker_cdh(a_sub);
```

They differ only in what happens after the call returns:

| Macro | Where to use it | On error |
|---|---|---|
| `worker` | inside an `a_try` body | `break` out of the try block → catch runs |
| `worker_e` | in a non-try function that wants to propagate | `a_return(0)` immediately |
| `worker_` | when you want full manual control | do nothing — you check `a_is_error` yourself |

Pick the one that matches the surrounding context:

- **In an `a_try` body:** use `worker`. The `break` is what transfers
  control to the catch block without running the rest of the try body.
- **In a function whose own `a_return(0)` should propagate the error
  upward:** use `worker_e`. It is a shorthand for the common pattern
  `if (!worker_cdh(...)) a_return(0);`.
- **When you need to inspect the error before deciding:** use `worker_`
  and follow it with your own `if (a_is_error) { ... }`.

Note that `worker_e` relies on `worker_cdh` returning a truthy value on
success and `0`/`a_false` on failure. Every `*_cdh` function should
follow that convention — `a_return_ok(a_true)` on the happy path,
`a_return(0)` on error.

---

## `a_try`

```c
#define a_try a_sure(context_dynamic_handler_start_try(cdh)) do {
```

`a_try` opens a try block. It does two things:

1. Ensures `cdh->sub_exception` is allocated. If this is the first
   `a_try` in the frame, the slot is created. If it already exists
   from a previous try, it is cleared and reused.
2. Opens a `do { ... } while (0)` block. Calls made inside the try that
   detect an error execute `break`, which exits this block and falls
   through to the `a_catch` line.

`a_try` must be paired with exactly one `a_catch`. Using `a_try` twice
in the same function is allowed — the second one reuses the same slot.

---

## `a_catch`

```c
#define a_catch } while(0); if(a_is_error){
```

`a_catch` closes the try block and opens the catch. It is the `} while(0)`
that pairs with `a_try`'s `do {`, followed by an `if` test on the error
state.

Three things to keep in mind:

1. **The catch body runs only when an error is set.** On the happy path,
   the `if` is false and the catch body is skipped entirely.
2. **`cdh->sub_exception` holds the payload.** Inside the catch, read
   `cdh->sub_exception->type`, `->code`, or `->data` depending on what
   was thrown.
3. **Every catch must end with `a_final` or `a_fail`.** They close the
   `if (a_is_error) {` block that `a_catch` opened. Leaving the catch
   without a terminator is a syntax error — the braces do not balance.

---

## `a_fail` vs `a_final`

Both close the catch block. They differ in what happens to the error
state and to the function's return.

```c
#define a_final } context_dynamic_handler_clear_exception_and_error(cdh);
#define a_fail  } if(a_is_error) a_return(0)
```

| | `a_final` | `a_fail` |
|---|---|---|
| Action | Clears `sub_exception` and `is_error` | Returns 0 if an error is set |
| Execution | Continues after the catch | Exits the function immediately |
| Error state | Reset to "no error" | Preserved for the caller |

Use `a_final` when the catch block fully handles the error and the
function should keep running:

```c
a_try
    read_optional_config();
a_catch
    log_warn("using defaults");
a_final
/* execution continues here with a clean cdh */
a_return_ok(a_true)
```

Use `a_fail` when the catch block has done what it can and the error
should propagate to the caller:

```c
a_try
    read_required_config();
a_catch
    log_error("cannot continue");
a_fail
/* never reached — the function has already returned 0 */
```

If you omit both, the braces do not close and the compiler will tell
you. If you use both, only the first is reached and the second is dead
code. Pick one per catch block.

---

## `a_throw_i`, `a_throw_s`, `a_throw_s_copy`, and `a_throw`

Throwing writes a payload into `cdh->exception` and returns from the
current function. Four macros are available, differing in the payload
type and in who owns the memory.

### `a_throw_i(code)`

```c
#define a_throw_i(code) a_throw(CDH_CODE,code,a_null,a_null)
```

Throws an integer code. No allocation, no cleanup. Use this for
well-known error codes and library exit statuses.

```c
a_throw_i(WORKER_ERROR_CODE)   /* -> cdh->exception->code = 2452 */
```

### `a_throw_s(str)`

```c
#define a_throw_s(str) a_throw(CDH_STR,CDH_ERROR,(void*)(str),a_null)
```

Throws a borrowed string. The pointer is stored as-is; nothing is
copied and nothing is freed. Use this for string literals and for
strings whose lifetime extends past the throw.

```c
a_throw_s("invalid argument")   /* -> cdh->exception->data = "invalid argument" */
```

Do not use `a_throw_s` on a buffer that will be freed before the
caller reads it. The catch block will see a dangling pointer.

### `a_throw_s_copy(str)`

```c
#define a_throw_s_copy(str) a_throw(CDH_STR,CDH_ERROR,cdh_str_copy_malloc(str),(gt_term_free_func)free)
```

Throws a copy of the string. `cdh_str_copy_malloc` allocates a new
buffer, and the exception's `free_fn` is set to `free`. When the
exception container is cleared — by `a_final`, by the parent's delete,
or by a subsequent throw into the same slot — the copy is freed
automatically.

Use this whenever the source string is a local buffer or may be
overwritten by the time the caller reads the exception.

```c
char buf[64];
snprintf(buf, sizeof(buf), "failed at offset %d", off);
a_throw_s_copy(buf)   /* safe: buf can go out of scope */
```

### `a_throw(type, code, data, free_fn)`

```c
#define a_throw(type,code,data,free_fn) \
    do { context_dynamic_handler_set_exception(cdh,type,code,data,free_fn); a_return(0) } while(0)
```

The general form. Use it for user-defined error types with custom
payloads:

```c
#define FILE_ERROR_TYPE (CDH_DATA_USER + 1)

typedef struct {
    int index;
    char * filename;
} file_error_t;

static void file_error_delete(void * p)
{
    file_error_t * e = p;
    if (!e) return;
    free(e->filename);
    free(e);
}

a_throw(FILE_ERROR_TYPE, 0,
        file_error_new(name, i),
        (gt_term_free_func)file_error_delete)
```

The `free_fn` is called when the exception is cleared. If you pass
`a_null`, the payload is treated as borrowed and is not freed — useful
for pointers into long-lived data.

### Which one to use

| Situation | Macro |
|---|---|
| Integer code | `a_throw_i(code)` |
| String literal or static string | `a_throw_s(str)` |
| Local string buffer, or any string you do not own | `a_throw_s_copy(str)` |
| Custom struct with a destructor | `a_throw(type, code, data, free_fn)` |
| Custom struct without a destructor | `a_throw(type, code, data, a_null)` |

All four write to `cdh->exception` and then call `a_return(0)`. If
`cdh->exception` is `NULL` — the case at the top level with no caller —
`set_exception` frees the payload immediately (using `free_fn`) and
returns. The exception cannot propagate any further; the function still
returns 0 to its caller. This is the boundary condition, and it is why
the top-level function should install an `a_try` if it wants to observe
errors from its callees.
