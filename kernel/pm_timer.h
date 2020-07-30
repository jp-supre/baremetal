#pragma once


void init_acpi_pm_timer(struct RSDP *rsdp);

void pm_timer_wait_millisec(unsigned int msec);
