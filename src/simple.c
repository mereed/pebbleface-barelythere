#include "pebble.h"

GColor background_color = GColorBlack;
GColor text_color = GColorWhite;

Window *window;
TextLayer *layer_date_text;
TextLayer *layer_date_text2;
TextLayer *layer_wday_text;
TextLayer *layer_wday_text2;
TextLayer *layer_time_text;
TextLayer *layer_time_text2;
TextLayer *layer_month_text;
TextLayer *layer_month_text2;

static GFont *time_font;
static GFont *week_font;
static GFont *day_font;
static GFont *batt_font;

int cur_day = -1;
int charge_percent = 0;

TextLayer *battery_text_layer;
TextLayer *battery_text_layer2;

InverterLayer *inverter_layer_bt = NULL;
InverterLayer *inverter_layer_batt = NULL;



void update_battery_state(BatteryChargeState charge_state) {
    static char battery_text[] = "xxxx xxxxx x100%";

    if (charge_state.is_charging) {

        snprintf(battery_text, sizeof(battery_text), "+%d%%", charge_state.charge_percent);
    } else {
        snprintf(battery_text, sizeof(battery_text), "Batt. about %d%%", charge_state.charge_percent);
        
    }
    charge_percent = charge_state.charge_percent;
    
    text_layer_set_text(battery_text_layer, battery_text);
    text_layer_set_text(battery_text_layer2, battery_text);
} 

void handle_bluetooth(bool connected) {

    if (connected) {

	// Remove Inverter layer
    layer_remove_from_parent(inverter_layer_get_layer(inverter_layer_bt));
    inverter_layer_destroy(inverter_layer_bt);
    inverter_layer_bt = NULL;
    
    } else {
		
	// Add inverter layer
    Layer *window_layer = window_get_root_layer(window);

    inverter_layer_bt = inverter_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer_bt));

   }
}	

void update_time(struct tm *tick_time) {

	static char time_text[] = "00:00";
    static char date_text[] = "00xx";
    static char wday_text[] = "xxxxxxxxx";
	static char month_text[] = "xxxxxxxxx";
    
    char *time_format;

    int new_cur_day = tick_time->tm_year*1000 + tick_time->tm_yday;
    if (new_cur_day != cur_day) {
        cur_day = new_cur_day;

			switch(tick_time->tm_mday)
  {
    case 1 :
    case 21 :
    case 31 :
      strftime(date_text, sizeof(date_text), "%%est", tick_time);
      break;
    case 2 :
    case 22 :
      strftime(date_text, sizeof(date_text), "%end", tick_time);
      break;
    case 3 :
    case 23 :
      strftime(date_text, sizeof(date_text), "%erd", tick_time);
      break;
    default :
      strftime(date_text, sizeof(date_text), "%eth", tick_time);
      break;
  }
	
	  text_layer_set_text(layer_date_text, date_text);
	  text_layer_set_text(layer_date_text2, date_text);
		
		
        strftime(wday_text, sizeof(wday_text), "%A", tick_time);
        text_layer_set_text(layer_wday_text, wday_text);
        text_layer_set_text(layer_wday_text2, wday_text);
		
		strftime(month_text, sizeof(month_text), "%B", tick_time);
        text_layer_set_text(layer_month_text, month_text);
        text_layer_set_text(layer_month_text2, month_text);
				
    }

    if (clock_is_24h_style()) {
        time_format = "%R";
		
    } else {
        time_format = "%I:%M";
				
    }

    strftime(time_text, sizeof(time_text), time_format, tick_time);

    if (!clock_is_24h_style() && (time_text[0] == '0')) {
        memmove(time_text, &time_text[1], sizeof(time_text) - 1);
    }

    text_layer_set_text(layer_time_text, time_text);
    text_layer_set_text(layer_time_text2, time_text);
}

void set_style(void) {
    
    background_color  = GColorBlack;
    text_color = GColorWhite;
	
	// set-up layer colours
    window_set_background_color(window, background_color);
    text_layer_set_text_color(layer_time_text, text_color);
    text_layer_set_text_color(layer_time_text2, background_color);
    text_layer_set_text_color(layer_wday_text, text_color);
    text_layer_set_text_color(layer_wday_text2, background_color);
    text_layer_set_text_color(layer_date_text, text_color);
    text_layer_set_text_color(layer_date_text2, background_color);
    text_layer_set_text_color(battery_text_layer, text_color);
    text_layer_set_text_color(battery_text_layer2, background_color);
    text_layer_set_text_color(layer_month_text, text_color);
    text_layer_set_text_color(layer_month_text2, background_color);
 
}

void force_update(void) {
    handle_bluetooth(bluetooth_connection_service_peek());
    time_t now = time(NULL);
    update_time(localtime(&now));
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    update_time(tick_time);
}


void handle_init(void) {
	
    window = window_create();
    window_stack_push(window, true);
 
	// resources
	time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FREESANS_48));
    week_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FREESANS_20));
    day_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FREESANS_24));
    batt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_FREESANS_16));
	
    // layer position and alignment
    layer_time_text = text_layer_create(GRect(-1, 60, 144, 50));
    layer_time_text2 = text_layer_create(GRect(0, 61, 144, 50));
    layer_wday_text = text_layer_create(GRect(-1, 39, 144, 28));
    layer_wday_text2 = text_layer_create(GRect(0, 40, 144, 28));
    layer_date_text = text_layer_create(GRect(-1, 137, 144, 24));
    layer_date_text2 = text_layer_create(GRect(0, 138, 144, 24));
    battery_text_layer = text_layer_create(GRect(-1, -2, 144, 22));
    battery_text_layer2 = text_layer_create(GRect(0, -1, 144, 22));
    layer_month_text = text_layer_create(GRect(-1, 114, 144, 22));
    layer_month_text2 = text_layer_create(GRect(0, 115, 144, 22));

    text_layer_set_background_color(layer_wday_text, GColorClear);
    text_layer_set_font(layer_wday_text, day_font);
	text_layer_set_background_color(layer_wday_text2, GColorClear);
    text_layer_set_font(layer_wday_text2, day_font);
    text_layer_set_background_color(layer_date_text, GColorClear);
    text_layer_set_font(layer_date_text, week_font);
	text_layer_set_background_color(layer_date_text2, GColorClear);
    text_layer_set_font(layer_date_text2, week_font);
    text_layer_set_background_color(layer_time_text, GColorClear);
    text_layer_set_font(layer_time_text, time_font);
	text_layer_set_background_color(layer_time_text2, GColorClear);
    text_layer_set_font(layer_time_text2, time_font);
    text_layer_set_background_color(battery_text_layer, GColorClear);
    text_layer_set_background_color(battery_text_layer2, GColorClear);
    text_layer_set_font(battery_text_layer, batt_font);
    text_layer_set_font(battery_text_layer2, batt_font);
	text_layer_set_background_color(layer_month_text, GColorClear);
    text_layer_set_font(layer_month_text, week_font);
	text_layer_set_background_color(layer_month_text2, GColorClear);
    text_layer_set_font(layer_month_text2, week_font);
	
	text_layer_set_text_alignment(layer_wday_text, GTextAlignmentCenter);
	text_layer_set_text_alignment(layer_wday_text2, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_date_text, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_date_text2, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_time_text, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_time_text2, GTextAlignmentCenter);
    text_layer_set_text_alignment(battery_text_layer, GTextAlignmentCenter);
    text_layer_set_text_alignment(battery_text_layer2, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_month_text, GTextAlignmentCenter);
    text_layer_set_text_alignment(layer_month_text2, GTextAlignmentCenter);

    // composing layers
    Layer *window_layer = window_get_root_layer(window);

    layer_add_child(window_layer, text_layer_get_layer(layer_wday_text));
    layer_add_child(window_layer, text_layer_get_layer(layer_wday_text2));
    layer_add_child(window_layer, text_layer_get_layer(layer_date_text));
    layer_add_child(window_layer, text_layer_get_layer(layer_date_text2));
    layer_add_child(window_layer, text_layer_get_layer(layer_time_text));
    layer_add_child(window_layer, text_layer_get_layer(layer_time_text2));
    layer_add_child(window_layer, text_layer_get_layer(battery_text_layer));
    layer_add_child(window_layer, text_layer_get_layer(battery_text_layer2));
    layer_add_child(window_layer, text_layer_get_layer(layer_month_text));
    layer_add_child(window_layer, text_layer_get_layer(layer_month_text2));
	
	inverter_layer_batt = inverter_layer_create(GRect(0, 0, 144, 17));
    layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer_batt));
	
    set_style();
	
    // handlers
    battery_state_service_subscribe(update_battery_state);
    bluetooth_connection_service_subscribe(&handle_bluetooth);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

	// update the battery on launch
    update_battery_state(battery_state_service_peek());
	
    // draw first frame
    force_update();
}

void handle_deinit(void) {
	
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
	
  text_layer_destroy( layer_time_text );
  text_layer_destroy( layer_time_text2 );
  text_layer_destroy( layer_wday_text );
  text_layer_destroy( layer_wday_text2 );
  text_layer_destroy( layer_date_text );
  text_layer_destroy( layer_date_text2 );
  text_layer_destroy( battery_text_layer );
  text_layer_destroy( battery_text_layer2 );
  text_layer_destroy( layer_month_text );
  text_layer_destroy( layer_month_text2 );
	
  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(day_font);
  fonts_unload_custom_font(week_font);
  fonts_unload_custom_font(batt_font);

	  inverter_layer_destroy( inverter_layer_batt );

	
  window_destroy(window);

}

int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}
