#pragma once

#include "typedef.h"

typedef enum{
  APP_STATE_RUNNING,
  APP_STATE_STOPPED
} AppState;

void set_app_state(AppState state);
AppState get_app_state();