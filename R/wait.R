#' Wait for User
#'
#' Wait for the user to interrupt.
#'
#' @export
#' @param progress TRUE or FALSE
#' @useDynLib testinterrupt C_wait_for_user
wait_for_user <- function(progress = TRUE){
  .Call(C_wait_for_user, as.logical(progress))
}

