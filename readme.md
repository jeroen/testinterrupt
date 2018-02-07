# Interruption

This package tests if C code in R packages properly catch a user interruption in order to clean up resources before exiting.

Our code uses [Simon's trick](https://stat.ethz.ch/pipermail/r-devel/2011-April/060702.html) to detect an interruption without jumping:

```c
void check_interrupt_fn(void *dummy) {
  R_CheckUserInterrupt();
}

int pending_interrupt() {
  return !(R_ToplevelExec(check_interrupt_fn, NULL));
}
```

Unfortunately this "hack" is the only method we have to gracefully deal with interrupts and widely used in R packages.

## What should happen

Our C function [C_wait_for_user](src/wait.c#L15-L23) loops until the user interrupts (by pressing `ESC` or `CTRL+C`)

```r
library(testinterrupt)
wait_for_user(progress = TRUE)
1 Mississippi...
```

When the user interrupts, the code should [break from the loop](src/wait.c#L19-L20) and execute some cleanup code before returning:

```r
> wait_for_user()
31 Mississippi...
User Interruption! Cleaning up!
[1] TRUE
```

This works as expected on Linux and MacOS.

## The problem

In __RStudio on Windows__ the interruption does not get caught by our C code. Instead it seems to get intercepted earlier by the IDE which sends the user straight back to the console, without running the cleanup code.

```r
> wait_for_user()
31 Mississippi...
>
```

This makes it impossible to write C code that properly free's and clean's resources (close connections, free handles, etc). I think it may also cause jumping over C++ destructors. The problem does not appear in RGUI on Windows, only in RStudio.

Interestingly the problem only appears if we call `Rprintf()` in our C code. When we disable the progress printing, the cleanup lines do get executed properly:


```r
> wait_for_user(FALSE)

User Interruption! Cleaning up!
[1] TRUE
```

This may suggest that the user interruption gets caught while executing the IDE's `ptr_R_WriteConsoleEx` callback function (which is [RWriteConsoleEx ](https://github.com/rstudio/rstudio/blob/master/src/cpp/r/session/RSession.cpp#L835-L855) in the case of rstudio). However it is unclear why this would only happen on Windows.


## Real world example

The ssh package allows for hosting an ssh tunnel. When the user interrupts, the local port should be closed.

```r
library(ssh)
session <- ssh_connect("dev.opencpu.org")
ssh_tunnel(session, port = 5555, target = "cran.r-project.org:21")
# interrupt here
```

In RStudio on Windows we get an error now because port 5555 is still in use:

```r
# fails because port 5555 is unavailable
ssh_tunnel(session, port = 5555, target = "cran.r-project.org:21")
```
