#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <locale.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>

// --- Global Variables ---
const char *_program_name;
const char *_Version = "1.0";

bool uniform;
bool split;
bool tagged;
bool crown;
int max_width;
char *prefix;
int prefix_full_length;
int prefix_lead_space;
int prefix_length;

extern int optind;
extern char *optarg;

int goal_width;

int tabs;
int other_indent;
int next_char;
int last_line_length;
int prefix_indent;
int first_indent;
int next_prefix_indent;
int in_column;
int out_column;

#define PARABUF_SIZE 16384
char parabuf[PARABUF_SIZE];
char *_wptr;

typedef struct Word {
    char *data;
    int length;
    int space_after;
    unsigned char flags; // Bit 0: opening_punct, Bit 1: sentence_end, Bit 2: closing_punct, Bit 3: forced_break
    int line_indent;
    struct Word *next_line_start;
    long total_cost;
} Word;

#define MAX_WORDS_IN_PARAGRAPH (PARABUF_SIZE / 4) // Conservative estimate
Word words[MAX_WORDS_IN_PARAGRAPH];
Word *word_limit;
Word *unused_word_type;

// --- Stub/Helper functions ---
void emit_stdin_note() {}
void emit_mandatory_arg_note() {}
void emit_ancillary_info(const char *data) {}
void close_stdout() { fflush(stdout); }
void set_program_name(const char *name) { _program_name = name; }
unsigned long xnumtoumax(const char *s, int base, unsigned long min, unsigned long max, const char *err_msg_operand, const char *err_msg_range, int flags, int line_width_arg) {
    char *endptr;
    unsigned long val = strtoul(s, &endptr, base);
    if (*endptr != '\0' || val < min || val > max) {
        error(EXIT_FAILURE, 0, err_msg_operand, s);
    }
    return val;
}
unsigned long xdectoumax(const char *s, unsigned long min, unsigned long max, const char *err_msg_operand, const char *err_msg_range, int flags) {
    return xnumtoumax(s, 10, min, max, err_msg_operand, err_msg_range, flags, 0);
}
const char *quotearg_style(int style, const char *arg) { return arg; }
FILE *rpl_fopen(const char *path, const char *mode) { return fopen(path, mode); }
int rpl_fclose(FILE *fp) { return fclose(fp); }
void error(int status, int errnum, const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s: ", _program_name ? _program_name : "fmt");
    vfprintf(stderr, format, args);
    if (errnum != 0) {
        fprintf(stderr, ": %s", strerror(errnum));
    }
    fprintf(stderr, "\n");
    va_end(args);
    if (status != 0) exit(status);
}
const char *proper_name_lite(const char *s1, const char *s2) { return s1; }
void version_etc(FILE *fp, const char *program_name, const char *package, const char *version, const char *authors, void *data) {
    fprintf(fp, "%s (GNU coreutils) %s\n", package, version);
    fprintf(fp, "Copyright (C) 2024 Free Software Foundation, Inc.\n");
    fprintf(fp, "License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    fprintf(fp, "This is free software: you are free to change and redistribute it.\n");
    fprintf(fp, "There is NO WARRANTY, to the extent permitted by law.\n");
}
void fadvise(FILE *fp, int advice) {}
void write_error();

static struct option const long_options[] =
{
  {"crown-margin", no_argument, NULL, 'c'},
  {"prefix", required_argument, NULL, 'p'},
  {"split-only", no_argument, NULL, 's'},
  {"tagged-paragraph", no_argument, NULL, 't'},
  {"uniform-spacing", no_argument, NULL, 'u'},
  {"width", required_argument, NULL, 'w'},
  {"goal", required_argument, NULL, 'g'},
  {"help", no_argument, NULL, 0x82},
  {"version", no_argument, NULL, 0x83},
  {NULL, 0, NULL, 0}
};

const char DAT_0010d219[] = "fmt";
const char DAT_0010ce54[] = "";
const char DAT_0010d32d[] = "-";
const char DAT_0010d32f[] = "r";
const char DAT_0010d363[] = "fmt";

// --- Function Prototypes ---
void usage(int status);
void set_prefix(char *p);
bool fmt(FILE *fp, const char *filename);
void set_other_indent(bool is_same_para);
bool get_paragraph(FILE *fp);
int copy_rest(FILE *fp, int c);
bool same_para(int c);
int get_line(FILE *fp, int c);
int get_prefix(FILE *fp);
int get_space(FILE *fp, int c);
void check_punctuation(Word *w);
void flush_paragraph(void);
void fmt_paragraph(void);
long base_cost(Word *w);
long line_cost(long next_word_ptr_val, int current_line_length);
void put_paragraph(Word *split_point);
void put_line(Word *line_start_word, int indent);
void put_word(Word *w);
void put_space(int target_column);

// --- Function Definitions ---
void write_error() { error(EXIT_FAILURE, errno, gettext("write error")); }

void usage(int status) {
  const char *program_name_str = _program_name;

  if (status == 0) {
    printf(gettext("Usage: %s [-WIDTH] [OPTION]... [FILE]...\n"), program_name_str);
    fputs_unlocked(gettext("Reformat each paragraph in the FILE(s), writing to standard output.\nThe option -WIDTH is an abbreviated form of --width=DIGITS.\n"), stdout);
    emit_stdin_note();
    emit_mandatory_arg_note();
    fputs_unlocked(gettext("  -c, --crown-margin        preserve indentation of first two lines\n  -p, --prefix=STRING       reformat only lines beginning with STRING,\n                              reattaching the prefix to reformatted lines\n  -s, --split-only          split long lines, but do not refill\n"), stdout);
    fputs_unlocked(gettext("  -t, --tagged-paragraph    indentation of first line different from second\n  -u, --uniform-spacing     one space between words, two after sentences\n  -w, --width=WIDTH         maximum line width (default of 75 columns)\n  -g, --goal=WIDTH          goal width (default of 93% of width)\n"), stdout);
    fputs_unlocked(gettext("      --help        display this help and exit\n"), stdout);
    fputs_unlocked(gettext("      --version     output version information and exit\n"), stdout);
    emit_ancillary_info(DAT_0010d219);
  } else {
    fprintf(stderr, gettext("Try \'%s --help\' for more information.\n"), program_name_str);
  }
  exit(status);
}

int main(int argc, char *argv[]) {
  bool ok = true;
  const char *width_arg = NULL;
  const char *goal_arg = NULL;

  set_program_name(argv[0]);
  setlocale(LC_ALL, "");
  bindtextdomain("coreutils", "/usr/local/share/locale");
  textdomain("coreutils");
  atexit(close_stdout);

  uniform = false;
  split = false;
  tagged = false;
  crown = false;
  max_width = 75; // 0x4b
  prefix = (char *)DAT_0010ce54; // empty string
  prefix_full_length = 0;
  prefix_lead_space = 0;
  prefix_length = 0;

  _wptr = parabuf;
  unused_word_type = words;
  word_limit = unused_word_type;

  // Handle -WIDTH argument if it's the first option
  if ((argc > 1) && (argv[1][0] == '-') && isdigit((unsigned char)argv[1][1])) {
    width_arg = argv[1] + 1;
    argv[1] = argv[0];
    argc--;
    argv++;
  }

  int opt;
  while ((opt = getopt_long(argc, argv, "0123456789cstuw:p:g:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'c': crown = true; break;
      case 'p': set_prefix(optarg); break;
      case 's': split = true; break;
      case 't': tagged = true; break;
      case 'u': uniform = true; break;
      case 'w': width_arg = optarg; break;
      case 'g': goal_arg = optarg; break;
      case 0x82: // --help
        usage(0);
      case 0x83: // --version
        version_etc(stdout, DAT_0010d219, "GNU coreutils", _Version, proper_name_lite("Ross Paterson", "Ross Paterson"), NULL);
        exit(0);
      case '?':
      default:
        if (isdigit((unsigned char)opt)) {
            error(0, 0, gettext("invalid option -- %c; -WIDTH is recognized only when it is the first\noption; use -w N instead"), opt);
        }
        usage(1);
    }
  }

  if (width_arg != NULL) {
    max_width = xnumtoumax(width_arg, 10, 0, 0x9c4, DAT_0010ce54, gettext("invalid width"), 0, 8);
  }

  if (goal_arg == NULL) {
    goal_width = (max_width * 93) / 100;
  } else {
    goal_width = xdectoumax(goal_arg, 0, max_width, DAT_0010ce54, gettext("invalid width"), 0);
    if (width_arg == NULL) {
      max_width = goal_width + 10;
    }
  }

  bool stdin_processed = false;
  if (optind == argc) {
    stdin_processed = true;
    ok = fmt(stdin, DAT_0010d32d);
  } else {
    for (; optind < argc; optind++) {
      const char *filename = argv[optind];
      if (strcmp(filename, DAT_0010d32d) == 0) {
        ok &= fmt(stdin, filename);
        stdin_processed = true;
      } else {
        FILE *fp = rpl_fopen(filename, DAT_0010d32f);
        if (fp == NULL) {
          error(0, errno, gettext("cannot open %s for reading"), quotearg_style(4, filename));
          ok = false;
        } else {
          ok &= fmt(fp, filename);
        }
      }
    }
  }

  if (stdin_processed && rpl_fclose(stdin) != 0) {
    error(1, errno, DAT_0010d363, gettext("closing standard input"));
  }

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

void set_prefix(char *p) {
  prefix_lead_space = 0;
  char *start = p;
  while (*start == ' ') {
    prefix_lead_space++;
    start++;
  }
  prefix = start;
  size_t full_len = strlen(start);
  prefix_full_length = (int)full_len;

  char *end = start + full_len;
  while (start < end && end[-1] == ' ') {
    end--;
  }
  *end = '\0';
  prefix_length = (int)(end - start);
}

bool fmt(FILE *fp, const char *filename) {
  fadvise(fp, 2);
  tabs = 0;
  other_indent = 0;
  next_char = get_prefix(fp);

  while (get_paragraph(fp)) {
    fmt_paragraph();
    put_paragraph(word_limit);
  }

  bool ok = true;
  if (ferror_unlocked(fp) != 0) {
    ok = false;
  }

  if (fp == stdin) {
    clearerr_unlocked(fp);
  } else {
    if (rpl_fclose(fp) != 0) {
      error(0, errno, gettext("closing %s"), quotearg_style(4, filename));
      ok = false;
    }
  }

  if (!ok) {
    if (fp == stdin) {
      error(0, 0, gettext("read error"));
    } else {
      error(0, 0, gettext("error reading %s"), quotearg_style(4, filename));
    }
  }
  return ok;
}

void set_other_indent(bool is_same_para) {
  if (split) {
    other_indent = first_indent;
  } else if (crown) {
    other_indent = first_indent;
    if (is_same_para) {
      other_indent = in_column;
    }
  } else if (tagged) {
    if (!is_same_para || in_column == first_indent) {
      if (other_indent == first_indent) {
        other_indent = (first_indent == 0) ? 3 : 0;
      }
    } else {
      other_indent = in_column;
    }
  } else {
    other_indent = first_indent;
  }
}

bool get_paragraph(FILE *fp) {
  last_line_length = 0;
  int current_char = next_char;

  while ((current_char == '\n' || current_char == EOF) ||
         (next_prefix_indent < prefix_lead_space) ||
         (in_column < next_prefix_indent + prefix_full_length)) {
    if (copy_rest(fp, current_char) == EOF) {
      next_char = EOF;
      return false;
    }
    putchar_unlocked('\n');
    current_char = get_prefix(fp);
  }

  prefix_indent = next_prefix_indent;
  first_indent = in_column;
  _wptr = parabuf;
  word_limit = unused_word_type;

  current_char = get_line(fp, current_char);
  bool is_same_para_val = same_para(current_char);
  set_other_indent(is_same_para_val);

  if (!split) {
    if (!crown) {
      if (!tagged) {
        while (same_para(current_char) && (in_column == other_indent)) {
          current_char = get_line(fp, current_char);
        }
      } else {
        if (same_para(current_char) && (in_column != first_indent)) {
          do {
            current_char = get_line(fp, current_char);
          } while (same_para(current_char) && (in_column == other_indent));
        }
      }
    } else {
      if (same_para(current_char)) {
        do {
          current_char = get_line(fp, current_char);
        } while (same_para(current_char) && (in_column == other_indent));
      }
    }
  }

  if (word_limit > unused_word_type) {
    (word_limit - 1)->flags |= (1 << 3);
    if (((word_limit - 1)->flags >> 3) & 1) {
      (word_limit - 1)->flags |= (1 << 1);
    } else {
      (word_limit - 1)->flags &= ~(1 << 1);
    }
  }

  next_char = current_char;
  return true;
}

int copy_rest(FILE *fp, int c) {
  out_column = 0;
  if ((next_prefix_indent < in_column) || (c != '\n' && c != EOF)) {
    put_space(next_prefix_indent);
    const char *p = prefix;
    while (out_column != in_column && *p != '\0') {
      putchar_unlocked((int)*p);
      out_column++;
      p++;
    }
    if (c != EOF && c != '\n') {
      put_space(in_column - out_column);
    }
    if (c == EOF && next_prefix_indent + prefix_length <= in_column) {
      putchar_unlocked('\n');
    }
  }
  while (c != '\n' && c != EOF) {
    putchar_unlocked(c);
    c = getc_unlocked(fp);
  }
  return c;
}

bool same_para(int c) {
  return (next_prefix_indent == prefix_indent &&
          next_prefix_indent + prefix_full_length <= in_column &&
          c != '\n' && c != EOF);
}

int get_line(FILE *fp, int c) {
  int current_char = c;
  Word *current_word;

  do {
    if (word_limit == &words[MAX_WORDS_IN_PARAGRAPH]) {
      set_other_indent(true);
      flush_paragraph();
    }
    current_word = word_limit;
    current_word->data = _wptr;

    do {
      if (_wptr == parabuf + PARABUF_SIZE) {
        set_other_indent(true);
        flush_paragraph();
      }
      *_wptr++ = (char)current_char;
      current_char = getc_unlocked(fp);
      if (current_char == EOF) break;
    } while (!isspace((unsigned char)current_char));

    current_word->length = (int)(_wptr - current_word->data);
    in_column += current_word->length;
    check_punctuation(current_word);

    int old_in_column = in_column;
    current_char = get_space(fp, current_char);
    current_word->space_after = in_column - old_in_column;

    bool ends_line_or_paragraph = (current_char == EOF || current_char == '\n' || uniform);
    bool long_line_break = (current_word->flags & (1 << 1)) != 0 && (current_char == '\n' || current_word->space_after > 1);

    current_word->flags = (current_word->flags & ~(1 << 3)) | ((ends_line_or_paragraph || long_line_break) ? (1 << 3) : 0);

    if (uniform) {
      current_word->space_after = ((current_word->flags & (1 << 1)) == 0) ? 1 : 2;
    }

    word_limit++;

    if (current_char == '\n' || current_char == EOF) {
      get_prefix(fp);
      return current_char;
    }
  } while (true);
}

int get_prefix(FILE *fp) {
  in_column = 0;
  int c = getc_unlocked(fp);
  c = get_space(fp, c);

  if (prefix_length == 0) {
    next_prefix_indent = prefix_lead_space;
    if (in_column <= prefix_lead_space) {
      next_prefix_indent = in_column;
    }
  } else {
    next_prefix_indent = in_column;
    for (const char *p = prefix; *p != '\0'; p++) {
      if (c != *p) {
        return c;
      }
      in_column++;
      c = getc_unlocked(fp);
    }
    c = get_space(fp, c);
  }
  return c;
}

int get_space(FILE *fp, int c) {
  int current_char = c;
  while (true) {
    if (current_char == ' ') {
      in_column++;
    } else if (current_char == '\t') {
      tabs = 1;
      in_column = ((in_column + 7) / 8) * 8;
    } else {
      return current_char;
    }
    current_char = getc_unlocked(fp);
  }
}

void check_punctuation(Word *w) {
  char *word_start = w->data;
  char *word_end = w->data + w->length - 1;
  unsigned char last_char = *word_end;

  w->flags = (w->flags & ~(1 << 0)) | (strchr("([\'`\"", (int)*word_start) != NULL);
  w->flags = (w->flags & ~(1 << 2)) | (ispunct((unsigned char)last_char) ? (1 << 2) : 0);

  char *p = word_end;
  while (word_start < p && strchr(")]\'\"", (int)*p) != NULL) {
    p--;
  }

  w->flags = (w->flags & ~(1 << 1)) | (strchr(".?!", (int)*p) != NULL ? (1 << 1) : 0);
}

void flush_paragraph(void) {
  if (word_limit == unused_word_type) {
    size_t data_len = _wptr - parabuf;
    size_t written_len = fwrite_unlocked(parabuf, 1, data_len, stdout);
    if (data_len != written_len) {
      write_error();
    }
    _wptr = parabuf;
  } else {
    fmt_paragraph();

    Word *best_split_word = word_limit;
    long min_cost_diff = -1;

    for (Word *w = unused_word_type; w < word_limit; w++) {
        long current_cost_diff = w->total_cost - (w->next_line_start ? w->next_line_start->total_cost : 0);
        if (best_split_word == word_limit || current_cost_diff < min_cost_diff) {
            min_cost_diff = current_cost_diff;
            best_split_word = w;
        }
    }

    put_paragraph(best_split_word);

    size_t bytes_to_move = (char*)_wptr - best_split_word->data;
    memmove(parabuf, best_split_word->data, bytes_to_move);
    _wptr = parabuf + bytes_to_move;

    long word_data_offset = (long)best_split_word->data - (long)parabuf;
    for (Word *w = best_split_word; w < word_limit; w++) {
        w->data = (char*)((long)w->data - word_data_offset);
    }

    size_t words_to_move = word_limit - best_split_word;
    memmove(unused_word_type, best_split_word, words_to_move * sizeof(Word));
    word_limit = unused_word_type + words_to_move;
  }
}

void fmt_paragraph(void) {
  word_limit->next_line_start = NULL;
  word_limit->length = max_width;
  word_limit->total_cost = 0;

  for (Word *current_word = word_limit - 1; current_word >= unused_word_type; current_word--) {
    long min_total_cost_for_suffix = -1;
    current_word->next_line_start = NULL;
    current_word->line_indent = 0;

    int current_line_indent_start;
    if (current_word == unused_word_type) {
      current_line_indent_start = first_indent;
    } else {
      current_line_indent_start = other_indent;
    }

    int line_length_so_far = current_line_indent_start;
    Word *line_end_word_candidate = current_word;

    do {
      line_length_so_far += line_end_word_candidate->length;
      Word *next_word_after_line = line_end_word_candidate + 1;

      long cost_of_current_line = line_cost((long)next_word_after_line, line_length_so_far);
      long cost_of_suffix = next_word_after_line->total_cost;

      long total_candidate_cost = cost_of_current_line + cost_of_suffix;

      if (current_word == unused_word_type && last_line_length > 0) {
        long diff = line_length_so_far - last_line_length;
        total_candidate_cost += (diff * diff * 100) / 2;
      }

      if (min_total_cost_for_suffix == -1 || total_candidate_cost < min_total_cost_for_suffix) {
        min_total_cost_for_suffix = total_candidate_cost;
        current_word->next_line_start = next_word_after_line;
        current_word->line_indent = line_length_so_far;
      }

      if (line_end_word_candidate < word_limit -1) {
         line_length_so_far += line_end_word_candidate->space_after;
      }
      line_end_word_candidate++;

    } while (line_end_word_candidate <= word_limit && line_length_so_far < max_width);

    current_word->total_cost = min_total_cost_for_suffix + base_cost(current_word);
  }
}

long base_cost(Word *w) {
  long cost = 0x1324;

  if (w > unused_word_type) {
    Word *prev_w = w - 1;
    if (!(prev_w->flags & (1 << 1))) { // If previous word is NOT sentence_end
      if (!(prev_w->flags & (1 << 2))) { // If previous word is NOT closing_punct
        if (w > unused_word_type + 1 && ((w - 2)->flags & (1 << 3))) {
          cost += 40000L / (prev_w->space_after + 2);
        }
      } else { // Previous word is closing_punct
        cost = 0xce4;
      }
    } else if (!(prev_w->flags & (1 << 3))) { // Previous word IS sentence_end, but NOT forced_break
      cost = 0x59164;
    } else { // Previous word IS sentence_end AND forced_break
      cost = 0x960;
    }
  }

  if (!(w->flags & (1 << 0))) { // If current word is NOT opening_punct
    if ((w->flags & (1 << 3))) { // If current word IS forced_break
      cost += 0x57e4L / (w->length + 2);
    }
  } else { // Current word IS opening_punct
    cost += -0x640;
  }
  return cost;
}

long line_cost(long next_word_ptr_val, int current_line_length) {
  Word *next_word = (Word*)next_word_ptr_val;
  if (next_word == word_limit) {
    return 0;
  } else {
    long cost = (long)(goal_width - current_line_length) * (goal_width - current_line_length) * 100;

    if (next_word->next_line_start != NULL) {
      int diff_from_max_width = current_line_length - next_word->line_indent;
      cost += (long)diff_from_max_width * diff_from_max_width * 100 / 2;
    }
    return cost;
  }
}

void put_paragraph(Word *split_point) {
  Word *current_line_start = unused_word_type;
  put_line(current_line_start, first_indent);
  current_line_start = current_line_start->next_line_start;

  while (current_line_start != split_point) {
    put_line(current_line_start, other_indent);
    current_line_start = current_line_start->next_line_start;
  }
}

void put_line(Word *line_start_word, int indent) {
  out_column = 0;
  put_space(prefix_indent);
  fputs_unlocked(prefix, stdout);
  out_column += prefix_length;
  put_space(indent);

  Word *current_word = line_start_word;
  Word *next_line_start = line_start_word->next_line_start;

  while (current_word != next_line_start) {
    put_word(current_word);
    if (current_word + 1 != next_line_start) {
        put_space(out_column + current_word->space_after);
    }
    current_word++;
  }
  last_line_length = out_column;
  putchar_unlocked('\n');
  if (ferror_unlocked(stdout) != 0) {
    write_error();
  }
}

void put_word(Word *w) {
  char *p = w->data;
  for (int i = 0; i < w->length; i++) {
    putchar_unlocked((int)*p++);
  }
  out_column += w->length;
}

void put_space(int target_column) {
  if (target_column <= out_column) {
      return;
  }

  if (tabs) {
    int current_tab_stop;
    if (out_column < 0) {
      current_tab_stop = ((out_column + 7) / 8) * 8;
    } else {
      current_tab_stop = ((out_column / 8) + 1) * 8;
    }

    while (out_column < target_column && out_column < current_tab_stop) {
      putchar_unlocked('\t');
      out_column = ((out_column + 7) / 8) * 8;
    }
  }

  while (out_column < target_column) {
    putchar_unlocked(' ');
    out_column++;
  }
}