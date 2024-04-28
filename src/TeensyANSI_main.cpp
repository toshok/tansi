
extern "C" void teensyansi_setup(void);
extern "C" void teensyansi_main_loop(void);

#ifdef USE_ARDUINO
extern "C" void setup()
{
  teensyansi_setup();
}

extern "C" void loop()
{
  teensyansi_main_loop();
}
#else
extern "C" int main(void)
{
  teensyansi_setup();
  while (1)
  {
    teensyansi_main_loop();
  }
}
#endif