# Interruption

This packages tests if C code in R can properly catch a user interruption in order to clean up resources before exiting.

## What should happen

The example function loops in C until the user interrupts (by pressing `ESC` or `CTRL+C`)

```r
library(testinterrupt)
wait_for_user(progress = TRUE)
```

When the user interrupts, the code should [break from the loop](src/wait.c#L19-L20) and execute to the cleanup code before returning:

```r
> wait_for_user()
31 Mississippi...
Done! Cleaning up!
[1] TRUE
```

This works as expected on Linux and MacOS.

## The problem

In RStudio on Windows the interruption does not get caught in C. Instead it seems to get intercepted by the IDE and sends the user straight back to the interpreter. This makes it impossible to write C code that properly free's and clean's resources (close connections, free handles, etc).

```r
> wait_for_user()
31 Mississippi
>
```

The problem does not appear in RGUI on Windows, but it is unclear to me if this is a bug in rstudio, or in how R is embedded on Windows. 

Interestingly the problem only appears if we use `Rprintf()` in the C code. If we disable the progress printing, it does execute the cleanup part:


```r
> wait_for_user(F)

Done! Cleaning up!
[1] TRUE
```

This may suggest that the user interruption gets caught while executing the IDE's `ptr_R_WriteConsoleEx` callback function which is [RWriteConsoleEx ](https://github.com/rstudio/rstudio/blob/master/src/cpp/r/session/RSession.cpp#L835-L855) in the case of rstudio. However it is unclear why this would only happen on Windows.


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
