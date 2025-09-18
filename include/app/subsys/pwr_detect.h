

#ifndef APP_SUBSYS_PWR_DETECT_H_
#define APP_SUBSYS_PWR_DETECT_H_

enum Power_source {
    PWR_SOURCE_UNKNOWN = 0,
    PWR_SOURCE_BATTERY,
    PWR_SOURCE_USB,
    PWR_SOURCE_12V,
};

int get_pwr_raw(void);
int raw2mv(int raw);
int get_pwr_source(void);
#endif /* APP_SUBSYS_PWR_DETECT_H_ */
