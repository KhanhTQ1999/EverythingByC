#include "app_state.h"

static AppState s_state = APP_STATE_STOPPED;

void set_app_state(AppState state) {
  s_state = state;
}

AppState get_app_state() {
  return s_state;
}