#include <pebble.h>
#include <PDUtils.h>

#define KEY_DATE_FOR 0
#define KEY_FAJR 1
#define KEY_SHUROOQ 2
#define KEY_DHUHR 3
#define KEY_ASR 4
#define KEY_MAGHRIB 5
#define KEY_ISHA 6

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_day_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_till_next_prayer_layer;
// static BitmapLayer *s_current_prayer_img_layer;
// static BitmapLayer *s_next_prayer_img_layer;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  static char date_for[10];
  // static char fajr[8];
  // static char shurooq[8];
  // static char dhuhr[8];
  // static char asr[8];
  // static char maghrib[8];
  // static char isha[8];

  Tuple *date_for_tuple = dict_find(iterator, KEY_DATE_FOR);
  // Tuple *fajr_tuple = dict_find(iterator, KEY_FAJR);
  // Tuple *shurooq_tuple = dict_find(iterator, KEY_SHUROOQ);
  // Tuple *dhuhr_tuple = dict_find(iterator, KEY_DHUHR);
  // Tuple *asr_tuple = dict_find(iterator, KEY_ASR);
  // Tuple *maghrib_tuple = dict_find(iterator, KEY_MAGHRIB);
  // Tuple *isha_tuple = dict_find(iterator, KEY_ISHA);
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Received info from JS...!");

  snprintf(date_for, sizeof(date_for), "%s", date_for_tuple->value->cstring);
  text_layer_set_text(s_time_till_next_prayer_layer, date_for);
  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_second(struct tm *tick_time) {
  time_t t = time(NULL);
	struct tm *now = localtime(&t);

  static char s_time_till_next_prayer_buffer[16];

	// Set the current time
	time_t seconds_now = p_mktime(now);

	now->tm_year = 15 + 100;
	now->tm_mon = 10 - 1;
	now->tm_mday = 23;
	now->tm_hour = 19;
	now->tm_min = 24;
	now->tm_sec = 0;

	time_t seconds_event = p_mktime(now);

	// Determine the time difference
	int difference = seconds_event - seconds_now;

	if(difference < 0) {
		difference = 0;
	}

	// Set the countdown display
	snprintf(s_time_till_next_prayer_buffer, sizeof(s_time_till_next_prayer_buffer), "%d", difference);
	// text_layer_set_text(s_time_till_next_prayer_layer, s_time_till_next_prayer_buffer);
}


static void update_minute() {

  // build a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // write hours:minutes
  static char s_time_buffer[8];
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ?
    "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(s_time_layer, s_time_buffer);

  // write day of week
  static char s_day_buffer[8];
  strftime(s_day_buffer, sizeof(s_day_buffer), "%A", tick_time);
  text_layer_set_text(s_day_layer, s_day_buffer);

  // write date
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%b %e, %Y", tick_time);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler_second(struct tm *tick_time, TimeUnits units_changed) {
  update_second(tick_time);
}

static void tick_handler_minute(struct tm *tick_time, TimeUnits units_changed) {
  update_minute();
}

static void main_window_load(Window *window) {

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = text_layer_create(GRect(0, 0, bounds.size.w, 42));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_day_layer = text_layer_create(GRect(0, 42, bounds.size.w, 32));
  text_layer_set_background_color(s_day_layer, GColorClear);
  text_layer_set_text_color(s_day_layer, GColorBlack);
  text_layer_set_font(s_day_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_day_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));

  s_date_layer = text_layer_create(GRect(0, 72, bounds.size.w, 30));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  int bottom_row_position_top = 168 - 30;
  // int bottom_row_width = bounds.size.w / 2;
  // s_time_till_next_prayer_layer = text_layer_create(
  //   GRect(0, bottom_row_position_top, bottom_row_width, 30));
  s_time_till_next_prayer_layer = text_layer_create(
    GRect(0, bottom_row_position_top, bounds.size.w, 30));
  text_layer_set_background_color(s_time_till_next_prayer_layer, GColorClear);
  text_layer_set_text_color(s_time_till_next_prayer_layer, GColorBlack);
  text_layer_set_font(s_time_till_next_prayer_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_time_till_next_prayer_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_till_next_prayer_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_till_next_prayer_layer);
}

static void init(void) {

  // register handlers for clock ticks
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler_second);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler_minute);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_second(tick_time);
  update_minute();

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
