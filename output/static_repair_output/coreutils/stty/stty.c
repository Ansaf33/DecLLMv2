// Function: wrapf
void wrapf(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
          undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,char *param_9,
          undefined8 param_10,undefined8 param_11,undefined8 param_12,undefined8 param_13,
          undefined8 param_14) {
  char in_AL;
  int iVar1;
  long in_FS_OFFSET;
  char *local_e0;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined *local_d0;
  undefined *local_c8;
  long local_c0;
  undefined local_b8 [8];
  undefined8 local_b0;
  undefined8 local_a8;
  undefined8 local_a0;
  undefined8 local_98;
  undefined8 local_90;
  undefined4 local_88;
  undefined4 local_78;
  undefined4 local_68;
  undefined4 local_58;
  undefined4 local_48;
  undefined4 local_38;
  undefined4 local_28;
  undefined4 local_18;
  
  if (in_AL != '\0') {
    local_88 = param_1;
    local_78 = param_2;
    local_68 = param_3;
    local_58 = param_4;
    local_48 = param_5;
    local_38 = param_6;
    local_28 = param_7;
    local_18 = param_8;
  }
  local_c0 = *(long *)(in_FS_OFFSET + 0x28);
  local_d8 = 8;
  local_d4 = 0x30;
  local_d0 = &stack0x00000008;
  local_c8 = local_b8;
  local_b0 = param_10;
  local_a8 = param_11;
  local_a0 = param_12;
  local_98 = param_13;
  local_90 = param_14;
  iVar1 = vasprintf(&local_e0,param_9,&local_d8);
  if (iVar1 < 0) {
    xalloc_die();
  }
  if (0 < current_col) {
    if (iVar1 < max_col - current_col) {
      putchar_unlocked(0x20);
      current_col = current_col + 1;
    }
    else {
      putchar_unlocked(10);
      current_col = 0;
    }
  }
  fputs_unlocked(local_e0,_stdout);
  free(local_e0);
  current_col = iVar1 + current_col;
  if (local_c0 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  char *pcVar2;
  undefined8 uVar3;
  
  uVar3 = _program_name;
  if (param_1 == 0) {
    pcVar2 = (char *)gettext(
                            "Usage: %s [-F DEVICE | --file=DEVICE] [SETTING]...\n  or:  %s [-F DEVICE | --file=DEVICE] [-a|--all]\n  or:  %s [-F DEVICE | --file=DEVICE] [-g|--save]\n"
                            );
    printf(pcVar2,uVar3,uVar3,uVar3);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("Print or change terminal characteristics.\n");
    fputs_unlocked(pcVar2,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "  -a, --all          print all current settings in human-readable form\n  -g, --save         print all current settings in a stty-readable form\n  -F, --file=DEVICE  open and use DEVICE instead of standard input\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "\nOptional - before SETTING indicates negation.  An * marks non-POSIX\nsettings.  The underlying system defines which settings are available.\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("\nSpecial characters:\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * discard CHAR  CHAR will toggle discarding of output\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   eof CHAR      CHAR will send an end of file (terminate the input)\n   eol CHAR      CHAR will end the line\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * eol2 CHAR     alternate CHAR for ending the line\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   erase CHAR    CHAR will erase the last character typed\n   intr CHAR     CHAR will send an interrupt signal\n   kill CHAR     CHAR will erase the current line\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * lnext CHAR    CHAR will enter the next character quoted\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   quit CHAR     CHAR will send a quit signal\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * rprnt CHAR    CHAR will redraw the current line\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   start CHAR    CHAR will restart the output after stopping it\n   stop CHAR     CHAR will stop the output\n   susp CHAR     CHAR will send a terminal stop signal\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * swtch CHAR    CHAR will switch to a different shell layer\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * werase CHAR   CHAR will erase the last word typed\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "\nSpecial settings:\n   N             set the input and output speeds to N bauds\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   cols N        tell the kernel that the terminal has N columns\n * columns N     same as cols N\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    if (tcsetattr_options == 1) {
      uVar3 = gettext(&DAT_001058d2);
    }
    else {
      uVar3 = gettext(&DAT_001058d5);
    }
    pcVar2 = (char *)gettext(
                            " * [-]drain      wait for transmission before applying settings (%s by default)\n"
                            );
    printf(pcVar2,uVar3);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   ispeed N      set the input speed to N\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * line N        use line discipline N\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   min N         with -icanon, set N characters minimum for a completed read\n   ospeed N      set the output speed to N\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   rows N        tell the kernel that the terminal has N rows\n   size          print the number of rows and columns according to the kernel\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   speed         print the terminal speed\n   time N        with -icanon, set read timeout of N tenths of a second\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "\nControl settings:\n   [-]clocal     disable modem control signals\n   [-]cread      allow input to be received\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]crtscts    enable RTS/CTS handshaking\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   csN           set character size to N bits, N in [5..8]\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   [-]cstopb     use two stop bits per character (one with \'-\')\n   [-]hup        send a hangup signal when the last process closes the tty\n   [-]hupcl      same as [-]hup\n   [-]parenb     generate parity bit in output and expect parity bit in input\n   [-]parodd     set odd parity (or even parity with \'-\')\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]cmspar     use \"stick\" (mark/space) parity\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "\nInput settings:\n   [-]brkint     breaks cause an interrupt signal\n   [-]icrnl      translate carriage return to newline\n   [-]ignbrk     ignore break characters\n   [-]igncr      ignore carriage return\n   [-]ignpar     ignore characters with parity errors\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]imaxbel    beep and do not flush a full input buffer on a character\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   [-]inlcr      translate newline to carriage return\n   [-]inpck      enable input parity checking\n   [-]istrip     clear high (8th) bit of input characters\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]iutf8      assume input characters are UTF-8 encoded\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]iuclc      translate uppercase characters to lowercase\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]ixany      let any character restart output, not only start character\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   [-]ixoff      enable sending of start/stop characters\n   [-]ixon       enable XON/XOFF flow control\n   [-]parmrk     mark parity errors (with a 255-0-character sequence)\n   [-]tandem     same as [-]ixoff\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("\nOutput settings:\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * bsN           backspace delay style, N in [0..1]\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * crN           carriage return delay style, N in [0..3]\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * ffN           form feed delay style, N in [0..1]\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * nlN           newline delay style, N in [0..1]\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]ocrnl      translate carriage return to newline\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]ofdel      use delete characters for fill instead of NUL characters\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]ofill      use fill (padding) characters instead of timing for delays\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]olcuc      translate lowercase characters to uppercase\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]onlcr      translate newline to carriage return-newline\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]onlret     newline performs a carriage return\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]onocr      do not print carriage returns in the first column\n")
    ;
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   [-]opost      postprocess output\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * tabN          horizontal tab delay style, N in [0..3]\n * tabs          same as tab0\n * -tabs         same as tab3\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * vtN           vertical tab delay style, N in [0..1]\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "\nLocal settings:\n   [-]crterase   echo erase characters as backspace-space-backspace\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * crtkill       kill all line by obeying the echoprt and echoe settings\n * -crtkill      kill all line by obeying the echoctl and echok settings\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]ctlecho    echo control characters in hat notation (\'^c\')\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   [-]echo       echo input characters\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]echoctl    same as [-]ctlecho\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   [-]echoe      same as [-]crterase\n   [-]echok      echo a newline after a kill character\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]echoke     same as [-]crtkill\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   [-]echonl     echo newline even if not echoing other characters\n")
    ;
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]echoprt    echo erased characters backward, between \'\\\' and \'/\'\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]extproc    enable \"LINEMODE\"; useful with high latency links\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]flusho     discard output\n");
    fputs_unlocked(pcVar2,pFVar1);
    pcVar2 = (char *)gettext(
                            "   [-]icanon     enable special characters: %s\n   [-]iexten     enable non-POSIX special characters\n"
                            );
    printf(pcVar2,"erase, kill, werase, rprnt");
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   [-]isig       enable interrupt, quit, and suspend special characters\n   [-]noflsh     disable flushing after interrupt and quit special characters\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]prterase   same as [-]echoprt\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]tostop     stop background jobs that try to write to the terminal\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            " * [-]xcase      with icanon, escape with \'\\\' for uppercase characters\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("\nCombination settings:\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]LCASE      same as [-]lcase\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext("   cbreak        same as -icanon\n   -cbreak       same as icanon\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   cooked        same as brkint ignpar istrip icrnl ixon opost isig\n                 icanon, eof and eol characters to their default values\n   -cooked       same as raw\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pcVar2 = (char *)gettext("   crt           same as %s\n");
    printf(pcVar2,"echoe echoctl echoke");
    pcVar2 = (char *)gettext(
                            "   dec           same as %s intr ^c erase 0177\n                 kill ^u\n"
                            );
    printf(pcVar2,"echoe echoctl echoke -ixany");
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]decctlq    same as [-]ixany\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   ek            erase and kill characters to their default values\n   evenp         same as parenb -parodd cs7\n   -evenp        same as -parenb cs8\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(" * [-]lcase      same as xcase iuclc olcuc\n");
    fputs_unlocked(pcVar2,pFVar1);
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   litout        same as -parenb -istrip -opost cs8\n   -litout       same as parenb istrip opost cs7\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pcVar2 = (char *)gettext("   nl            same as %s\n   -nl           same as %s\n");
    printf(pcVar2,"-icrnl -onlcr","icrnl -inlcr -igncr onlcr -ocrnl -onlret");
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "   oddp          same as parenb parodd cs7\n   -oddp         same as -parenb cs8\n   [-]parity     same as [-]evenp\n   pass8         same as -parenb -istrip cs8\n   -pass8        same as parenb istrip cs7\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    pcVar2 = (char *)gettext(
                            "   raw           same as -ignbrk -brkint -ignpar -parmrk -inpck -istrip\n                 -inlcr -igncr -icrnl -ixon -ixoff -icanon -opost\n                 -isig%s min 1 time 0\n   -raw          same as cooked\n"
                            );
    printf(pcVar2," -iuclc -ixany -imaxbel -xcase");
    pcVar2 = (char *)gettext(
                            "   sane          same as cread -ignbrk brkint -inlcr -igncr icrnl\n                 icanon iexten echo echoe echok -echonl -noflsh\n                 %s\n                 %s\n                 %s,\n                 all special characters to their default values\n"
                            );
    printf(pcVar2,"-ixoff -iutf8 -iuclc -ixany imaxbel -xcase -olcuc -ocrnl",
           "opost -ofill onlcr -onocr -onlret nl0 cr0 tab0 bs0 vt0 ff0",
           "isig -tostop -ofdel -echoprt echoctl echoke -extproc -flusho");
    pFVar1 = _stdout;
    pcVar2 = (char *)gettext(
                            "\nHandle the tty line connected to standard input.  Without arguments,\nprints baud rate, line discipline, and deviations from stty sane.  In\nsettings, CHAR is taken literally, or coded as in ^c, 0x37, 0177 or\n127; special values ^- or undef used to disable special characters.\n"
                            );
    fputs_unlocked(pcVar2,pFVar1);
    emit_ancillary_info(&DAT_001071d5);
  }
  else {
    pcVar2 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar2,uVar3);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: apply_settings
void apply_settings(char param_1,undefined8 param_2,long param_3,int param_4,long param_5,
                   undefined *param_6) {
  bool bVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  undefined8 uVar5;
  undefined8 uVar6;
  ulong uVar7;
  bool bVar8;
  char local_e3;
  int local_e0;
  int local_dc;
  char *local_80;
  
  local_e0 = 1;
  do {
    if (param_4 <= local_e0) {
      if (param_1 != '\0') {
        check_speed(param_5);
      }
      return;
    }
    local_80 = *(char **)(param_3 + (long)local_e0 * 8);
    local_e3 = '\0';
    bVar1 = false;
    if (local_80 != (char *)0x0) {
      bVar8 = *local_80 == '-';
      if (bVar8) {
        local_80 = local_80 + 1;
      }
      cVar2 = streq(local_80,"drain");
      if (cVar2 == '\0') {
        local_dc = 0;
        while (*(long *)(mode_info + (long)local_dc * 0x20) != 0) {
          cVar2 = streq(local_80,*(undefined8 *)(mode_info + (long)local_dc * 0x20));
          if (cVar2 != '\0') {
            if ((mode_info[(long)local_dc * 0x20 + 0xc] & 0x10) == 0) {
              local_e3 = set_mode(mode_info + (long)local_dc * 0x20,bVar8,param_5);
              *param_6 = 1;
            }
            else {
              bVar1 = true;
              local_e3 = '\x01';
            }
            break;
          }
          local_dc = local_dc + 1;
        }
        if ((local_e3 != '\x01') && (bVar8)) {
          uVar5 = quote(local_80 + -1);
          uVar6 = gettext("invalid argument %s");
          error(0,0,uVar6,uVar5);
          usage(1);
        }
        if (local_e3 != '\x01') {
          local_dc = 0;
          while (*(long *)(control_info + (long)local_dc * 0x18) != 0) {
            cVar2 = streq(local_80,*(undefined8 *)(control_info + (long)local_dc * 0x18));
            if (cVar2 != '\0') {
              if ((local_e0 == param_4 + -1) || (*(long *)(param_3 + ((long)local_e0 + 1) * 8) == 0)
                 ) {
                uVar5 = quote(local_80);
                uVar6 = gettext("missing argument to %s");
                error(0,0,uVar6,uVar5);
                usage(1);
              }
              local_e3 = '\x01';
              local_e0 = local_e0 + 1;
              set_control_char(control_info + (long)local_dc * 0x18,
                               *(undefined8 *)(param_3 + (long)local_e0 * 8),param_5);
              *param_6 = 1;
              break;
            }
            local_dc = local_dc + 1;
          }
        }
        if ((local_e3 != '\x01') || (bVar1)) {
          cVar2 = streq(local_80,"ispeed");
          if (cVar2 == '\0') {
            cVar2 = streq(local_80,"ospeed");
            if (cVar2 == '\0') {
              cVar2 = streq(local_80,&DAT_0010723d);
              if (cVar2 == '\0') {
                cVar2 = streq(local_80,&DAT_00107242);
                if ((cVar2 == '\0') && (cVar2 = streq(local_80,"columns"), cVar2 == '\0')) {
                  cVar2 = streq(local_80,&DAT_0010724f);
                  if (cVar2 == '\0') {
                    cVar2 = streq(local_80,&DAT_00107254);
                    if (cVar2 == '\0') {
                      cVar2 = streq(local_80,"speed");
                      if (cVar2 == '\0') {
                        iVar3 = string_to_baud(local_80);
                        if (iVar3 == -1) {
                          cVar2 = recover_mode(local_80,param_5);
                          if (cVar2 != '\x01') {
                            uVar5 = quote(local_80);
                            uVar6 = gettext("invalid argument %s");
                            error(0,0,uVar6,uVar5);
                            usage(1);
                          }
                          *param_6 = 1;
                        }
                        else {
                          set_speed(2,local_80,param_5);
                          if (param_1 == '\0') {
                            *param_6 = 1;
                          }
                        }
                      }
                      else if (param_1 == '\0') {
                        max_col = screen_columns();
                        display_speed(param_5,0);
                      }
                    }
                    else {
                      if ((local_e0 == param_4 + -1) ||
                         (*(long *)(param_3 + ((long)local_e0 + 1) * 8) == 0)) {
                        uVar5 = quote(local_80);
                        uVar6 = gettext("missing argument to %s");
                        error(0,0,uVar6,uVar5);
                        usage(1);
                      }
                      local_e0 = local_e0 + 1;
                      uVar7 = integer_arg(*(undefined8 *)(param_3 + (long)local_e0 * 8),
                                          0xffffffffffffffff);
                      *(char *)(param_5 + 0x10) = (char)uVar7;
                      if (uVar7 != (uVar7 & 0xff)) {
                        uVar5 = quote(*(undefined8 *)(param_3 + (long)local_e0 * 8));
                        uVar6 = gettext("invalid line discipline %s");
                        error(0,0x4b,uVar6,uVar5);
                      }
                      *param_6 = 1;
                    }
                  }
                  else if (param_1 == '\0') {
                    max_col = screen_columns();
                    current_col = 0;
                    display_window_size(0,param_2);
                  }
                }
                else {
                  if ((local_e0 == param_4 + -1) ||
                     (*(long *)(param_3 + ((long)local_e0 + 1) * 8) == 0)) {
                    uVar5 = quote(local_80);
                    uVar6 = gettext("missing argument to %s");
                    error(0,0,uVar6,uVar5);
                    usage(1);
                  }
                  local_e0 = local_e0 + 1;
                  if (param_1 == '\0') {
                    uVar4 = integer_arg(*(undefined8 *)(param_3 + (long)local_e0 * 8),0x7fffffff);
                    set_window_size(0xffffffff,uVar4,param_2);
                  }
                }
              }
              else {
                if ((local_e0 == param_4 + -1) ||
                   (*(long *)(param_3 + ((long)local_e0 + 1) * 8) == 0)) {
                  uVar5 = quote(local_80);
                  uVar6 = gettext("missing argument to %s");
                  error(0,0,uVar6,uVar5);
                  usage(1);
                }
                local_e0 = local_e0 + 1;
                if (param_1 == '\0') {
                  uVar4 = integer_arg(*(undefined8 *)(param_3 + (long)local_e0 * 8),0x7fffffff);
                  set_window_size(uVar4,0xffffffff,param_2);
                }
              }
            }
            else {
              if ((local_e0 == param_4 + -1) || (*(long *)(param_3 + ((long)local_e0 + 1) * 8) == 0)
                 ) {
                uVar5 = quote(local_80);
                uVar6 = gettext("missing argument to %s");
                error(0,0,uVar6,uVar5);
                usage(1);
              }
              local_e0 = local_e0 + 1;
              iVar3 = string_to_baud(*(undefined8 *)(param_3 + (long)local_e0 * 8));
              if (iVar3 == -1) {
                uVar5 = quote(*(undefined8 *)(param_3 + (long)local_e0 * 8));
                uVar6 = gettext("invalid ospeed %s");
                error(0,0,uVar6,uVar5);
                usage(1);
              }
              set_speed(1,*(undefined8 *)(param_3 + (long)local_e0 * 8),param_5);
              if (param_1 == '\0') {
                *param_6 = 1;
              }
            }
          }
          else {
            if ((local_e0 == param_4 + -1) || (*(long *)(param_3 + ((long)local_e0 + 1) * 8) == 0))
            {
              uVar5 = quote(local_80);
              uVar6 = gettext("missing argument to %s");
              error(0,0,uVar6,uVar5);
              usage(1);
            }
            local_e0 = local_e0 + 1;
            iVar3 = string_to_baud(*(undefined8 *)(param_3 + (long)local_e0 * 8));
            if (iVar3 == -1) {
              uVar5 = quote(*(undefined8 *)(param_3 + (long)local_e0 * 8));
              uVar6 = gettext("invalid ispeed %s");
              error(0,0,uVar6,uVar5);
              usage(1);
            }
            set_speed(0,*(undefined8 *)(param_3 + (long)local_e0 * 8),param_5);
            if (param_1 == '\0') {
              *param_6 = 1;
            }
          }
        }
      }
      else {
        tcsetattr_options = (uint)!bVar8;
      }
    }
    local_e0 = local_e0 + 1;
  } while( true );
}

// Function: main
undefined8 main(int param_1,undefined8 *param_2) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  int *piVar4;
  undefined8 uVar5;
  undefined *puVar6;
  long in_FS_OFFSET;
  char local_104;
  char local_103;
  char local_102;
  char local_101;
  undefined4 local_100;
  int local_fc;
  int local_f8;
  uint local_f4;
  int local_f0;
  uint local_ec;
  uint local_e8;
  uint local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  int local_c4;
  undefined4 local_c0;
  int local_bc;
  undefined4 local_b8;
  int local_b4;
  undefined4 local_b0;
  int local_ac;
  undefined4 local_a8;
  int local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  long local_88;
  long local_80;
  undefined8 local_78;
  undefined8 local_70;
  undefined8 local_68;
  undefined *local_60;
  undefined8 local_58;
  undefined *local_50;
  undefined *local_48;
  undefined *local_40;
  undefined8 local_38;
  char *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_fc = 0;
  local_f8 = 1;
  local_101 = '\x01';
  local_88 = 0;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  atexit((__func *)&close_stdout);
  local_100 = 0;
  local_103 = '\0';
  local_102 = '\0';
  _opterr = 0;
  do {
    local_f0 = getopt_long(param_1 - local_fc,param_2 + local_fc,"-agF:",longopts,0);
    if (local_f0 == -1) {
      if ((local_103 != '\0') && (local_102 != '\0')) {
        local_70 = gettext(
                          "the options for verbose and stty-readable output styles are\nmutually exclusive"
                          );
        local_d8 = 1;
        local_d4 = 0;
        error(1,0,local_70);
      }
      if ((local_101 != '\x01') && ((local_103 != '\0' || (local_102 != '\0')))) {
        local_68 = gettext("when specifying an output style, modes may not be set");
        local_d0 = 1;
        local_cc = 0;
        error(1,0,local_68);
      }
      local_80 = local_88;
      if (local_88 == 0) {
        local_80 = gettext("standard input");
      }
      if (((local_101 != '\x01') && (local_103 != '\x01')) && (local_102 != '\x01')) {
        apply_settings(1,local_80,param_2,param_1,check_mode_4,&local_104);
      }
      if (local_88 != 0) {
        iVar2 = fd_reopen(0,local_80,0x800,0);
        if (iVar2 < 0) {
          uVar3 = quotearg_n_style_colon(0,3,local_80);
          piVar4 = __errno_location();
          local_c4 = *piVar4;
          local_c8 = 1;
          local_60 = &DAT_0010737d;
          error(1,local_c4,&DAT_0010737d,uVar3);
        }
        local_ec = rpl_fcntl(0,3);
        if ((local_ec == 0xffffffff) || (iVar2 = rpl_fcntl(0,4,local_ec & 0xfffff7ff), iVar2 < 0)) {
          uVar3 = quotearg_n_style_colon(0,3,local_80);
          uVar5 = gettext("%s: couldn\'t reset non-blocking mode");
          piVar4 = __errno_location();
          local_bc = *piVar4;
          local_c0 = 1;
          local_58 = uVar5;
          error(1,local_bc,uVar5,uVar3);
        }
      }
      iVar2 = tcgetattr(0,(termios *)mode_3);
      if (iVar2 != 0) {
        uVar3 = quotearg_n_style_colon(0,3,local_80);
        piVar4 = __errno_location();
        local_b4 = *piVar4;
        local_b8 = 1;
        local_50 = &DAT_0010737d;
        error(1,local_b4,&DAT_0010737d,uVar3);
      }
      if (((local_103 == '\0') && (local_102 == '\0')) && (local_101 == '\0')) {
        local_104 = '\0';
        apply_settings(0,local_80,param_2,param_1,mode_3,&local_104);
        if (local_104 != '\0') {
          iVar2 = tcsetattr(0,tcsetattr_options,(termios *)mode_3);
          if (iVar2 != 0) {
            uVar3 = quotearg_n_style_colon(0,3,local_80);
            piVar4 = __errno_location();
            local_ac = *piVar4;
            local_b0 = 1;
            local_48 = &DAT_0010737d;
            error(1,local_ac,&DAT_0010737d,uVar3);
          }
          iVar2 = tcgetattr(0,(termios *)new_mode_2);
          if (iVar2 != 0) {
            uVar3 = quotearg_n_style_colon(0,3,local_80);
            piVar4 = __errno_location();
            local_a4 = *piVar4;
            local_a8 = 1;
            local_40 = &DAT_0010737d;
            error(1,local_a4,&DAT_0010737d,uVar3);
          }
          cVar1 = eq_mode(mode_3,new_mode_2);
          if (cVar1 != '\x01') {
            if (dev_debug != '\0') {
              local_38 = gettext("indx: mode: actual mode");
              local_a0 = 0;
              local_9c = 0;
              error(0,0,local_38);
              for (local_f4 = 0; local_f4 < 0x3c; local_f4 = local_f4 + 1) {
                local_e8 = (uint)(byte)new_mode_2[local_f4];
                local_e4 = (uint)(byte)mode_3[local_f4];
                if (local_e8 == local_e4) {
                  puVar6 = &DAT_00104f24;
                }
                else {
                  puVar6 = &DAT_001073bd;
                }
                local_98 = 0;
                local_94 = 0;
                local_30 = "0x%02x, 0x%02x: 0x%02x%s";
                error(0,0,"0x%02x, 0x%02x: 0x%02x%s",local_f4,local_e4,local_e8,puVar6);
              }
            }
            uVar3 = quotearg_n_style_colon(0,3,local_80);
            local_28 = gettext("%s: unable to perform all requested operations");
            local_90 = 1;
            local_8c = 0;
            error(1,0,local_28,uVar3);
          }
        }
      }
      else {
        max_col = screen_columns();
        current_col = 0;
        display_settings(local_100,mode_3,local_80);
      }
      if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
        return 0;
      }
                    /* WARNING: Subroutine does not return */
      __stack_chk_fail();
    }
    if (local_f0 == 0x80) {
      dev_debug = '\x01';
    }
    else {
      if (local_f0 < 0x81) {
        if (local_f0 == 0x67) {
          local_102 = '\x01';
          local_100 = 2;
          goto LAB_00101fc5;
        }
        if (local_f0 < 0x68) {
          if (local_f0 == 0x61) {
            local_103 = '\x01';
            local_100 = 1;
            goto LAB_00101fc5;
          }
          if (local_f0 < 0x62) {
            if (local_f0 == 0x46) {
              if (local_88 != 0) {
                local_78 = gettext("only one device may be specified");
                local_e0 = 1;
                local_dc = 0;
                error(1,0,local_78);
              }
              local_88 = _optarg;
              goto LAB_00101fc5;
            }
            if (local_f0 < 0x47) {
              if (local_f0 == -0x83) {
LAB_00101e92:
                uVar3 = proper_name_lite("David MacKenzie","David MacKenzie");
                version_etc(_stdout,&DAT_001071d5,"GNU coreutils",_Version,uVar3,0);
                    /* WARNING: Subroutine does not return */
                exit(0);
              }
              if (local_f0 == -0x82) {
                usage(0);
                goto LAB_00101e92;
              }
            }
          }
        }
      }
      cVar1 = streq(param_2[local_f8 + local_fc],"-drain");
      if ((cVar1 != '\x01') &&
         (cVar1 = streq(param_2[local_f8 + local_fc],"drain"), cVar1 != '\x01')) {
        local_101 = '\0';
      }
      local_fc = local_fc + local_f8;
      local_f8 = 1;
      _optind = 0;
    }
LAB_00101fc5:
    while (local_f8 < _optind) {
      param_2[local_f8 + local_fc] = 0;
      local_f8 = local_f8 + 1;
    }
  } while( true );
}

// Function: eq_mode
undefined4 eq_mode(termios *param_1,termios *param_2) {
  char cVar1;
  speed_t sVar2;
  speed_t sVar3;
  
  if ((((param_1->c_iflag == param_2->c_iflag) && (param_1->c_oflag == param_2->c_oflag)) &&
      (param_1->c_cflag == param_2->c_cflag)) &&
     (((param_1->c_lflag == param_2->c_lflag && (param_1->c_line == param_2->c_line)) &&
      (cVar1 = memeq(param_1->c_cc,param_2->c_cc,0x20), cVar1 != '\0')))) {
    sVar2 = cfgetispeed(param_1);
    sVar3 = cfgetispeed(param_2);
    if (sVar2 == sVar3) {
      sVar2 = cfgetospeed(param_1);
      sVar3 = cfgetospeed(param_2);
      if (sVar2 == sVar3) {
        return 1;
      }
    }
  }
  return 0;
}

// Function: set_mode
undefined8 set_mode(char **param_1,char param_2,uint *param_3) {
  char cVar1;
  uint *puVar2;
  
  if ((param_2 != '\0') && ((*(byte *)((long)param_1 + 0xc) & 4) == 0)) {
    return 0;
  }
  puVar2 = (uint *)mode_type_flag(*(undefined4 *)(param_1 + 1),param_3);
  if (puVar2 == (uint *)0x0) {
    cVar1 = streq(*param_1,"evenp");
    if ((cVar1 == '\0') && (cVar1 = streq(*param_1,"parity"), cVar1 == '\0')) {
      cVar1 = streq(*param_1,&DAT_00105116);
      if (cVar1 == '\0') {
        cVar1 = streq(*param_1,&DAT_0010511b);
        if (cVar1 == '\0') {
          cVar1 = streq(*param_1,&DAT_0010511e);
          if (cVar1 == '\0') {
            cVar1 = streq(*param_1,&DAT_00105121);
            if (cVar1 == '\0') {
              cVar1 = streq(*param_1,"cbreak");
              if (cVar1 == '\0') {
                cVar1 = streq(*param_1,"pass8");
                if (cVar1 == '\0') {
                  cVar1 = streq(*param_1,"litout");
                  if (cVar1 == '\0') {
                    cVar1 = streq(*param_1,&DAT_0010512d);
                    if ((cVar1 == '\0') && (cVar1 = streq(*param_1,"cooked"), cVar1 == '\0')) {
                      cVar1 = streq(*param_1,"decctlq");
                      if (cVar1 == '\0') {
                        cVar1 = streq(*param_1,&DAT_0010514d);
                        if (cVar1 == '\0') {
                          cVar1 = streq(*param_1,"lcase");
                          if ((cVar1 == '\0') && (cVar1 = streq(*param_1,"LCASE"), cVar1 == '\0')) {
                            cVar1 = streq(*param_1,&DAT_0010515e);
                            if (cVar1 == '\0') {
                              cVar1 = streq(*param_1,&DAT_00105162);
                              if (cVar1 != '\0') {
                                *(undefined *)((long)param_3 + 0x11) = 3;
                                *(undefined *)((long)param_3 + 0x13) = 0x7f;
                                *(undefined *)(param_3 + 5) = 0x15;
                                param_3[3] = param_3[3] | 0xa10;
                                *param_3 = *param_3 & 0xfffff7ff;
                              }
                            }
                            else {
                              param_3[3] = param_3[3] | 0xa10;
                            }
                          }
                          else if (param_2 == '\0') {
                            param_3[3] = param_3[3] | 4;
                            *param_3 = *param_3 | 0x200;
                            param_3[1] = param_3[1] | 2;
                          }
                          else {
                            param_3[3] = param_3[3] & 0xfffffffb;
                            *param_3 = *param_3 & 0xfffffdff;
                            param_3[1] = param_3[1] & 0xfffffffd;
                          }
                        }
                        else if (param_2 == '\0') {
                          param_3[1] = param_3[1] & 0xffffe7ff;
                        }
                        else {
                          param_3[1] = param_3[1] | 0x1800;
                        }
                      }
                      else if (param_2 == '\0') {
                        *param_3 = *param_3 & 0xfffff7ff;
                      }
                      else {
                        *param_3 = *param_3 | 0x800;
                      }
                    }
                    else if (((**param_1 == 'r') && (param_2 != '\0')) ||
                            ((**param_1 == 'c' && (param_2 != '\x01')))) {
                      *param_3 = *param_3 | 0x526;
                      param_3[1] = param_3[1] | 1;
                      param_3[3] = param_3[3] | 3;
                    }
                    else {
                      *param_3 = 0;
                      param_3[1] = param_3[1] & 0xfffffffe;
                      param_3[3] = param_3[3] & 0xfffffff8;
                      *(undefined *)((long)param_3 + 0x17) = 1;
                      *(undefined *)((long)param_3 + 0x16) = 0;
                    }
                  }
                  else if (param_2 == '\0') {
                    param_3[2] = param_3[2] & 0xfffffecf | 0x30;
                    *param_3 = *param_3 & 0xffffffdf;
                    param_3[1] = param_3[1] & 0xfffffffe;
                  }
                  else {
                    param_3[2] = param_3[2] & 0xfffffecf | 0x120;
                    *param_3 = *param_3 | 0x20;
                    param_3[1] = param_3[1] | 1;
                  }
                }
                else if (param_2 == '\0') {
                  param_3[2] = param_3[2] & 0xfffffecf | 0x30;
                  *param_3 = *param_3 & 0xffffffdf;
                }
                else {
                  param_3[2] = param_3[2] & 0xfffffecf | 0x120;
                  *param_3 = *param_3 | 0x20;
                }
              }
              else if (param_2 == '\0') {
                param_3[3] = param_3[3] & 0xfffffffd;
              }
              else {
                param_3[3] = param_3[3] | 2;
              }
            }
            else {
              sane_mode(param_3);
            }
          }
          else {
            *(undefined *)((long)param_3 + 0x13) = 0x7f;
            *(undefined *)(param_3 + 5) = 0x15;
          }
        }
        else if (param_2 == '\0') {
          *param_3 = *param_3 & 0xfffffeff;
          param_3[1] = param_3[1] & 0xfffffffb;
        }
        else {
          *param_3 = *param_3 & 0xfffffe3f | 0x100;
          param_3[1] = param_3[1] & 0xffffffd3 | 4;
        }
      }
      else if (param_2 == '\0') {
        param_3[2] = param_3[2] & 0xfffffccf | 800;
      }
      else {
        param_3[2] = param_3[2] & 0xfffffecf | 0x30;
      }
    }
    else if (param_2 == '\0') {
      param_3[2] = param_3[2] & 0xfffffccf | 0x120;
    }
    else {
      param_3[2] = param_3[2] & 0xfffffecf | 0x30;
    }
  }
  else if (param_2 == '\0') {
    *puVar2 = *puVar2 & ~(uint)param_1[3] | (uint)param_1[2];
  }
  else {
    *puVar2 = *puVar2 & ~(uint)param_1[3] & ~(uint)param_1[2];
  }
  return 1;
}

// Function: set_control_char
void set_control_char(undefined8 *param_1,char *param_2,long param_3) {
  char cVar1;
  byte local_10;
  
  cVar1 = streq(*param_1,&DAT_001051bf);
  if (cVar1 == '\0') {
    cVar1 = streq(*param_1,&DAT_001051c3);
    if (cVar1 == '\0') {
      if ((*param_2 == '\0') || (param_2[1] == '\0')) {
        local_10 = to_uchar((int)*param_2);
      }
      else {
        cVar1 = streq(param_2,&DAT_0010740f);
        if (cVar1 == '\0') {
          cVar1 = streq(param_2,"undef");
          if (cVar1 == '\0') {
            if ((*param_2 == '^') && (param_2[1] != '\0')) {
              if (param_2[1] == '?') {
                local_10 = 0x7f;
              }
              else {
                local_10 = to_uchar((int)param_2[1]);
                local_10 = local_10 & 0x9f;
              }
            }
            else {
              local_10 = integer_arg(param_2,0xff);
            }
            goto LAB_00102f5d;
          }
        }
        local_10 = 0;
      }
      goto LAB_00102f5d;
    }
  }
  local_10 = integer_arg(param_2,0xff);
LAB_00102f5d:
  *(byte *)(param_3 + 0x11 + param_1[2]) = local_10;
  return;
}

// Function: set_speed
void set_speed(int param_1,undefined8 param_2,termios *param_3) {
  speed_t __speed;
  int iVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  
  __speed = string_to_baud(param_2);
  if (__speed == 0xffffffff) {
                    /* WARNING: Subroutine does not return */
    __assert_fail("baud != (speed_t) -1",
                  "/workspace/home/aiclub1/B220032CS_Jaefar/fyp/repos/ansaf/coreutils/src/stty.c",
                  0x6ba,"set_speed");
  }
  if ((param_1 == 0) || (param_1 == 2)) {
    last_ibaud = __speed;
    iVar1 = cfsetispeed(param_3,__speed);
    if (iVar1 != 0) {
      uVar2 = quotearg_style(4,param_2);
      uVar3 = gettext("unsupported ispeed %s");
      error(1,0,uVar3,uVar2);
    }
  }
  if ((param_1 == 1) || (param_1 == 2)) {
    last_obaud = __speed;
    iVar1 = cfsetospeed(param_3,__speed);
    if (iVar1 != 0) {
      uVar2 = quotearg_style(4,param_2);
      uVar3 = gettext("unsupported ospeed %s");
      error(1,0,uVar3,uVar2);
    }
  }
  return;
}

// Function: get_win_size
int get_win_size(int param_1,undefined8 param_2) {
  int iVar1;
  
  iVar1 = ioctl(param_1,0x5413,param_2);
  return iVar1;
}

// Function: set_window_size
void set_window_size(int param_1,int param_2,undefined8 param_3) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  undefined2 local_28;
  undefined2 local_26;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = get_win_size(0,&local_28);
  if (iVar1 != 0) {
    piVar2 = __errno_location();
    if (*piVar2 != 0x16) {
      uVar3 = quotearg_n_style_colon(0,3,param_3);
      piVar2 = __errno_location();
      error(1,*piVar2,&DAT_0010737d,uVar3);
    }
    memset(&local_28,0,8);
  }
  if (-1 < param_1) {
    local_28 = (undefined2)param_1;
  }
  if (-1 < param_2) {
    local_26 = (undefined2)param_2;
  }
  iVar1 = ioctl(0,0x5414,&local_28);
  if (iVar1 != 0) {
    uVar3 = quotearg_n_style_colon(0,3,param_3);
    piVar2 = __errno_location();
    error(1,*piVar2,&DAT_0010737d,uVar3);
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: display_window_size
void display_window_size(char param_1,undefined8 param_2) {
  int iVar1;
  int *piVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  char *pcVar5;
  long in_FS_OFFSET;
  undefined2 local_28;
  undefined2 local_26;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = get_win_size(0,&local_28);
  if (iVar1 != 0) {
    piVar2 = __errno_location();
    if (*piVar2 != 0x16) {
      uVar3 = quotearg_n_style_colon(0,3,param_2);
      piVar2 = __errno_location();
      error(1,*piVar2,&DAT_0010737d,uVar3);
    }
    if (param_1 == '\x01') goto code_r0x0010338b;
    uVar3 = quotearg_n_style_colon(0,3,param_2);
    uVar4 = gettext("%s: no size information for this device");
    error(1,0,uVar4,uVar3);
  }
  if (param_1 == '\0') {
    pcVar5 = "%d %d\n";
  }
  else {
    pcVar5 = "rows %d; columns %d;";
  }
  wrapf(pcVar5,local_28,local_26);
  if (param_1 != '\x01') {
    current_col = 0;
  }
code_r0x0010338b:
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return;
}

// Function: screen_columns
ulong screen_columns(void) {
  int iVar1;
  ulong uVar2;
  long in_FS_OFFSET;
  ulong local_28;
  char *local_20;
  undefined local_18 [2];
  ushort local_16;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  iVar1 = get_win_size(1,local_18);
  if ((iVar1 == 0) && (local_16 != 0)) {
    uVar2 = (ulong)local_16;
  }
  else {
    local_20 = getenv("COLUMNS");
    if ((((local_20 == (char *)0x0) ||
         (iVar1 = xstrtol(local_20,0,0,&local_28,&DAT_00104f24), iVar1 != 0)) ||
        ((long)local_28 < 1)) || (uVar2 = local_28, 0x7fffffff < (long)local_28)) {
      local_28 = 0x50;
      uVar2 = local_28;
    }
  }
  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) {
    return uVar2;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: mode_type_flag
void mode_type_flag(uint param_1) {
                    /* WARNING: Could not recover jumptable at 0x00103492. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  (*(code *)(&DAT_001074f4 + *(int *)(&DAT_001074f4 + (ulong)param_1 * 4)))();
  return;
}

// Function: display_settings
void display_settings(uint param_1,undefined8 param_2,undefined8 param_3) {
  if (param_1 == 2) {
    display_recoverable(param_2);
  }
  else if (param_1 < 3) {
    if (param_1 == 0) {
      display_changed(param_2);
    }
    else if (param_1 == 1) {
      display_all(param_2,param_3);
    }
  }
  return;
}

// Function: display_changed
void display_changed(long param_1) {
  bool bVar1;
  char cVar2;
  undefined8 uVar3;
  uint *puVar4;
  ulong uVar5;
  int local_20;
  int local_1c;
  
  local_1c = 0;
  display_speed(param_1,1);
  wrapf("line = %d;",*(undefined *)(param_1 + 0x10));
  putchar_unlocked(10);
  current_col = 0;
  bVar1 = true;
  local_20 = 0;
  while (cVar2 = streq(*(undefined8 *)(control_info + (long)local_20 * 0x18),&DAT_001051bf),
        cVar2 != '\x01') {
    if ((*(char *)(param_1 + 0x11 + *(long *)(control_info + (long)local_20 * 0x18 + 0x10)) !=
         control_info[(long)local_20 * 0x18 + 8]) &&
       (cVar2 = streq(*(undefined8 *)(control_info + (long)local_20 * 0x18),"flush"), cVar2 == '\0')
       ) {
      bVar1 = false;
      uVar3 = visible(*(undefined *)
                       (param_1 + 0x11 + *(long *)(control_info + (long)local_20 * 0x18 + 0x10)));
      wrapf("%s = %s;",*(undefined8 *)(control_info + (long)local_20 * 0x18),uVar3);
    }
    local_20 = local_20 + 1;
  }
  if ((*(uint *)(param_1 + 0xc) & 2) == 0) {
    wrapf("min = %lu; time = %lu;\n",*(undefined *)(param_1 + 0x17),*(undefined *)(param_1 + 0x16));
  }
  else if (!bVar1) {
    putchar_unlocked(10);
  }
  current_col = 0;
  bVar1 = true;
  local_20 = 0;
  while (*(long *)(mode_info + (long)local_20 * 0x20) != 0) {
    if ((mode_info[(long)local_20 * 0x20 + 0xc] & 8) == 0) {
      if (local_1c != *(int *)(mode_info + (long)local_20 * 0x20 + 8)) {
        if (!bVar1) {
          putchar_unlocked(10);
          current_col = 0;
          bVar1 = true;
        }
        local_1c = *(int *)(mode_info + (long)local_20 * 0x20 + 8);
      }
      puVar4 = (uint *)mode_type_flag(*(undefined4 *)(mode_info + (long)local_20 * 0x20 + 8),param_1
                                     );
      if (*(long *)(mode_info + (long)local_20 * 0x20 + 0x18) == 0) {
        uVar5 = *(ulong *)(mode_info + (long)local_20 * 0x20 + 0x10);
      }
      else {
        uVar5 = *(ulong *)(mode_info + (long)local_20 * 0x20 + 0x18);
      }
      if ((*puVar4 & uVar5) == *(ulong *)(mode_info + (long)local_20 * 0x20 + 0x10)) {
        if ((mode_info[(long)local_20 * 0x20 + 0xc] & 2) != 0) {
          wrapf(&DAT_0010737d,*(undefined8 *)(mode_info + (long)local_20 * 0x20));
          bVar1 = false;
        }
      }
      else if ((mode_info[(long)local_20 * 0x20 + 0xc] & 5) == 5) {
        wrapf(&DAT_00107534,*(undefined8 *)(mode_info + (long)local_20 * 0x20));
        bVar1 = false;
      }
    }
    local_20 = local_20 + 1;
  }
  if (!bVar1) {
    putchar_unlocked(10);
  }
  current_col = 0;
  return;
}

// Function: display_all
void display_all(long param_1,undefined8 param_2) {
  char cVar1;
  undefined8 uVar2;
  uint *puVar3;
  ulong uVar4;
  int local_20;
  int local_1c;
  
  local_1c = 0;
  display_speed(param_1,1);
  display_window_size(1,param_2);
  wrapf("line = %d;",*(undefined *)(param_1 + 0x10));
  putchar_unlocked(10);
  current_col = 0;
  local_20 = 0;
  while( true ) {
    cVar1 = streq(*(undefined8 *)(control_info + (long)local_20 * 0x18),&DAT_001051bf);
    if (cVar1 == '\x01') break;
    cVar1 = streq(*(undefined8 *)(control_info + (long)local_20 * 0x18),"flush");
    if (cVar1 == '\0') {
      uVar2 = visible(*(undefined *)
                       (param_1 + 0x11 + *(long *)(control_info + (long)local_20 * 0x18 + 0x10)));
      wrapf("%s = %s;",*(undefined8 *)(control_info + (long)local_20 * 0x18),uVar2);
    }
    local_20 = local_20 + 1;
  }
  wrapf("min = %lu; time = %lu;",*(undefined *)(param_1 + 0x17),*(undefined *)(param_1 + 0x16));
  if (current_col != 0) {
    putchar_unlocked(10);
  }
  current_col = 0;
  local_20 = 0;
  while (*(long *)(mode_info + (long)local_20 * 0x20) != 0) {
    if ((mode_info[(long)local_20 * 0x20 + 0xc] & 8) == 0) {
      if (local_1c != *(int *)(mode_info + (long)local_20 * 0x20 + 8)) {
        putchar_unlocked(10);
        current_col = 0;
        local_1c = *(int *)(mode_info + (long)local_20 * 0x20 + 8);
      }
      puVar3 = (uint *)mode_type_flag(*(undefined4 *)(mode_info + (long)local_20 * 0x20 + 8),param_1
                                     );
      if (*(long *)(mode_info + (long)local_20 * 0x20 + 0x18) == 0) {
        uVar4 = *(ulong *)(mode_info + (long)local_20 * 0x20 + 0x10);
      }
      else {
        uVar4 = *(ulong *)(mode_info + (long)local_20 * 0x20 + 0x18);
      }
      if ((*puVar3 & uVar4) == *(ulong *)(mode_info + (long)local_20 * 0x20 + 0x10)) {
        wrapf(&DAT_0010737d,*(undefined8 *)(mode_info + (long)local_20 * 0x20));
      }
      else if ((mode_info[(long)local_20 * 0x20 + 0xc] & 4) != 0) {
        wrapf(&DAT_00107534,*(undefined8 *)(mode_info + (long)local_20 * 0x20));
      }
    }
    local_20 = local_20 + 1;
  }
  putchar_unlocked(10);
  current_col = 0;
  return;
}

// Function: check_speed
void check_speed(termios *param_1) {
  speed_t sVar1;
  undefined8 uVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  
  if ((last_ibaud != 0xffffffff) && (last_obaud != 0xffffffff)) {
    sVar1 = cfgetispeed(param_1);
    if ((sVar1 == last_ibaud) && (sVar1 = cfgetospeed(param_1), sVar1 == last_obaud)) {
      return;
    }
    uVar2 = baud_to_value(last_obaud);
    uVar3 = baud_to_value(last_ibaud);
    uVar4 = gettext("asymmetric input (%lu), output (%lu) speeds not supported");
    error(1,0,uVar4,uVar3,uVar2);
  }
  return;
}

// Function: display_speed
void display_speed(termios *param_1,char param_2) {
  speed_t sVar1;
  speed_t sVar2;
  undefined8 uVar3;
  char *pcVar4;
  undefined8 uVar5;
  
  sVar1 = cfgetispeed(param_1);
  if (sVar1 != 0) {
    sVar1 = cfgetispeed(param_1);
    sVar2 = cfgetospeed(param_1);
    if (sVar1 != sVar2) {
      sVar1 = cfgetospeed(param_1);
      uVar3 = baud_to_value(sVar1);
      sVar1 = cfgetispeed(param_1);
      uVar5 = baud_to_value(sVar1);
      if (param_2 == '\0') {
        pcVar4 = "%lu %lu\n";
      }
      else {
        pcVar4 = "ispeed %lu baud; ospeed %lu baud;";
      }
      wrapf(pcVar4,uVar5,uVar3);
      goto LAB_00103e8a;
    }
  }
  sVar1 = cfgetospeed(param_1);
  uVar3 = baud_to_value(sVar1);
  if (param_2 == '\0') {
    pcVar4 = "%lu\n";
  }
  else {
    pcVar4 = "speed %lu baud;";
  }
  wrapf(pcVar4,uVar3);
LAB_00103e8a:
  if (param_2 != '\x01') {
    current_col = 0;
  }
  return;
}

// Function: display_recoverable
void display_recoverable(uint *param_1) {
  ulong local_10;
  
  printf("%lx:%lx:%lx:%lx",(ulong)*param_1,(ulong)param_1[1],(ulong)param_1[2],(ulong)param_1[3]);
  for (local_10 = 0; local_10 < 0x20; local_10 = local_10 + 1) {
    printf(":%lx",(ulong)*(byte *)((long)param_1 + local_10 + 0x11));
  }
  putchar_unlocked(10);
  return;
}

// Function: strtoul_tcflag_t
undefined8
strtoul_tcflag_t(char *param_1,undefined4 param_2,char **param_3,undefined4 *param_4,char param_5) {
  int *piVar1;
  ulong uVar2;
  undefined8 uVar3;
  
  piVar1 = __errno_location();
  *piVar1 = 0;
  uVar2 = __isoc23_strtoul(param_1,param_3,param_2);
  piVar1 = __errno_location();
  if ((((*piVar1 == 0) && (param_5 == **param_3)) && (param_1 != *param_3)) &&
     (uVar2 == (uVar2 & 0xffffffff))) {
    *param_4 = (int)uVar2;
    uVar3 = 0;
  }
  else {
    uVar3 = 0xffffffff;
  }
  return uVar3;
}

// Function: strtoul_cc_t
undefined8
strtoul_cc_t(char *param_1,undefined4 param_2,char **param_3,undefined *param_4,char param_5) {
  int *piVar1;
  ulong uVar2;
  undefined8 uVar3;
  
  piVar1 = __errno_location();
  *piVar1 = 0;
  uVar2 = __isoc23_strtoul(param_1,param_3,param_2);
  piVar1 = __errno_location();
  if ((((*piVar1 == 0) && (param_5 == **param_3)) && (param_1 != *param_3)) &&
     (uVar2 == (uVar2 & 0xff))) {
    *param_4 = (char)uVar2;
    uVar3 = 0;
  }
  else {
    uVar3 = 0xffffffff;
  }
  return uVar3;
}

// Function: recover_mode
undefined8 recover_mode(long param_1,undefined4 *param_2) {
  undefined uVar1;
  int iVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  long local_40;
  long local_38;
  ulong local_30;
  undefined4 local_28 [6];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_38 = param_1;
  for (local_30 = 0; local_30 < 4; local_30 = local_30 + 1) {
    iVar2 = strtoul_tcflag_t(local_38,0x10,&local_40,local_28 + local_30,0x3a);
    if (iVar2 != 0) {
      uVar3 = 0;
      goto LAB_0010418e;
    }
    local_38 = local_40 + 1;
  }
  *param_2 = local_28[0];
  param_2[1] = local_28[1];
  param_2[2] = local_28[2];
  param_2[3] = local_28[3];
  local_30 = 0;
  do {
    if (0x1f < local_30) {
      uVar3 = 1;
LAB_0010418e:
      if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
        __stack_chk_fail();
      }
      return uVar3;
    }
    if (local_30 < 0x1f) {
      uVar1 = 0x3a;
    }
    else {
      uVar1 = 0;
    }
    iVar2 = strtoul_cc_t(local_38,0x10,&local_40,(long)param_2 + local_30 + 0x11,uVar1);
    if (iVar2 != 0) {
      uVar3 = 0;
      goto LAB_0010418e;
    }
    local_38 = local_40 + 1;
    local_30 = local_30 + 1;
  } while( true );
}

// Function: string_to_baud
undefined8 string_to_baud(char *param_1) {
  char *pcVar1;
  char cVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  char *local_30;
  char local_22;
  char *local_20;
  ulong local_18;
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_30 = param_1;
  while( true ) {
    cVar2 = c_isspace((int)*local_30);
    if (cVar2 == '\0') break;
    local_30 = local_30 + 1;
  }
  if (*local_30 == '-') {
    uVar3 = 0xffffffff;
  }
  else {
    local_18 = __isoc23_strtoul(local_30,&local_20,10);
    pcVar1 = local_20 + 1;
    cVar2 = *local_20;
    if (cVar2 == '.') {
      local_20 = local_20 + 2;
      local_22 = *pcVar1;
      if (local_22 != '\0') {
        if ((byte)(local_22 - 0x30U) < 6) {
          if ((byte)(local_22 - 0x30U) == 5) {
            do {
              pcVar1 = local_20 + 1;
              local_22 = *local_20;
              local_20 = pcVar1;
            } while (local_22 == '0');
            if (local_22 == '\0') {
              local_18 = local_18 + 1 & 0xfffffffffffffffe;
            }
            else {
              local_18 = local_18 + 1;
            }
          }
        }
        else {
          local_18 = local_18 + 1;
        }
        while( true ) {
          cVar2 = c_isdigit(local_22);
          if (cVar2 == '\0') break;
          local_22 = *local_20;
          local_20 = local_20 + 1;
        }
        if (local_22 != '\0') {
          uVar3 = 0xffffffff;
          goto LAB_001048f9;
        }
      }
    }
    else {
      local_20 = pcVar1;
      if (cVar2 != '\0') {
        cVar2 = streq(local_30,&DAT_0010765c);
        if (cVar2 == '\0') {
          cVar2 = streq(local_30,&DAT_00107661);
          if (cVar2 == '\0') {
            uVar3 = 0xffffffff;
          }
          else {
            uVar3 = 0xf;
          }
        }
        else {
          uVar3 = 0xe;
        }
        goto LAB_001048f9;
      }
    }
    uVar3 = value_to_baud(local_18);
  }
LAB_001048f9:
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return uVar3;
}

// Function: sane_mode
void sane_mode(long param_1) {
  uint *puVar1;
  int local_14;
  
  local_14 = 0;
  while (*(long *)(control_info + (long)local_14 * 0x18) != 0) {
    *(undefined1 *)(param_1 + 0x11 + *(long *)(control_info + (long)local_14 * 0x18 + 0x10)) =
         control_info[(long)local_14 * 0x18 + 8];
    local_14 = local_14 + 1;
  }
  local_14 = 0;
  while (*(long *)(mode_info + (long)local_14 * 0x20) != 0) {
    if ((mode_info[(long)local_14 * 0x20 + 0xc] & 0x10) == 0) {
      if ((mode_info[(long)local_14 * 0x20 + 0xc] & 1) == 0) {
        if ((mode_info[(long)local_14 * 0x20 + 0xc] & 2) != 0) {
          puVar1 = (uint *)mode_type_flag(*(undefined4 *)(mode_info + (long)local_14 * 0x20 + 8),
                                          param_1);
          *puVar1 = *puVar1 & ~(uint)*(undefined8 *)(mode_info + (long)local_14 * 0x20 + 0x18) &
                    ~(uint)*(undefined8 *)(mode_info + (long)local_14 * 0x20 + 0x10);
        }
      }
      else {
        puVar1 = (uint *)mode_type_flag(*(undefined4 *)(mode_info + (long)local_14 * 0x20 + 8),
                                        param_1);
        *puVar1 = *puVar1 & ~(uint)*(undefined8 *)(mode_info + (long)local_14 * 0x20 + 0x18) |
                  (uint)*(undefined8 *)(mode_info + (long)local_14 * 0x20 + 0x10);
      }
    }
    local_14 = local_14 + 1;
  }
  return;
}

// Function: visible
char * visible(byte param_1) {
  char *pcVar1;
  undefined *local_10;
  
  if (param_1 == 0) {
    pcVar1 = "<undef>";
  }
  else {
    if (param_1 < 0x20) {
      buf_0 = 0x5e;
      DAT_00104dc1 = param_1 + 0x40;
      local_10 = &DAT_00104dc2;
    }
    else if (param_1 < 0x7f) {
      local_10 = &DAT_00104dc1;
      buf_0 = param_1;
    }
    else if (param_1 == 0x7f) {
      buf_0 = 0x5e;
      local_10 = &DAT_00104dc2;
      DAT_00104dc1 = '?';
    }
    else {
      buf_0 = 0x4d;
      DAT_00104dc1 = '-';
      if (param_1 < 0xa0) {
        DAT_00104dc2 = '^';
        DAT_00104dc3 = param_1 - 0x40;
        local_10 = &DAT_00104dc4;
      }
      else if (param_1 == 0xff) {
        DAT_00104dc2 = '^';
        local_10 = &DAT_00104dc4;
        DAT_00104dc3 = '?';
      }
      else {
        DAT_00104dc2 = param_1 + 0x80;
        local_10 = &DAT_00104dc3;
      }
    }
    *local_10 = 0;
    pcVar1 = (char *)&buf_0;
  }
  return pcVar1;
}

// Function: integer_arg
void integer_arg(undefined8 param_1,undefined8 param_2) {
  undefined8 uVar1;
  
  uVar1 = gettext("invalid integer argument");
  xnumtoumax(param_1,0,0,param_2,&DAT_00107687,uVar1,0,0);
  return;
}

