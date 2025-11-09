char * chal_cat_to_string(int param_1) {
  switch(param_1) {
  case 0:
    return "Crypto";
  case 1:
    return "Pwnable";
  case 2:
    return "Forensics";
  case 3:
    return "Web";
  case 4:
    return "Network";
  case 5:
    return "Reversing";
  case 6:
    return "Misc.";
  default:
    return "Unknown";
  }
}

char * chal_status_to_string(unsigned int param_1) {
  switch(param_1) {
  case 0:
    return "Locked";
  case 1:
    return "Solved";
  case 2:
    return "Cleared";
  case 3:
    return "Open";
  default:
    return "Unknown";
  }
}