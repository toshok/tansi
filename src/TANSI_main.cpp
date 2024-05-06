
extern "C" void tansi_setup(void);
extern "C" void tansi_main_loop(void);

#ifdef USE_ARDUINO
extern "C" void setup()
{
  tansi_setup();
}

extern "C" void loop()
{
  tansi_main_loop();
}
#else
extern "C" int main(void)
{
  tansi_setup();
  while (1)
  {
    tansi_main_loop();
  }
}
#endif