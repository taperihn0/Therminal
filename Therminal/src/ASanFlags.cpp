
/* Here we encode Adress Sanitizer flags.
*  We ignore leaks for now, since video drivers seems
*  to be producing big memory leak log inside the terminal.
*/

extern "C"
const char* __asan_default_options() 
{ 
   return "detect_leaks=0"; 
}

