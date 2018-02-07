#include <Rinternals.h>
#include <unistd.h>

/* Check for interrupt without long jumping */
void check_interrupt_fn(void *dummy) {
  R_CheckUserInterrupt();
}

int pending_interrupt() {
  return !(R_ToplevelExec(check_interrupt_fn, NULL));
}

SEXP C_wait_for_user(SEXP progress){
  static int count = 0;
  for(;;){
    if(asLogical(progress))
      Rprintf("\r%d Mississippi...", count++);
    usleep(200000);
    if(pending_interrupt())
      break;
  }
  Rprintf("User Interruption! Cleaning up!\n");
  return ScalarLogical(TRUE);
}
