#include <pebble.h>
#include <PDUtils.h>

#define KEY_DATE_YEAR 0
#define KEY_DATE_MONTH 1
#define KEY_DATE_DAY 2
#define KEY_FAJR_HOUR 3
#define KEY_FAJR_MINUTE 4
#define KEY_SHUROOQ_HOUR 5
#define KEY_SHUROOQ_MINUTE 6
#define KEY_DHUHR_HOUR 7
#define KEY_DHUHR_MINUTE 8
#define KEY_ASR_HOUR 9
#define KEY_ASR_MINUTE 10
#define KEY_MAGHRIB_HOUR 11
#define KEY_MAGHRIB_MINUTE 12
#define KEY_ISHA_HOUR 13
#define KEY_ISHA_MINUTE 14

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_day_layer;
static TextLayer *s_date_layer;
static TextLayer *s_time_till_next_prayer_layer;
// static BitmapLayer *s_current_prayer_img_layer;
// static BitmapLayer *s_next_prayer_img_layer;
static struct tm *s_fajr;
static struct tm *s_shurooq;
static struct tm *s_dhuhr;
static struct tm *s_asr;
static struct tm *s_maghrib;
static struct tm *s_isha;

static void build_prayer_time(struct tm *prayer_time, int year, int month, int day,
                              DictionaryIterator *iterator, const uint32_t hour_key, const uint32_t minute_key) {
  
  time_t t = time(NULL);
  
  Tuple *prayer_time_hour_tuple = dict_find(iterator, hour_key);
  Tuple *prayer_time_minute_tuple = dict_find(iterator, minute_key);
  
  prayer_time->tm_year = year;
  prayer_time->tm_mon = month;
  prayer_time->tm_mday = day;  
  prayer_time->tm_hour = (int)prayer_time_hour_tuple->value->int32;
  prayer_time->tm_min = (int)prayer_time_minute_tuple->value->int32;
  prayer_time->tm_sec = 0;  
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  APP_LOG(APP_LOG_LEVEL_INFO, "Received info from JS...!");
  
  // date components
  Tuple *date_year_tuple = dict_find(iterator, KEY_DATE_YEAR);  
  Tuple *date_month_tuple = dict_find(iterator, KEY_DATE_MONTH);
  Tuple *date_day_tuple = dict_find(iterator, KEY_DATE_DAY);
  
  int year = (int)date_year_tuple->value->int32;
  int month = (int)date_month_tuple->value->int32;
  int day = (int)date_day_tuple->value->int32;

  build_prayer_time(s_fajr, year, month, day, iterator, KEY_FAJR_HOUR, KEY_FAJR_MINUTE);  
  build_prayer_time(s_shurooq, year, month, day, iterator, KEY_SHUROOQ_HOUR, KEY_SHUROOQ_MINUTE);
  build_prayer_time(s_dhuhr, year, month, day, iterator, KEY_DHUHR_HOUR, KEY_DHUHR_MINUTE);
  build_prayer_time(s_asr, year, month, day, iterator, KEY_ASR_HOUR, KEY_ASR_MINUTE);  
  build_prayer_time(s_maghrib, year, month, day, iterator, KEY_MAGHRIB_HOUR, KEY_MAGHRIB_MINUTE);
  build_prayer_time(s_isha, year, month, day, iterator, KEY_ISHA_HOUR, KEY_ISHA_MINUTE);
  
  static char s_output_buffer[16];
  strftime(s_output_buffer, sizeof(s_output_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", s_asr);
  text_layer_set_text(s_time_till_next_prayer_layer, s_output_buffer);
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

static void update_second() {
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  time_t t = time(NULL);
	struct tm *now = localtime(&t);

  static char s_time_till_next_prayer_buffer[16];
    
  // Get prayer times every hour
  if(tick_time->tm_sec % 10 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);  
    app_message_outbox_send();
  }
  
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
// 	snprintf(s_time_till_next_prayer_buffer, sizeof(s_time_till_next_prayer_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", difference);
//  text_layer_set_text(s_time_till_next_prayer_layer, s_time_till_next_prayer_buffer);
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
  static char s_day_buffer[16];
  strftime(s_day_buffer, sizeof(s_day_buffer), "%A", tick_time);
  text_layer_set_text(s_day_layer, s_day_buffer);

  // write date
  static char s_date_buffer[16];
  strftime(s_date_buffer, sizeof(s_date_buffer), "%e %B", tick_time);
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
  update_second();
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
