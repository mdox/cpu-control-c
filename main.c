#include <math.h>
#include <glob.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

static double TEMP_NORMAL = 50.0;
static double TEMP_TOLERATE = 70.0;
static double TEMP_CRITICAL = 90.0;

const double read_number(const char *filepath);
const double get_temp();
const char *get_governor();
const double get_freq();
const double set_freq(const double freq);

int main(int argc, char *argv[])
{
  for (int opt; (opt = getopt(argc, argv, "n:t:c:")) != -1;)
  {
    switch (opt)
    {
    case 'n':
      TEMP_NORMAL = atof(optarg);
      break;
    case 't':
      TEMP_TOLERATE = atof(optarg);
      break;
    case 'c':
      TEMP_CRITICAL = atof(optarg);
      break;
    }
  }

  const double min_freq = read_number("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_min_freq");
  const double max_freq = read_number("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");

  if (min_freq <= 0.0)
    return 1;
  if (max_freq <= 0.0)
    return 1;

  while (true)
  {
    if (strstr(get_governor(), "powersave"))
      continue;

    const double temp = get_temp();

    if (temp >= TEMP_TOLERATE)
      set_freq(fmax(min_freq, get_freq() - (max_freq - min_freq) * 0.1 * (temp - TEMP_TOLERATE) / (TEMP_CRITICAL - TEMP_TOLERATE)));
    else if (temp >= TEMP_NORMAL)
      set_freq(fmin(max_freq, get_freq() + (max_freq - min_freq) * 0.1 * (temp - TEMP_NORMAL) / (TEMP_TOLERATE - TEMP_NORMAL)));
    else if (temp >= 0.0)
      set_freq(max_freq);

    sleep(1);
  }

  return 0;
}

const double read_number(const char *filepath)
{
  double result = 0.0;
  FILE *file;
  if (file = fopen(filepath, "r"))
  {
    fscanf(file, "%lf", &result);
    fclose(file);
  }
  return result;
}

const double get_temp()
{
  double result = 0.0;
  glob_t glob_instance;
  memset(&glob_instance, 0, sizeof(glob_t));
  if (!glob("/sys/class/thermal/thermal_zone*/temp", GLOB_DOOFFS, NULL, &glob_instance))
  {
    for (int i = 0; i < glob_instance.gl_pathc; ++i)
    {
      result = fmax(result, read_number(glob_instance.gl_pathv[i]));
    }
    globfree(&glob_instance);
  }
  return result / 1000.0;
}

const char *get_governor()
{
  static char result[16];
  FILE *file;
  if (file = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "r"))
  {
    memset(result, 0, 16);
    fscanf(file, "%15[a-z]", result);
    fclose(file);
  }
  return result;
}

const double get_freq()
{
  return read_number("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
}

const double set_freq(double freq)
{
  glob_t glob_instance;
  memset(&glob_instance, 0, sizeof(glob_t));
  if (!glob("/sys/devices/system/cpu/cpu*/cpufreq/scaling_max_freq", GLOB_DOOFFS, NULL, &glob_instance))
  {
    for (int i = 0; i < glob_instance.gl_pathc; ++i)
    {
      FILE *file;
      if (file = fopen(glob_instance.gl_pathv[i], "w"))
      {
        fprintf(file, "%d", (int)(freq));
        fclose(file);
      }
    }
    globfree(&glob_instance);
  }
}
