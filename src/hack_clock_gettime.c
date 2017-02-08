//
//  hack_clock_gettime.c
//  hack_clock_gettime
//
//  Created by huke on 2/8/17.
//  Copyright (c) 2017 com.cocoahuke. All rights reserved.
//

#include <mach/mach.h>
#include <mach/clock.h>
#include <time.h>
#include "fishhook.h"

int (*orig_clock_gettime)(int clk_id,struct timespec *tp);
int my_clock_gettime(int clk_id,struct timespec *tp);

int my_clock_gettime(int clk_id,struct timespec *tp){
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(),REALTIME_CLOCK,&cclock);
    clock_get_time(cclock,&mts);
    mach_port_deallocate(mach_task_self(),cclock);
    tp->tv_sec = mts.tv_sec;
    tp->tv_nsec = mts.tv_nsec;
    return 0;
}

__attribute__((constructor)) static void initialize(){
    rebind_symbols((struct rebinding[1]){{"clock_gettime",my_clock_gettime,(void*)&orig_clock_gettime}},1);
}
