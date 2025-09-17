#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pwr_detect_shell, LOG_LEVEL_INF);

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/init.h>
#include <app/subsys/pwr_detect.h>

static int cmd_get_mv(const struct shell *shell, size_t argc, char **argv)
{
	int raw = get_pwr_raw();
	int mv = raw2mv(raw);

	if (mv < 0) {
		shell_print(shell, "Power reading in mV not available");
	} else {
		shell_print(shell, "Latest power reading: %d mV", mv);
	}
	return 0;
}

static int cmd_get_raw(const struct shell *shell, size_t argc, char **argv)
{
	int raw = get_pwr_raw();
	shell_print(shell, "Latest raw power reading: %d", raw);
	return 0;
}

static int cmd_get_source(const struct shell *shell, size_t argc, char **argv)
{
	int source = get_pwr_source();
	switch (source) {
		case PWR_SOURCE_BATTERY:
			shell_print(shell, "Power source: Battery");
			break;
		case PWR_SOURCE_USB:
			shell_print(shell, "Power source: USB");
			break;
		case PWR_SOURCE_12V:
			shell_print(shell, "Power source: 12V");
			break;
		default:
			shell_print(shell, "Power source: Unknown");
			break;
	}
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_pwr_detect,
	SHELL_CMD(get-mv, NULL, "Get the latest power reading in mV", cmd_get_mv),
	SHELL_CMD(get-raw, NULL, "Get the latest raw power reading", cmd_get_raw),
	SHELL_CMD(get-source, NULL, "Get the current power source", cmd_get_source),
	SHELL_SUBCMD_SET_END /* Array terminated. */
);

SHELL_CMD_REGISTER(pwr_detect, &sub_pwr_detect, "Power detection commands", NULL);

static int pwr_detect_shell_init(void)
{
	LOG_INF("pwr_detect_shell_init()");
	return 0;
}

SYS_INIT(pwr_detect_shell_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
