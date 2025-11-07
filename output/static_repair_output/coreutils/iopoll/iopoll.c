#include <stdio.h>    // FILE, fflush_unlocked, fwrite_unlocked, clearerr_unlocked, fileno_unlocked
#include <stdlib.h>   // For __assert_fail and __stack_chk_fail (though often linked directly)
#include <stdbool.h>  // bool
#include <poll.h>     // poll, pollfd, POLLIN, POLLPRI, POLLOUT, POLLERR
#include <errno.h>    // errno, EINTR, EWOULDBLOCK
#include <sys/stat.h> // stat, fstat, S_ISREG, S_ISBLK
#include <unistd.h>   // fstat
#include <stddef.h>   // size_t

// External declarations for functions not provided in the snippet
// These are likely GNU extensions or coreutils specific wrappers
extern int isapipe(int fd);
extern int rpl_fclose(FILE *fp);

// Stack protector symbols (standard for GCC/Clang with -fstack-protector)
extern long __stack_chk_guard;
extern void __stack_chk_fail(void) __attribute__((noreturn));

// __assert_fail is typically from assert.h or glibc headers
extern void __assert_fail(const char *__assertion, const char *__file,
                          unsigned int __line, const char *__function)
    __attribute__((__nothrow__)) __attribute__((__noreturn__));

// Function: iopoll_internal
long long iopoll_internal(int fdin, int fdout, char block, char check_input) {
  long stack_chk_local = __stack_chk_guard;
  long long ret_val = -1; // Default error return value

  if ((fdin == -1) && (fdout == -1)) {
    __assert_fail("fdin != -1 || fdout != -1",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/iopoll.c",
                  0x40, "iopoll_internal");
  }

  struct pollfd fds[2];
  unsigned short second_fd_revents_mask; // Corresponds to uVar1 in original

  fds[0].fd = fdin;
  fds[0].revents = 0;
  fds[1].fd = fdout;
  fds[1].revents = 0;

  if (check_input != 1) { // param_4 != '\x01'
    fds[0].events = POLLERR; // 4
    fds[1].events = POLLERR; // 4 (local_1c = 4)
    second_fd_revents_mask = POLLERR; // 4 (uVar1 = 4)
  } else {
    fds[0].events = POLLIN | POLLPRI; // 0x81
    fds[1].events = POLLOUT;          // 0x80 (local_1c = 0x80)
    second_fd_revents_mask = POLLIN | POLLPRI | POLLOUT; // 0x38 (uVar1 = 0x38)
  }

  int poll_timeout;
  int poll_result;

  while (true) {
    if (block == 0) { // param_3 == '\0'
      poll_timeout = 0;
    } else {
      poll_timeout = -1;
    }

    poll_result = poll(fds, 2, poll_timeout);

    if (poll_result < 0) {
      if (errno == EINTR) {
        continue; // Retry if interrupted
      }
      ret_val = -3; // 0xfffffffd
      break;
    }

    if (poll_result == 0 && block == 0) { // param_3 != '\x01' (non-blocking, no events)
      ret_val = 0;
      break;
    }

    if (poll_result < 1) { // Should not happen if poll_result is not < 0 and not 0
      __assert_fail("0 < ret",
                    "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/iopoll.c",
                    0x58, "iopoll_internal");
    }

    if (fds[0].revents != 0) {
      ret_val = 0;
      break;
    }

    // Check fds[1].revents using the derived mask
    // Original local_1a was 0, but it must be fds[1].revents based on context
    if ((fds[1].revents & second_fd_revents_mask) != 0) {
      if (check_input == 0) { // param_4 == '\0'
        ret_val = 0;
      } else {
        ret_val = -2; // 0xfffffffe
      }
      break;
    }
    // If no relevant events, loop again (e.g., if poll_result > 0 but only irrelevant events occurred)
  }

  if (stack_chk_local != __stack_chk_guard) {
    __stack_chk_fail();
  }
  return ret_val;
}

// Function: iopoll
void iopoll(int param_1, int param_2, char param_3) {
  iopoll_internal(param_1, param_2, param_3, 1);
}

// Function: iopoll_input_ok
bool iopoll_input_ok(int param_1) {
  long stack_chk_local = __stack_chk_guard;
  struct stat local_a8;
  bool is_reg_or_blk;

  if (fstat(param_1, &local_a8) == 0 &&
      (S_ISREG(local_a8.st_mode) || S_ISBLK(local_a8.st_mode))) {
    is_reg_or_blk = true;
  } else {
    is_reg_or_blk = false;
  }

  if (stack_chk_local != __stack_chk_guard) {
    __stack_chk_fail();
  }
  return !is_reg_or_blk; // Returns true if NOT regular/block, false if IS regular/block
}

// Function: iopoll_output_ok
bool iopoll_output_ok(int param_1) {
  return isapipe(param_1) > 0;
}

// Function: fwait_for_nonblocking_write
long long fwait_for_nonblocking_write(FILE *param_1) {
  if (errno == EWOULDBLOCK) { // EWOULDBLOCK is 0xb
    int fd = fileno_unlocked(param_1);
    if (fd == -1) {
      errno = EWOULDBLOCK; // Restore errno if fileno_unlocked failed
      return 0;
    }
    // Original call: iopoll_internal(0xffffffff, iVar1, 1, 0)
    // fdin = -1, fdout = fd, block = 1, check_input = 0
    // When check_input = 0, iopoll_internal waits for POLLERR on fdout.
    // If iopoll_internal returns 0, it means POLLERR occurred on fdout.
    // This is interpreted as success for fwait_for_nonblocking_write.
    if (iopoll_internal(-1, fd, 1, 0) != 0) {
      errno = EWOULDBLOCK; // Restore errno as original code does
      return 0; // fwait_for_nonblocking_write returns 0 on failure
    } else {
      clearerr_unlocked(param_1);
      return 1; // fwait_for_nonblocking_write returns 1 on success
    }
  } else {
    return 0; // errno was not EWOULDBLOCK
  }
}

// Function: fclose_wait
bool fclose_wait(FILE *param_1) {
  int fflush_ret;
  do {
    fflush_ret = fflush_unlocked(param_1);
    if (fflush_ret == 0) break;
  } while (fwait_for_nonblocking_write(param_1) == 1); // Loop while fwait_for_nonblocking_write succeeds
  return rpl_fclose(param_1) == 0;
}

// Function: fwrite_wait
long long fwrite_wait(void *param_1, size_t param_2, FILE *param_3) {
  size_t remaining = param_2;
  void *current_ptr = param_1;

  while (remaining > 0) { // Loop while there's data to write
    size_t written_this_iter = fwrite_unlocked(current_ptr, 1, remaining, param_3);
    remaining -= written_this_iter;

    if (remaining == 0) { // All bytes written
      break;
    }

    // If not all bytes written, wait for non-blocking write
    if (fwait_for_nonblocking_write(param_3) != 1) { // fwait_for_nonblocking_write returns 1 on success
      return 0; // Return 0 on failure to wait
    }
    current_ptr = (void *)((char *)current_ptr + written_this_iter);
  }
  return 1; // All bytes successfully written
}