#include <lvgl.h>
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <AXP192.h>


hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


/*Change to your screen resolution*/
static const uint16_t screenWidth  = 135;
static const uint16_t screenHeight = 240;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */
AXP192 axp = AXP192();

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

//////////////////////////////////
lv_indev_t * my_indev;
// static void keypad_init(void);
// static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
// static uint32_t keypad_get_key(void);
// static void mouse_get_xy(lv_coord_t * x, lv_coord_t * y);
//////////////////////////////////
static void keypad_init(void)
{
    /*Your code comes here*/
    pinMode(37,INPUT_PULLUP);
    pinMode(39,INPUT_PULLUP);
}
/*Get the currently being pressed key.  0 if no key is pressed*/
static uint32_t keypad_get_key(void)
{
    /*Your code comes here*/
    if (digitalRead(39) == 0) {
        Serial.println("下一个");
        return 1;   //和 LV_KEY_LEFT 对应 
    } else if (digitalRead(37) == 0) {
        Serial.println("回车");
        return 5;   //和 LV_KEY_RIGHT 对应 
    }
    return 0;
}

/*Will be called by the library to read the mouse*/
static void keypad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static uint32_t last_key = 0;

    /*Get the current x and y coordinates*/
    // mouse_get_xy(&data->point.x, &data->point.y);

    // Serial.println("读键盘键值");
    /*Get whether the a key is pressed and save the pressed key*/
    uint32_t act_key = keypad_get_key();
    if(act_key != 0) {
        data->state = LV_INDEV_STATE_PR;

        /*Translate the keys to LVGL control characters according to your key definitions*/
        switch(act_key) {
        case 1:
            act_key = LV_KEY_NEXT;
            break;
        case 2:
            act_key = LV_KEY_PREV;
            break;
        case 3:
            act_key = LV_KEY_LEFT;
            break;
        case 4:
            act_key = LV_KEY_RIGHT;
            break;
        case 5:
            act_key = LV_KEY_ENTER;
            break;
        }

        last_key = act_key;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->key = last_key;
}

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;

    keypad_init();

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = keypad_read;
    
    my_indev = lv_indev_drv_register(&indev_drv);
    Serial.println("键盘已经初始化");
    /*Later you should create group(s) with `lv_group_t * group = lv_group_create()`,
     *add objects to the group with `lv_group_add_obj(group, obj)`
     *and assign this input device to group to navigate in it:
     *`lv_indev_set_group(indev_keypad, group);`*/
}

void light_CB(lv_event_t * e)
{
	// Your code here
    if (lv_obj_get_state(ui_Switch1) == 7)
        digitalWrite(10,LOW);
    else if (lv_obj_get_state(ui_Switch1) == 6)
        digitalWrite(10,HIGH);
    // Serial.println(lv_obj_get_state(ui_Switch2));
}



/*------------------
 * Keypad
 * -----------------*/

/*Initialize your keypad*/

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  lv_tick_inc(5);
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */

    lv_init();

    axp.begin();
    axp.ScreenBreath(9);
    tft.fillScreen(TFT_BLACK); //用某一颜色填充屏幕
    tft.begin();          /* TFT init */
    tft.setRotation( 0 ); /* Landscape orientation, flipped */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

    pinMode(10,OUTPUT);
    pinMode(2,OUTPUT);
    digitalWrite(10,HIGH);
    digitalWrite(2,LOW);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    ui_init();
    lv_port_indev_init();
    lv_group_t * group = lv_group_create();
    lv_group_add_obj(group,ui_Switch1);
    lv_indev_set_group(my_indev,group);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 5000, true);
    timerAlarmEnable(timer);
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    
    delay( 5 );
}
