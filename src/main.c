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
static TextLayer *s_fajr_layer;
static TextLayer *s_shurooq_layer;
static TextLayer *s_dhuhr_layer;
static TextLayer *s_asr_layer;
static TextLayer *s_maghrib_layer;
static TextLayer *s_isha_layer;

static struct tm s_fajr;
static struct tm s_shurooq;
static struct tm s_dhuhr;
static struct tm s_asr;
static struct tm s_maghrib;
static struct tm s_isha;

static char str_fajr[8];
static char str_shurooq[8];
static char str_dhuhr[8];
static char str_asr[8];
static char str_maghrib[8];
static char str_isha[8];
  
static bool s_has_prayer_times = 0;

static struct tm build_prayer_time_next(int year, int month, int day, DictionaryIterator *iterator) {  
  Tuple *hour_tuple = dict_read_next(iterator);
  Tuple *minute_tuple = dict_read_next(iterator);
  time_t temp = time(NULL);  
  struct tm *prayer_time = localtime(&temp);    
  prayer_time->tm_year = year;
  prayer_time->tm_mon = month;
  prayer_time->tm_mday = day;  
  prayer_time->tm_hour = (int)hour_tuple->value->int32;
  prayer_time->tm_min = (int)minute_tuple->value->int32;
  prayer_time->tm_sec = 0;  
  return *prayer_time;
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  APP_LOG(APP_LOG_LEVEL_INFO, "Received info from JS");
  
  // date components
  Tuple *date_year_tuple = dict_read_first(iterator);  
  Tuple *date_month_tuple = dict_read_next(iterator);
  Tuple *date_day_tuple = dict_read_next(iterator);
  
  int year = (int)date_year_tuple->value->int32;
  int month = (int)date_month_tuple->value->int32;
  int day = (int)date_day_tuple->value->int32;
    
  char str_ymd[32];
  snprintf(str_ymd, sizeof(str_ymd), "h-m-hs -- %i:%i:%i", year, month, day); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_ymd);  
  
  s_fajr = build_prayer_time_next(year, month, day, iterator);  
  s_shurooq = build_prayer_time_next(year, month, day, iterator);
  s_dhuhr = build_prayer_time_next(year, month, day, iterator);
  s_asr = build_prayer_time_next(year, month, day, iterator);  
  s_maghrib = build_prayer_time_next(year, month, day, iterator);
  s_isha = build_prayer_time_next(year, month, day, iterator);  
  
  s_has_prayer_times = 1;  

  strftime(str_fajr, sizeof(str_fajr), "%H:%M", &s_fajr);  
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_fajr);
  text_layer_set_text(s_fajr_layer, str_fajr);

  strftime(str_shurooq, sizeof(str_shurooq), "%H:%M", &s_shurooq); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_shurooq);
  text_layer_set_text(s_shurooq_layer, str_shurooq);  
  
  strftime(str_dhuhr, sizeof(str_dhuhr), "%H:%M", &s_dhuhr); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_dhuhr);  
  text_layer_set_text(s_dhuhr_layer, str_dhuhr);
  
  strftime(str_asr, sizeof(str_asr), "%H:%M", &s_asr); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_asr);
  text_layer_set_text(s_asr_layer, str_asr);  

  strftime(str_maghrib, sizeof(str_maghrib), "%H:%M", &s_maghrib); 
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_maghrib); 
  text_layer_set_text(s_maghrib_layer, str_maghrib);     
  
  strftime(str_isha, sizeof(str_isha), "%H:%M", &s_isha);  
  APP_LOG(APP_LOG_LEVEL_DEBUG, str_isha);
  text_layer_set_text(s_isha_layer, str_isha);  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message from JS dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send to JS failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send to JS success!");
}

static void update_minute() {
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Updating minutes");

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

  int width = bounds.size.w / 2;
  int height = 22;
  
  int row1_y = 168 - (height * 3);
  int row2_y = 168 - (height * 2);
  int row3_y = 168 - height;

  int col1_left = 0;
  int col2_left = width;
  
  // +-------------------+
  // | fajr    | asr     |
  // | shurooq | maghrib |
  // | dhuhr   | isha    |
  // +-------------------+
  
  // fajr
  s_fajr_layer = text_layer_create(GRect(col1_left, row1_y, width, height));  
  text_layer_set_background_color(s_fajr_layer, GColorClear);
  text_layer_set_text_color(s_fajr_layer, GColorBlack);
  text_layer_set_font(s_fajr_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_fajr_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_fajr_layer));  
  
  // shurooq
  s_shurooq_layer = text_layer_create(GRect(col1_left, row2_y, width, height));
  text_layer_set_background_color(s_shurooq_layer, GColorClear);
  text_layer_set_text_color(s_shurooq_layer, GColorBlack);
  text_layer_set_font(s_shurooq_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_shurooq_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_shurooq_layer));  
  
  // dhuhr
  s_dhuhr_layer = text_layer_create(GRect(col1_left, row3_y, width, height));
  text_layer_set_background_color(s_dhuhr_layer, GColorClear);
  text_layer_set_text_color(s_dhuhr_layer, GColorBlack);
  text_layer_set_font(s_dhuhr_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_dhuhr_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_dhuhr_layer));
  
  // asr
  s_asr_layer = text_layer_create(GRect(col2_left, row1_y, width, height));
  text_layer_set_background_color(s_asr_layer, GColorClear);
  text_layer_set_text_color(s_asr_layer, GColorBlack);
  text_layer_set_font(s_asr_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_asr_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_asr_layer));
  
  // maghrib
  s_maghrib_layer = text_layer_create(GRect(col2_left, row2_y, width, height));
  text_layer_set_background_color(s_maghrib_layer, GColorClear);
  text_layer_set_text_color(s_maghrib_layer, GColorBlack);
  text_layer_set_font(s_maghrib_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_maghrib_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_maghrib_layer));
  
  // isha
  s_isha_layer = text_layer_create(GRect(col2_left, row3_y, width, height));
  text_layer_set_background_color(s_isha_layer, GColorClear);
  text_layer_set_text_color(s_isha_layer, GColorBlack);
  text_layer_set_font(s_isha_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_isha_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_isha_layer));
    
  text_layer_set_text(s_fajr_layer, "loading");
  text_layer_set_text(s_shurooq_layer, "loading");
  text_layer_set_text(s_dhuhr_layer, "loading");
  text_layer_set_text(s_asr_layer, "loading");
  text_layer_set_text(s_maghrib_layer, "loading");
  text_layer_set_text(s_isha_layer, "loading");
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_day_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_time_till_next_prayer_layer);
  text_layer_destroy(s_fajr_layer);
  text_layer_destroy(s_shurooq_layer);
  text_layer_destroy(s_dhuhr_layer);
  text_layer_destroy(s_asr_layer);
  text_layer_destroy(s_maghrib_layer);
  text_layer_destroy(s_isha_layer);
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
