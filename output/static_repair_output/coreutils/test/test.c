#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <libintl.h> // For gettext, bindtextdomain, textdomain
#include <ctype.h>   // For isspace, isdigit
#include <time.h>    // For struct timespec
#include <stdarg.h>  // For va_list in test_syntax_error

// --- Global variables and string literals ---
int pos;
int argc;
char **argv;

// Mock program_invocation_name if not available (e.g., on some non-GNU systems)
#ifndef program_invocation_name
char *program_invocation_name = "test";
#endif

// String literals used in the original code, defined as static const char arrays
static const char S_EQ[] = "=";
static const char S_NE[] = "!=";
static const char S_GT[] = ">";
static const char S_LT[] = "<";
static const char S_GE_INT[] = "-ge";
static const char S_LE_INT[] = "-le";
static const char S_EQ_INT[] = "-eq";
static const char S_NE_INT[] = "-ne";
static const char S_GT_INT[] = "-gt";
static const char S_LT_INT[] = "-lt";
static const char S_NT[] = "-nt"; // newer than
static const char S_OT[] = "-ot"; // older than
static const char S_RPAREN[] = ")";
static const char S_A[] = "-a"; // and
static const char S_OR[] = "-o"; // or
static const char S_LPAREN[] = "("; // for `test ( EXPR )` syntax
static const char S_NOT_PAREN[] = "!"; // for `test ! EXPR` syntax

// --- Mock/Helper functions for compilation ---

// Mock for GNU coreutils specific functions
static inline void set_program_name(char *name) {
    program_invocation_name = name;
}
static inline void initialize_exit_failure(int status) {
    // In a real coreutils context, this might set a global exit_failure_status.
    // For this snippet, it's a no-op.
}
static inline void close_stdout(void) {
    // In a real coreutils context, this ensures stdout is flushed and closed.
    // For this snippet, it's a no-op.
}
static inline void emit_ancillary_info(const char *info) {
    // In a real coreutils context, this prints additional info.
    // For this snippet, it's a no-op.
}

// For a simple compilable snippet, a pass-through is sufficient.
#ifndef _
#define _(msgid) gettext(msgid)
#endif

// Simplified quote function
static const char *quote(const char *s) {
    // A real 'quote' function would handle shell quoting rules.
    // For this snippet, we just return the string itself.
    return s;
}

// Simplified quote_n function
static const char *quote_n(int n, const char *s) {
    // A real 'quote_n' would handle shell quoting rules and pluralization.
    // For this snippet, we just return the string itself.
    return s;
}

// Custom strcmp wrapper for `streq`
static bool streq(const char *s1, const char *s2) {
    if (!s1 || !s2) return false;
    return strcmp(s1, s2) == 0;
}

// Mock for verror / test_syntax_error (simplified)
// This function must not return
void test_syntax_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", program_invocation_name);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(2);
}

// Mock timespec functions
static struct timespec make_timespec(long sec, long nsec) {
    struct timespec ts = { .tv_sec = sec, .tv_nsec = nsec };
    return ts;
}

static struct timespec get_stat_mtime(const struct stat *st) {
#ifdef __USE_XOPEN2K8
    return st->st_mtim;
#else
    struct timespec ts = { .tv_sec = st->st_mtime, .tv_nsec = 0 };
    return ts;
#endif
}

static struct timespec get_stat_atime(const struct stat *st) {
#ifdef __USE_XOPEN2K8
    return st->st_atim;
#else
    struct timespec ts = { .tv_sec = st->st_atime, .tv_nsec = 0 };
    return ts;
#endif
}

static int timespec_cmp(struct timespec ts1, struct timespec ts2) {
    if (ts1.tv_sec < ts2.tv_sec) return -1;
    if (ts1.tv_sec > ts2.tv_sec) return 1;
    if (ts1.tv_nsec < ts2.tv_nsec) return -1;
    if (ts1.tv_nsec > ts2.tv_nsec) return 1;
    return 0;
}

// Mock for psame_inode
static bool psame_inode(const struct stat *s1, const struct stat *s2) {
    return s1->st_ino == s2->st_ino && s1->st_dev == s2->st_dev;
}

// Mock for umaxtostr (unsigned long long to string)
static char *umaxtostr(size_t val, char *buf) {
    snprintf(buf, 32, "%zu", val); // Assuming buf is large enough (e.g., 32 bytes)
    return buf;
}

// Mock for strintcmp (string integer comparison)
static int strintcmp(const char *s1, const char *s2) {
    long l1 = strtol(s1, NULL, 10);
    long l2 = strtol(s2, NULL, 10);
    if (l1 < l2) return -1;
    if (l1 > l2) return 1;
    return 0;
}

// Mock for issymlink (standard way is lstat + S_ISLNK)
static int issymlink(const char *path) {
    struct stat st;
    if (lstat(path, &st) == 0) {
        return S_ISLNK(st.st_mode);
    }
    return 0; // Not a symlink or error
}

// --- Original functions, fixed and adapted ---

// Forward declarations for mutual recursion
static bool binary_operator(bool param_1_dash_l_flag, int param_2_op_code);
static bool unary_operator(void);
static bool posixtest(int param_1_nargs);
static bool expr(void);
static void beyond(void);

// Function: advance
static void advance(bool check_beyond) {
    pos++;
    if (check_beyond && argc <= pos) {
        beyond();
    }
}

// Function: unary_advance
static void unary_advance(void) {
    advance(true);
    pos = pos + 1; // This second increment is from the original snippet (skips operand)
}

// Function: beyond
static void beyond(void) {
    test_syntax_error(_("missing argument after %s"), quote(argv[argc - 1]));
}

// Function: find_int
static char *find_int(char *param_1) {
    char *current_char = param_1;
    while (isspace((unsigned char)*current_char)) {
        current_char++;
    }

    char *start_of_number = current_char;
    if (*current_char == '+') {
        current_char++;
        start_of_number = current_char;
    } else if (*current_char == '-') {
        current_char++;
    }

    if (isdigit((unsigned char)*current_char)) {
        do {
            current_char++;
        } while (isdigit((unsigned char)*current_char));

        while (isspace((unsigned char)*current_char)) {
            current_char++;
        }

        if (*current_char == '\0') {
            return start_of_number;
        }
    }

    test_syntax_error(_("invalid integer %s"), quote(param_1));
    return NULL; // Should not be reached
}

// Function: get_mtime
static struct timespec get_mtime(char *param_1) {
    struct stat st;
    if (stat(param_1, &st) < 0) {
        return make_timespec(0x8000000000000000LL, 0xffffffff); // Represents error/min time
    } else {
        return get_stat_mtime(&st);
    }
}

// Function: binop
static int binop(const char *op_str) {
    if (streq(op_str, S_EQ)) return 0;
    if (streq(op_str, S_NE)) return 1;
    if (streq(op_str, S_GT)) return 2;
    if (streq(op_str, S_LT)) return 3;
    if (streq(op_str, S_GE_INT)) return 4;
    if (streq(op_str, S_LE_INT)) return 5;
    if (streq(op_str, S_EQ_INT)) return 6;
    if (streq(op_str, S_NE_INT)) return 7;
    if (streq(op_str, S_GT_INT)) return 8;
    if (streq(op_str, S_LT_INT)) return 9;
    if (streq(op_str, "-ef")) return 10;
    if (streq(op_str, S_NT)) return 11;
    if (streq(op_str, S_OT)) return 12;
    return -1; // Not found
}

// Function: term
static bool term(void) {
    bool negated = false;
    while (pos < argc && streq(argv[pos], S_NOT_PAREN)) { // "!"
        advance(true); // Advance past "!"
        negated = !negated;
    }

    if (argc <= pos) {
        beyond();
    }

    bool result;
    if (streq(argv[pos], S_LPAREN)) { // "("
        advance(true); // Advance past "("
        int paren_expr_len = 0;
        for (int i = 1; pos + i < argc; i++) {
            if (streq(argv[pos + i], S_RPAREN)) { // ")"
                paren_expr_len = i;
                break;
            }
            if (i == 4) { // Heuristic from original code, possibly for specific cases
                paren_expr_len = argc - pos;
                break;
            }
        }

        result = posixtest(paren_expr_len);

        if (pos >= argc || !streq(argv[pos], S_RPAREN)) { // Should be ")"
            test_syntax_error(_("%s expected, found %s"),
                              quote_n(0, S_RPAREN),
                              quote_n(1, (pos < argc ? argv[pos] : "(end of arguments)")));
        }
        advance(false); // Advance past ")"
    } else {
        bool result_determined = false;

        // Check for -l BINOP ARG (4 arguments: -l ARG OP ARG)
        if (argc - pos > 3 && streq(argv[pos], "-l")) {
            int op_code = binop(argv[pos + 2]);
            if (op_code != -1) {
                result = binary_operator(true, op_code);
                result_determined = true;
            }
        }
        
        if (!result_determined) {
            // Check for ARG BINOP ARG (3 arguments: ARG OP ARG)
            if (argc - pos > 2) {
                int op_code = binop(argv[pos + 1]);
                if (op_code != -1) {
                    result = binary_operator(false, op_code);
                    result_determined = true;
                }
            }
        }

        if (!result_determined) {
            // Check for unary operator or simple string (2 or 1 argument)
            if (strlen(argv[pos]) == 2 && argv[pos][0] == '-') {
                result = unary_operator();
            } else {
                // Simple string: true if non-empty, false if empty
                result = (argv[pos][0] != '\0');
                advance(false); // Consume the string argument
            }
        }
    }
    return negated != result; // XOR for negation
}

// Function: binary_operator
static bool binary_operator(bool param_1_dash_l_flag, int param_2_op_code) {
    bool rhs_has_dash_l = false;
    int arg_idx_left = pos;
    int arg_idx_op = pos + 1;
    int arg_idx_right = pos + 2;

    if (param_1_dash_l_flag) {
        advance(false); // Consume the initial "-l"
        arg_idx_left++;
        arg_idx_op++;
        arg_idx_right++;
    }

    // Check for "-l" on the RHS
    if (arg_idx_op + 1 < argc && streq(argv[arg_idx_op + 1], "-l")) {
        rhs_has_dash_l = true;
    }

    if (rhs_has_dash_l) {
        advance(false); // Consume the "-l" on RHS
        arg_idx_op++; // Operator moves one more if RHS has -l
        arg_idx_right++;
    }

    pos = arg_idx_right + 1; // Advance past operator and both operands

    switch (param_2_op_code) {
        case 0: // =
            return streq(argv[arg_idx_left], argv[arg_idx_right]);
        case 1: // !=
            return !streq(argv[arg_idx_left], argv[arg_idx_right]);
        case 2: // >
            return strcoll(argv[arg_idx_left], argv[arg_idx_right]) > 0;
        case 3: // <
            return strcoll(argv[arg_idx_left], argv[arg_idx_right]) < 0;

        case 4: // -ge (>=)
        case 5: // -le (<=)
        case 6: // -eq (==)
        case 7: // -ne (!=)
        case 8: // -gt (>)
        case 9: // -lt (<)
        {
            char buf1[32], buf2[32]; // Buffers for umaxtostr
            const char *val1_str;
            const char *val2_str;

            if (param_1_dash_l_flag) {
                val1_str = umaxtostr(strlen(argv[arg_idx_left]), buf1);
            } else {
                val1_str = find_int(argv[arg_idx_left]);
            }

            if (rhs_has_dash_l) {
                val2_str = umaxtostr(strlen(argv[arg_idx_right]), buf2);
            } else {
                val2_str = find_int(argv[arg_idx_right]);
            }

            int cmp_result = strintcmp(val1_str, val2_str);
            switch (param_2_op_code) {
                case 4: return cmp_result >= 0; // -ge
                case 5: return cmp_result <= 0; // -le
                case 6: return cmp_result == 0; // -eq
                case 7: return cmp_result != 0; // -ne
                case 8: return cmp_result > 0;  // -gt
                case 9: return cmp_result < 0;  // -lt
            }
            break; // Should not be reached
        }
        case 10: // -ef
            if (param_1_dash_l_flag || rhs_has_dash_l) {
                test_syntax_error(_("-ef does not accept -l"));
            }
            struct stat st1, st2;
            if (stat(argv[arg_idx_left], &st1) == 0 &&
                stat(argv[arg_idx_right], &st2) == 0) {
                return psame_inode(&st1, &st2);
            }
            return false;
        case 11: // -nt (newer than)
        case 12: // -ot (older than)
            if (param_1_dash_l_flag || rhs_has_dash_l) {
                test_syntax_error(_("%s does not accept -l"), argv[arg_idx_op]);
            }
            struct timespec ts1 = get_mtime(argv[arg_idx_left]);
            struct timespec ts2 = get_mtime(argv[arg_idx_right]);
            int cmp_time_result = timespec_cmp(ts1, ts2);
            if (param_2_op_code == 11) { // -nt
                return cmp_time_result > 0;
            } else { // -ot
                return cmp_time_result < 0;
            }
        default:
            __assert_fail("0", __FILE__, __LINE__, __func__);
    }
    return false; // Should not be reached
}

// Function: unary_operator
static bool unary_operator(void) {
    char operator_char = argv[pos][1];
    unary_advance(); // Consume operator and operand. pos is now past operand.
    const char *operand = argv[pos - 1]; // Operand is at pos - 1

    struct stat st;
    bool stat_ok;
    int access_result;
    long fd_val;

    switch (operator_char) {
        case 'G': // -G FILE exists and is owned by the effective group ID
            stat_ok = (stat(operand, &st) == 0);
            if (stat_ok) {
                errno = 0;
                gid_t egid = getegid();
                if (egid == (gid_t)-1 && errno != 0) return false;
                return egid == st.st_gid;
            }
            return false;
        case 'L': // -L FILE exists and is a symbolic link (same as -h)
        case 'h': // -h FILE exists and is a symbolic link
            return issymlink(operand);
        case 'N': // -N FILE exists and has been modified since it was last read
            stat_ok = (stat(operand, &st) == 0);
            if (stat_ok) {
                return timespec_cmp(get_stat_mtime(&st), get_stat_atime(&st)) > 0;
            }
            return false;
        case 'O': // -O FILE exists and is owned by the effective user ID
            stat_ok = (stat(operand, &st) == 0);
            if (stat_ok) {
                errno = 0;
                uid_t euid = geteuid();
                if (euid == (uid_t)-1 && errno != 0) return false;
                return euid == st.st_uid;
            }
            return false;
        case 'S': // -S FILE exists and is a socket
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && S_ISSOCK(st.st_mode);
        case 'b': // -b FILE exists and is block special
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && S_ISBLK(st.st_mode);
        case 'c': // -c FILE exists and is character special
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && S_ISCHR(st.st_mode);
        case 'd': // -d FILE exists and is a directory
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && S_ISDIR(st.st_mode);
        case 'e': // -e FILE exists
            return stat(operand, &st) == 0;
        case 'f': // -f FILE exists and is a regular file
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && S_ISREG(st.st_mode);
        case 'g': // -g FILE exists and is set-group-ID
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && (st.st_mode & S_ISGID);
        case 'k': // -k FILE exists and has its sticky bit set
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && (st.st_mode & S_ISVTX);
        case 'n': // -n STRING the length of STRING is nonzero
            return operand[0] != '\0';
        case 'p': // -p FILE exists and is a named pipe
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && S_ISFIFO(st.st_mode);
        case 'r': // -r FILE exists and the user has read access
            access_result = access(operand, R_OK);
            return access_result == 0;
        case 's': // -s FILE exists and has a size greater than zero
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && st.st_size > 0;
        case 't': // -t FD file descriptor FD is opened on a terminal
            fd_val = strtol(find_int(operand), NULL, 10);
            if (errno == ERANGE || fd_val < 0 || fd_val > 0x7fffffff) { // Check for overflow and negative
                return false;
            }
            return isatty((int)fd_val);
        case 'u': // -u FILE exists and its set-user-ID bit is set
            stat_ok = (stat(operand, &st) == 0);
            return stat_ok && (st.st_mode & S_ISUID);
        case 'w': // -w FILE exists and the user has write access
            access_result = access(operand, W_OK);
            return access_result == 0;
        case 'x': // -x FILE exists and the user has execute (or search) access
            access_result = access(operand, X_OK);
            return access_result == 0;
        case 'z': // -z STRING the length of STRING is zero
            return operand[0] == '\0';
        default:
            test_syntax_error(_("%s: unary operator expected"), quote(argv[pos - 2]));
    }
    return false; // Should not be reached
}

// Function: and
static bool and(void) {
    bool result = true;
    while (true) {
        result = term() && result;
        if (argc <= pos) {
            return result;
        }
        if (!streq(argv[pos], S_A)) { // -a
            break;
        }
        advance(false);
    }
    return result;
}

// Function: or
static bool or(void) {
    bool result = false;
    while (true) {
        result = and() || result;
        if (argc <= pos) {
            return result;
        }
        if (!streq(argv[pos], S_OR)) { // -o
            break;
        }
        advance(false);
    }
    return result;
}

// Function: expr
static bool expr(void) {
    if (argc <= pos) {
        beyond();
    }
    return or();
}

// Function: one_argument
static bool one_argument(void) {
    bool result = (argv[pos][0] != '\0');
    pos++; // Consume the argument
    return result;
}

// Function: two_arguments
static bool two_arguments(void) {
    if (streq(argv[pos], S_LPAREN)) { // "("
        advance(false); // Consume "("
        bool result = one_argument();
        return !result;
    } else if (strlen(argv[pos]) == 2 && argv[pos][0] == '-') {
        return unary_operator();
    } else {
        beyond();
    }
    return false; // Should not be reached
}

// Function: three_arguments
static bool three_arguments(void) {
    int op_code = binop(argv[pos + 1]);
    if (op_code != -1) {
        return binary_operator(false, op_code);
    }

    if (streq(argv[pos], S_LPAREN)) { // `( EXPR )` syntax with 3 args
        advance(true); // Consume "("
        bool result = two_arguments();
        return !result; // Original `cVar1 == '\0'` (negation)
    }

    if (streq(argv[pos], S_NOT_PAREN) && streq(argv[pos + 2], S_RPAREN)) { // `! ARG )`
        advance(false); // Consume `!`
        bool result = one_argument(); // Consume `ARG`
        advance(false); // Consume `)`
        return !result; // Negate the result of one_argument
    }
    
    // Check for specific operators which are not binary ops but part of expressions
    if (streq(argv[pos + 1], S_A) || streq(argv[pos + 1], S_OR) ||
        streq(argv[pos + 1], S_LT_INT) || streq(argv[pos + 1], S_GE_INT)) {
        // These are handled by expr(), not a simple three-argument form
        return expr();
    }

    test_syntax_error(_("%s: binary operator expected"), quote(argv[pos + 1]));
    return false; // Should not be reached
}

// Function: posixtest
static bool posixtest(int nargs) {
    if (nargs == 4) {
        if (streq(argv[pos], S_LPAREN)) { // `( ... )`
            advance(true); // Consume "("
            bool result = three_arguments();
            if (pos >= argc || !streq(argv[pos], S_RPAREN)) {
                 test_syntax_error(_("%s expected, found %s"),
                              quote_n(0, S_RPAREN),
                              quote_n(1, (pos < argc ? argv[pos] : "(end of arguments)")));
            }
            advance(false); // Consume ")"
            return result;
        }
        if (streq(argv[pos], S_NOT_PAREN) && streq(argv[pos + 3], S_RPAREN)) { // `! TWO_ARGS )`
            advance(false); // Consume "!"
            bool result = two_arguments(); // Consumes two args
            advance(false); // Consume ")"
            return !result;
        }
    } else if (nargs == 3) {
        return three_arguments();
    } else if (nargs == 2) {
        return two_arguments();
    } else if (nargs == 1) {
        return one_argument();
    } else if (nargs == 0) {
        return false; // An omitted EXPRESSION defaults to false
    }

    if (nargs > 0) {
        return expr();
    }
    
    __assert_fail("0 < nargs", __FILE__, __LINE__, __func__);
    return false; // Should not be reached
}

// Function: usage
static void usage(int status) {
    FILE *out = (status == 0) ? stdout : stderr;
    const char *prog_name = program_invocation_name;

    if (status == 0) {
        fputs(_("Usage: test EXPRESSION\n  or:  test\n  or:  [ EXPRESSION ]\n  or:  [ ]\n  or:  [ OPTION\n"), out);
        fputs(_("Exit with the status determined by EXPRESSION.\n\n"), out);
        fputs(_("      --help        display this help and exit\n"), out);
        fputs(_("      --version     output version information and exit\n"), out);
        fputs(_("\nAn omitted EXPRESSION defaults to false.  Otherwise,\nEXPRESSION is true or false and sets exit status.  It is one of:\n"), out);
        fputs(_("\n  ( EXPRESSION )               EXPRESSION is true\n  ! EXPRESSION                 EXPRESSION is false\n  EXPRESSION1 -a EXPRESSION2   both EXPRESSION1 and EXPRESSION2 are true\n  EXPRESSION1 -o EXPRESSION2   either EXPRESSION1 or EXPRESSION2 is true\n"), out);
        fputs(_("\n  -n STRING            the length of STRING is nonzero\n  STRING               equivalent to -n STRING\n  -z STRING            the length of STRING is zero\n  STRING1 = STRING2    the strings are equal\n  STRING1 != STRING2   the strings are not equal\n  STRING1 > STRING2    STRING1 is greater than STRING2 in the current locale\n  STRING1 < STRING2    STRING1 is less than STRING2 in the current locale\n"), out);
        fputs(_("\n  INTEGER1 -eq INTEGER2   INTEGER1 is equal to INTEGER2\n  INTEGER1 -ge INTEGER2   INTEGER1 is greater than or equal to INTEGER2\n  INTEGER1 -gt INTEGER2   INTEGER1 is greater than INTEGER2\n  INTEGER1 -le INTEGER2   INTEGER1 is less than or equal to INTEGER2\n  INTEGER1 -lt INTEGER2   INTEGER1 is less than INTEGER2\n  INTEGER1 -ne INTEGER2   INTEGER1 is not equal to INTEGER2\n"), out);
        fputs(_("\n  FILE1 -ef FILE2   FILE1 and FILE2 have the same device and inode numbers\n  FILE1 -nt FILE2   FILE1 is newer (modification date) than FILE2\n  FILE1 -ot FILE2   FILE1 is older than FILE2\n"), out);
        fputs(_("\n  -b FILE     FILE exists and is block special\n  -c FILE     FILE exists and is character special\n  -d FILE     FILE exists and is a directory\n  -e FILE     FILE exists\n"), out);
        fputs(_("  -f FILE     FILE exists and is a regular file\n  -g FILE     FILE exists and is set-group-ID\n  -G FILE     FILE exists and is owned by the effective group ID\n  -h FILE     FILE exists and is a symbolic link (same as -L)\n  -k FILE     FILE exists and has its sticky bit set\n"), out);
        fputs(_("  -L FILE     FILE exists and is a symbolic link (same as -h)\n  -N FILE     FILE exists and has been modified since it was last read\n  -O FILE     FILE exists and is owned by the effective user ID\n  -p FILE     FILE exists and is a named pipe\n  -r FILE     FILE exists and the user has read access\n  -s FILE     FILE exists and has a size greater than zero\n"), out);
        fputs(_("  -S FILE     FILE exists and is a socket\n  -t FD       file descriptor FD is opened on a terminal\n  -u FILE     FILE exists and its set-user-ID bit is set\n  -w FILE     FILE exists and the user has write access\n  -x FILE     FILE exists and the user has execute (or search) access\n"), out);
        fputs(_("\nExcept for -h and -L, all FILE-related tests dereference symbolic links.\nBeware that parentheses need to be escaped (e.g., by backslashes) for shells.\nINTEGER may also be -l STRING, which evaluates to the length of STRING.\n"), out);
        fputs(_("\nBinary -a and -o are ambiguous.  Use 'test EXPR1 && test EXPR2'\nor 'test EXPR1 || test EXPR2' instead.\n"), out);
        fputs(_("\n'[' honors --help and --version, but 'test' treats them as STRINGs.\n"), out);
        printf(_("\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell's documentation\nfor details about the options it supports.\n"), _("test and/or ["));
        emit_ancillary_info("test"); // Placeholder for DAT_00102477
    } else {
        fprintf(out, _("Try '%s --help' for more information.\n"), prog_name);
    }
    exit(status);
}

// Function: main
int main(int main_argc, char **main_argv) {
    set_program_name(main_argv[0]);
    setlocale(LC_ALL, ""); // Use LC_ALL for 6
    bindtextdomain("coreutils", "/usr/local/share/locale");
    textdomain("coreutils");
    initialize_exit_failure(2);
    atexit(close_stdout);

    pos = 1;
    argc = main_argc;
    argv = main_argv;

    // Handle --help and --version for 'test' command (not '[')
    if (argc > 1 && streq(argv[1], "--help")) {
        usage(0);
    }
    if (argc > 1 && streq(argv[1], "--version")) {
        // Minimal version output
        printf("%s (GNU coreutils) 9.0\n", program_invocation_name);
        exit(0);
    }

    bool result;
    if (argc < 2) {
        result = false; // An omitted EXPRESSION defaults to false
    } else {
        result = posixtest(argc - 1); // Pass the number of arguments for the expression
        if (pos != argc) {
            test_syntax_error(_("extra argument %s"), quote(argv[pos]));
        }
    }
    return !result; // Exit with 0 if true, 1 if false (original `bVar1 ^ 1`)
}