// Function: add_user
bool add_user(void) {
  char cVar1;
  size_t sVar2;
  int local_24;
  int local_20;
  int local_1c;
  void *local_18;
  char *local_14;
  int local_10;
  
  local_20 = 1;
  local_10 = 0;
  local_14 = (char *)0x0;
  local_18 = (void *)0x0;
  local_1c = 0;
  do {
    if (local_20 == 0) {
      local_18 = (void *)q_and_a("Enter a short \"about me\" section.: ",0x200,line,0x800,&local_20,
                                 0);
      if (local_20 == 0) {
        local_24 = create_user(local_14,local_18);
        cVar1 = insert_in_order(&g_all_users,local_24,compare_users);
        if (cVar1 == '\0') {
          if (local_24 != 0) {
            delete_user(&local_24);
          }
          printf("Cannot add user at this time\n");
        }
        else {
          printf("Successfully added user\n");
          g_num_users = g_num_users + 1;
        }
      }
      break;
    }
    local_1c = find_user_from_list(line,0x800,&local_20,g_all_users,g_num_users);
    if (local_20 == -1) break;
    if (local_20 == 0) {
      if (((local_1c != 0) || (sVar2 = strlen(line), sVar2 == 0)) ||
         (sVar2 = strlen(line), 0x80 < sVar2)) {
        local_20 = 1;
        goto LAB_00012b7e;
      }
      local_14 = strdup(line);
    }
    else {
LAB_00012b7e:
      printf("Try again\n");
    }
    local_10 = local_10 + 1;
  } while (local_10 != 5);
  if (local_14 != (char *)0x0) {
    free(local_14);
  }
  if (local_18 != (void *)0x0) {
    free(local_18);
  }
  return local_20 != -1;
}

// Function: remove_user
undefined4 remove_user(void) {
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  int local_14;
  int local_10 [2];
  
  local_10[0] = 1;
  local_14 = find_user_from_list(line,0x800,local_10,g_all_users,g_num_users);
  if (local_10[0] == -1) {
    uVar2 = 0;
  }
  else if (local_14 == 0) {
    printf("User not found\n");
    uVar2 = 1;
  }
  else {
    local_14 = pop(&g_all_users,local_14,compare_users);
    iVar3 = compare_users(local_14,g_cur_user);
    if (iVar3 == 0) {
      g_cur_user = 0;
    }
    cVar1 = delete_user(&local_14);
    if (cVar1 != '\0') {
      printf("Suceessfully deleted user\n");
      g_num_users = g_num_users + -1;
    }
    uVar2 = 1;
  }
  return uVar2;
}

// Function: set_current_user
undefined4 set_current_user(void) {
  int iVar1;
  undefined4 uVar2;
  int local_14;
  int local_10;
  
  local_14 = 1;
  local_10 = 0;
  print_user_list("------All Users------\n",g_all_users);
  printf("Entering a username will set it as the current user\n");
  local_10 = find_user_from_list(line,0x800,&local_14,g_all_users,g_num_users);
  if (local_14 == -1) {
    uVar2 = 0;
  }
  else {
    iVar1 = local_10;
    if (local_10 == 0) {
      printf("User not set\n");
      iVar1 = g_cur_user;
    }
    g_cur_user = iVar1;
    uVar2 = 1;
  }
  return uVar2;
}

// Function: change_cur_date
undefined4 change_cur_date(void) {
  char cVar1;
  undefined4 uVar2;
  int local_14;
  void *local_10;
  
  local_14 = 1;
  local_10 = (void *)0x0;
  local_10 = (void *)q_and_a("Enter new date (MM-DD-YYYY): ",10,line,0x800,&local_14,1);
  if (local_14 == -1) {
    uVar2 = 0;
  }
  else {
    if (local_10 == (void *)0x0) {
      printf("Try again\n");
    }
    else {
      cVar1 = strtodate(local_10,0x1c000);
      if (cVar1 == '\0') {
        printf("Date not changed\n");
      }
      else {
        printf("Date successfully changed\n");
        g_view_date = g_cur_date;
      }
      free(local_10);
    }
    uVar2 = 1;
  }
  return uVar2;
}

// Function: change_view_date
undefined4 change_view_date(void) {
  char cVar1;
  undefined4 uVar2;
  int local_14;
  void *local_10;
  
  local_14 = 1;
  local_10 = (void *)0x0;
  local_10 = (void *)q_and_a("Enter date to view (MM-DD-YYYY): ",10,line,0x800,&local_14,1);
  if (local_14 == -1) {
    uVar2 = 0;
  }
  else {
    if (local_10 == (void *)0x0) {
      printf("Try again\n");
    }
    else {
      cVar1 = strtodate(local_10,&g_view_date);
      if (cVar1 == '\0') {
        printf("Date not changed\n");
      }
      else {
        printf("Successfully changed calendar view date\n");
      }
      free(local_10);
    }
    uVar2 = 1;
  }
  return uVar2;
}

// Function: add_user_event
undefined4 add_user_event(int param_1) {
  char cVar1;
  size_t sVar2;
  int iVar3;
  char local_68 [32];
  undefined4 local_48;
  undefined2 local_44;
  undefined4 local_42;
  undefined2 local_3e;
  undefined2 local_3c;
  undefined2 local_3a;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  void *local_2c;
  void *local_28;
  char local_21;
  char *local_20;
  char *local_1c;
  char *local_18;
  int local_14;
  int local_10;
  
  if (param_1 == 0) {
    printf("Must set a user first\n");
    return 1;
  }
  local_30 = 1;
  local_10 = 0;
  local_14 = 0;
  local_1c = (char *)0x0;
  local_20 = (char *)0x0;
  local_21 = '\0';
  local_28 = (void *)0x0;
  do {
    if (local_10 != 0) {
      do {
        if (local_14 != 0) {
          while (local_1c == (char *)0x0) {
            local_1c = (char *)q_and_a("Start Date (MM-DD-YYYY): ",10,line,0x800,&local_30,0);
            if (local_30 == -1) {
              return 0;
            }
            local_18 = strdup(local_1c);
            cVar1 = strtodate(local_1c,&local_34);
            if (cVar1 == '\0') {
              if (local_1c != (char *)0x0) {
                free(local_18);
                local_18 = (char *)0x0;
                free(local_1c);
                local_1c = (char *)0x0;
              }
              printf("Try again\n");
            }
          }
          free(local_1c);
          local_1c = (char *)0x0;
          while (local_1c == (char *)0x0) {
            sprintf(local_68,"End Date (MM-DD-YYYY) [%s]: ",local_18);
            local_1c = (char *)q_and_a(local_68,10,line,0x800,&local_30,1);
            if (local_30 == -1) {
              return 0;
            }
            if ((local_1c == (char *)0x0) || (sVar2 = strlen(local_1c), sVar2 != 0)) {
              cVar1 = strtodate(local_1c,&local_38);
              if ((cVar1 == '\0') || (iVar3 = compare_date(&local_38,&local_34), iVar3 < 0)) {
                if (local_1c != (char *)0x0) {
                  free(local_1c);
                  local_1c = (char *)0x0;
                }
                printf("Try again\n");
              }
            }
            else {
              local_38 = local_34;
            }
          }
          free(local_18);
          local_18 = (char *)0x0;
          free(local_1c);
          local_1c = (char *)0x0;
          while (local_20 == (char *)0x0) {
            local_20 = (char *)q_and_a("All day event? (y/n) [n]: ",0x800,line,0x800,&local_30,1);
            if (local_30 == -1) {
              return 0;
            }
            if ((local_20 == (char *)0x0) || (sVar2 = strlen(local_20), sVar2 != 0)) {
              if (local_20 != (char *)0x0) {
                if (local_30 == 0) {
                  if ((*local_20 == 'y') || (*local_20 == 'Y')) {
                    local_21 = '\x01';
                  }
                }
                else {
                  free(local_20);
                  local_20 = (char *)0x0;
                }
              }
            }
            else {
              local_21 = '\0';
            }
          }
          free(local_20);
          local_20 = (char *)0x0;
          if (local_21 == '\0') {
            while (local_28 == (void *)0x0) {
              local_28 = (void *)q_and_a("Start Time (HH:MM): ",5,line,0x800,&local_30,0);
              if (local_30 == -1) {
                return 0;
              }
              cVar1 = strtotime(local_28,&local_3a);
              if (cVar1 == '\0') {
                if (local_28 != (void *)0x0) {
                  free(local_28);
                  local_28 = (void *)0x0;
                }
                printf("Try again\n");
              }
            }
            free(local_28);
            local_28 = (void *)0x0;
            while (local_28 == (void *)0x0) {
              sprintf(local_68,"End time (HH:MM): ",0);
              local_28 = (void *)q_and_a(local_68,5,line,0x800,&local_30,0);
              if (local_30 == -1) {
                return 0;
              }
              cVar1 = strtotime(local_28,&local_3c);
              if ((cVar1 == '\0') ||
                 ((iVar3 = compare_date(&local_34,&local_38), iVar3 == 0 &&
                  (iVar3 = compare_time(&local_3c,&local_3a), iVar3 < 1)))) {
                if (local_28 != (void *)0x0) {
                  free(local_28);
                  local_28 = (void *)0x0;
                }
                printf("Try again\n");
              }
            }
            free(local_28);
            local_28 = (void *)0x0;
          }
          else {
            set_time(&local_3a,0,0);
            set_time(&local_3c,0x17,0x3b);
          }
          local_48 = local_34;
          local_44 = local_3a;
          local_42 = local_38;
          local_3e = local_3c;
          local_2c = (void *)create_event(param_1,local_10,local_14,&local_48,local_21);
          cVar1 = add_calendar_event(param_1 + 0x284,local_2c);
          if (cVar1 == '\0') {
            if (local_2c != (void *)0x0) {
              free(local_2c);
              local_2c = (void *)0x0;
            }
            printf("Could not add event\n");
          }
          else {
            print_event(local_2c);
          }
          return 1;
        }
        local_14 = q_and_a("Event Description: ",0x100,line,0x800,&local_30,0);
      } while (local_30 != -1);
      return 0;
    }
    local_10 = q_and_a("Event Name: ",0x80,line,0x800,&local_30,0);
  } while (local_30 != -1);
  return 0;
}

// Function: remove_user_event
undefined4 remove_user_event(int param_1) {
  char cVar1;
  undefined4 uVar2;
  int local_14;
  int local_10;
  
  local_14 = 1;
  local_10 = find_event_from_list(line,0x800,&local_14,*(undefined4 *)(g_cur_user + 0x288));
  if (local_14 == -1) {
    uVar2 = 0;
  }
  else if (local_10 == 0) {
    printf("Event not found\n");
    uVar2 = 1;
  }
  else {
    cVar1 = remove_calendar_event(param_1 + 0x284,local_10);
    if (cVar1 == '\0') {
      printf("Could not remove event\n");
    }
    else {
      printf("Successfully removed event\n");
    }
    uVar2 = 1;
  }
  return uVar2;
}

// Function: user_menu
undefined4 user_menu(void) {
  size_t sVar1;
  int iVar2;
  undefined4 uVar3;
  uint local_14;
  undefined *local_10;
  
  local_14 = 0;
  if (g_cur_user != 0) {
    sVar1 = strlen((char *)(g_cur_user + 4));
    if (sVar1 != 0) {
      local_10 = (undefined *)(g_cur_user + 4);
      goto LAB_000136c1;
    }
  }
  local_10 = &DAT_0001842d;
LAB_000136c1:
  printf("Options:\n");
  printf("1. View users\n");
  printf("2. Add user\n");
  printf("3. Remove user\n");
  printf("4. Set user\n");
  printf("5. View Calendar\n");
  printf("6. Change Date\n");
  printf("7. Exit\n");
  printf("---Current User is *%s*---\n",local_10);
  printf("---Today\'s date is ");
  print_date(0x1c000);
  printf("---\n");
  printf("Selection: ");
  iVar2 = readnum(line,0x800,&local_14);
  if (iVar2 == -1) {
    uVar3 = 0;
  }
  else {
    if (local_14 < 8) {
                    /* WARNING: Could not recover jumptable at 0x000137e9. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      uVar3 = (*(code *)((int)&_GLOBAL_OFFSET_TABLE_ + *(int *)(&DAT_000184e4 + local_14 * 4)))();
      return uVar3;
    }
    printf("Try again\n");
    uVar3 = 1;
  }
  return uVar3;
}

// Function: calendar_menu
undefined4 calendar_menu(undefined *param_1) {
  size_t sVar1;
  int iVar2;
  undefined4 uVar3;
  uint local_14;
  int local_10;
  
  local_14 = 0;
  if ((g_cur_user == 0) || (sVar1 = strlen((char *)(g_cur_user + 4)), sVar1 == 0)) {
    printf("Must select a user before viewing a calendar\n");
    *param_1 = 0;
    return 1;
  }
  local_10 = g_cur_user + 4;
  printf("Options:\n");
  printf("1. View Day\n");
  printf("2. View Month\n");
  printf("3. View Agenda\n");
  printf("4. Change Date\n");
  printf("5. Change View Date\n");
  printf("6. Add Calendar Event\n");
  printf("7. Remove Calendar Event\n");
  printf("8. Return to User Selection\n");
  printf("---Current User is *%s*---\n",local_10);
  printf("---Today\'s date is ");
  print_date(0x1c000);
  printf("---\n");
  printf("---Calendar view date is ");
  print_date(&g_view_date);
  printf("---\n");
  printf("Selection: ");
  iVar2 = readnum(line,0x800,&local_14);
  if (iVar2 == -1) {
    return 0;
  }
  if (8 < local_14) {
    printf("Try again\n");
    return 1;
  }
                    /* WARNING: Could not recover jumptable at 0x00013a38. Too many branches */
                    /* WARNING: Treating indirect jump as call */
  uVar3 = (*(code *)((int)&_GLOBAL_OFFSET_TABLE_ + *(int *)(&DAT_000185ec + local_14 * 4)))();
  return uVar3;
}

// Function: check_seed
void check_seed(void) {
  FILE *__s;
  int local_10 [2];
  
  __s = (FILE *)0x13b14;
  local_10[0] = 0;
  fread(local_10,4,(size_t)stdin,(FILE *)0x13b14);
  if (_DAT_4347c000 == local_10[0]) {
    fwrite(&DAT_4347c000,0x1000,(size_t)stdout,__s);
  }
  return;
}

// Function: main
undefined4 main(void) {
  char local_1d;
  undefined4 local_1c;
  undefined4 local_18;
  char local_11;
  undefined *local_c;
  
  local_c = &stack0x00000004;
  local_18 = 0;
  local_1c = 1;
  local_1d = '\0';
  local_11 = '\x01';
  check_seed();
  while (local_11 != '\0') {
    if (local_1d == '\0') {
      local_11 = user_menu(&local_1d);
    }
    else {
      local_11 = calendar_menu(&local_1d);
    }
  }
  return 0;
}

