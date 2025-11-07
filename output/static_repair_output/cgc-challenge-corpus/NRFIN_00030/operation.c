// Function: process_remote_player_empty_hand
int process_remote_player_empty_hand(void) {
  undefined uVar1;
  int iVar2;
  
  iVar2 = is_player_hand_empty(*(undefined4 *)(game + 8));
  if (iVar2 == 1) {
    iVar2 = recv_draw_request();
    if (iVar2 != 0) {
      return iVar2;
    }
    uVar1 = get_hand_size();
    iVar2 = draw_new_hand(*(undefined4 *)(game + 8),*(undefined4 *)(game + 4),uVar1);
    if ((iVar2 < 0) && (iVar2 != -0x2b)) {
      return iVar2;
    }
    send_hand(*(undefined4 *)(*(int *)(game + 8) + 8));
  }
  return 0;
}

// Function: process_bot_player_empty_hand
int process_bot_player_empty_hand(void) {
  undefined uVar1;
  int iVar2;
  int local_10;
  
  local_10 = 0;
  iVar2 = is_player_hand_empty(*(undefined4 *)(game + 0xc));
  if (iVar2 == 1) {
    uVar1 = get_hand_size();
    local_10 = draw_new_hand(*(undefined4 *)(game + 0xc),*(undefined4 *)(game + 4),uVar1);
    if ((-1 < local_10) || (local_10 == -0x2b)) {
      local_10 = 0;
    }
  }
  return local_10;
}

// Function: process_remote_player_fishing
int process_remote_player_fishing(char param_1) {
  int local_14;
  int local_10;
  
  local_10 = 0;
  local_14 = 0;
  local_10 = recv_fish_request();
  if (local_10 == 0) {
    if (**(char **)(game + 4) == '\0') {
      send_cards(&local_14,0);
    }
    else {
      local_10 = take_top_card(*(undefined4 *)(game + 8),*(undefined4 *)(game + 4));
      if ((local_10 == 0) &&
         (local_10 = get_players_newest_card(*(undefined4 *)(game + 8),&local_14), local_10 == 0)) {
        send_cards(&local_14,1);
        if (param_1 == *(char *)(local_14 + 1)) {
          local_10 = recv_and_match_cards(&local_14,1);
        }
        else {
          local_14 = 0;
          local_10 = recv_and_match_cards(&local_14,0);
        }
        if (local_10 == 0) {
          local_10 = -0x2a;
        }
        else {
          local_10 = 0;
        }
      }
    }
  }
  return local_10;
}

// Function: process_bot_player_fishing
int process_bot_player_fishing(char param_1) {
  int local_14;
  int local_10;
  
  local_10 = 0;
  local_14 = 0;
  if (**(char **)(game + 4) != '\0') {
    local_10 = take_top_card(*(undefined4 *)(game + 0xc),*(undefined4 *)(game + 4));
    if (local_10 != 0) {
      return local_10;
    }
    local_10 = get_players_newest_card(*(undefined4 *)(game + 0xc),&local_14);
    if (local_10 != 0) {
      return local_10;
    }
    if (param_1 == *(char *)(local_14 + 1)) {
      send_cards(&local_14,1);
    }
    else {
      send_cards(&local_14,0);
    }
  }
  return 0;
}

// Function: process_remote_player_accept_cards
void process_remote_player_accept_cards(undefined4 param_1,undefined param_2) {
  send_cards(param_1,param_2);
  accept_cards(*(undefined4 *)(game + 8),param_1,param_2);
  return;
}

// Function: process_bot_player_accept_cards
int process_bot_player_accept_cards(undefined4 param_1,undefined param_2) {
  int local_10;
  
  local_10 = recv_and_match_cards(param_1,param_2);
  if (local_10 == 1) {
    local_10 = accept_cards(*(undefined4 *)(game + 0xc),param_1,param_2);
  }
  else if (local_10 == 0) {
    local_10 = -0x2a;
  }
  return local_10;
}

// Function: process_remote_player_ask
uint process_remote_player_ask(void) {
  bool bVar1;
  int iVar2;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  
  local_14 = 0;
  local_18 = 0;
  local_10 = 0;
  local_28 = 0;
  local_24 = 0;
  local_20 = 0;
  local_1c = 0;
  local_14 = recv_ask_and_get_rank();
  if ((int)local_14 < 0) {
    if ((local_14 == 0xffffffd4) &&
       (iVar2 = is_hand_empty(*(undefined4 *)(*(int *)(game + 8) + 8)), iVar2 == 1)) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (!bVar1) {
      return local_14;
    }
  }
  local_18 = have_cards_of_rank(*(undefined4 *)(game + 0xc),local_14 & 0xff,&local_28);
  if (-1 < (int)local_18) {
    send_go_fish_notice(local_18 & 0xff);
  }
  if (local_18 == 0) {
    local_10 = process_remote_player_fishing(local_14 & 0xff);
  }
  else if (((local_18 == 1) || (local_18 == 2)) || (local_18 == 3)) {
    local_10 = process_remote_player_accept_cards(&local_28,local_18 & 0xff);
  }
  else {
    local_10 = local_18;
  }
  turn_complete(game);
  return local_10;
}

// Function: process_bot_player_ask
uint process_bot_player_ask(void) {
  uint uVar1;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  uint local_18;
  uint local_14;
  uint local_10;
  
  local_10 = 0;
  local_18 = 0;
  local_14 = 0;
  local_28 = 0;
  local_24 = 0;
  local_20 = 0;
  local_1c = 0;
  uVar1 = select_random_card(*(undefined4 *)(game + 0xc));
  if ((-1 < (int)uVar1) || (uVar1 == 0xffffffe8)) {
    local_10 = uVar1;
    if (uVar1 == 0xffffffe8) {
      local_10 = 0;
    }
    send_ask(local_10 & 0xff);
    local_18 = have_cards_of_rank(*(undefined4 *)(game + 8),local_10 & 0xff,&local_28);
    uVar1 = recv_go_fish_notice();
    if (uVar1 == local_18) {
      local_14 = uVar1;
      if (local_18 == 0) {
        local_14 = process_bot_player_fishing(local_10 & 0xff);
      }
      else if (((local_18 == 1) || (local_18 == 2)) || (local_18 == 3)) {
        local_14 = process_bot_player_accept_cards(&local_28,local_18 & 0xff);
      }
      turn_complete(game);
      uVar1 = local_14;
    }
    else if (-1 < (int)uVar1) {
      uVar1 = 0xffffffcc;
    }
  }
  return uVar1;
}

// Function: process_remote_player_books
uint process_remote_player_books(void) {
  uint local_10;
  
  local_10 = play_books(*(undefined4 *)(game + 8));
  if (-1 < (int)local_10) {
    local_10 = recv_and_match_count_books_played(local_10 & 0xff);
    if (local_10 == 1) {
      local_10 = 0;
    }
    else if (local_10 == 0) {
      local_10 = 0xffffffcc;
    }
  }
  return local_10;
}

// Function: process_bot_player_books
uint process_bot_player_books(void) {
  uint uVar1;
  
  uVar1 = play_books(*(undefined4 *)(game + 0xc));
  if (-1 < (int)uVar1) {
    send_count_books_played(uVar1 & 0xff);
    uVar1 = 0;
  }
  return uVar1;
}

// Function: do_bot_turn
int do_bot_turn(void) {
  int iVar1;
  
  send_turn_notice(1);
  iVar1 = process_bot_player_empty_hand();
  if (((iVar1 == 0) && (iVar1 = process_bot_player_ask(), iVar1 == 0)) &&
     (iVar1 = process_bot_player_books(), iVar1 == 0)) {
    iVar1 = 0;
  }
  return iVar1;
}

// Function: do_player_turn
int do_player_turn(void) {
  int iVar1;
  
  send_turn_notice(0);
  iVar1 = process_remote_player_empty_hand();
  if (((iVar1 == 0) && (iVar1 = process_remote_player_ask(), iVar1 == 0)) &&
     (iVar1 = process_remote_player_books(), iVar1 == 0)) {
    iVar1 = 0;
  }
  return iVar1;
}

// Function: do_turn
undefined4 do_turn(void) {
  int iVar1;
  undefined4 local_10;
  
  iVar1 = is_player_turn(game);
  if (iVar1 == 1) {
    local_10 = do_player_turn();
  }
  else {
    local_10 = do_bot_turn();
  }
  return local_10;
}

// Function: play_game
int play_game(void) {
  int iVar1;
  undefined4 local_14;
  int local_10;
  
  local_10 = 0;
  local_14 = 0;
  local_10 = recv_player_name(&local_14);
  if (local_10 == 0) {
    game = create_game(local_14);
    local_10 = deal(game);
    if (local_10 == 0) {
      local_10 = send_initial_hand_to_player(game);
      while (local_10 == 0) {
        iVar1 = is_game_over(game);
        if (iVar1 != 0) {
          send_final_results(game);
          return local_10;
        }
        local_10 = do_turn();
      }
    }
  }
  return local_10;
}

