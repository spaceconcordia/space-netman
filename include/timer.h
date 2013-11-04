#ifndef TIMER_H_
#define TIMER_H_

#include <inttypes.h>
#include <stdbool.h>

typedef /*

whatever is neccessary for the implementation

*/ timer_t;



// This function should set the duration of `timer`
//
// `timeout` is the amount of time until this timer finishes.
// The units should be whatever makes most sense for us, I dunno
// if that is milliseconds or seconds, but probably its one of those
// two. If some type other than uint32_t makes more sense for this
// then it should be changed
void timer_set_timeout(timer_t * timer, uint32_t timeout);


// This function starts `timer`. It should be assumed that `timer`
// has already had its timeout value set with `timer_set_timeout`
//
// This function should reset the timer when it is called, and should be
// able to be called multiple times in a row without causing problems
void timer_start(timer_t * timer);

// This function should return true if it has been more than `timeout`
// units of time since `timer` was started, and false otherwise.
bool timer_complete(timer_t * timer);

/*

Sample Usage:

This is how the timer will be used in the NETMAN

timer_t window_timer;
timer_set_timeout(&window_timer, WINDOW_TIMEOUT);

timer_t resend_timer;
timer_set_timeout(&resend_timer, RESEND_TIMEOUT);

while(session_active){


   if(waiting_for_ack && timer_complete(&resend_timer)){
      // We sent something, and have waited for a response but
      // didn't get one. We will send the same thing again, because
      // it probably got lost.
      resend_data();
      timer_start(&resend_timer);
   }else if(new_data_to_send){
      // We have new data to send, so we will send it
      send_data();
      timer_start(&resend_timer);
   }

   ...

   if(successfully_received_something){
      // Any time we receive good data, we should restart the window
      // timer
      timer_start(&window_timer);
   }

   ...

   if(timer_complete(&window_timer)){
      // If the window timer completes, it means we haven't received
      // anything in a while, and the session should be ended.
      close_session();
      session_active = false;
   }
}

...


*/


# endif
